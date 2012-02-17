


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


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/signal.h>
#include <string.h> // for memcpy
#include "linuxaldl_gui.h"
#include "linuxaldl.h"
#include "sts_serial.h"


// global variable which holds the current definition pointer, file descriptors, etc
extern linuxaldl_settings aldl_settings;

// global variable which holds gui-specific pointers, data, etc
linuxaldl_gui_settings aldl_gui_settings = { NULL, {0,0}, ALDL_LOG_RAW, NULL, 0};

// ========================================================================
//
// 					GUI MODE FUNCTION DEFINITIONS
//
// ========================================================================


// runs the linuxaldl GTK+ graphical user interface
int linuxaldl_gui(int argc, char* argv[])
{
	// ================================================================
	//				MAIN GUI INTIIALIZATION 
	// ================================================================
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *vbox_main;
	GtkWidget *vbox_settings;
	GtkWidget *bbox_settings_top;
	GtkWidget *frame_data_control;
	GtkWidget *bbox_cmds;
	GtkWidget *frame_cmds;
	GtkWidget *lfilew; // load file selection dialogue 
	GtkWidget *sfilew; // save file selection dialogue 
	GtkWidget *optionsw; // options/settings window
	GtkWidget *choosedefw; // definition selection dialogue
	GtkWidget *datareadoutw; // datareadout window
	char* logfilename;
	
	gtk_init(&argc, &argv);
	


	// Main window settings
	// ========================================================================
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (window), "linuxaldl - GUI Mode");
	gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT(window),"delete_event", G_CALLBACK (linuxaldl_gui_quit), NULL);
	g_signal_connect(G_OBJECT(window),"destroy", G_CALLBACK (gtk_main_quit), NULL);

	// main vbox organizes settings seperately from progress bar and command buttons
	vbox_main = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox_main);	
	gtk_container_add(GTK_CONTAINER (window), vbox_main);
	
	// hbox to seperate chip selection and log load/save from addressing information entry forms
	vbox_settings = gtk_vbox_new(FALSE,0);
	gtk_widget_show(vbox_settings);
	
	// bbox for save/load/select definition buttons along top of window
	bbox_settings_top = gtk_hbutton_box_new();
	gtk_button_box_set_layout(GTK_BUTTON_BOX (bbox_settings_top), GTK_BUTTONBOX_SPREAD);

	// Load File selection dialogue
	// ========================================================================
	lfilew = gtk_file_selection_new("Load .log file to view");
	gtk_window_set_position(GTK_WINDOW (lfilew), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT (lfilew), "destroy", G_CALLBACK (linuxaldl_gui_widgethide), (gpointer) lfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (lfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxaldl_gui_load), (gpointer) lfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (lfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxaldl_gui_widgethide), (gpointer) lfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (lfilew)->cancel_button), "clicked",
										G_CALLBACK (linuxaldl_gui_widgethide), (gpointer) lfilew);
	// stop the window from being destroyed when the X is clicked at the top right
	g_signal_connect(G_OBJECT(lfilew),"delete_event", G_CALLBACK (hide_on_delete), NULL);
										
	// Save File selection dialogue
	// ========================================================================
	sfilew = gtk_file_selection_new("Select .log file to write to");
	gtk_window_set_position(GTK_WINDOW (sfilew), GTK_WIN_POS_CENTER);
	g_signal_connect(G_OBJECT (sfilew), "destroy", G_CALLBACK (linuxaldl_gui_widgethide), (gpointer) sfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (sfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxaldl_gui_save), (gpointer) sfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (sfilew)->ok_button), "clicked", 
										G_CALLBACK (linuxaldl_gui_widgethide), (gpointer) sfilew);
	g_signal_connect(G_OBJECT (GTK_FILE_SELECTION (sfilew)->cancel_button), "clicked",
										G_CALLBACK (linuxaldl_gui_widgethide), (gpointer) sfilew);
	// stop the window from being destroyed when the X is clicked at the top right
	g_signal_connect(G_OBJECT(sfilew),"delete_event", G_CALLBACK (hide_on_delete), NULL);
	
	// Choose Definition dialogue
	// ========================================================================
	choosedefw = linuxaldl_gui_choosedef_new();

	// Options window
	// ========================================================================
	optionsw = linuxaldl_gui_options_new();

	// Data Readout window
	// ========================================================================
	datareadoutw = linuxaldl_gui_datareadout_new();

	// Commands buttons (scan, stop, options, quit)
	// ========================================================================
		
	// button box for commands
	bbox_cmds = gtk_hbutton_box_new ();
	gtk_button_box_set_layout(GTK_BUTTON_BOX (bbox_cmds), GTK_BUTTONBOX_SPREAD);
	gtk_widget_show(bbox_cmds);	
	
	// frame for commands
	frame_cmds = gtk_frame_new("Commands");	
	gtk_container_add(GTK_CONTAINER (frame_cmds), bbox_cmds);
	gtk_widget_show(frame_cmds);
	
	// scan button
	button = gtk_toggle_button_new_with_label("Scan");
	g_signal_connect(G_OBJECT(button), "toggled", G_CALLBACK(linuxaldl_gui_scan_toggle), (gpointer) &aldl_settings);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);	

	
	// options button
	button = gtk_button_new_with_label("Options");
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(linuxaldl_gui_widgetshow), (gpointer) optionsw);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);	
	
	// exit button
	button = gtk_button_new_with_label("Quit");
	g_signal_connect(G_OBJECT(button), "clicked",
							G_CALLBACK(linuxaldl_gui_quit), NULL);
	gtk_container_add(GTK_CONTAINER (bbox_cmds), button);
	gtk_widget_show(button);
	

	gtk_box_pack_end (GTK_BOX (vbox_main), frame_cmds, FALSE, FALSE, 0);
	

	// Settings (load .log, save .log, select definition, etc.)
	// ========================================================================
	gtk_box_pack_start (GTK_BOX (vbox_main), vbox_settings, FALSE, FALSE, 0);
	gtk_widget_show(vbox_settings);
	
	// add vbox to top of settings area 
	gtk_box_pack_start (GTK_BOX (vbox_settings), bbox_settings_top, FALSE, FALSE, 0);
	gtk_widget_show(bbox_settings_top);
	
	// -------------------------------------------------------------------------
	// Buttons for top of settings area
	// -------------------------------------------------------------------------

	// load .log file button
	button = gtk_button_new_with_label("Load .log (playback)");
	g_signal_connect(G_OBJECT(button), "clicked",
					G_CALLBACK(linuxaldl_gui_widgetshow), (gpointer) lfilew);
	gtk_container_add(GTK_CONTAINER (bbox_settings_top), button);
	gtk_widget_show(button);
	
	// save .log file button
	button = gtk_button_new_with_label("Select .log (to record)");
	g_signal_connect(G_OBJECT(button), "clicked",
					G_CALLBACK(linuxaldl_gui_widgetshow), (gpointer) sfilew);
	gtk_container_add(GTK_CONTAINER (bbox_settings_top), button);
	gtk_widget_show(button);
	
	// select definition
	button = gtk_button_new_with_label("Select definition");
	g_signal_connect(G_OBJECT(button), "clicked",
					G_CALLBACK(linuxaldl_gui_try_choosedef), (gpointer) choosedefw);
	gtk_container_add(GTK_CONTAINER (bbox_settings_top), button);
	gtk_widget_show(button);

	// ----------------------------------------------------------------
	// ALDL Data Access frame
	// ----------------------------------------------------------------
	// frame for data control settings/commands
	frame_data_control = gtk_frame_new("Data Access");
	gtk_box_pack_start(GTK_BOX(vbox_settings), frame_data_control, FALSE, FALSE, 0);
	gtk_widget_show(frame_data_control);

	// select definition
	button = gtk_button_new_with_label("Show Data Readout");
	g_signal_connect(G_OBJECT(button), "clicked",
					G_CALLBACK(linuxaldl_gui_datareadout_show), (gpointer) datareadoutw);
	gtk_container_add(GTK_CONTAINER (frame_data_control), button);
	gtk_widget_show(button);

	// -------------------------------------------------------------------------
	//		Display the main window and start the gtk main loop
	// -------------------------------------------------------------------------
	gtk_widget_show(window);
	
	gtk_main();
	
	return 0;
} 

// ==================================
//  GENERAL PURPOSE WINDOW CONTROL
// ==================================

// calls gtk_widget_show on the widget specified in the data argument
static void linuxaldl_gui_widgetshow(GtkWidget *widget, gpointer data)
{
	gtk_widget_show(GTK_WIDGET (data));
}

// calls gtk_widget_hide on the widget specified in the data argument
static void linuxaldl_gui_widgethide(GtkWidget *widget, gpointer data)
{
	gtk_widget_hide(GTK_WIDGET (data));
}

// ===================================
//    EXIT / DELETE_EVENT
// ===================================

// XXX need to free stuff here, also pop up a confirmation dialogue XXX
// called on delete_event, closes the connection (if there is one)
// and call gtk_main_quit(). if a transfer is currently in progress,
// should pop up a dialogue to confirm quitting.
static gboolean linuxaldl_gui_quit( GtkWidget *widget, GdkEvent *event, gpointer data)
{
	gtk_main_quit();
	g_free(aldl_gui_settings.data_readout_labels);
	g_free(aldl_settings.data_set_raw);
	g_free(aldl_settings.data_set_strings); // XXX need to free each string too, not just the pointer array!!
	g_free(aldl_settings.data_set_floats);
	return FALSE;
}

// returns TRUE so that a window is not destroyed on a delete event
static gboolean hide_on_delete( GtkWidget *widget, GdkEvent *event, gpointer data){
	linuxaldl_gui_widgethide(widget,widget);
	return TRUE;
}

// ===================================
//    SCAN OPERATION FOR GUI MODE
// ===================================


// callback for change in the adjustment for the aldl_settings.scan_interval field.
// if scanning is not taking place, does nothing except store the new values and enforce
// timeout/ scan interval constraints (interval must be at least 20msec more than timeout).
// otherwise it reassigns the scan interval to the new value immediately.
// adj must point to the GtkAdjustment for the scan interval.
static void linuxaldl_gui_scan_interval_changed( GtkAdjustment *adj, gpointer data)
{
	unsigned int new_interval = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

	if (new_interval <= aldl_settings.scan_timeout+19)
	{
		new_interval = 20 + aldl_settings.scan_timeout;
		gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),new_interval);
	}

	aldl_settings.scan_interval = new_interval;


	if (aldl_settings.scanning == 1)
	{
		g_source_remove(aldl_gui_settings.scanning_tag);

		aldl_gui_settings.scanning_tag = g_timeout_add(aldl_settings.scan_interval,
															linuxaldl_gui_scan_on_interval,
															NULL);
	}
}

// callback for change in the adjustment for the aldl_settings.scan_timeout field.
// stores the new values and enforces timeout/ scan interval constraints (interval
// must be at least 20msec more than timeout).
static void linuxaldl_gui_scan_timeout_changed( GtkAdjustment *adj, gpointer data)
{
	unsigned int new_timeout = gtk_adjustment_get_value(GTK_ADJUSTMENT(adj));

	if (new_timeout+19 >= aldl_settings.scan_interval)
	{
		new_timeout = aldl_settings.scan_interval-20;
		gtk_adjustment_set_value(GTK_ADJUSTMENT(adj),new_timeout);
	}
	
	aldl_settings.scan_timeout = new_timeout;
}

// callback for g_timeout interval timer. if aldl_settings.scanning == 1 
// then this function will call linuxaldl_gui_scan
gint linuxaldl_gui_scan_on_interval(gpointer data)
{
 	if (aldl_settings.scanning == 0)
	{
		send_aldl_message(_ALDL_MESSAGE_MODE9); // send a mode 9 message to allow the ecm to resume normal mode
		return 0; // returning 0 tells GTK to turn off the interval timer for this function
	}
	linuxaldl_gui_scan(NULL, NULL); // perform a scan operation
	return 1;
}

// perform a single scan operation: attempt to get one mode1 message,
// save the data to the log and update the current data array
static void linuxaldl_gui_scan( GtkWidget *widget, gpointer data)
{
	int res;
	char* inbuffer;
	unsigned int buf_size;	
	buf_size = aldl_settings.definition->mode1_response_length;

	inbuffer = g_malloc(buf_size);
	
	// send a mode 8 message to silence the ecm
	send_aldl_message(_ALDL_MESSAGE_MODE8);
	tcflush(aldl_settings.faldl,TCIOFLUSH); // flush send and receive buffers

	// request a mode 1 message
	res = get_mode1_message(inbuffer, buf_size);
#ifdef _LINXUALDL_DEBUG
	if (res==-1)
	{
		// bad checksum
		fprintf(stderr,"c");
	}
	else if (res==0)
	{
		// timeout/partial response
		fprintf(stderr,"t");
	}
#endif

	if (res>0)
	{
#ifdef _LINXUALDL_DEBUG
		// got full mode1 message
		fprintf(stderr,"+");
#endif
		// update the timestamp
		gettimeofday(&aldl_gui_settings.data_timestamp, NULL);


		// check to see if a data set for the display has been allocated
		if (aldl_settings.data_set_raw == NULL)
		{
			g_warning("Data set labels not initialized. Display and .csv log files will not be updated. \n");
		}
		else
		{
			// copy the data to the current data set
			memcpy(	  aldl_settings.data_set_raw,
					  inbuffer + aldl_settings.definition->mode1_data_offset,
					  aldl_settings.definition->mode1_data_length);

			// update string and float representations
			aldl_update_sets(ALDL_UPDATE_FLOATS|ALDL_UPDATE_STRINGS);

			// update the Data Readout
			linuxaldl_gui_datareadout_update(NULL,NULL);
		}

		// if a log file has been selected
		if (aldl_settings.flogfile!=1)
		{
			// ALDL_LOG_RAW
			// ============
			// raw format just dumps the timestamp and entire mode1 message to a file
			if (aldl_gui_settings.log_format == ALDL_LOG_RAW)
			{
				// note: this writes the integer timestamp using the endianness of the platform this
				// process is running on. this means if the log file is read back using a platform
				// with a different endianness, the timestamps will be incorrect.
				// x86 platforms are little-endian.

				// write the seconds to the file
				write(aldl_settings.flogfile,(char*)(&aldl_gui_settings.data_timestamp.tv_sec),sizeof(time_t));

				// write the microseconds to the file
				write(aldl_settings.flogfile,(char*)(&aldl_gui_settings.data_timestamp.tv_usec),sizeof(suseconds_t));

				// write the data to the file
				write(aldl_settings.flogfile,inbuffer,res);
				//g_print("%d bytes written to file/output.",res);
			}
			// ALDL_LOG_CSV
			// ============
			// CSV format conforming to RFC4180 http://tools.ietf.org/html/rfc4180
			else if (aldl_gui_settings.log_format == ALDL_LOG_CSV)
				linuxaldl_gui_write_csv_line();
		}

	}
	g_free(inbuffer);
	return;
}

// this function is called when the scan button is toggled
static void linuxaldl_gui_scan_toggle( GtkWidget *widget, gpointer data)
{
	struct itimerval timer_value;

	// if the button is down
    if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) 
    {
		// abort if no definition file selected, and toggle the button to off
        if (aldl_settings.definition == NULL)
		{
			g_print("Scan aborted -- No definition file selected.\n");
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(widget),FALSE);
			return;
		}
		if (aldl_settings.scanning == 0)
		{
			g_print("Starting scan.\n");
			aldl_gui_settings.scanning_tag = g_timeout_add(aldl_settings.scan_interval,
															linuxaldl_gui_scan_on_interval,
															NULL);
			aldl_settings.scanning = 1;

			linuxaldl_gui_scan(NULL,NULL); // perform initial scan
		}
    } 
	else 	{
		// button is up (turned off)
		g_print("Stopping scan.\n");
		aldl_settings.scanning = 0; // reset scan flag	
		return;
	}
}

// ===================================
//    LOAD .LOG FILE SELECTION
// ===================================

// this function is called as the result of the "ok" button being
// selected in the load log file selection dialogue.
// XXX not implemented
static void linuxaldl_gui_load( GtkWidget *widget, GtkFileSelection *fs){
	g_warning("Log viewing module is not yet implemented.\n");
	return;
}

// ===================================
//    SAVE .LOG FILE SELECTION
// ===================================

// this function is called as the result of the "ok" button being
// selected in the save log file selection dialogue. it saves 
// the current buffer into the selected file.
static void linuxaldl_gui_save( GtkWidget *widget, GtkFileSelection *fs){
	int i, length, res;
	char extension[4];
	const char* logfilename = gtk_file_selection_get_filename(fs);
	aldl_settings.logfilename= logfilename;	

	// Open the .log file for write, create if it doesnt exist
	// ------------------------------------------------------------
	aldl_settings.flogfile = open(logfilename,O_RDWR | O_CREAT, 0666);
	if (aldl_settings.flogfile == -1)
	{
		g_print("Unable to open/create %s for writing.\n",logfilename);
		return;	}
	
	g_print("Log file %s opened. ALDL data will be written to this file as it is received.\n",logfilename);


	// Get the file extension to determine what format to use
	// ----------------------------------------------------------

	// count how many characters are in the filename
	for (length=0; logfilename[length]!=0; length++)
 	{ /* do nothing */ }
	

	if (length<5)
	{
		// too short for an extension
		// use raw format.
		aldl_gui_settings.log_format = ALDL_LOG_RAW;
		return;
	}

	// get the extension
	memcpy(extension,logfilename+length-4,4);

	// .csv or .CSV extension
	if (0 == strncmp(".csv",extension,4) || 0 == strncmp(".CSV",extension,4))
	{
		aldl_gui_settings.log_format = ALDL_LOG_CSV;

		aldl_gui_settings.slogfile = fdopen(aldl_settings.flogfile,"a+");


		// if no definition is loaded yet, the definition loader has to write the header line.

		// if no definition file selected, dont try to generate the header labels yet.
        if (aldl_settings.definition == NULL)
			g_warning(".csv format selected but no definition has been loaded.\n");
		else
			linuxaldl_gui_write_csv_header();

	}

	return;
}


// =======================
//	 CSV FORMAT LOGGING
// =======================

// write the header line to the csv file
static void linuxaldl_gui_write_csv_header()
{
	int i;

	if (aldl_gui_settings.slogfile==NULL)
	{
		g_warning("linuxaldl_gui_write_csv_header() invoked but no CSV file stream opened.\n");
		return;
	}
	if (aldl_settings.definition == NULL)
	{
		g_warning("Definition file not selected. .CSV header line could not be written.\n");
		return;
	}

	byte_def_t* def = aldl_settings.definition->mode1_def;

	// print the timestamp label
	fprintf(aldl_gui_settings.slogfile,"Timestamp");

	// until at the end of the items in the definition
	for (i=0; def[i].label!=NULL; i++)
	{
		if (def[i].operation!=ALDL_OP_SEPERATOR)
			fprintf(aldl_gui_settings.slogfile,",%s",def[i].label);
	}
	fprintf(aldl_gui_settings.slogfile,"\n");
}


// write a data line for the csv file
static void linuxaldl_gui_write_csv_line()
{
	int i;

	if (aldl_gui_settings.log_format != ALDL_LOG_CSV)
	{
		g_warning("linuxaldl_gui_write_csv_line() invoked but CSV format not selected.\n");
		return;
	}
	else if (aldl_gui_settings.slogfile==NULL)
	{
		g_warning("linuxaldl_gui_write_csv_line() invoked but no CSV file stream opened.\n");
		return;
	}
	else if (aldl_settings.definition == NULL)
	{
		g_warning("Definition file not selected. .CSV data line could not be written.\n");
		return;
	}
	else if (aldl_settings.data_set_strings!= NULL)
	{
		byte_def_t* def = aldl_settings.definition->mode1_def;

		// write the timestamp
		fprintf(aldl_gui_settings.slogfile,"%d+%f", (int)aldl_gui_settings.data_timestamp.tv_sec,
											(float)aldl_gui_settings.data_timestamp.tv_usec/1000000.0);

		// until at the end of the items in the definition...
		for (i=0; def[i].label!=NULL; i++)
		{
			// if the item is not a seperator, write the string
			if (def[i].operation != ALDL_OP_SEPERATOR)
				fprintf(aldl_gui_settings.slogfile,",%s",aldl_settings.data_set_strings[i]);
		}
		fprintf(aldl_gui_settings.slogfile,"\n"); // end the line
	}
}


// ==========================================================================
//
//     GUI accessory windows
//
// ==========================================================================



// ==================================
//    Options Window
// ==================================

// returns a GtkWidget pointer to a new options window
GtkWidget* linuxaldl_gui_options_new()
{
	// base window
	// -----------
	GtkWidget* optionsw;

	optionsw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(optionsw), "linuxaldl - Options & Settings");
	gtk_window_set_position(GTK_WINDOW(optionsw), GTK_WIN_POS_CENTER);

	// stop the window from being destroyed when the X is clicked at the top right
	g_signal_connect(G_OBJECT(optionsw),"delete_event", G_CALLBACK (hide_on_delete), NULL);

	// main vbox
	// ---------
	GtkWidget* vbox_main = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(optionsw), vbox_main);
	gtk_widget_show(vbox_main);

	// 'options' frame
	// ---------------
	GtkWidget* frame_options = gtk_frame_new("Options");
	gtk_box_pack_start(GTK_BOX(vbox_main),frame_options, FALSE, FALSE, 0);
	gtk_widget_show(frame_options);

	GtkWidget* vbox_options = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(frame_options), vbox_options);
	gtk_widget_show(vbox_options);

	// scan interval adjustment
	GtkWidget* interval_adj = hscale_new_with_label(aldl_settings.scan_interval,
														50.0, 300.0, 1.0,
									G_CALLBACK(linuxaldl_gui_scan_interval_changed),
									"Scan Interval (msec)");


	gtk_box_pack_start(GTK_BOX(vbox_options),interval_adj,FALSE,FALSE,0);
	gtk_widget_show(interval_adj);
	

	// timeout adjustment
	GtkWidget* timeout_adj = hscale_new_with_label(aldl_settings.scan_timeout,
													50.0,300.0, 1.0,
									G_CALLBACK(linuxaldl_gui_scan_timeout_changed),
									"Scan Timeout (msec)");

	gtk_box_pack_start(GTK_BOX(vbox_options),timeout_adj,FALSE,FALSE,0);
	gtk_widget_show(timeout_adj);

	// 'settings' frame
	// ----------------
	GtkWidget* frame_settings = gtk_frame_new("Settings");
	gtk_box_pack_start(GTK_BOX(vbox_main),frame_settings, FALSE, FALSE, 0);
	gtk_widget_show(frame_settings);

	return optionsw;
}



// ==================================
//    Data Readout window
// ==================================

// returns a GtkWidget pointer to an empty datareadout window
GtkWidget* linuxaldl_gui_datareadout_new()
{
	int i = 0;
	GtkWidget* dataw;

	dataw = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (dataw), "linuxaldl - Data Readout");
	gtk_window_set_position(GTK_WINDOW (dataw), GTK_WIN_POS_CENTER);

	// stop the window from being destroyed when the X is clicked at the top right
	g_signal_connect(G_OBJECT(dataw),"delete_event", G_CALLBACK (hide_on_delete), NULL);

	return dataw;	
}


// updates the Data Readout window, refreshing it with the current data values.
// this function will check if aldl_settings.data_readout_labels
// has been allocated yet before it tries to update the values.
// if it has not yet been allocated, it returns doing nothing.
// .csv log file updating is also done here.
static void linuxaldl_gui_datareadout_update(GtkWidget* widget, gpointer data)
{
	if (aldl_gui_settings.data_readout_labels == NULL)
		return;

	unsigned int i=0;
	byte_def_t* defs = aldl_settings.definition->mode1_def;
	byte_def_t* cur_def;
	float converted_val;
	char new_label_string[10];


	for (i=0; defs[i].label != NULL; i++) // while not at the last defined byte
	{
		cur_def = defs+i;
		// if the item is a seperator, skip it
		if (cur_def->operation == ALDL_OP_SEPERATOR)
			continue;

		// assign the new label
		gtk_label_set_text(GTK_LABEL(aldl_gui_settings.data_readout_labels[i]), aldl_settings.data_set_strings[i]);
	}
}

// shows the Data Readout window, setting it up for the current definition.
// data must point to the Data Readout window object generated by
// the linuxaldl_gui_datareadout_new function
static void linuxaldl_gui_datareadout_show(GtkWidget* widget, gpointer data)
{
		// if no definition selected, return without doing anything
		if (aldl_settings.definition == NULL)
		{
			g_print("No definition selected. Cannot show data readout. Choose a definition first.\n");
			return;
		}
		// if the Data Readout labels have been defined, no need to generate, just show the window
		if (aldl_gui_settings.data_readout_labels != NULL)
		{
			linuxaldl_gui_widgetshow(widget,data);
			return;
		}

		GtkWidget *vbox;
		GtkWidget *databox;
		GtkWidget *frame_main;
		byte_def_t* def = aldl_settings.definition->mode1_def;
		unsigned int i = 0, num_items=0, row=0;

		vbox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(data), vbox);
		frame_main = gtk_frame_new("ALDL Data");		
		gtk_box_pack_start(GTK_BOX (vbox), frame_main, FALSE, FALSE, 0);

		// the frame for each data seperator go in databox
		databox = gtk_vbox_new(FALSE,0);
		gtk_container_add(GTK_CONTAINER(frame_main), databox);

		gtk_widget_show(vbox);
		gtk_widget_show(frame_main);
		gtk_widget_show(databox);

		// Parse the Definition

		// count the number of items in the definition
		while (def[num_items].label != NULL)
		{
			num_items++;
		}


		// allocate an array for the data label pointers.
		// data_readout_labels[i] points to the label that
		// contains a string representation of the value of
		// the data element described by definition->mode1_def[i].
		// where mode1_def[i] describes a seperator/label the value of
		// data_readout_labels[i] is not defined.
		// XXX need to free all the strings on exit, as well as this array..
		aldl_gui_settings.data_readout_labels = g_malloc0((num_items)*sizeof(GtkWidget*));
		GtkWidget *cur_vbox;
		GtkWidget *cur_frame;
		GtkWidget *cur_label;
		GtkWidget *cur_table;

		// until at the end of the items in the definition
		for (i=0; def[i].label!=NULL; i++)
		{
			// if the item is a seperator
			if (def[i].operation == ALDL_OP_SEPERATOR)
			{
				// make a new frame
				cur_frame = gtk_frame_new(def[i].label);
				gtk_box_pack_start(GTK_BOX(databox), cur_frame, FALSE, FALSE, 0);
				gtk_widget_show(cur_frame);

				// make a table for the data elements in the frame

				// count how many elements there are until the next divider (or end of the definition)
				for (row=0; (def[i+row+1].label!=NULL && def[i+row+1].operation !=ALDL_OP_SEPERATOR); row++)
				{ /* do nothing (just count up rows)*/	}

				cur_table = gtk_table_new(row,3,FALSE);

				gtk_container_add(GTK_CONTAINER(cur_frame),cur_table);
				gtk_widget_show(cur_table);

				// reset row to zero so it can be used when putting items in the table
				row=0; 
			}

			else // data item
			{
				// make a label for the data item's name
				cur_label = gtk_label_new(def[i].label);
				// make an alignment to align the label in the table
				GtkWidget *align = gtk_alignment_new(0.0,0.0,0.0,0.0);

				// add the label to the alignment, alignment to the current table
				gtk_container_add(GTK_CONTAINER(align), cur_label);
				gtk_table_attach_defaults(GTK_TABLE(cur_table), align, 0,1, row,row+1);
				gtk_widget_show(cur_label);
				gtk_widget_show(align);

				// make a label for the data value
				cur_label = gtk_label_new("N/A");
				// make an alignment to align the label in the table
				align = gtk_alignment_new(0.0,0.0,0.0,0.0);
				// add the label to the alignment, alignment to the current table
				gtk_container_add(GTK_CONTAINER(align), cur_label);
				gtk_table_attach(GTK_TABLE(cur_table), align, 1,2, row,row+1,
									GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 10, 0);
				gtk_widget_show(cur_label);
				gtk_widget_show(align);

				// register the data value label with the data displays array
				aldl_gui_settings.data_readout_labels[i] = cur_label;

				// make a label for the units
				cur_label = gtk_label_new(def[i].units);
				// make an alignemnt to align the label in the box
				align = gtk_alignment_new(0.0,0.0,0.0,0.0);
				// add the label to the alignment, alignment to the current table
				gtk_container_add(GTK_CONTAINER(align), cur_label);
				gtk_table_attach_defaults(GTK_TABLE(cur_table), align, 2,3, row,row+1);
				gtk_widget_show(cur_label);
				gtk_widget_show(align);
				
				row++;
			}
		}

		linuxaldl_gui_widgetshow(widget,data);
}

// ==================================
//    Definition selection dialog
// ==================================


// opens the definition selection dialog if no definition has been selected,
// otherwise pops up an alert and returns
static void linuxaldl_gui_try_choosedef( GtkWidget *widget, gpointer data)
{
	if (aldl_settings.definition==NULL)
		linuxaldl_gui_widgetshow(widget,data);
	else
	{
		quick_alert("Definition already selected.\nRestart the program to select a new definition\n");													
	}
}

// returns a GtkWidget pointer to the definition selection dialog
GtkWidget* linuxaldl_gui_choosedef_new()
{
	int i = 0;
	GtkWidget* defwindow;
	GtkWidget* dropdown; // dropdown box for definition file selection
	GtkWidget* vbox;
	GtkWidget* bbox; // button box for commands
	GtkWidget* button; // command buttons

	defwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (defwindow), "linuxaldl - Choose ALDL Definition");
	gtk_window_set_position(GTK_WINDOW (defwindow), GTK_WIN_POS_MOUSE);
	gtk_window_set_resizable(GTK_WINDOW(defwindow), FALSE);

	vbox = gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER (defwindow), vbox);
	gtk_widget_show(vbox);	

	// Combo Box for ALDL definition selection
	// ---------------------------------------
	dropdown = gtk_combo_new();
	gtk_entry_set_text (GTK_ENTRY (GTK_COMBO (dropdown)->entry),
						"No Definitions");
	
	// Form the entries for the combo box
	GList *deflist = NULL;
	g_print("Loading definitions...\n");
	g_print(" Found definitions: ");
	while (aldl_settings.aldl_definition_table[i] != NULL)
	{
		if (i!=0)
			g_print(", ");
		g_print("%s",aldl_settings.aldl_definition_table[i]->name);
		deflist = g_list_append (deflist, (gpointer) aldl_settings.aldl_definition_table[i]->name);
		i++;
	}
	if (i==0) g_print(" none.\n");
	else g_print("\n");

	gtk_combo_set_popdown_strings(GTK_COMBO (dropdown), deflist);
	
	gtk_box_pack_start (GTK_BOX (vbox), dropdown, FALSE, FALSE, 0);

	gtk_widget_show(GTK_WIDGET (dropdown));


	// Buttons (select, cancel)
	// --------------------------------------

	// button box for commands
	bbox = gtk_hbutton_box_new ();
	gtk_button_box_set_layout(GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
	gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);
	gtk_widget_show(bbox);	

	// "cancel" button
	button = gtk_button_new_with_label("Cancel");
	g_signal_connect(G_OBJECT(button), "clicked",
					G_CALLBACK(linuxaldl_gui_widgethide), (gpointer) defwindow);
	gtk_container_add(GTK_CONTAINER (bbox), button);
	gtk_widget_show(button);

	// "select" button
	button = gtk_button_new_with_label("Select");
	g_signal_connect(G_OBJECT(button), "clicked",
						G_CALLBACK(linuxaldl_gui_widgethide), (gpointer) defwindow);
	g_signal_connect(G_OBJECT(button), "clicked",
						G_CALLBACK (linuxaldl_gui_load_definition),
						GTK_ENTRY (GTK_COMBO (dropdown)->entry));
	gtk_container_add(GTK_CONTAINER (bbox), button);
	gtk_widget_show(button);

	// stop the window from being destroyed when the X is clicked at the top right
	g_signal_connect(G_OBJECT(defwindow),"delete_event", G_CALLBACK (hide_on_delete), NULL);

	return defwindow;	
}




// this function refreshes the aldl definition in aldl_settings when the user
// presses the "select" button in the definition selection dialogue.
// data must point to a valid GtkEntry that contains the string for the definition name.

static void linuxaldl_gui_load_definition( GtkWidget *widget, gpointer data)
{
	aldl_settings.aldldefname = gtk_entry_get_text (GTK_ENTRY (data));

	// get the aldl definition address
	aldl_settings.definition = aldl_get_definition(aldl_settings.aldldefname);

	// if no definition by that name exists, definition will be NULL
	if (aldl_settings.definition == NULL)
	{
		g_print("No definition with name \"%s\" found.\n",aldl_settings.aldldefname);
		return;
	}

	g_print("Definition \"%s\" selected:\n",aldl_settings.definition->name);
	g_print(" Mode 1 message has %d data bytes.\n",aldl_settings.definition->mode1_data_length);
	



	// allocate memory for the raw data array and zero it out
	aldl_settings.data_set_raw = g_malloc0(aldl_settings.definition->mode1_data_length);
	// allocate memory for the float array
	aldl_settings.data_set_floats = g_malloc0(aldl_settings.definition->mode1_data_length*sizeof(float));
	// allocate memory for the string pointers array
	aldl_settings.data_set_strings = g_malloc0(aldl_settings.definition->mode1_data_length*sizeof(char*));

	// if the log format is already set to CSV but we are just now loading a definition,
	// then the labels were not ready when we started. write the header line now.
	if(aldl_gui_settings.log_format == ALDL_LOG_CSV)
	{
		linuxaldl_gui_write_csv_header();
	}

	return;
}

// ===========================================
//		GENERAL USE GUI HELPER FUNCTIONS
// ===========================================

// ==================================
//  Alert popup
// ==================================

// pops up an alert with message in the body and an ok button.
// the alert will block access to other windows until the button is clicked.
// background activity will continue.
void quick_alert(gchar *message)
{
		GtkWidget *label, *content_area;
		GtkWidget *dialog = gtk_dialog_new_with_buttons("Alert",NULL,
						GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT |GTK_DIALOG_NO_SEPARATOR,
						GTK_STOCK_OK, GTK_RESPONSE_DELETE_EVENT, NULL);
		g_signal_connect_swapped(dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
		content_area = GTK_DIALOG(dialog)->vbox;
		label = gtk_label_new(message);
		gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
		gtk_box_pack_start(GTK_BOX(content_area),label,FALSE,FALSE,10);
		gtk_widget_show_all(dialog);
}



// ==============================
//  Adjustment scale with label
// ==============================
// returns a vbox containing a horizontal scale with a new adjustment
// with range min to max and step size step that starts out with the initial value init_val.
// the adjustment calls the function change_callback when the value changes.
// it will have a label placed above it with the text adj_label
GtkWidget* hscale_new_with_label(gdouble init_val, gdouble min, gdouble max, gdouble step, GtkSignalFunc changed, gchar *adj_label)
{
	GtkWidget* vbox = gtk_vbox_new(FALSE,0);
	GtkWidget* label = gtk_label_new(adj_label);
	gtk_box_pack_start(GTK_BOX(vbox),label, FALSE, FALSE,0);
	gtk_widget_show(label);

	GtkObject* new_adj = gtk_adjustment_new(init_val, min, max, step, 0.0, 0.0);

	g_signal_connect(G_OBJECT (new_adj), "value_changed",
				     G_CALLBACK(changed), NULL);

	GtkWidget* new_scale = gtk_hscale_new(GTK_ADJUSTMENT(new_adj));
	gtk_scale_set_value_pos( GTK_SCALE(new_scale),GTK_POS_RIGHT );
	gtk_range_set_update_policy( GTK_RANGE(new_scale),GTK_UPDATE_DISCONTINUOUS);

	gtk_box_pack_start(GTK_BOX(vbox),new_scale,FALSE,FALSE,0);
	gtk_widget_show(new_scale);
	gtk_widget_show(vbox);

	return vbox;
}
