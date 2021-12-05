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

#define TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_TYPE           STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_INVALID // --> off
#define TEST_U_STREAM_AUDIOEFFECT_NOISE_DEFAULT_FREQUENCY_D    440.0 // Hz
#define TEST_U_STREAM_AUDIOEFFECT_DEFAULT_OUTPUT_FILE          "output.wav"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#define TEST_U_STREAM_AUDIOEFFECT_GTK2_GLADE_FILE              "audioeffect.gtk2"
#define TEST_U_STREAM_AUDIOEFFECT_GTK3_GLADE_FILE              "audioeffect.gtk3"
#elif defined (WXWIDGETS_USE)
#define TEST_U_STREAM_AUDIOEFFECT_WXWIDGETS_XRC_FILE           "audioeffect.xrc"
#endif
#endif // GUI_SUPPORT

// defaults
// *TODO*: remove these ASAP
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_DEFAULT_TEXTURE_FILE  "image.png"
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_CAMERA_DEFAULT_ZOOM_F 5.0F
// perspective(s)
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_PERSPECTIVE_FOVY_D    60.0
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_PERSPECTIVE_ZNEAR_D   1.0
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_PERSPECTIVE_ZFAR_D    100.0
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_ORTHO_ZNEAR_D         -1.0
#define TEST_U_STREAM_AUDIOEFFECT_OPENGL_ORTHO_ZFAR_D          1.0

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define TEST_U_STREAM_AUDIOEFFECT_SOX_DEFAULT_EFFECT_NAME      "echo"
#define TEST_U_STREAM_AUDIOEFFECT_SOX_HELP_SHELL_COMMAND       "sox -h"
#endif // ACE_WIN32 || ACE_WIN64

//----------------------------------------

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
// GTK widget names
#define TEST_U_STREAM_UI_GTK_BUTTON_CUT_NAME                   "button_cut"
#define TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME                "button_report"
#define TEST_U_STREAM_UI_GTK_BOX_DISPLAY_NAME                  "vbox_display"
#define TEST_U_STREAM_UI_GTK_BUTTONBOX_ACTIONS_NAME            "buttonbox_actions"
#define TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME                 "button_about"
#define TEST_U_STREAM_UI_GTK_BUTTON_PROPERTIES_NAME            "button_properties"
#define TEST_U_STREAM_UI_GTK_BUTTON_RESET_NAME                 "button_reset"
#define TEST_U_STREAM_UI_GTK_BUTTON_SELECT_NAME                "button_select"
#define TEST_U_STREAM_UI_GTK_BUTTON_SETTINGS_NAME              "button_settings"
#define TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME                  "button_quit"
#define TEST_U_STREAM_UI_GTK_CHECKBUTTON_EFFECT_NAME           "checkbutton_effect"
#define TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME             "checkbutton_save"
#define TEST_U_STREAM_UI_GTK_CHECKBUTTON_SINUS_NAME            "checkbutton_sinus"
#define TEST_U_STREAM_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME    "checkbutton_visualization"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME            "combobox_channels"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_EFFECT_NAME              "combobox_effect"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME              "combobox_format"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME           "combobox_frequency"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME          "combobox_resolution"
#define TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME              "combobox_source"
#define TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME                 "dialog_about"
#define TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME                  "dialog_main"
#define TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME                  "drawingarea_2d"
#if defined (GTKGL_SUPPORT)
#define TEST_U_STREAM_UI_GTK_DRAWINGAREA_3D_NAME               "drawingarea_3d"
#endif // GTKGL_SUPPORT
#define TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME       "filechooserbutton_destination"
#define TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME       "filechooserdialog_destination"
#define TEST_U_STREAM_UI_GTK_FILEFILTER_WAV_NAME               "filefilter_wav"
#define TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME          "frame_configuration"
#define TEST_U_STREAM_UI_GTK_FRAME_EFFECT_NAME                 "frame_effect"
#define TEST_U_STREAM_UI_GTK_FRAME_FORMAT_NAME                 "frame_format"
#define TEST_U_STREAM_UI_GTK_FRAME_SAVE_NAME                   "frame_save"
#define TEST_U_STREAM_UI_GTK_FRAME_SINUS_NAME                  "frame_sinus"
#define TEST_U_STREAM_UI_GTK_BOX_EFFECT_NAME                   "box_effect_2"
#define TEST_U_STREAM_UI_GTK_BOX_OPTIONS_NAME                  "box_options"
#define TEST_U_STREAM_UI_GTK_BOX_SINUS_NAME                    "box_sinus_2"
#define TEST_U_STREAM_UI_GTK_BOX_VISUALIZATION_NAME            "box_visualization_2"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_EFFECT_NAME             "liststore_effect"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME             "liststore_format"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME          "liststore_frequency"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME           "liststore_channels"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME         "liststore_resolution"
#define TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME             "liststore_source"
//#define TEST_U_STREAM_UI_GTK_RADIOACTION_OSCILLOSCOPE_NAME   "radioaction_oscilloscope"
//#define TEST_U_STREAM_UI_GTK_RADIOACTION_SPECTRUM_NAME       "radioaction_spectrum"
#define TEST_U_STREAM_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME     "radiobutton_oscilloscope"
#define TEST_U_STREAM_UI_GTK_RADIOBUTTON_SPECTRUM_NAME         "radiobutton_spectrum"
#define TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME                  "progressbar"
#define TEST_U_STREAM_UI_GTK_SCALE_SINUS_FREQUENCY_NAME        "scale_sinus_frequency"
#define TEST_U_STREAM_UI_GTK_HSCALE_BOOST_NAME                 "hscale_boost"
#define TEST_U_STREAM_UI_GTK_HSCALE_VOLUME_NAME                "hscale_volume"
#define TEST_U_STREAM_UI_GTK_SIZEGROUP_OPTIONS_NAME            "sizegroup_options"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME        "spinbutton_buffersize"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME    "spinbutton_captured_frames"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME              "spinbutton_data"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME      "spinbutton_data_messages"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME     "spinbutton_dropped_frames"
#define TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME   "spinbutton_session_messages"
#define TEST_U_STREAM_UI_GTK_STATUSBAR_NAME                    "statusbar"
#define TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME          "togglebutton_record"
#define TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_3D_NAME              "togglebutton_3d"
#define TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_MUTE_NAME            "togglebutton_mute"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

//----------------------------------------

#endif
