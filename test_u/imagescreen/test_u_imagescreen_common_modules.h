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

#include "ace/config-lite.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_file_source.h"

#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_img_decoder.h"

#include "stream_vis_libav_resize.h"
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
#include "stream_vis_target_direct3d.h"
#else
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_imagescreen_common.h"
#include "test_u_imagescreen_message.h"
#include "test_u_imagescreen_session_message.h"

// declare module(s)
#if defined (FFMPEG_SUPPORT)
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Stream_ImageScreen_Message_t,
                                    Stream_ImageScreen_SessionMessage_t,
                                    struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Stream_ImageScreen_StreamState,
                                    Stream_ImageScreen_SessionData,
                                    Stream_ImageScreen_SessionData_t,
                                    struct Stream_Statistic,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Stream_ImageScreen_Source;
//typedef Stream_Decoder_LibAV_ImageDecoder_T<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                            Stream_ControlMessage_t,
//                                            Stream_ImageScreen_Message_t,
//                                            Stream_ImageScreen_SessionMessage_t,
//                                            Stream_ImageScreen_SessionData_t,
//                                            struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Decode;
typedef Stream_Decoder_LibAVConverter1_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Stream_ImageScreen_Message_t,
                                         Stream_ImageScreen_SessionMessage_t,
                                         Stream_ImageScreen_SessionData_t,
                                         struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Convert;
typedef Stream_Visualization_LibAVResize1_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Stream_ImageScreen_Message_t,
                                            Stream_ImageScreen_SessionMessage_t,
                                            struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Resize;
#elif defined (IMAGEMAGICK_SUPPORT)
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
                                         struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Source;
//typedef Stream_Decoder_ImageMagick_Decoder_T<ACE_MT_SYNCH,
//                                             Common_TimePolicy_t,
//                                             struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                             Stream_ControlMessage_t,
//                                             Stream_ImageScreen_Message_t,
//                                             Stream_ImageScreen_SessionMessage_t,
//                                             struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_ImageMagickDecoder;
typedef Stream_Visualization_ImageMagickResize1_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Stream_ImageScreen_Message_t,
                                                  Stream_ImageScreen_SessionMessage_t,
                                                  struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Resize;
#endif // FFMPEG_SUPPORT || IMAGEMAGICK_SUPPORT

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_ImageScreen_Message_t,
//                                                      Stream_ImageScreen_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_ImageScreen_SessionData,
//                                                      Stream_ImageScreen_SessionData_t> Stream_ImageScreen_Statistic_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_ImageScreen_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_ImageScreen_Message_t,
//                                                      Stream_ImageScreen_SessionMessage_t,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      Stream_ImageScreen_SessionData,
//                                                      Stream_ImageScreen_SessionData_t> Stream_ImageScreen_Statistic_WriterTask_t;

typedef Stream_Module_Delay_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              struct Stream_ImageScreen_ModuleHandlerConfiguration,
                              Stream_ControlMessage_t,
                              Stream_ImageScreen_Message_t,
                              Stream_ImageScreen_SessionMessage_t,
                              Stream_UserData> Stream_ImageScreen_Delay;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_ImageScreen_Message_t,
                                     Stream_ImageScreen_SessionMessage_t,
                                     Stream_ImageScreen_SessionData,
                                     Stream_ImageScreen_SessionData_t,
                                     struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Display;
#else
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_ImageScreen_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_MediaType> Stream_ImageScreen_Display;
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_ImageScreen_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Stream_ImageScreen_Message_t,
                                       Stream_ImageScreen_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_ImageScreen_SessionData,
                                       struct Stream_UserData> Stream_ImageScreen_MessageHandler;

//////////////////////////////////////////
#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Source);                       // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_dec_libav_decoder_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_ImageScreen_LibAVDecoder);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Convert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Resize);                       // writer type
#elif defined (IMAGEMAGICK_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_imagemagick_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Source);                       // writer type
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
                              Stream_ImageScreen_Resize);                       // writer type
#endif // FFMPEG_SUPPORT || IMAGEMAGICK_SUPPORT

//DATASTREAM_MODULE_DUPLEX (Stream_ImageScreen_SessionData,                // session data type
//                          enum Stream_SessionMessageType,                   // session event type
//                          struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                 // stream notification interface type
//                          Stream_ImageScreen_Statistic_ReaderTask_t,            // reader type
//                          Stream_ImageScreen_Statistic_WriterTask_t,            // writer type
//                          Stream_ImageScreen_StatisticReport);                  // name

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
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_ImageScreen_Display);                              // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_ImageScreen_Display);                      // writer type
#endif // ACE_WIN32 || ACE_WIN64

DATASTREAM_MODULE_INPUT_ONLY (Stream_ImageScreen_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_ImageScreen_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_ImageScreen_MessageHandler);                       // writer type

#endif
