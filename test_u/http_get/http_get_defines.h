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

#ifndef HTTP_GET_DEFINES_H
#define HTTP_GET_DEFINES_H

#define HTTP_GET_DEFAULT_OUTPUT_FILE "output.bin"
#define HTTP_GET_DEFAULT_URI         "https://www.mirc.com/servers.ini"

//#define HTTP_GET_DEFAULT_BUFFER_SIZE 16384 // bytes

// *** Gtk UI-related ***
#define HTTPGET_UI_WIDGET_NAME_BUTTON_ABOUT                "button_about"
#define HTTPGET_UI_WIDGET_NAME_BUTTON_CANCEL               "button_cancel"
#define HTTPGET_UI_WIDGET_NAME_BUTTON_CLEAR                "button_clear"
#define HTTPGET_UI_WIDGET_NAME_BUTTON_EXECUTE              "button_execute"
#define HTTPGET_UI_WIDGET_NAME_BUTTON_QUIT                 "button_quit"
#define HTTPGET_UI_WIDGET_NAME_CHECKBUTTON_ASYNCH          "checkbutton_asynch"
#define HTTPGET_UI_WIDGET_NAME_CHECKBUTTON_SAVE            "checkbutton_save"
#define HTTPGET_UI_WIDGET_NAME_DIALOG_ABOUT                "dialog_about"
#define HTTPGET_UI_WIDGET_NAME_DIALOG_MAIN                 "dialog_main"
#define HTTPGET_UI_WIDGET_NAME_ENTRY_URL                   "entry_url"
#define HTTPGET_UI_WIDGET_NAME_FILECHOOSERBUTTON_SAVE      "filechooserbutton_save"
#define HTTPGET_UI_WIDGET_NAME_FRAME_OPTIONS               "frame_options"
#define HTTPGET_UI_WIDGET_NAME_FRAME_SAVE                  "frame_save"
#define HTTPGET_UI_WIDGET_NAME_FRAME_URL                   "frame_url"
#define HTTPGET_UI_WIDGET_NAME_PROGRESSBAR                 "progressbar"
#define HTTPGET_UI_WIDGET_NAME_SPINBUTTON_BUFFER           "spinbutton_buffer"
#define HTTPGET_UI_WIDGET_NAME_SPINBUTTON_CONNECTIONS      "spinbutton_connections"
#define HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATA             "spinbutton_data"
#define HTTPGET_UI_WIDGET_NAME_SPINBUTTON_DATAMESSAGES     "spinbutton_data_messages"
#define HTTPGET_UI_WIDGET_NAME_SPINBUTTON_SESSIONMESSAGES  "spinbutton_session_messages"
#define HTTPGET_UI_WIDGET_NAME_STATUSBAR                   "statusbar"
//#define HTTPGET_UI_WIDGET_NAME_TEXTVIEW_LOG                "textview_log"

#define HTTPGET_UI_GTK_PANGO_LOG_FONT_DESCRIPTION          "Monospace 8"
#define HTTPGET_UI_GTK_PANGO_LOG_COLOR_BASE                "#FFFFFF" // white
#define HTTPGET_UI_GTK_PANGO_LOG_COLOR_TEXT                "#000000" // black

#define HTTPGET_UI_STATUSBAR_CONTEXT_DATA                  "data"
#define HTTPGET_UI_STATUSBAR_CONTEXT_INFORMATION           "information"

#define HTTPGET_UI_DEFINITION_FILE_NAME                    "httpget.glade"
#define HTTPGET_UI_PROCESSING_THREAD_NAME                  "stream processor"

#endif
