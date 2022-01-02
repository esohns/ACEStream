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

#ifndef TEST_I_SPEECHCOMMAND_DEFINES_H
#define TEST_I_SPEECHCOMMAND_DEFINES_H

#define TEST_I_DEFAULT_MODEL_FILE                      "deepspeech-0.9.3-models.pbmm"
#define TEST_I_DEFAULT_SCORER_FILE                     "deepspeech-0.9.3-models.scorer"
#define TEST_I_DEFAULT_OUTPUT_FILE                     "output.wav"

//---------------------------------------

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#define TEST_I_UI_GTK_ACTION_RECORD_NAME               "action_record"
#define TEST_I_UI_GTK_SCROLLEDWINDOW_ADJUSTMENT_V_NAME "adjustment_sw_v"
#define TEST_I_UI_GTK_BUTTON_ABOUT_NAME                "about"
#define TEST_I_UI_GTK_BUTTON_DEVICE_SETTINGS_NAME      "button_device_settings"
#define TEST_I_UI_GTK_BUTTON_QUIT_NAME                 "quit"
#define TEST_I_UI_GTK_CHECKBUTTON_SAVE_NAME            "checkbutton_save"
#define TEST_I_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME   "checkbutton_visualization"
#define TEST_I_UI_GTK_COMBOBOX_DEVICE_NAME             "combobox_device"
#define TEST_I_UI_GTK_DIALOG_ABOUT_NAME                "dialog_about"
#define TEST_I_UI_GTK_DIALOG_MAIN_NAME                 "dialog_main"
#define TEST_I_UI_GTK_DRAWINGAREA_NAME                 "drawingarea"
#define TEST_I_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME      "filechooserbutton_save"
#define TEST_I_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME      "filechooserdialog_save"
#define TEST_I_UI_GTK_FILEFILTER_WAV_NAME              "filefilter_wav"
#define TEST_I_UI_GTK_FRAME_SAVE_NAME                  "frame_save"
#define TEST_I_UI_GTK_HSCALE_VOLUME_NAME               "hscale_volume"
#define TEST_I_UI_GTK_LISTSTORE_DEVICE_NAME            "liststore_device"
//#define TEST_I_UI_GTK_FRAME_CONFIGURATION_NAME         "frame_configuration"
#define TEST_I_UI_GTK_PANGO_LOG_FONT_DESCRIPTION       "Monospace 8"
#define TEST_I_UI_GTK_PANGO_LOG_COLOR_BASE             "#FFFFFF" // white
#define TEST_I_UI_GTK_PANGO_LOG_COLOR_TEXT             "#000000" // black
#define TEST_I_UI_GTK_PROGRESSBAR_NAME                 "progressbar"
#define TEST_I_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME    "radiobutton_oscilloscope"
#define TEST_I_UI_GTK_RADIOBUTTON_SPECTRUM_NAME        "radiobutton_spectrum"
#define TEST_I_UI_GTK_SPINBUTTON_DATA_NAME             "spinbutton_data"
#define TEST_I_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME     "spinbutton_data_messages"
#define TEST_I_UI_GTK_SPINBUTTON_PORT_NAME             "spinbutton_port"
#define TEST_I_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME  "spinbutton_session_messages"
#define TEST_I_UI_GTK_SCROLLEDWINDOW_NAME              "scrolledwindow"
#define TEST_I_UI_GTK_TOGGLEBUTTON_MUTE_NAME           "togglebutton_mute"
#define TEST_I_UI_GTK_TOGGLEBUTTON_RECORD_NAME         "togglebutton_record"
#define TEST_I_UI_GTK_SCROLLMARK_NAME                  "insert"
#define TEST_I_UI_GTK_STATUSBAR_NAME                   "statusbar"
#define TEST_I_UI_GTK_TEXTBUFFER_NAME                  "textbuffer"
#define TEST_I_UI_GTK_TEXTVIEW_NAME                    "textview"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#endif
