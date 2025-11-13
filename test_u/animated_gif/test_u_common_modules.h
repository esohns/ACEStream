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

#ifndef TEST_U_COMMON_MODULES_H
#define TEST_U_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"
#include "stream_session_manager.h"

//#include "stream_file_sink.h"
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

#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#include "test_u_animated_gif_common.h"
#include "test_u_imagemagick_target.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_U_AnimatedGIF_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

// declare module(s)
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_Message,
                                Test_U_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_TaskBaseAsynch_t;

typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_SessionMessage,
                                    struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_U_StreamState,
                                    struct Stream_Statistic,
                                    Test_U_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Test_U_FileReader;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              Test_U_FileReader);                       // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_U_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message,
                                                      Test_U_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_U_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (Test_U_AnimatedGIF_SessionData,                // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                  // stream notification interface type
                          Test_U_Module_Statistic_ReaderTask_t,     // reader type
                          Test_U_Module_Statistic_WriterTask_t,     // writer type
                          Test_U_StatisticReport);                  // name

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAV_ImageDecoder_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_U_Message,
                                            Test_U_SessionMessage,
                                            Test_U_SessionData_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                            struct _AMMediaType> Test_U_FFMPEG_Decode;
#else
                                            struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_FFMPEG_Decode;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_FFMPEG_Decode);                // writer type

typedef Stream_Decoder_LibAVConverter_T<Test_U_TaskBaseSynch_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                        struct _AMMediaType> Test_U_LibAVConverter;
#else
                                        struct Stream_MediaFramework_FFMPEG_MediaType> Test_U_LibAVConverter;
#endif // ACE_WIN32 || ACE_WIN64
typedef Stream_Visualization_LibAVResize1_T<Test_U_TaskBaseSynch_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                            struct _AMMediaType> Test_U_FFMPEG_Resize;
#else
                                            struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_FFMPEG_Resize;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_FFMPEG_Resize);                // writer type

typedef Stream_Decoder_LibAVConverter1_T<Test_U_TaskBaseSynch_t,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                         struct _AMMediaType> Test_U_FFMPEG_Convert;
#else
                                         struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_FFMPEG_Convert;
#endif // ACE_WIN32 || ACE_WIN64
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_FFMPEG_Convert);               // writer type
#endif // FFMPEG_SUPPORT

#if defined (IMAGEMAGICK_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_File_ImageMagick_Source_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         struct Test_U_StreamState,
                                         struct Stream_Statistic,
                                         Test_U_SessionManager_t,
                                         Common_Timer_Manager_t,
                                         struct Stream_UserData,
                                         struct _AMMediaType> Test_U_ImageMagick_Source;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_imagemagick_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_ImageMagick_Source);           // writer type

typedef Stream_Visualization_ImageMagickResize1_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Test_U_Message,
                                                  Test_U_SessionMessage,
                                                  struct _AMMediaType> Test_U_ImageMagick_Resize;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_imagemagick_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_ImageMagick_Resize);           // writer type

typedef Test_U_ImageMagick_Target_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_SessionMessage,
                                    struct _AMMediaType> Test_U_ImageMagick_Target;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData, // session data type
                              enum Stream_SessionMessageType, // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_imagemagick_target_module_name_string,
                              Stream_INotify_t, // stream notification interface type
                              Test_U_ImageMagick_Target); // writer type
#else
typedef Stream_File_ImageMagick_Source_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         struct Test_U_StreamState,
                                         struct Stream_Statistic,
                                         Test_U_SessionManager_t,
                                         Common_Timer_Manager_t,
                                         struct Stream_UserData,
                                         struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_ImageMagick_Source;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_imagemagick_source_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_ImageMagick_Source);           // writer type

//typedef Stream_Decoder_ImageMagick_Decoder_T<ACE_MT_SYNCH,
//                                             Common_TimePolicy_t,
//                                             struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
//                                             Stream_ControlMessage_t,
//                                             Test_U_Message,
//                                             Test_U_SessionMessage,
//                                             struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_ImageMagick_Decoder;
//DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_dec_imagemagick_decoder_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Test_U_ImageMagickDecoder);                     // writer type

typedef Stream_Visualization_ImageMagickResize1_T<ACE_MT_SYNCH,
                                                  Common_TimePolicy_t,
                                                  struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                                  Stream_ControlMessage_t,
                                                  Test_U_Message,
                                                  Test_U_SessionMessage,
                                                  struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_ImageMagick_Resize;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_imagemagick_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_ImageMagick_Resize);           // writer type

typedef Test_U_ImageMagick_Target_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                    Stream_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_SessionMessage,
                                    struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_ImageMagick_Target;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData, // session data type
                              enum Stream_SessionMessageType, // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_imagemagick_target_module_name_string,
                              Stream_INotify_t, // stream notification interface type
                              Test_U_ImageMagick_Target); // writer type
#endif // ACE_WIN32 || ACE_WIN64
#endif // IMAGEMAGICK_SUPPORT

//typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
//                                   Common_TimePolicy_t,
//                                   struct struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
//                                   Stream_ControlMessage_t,
//                                   Test_U_Message,
//                                   Test_U_SessionMessage> Test_U_FileWriter;
//DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                    // session event type
//                              struct struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_file_sink_module_name_string,
//                              Stream_INotify_t,                                  // stream notification interface type
//                              Test_U_FileWriter);                       // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_AnimatedGIF_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       Test_U_AnimatedGIF_SessionData,
                                       struct Stream_UserData> Test_U_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (Test_U_AnimatedGIF_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Test_U_AnimatedGIF_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              Test_U_MessageHandler);                       // writer type

#endif
