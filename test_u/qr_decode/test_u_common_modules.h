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

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"
#include "stream_task_base_asynch.h"
#include "stream_task_base_synch.h"

#include "stream_dec_rgb_hflip.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#include "stream_dev_cam_source_vfw.h"
#else
#include "stream_dev_cam_source_v4l.h"
#if defined (LIBCAMERA_SUPPORT)
#include "stream_dev_cam_source_libcamera.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"
#endif // FFMPEG_SUPPORT

#if defined (OPENCV_SUPPORT)
#include "stream_dec_opencv_qr_decoder.h"
#endif // OPENCV_SUPPORT

#include "stream_misc_messagehandler.h"

#include "test_u_stream_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 QRDecode_DirectShow_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_DirectShow_SessionManager_t;
//typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
//                                 enum Stream_SessionMessageType,
//                                 struct Stream_SessionManager_Configuration,
//                                 QRDecode_MediaFoundation_SessionData,
//                                 struct Stream_Statistic,
//                                 struct Stream_UserData> Test_U_MediaFoundation_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct QRDecode_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_DirectShow_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct QRDecode_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_Message,
                                Test_U_DirectShow_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_DirectShow_TaskBaseAsynch_t;
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 QRDecode_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct QRDecode_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message,
                               Test_U_SessionMessage,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct QRDecode_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_Message,
                                Test_U_SessionMessage,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_TaskBaseAsynch_t;
#endif // ACE_WIN32 || ACE_WIN64

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Cam_Source_VfW_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_U_Message,
                                    Test_U_DirectShow_SessionMessage,
                                    struct QRDecode_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_U_DirectShow_StreamState,
                                    struct Stream_Statistic,
                                    Test_U_DirectShow_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData,
                                    struct _AMMediaType> Test_U_VfW_Source;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                 // module handler configuration type
                              libacestream_default_dev_cam_source_vfw_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_U_VfW_Source);                                         // writer type

typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Test_U_Message,
                                           Test_U_DirectShow_SessionMessage,
                                           struct QRDecode_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_U_DirectShow_StreamState,
                                           struct Stream_Statistic,
                                           Test_U_DirectShow_SessionManager_t,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData,
                                           struct _AMMediaType,
                                           false> Test_U_DirectShow_Source;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                                    // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                        // module handler configuration type
                              libacestream_default_dev_cam_source_directshow_module_name_string,
                              Stream_INotify_t,                                                  // stream notification interface type
                              Test_U_DirectShow_Source);                                         // writer type

//typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
//                                                Stream_ControlMessage_t,
//                                                Test_U_Message,
//                                                Test_U_SessionMessage,
//                                                struct QRDecode_ModuleHandlerConfiguration,
//                                                enum Stream_ControlType,
//                                                enum Stream_SessionMessageType,
//                                                struct Test_U_StreamState,
//                                                struct QRDecode_SessionData,
//                                                QRDecode_SessionData_t,
//                                                struct Stream_Statistic,
//                                                Common_Timer_Manager_t,
//                                                struct Stream_UserData,
//                                                IMFMediaType*> Test_U_MediaFoundation_Source;
//DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                            // session data type
//                              enum Stream_SessionMessageType,                                         // session event type
//                              struct QRDecode_ModuleHandlerConfiguration,                             // module handler configuration type
//                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
//                              Stream_INotify_t,                                                       // stream notification interface type
//                              Test_U_MediaFoundation_Source);                                         // writer type
#else
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_Module_CamSource_LibCamera_T<ACE_MT_SYNCH,
                                            Stream_ControlMessage_t,
                                            Test_U_Message,
                                            Test_U_SessionMessage,
                                            struct QRDecode_ModuleHandlerConfiguration,
                                            enum Stream_ControlType,
                                            enum Stream_SessionMessageType,
                                            struct Test_U_StreamState,
                                            struct QRDecode_SessionData,
                                            QRDecode_SessionData_t,
                                            struct Stream_Statistic,
                                            Common_Timer_Manager_t,
                                            struct Stream_UserData> Test_U_LibCamera_Source;
DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                      // session data type
                              enum Stream_SessionMessageType,                                   // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                       // module handler configuration type
                              libacestream_default_dev_cam_source_libcamera_module_name_string,
                              Stream_INotify_t,                                                 // stream notification interface type
                              Test_U_LibCamera_Source);                                         // writer type
#endif // LIBCAMERA_SUPPORT

typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Test_U_Message,
                                      Test_U_SessionMessage,
                                      struct QRDecode_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_U_StreamState,
                                      struct Stream_Statistic,
                                      Test_U_SessionManager_t,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Test_U_V4L_Source;
DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                 // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_U_V4L_Source);                                         // writer type
#endif // ACE_WIN32 || ACE_WIN64

//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct QRDecode_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_Message,
//                                                      Test_U_SessionMessage,
//                                                      int,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      struct QRDecode_SessionData,
//                                                      QRDecode_SessionData_t> Test_U_StatisticReport_ReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct QRDecode_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Test_U_Message,
//                                                      Test_U_SessionMessage,
//                                                      int,
//                                                      struct Stream_Statistic,
//                                                      Common_Timer_Manager_t,
//                                                      struct QRDecode_SessionData,
//                                                      QRDecode_SessionData_t> Test_U_StatisticReport_WriterTask_t;
//DATASTREAM_MODULE_DUPLEX (struct QRDecode_SessionData,                         // session data type
//                          enum Stream_SessionMessageType,                      // session event type
//                          struct QRDecode_ModuleHandlerConfiguration,            // module handler configuration type
//                          libacestream_default_stat_report_module_name_string,
//                          Stream_INotify_t,                                    // stream notification interface type
//                          Test_U_StatisticReport_ReaderTask_t,                 // reader type
//                          Test_U_StatisticReport_WriterTask_t,                 // writer type
//                          Test_U_StatisticReport);                             // name

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_RGB_HFlip_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct QRDecode_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_U_Message,
                                   Test_U_DirectShow_SessionMessage,
                                   QRDecode_DirectShow_SessionData_t,
                                   struct _AMMediaType> Test_U_DirectShow_RGB24Flip;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                  // module handler configuration type
                              libacestream_default_dec_rgb_hflip_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_U_DirectShow_RGB24Flip);                                // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_DirectShow_TaskBaseSynch_t,
                                        struct _AMMediaType> Test_U_DirectShow_LibAVConverter;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                             // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                  // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_U_DirectShow_LibAVConverter);                           // writer type

//typedef Stream_Decoder_LibAVConverter_T<Test_U_MediaFoundation_TaskBaseSynch_t,
//                                        IMFMediaType*> Test_U_MediaFoundation_LibAVConverter;
//DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                             // session data type
//                              enum Stream_SessionMessageType,                              // session event type
//                              struct QRDecode_ModuleHandlerConfiguration,                  // module handler configuration type
//                              libacestream_default_dec_libav_converter_module_name_string,
//                              Stream_INotify_t,                                            // stream notification interface type
//                              Test_U_MediaFoundation_LibAVConverter);                           // writer type
#endif // FFMPEG_SUPPORT
#else
#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Test_U_LibAVConverter;
DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                 // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                  // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_U_LibAVConverter);                                      // writer type

typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct QRDecode_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_U_Message,
                                      Test_U_SessionMessage,
                                      QRDecode_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Test_U_LibAVDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                 // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_U_LibAVDecoder);                                       // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (OPENCV_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_OpenCVQRDecoder_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct QRDecode_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_DirectShow_SessionMessage,
                                         QRDecode_DirectShow_SessionData_t,
                                         struct _AMMediaType> Test_U_DirectShow_QRDecoder;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                               // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                    // module handler configuration type
                              libacestream_default_dec_opencv_qr_decoder_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_U_DirectShow_QRDecoder);                                  // writer type

//typedef Stream_Decoder_OpenCVQRDecoder_T<ACE_MT_SYNCH,
//                                         Common_TimePolicy_t,
//                                         struct QRDecode_ModuleHandlerConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Test_U_Message,
//                                         Test_U_SessionMessage,
//                                         QRDecode_SessionData_t,
//                                         IMFMediaType*> Test_U_MediaFoundation_QRDecoder;
//DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                   // session data type
//                              enum Stream_SessionMessageType,                                // session event type
//                              struct QRDecode_ModuleHandlerConfiguration,                    // module handler configuration type
//                              libacestream_default_dec_opencv_qr_decoder_module_name_string,
//                              Stream_INotify_t,                                              // stream notification interface type
//                              Test_U_MediaFoundation_QRDecoder);                             // writer type
#else
typedef Stream_Decoder_OpenCVQRDecoder_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct QRDecode_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_U_Message,
                                         Test_U_SessionMessage,
                                         QRDecode_SessionData_t,
                                         struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_QRDecoder;
DATASTREAM_MODULE_INPUT_ONLY (struct QRDecode_SessionData,                                   // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                    // module handler configuration type
                              libacestream_default_dec_opencv_qr_decoder_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_U_QRDecoder);                                             // writer type
#endif // ACE_WIN32 || ACE_WIN64
#endif // OPENCV_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct QRDecode_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_DirectShow_SessionMessage,
                                       QRDecode_DirectShow_SessionData,
                                       struct Stream_UserData> Test_U_DirectShow_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_DirectShow_SessionData,                               // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_U_DirectShow_MessageHandler);                                  // writer type
#else
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct QRDecode_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message,
                                       Test_U_SessionMessage,
                                       QRDecode_SessionData,
                                       struct Stream_UserData> Test_U_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (QRDecode_SessionData,                                          // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct QRDecode_ModuleHandlerConfiguration,                    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_U_MessageHandler);                                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
