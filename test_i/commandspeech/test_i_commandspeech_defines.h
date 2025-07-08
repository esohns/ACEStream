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

#ifndef TEST_I_COMMANDSPEECH_DEFINES_H
#define TEST_I_COMMANDSPEECH_DEFINES_H

#if defined (FESTIVAL_SUPPORT) || defined (FLITE_SUPPORT)
#define TEST_I_DEFAULT_VOICE_DIRECTORY                 ""
#define TEST_I_DEFAULT_VOICE                           "cmu_us_slt"
#endif // FESTIVAL_SUPPORT || FLITE_SUPPORT
#define TEST_I_DEFAULT_OUTPUT_FILE                     "output.wav"

//----------------------------------------

#if defined (GTK_SUPPORT)
#define TEST_I_UI_GTK_ACTION_RECORD_NAME               "action_record"
#define TEST_I_UI_GTK_SCROLLEDWINDOW_ADJUSTMENT_V_NAME "adjustment_sw_v"
#define TEST_I_UI_GTK_BOX_DISPLAY_2_NAME               "box_display_2"
#define TEST_I_UI_GTK_BOX_SAVE_2_NAME                  "box_save_2"
#define TEST_I_UI_GTK_BOX_TARGET_2_NAME                "box_target_2"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                "button_about"
#define TEST_I_UI_GTK_BUTTON_HW_SETTINGS_NAME          "button_hw_settings"
#define TEST_I_UI_GTK_BUTTON_REPORT_NAME               "button_report"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                 "button_quit"
#define TEST_I_UI_GTK_BUTTON_VOICE_RESET_NAME          "button_voice_reset"
#define TEST_I_UI_GTK_BUTTONBOX_DISPLAY_NAME           "buttonbox_display"
#define TEST_I_UI_GTK_COMBOBOX_ADAPTER_NAME            "combobox_adapter"
#define TEST_I_UI_GTK_COMBOBOX_DISPLAY_NAME            "combobox_display"
#define TEST_I_UI_GTK_COMBOBOX_TARGET_NAME             "combobox_target"
#define TEST_I_UI_GTK_COMBOBOX_VOICE_NAME              "combobox_voice"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                "dialog_about"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_NAME                 "drawingarea"
#define TEST_I_UI_GTK_ENTRY_SAVE_NAME                  "entry_save"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME      "filechooserbutton_save"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_VOICE_NAME     "filechooserbutton_voice"
#define TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME      "filechooserdialog_save"
#define TEST_I_UI_GTK_FILECHOOSERDIALOG_VOICE_NAME     "filechooserdialog_voice"
//#define TEST_I_UI_GTK_FILEFILTER_WAV_NAME              "filefilter_wav"
#define TEST_I_UI_GTK_FRAME_SAVE_NAME                  "frame_save"
#define TEST_I_UI_GTK_FRAME_VOICE_NAME                 "frame_voice"
#define TEST_I_UI_GTK_HSCALE_VOLUME_NAME               "hscale_volume"
#define TEST_I_UI_GTK_LISTSTORE_ADAPTER_NAME           "liststore_adapter"
#define TEST_I_UI_GTK_LISTSTORE_DISPLAY_NAME           "liststore_display"
#define TEST_I_UI_GTK_LISTSTORE_TARGET_NAME            "liststore_target"
#define TEST_I_UI_GTK_LISTSTORE_VOICE_NAME             "liststore_voice"
#define TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION       "Monospace 8"
#define TEST_I_UI_GTK_PANGO_LOG_COLOR_BASE             "#FFFFFF" // white
#define TEST_I_UI_GTK_PANGO_LOG_COLOR_TEXT             "#000000" // black
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
#define TEST_I_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME    "radiobutton_oscilloscope"
#define TEST_I_UI_GTK_RADIOBUTTON_SPECTRUM_NAME        "radiobutton_spectrum"
#define TEST_I_UI_GTK_SPINBUTTON_CAPTURED_FRAMES_NAME  "spinbutton_captured_frames"
#define TEST_I_UI_GTK_SPINBUTTON_DROPPED_FRAMES_NAME   "spinbutton_dropped_frames"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME             "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME     "spinbutton_data_messages"
#define TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME  "spinbutton_session_messages"
#define TEST_I_UI_GTK_SCROLLEDWINDOW_NAME              "scrolledwindow"
#define TEST_I_UI_GTK_TEXTBUFFER_NAME                  "textbuffer"
#define TEST_I_UI_GTK_TEXTMARK_BEGIN_NAME              "begin"
#define TEST_I_UI_GTK_TEXTVIEW_NAME                    "textview"
#define TEST_I_UI_GTK_TOGGLEBUTTON_DISPLAY_NAME        "togglebutton_display"
#define TEST_I_UI_GTK_TOGGLEBUTTON_PLAYBACK_NAME       "togglebutton_playback"
#define TEST_I_UI_GTK_TOGGLEBUTTON_RECORD_NAME         "togglebutton_record"
#define TEST_I_UI_GTK_TOGGLEBUTTON_SAVE_NAME           "togglebutton_save"
#define TEST_I_UI_GTK_SCROLLMARK_NAME                  "insert"
#define TEST_I_UI_GTK_STATUSBAR_NAME                   "statusbar"
#define TEST_I_UI_GTK_TEXTBUFFER_NAME                  "textbuffer"
#define TEST_I_UI_GTK_TEXTVIEW_NAME                    "textview"
#endif // GTK_SUPPORT

#endif
