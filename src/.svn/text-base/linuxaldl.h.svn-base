#ifndef LINUXALDL_INCLUDED
#define LINUXALDL_INCLUDED



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

// debug mode
//#define _LINUXALDL_DEBUG

#define MAX_CONNECT_ATTEMPTS 3
#define BAUDRATE B9600

// macros

#define _ALDL_MESSAGE_MODE8 aldl_settings.definition->mode8_request,aldl_settings.definition->mode8_request_length
#define _ALDL_MESSAGE_MODE9 aldl_settings.definition->mode9_request,aldl_settings.definition->mode9_request_length

#define __MAX_REQUEST_SIZE 16 // maximum size (bytes) of a request message
							  // to send to the ECM

#define LINUXALDL_MODE1_END_DEF {NULL,0,0,0,0,0,NULL}


typedef enum _ALDL_OP { ALDL_OP_MULTIPLY=0, ALDL_OP_DIVIDE=1, ALDL_OP_SEPERATOR=9} ALDL_OP_t;

#define _DEF_SEP(label) {label,0,0,ALDL_OP_SEPERATOR,0,0,NULL}


// ============================================================================
// ALDL DEFINITION STRUCTS
// ============================================================================

// See linuxaldl_definitions.h for instructions on how to make a new definition


// byte_def_t struct
typedef struct _linuxaldl_byte_definition{
	const char* label;
	unsigned int byte_offset; // the offset from the 1st byte of the data part
							  // of the mode1 message
	unsigned int bits; // 8 or 16 are currently supported

	unsigned int operation; // ALDL_OP_MULTIPLY: (X*factor)+offest
							// ALDL_OP_DIVIDE: (factor/X)+offset
							// ALDL_OP_SEPERATOR: use this for a seperator for the display,
							//    not a data item.  with this option no other
							//	  values matter except label.
							//	  you can also use the _DEF_SEP(label) macro like:
							//	  _DEF_SEP("---Basic Data---")

	float op_factor; // factor for the operation
	float op_offset; // offset for the operation


	const char* units;
} byte_def_t;

typedef struct _linuxaldl_definition{
	const char* name;
	char mode1_request[__MAX_REQUEST_SIZE];  // the mode 1 request message, including the checksum
	unsigned int mode1_request_length;  // the length of the mode 1 message including the checksum

	unsigned int mode1_response_length; // the total length of the response from the ecm

	unsigned int mode1_data_length; // the number of data bytes in the mode1 message response

	unsigned int mode1_data_offset; // the byte offest from the start of the mode1 message response
									// to the first byte of the data. e.g. if the data part of the
									// message is the 4th byte onward, this should be 3. (1+3 = 4)

	byte_def_t* mode1_def; // pointer to start of table of byte_def_t structs.
							// the last element must be LINUXALDL_MODE1_END_DEF

	char mode8_request[__MAX_REQUEST_SIZE];  // the mode 8 (silence) request message, incl checksum
	unsigned int mode8_request_length;  // the length of the mode 8 message incl checksum

	char mode9_request[__MAX_REQUEST_SIZE];  // the mode 9 (un-silence) request message, incl checksum
	unsigned int mode9_request_length;  // the length of the mode 9 message including the checksum


} aldl_definition;



// looks up def_name in the aldl_definition_table until it finds the first 
// definition in the table with the name def_name
// if the definition is not in the table, returns NULL
aldl_definition* aldl_get_definition(const char* defname);

typedef struct _linuxaldl_settings
{
	const char* aldlportname; // path to aldl interface port
	int faldl; 			// aldl serial interface file descriptor

	const char* logfilename; 	// filename for the log file
	int flogfile; 		// file descriptor for log file

	int scanning; // 1 when the timer has been set for making scans
					// 0 when not scanning

	const char* aldldefname; // name for the ALDL definition to be used
	aldl_definition* definition; // see linuxaldl_definitions.h


	aldl_definition** aldl_definition_table; // array of pointers to data definitions.
											// this table is allocated in linuxaldl_definitions.h
											// and its value assigned upon the initialization of
											// the global "aldl_settings" variable

	char* data_set_raw;			// the current/most recent set of data from a mode1 message.
								// this is allocated when a definition is selected

	char** data_set_strings;	// pointer to array of data set in string format.
								// allocated when a definition is selected in the GUI

	float* data_set_floats;		// data set in float format
								// allocated when a definition is selected in the GUI

	unsigned int scan_interval; // msec between scan requests
	unsigned int scan_timeout; // msec to timeout on scan request.
						// note that read-sequence takes timeout in usec.
						// usec = msec*1000
} linuxaldl_settings;

// function prototypes
// =================================================

int verifyaldl();
// wake up / verify the ALDL interface

int aldl_scan_and_log(int fd);
// listens for aldl data and writes it to the file descriptor fd 
// stops when terminate_scan = 1 in the global settings struct and
// returns number of bytes written to fd.

char get_checksum(char* buffer, unsigned int len);
// calculates the single-byte checksum, summing from the start of buffer
// through len bytes. the checksum is calculated by summing the bytes,
// dropping carried bits, then adding 1 and taking the two's complement
// (subtract from FF) 

int send_aldl_message(char* msg_buf, unsigned int size);
// sends an artibtrary aldl message contained in the buffer msg_buf.
// the checksum must be set in the buffer by the caller.
// the following macros can be used as arguments:
//		_ALDL_MESSAGE_MODE8
//		_ALDL_MESSAGE_MODE9
// which use the mode 8 and mode 9 message definitions from the
// current aldl definition.

int get_mode1_message(char* inbuffer, unsigned int size);
// requests a mode1 message from the ECM using the currently loaded
// aldl definition. 
// returns 0 if the message was received successfully, -1 no response
// or bad checksum. 

int aldl_listen_raw(char* inbuffer, unsigned int len, int timeout);
// reads up to len bytes into inbuffer from the interface.
// listens for a maximum of timeout seconds.
// returns -1 on failure, 0 on timeout with no bytes received,
// and otherwise returns the number of bytes received 

typedef enum _ALDL_UPDATE_FLAGS { ALDL_UPDATE_STRINGS=1, ALDL_UPDATE_FLOATS=2} ALDL_UPDATE_FLAGS_t;
void aldl_update_sets(int flags);
// updates data_set_floats and/or data_set_strings using the current data_set_raw bytes.
// if the flags argument is ALDL_UPDATE_STRINGS then both sets will be updated.
// if it is ALDL_UPDATE_FLOATS then only floats will be updated, and the data_set_strings
// array will not be modified in any way.

float aldl_raw8_to_float(unsigned char val, int operation, float op_factor, float op_offset);
// converts the raw 8-bit data value val into a float by performing operation
// using op_factor and op_offset.
// see the documentation for the byte_def_t struct for more information


float aldl_raw16_to_float(unsigned char msb, unsigned char lsb, int operation, float op_factor, float op_offset);
// converts the raw 16-bit data value defined by lsb and msb into a float by performing operation
// using op_factor and op_offset.
// see the documentation for the byte_def_t struct for more information

#endif
