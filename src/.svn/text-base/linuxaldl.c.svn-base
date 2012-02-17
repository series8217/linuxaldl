

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


#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <popt.h> // for command line parsing
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h> // for memcpy
#include <errno.h>
#include "linuxaldl.h"
#include "linuxaldl_definitions.h"
#include "linuxaldl_gui.h"
#include "sts_serial.h"


// global variables
// =================================================

linuxaldl_settings aldl_settings = { NULL, 0, NULL, 1, 0, NULL, NULL, aldl_definition_table, NULL, NULL, NULL, 150, 100};

// ============================================================================
//
//					linuxaldl
//  					 main ()														 
//
// ============================================================================

int main(int argc, const char* argv[]){
	int res; // temporary storage for function results

	int guimode = 0;

	// ========================================================================
	// 			COMMAND LINE OPTION PARSING 
	// ========================================================================

	poptContext  popt_aldl;

	// popt option table
	// -----------------
	struct poptOption aldl_opt_table[] =
			{
				{ "serial",'\0',
				POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,&aldl_settings.aldlportname,0,
				"Serial port the aldl interface is on",
				"/dev/ttyUSB0" },
				{ "mask",'\0',
				POPT_ARG_STRING | POPT_ARGFLAG_ONEDASH,&aldl_settings.aldldefname,0,
				"ALDL code definition to use",
				"DF"},
				POPT_AUTOHELP
				{ NULL, 0, 0, NULL, 0, 0, NULL}
			};

	// popt context
	popt_aldl = poptGetContext(NULL, argc, argv,aldl_opt_table,0);
	poptSetOtherOptionHelp(popt_aldl,"[logfile.log]\nTo use GUI: linuxaldl [-serial=/dev/ttyUSB0]");

	if (argc<2) { poptPrintUsage(popt_aldl,stderr,0); return 1; }

	res = poptGetNextOpt(popt_aldl); // parse the command line arguments

	// if no serial port selected, print usage instructions and exit
	if (aldl_settings.aldlportname == NULL)
	{ poptPrintUsage(popt_aldl,stderr,0); return 1; }

	aldl_settings.logfilename = (char*)poptGetArg(popt_aldl);
	// if no log file specified or no definition file specified
	if ((aldl_settings.logfilename == NULL || !(poptPeekArg(popt_aldl)==NULL || aldl_settings.aldldefname==NULL)))
	{
		guimode = 1;
	}
	else // in command line mode, choose definition using the -mask=DEF argument
	{
		// XXX suggestion: how about adding support for pre-selecting the definition at the command line?
		// get the aldl definition address
		aldl_settings.definition = aldl_get_definition(aldl_settings.aldldefname);

		// if no definition by that name exists, exit
		if (aldl_settings.definition == NULL)
		{
			fprintf(stderr,"Error: No definition with name \"%s\" found.",
													aldl_settings.aldldefname);
			fprintf(stderr,"Consult the documentation.\n");							
			fprintf(stderr," Note: definition names are case sensitive.\n");
			return -1;	
		}
	}
	poptFreeContext(popt_aldl); // free the popt context

	// ========================================================================
	// 							CONNECT / VERIFY ALDL INTERFACE 
	// ========================================================================

	// Establish a connection to the aldl
	// ===================================
	printf("Trying to connect to ALDL interface on %s...\n",aldl_settings.aldlportname);

	// connect to the device and set the serial port settings
	aldl_settings.faldl = serial_connect(aldl_settings.aldlportname,O_RDWR | O_NOCTTY | O_NONBLOCK,BAUDRATE);
	if (aldl_settings.faldl == -1)
	{
		fprintf(stderr," Couldn't open to %s\n",aldl_settings.aldlportname);
		return -1;
	}

	// set the custom baud rate:
	// the ALDL interface operates at 8192. it would be preferable to get as close to this
	// baud rate as possible. if it cannot be set; it should still work at the standard
	// baud rate of 9600; the framing errors aren't excessive enough to cause problems,
	// and will be caught by bad checksums.
	if (set_custom_baud_rate(aldl_settings.faldl,8192)!=0)
	{
		fprintf(stderr," Couldn't set baud rate to 8192. Using standard rate (9600).\n");
		fprintf(stderr," There may be framing errors.\n");
	}
	
	// verify the aldl
	if (verifyaldl()<0)
	{
		fprintf(stderr," ALDL verification failure. No response from ECM.\n");
		tcflush(aldl_settings.faldl, TCIOFLUSH);
		close(aldl_settings.faldl);
		return -1;
	}

	// GUI mode if no .log file specified at the command line 
	// ========================================================================
	// 							LOAD GUI MODE 
	// ========================================================================
	if (guimode)
	{
		// GUI mode
		linuxaldl_gui(argc, (char**)argv);
	}

	// ========================================================================
	// 						COMMAND LINE MODE
	// ========================================================================
	else
	{
		// Open the .log file for writing, create if it doesnt exist
		// ------------------------------------------------------------
		aldl_settings.flogfile = open(aldl_settings.logfilename,O_RDWR | O_CREAT, 0666);
		if (aldl_settings.flogfile == -1)
		{
			fprintf(stderr,"Unable to open/create %s for writing.\n",aldl_settings.logfilename);
			return 0;	}
		printf("Opened %s for writing.\n",aldl_settings.logfilename);
		// Read from the aldl
		// ------------------
		res = aldl_scan_and_log(aldl_settings.flogfile);
			if (res==-1) fprintf(stderr,"Error: Fatal read error occured. log file may be corrupted.\n");
		else printf("Received %d bytes from device and wrote to file: %s.\n", res,aldl_settings.logfilename);
	
		close(aldl_settings.flogfile);
	}


	// ========================================================================
	// 				CLEANUP (FLUSH SERIAL LINE, CLOSE PORT) 
	// ========================================================================

	// discard any unwritten data
	tcflush(aldl_settings.faldl, TCIOFLUSH);

	// close the port
	close(aldl_settings.faldl);
	printf("Connection closed.\n"); 
	return 0;
}



// ============================================================
//
// 			linuxaldl general function definitions 
//
// ============================================================
// (mostly used for the command line interface but also main())

// wake up / verify the aldl
// This function should check that the aldl interface is working by listening
// for ECM chatter, or some other operation to determine whether there is
// a line connected to the serial port.
int verifyaldl()
{
	//XXX NOT IMPLEMENTED
	return 0;
	
}


int aldl_scan_and_log(int fd)
{
	//XXX NOT IMPLEMENTED
	return 0;
}

// sends an artibtrary aldl message contained in the buffer msg_buf.
// the checksum must be set in the buffer by the caller.
// the following macros can be used as arguments:
//		_ALDL_MESSAGE_MODE8
//		_ALDL_MESSAGE_MODE9
// which use the mode 8 and mode 9 message definitions from the
// current aldl definition.
// returns 0 on success.
int send_aldl_message(char* msg_buf, unsigned int size)
{
	int res;

#ifdef _LINUXALDL_DEBUG
	printf("Sending sequence: ");
	fprinthex(stdout,msg_buf,size);
	printf("\n");
#endif

	res = write(aldl_settings.faldl,&msg_buf,size);
	if (res <= 0)
		return -1;
	else if ((unsigned)res<size)
		return -1;
	tcdrain(aldl_settings.faldl);

	return res;
}

// requests a mode1 message from the ECM using the currently loaded
// aldl definition. 
// returns the number of bytes received, if the message was complete.
//  0 is no response/timeout/partial message
// -1 is returned if the checksum is bad.
// XXX this is likely to fail if mode 8 is not set, because the call to read
// will in all probability return part of a normal mode message. a correct
// implementation should wait for a mode 1 message header in the response,
// so that mode 8 isn't even required.
int get_mode1_message(char* inbuffer, unsigned int size)
{
	int res;
	char checkval;
	char outbuffer[__MAX_REQUEST_SIZE]; // max request size defined in linuxaldl_definitions.h

	aldl_definition* def = aldl_settings.definition;
	unsigned int mode1_len = def->mode1_response_length;

	if (size < mode1_len)
	{
		printf("Read buffer must be at least %d bytes.\n",mode1_len);
		return -1;
	}

	// put the mode 1 request message and checksum in the output buffer
	memcpy(outbuffer,def->mode1_request,def->mode1_request_length-1);
	outbuffer[def->mode1_request_length-1] = get_checksum(outbuffer,def->mode1_request_length-1);

	// form the response message start sequence
	char seq[] = { def->mode1_request[0], 0x52+def->mode1_response_length, 0x01};

	// flush the serial receive buffer
	tcflush(aldl_settings.faldl,TCIFLUSH);

	// write the request to the serial interface
	write(aldl_settings.faldl,&outbuffer,def->mode1_request_length); 

	// wait for the bytes to be written
	tcdrain(aldl_settings.faldl); 

	// wait for response from ECM
	// read sequence, 50msec timeout
	res=read_sequence(aldl_settings.faldl, inbuffer, mode1_len,
											seq, 3, 0, 
							aldl_settings.scan_timeout*1000);

	if (res<0)
	{
		fprintf(stderr,"Error receiving mode1 message: %s\n",strerror(errno));
		return -1;
	}
	if ((unsigned)res<mode1_len)
	{
		
#ifdef _LINUXALDL_DEBUG
		fprintf(stderr,"MODE1 timeout occured. (Received %d/%d bytes)\n",res,mode1_len);
#endif
		return 0;
	}

	char checksum = get_checksum(inbuffer,mode1_len-1);
	if (inbuffer[mode1_len-1]!=checksum)
	{
		fprintf(stderr,"MODE 1 bad checksum.\n");
		return -1;
	}

	return res;
}

// reads up to len bytes into inbuffer from the interface.
// listens for a maximum of timeout seconds.
// returns -1 on failure, 0 on timeout with no bytes received,
// and otherwise returns the number of bytes received 
int aldl_listen_raw(char* inbuffer, unsigned int len, int timeout)
{
	int res;
	res = readwithtimeout(aldl_settings.faldl,inbuffer,len,timeout);
	return res;
}

// calculates the single-byte checksum, summing from the start of buffer
// through len bytes. the checksum is calculated by adding each byte
// together and ignoring overflow, then taking the two's complement and adding 1
char get_checksum(char* buffer, unsigned int len)
{
	char acc = 0x00;

	unsigned int i;
	for (i=0; i<len; i++)
	{
		//printf("%d,",buffer[i]);
		//if (!(i%16)) printf("\n");
		acc+=buffer[i];
	}
	
	acc=0xFF-acc;
	acc+=0x01;
	//printf("Checksum: %d\n",acc);

	return acc;
}

// looks up def_name in the aldl_definition_table until it finds the first 
// definition in the table with the name def_name
// if the definition is not in the table, returns NULL
aldl_definition* aldl_get_definition(const char* defname)
{
	int index = 0;
	aldl_definition* result = aldl_definition_table[0];
	if (defname == NULL)
		return NULL;
	while(result!=NULL){
		if (strcmp(defname,result->name)==0)
			break;
		index++;
		result = aldl_definition_table[index];
	}
	return result;
}


// updates data_set_floats and/or data_set_strings using the current data_set_raw bytes.
// if the flags argument is ALDL_UPDATE_STRINGS then only sets will be updated.
// if flags is ALDL_UPDATE_FLOATS then only floats will be updated, and the data_set_strings
// array will not be modified in any way.
// if flags is ALDL_UPDATE_FLOATS|ALDL_UPDATE_STRINGS, then both will be updated.
// if the aldl_settings.data_set_floats or aldl_settings.data_set_strings arrays have
// not yet been initialized (e.g. are NULL pointers) then they will not be modified.
void aldl_update_sets(int flags)
{
	unsigned int i=0;
	byte_def_t* defs = aldl_settings.definition->mode1_def;
	byte_def_t* cur_def;
	float converted_val;
	char* new_data_string=NULL;

	while (defs[i].label != NULL) // while not at the last defined byte
	{
		cur_def = defs+i;
		// if the item is a seperator, skip it
		if (cur_def->operation == ALDL_OP_SEPERATOR)
		{
			i++;
			continue;
		}

		// convert the raw data to a float based on the byte definition
		if (cur_def->bits==8)
		{
			converted_val = aldl_raw8_to_float(aldl_settings.data_set_raw[cur_def->byte_offset-1],
											cur_def->operation, cur_def->op_factor, cur_def->op_offset);
											
		}
		else if (cur_def->bits==16)
		{
			converted_val = aldl_raw16_to_float(aldl_settings.data_set_raw[cur_def->byte_offset-1],
											aldl_settings.data_set_raw[cur_def->byte_offset],
											cur_def->operation, cur_def->op_factor, cur_def->op_offset);
		}
		else
		{ // other numbers of bits not supported
			i++;
			continue;
		}

		if ((flags & ALDL_UPDATE_FLOATS) && aldl_settings.data_set_floats != NULL)
			aldl_settings.data_set_floats[i] = converted_val;

		// convert the result to a string
		if ((flags & ALDL_UPDATE_STRINGS) && aldl_settings.data_set_strings != NULL)
		{
			new_data_string=malloc(10); // allocate ten bytes for the string

			snprintf(new_data_string,10,"%.1f",converted_val); // convert the floating point value to a string

			// if there is currently a string registered, free it
			if (aldl_settings.data_set_strings[i] != NULL)
				free(aldl_settings.data_set_strings[i]);

			// register the new string in the table
			aldl_settings.data_set_strings[i] = new_data_string;
		}

		//fprintf(stderr,"Updating element %s\n",cur_def->label);
		i++;
	}
}

// converts the raw 8-bit data value val into a float by performing operation
// using op_factor and op_offset.
// see the documentation for the byte_def_t struct in linuxaldl.h for more information
float aldl_raw8_to_float(unsigned char val, int operation, float op_factor, float op_offset)
{
	float result = val;
	if (operation == ALDL_OP_MULTIPLY)
		result = ((float)val*op_factor)+op_offset;
	else if (operation == ALDL_OP_DIVIDE)
		result = (op_factor/val)+op_offset;
	else
	{
		result = -999;
		fprintf(stderr," aldl_raw_to_float8() error: undefined operation: %d",operation);
	}

	return result;

}


// converts the raw 16-bit data value val into a float by performing operation
// using op_factor and op_offset.
// see the documentation for the byte_def_t struct in linuxaldl.h for more information
float aldl_raw16_to_float(unsigned char msb, unsigned char lsb, int operation, float op_factor, float op_offset)
{
	float result = ((float)msb*256)+(float)lsb;
	if (operation == ALDL_OP_MULTIPLY)
		result = (result*op_factor)+op_offset;
	else if (operation == ALDL_OP_DIVIDE)
		result = (op_factor/result)+op_offset;
	else
	{
		result = -999.0;
		fprintf(stderr," aldl_raw_to_float16() error: undefined operation: %d",operation);
	}
	return result;

}


