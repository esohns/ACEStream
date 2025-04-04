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

#ifndef TEST_U_STREAM_FILECOPY_DEFINES_H
#define TEST_U_STREAM_FILECOPY_DEFINES_H

#if defined (GTK2_USE)
#define TEST_U_STREAM_FILECOPY_DEFAULT_GLADE_FILE            "filecopy.gtk2"
#elif defined (GTK3_USE) || defined (GTK4_USE)
#define TEST_U_STREAM_FILECOPY_DEFAULT_GLADE_FILE            "filecopy.gtk3"
#else
#define TEST_U_STREAM_FILECOPY_DEFAULT_GLADE_FILE            ""
#endif // GTK2_USE || GTK3_USE || GTK4_USE
#define TEST_U_STREAM_FILECOPY_DEFAULT_OUTPUT_FILE           "output.tmp"

#define TEST_U_STREAM_FILECOPY_DEFAULT_BUFFER_SIZE           4096 // bytes

//---------------------------------------

//#define TEST_U_STREAM_UI_GTKEVENT_RESOLUTION                 200 // ms --> 5 FPS
#define TEST_U_STREAM_UI_GTK_ACTION_START_NAME               "action_start"
#define TEST_U_STREAM_UI_GTK_ACTION_STOP_NAME                "action_stop"
#define TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME                 "scrolledwindow_vadjustment"
#define TEST_U_STREAM_UI_GTK_BUTTONBOX_ACTIONS_NAME          "vbuttonbox"
#define TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME               "about"
#define TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME               "clear"
#define TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME              "report"
//#define TEST_U_STREAM_UI_GTK_BUTTON_START_NAME               "start"
//#define TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME                "stop"
#define TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME                "quit"
#define TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME               "dialog_about"
#define TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME                "dialog_main"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_OPEN_NAME     "filechooserbutton_source"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME     "filechooserbutton_destination"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_OPEN_NAME     "dialog_filechooser"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME     "dialog_filesaver"
#define TEST_U_STREAM_UI_GTK_HBOX_OPTIONS_NAME               "hbox_options"
//#define TEST_U_STREAM_UI_GTK_IMAGE_PLAY_NAME                 "image_play"
//#define TEST_U_STREAM_UI_GTK_IMAGE_PAUSE_NAME                "image_pause"
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION      "Monospace 8"
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE            "#FFFFFF" // white
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT            "#000000" // green
#define TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME                "progressbar"
#define TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME                 "insert"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME      "spinbutton_buffersize"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME    "spinbutton_data_messages"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME "spinbutton_session_messages"
#define TEST_U_STREAM_UI_GTK_STATUSBAR_NAME                  "statusbar"
#define TEST_U_STREAM_UI_GTK_TABLE_OPTIONS_NAME              "table_options"
#define TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME                   "textview"

//---------------------------------------

#endif
