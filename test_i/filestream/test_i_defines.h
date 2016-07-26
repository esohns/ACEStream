/***************************************************************************
*   Copyright (C) 2009 by Erik Sohns   *
*   erik.sohns@web.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef TEST_I_DEFINES_H
#define TEST_I_DEFINES_H

//#include "ace/Default_Constants.h"

//#include "net_common.h"

#define TEST_I_DEFAULT_GTK_RC_FILE                           "resources.rc"
#define TEST_I_DEFAULT_SOURCE_GLADE_FILE                     "source.glade"
#define TEST_I_DEFAULT_TARGET_GLADE_FILE                     "target.glade"
#define TEST_I_DEFAULT_OUTPUT_FILE                           "output.tmp"
#define TEST_I_THREAD_NAME                                   "stream processor"

//#define TEST_I_DEFAULT_BUFFER_SIZE                           32768 // bytes
#define TEST_I_DEFAULT_BUFFER_SIZE                           65536 // bytes
#define TEST_I_MAX_MESSAGES                                  0 // 0 --> no limits

#define TEST_I_DEFAULT_TARGET_HOSTNAME                       ACE_LOCALHOST
#define TEST_I_DEFAULT_PORT                                  10001
#define TEST_I_MAXIMUM_NUMBER_OF_OPEN_CONNECTIONS            0 // 0 --> no limits
#define TEST_I_DEFAULT_NUMBER_OF_DISPATCHING_THREADS         10

#define TEST_I_DEFAULT_TRANSPORT_LAYER                       NET_TRANSPORTLAYER_TCP

//---------------------------------------

#define TEST_I_STREAM_UI_GTKEVENT_RESOLUTION                 200 // ms --> 5 FPS
#define TEST_I_STREAM_UI_GTK_ACTION_CLOSE_ALL_NAME           "action_close_all"
#define TEST_I_STREAM_UI_GTK_ACTION_REPORT_NAME              "action_report"
#define TEST_I_STREAM_UI_GTK_ACTION_START_NAME               "action_start"
#define TEST_I_STREAM_UI_GTK_ACTION_STOP_NAME                "action_stop"
#define TEST_I_STREAM_UI_GTK_ADJUSTMENT_NAME                 "scrolledwindow_vadjustment"
#define TEST_I_STREAM_UI_GTK_BUTTONBOX_ACTIONS_NAME          "vbuttonbox"
#define TEST_I_STREAM_UI_GTK_BUTTON_ABOUT_NAME               "about"
#define TEST_I_STREAM_UI_GTK_BUTTON_CLEAR_NAME               "clear"
#define TEST_I_STREAM_UI_GTK_BUTTON_REPORT_NAME              "report"
//#define TEST_I_STREAM_UI_GTK_BUTTON_START_NAME               "start"
//#define TEST_I_STREAM_UI_GTK_BUTTON_STOP_NAME                "stop"
#define TEST_I_STREAM_UI_GTK_BUTTON_QUIT_NAME                "quit"
#define TEST_I_STREAM_UI_GTK_CHECKBUTTON_ASYNCH_NAME         "checkbutton_asynch"
#define TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOPBACK_NAME       "checkbutton_loopback"
#define TEST_I_STREAM_UI_GTK_CHECKBUTTON_LOOP_NAME           "checkbutton_loop"
#define TEST_I_STREAM_UI_GTK_DIALOG_ABOUT_NAME               "dialog_about"
#define TEST_I_STREAM_UI_GTK_DIALOG_MAIN_NAME                "dialog_main"
#define TEST_I_STREAM_UI_GTK_ENTRY_DESTINATION_NAME          "entry_destination"
#define TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_OPEN_NAME     "filechooserbutton_source"
#define TEST_I_STREAM_UI_GTK_FILECHOOSERDIALOG_OPEN_NAME     "dialog_filechooser"
#define TEST_I_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME     "filechooserbutton_destination"
#define TEST_I_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME     "dialog_filechooser"
#define TEST_I_STREAM_UI_GTK_HBOX_OPTIONS_NAME               "hbox_options"
#define TEST_I_STREAM_UI_GTK_IMAGE_CONNECT_NAME              "image_connect"
#define TEST_I_STREAM_UI_GTK_IMAGE_DISCONNECT_NAME           "image_disconnect"
#define TEST_I_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION      "Monospace 8"
#define TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE            "#FFFFFF" // white
#define TEST_I_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT            "#000000" // black
#define TEST_I_STREAM_UI_GTK_PROGRESSBAR_NAME                "progressbar"
#define TEST_I_STREAM_UI_GTK_RADIOBUTTON_TCP_NAME            "radiobutton_tcp"
#define TEST_I_STREAM_UI_GTK_RADIOBUTTON_UDP_NAME            "radiobutton_udp"
#define TEST_I_STREAM_UI_GTK_SCROLLEDWINDOW_NAME             "scrolledwindow"
//#define TEST_I_STREAM_UI_GTK_SCROLLMARK_NAME                 "insert"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME      "spinbutton_buffersize"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_CONNECTIONS_NAME     "spinbutton_connections"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_DATA_NAME            "spinbutton_data"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME    "spinbutton_data_messages"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME "spinbutton_session_messages"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_LOOP_NAME            "spinbutton_loop"
#define TEST_I_STREAM_UI_GTK_SPINBUTTON_PORT_NAME            "spinbutton_port"
#define TEST_I_STREAM_UI_GTK_STATUSBAR_NAME                  "statusbar"
#define TEST_I_STREAM_UI_GTK_TABLE_OPTIONS_NAME              "table_options"
#define TEST_I_STREAM_UI_GTK_TOGGLEACTION_LISTEN_NAME        "toggleaction_listen"
#define TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LISTEN_NAME        "listen"
#define TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LABEL_LISTEN_STRING    "Listen"
#define TEST_I_STREAM_UI_GTK_TOGGLEBUTTON_LABEL_LISTENING_STRING "Listening"
#define TEST_I_STREAM_UI_GTK_TEXTVIEW_NAME                   "textview"

// GTK progress/status bar
#define TEST_I_STREAM_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL     27 // ms (?)
#define TEST_I_STREAM_UI_GTK_STATUSBAR_CONTEXT_DESCRIPTION   "Stream::main"

//---------------------------------------

#endif
