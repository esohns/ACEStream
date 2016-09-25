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

#ifndef TEST_U_STREAM_AUDIOEFFECT_DEFINES_H
#define TEST_U_STREAM_AUDIOEFFECT_DEFINES_H

#include "ace/config-lite.h"

#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_GLADE_FILE         "audioeffect.glade"
#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE        "output.wav"
#define TEST_U_STREAM_AUDIOEFFECT_THREAD_NAME                "stream processor"
#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_IMAGE_FILE         "image.png"

#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_BUFFER_SIZE        4096 // bytes
#define TEST_U_STREAM_AUDIOEFFECT_MAX_MESSAGES               0  // 0 --> no limits

#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS              false
#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS_FREQUENCY    440.0 // Hz

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define TEST_U_STREAM_AUDIOEFFECT_SOX_DEFAULT_EFFECT_NAME    "echo"
#define TEST_U_STREAM_AUDIOEFFECT_SOX_HELP_SHELL_COMMAND     "sox -h"
#endif

//----------------------------------------

// GTK info widget updates
#define TEST_U_STREAM_UI_GTK_SIGNAL_TOOLTIP_DELAY            100 // ms
#define TEST_U_STREAM_UI_GTK_EVENT_RESOLUTION                200 // ms --> 5 FPS

// GTK progress/status-bar
//#define TEST_U_STREAM_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL     27 // ms (?)
#define TEST_U_STREAM_UI_GTK_STATUSBAR_CONTEXT_DESCRIPTION   "Stream::main"

//----------------------------------------

// GTK widget names
#define TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME                 "action_cut"
#define TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME              "action_report"
//#define TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME                 "scrolledwindow_vadjustment"
#define TEST_U_STREAM_UI_GTK_BUTTONBOX_ACTIONS_NAME          "vbuttonbox"
#define TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME               "about"
//#define TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME               "clear"
#define TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME              "report"
#define TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME                "quit"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME          "combobox_channels"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_EFFECT_NAME            "combobox_effect"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME            "combobox_format"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME         "combobox_frequency"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME        "combobox_resolution"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME            "combobox_source"
#define TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME               "dialog_about"
#define TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME                "dialog_main"
#define TEST_U_STREAM_UI_GTK_DRAWINGAREA_SIGNAL_NAME         "drawingarea_signal"
#define TEST_U_STREAM_UI_GTK_DRAWINGAREA_OPENGL_NAME         "drawingarea_opengl"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME     "filechooserbutton_destination"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME     "filechooserdialog_destination"
#define TEST_U_STREAM_UI_GTK_FILEFILTER_AVI_NAME             "filefilter_avi"
#define TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME        "frame_configuration"
#define TEST_U_STREAM_UI_GTK_FRAME_DESTINATION_NAME          "frame_destination"
#define TEST_U_STREAM_UI_GTK_FRAME_EFFECT_NAME               "frame_effect"
#define TEST_U_STREAM_UI_GTK_FRAME_OPTIONS_NAME              "frame_options"
#define TEST_U_STREAM_UI_GTK_FRAME_SINUS_NAME                "frame_sinus"
#define TEST_U_STREAM_UI_GTK_FRAME_VISUALIZATION_NAME        "frame_visualization"
#define TEST_U_STREAM_UI_GTK_GLAREA_3D_NAME                  "glarea_3d"
#define TEST_U_STREAM_UI_GTK_HBOX_OPTIONS_NAME               "hbox_options"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_EFFECT_NAME           "liststore_effect"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME           "liststore_format"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME        "liststore_frequency"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME         "liststore_channels"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME       "liststore_resolution"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME           "liststore_source"
//#define TEST_U_STREAM_UI_GTK_RADIOACTION_OSCILLOSCOPE_NAME   "radioaction_oscilloscope"
//#define TEST_U_STREAM_UI_GTK_RADIOACTION_SPECTRUM_NAME       "radioaction_spectrum"
#define TEST_U_STREAM_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME   "radiobutton_oscilloscope"
#define TEST_U_STREAM_UI_GTK_RADIOBUTTON_SPECTRUM_NAME       "radiobutton_spectrum"
//#define TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION      "Monospace 8"
//#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE            "#FFFFFF" // white
//#define TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT            "#000000" // green
//#define TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME                "progressbar"
#define TEST_U_STREAM_UI_GTK_SCALE_FREQUENCY_NAME            "scale_frequency"
//#define TEST_U_STREAM_UI_GTK_SCROLLEDWINDOW_NAME             "scrolledwindow"
//#define TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME                 "insert"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME      "spinbutton_buffersize"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME  "spinbutton_captured_frames"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME            "spinbutton_data"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME    "spinbutton_data_messages"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME   "spinbutton_dropped_frames"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME "spinbutton_session_messages"
#define TEST_U_STREAM_UI_GTK_STATUSBAR_NAME                  "statusbar"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_EFFECT_NAME        "toggleaction_effect"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_OPENGL_NAME        "toggleaction_opengl"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME        "toggleaction_record"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_SAVE_NAME          "toggleaction_save"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_SINUS_NAME         "toggleaction_sinus"
#define TEST_U_STREAM_UI_GTK_TOGGLEACTION_VISUALIZATION_NAME "toggleaction_visualization"
#define TEST_U_STREAM_UI_GTK_VBOX_DISPLAY_NAME               "vbox_display"
//#define TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME                   "textview"

//----------------------------------------

#endif
