#ifndef STS_SERIAL_INCLUDED
#define STS_SERIAL_INCLUDED

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
#include <fcntl.h>

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

void fprinthex(FILE *stream, char *seq, unsigned int seq_len);
// print hex sequence of len bytes from char array seq.
// converts the numerical value of each byte in seq to a 
// hex character string and prints it to stream.
// hexadecimal bytes are each seperated by spaces.
// there is no trailing space.


void char_to_hexstr(char val, char* buf, unsigned int len);
void int_to_hexstr(int val, char* buf, unsigned int len);


#endif
