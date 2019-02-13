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

#ifndef TEST_U_STREAM_CAMSAVE_DEFINES_H
#define TEST_U_STREAM_CAMSAVE_DEFINES_H

#define TEST_U_STREAM_CAMSAVE_DEFAULT_OUTPUT_FILE             "output.avi"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTK3_USE)
#define TEST_U_STREAM_CAMSAVE_UI_DEFINITION_FILE              "camsave.gtk3"
#elif defined (GTK2_USE)
#define TEST_U_STREAM_CAMSAVE_UI_DEFINITION_FILE              "camsave.gtk2"
#endif
#elif defined (WXWIDGETS_USE)
#define TEST_U_STREAM_CAMSAVE_UI_DEFINITION_FILE              "camsave.xrc"
#endif // WXWIDGETS_USE
#define TEST_U_STREAM_CAMSAVE_UI_CSS_FILE                     "camsave.css"

//---------------------------------------
#if defined (GTK_USE)
#define TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME                  "action_cut"
#define TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME               "action_report"
#define TEST_U_STREAM_UI_GTK_ACTION_SNAPSHOT_NAME             "action_snapshot"
#define TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME                  "scrolledwindow_vadjustment"
#define TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME                "about"
#define TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME                "clear"
#define TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME                 "quit"
#define TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME            "checkbutton_save"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME             "combobox_format"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_RATE_NAME               "combobox_rate"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME         "combobox_resolution"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME             "combobox_source"
#define TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME                "dialog_about"
#define TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME                 "drawingarea"
#define TEST_U_STREAM_UI_GTK_ENTRY_SAVE_NAME                  "entry_save"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME      "filechooserbutton_save"
//#define TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME     "filechooserdialog_save"
#define TEST_U_STREAM_UI_GTK_FILEFILTER_AVI_NAME              "filefilter_avi"
#define TEST_U_STREAM_UI_GTK_FRAME_DISPLAY_NAME               "frame_display"
#define TEST_U_STREAM_UI_GTK_FRAME_OPTIONS_NAME               "frame_options"
#define TEST_U_STREAM_UI_GTK_FRAME_SAVE_NAME                  "frame_save"
#define TEST_U_STREAM_UI_GTK_FRAME_SOURCE_NAME                "frame_source"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME            "liststore_format"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_RATE_NAME              "liststore_rate"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME        "liststore_resolution"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME            "liststore_source"
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION       "Monospace 8"
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE             "#FFFFFF" // white
#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT             "#000000" // green
#define TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
#define TEST_U_STREAM_UI_GTK_SCROLLEDWINDOW_NAME              "scrolledwindow"
#define TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME                  "insert"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME       "spinbutton_buffersize"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME   "spinbutton_captured_frames"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME             "spinbutton_data"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME     "spinbutton_data_messages"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME    "spinbutton_dropped_frames"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME  "spinbutton_session_messages"
#define TEST_U_STREAM_UI_GTK_STATUSBAR_NAME                   "statusbar"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_FULLSCREEN_NAME     "toggleaction_fullscreen"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME         "toggleaction_record"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_SAVE_NAME           "toggleaction_save"
#define TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME                    "textview"
#define TEST_U_STREAM_UI_GTK_WINDOW_FULLSCREEN                "window_fullscreen"
#elif defined (WXWIDGETS_USE)
#define TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME       "dialog_main"
#define TEST_U_STREAM_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME "wxDialog"
#endif
#endif // GUI_SUPPORT

#endif
