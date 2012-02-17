#ifndef LINUXALDL_DEFINITIONS_INCLUDED
#define LINUXALDL_DEFINITIONS_INCLUDED

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

#include <string.h>
#include "linuxaldl.h"
// ===================================================================
//		WRITING A DEFINITION FOR LINUXALDL
// ===================================================================

// XXX IMPORTANT NOTE: The definition format is likely to be extremely
// volatile up until version 1.0. I don't recommend spending a lot of
// time writing a definition. It probably wont work in the next version.

// Labels/names/units must not contain white space, commas, newlines, 
// or double quotes.
// They may contain: a-z A-Z 0-9 ~!@#$%^&*()-=_+|\/}{:<>?.[];'
// Label/names/units may not be NULL, except for units for seperators,
// and in the last element of the mode1_def[] array. (see below)

// The last element of the mode1_def[] array must be LINUXALDL_MODE1_END_DEF
// (which is a byte_def_t with label and units NULL and all other values 0).

// ===================================================================

// see the DF definition below for a complete example of a definition


// ==========================================
//
//   DF CODE MASK DEFINITION
// Engine: 1991-1993 3.4 DOHC V6 (LQ1) Vin "X"
//
// aldl_definition_table entry: aldl_DF
// mode1 definition table: aldl_DF_mode1
// ===========================================
byte_def_t aldl_DF_mode1[]=
	{
			_DEF_SEP("---Basic Data---"),
			{"Engine RPM",			11,	8,	0,	25.0,		0.0,	"RPM"},
			{"Throttle Position", 	10, 8, 	0, 	0.003906, 	0.00, 	"%"},
			{"Vehicle Speed", 		17, 8, 	0, 	1.0, 		0.0, 	"MPH"},
			{"Engine Airflow", 		37, 8, 	0, 	1.0, 		0.0, 	"gm/sec"},
			{"Coolant Temp", 		7, 	8, 	0, 	1.35, 		-40.0, 	"Deg F"},
			{"Intake Air Temp",		30, 8,	0,	1.0,		0.0,	"adc"},
			{"MAP",					29,	8,	0,	0.369,		10.354,	"kPa"},
			_DEF_SEP("----Fuel----"),
			{"Desired AFR",			41,	8,	0,	0.100,		0.0,	"A/F"},
			{"Narrowband O2",		19,	8,	0,	4.42,		0.0,	"mV"},
			{"Final Base Pulse Width", 42, 16, 0, 0.015259, 0.0,	"mSec"},
			{"Current BLM Cell",	23,	8,	0,	1.0,		0.0,	""},
			{"BLM",					22,	8,	0,	1.0,		0.0,	"counts"},
			{"Integrator",			24,	8,	0,	1.0,		0.0,	"counts"},
			{"Base Pulse Fine Corr.",21,8,	0,	1.0,		0.0,	"counts"},
			{"BLM Cell 0 Timer",	36,	8,	0,	1.0,		0.0,	"counts"},
			_DEF_SEP("--Ignition--"),
			{"Knock Events",		51,	8,	0,	1.0,		0.0,	"counts"},
			{"Spark Advance",		40, 8,	0,	0.351560,	0.0,	"degrees"},
			{"Knock Retard",		46,	8,	0,	0.175781,	0.0,	"degrees"},
			_DEF_SEP("--Accessory Data--"),
			{"PROM ID",				1,	16,	0,	1.0,		0.0,	"ID"},
			{"TPS Voltage",			9,	8,	0,	0.019531,	0.0,	"volts"},
			{"IAC Steps",			25, 8,  0,  1.0,		0.0,	"steps"},
			{"IAC Min Position",	22, 8,  0,	1.0,		0.0,	"steps"},
			{"Barometric Pressure",	28,	8,	0,	0.369,		10.3542,"kPa"},
			{"Engine Run Time",		48,	16,	0,	1.0,		0.0,	"secs"},
			{"Catalytic Conv Temp",	50,	8,	0,	3.0,		300.0,	"Deg C"},
			{"Fuel Pump Relay Volts",31,8,	0,	0.1,		0.0,	"volts"},
			{"O2 Cross-Count",		20,	8,	0,	1.0,		0.0,	"counts"},
			{"Desired Idle Speed",	27, 8, 	0,	12.5,		0.0,	"RPM"},
			{"Battery Voltage",		34,	8,	0,	0.1,		0.0,	"volts"},
			{"CCP Duty Cycle",		45,	8,	0,	0.390650,	0.0,	"% CCP"},
			{"RPM/MPH",				47, 8,	0,	1.0,		0.0,	"RPM/MPH"},
			{"A/C Pressure Sensor",	33,	8,	0,	1.0,		0.0,	"A/D Counts"},
			{"Corrosivity Sensor",	44,	8,	0,	0.0196,		0.0,	"volts"},
			LINUXALDL_MODE1_END_DEF
	};


aldl_definition aldl_DF = { "91-93 3.4 DOHC LQ1 ($DF)",
							{0xF4, 0x57, 0x01, 0x00, 0xB4}, 5, 67, 63, 3, aldl_DF_mode1,
							{0xF4, 0x56, 0x08, 0xAE}, 4,
							{0xF4, 0x56, 0x09, 0xAD}, 4
						};

// ===========================================
//  End DF code mask definition
// ===========================================


// =================================================================================
// Definition table -- pointers to each definition must be in this table to use them
// =================================================================================
//   aldl_get_definition() uses this table to find the definition corresponding 
//   to the string given at the command line for the mask= argument.
//   the gui uses this table to make the list of definitions.
//   the last entry must be NULL.
aldl_definition* aldl_definition_table[2] = { &aldl_DF, NULL };


#endif
