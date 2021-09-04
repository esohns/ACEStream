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

#ifndef TEST_I_IMAGESAVE_DEFINES_H
#define TEST_I_IMAGESAVE_DEFINES_H

#define TEST_I_DEFAULT_OUTPUT_FILE             "output.bmp"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTK3_USE)
#define TEST_I_UI_DEFINITION_FILE              "imagesave.gtk3"
#elif defined (GTK2_USE)
#define TEST_I_UI_DEFINITION_FILE              "imagesave.gtk2"
#endif
#elif defined (QT_USE)
#define TEST_I_UI_DEFINITION_FILE              "mainwindow.ui"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_DEFINITION_FILE              "imagesave.xrc"
#else
#define TEST_I_UI_DEFINITION_FILE              ""
#endif
#define TEST_I_UI_CSS_FILE                     "imagesave.css"

//---------------------------------------
#if defined (GTK_USE)
#define TEST_I_UI_GTK_ADJUSTMENT_NAME                  "scrolledwindow_vadjustment"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                "about"
#define TEST_I_UI_GTK_BUTTON_CLEAR_NAME                "clear"
#define TEST_I_UI_GTK_BUTTON_CUT_NAME                  "button_cut"
#define TEST_I_UI_GTK_BUTTON_REPORT_NAME               "button_report"
#define TEST_I_UI_GTK_BUTTON_SNAPSHOT_NAME             "button_snapshot"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                 "quit"
#define TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME            "combobox_adapter"
#define TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME            "combobox_display"
#define TEST_I_UI_GTK_COMBOBOX_FORMAT_NAME             "combobox_format"
#define TEST_I_UI_GTK_COMBOBOX_RATE_NAME               "combobox_rate"
#define TEST_I_UI_GTK_COMBOBOX_RESOLUTION_NAME         "combobox_resolution"
#define TEST_I_UI_GTK_COMBOBOX_SOURCE_NAME             "combobox_source"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                "dialog_about"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_NAME                 "drawingarea"
#define TEST_I_UI_GTK_ENTRY_SOURCE_NAME                "entry_source"
#define TEST_I_UI_GTK_ENTRY_TARGET_NAME                "entry_target"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SOURCE_NAME    "filechooserbutton_source"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_TARGET_NAME    "filechooserbutton_target"
//#define TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME     "filechooserdialog_source"
#define TEST_I_UI_GTK_FILEFILTER_MP4_NAME              "filefilter_mp4"
#define TEST_I_UI_GTK_FRAME_SOURCE_NAME                "frame_source"
#define TEST_I_UI_GTK_FRAME_TARGET_NAME                "frame_target"
//#define TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME           "liststore_adapter"
//#define TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME           "liststore_display"
#define TEST_I_UI_GTK_LISTSTORE_FORMAT_NAME            "liststore_format"
#define TEST_I_UI_GTK_LISTSTORE_RESOLUTION_NAME        "liststore_resolution"
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
#define TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME       "spinbutton_buffersize"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME             "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME     "spinbutton_data_messages"
#define TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME  "spinbutton_session_messages"
#define TEST_I_UI_GTK_STATUSBAR_NAME                   "statusbar"
//#define TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME        "togglebutton_display"
#define TEST_I_UI_GTK_TOGGLEBUTTON_PROCESS_NAME         "togglebutton_process"
//#define TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME           "togglebutton_save"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME       "dialog_main"
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME "wxDialog"
#endif
#endif // GUI_SUPPORT

#endif
