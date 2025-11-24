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

#ifndef TEST_U_MICVISUALIZE_DEFINES_H
#define TEST_U_MICVISUALIZE_DEFINES_H

#define TEST_U_NOISE_DEFAULT_TYPE           STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE
#define TEST_U_NOISE_DEFAULT_FREQUENCY_D    440.0 // Hz
#define TEST_U_DEFAULT_OUTPUT_FILE          "output.wav"

#if defined (GTK_USE)
#if defined (GTK2_USE)
#define TEST_U_GLADE_FILE                   "test_u.gtk2"
#elif defined (GTK3_USE)
#define TEST_U_GLADE_FILE                   "test_u.gtk3"
#endif // GTK2_USE || GTK3_USE
#elif defined (WXWIDGETS_USE)
#define TEST_U_WXWIDGETS_XRC_FILE           "test_u.xrc"
#endif

// defaults
// *TODO*: remove these ASAP
#define TEST_U_OPENGL_DEFAULT_TEXTURE_FILE  "image.png"
#define TEST_U_OPENGL_DEFAULT_FS_FILE       "fragment_shader.glsl"
#define TEST_U_OPENGL_DEFAULT_VS_FILE       "vertex_shader.glsl"

#define TEST_U_OPENGL_CAMERA_DEFAULT_ZOOM_F 5.0f
// perspective(s)
#define TEST_U_OPENGL_PERSPECTIVE_FOVY_D    60.0
#define TEST_U_OPENGL_PERSPECTIVE_ZNEAR_D   1.0
#define TEST_U_OPENGL_PERSPECTIVE_ZFAR_D    100.0
#define TEST_U_OPENGL_ORTHO_ZNEAR_D         -1.0
#define TEST_U_OPENGL_ORTHO_ZFAR_D          1.0

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define TEST_U_SOX_DEFAULT_EFFECT_NAME      "echo"
#define TEST_U_SOX_HELP_SHELL_COMMAND       "sox -h"
#endif // ACE_WIN32 || ACE_WIN64

//----------------------------------------

#if defined (GTK_SUPPORT)
// GTK widget names
#define TEST_U_UI_GTK_BUTTON_CUT_NAME                   "button_cut"
#define TEST_U_UI_GTK_BUTTON_REPORT_NAME                "button_report"
#define TEST_U_UI_GTK_BOX_DISPLAY_NAME                  "vbox_display"
#define TEST_U_UI_GTK_BOX_EFFECT_NAME                   "box_effect"
#define TEST_U_UI_GTK_BOX_EFFECT_2_NAME                 "box_effect_2"
#define TEST_U_UI_GTK_BUTTON_ABOUT_NAME                 "button_about"
#define TEST_U_UI_GTK_BUTTON_DEVICE_RESET_NAME          "button_device_reset"
#define TEST_U_UI_GTK_BUTTON_DEVICE_SETTINGS_NAME       "button_device_settings"
#define TEST_U_UI_GTK_BUTTON_PROPERTIES_NAME            "button_properties"
#define TEST_U_UI_GTK_BUTTON_SELECT_NAME                "button_select"
#define TEST_U_UI_GTK_BUTTON_QUIT_NAME                  "button_quit"
#define TEST_U_UI_GTK_CHECKBUTTON_EFFECT_NAME           "checkbutton_effect"
#define TEST_U_UI_GTK_CHECKBUTTON_SAVE_NAME             "checkbutton_save"
#define TEST_U_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME    "checkbutton_visualization"
#define TEST_U_UI_GTK_COMBOBOX_CHANNELS_NAME            "combobox_channels"
#define TEST_U_UI_GTK_COMBOBOX_DEVICE_NAME              "combobox_device"
#define TEST_U_UI_GTK_COMBOBOX_EFFECT_NAME              "combobox_effect"
#define TEST_U_UI_GTK_COMBOBOX_FORMAT_NAME              "combobox_format"
#define TEST_U_UI_GTK_COMBOBOX_FREQUENCY_NAME           "combobox_frequency"
#define TEST_U_UI_GTK_COMBOBOX_RESOLUTION_NAME          "combobox_resolution"
#define TEST_U_UI_GTK_COMBOBOX_SOURCE_NAME              "combobox_source"
#define TEST_U_UI_GTK_DIALOG_ABOUT_NAME                 "dialog_about"
#define TEST_U_UI_GTK_DIALOG_MAIN_NAME                  "dialog_main"
#define TEST_U_UI_GTK_DRAWINGAREA_NAME                  "drawingarea_2d"
#if defined (GTKGL_SUPPORT)
#define TEST_U_UI_GTK_DRAWINGAREA_3D_NAME               "drawingarea_3d"
#endif // GTKGL_SUPPORT
#define TEST_U_UI_GTK_FILECHOOSERBUTTON_FILE_NAME       "filechooserbutton_file"
#define TEST_U_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME       "filechooserbutton_save"
#define TEST_U_UI_GTK_FILECHOOSERDIALOG_FILE_NAME       "filechooserdialog_file"
#define TEST_U_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME       "filechooserdialog_save"
#define TEST_U_UI_GTK_FILEFILTER_MP3_NAME               "filefilter_mp3"
#define TEST_U_UI_GTK_FILEFILTER_WAV_NAME               "filefilter_wav"
#define TEST_U_UI_GTK_FRAME_DEVICE_NAME                 "frame_device"
#define TEST_U_UI_GTK_FRAME_EFFECT_NAME                 "frame_effect"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#define TEST_U_UI_GTK_FRAME_EFFECT_WIN32_DS_FLANGER_OPTIONS_NAME "frame_effect_win32_ds_flanger_options"
#endif // ACE_WIN32 || ACE_WIN64
#define TEST_U_UI_GTK_FRAME_FILE_NAME                   "frame_file"
#define TEST_U_UI_GTK_FRAME_NOISE_NAME                  "frame_noise"
#define TEST_U_UI_GTK_FRAME_PERLIN_NAME                 "frame_perlin"
#define TEST_U_UI_GTK_FRAME_SAVE_NAME                   "frame_save"
#define TEST_U_UI_GTK_FRAME_SINUS_NAME                  "frame_sinus"
#define TEST_U_UI_GTK_GRID_NOISE_NAME                   "grid_noise"
#define TEST_U_UI_GTK_HBOX_EFFECT_NAME                  "hbox_effect"
#define TEST_U_UI_GTK_HBOX_NOISE_NAME                   "hbox_noise"
#define TEST_U_UI_GTK_HBOX_VISUALIZATION_NAME           "hbox_visualization_2"
#define TEST_U_UI_GTK_HSCALE_DEVICE_BOOST_NAME          "hscale_device_boost"
#define TEST_U_UI_GTK_HSCALE_DEVICE_VOLUME_NAME         "hscale_device_volume"
#define TEST_U_UI_GTK_HSCALE_SINUS_FREQUENCY_NAME       "hscale_sinus_frequency"
#define TEST_U_UI_GTK_HSCALE_VOLUME_NAME                "hscale_volume"
#define TEST_U_UI_GTK_LISTSTORE_CHANNELS_NAME           "liststore_channels"
#define TEST_U_UI_GTK_LISTSTORE_DEVICE_NAME             "liststore_device"
#define TEST_U_UI_GTK_LISTSTORE_EFFECT_NAME             "liststore_effect"
#define TEST_U_UI_GTK_LISTSTORE_FORMAT_NAME             "liststore_format"
#define TEST_U_UI_GTK_LISTSTORE_FREQUENCY_NAME          "liststore_frequency"
#define TEST_U_UI_GTK_LISTSTORE_RESOLUTION_NAME         "liststore_resolution"
#define TEST_U_UI_GTK_LISTSTORE_SOURCE_NAME             "liststore_source"
#define TEST_U_UI_GTK_RADIOBUTTON_NOISE_NAME            "radiobutton_noise"
#define TEST_U_UI_GTK_RADIOBUTTON_PINK_NAME             "radiobutton_pink"
#if defined (LIBNOISE_SUPPORT)
#define TEST_U_UI_GTK_RADIOBUTTON_PERLIN_NAME           "radiobutton_perlin"
#endif // LIBNOISE_SUPPORT
#define TEST_U_UI_GTK_RADIOBUTTON_SAWTOOTH_NAME         "radiobutton_sawtooth"
#define TEST_U_UI_GTK_RADIOBUTTON_SINUS_NAME            "radiobutton_sinus"
#define TEST_U_UI_GTK_RADIOBUTTON_SQUARE_NAME           "radiobutton_square"
#define TEST_U_UI_GTK_RADIOBUTTON_TRIANGLE_NAME         "radiobutton_triangle"
#define TEST_U_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME     "radiobutton_oscilloscope"
#define TEST_U_UI_GTK_RADIOBUTTON_SPECTRUM_NAME         "radiobutton_spectrum"
#define TEST_U_UI_GTK_PROGRESSBAR_NAME                  "progressbar"
#define TEST_U_UI_GTK_SIZEGROUP_OPTIONS_NAME            "sizegroup_options"
#define TEST_U_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME        "spinbutton_buffersize"
#define TEST_U_UI_GTK_SPINBUTTON_CAPTUREDSAMPLES_NAME   "spinbutton_captured_samples"
#define TEST_U_UI_GTK_SPINBUTTON_DATA_NAME              "spinbutton_data"
#define TEST_U_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME      "spinbutton_data_messages"
#define TEST_U_UI_GTK_SPINBUTTON_DROPPEDSAMPLES_NAME    "spinbutton_dropped_samples"
#define TEST_U_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME   "spinbutton_session_messages"
#define TEST_U_UI_GTK_STATUSBAR_NAME                    "statusbar"
#define TEST_U_UI_GTK_TOGGLEBUTTON_RECORD_NAME          "togglebutton_record"
#define TEST_U_UI_GTK_TOGGLEBUTTON_3D_NAME              "togglebutton_3d"
#define TEST_U_UI_GTK_TOGGLEBUTTON_MUTE_NAME            "togglebutton_mute"
#define TEST_U_UI_GTK_VBOX_CONFIGURATION_NAME           "vbox_configuration"
#define TEST_U_UI_GTK_VBOX_FORMAT_OPTIONS_NAME          "vbox_format_options"
#define TEST_U_UI_GTK_VBOX_NOISE_NAME                   "vbox_noise"
#define TEST_U_UI_GTK_VBOX_NOISE_OPTIONS_NAME           "vbox_noise_options"
#endif // GTK_SUPPORT

//----------------------------------------

#if defined (GLUT_SUPPORT)
#define TEST_U_GLUT_DEFAULT_HEIGHT         834
#define TEST_U_GLUT_DEFAULT_WIDTH          1112
#define TEST_U_GLUT_DEFAULT_LAYERS         20
#define TEST_U_GLUT_DEFAULT_D              12
#define TEST_U_GLUT_DEFAULT_AMP_FACTOR     0.002f
#define TEST_U_GLUT_DEFAULT_FPS            60
#endif // GLUT_SUPPORT

#endif
