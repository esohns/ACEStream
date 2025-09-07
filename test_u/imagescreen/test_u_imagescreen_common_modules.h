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

#ifndef TEST_U_IMAGESCREEN_COMMON_MODULES_H
#define TEST_U_IMAGESCREEN_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#include "stream_file_source.h"
#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_img_decoder.h"

#include "stream_vis_libav_resize.h"

#include "stream_dec_libav_converter.h"
#endif // FFMPEG_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
#include "stream_file_imagemagick_source.h"

#include "stream_dec_imagemagick_decoder.h"

#include "stream_vis_imagemagick_resize.h"
#endif // IMAGEMAGICK_SUPPORT

#include "stream_misc_defines.h"
#include "stream_misc_delay.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct2d.h"
#include "stream_vis_target_direct3d.h"
#else
#include "stream_vis_wayland_window.h"
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo.h"
#endif // GTK_SUPPORT

#include "test_u_imagescreen_common.h"
#include "test_u_imagescreen_message.h"
#include "test_u_imagescreen_session_message.h"

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Stream_ImageScreen_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

// declare module(s)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_ImageScreen_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_ImageScreen_Message_t,
                               Stream_ImageScreen_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_ImageScreen_Message_t,
                                Stream_ImageScreen_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_TaskBaseAsynch_t;

typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Stream_ImageScreen_Message_t,
                                    Stream_ImageScreen_SessionMessage_t,
                                    struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Stream_ImageScreen_StreamState,
                                    struct Stream_Statistic,
                                    Test_U_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Stream_ImageScreen_Source;
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAV_ImageDecoder_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_ImageScreen_Message_t,
                                            Stream_ImageScreen_SessionMessage_t,
                                            Stream_ImageScreen_SessionData_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                            struct _AMMediaType> Stream_ImageScreen_FFMPEG_Decode;
#else
                                            struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_FFMPEG_Decode;
#endif // ACE_WIN32 || ACE_WIN64
typedef Stream_Decoder_LibAVConverter_T<Test_U_TaskBaseSynch_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                        struct _AMMediaType> Test_U_LibAVConverter;
#else
                                        struct Stream_MediaFramework_FFMPEG_MediaType> Test_U_LibAVConverter;
#endif // ACE_WIN32 || ACE_WIN64
typedef Stream_Visualization_LibAVResize1_T<Test_U_TaskBaseSynch_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                            struct _AMMediaType> Stream_ImageScreen_FFMPEG_Resize;
#else
                                            struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_FFMPEG_Resize;
#endif // ACE_WIN32 || ACE_WIN64
typedef Stream_Decoder_LibAVConverter1_T<Test_U_TaskBaseSynch_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                         struct _AMMediaType> Stream_ImageScreen_FFMPEG_Convert;
#else
                                         struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_FFMPEG_Convert;
#endif // ACE_WIN32 || ACE_WIN64
#endif // FFMPEG_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_File_ImageMagick_Source_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Stream_ImageScreen_Message_t,
                                         Stream_ImageScreen_SessionMessage_t,
                                         struct Stream_ImageScreen_StreamState,
                                         struct Stream_Statistic,
                                         Test_U_SessionManager_t,
                                         Common_Timer_Manager_t,
                                         struct Stream_UserData,
                                         struct _AMMediaType> Stream_ImageScreen_ImageMagick_Source;
//typedef Stream_Decoder_ImageMagick_Decoder_T<ACE_MT_SYNCH,
//                                             Common_TimePolicy_t,
//                                             struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                             Stream_ControlMessage_t,
//                                             Stream_ImageScreen_Message_t,
//                                             Stream_ImageScreen_SessionMessage_t,
//                                             struct _AMMediaType> Stream_ImageScreen_ImageMagick_Decoder;
typedef Stream_Visualization_ImageMagickResize1_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Stream_ImageScreen_Message_t,
                                                  Stream_ImageScreen_SessionMessage_t,
                                                  struct _AMMediaType> Stream_ImageScreen_ImageMagick_Resize;
#else
typedef Stream_File_ImageMagick_Source_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Stream_ImageScreen_Message_t,
                                         Stream_ImageScreen_SessionMessage_t,
                                         struct Stream_ImageScreen_StreamState,
                                         struct Stream_Statistic,
                                         Common_Timer_Manager_t,
                                         struct Stream_UserData,
                                         struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_ImageMagick_Source;
//typedef Stream_Decoder_ImageMagick_Decoder_T<ACE_MT_SYNCH,
//                                             Common_TimePolicy_t,
//                                             struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                             Stream_ControlMessage_t,
//                                             Stream_ImageScreen_Message_t,
//                                             Stream_ImageScreen_SessionMessage_t,
//                                             struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_ImageMagick_Decoder;
typedef Stream_Visualization_ImageMagickResize1_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Stream_ImageScreen_Message_t,
                                                  Stream_ImageScreen_SessionMessage_t,
                                                  struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_ImageMagick_Resize;
#endif // ACE_WIN32 || ACE_WIN64
#endif // IMAGEMAGICK_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Stream_ImageScreen_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Stream_ImageScreen_Message_t,
                              Stream_ImageScreen_SessionMessage_t,
                              struct _AMMediaType,
                              struct Stream_UserData> Stream_ImageScreen_Delay;
#else
typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Stream_ImageScreen_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Stream_ImageScreen_Message_t,
                              Stream_ImageScreen_SessionMessage_t,
                              struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                              struct Stream_UserData> Stream_ImageScreen_Delay;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct2D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_ImageScreen_Message_t,
                                     Stream_ImageScreen_SessionMessage_t,
                                     Stream_ImageScreen_SessionData,
                                     Stream_ImageScreen_SessionData_t,
                                     struct _AMMediaType> Stream_ImageScreen_Display2D;

typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_ImageScreen_Message_t,
                                     Stream_ImageScreen_SessionMessage_t,
                                     Stream_ImageScreen_SessionData,
                                     Stream_ImageScreen_SessionData_t,
                                     struct _AMMediaType> Stream_ImageScreen_Display3D;

#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_ImageScreen_Message_t,
                                      Stream_ImageScreen_SessionMessage_t,
                                      Stream_ImageScreen_SessionData,
                                      Stream_ImageScreen_SessionData_t,
                                      struct _AMMediaType> Stream_ImageScreen_GTKDisplay;
#endif // GTK_SUPPORT
#else
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_ImageScreen_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_X11Display;

#if defined (GTK_SUPPORT)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Stream_ImageScreen_Message_t,
                                      Stream_ImageScreen_SessionMessage_t,
                                      Stream_ImageScreen_SessionData,
                                      Stream_ImageScreen_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_ImageScreen_GTKDisplay;
#endif // GTK_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_ImageScreen_SessionData,
                                       struct Stream_UserData> Stream_ImageScreen_MessageHandler;

//////////////////////////////////////////
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Source);                       // writer type
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_FFMPEG_Decode);                // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_FFMPEG_Resize);                // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_FFMPEG_Convert);               // writer type
#endif // FFMPEG_SUPPORT
#if defined (IMAGEMAGICK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_imagemagick_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_ImageMagick_Source);           // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_dec_imagemagick_decoder_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_ImageScreen_ImageMagickDecoder);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_imagemagick_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_ImageMagick_Resize);           // writer type
#endif // IMAGEMAGICK_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_delay_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Delay);                        // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct2d_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_ImageScreen_Display2D);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_ImageScreen_Display3D);                        // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Stream_ImageScreen_X11Display);                         // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                        // session data type
                              enum Stream_SessionMessageType,                        // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                      // stream notification interface type
                              Stream_ImageScreen_GTKDisplay);                        // writer type
#endif // GTK_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_ImageScreen_MessageHandler);                   // writer type

#endif
