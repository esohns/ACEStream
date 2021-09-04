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

#ifndef TEST_I_SMTPSEND_DEFINES_H
#define TEST_I_SMTPSEND_DEFINES_H

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTK3_USE)
#define TEST_I_UI_DEFINITION_FILE              "test_i.gtk3"
#elif defined (GTK2_USE)
#define TEST_I_UI_DEFINITION_FILE              "test_i.gtk2"
#endif
#elif defined (QT_USE)
#define TEST_I_UI_DEFINITION_FILE              "mainwindow.ui"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_DEFINITION_FILE              "test_i.xrc"
#else
#define TEST_I_UI_DEFINITION_FILE              ""
#endif // WXWIDGETS_USE
#define TEST_I_UI_CSS_FILE                     "test_i.css"

//---------------------------------------
#if defined (GTK_USE)
#define TEST_I_UI_GTK_ACTION_SEND_NAME                 "action_send"
#define TEST_I_UI_GTK_SCROLLEDWINDOW_ADJUSTMENT_V_NAME "adjustment_sw_v"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                "about"
#define TEST_I_UI_GTK_BUTTON_SEND_NAME                 "button_send"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                 "quit"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                "dialog_about"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_I_UI_GTK_ENTRY_FROM_NAME                  "entry_from"
#define TEST_I_UI_GTK_ENTRY_SERVER_NAME                "entry_server"
#define TEST_I_UI_GTK_ENTRY_TO_NAME                    "entry_to"
#define TEST_I_UI_GTK_ENTRY_USERNAME_NAME              "entry_username"
#define TEST_I_UI_GTK_ENTRY_PASSWORD_NAME              "entry_password"
//#define TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME         "frame_configuration"
#define TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION       "Monospace 8"
#define TEST_I_UI_GTK_PANGO_LOG_COLOR_BASE             "#FFFFFF" // white
#define TEST_I_UI_GTK_PANGO_LOG_COLOR_TEXT             "#000000" // black
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME             "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME     "spinbutton_data_messages"
#define TEST_I_UI_GTK_SPINBUTTON_PORT_NAME             "spinbutton_port"
#define TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME  "spinbutton_session_messages"
#define TEST_I_UI_GTK_SCROLLEDWINDOW_NAME              "scrolledwindow"
#define TEST_I_UI_GTK_SCROLLMARK_NAME                  "insert"
#define TEST_I_UI_GTK_STATUSBAR_NAME                   "statusbar"
#define TEST_I_UI_GTK_TEXTBUFFER_NAME                  "textbuffer"
#define TEST_I_UI_GTK_TEXTVIEW_NAME                    "textview"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME       "dialog_main"
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME "wxDialog"
#endif
#endif // GUI_SUPPORT

#endif
