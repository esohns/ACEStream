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

#ifndef TEST_I_AVSAVE_DEFINES_H
#define TEST_I_AVSAVE_DEFINES_H

#include "gtk/gtk.h"

#define TEST_I_DEFAULT_OUTPUT_FILE             "output.avi"

#if defined (GTK_USE)
#undef TEST_I_UI_DEFINITION_FILE
#if defined (GTK3_USE)
#define TEST_I_UI_DEFINITION_FILE              "avsave.gtk3"
#elif defined (GTK2_USE)
#define TEST_I_UI_DEFINITION_FILE              "avsave.gtk2"
#endif // GTK3_USE || GTK2_USE
#elif defined (QT_USE)
#define TEST_I_UI_DEFINITION_FILE              "mainwindow.ui"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_DEFINITION_FILE              "avsave.xrc"
#else
#define TEST_I_UI_DEFINITION_FILE              ""
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#undef TEST_I_UI_CSS_FILE
#define TEST_I_UI_CSS_FILE                     "avsave.css"

//---------------------------------------
#if defined (GTK_USE)
#define TEST_I_UI_GTK_ADJUSTMENT_NAME                  "scrolledwindow_vadjustment"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                "button_about"
//#define TEST_I_UI_GTK_BUTTON_CLEAR_NAME                "clear"
#define TEST_I_UI_GTK_BUTTON_CUT_NAME                  "button_cut"
//#define TEST_I_UI_GTK_BUTTON_REPORT_NAME               "button_report"
#define TEST_I_UI_GTK_BUTTON_SNAPSHOT_NAME             "button_snapshot"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                 "button_quit"
#define TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME            "combobox_adapter"
#define TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME            "combobox_display"
#define TEST_I_UI_GTK_COMBOBOX_VIDEO_FORMAT_NAME       "combobox_video_format"
#define TEST_I_UI_GTK_COMBOBOX_VIDEO_FRAMERATE_NAME    "combobox_video_framerate"
#define TEST_I_UI_GTK_COMBOBOX_VIDEO_RESOLUTION_NAME   "combobox_video_resolution"
#define TEST_I_UI_GTK_COMBOBOX_VIDEO_SOURCE_NAME       "combobox_video_source"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                "aboutdialog"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_AUDIO_NAME           "drawingarea_audio"
#define TEST_I_UI_GTK_DRAWINGAREA_VIDEO_NAME           "drawingarea_video"
#define TEST_I_UI_GTK_ENTRY_SAVE_NAME                  "entry_save"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME      "filechooserbutton_save"
//#define TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME     "filechooserdialog_save"
#define TEST_I_UI_GTK_FILEFILTER_AVI_NAME              "filefilter_avi"
#define TEST_I_UI_GTK_FRAME_DISPLAY_NAME               "frame_display"
#define TEST_I_UI_GTK_FRAME_SAVE_NAME                  "frame_save"
#define TEST_I_UI_GTK_FRAME_VIDEO_NAME                 "frame_video"
#define TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME           "liststore_adapter"
#define TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME           "liststore_display"
#define TEST_I_UI_GTK_LISTSTORE_VIDEO_FORMAT_NAME      "liststore_video_format"
#define TEST_I_UI_GTK_LISTSTORE_VIDEO_FRAMERATE_NAME   "liststore_video_framerate"
#define TEST_I_UI_GTK_LISTSTORE_VIDEO_RESOLUTION_NAME  "liststore_video_resolution"
#define TEST_I_UI_GTK_LISTSTORE_VIDEO_SOURCE_NAME      "liststore_video_source"
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
//#define TEST_I_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME       "spinbutton_buffersize"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME             "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_AUDIO_MESSAGES_NAME   "spinbutton_audio_messages"
#define TEST_I_UI_GTK_SPINBUTTON_VIDEO_MESSAGES_NAME   "spinbutton_video_messages"
#define TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME  "spinbutton_session_messages"
#define TEST_I_UI_GTK_STATUSBAR_NAME                   "statusbar"
#define TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME        "togglebutton_display"
#define TEST_I_UI_GTK_TOGGLEBUTTON_FULLSCREEN_NAME     "togglebutton_fullscreen"
#define TEST_I_UI_GTK_TOGGLEBUTTON_RECORD_NAME         "togglebutton_record"
#define TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME           "togglebutton_save"
#define TEST_I_UI_GTK_WINDOW_FULLSCREEN                "window_fullscreen"
#elif defined (WXWIDGETS_USE)
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_NAME       "dialog_main"
#define TEST_I_UI_WXWIDGETS_TOPLEVEL_WIDGET_CLASS_NAME "wxDialog"
#endif

#endif
