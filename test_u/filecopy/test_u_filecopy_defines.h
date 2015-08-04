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

#define TEST_U_STREAM_FILECOPY_UI                        "filecopy.glade"
#define TEST_U_STREAM_FILECOPY_DEFAULT_OUTPUT_FILE       "output.tmp"

#define TEST_U_STREAM_FILECOPY_MAX_MESSAGES              0  // 0 --> no limits

//---------------------------------------

#define TEST_U_STREAM_UI_GTKEVENT_RESOLUTION             200 // ms --> 5 FPS
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_NUMMESSAGES_NAME "spinbutton_messages"
#define TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME               "textview"
#define TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME             "scrolledwindow_vadjustment"
#define TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME             "insert"
#define TEST_U_STREAM_UI_GTK_BUTTONBOX_ACTIONS_NAME      "vbuttonbox"
#define TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME           "about"
#define TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME           "clear"
#define TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME          "report"
#define TEST_U_STREAM_UI_GTK_BUTTON_START_NAME           "start"
#define TEST_U_STREAM_UI_GTK_BUTTON_STOP_NAME            "stop"
#define TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME            "quit"
#define TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME           "dialog_about"
#define TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME            "dialog_main"
#define TEST_U_STREAM_UI_GTK_IMAGE_START_NAME            "image_start"
#define TEST_U_STREAM_UI_GTK_IMAGE_STOP_NAME             "image_stop"
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION  "Monospace 8"
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE        "#FFFFFF" // white
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT        "#000000" // green

#endif
