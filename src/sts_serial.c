/*(C) copyright 2008, Steven Snyder, All Rights Reserved

Steven T. Snyder, <stsnyder@ucla.edu> http://www.steventsnyder.com

LICENSING INFORMATION:
 This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>
#include <linux/serial.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <string.h> // for strerror
#include <stdlib.h> // for malloc
#include <sys/time.h>
#include "sts_serial.h"

// global variables
// ================

char sts_serial_read_seq_timeout = 0; // timeout flag for read_sequence()

// serial helper function prototypes
// ====================================================

int serial_connect(const char* portpath, int flags, speed_t baudrate);
// attempts to connect to a serial device at portpath (i.e. "/dev/ttyUSB0")
// sets raw mode and sets the baud rate to baudrate. (see `man termios`)
// returns the file descriptor for the connection, or -1 if an error occurs.
// the flags field is the same as the flags for a call to read()


int set_custom_baud_rate(int fport, unsigned int desired_baudrate);
// Attempts to set the baud rate to the closest rate possible to 
// the desired_baudrate argument using divisors.
// fport is the file descriptor for the port opened by a call to serial_connect() or open()
// Divisor method:
//   it should be possible to do custom baud rates by using a divisor, like
//   you would do when you call "setserial /dev/ttyS0 baud_base 115200 divisor 14 spd_cust"
//   If the call to setserial wouldn't work for the device, the divisor method wont work here either.
// 	 This is usually due to an unimplemented ioctl function in the device driver.
// Termios custom baud rate method:
//   I think tty_ioctl.c has to be compiled into the kernel with BOTHER defined for this to work.
//   set cbaud to BOTHER and c_ospeed to the desired setting.
//   This is done through a call to set_custom_baud_rate_no_ioctl()



int set_custom_baud_rate_no_ioctl(int fport, unsigned int desired_baudrate);
// XXX this is an untested function (my architecture doesn't have BOTHER defined)
// Termios custom baud rate method:
//   tty_ioctl.c has to be compiled into the kernel with BOTHER defined for this to work.
//   This is called through set_custom_baud_rate() if a call to ioctl() fails
//	 ... assuming BOTHER is defined. 



int readwithtimeout(int fd, void *buf, size_t count, int secs);
// readwithtimeout - read from a file descriptor with timeout
// description:
//  mimics read() but with a timeout that resets whenever a byte is received.
//  attempts to read up to count bytes from file descriptor fd into 
//  the buffer starting at buf. continues to read bytes until count bytes
//  have been read or no data has been received for secs seconds.
// return value:
//	on success, the number of bytes read is returned, and the file
//  position is advanced by this number. it is not an error if this
//  number is smaller than the number of bytes requested; this
//  may happen for example because fewer bytes are actually available
//  right now. on an error, -1 is returned and errno is set appropriately.



void read_sequence_timeout(int signalno);
//read_sequence_timeout()
// handles the SIGALRM signal


int read_sequence(int fd, void *buf, size_t count, char *seq, size_t seq_size, long secs, long usecs);
// read_sequence is used to wait for a specific byte/character, ignoring other sequences
// that arrive on the device. it stops when a timeout occurs or the buffer is filled.
// detailed behavior:
// waits for the first sequence starting with the characters in seq, or times
// out if secs seconds + msecs microseconds pass before seq is matched.
// returns 0 if timeout or sequence never matched, -1 on read failure,
// and otherwise returns the total number of bytes written into buf.
// all bytes up to the first byte of the start sequence are discarded/ignored
// from the device. the start sequence is copied into buf along with the
// following bytes up to count total bytes or the timeout occurs.
//  count must be >= seq_size so the sequence can fit in the buffer.



unsigned int convert_baudrate(speed_t baudrate);
// returns the speed_t baudrate defined in <termios.h> in unsigned integer format
// e.g. convert_baudrate(B57600) returns 57600

void fprinthex(FILE *stream, char *seq, unsigned int len);
// print hex sequence of len bytes from char array seq.
// converts the numerical value of each byte in seq to a 
// hex character string and prints it to stream.
// hexadecimal bytes are each seperated by spaces.
// there is no trailing space.


void char_to_hexstr(char val, char* buf, unsigned int len);
void int_to_hexstr(int val, char* buf, unsigned int len);


// serial communication helper function definitions
// ==========================================================================


// readwithtimeout - read from a file descriptor with timeout
// description:
//  mimics read() but with a timeout that resets whenever a byte is received.
//  attempts to read up to count bytes from file descriptor fd into 
//  the buffer starting at buf. continues to read bytes until count bytes
//  have been read or no data has been received for secs seconds.
// return value:
//	on success, the number of bytes read is returned, and the file
//  position is advanced by this number. if this number is smaller
//  than the number of bytes requested, no data was received
//  for a period of sec secs; the timeout occured. the data was
//  not available (e.g. the device sending the data was too slow).
//  on an error, -1 is returned and errno is set appropriately.
int readwithtimeout(int fd, void *buf, size_t count, int secs)
{
	fd_set readfs;
	struct timeval timeout;
	int res;
	unsigned int attempts = 0, bytesread = 0;
 
	// wait for input
	while (bytesread < count)
	{
		timeout.tv_usec = 0; timeout.tv_sec = secs;
		FD_SET(fd, &readfs);
		res = select(fd+1, &readfs, NULL, NULL, &timeout);

		if (res == 0) { // nothing to read after timeout
				return bytesread;
		}
		else
		{
			res = read(fd, buf, count-bytesread);
			if (res==-1)
				return -1;
			bytesread+= res;
			attempts++;
		}
	}
	return bytesread;	
}



//read_seq_timeout()
// handles the SIGALRM signal
void read_sequence_timeout(int signalno)
{
	sts_serial_read_seq_timeout = 1;
}

typedef void (*sighandler_t)(int);




// read_sequence is used to wait for a specific byte/character, ignoring other sequences
// that arrive on the device. it stops when a timeout occurs or the buffer is filled.
// detailed behavior:
// waits for the first sequence starting with the characters in seq, or times
// out if secs seconds + msecs microseconds pass before seq is matched.
// returns 0 if timeout or sequence never matched, -1 on read failure,
// and otherwise returns the total number of bytes written into buf.
// all bytes up to the first byte of the start sequence are discarded/ignored
// from the device. the start sequence is copied into buf along with the
// following bytes up to count total bytes or the timeout occurs.
//  count must be >= seq_size so the sequence can fit in the buffer.
// XXX this uses the ITIMER_REAL timer for the timeout. if this timer is
// being used elsewhere in the process, its timer will be delayed by however
// long this call to read_sequence takes, since the timer is saved and then
// restored to its original state.
int read_sequence(int fd, void *buf, size_t count, char *seq, size_t seq_size, long secs, long usecs)
{
	unsigned int seq_matched = 0, bytes_read = 0, i;
	int res, retval = 0;

	size_t seqbuf_size = count;
	char* seqbuf = malloc(seqbuf_size);

	struct itimerval timer_value, old_timer_value;
	sighandler_t old_sig_handler;

	sts_serial_read_seq_timeout = 0;

	// activate the timeout alarm -- a SIGALRM will be delievered in secs seconds + uscs microseconds.
	if (secs > 0 || usecs > 0)
	{
		old_sig_handler = signal(SIGALRM,read_sequence_timeout); // Register the timeout handler, save the old one
											   // XXX really should be using sigaction here..

		timer_value.it_value.tv_sec = secs;
		timer_value.it_value.tv_usec = usecs;
		timer_value.it_interval.tv_sec = 0;
		timer_value.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL,&timer_value,&old_timer_value);
	}
	while(sts_serial_read_seq_timeout == 0)
	{
		// if the sequence has been matched
		if (seq_matched == seq_size)
		{
			//printf("Sequence matched.\n");
			// if count bytes have been read, stop and return.
			if (bytes_read == count)
			{
				sts_serial_read_seq_timeout = 1;
				break;
			}
			else{
				//printf("Waiting for %d bytes.\n",count-bytes_read);
				res = read(fd,buf+bytes_read,count-bytes_read);
				if (res==0)
					continue;
				else if (res<0)
				{
					if (errno == EAGAIN)
						continue;
					printf(" read_sequence() call to read() failed: %s\n",strerror(errno));
					retval=-1;
				}
				else bytes_read+=res;
				continue;
			}
		}
		else // if the sequence hasn't been matched...
		{
			
			res = read(fd, seqbuf, seqbuf_size);
			if (res==0)
				continue;
			else if (res<0)
			{
				if (errno == EAGAIN)
					continue;
				printf(" read_sequence() call to read() failed: %s\n",strerror(errno));
				retval=-1;
			}
			//printf("Read %d bytes\n",res);
			// for each byte read
			else for (i=0; i<res;i++)
			{
				if (sts_serial_read_seq_timeout == 1)
					break;
				// if the byte matches the next byte of the sequence to match,
				// or the sequence has been matched and there are bytes left in the buffer
				else if (seqbuf[i]==seq[seq_matched] || (seq_matched == seq_size))
				{
					// copy the byte into the buffer
					((char *)buf)[bytes_read] = ((char*)seqbuf)[i];
					bytes_read++; // increment the number of bytes written to buf
					if (seq_matched<seq_size)
					{
						seq_matched++; // increment the number of seq bytes matched
					}
				}
				else // otherwise the sequence didn't match...
				{
					// .. reset the counts
					seq_matched = 0;
					bytes_read = 0;
				}
			}
		} // if the sequence hasn't been matched...
	}
	// disable the timeout alarm
	if (secs > 0 || usecs > 0)
	{
		signal(SIGALRM,old_sig_handler); // register the old timeout handler
								// XXX really should be using sigaction here
		setitimer(ITIMER_REAL,&old_timer_value,0); // restore the old timer
	}
	free(seqbuf);

	if (retval == -1)
		return retval;
	else return bytes_read;
}


// Attempts to set the baud rate to the closest rate possible to 
// the desired_baudrate argument using divisors.
// fport is the file descriptor for the port opened by a call to serial_connect() or open()
// Divisor method:
//   it should be possible to do custom baud rates by using a divisor, like
//   you would do when you call "setserial /dev/ttyS0 baud_base 115200 divisor 14 spd_cust"
//   If the call to setserial wouldn't work for the device, the divisor method wont work here either.
// 	 This is usually due to an unimplemented ioctl function in the device driver.
// Termios custom baud rate method:
//   I think tty_ioctl.c has to be compiled into the kernel with BOTHER defined for this to work.
//   set cbaud to BOTHER and c_ospeed to the desired setting.
//   This is done through a call to set_custom_baud_rate_no_ioctl()
int set_custom_baud_rate(int fport, unsigned int desired_baudrate)
{
	unsigned int new_baudrate;
	struct termios port_attrib;
	struct serial_struct serial_info;
	int divisor = 1;
	

	if (tcgetattr(fport, &port_attrib) < 0)
	{
		printf(" tcgetattr() failed to get port settings.\n");
		return -1;
	}
	if (ioctl(fport, TIOCGSERIAL, &serial_info) !=0)
	{
		printf(" ioctl TIOCGSERIAL failed to get port settings: %s.\n",strerror(errno));
		
		return set_custom_baud_rate_no_ioctl(fport, desired_baudrate);
	}


	// set the baudrate to B38400 (custom baud setting)
	
	if (cfsetspeed(&port_attrib,B38400) < 0)
	{	printf(" Call to cfsetspeed failed. Unable to set baud rate.\n");
		return -1;
	}

	
	// clear the serial line
	tcflush(fport, TCIOFLUSH);


	// XXX should make this round to nearest integer instead of
	// just using integer division with drops the fractional component

	// set the base baud rate if it is less than 115200, to 115200
	if (serial_info.baud_base < 115200)
		serial_info.baud_base = 115200;

	divisor = serial_info.baud_base / desired_baudrate;

	// set the custom divisor
	serial_info.custom_divisor = divisor;
	// set the ASYNC_SPD_CUST flag
	serial_info.flags |= (ASYNC_SPD_MASK & ASYNC_SPD_CUST);
	// apply the port settings (divisor and baud base)
	if (ioctl(fport,TIOCSSERIAL,&serial_info) !=0)
	{
		printf(" ioctl() TIOCSSERIAL failed to set custom baud rate: %s.\n",strerror(errno));
		return set_custom_baud_rate_no_ioctl(fport, desired_baudrate);
	}
	// apply the port settings (baud rate)
	if (tcsetattr(fport,TCSANOW,&port_attrib) < 0)
	{
		printf(" Failed to apply new port settings.\n");
		return -1;
	}	

	// try to get the new termios port settings
	if (tcgetattr(fport, &port_attrib) < 0)
	{
		printf(" Failed to get new port settings.\n");
		return -1;
	}
	// check the new baud rate
	new_baudrate = cfgetospeed(&port_attrib);
	if ((new_baudrate != B38400) && (new_baudrate != CBAUDEX))
	{
		printf(" Custom baud rate could not be set with tcsetattr.\n");
		return -1;
	}
	
	// try to get the new ioctl port settings
	if (ioctl(fport, TIOCGSERIAL, &serial_info) !=0)
	{
		printf(" ioctl TIOCGSERIAL failed to get new port settings.\n");
		return set_custom_baud_rate_no_ioctl(fport, desired_baudrate);
	}
	// check the new baud rate and divisor
	if (serial_info.custom_divisor!= divisor)
	{
		printf(" Custom baud rate could not be set by ioctl.\n");
		return set_custom_baud_rate_no_ioctl(fport, desired_baudrate);
	}
	new_baudrate = serial_info.baud_base/serial_info.custom_divisor;

	printf(" Baud rate set to: %d. (%d was requested)\n",new_baudrate, desired_baudrate);
	if (desired_baudrate != new_baudrate)
		printf("  Exact baud rate could not be set due to hardware limitations.\n");

	// clear the serial line
	tcflush(fport, TCIOFLUSH);

	return 0;
}



// Termios custom baud rate method:
//   tty_ioctl.c has to be compiled into the kernel with BOTHER defined for this to work.
//   This is called through set_custom_baud_rate() if a call to ioctl() fails
//	 ... assuming BOTHER is defined. 
// XXX this is an untested function (my architecture doesn't have BOTHER defined)
int set_custom_baud_rate_no_ioctl(int fport, unsigned int desired_baudrate)
{
#ifndef BOTHER
	return -1;
#endif
#ifdef BOTHER
	unsigned int new_baudrate;
	struct ktermios port_attrib;
	int divisor = 1;
	

	if (tcgetattr(fport, &port_attrib) < 0)
	{
		printf(" tcgetattr() failed to get port settings.\n");
		return -1;
	}

	// set the baudrate to BOTHER (custom baud setting)
	
	port_attrib.c_cflag &= ~CBAUD; // clear the baud setting
	port_attrib.c_cflag |= CBAUDEX; // use custom baud
	port_attrib.c_ospeed = desired_baudrate; // custom baud rate

	// clear the serial line
	tcflush(fport, TCIOFLUSH);

	// apply the port settings (baud rate)
	if (tcsetattr(fport,TCSANOW,&port_attrib) < 0)
	{
		printf(" Failed to apply new port settings.\n");
		return -1;
	}	

	// try to get the new termios port settings
	if (tcgetattr(fport, &port_attrib) < 0)
	{
		printf(" Failed to get new port settings.\n");
		return -1;
	}
	// check the new baud rate
	new_baudrate = cfgetospeed(&port_attrib);
	if (new_baudrate != desired_baudrate)
	{
		printf(" Custom baud rate could not be set with tcsetattr.\n");
		return -1;
	}
	
	printf(" Baud rate set to: %d. (%d was requested)\n",new_baudrate, desired_baudrate);
	if (desired_baudrate != new_baudrate)
		printf("  Exact baud rate could not be set due to hardware limitations.\n");
	return 0;
#endif
}



// attempts to connect to a serial device at portpath (i.e. "/dev/ttyUSB0")
// sets raw mode and sets the baud rate to baudrate. (see `man termios`)
// returns the file descriptor for the connection, or -1 if an error occurs.

// note that this only supports baud rates defined in termios.
int serial_connect(const char* portpath, int flags, speed_t baudrate)
{
	speed_t new_baudrate;
	struct termios port_attrib; // termios attributes struct
	int fport = open(portpath,flags);
	if (fport == -1)
	{
		printf("Unable to open %s.\n", portpath);
		printf("Check that the device is plugged in and turned on.\n");
		return -1;
	}

	// get the current port settings
	if (tcgetattr(fport, &port_attrib) < 0)
	{
		printf(" Failed to get port settings.\n");
		close(fport);
		return -1;
	}

	// set raw mode
    cfmakeraw(&port_attrib);

	// set the baud rate to baudrate
	if (cfsetspeed(&port_attrib,baudrate) < 0)
	{	printf(" Invalid baud rate specified or other baud rate error.\n");
		close(fport);
		return -1;}

	// clear the serial line
	tcflush(fport, TCIOFLUSH);
	// apply the port settings (baud rate and raw mode)
	if (tcsetattr(fport,TCSANOW,&port_attrib) < 0)
	{
		printf(" Failed to apply new port settings.\n");
		close(fport);
		return -1;
	}	

	// try to get the new port settings
	if (tcgetattr(fport, &port_attrib) < 0)
	{
		printf(" Failed to get port settings.\n");
		close(fport);
		return -1;
	}

	// print the new baud rate
	new_baudrate = cfgetospeed(&port_attrib);
	if (new_baudrate == baudrate)
		printf(" Port opened at baud rate: %d.\n",convert_baudrate(baudrate));
	else
	{
		printf("Couldn't set baud rate.\n");
		close(fport);
		return -1;
	}

	return fport;
}

// returns the speed_t baudrate defined in <termios.h> in unsigned integer format
// e.g. convert_baudrate(B57600) returns 57600. on unrecognized baudrate, returns 0.
unsigned int convert_baudrate(speed_t baudrate)
{
	unsigned int res;
	switch (baudrate)
	{
		case B0 :  res = 0; break;
		case B50 :  res = 50; break;
		case B75 :  res = 75; break;
		case B110 :  res = 110; break;
		case B134 :  res = 134; break;
		case B150 :  res = 150; break;
		case B200 :  res = 200; break;
		case B300 :  res = 300; break;
		case B600 :  res = 600; break;
		case B1200 :  res = 1200; break;
		case B1800 :  res = 1800; break;
		case B2400 :  res = 2400; break;
		case B4800 :  res = 4800; break;
		case B9600 :  res = 9600; break;
		case B19200 :  res = 19200; break;
		case B38400 :  res = 38400; break;
		case B57600 :  res = 57600; break;
		case B115200 :  res = 115200; break;
		case B230400 :  res = 230400; break;
		case B460800 :  res = 460800; break;
		default: res = 0;
	}
	return res;
}


// print hex sequence of len bytes from char array seq.
// converts the numerical value of each byte in seq to a 
// hex character string and prints it to stream.
// hexadecimal bytes are each seperated by spaces.
// there is no trailing space.
void fprinthex(FILE *stream, char *seq, unsigned int seq_len)
{
	unsigned int i;
	char hexbuf[3];
	for (i=0; i<seq_len; i++)
	{
		char_to_hexstr(seq[i],hexbuf,3);
		fprintf(stream,"%s",hexbuf);
		if (i<(seq_len-1))
			fprintf(stream," ");
	}
}

// convert an integer or char value into a hex string
// and places it in buf. the value is treated as unsigned.
// buf is a char array of length len bytes, val is the
// integer or char value to be converted.
// character conversion: buf must be 3 bytes long
// one for each hex digit and one for the null terminator.
// if len is too small, this function will print an
// error message and return.
// integer conversion: buf must be 9 bytes long

void char_to_hexstr(char val, char* buf, unsigned int len)
{
	unsigned char value = (unsigned) val;
	char hex_char[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

	if (len < 3)
	{
		fprintf(stderr,"Buffer not large enough for hex conversion.\n");
		return;
	}
	buf[1] = hex_char[value%16];
	buf[0] = hex_char[(value>>4)%16];
	buf[2] = '\0';
	
}

void int_to_hexstr(int val, char* buf, unsigned int len)
{
	unsigned int value = (unsigned) val;
	char hex_char[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	int i=0;

	if (len < 9)
	{
		fprintf(stderr,"Buffer not large enough for hex conversion.\n");
		return;
	}

	for (i=0; i<8; i++)
		buf[7-i] = hex_char[(value>>(4*i))%16];
	buf[i] = '\0';
}
