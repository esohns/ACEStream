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

#ifndef TEST_I_CAMERA_AR_COMMON_MODULES_H
#define TEST_I_CAMERA_AR_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#else
#include "stream_dev_cam_source_v4l.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"
#endif // FFMPEG_SUPPORT
#include "stream_dec_rgb_hflip.h"

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT

#include "test_i_camera_msa_stream_common.h"
#include "test_i_message.h"
#if defined (OLC_PGE_SUPPORT)
#include "test_i_module_pge.h"
#endif // OLC_PGE_SUPPORT
#include "test_i_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_I_CameraMSA_DirectShow_SessionData,
                                 struct Test_I_StatisticData,
                                 struct Stream_UserData> Test_I_DirectShow_SessionManager_t;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_I_CameraMSA_MediaFoundation_SessionData,
                                 struct Test_I_StatisticData,
                                 struct Stream_UserData> Test_I_MediaFoundation_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_DirectShow_Message_t,
                               Test_I_DirectShow_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_DirectShow_Message_t,
                                Test_I_DirectShow_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_DirectShow_TaskBaseAsynch_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_MediaFoundation_Message_t,
                               Test_I_MediaFoundation_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_MediaFoundation_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_MediaFoundation_Message_t,
                                Test_I_MediaFoundation_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_MediaFoundation_TaskBaseAsynch_t;

typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Test_I_DirectShow_Message_t,
                                           Test_I_DirectShow_SessionMessage_t,
                                           struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_I_CameraMSA_DirectShow_StreamState,
                                           struct Test_I_StatisticData,
                                           Test_I_DirectShow_SessionManager_t,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData,
                                           struct _AMMediaType,
                                           false> Test_I_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Test_I_MediaFoundation_Message_t,
                                                Test_I_MediaFoundation_SessionMessage_t,
                                                struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Test_I_CameraMSA_MediaFoundation_StreamState,
                                                struct Test_I_StatisticData,
                                                Test_I_MediaFoundation_SessionManager_t,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData,
                                                IMFMediaType*> Test_I_MediaFoundation_Source;

typedef Stream_Decoder_RGB_HFlip_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_DirectShow_Message_t,
                                   Test_I_DirectShow_SessionMessage_t,
                                   Test_I_CameraMSA_DirectShow_SessionData_t,
                                   struct _AMMediaType> Test_I_DirectShow_HFlip;
typedef Stream_Decoder_RGB_HFlip_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_MediaFoundation_Message_t,
                                   Test_I_MediaFoundation_SessionMessage_t,
                                   Test_I_CameraMSA_MediaFoundation_SessionData_t,
                                   IMFMediaType*> Test_I_MediaFoundation_HFlip;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_DirectShow_TaskBaseSynch_t,
                                        struct _AMMediaType> Test_I_DirectShow_LibAVConvert;
typedef Stream_Decoder_LibAVConverter_T<Test_U_MediaFoundation_TaskBaseSynch_t,
                                        IMFMediaType*> Test_I_MediaFoundation_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<Test_U_DirectShow_TaskBaseSynch_t,
                                           struct _AMMediaType> Test_I_DirectShow_LibAVResize;
typedef Stream_Visualization_LibAVResize_T<Test_U_MediaFoundation_TaskBaseSynch_t,
                                           IMFMediaType*> Test_I_MediaFoundation_LibAVResize;
#endif // FFMPEG_SUPPORT
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_I_CameraMSA_V4L_SessionData,
                                 struct Test_I_StatisticData,
                                 struct Stream_UserData> Test_I_V4L_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Message_t,
                               Test_I_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_I_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_I_Message_t,
                                Test_I_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_I_TaskBaseAsynch_t;

typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Test_I_Message_t,
                                      Test_I_SessionMessage_t,
                                      struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_I_CameraMSA_StreamState,
                                      struct Test_I_StatisticData,
                                      Test_I_V4L_SessionManager_t,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Test_I_V4L_Source;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_I_Message_t,
                                      Test_I_SessionMessage_t,
                                      Test_I_CameraMSA_V4L_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Test_I_LibAVDecode;

typedef Stream_Decoder_LibAVConverter_T<Test_I_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Test_I_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<Test_I_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Test_I_LibAVResize;
#endif // FFMPEG_SUPPORT

typedef Stream_Decoder_RGB_HFlip_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Message_t,
                                   Test_I_SessionMessage_t,
                                   Test_I_CameraMSA_V4L_SessionData_t,
                                   struct Stream_MediaFramework_V4L_MediaType> Test_I_V4L_HFlip;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_DirectShow_Message_t,
                                                      Test_I_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_DirectShow_Message_t,
                                                      Test_I_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_MediaFoundation_Message_t,
                                                      Test_I_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_MediaFoundation_Message_t,
                                                      Test_I_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_MediaFoundation_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message_t,
                                                      Test_I_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Message_t,
                                                      Test_I_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_I_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Test_I_DirectShow_Message_t,
//                                       Test_I_DirectShow_SessionMessage_t,
//                                       Test_I_CameraMSA_DirectShow_SessionData,
//                                       struct Stream_UserData> Test_I_DirectShow_MessageHandler;
//
//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Test_I_MediaFoundation_Message_t,
//                                       Test_I_MediaFoundation_SessionMessage_t,
//                                       Test_I_CameraMSA_MediaFoundation_SessionData,
//                                       struct Stream_UserData> Test_I_MediaFoundation_MessageHandler;

#if defined (OLC_PGE_SUPPORT)
typedef Test_I_Module_PGE_T<Test_U_DirectShow_TaskBaseAsynch_t,
                            struct _AMMediaType> Test_I_DirectShow_PGE;
typedef Test_I_Module_PGE_T<Test_U_MediaFoundation_TaskBaseAsynch_t,
                            IMFMediaType*> Test_I_MediaFoundation_PGE;
#endif // OLC_PGE_SUPPORT
#else
//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Test_I_Message_t,
//                                       Test_I_SessionMessage_t,
//                                       Test_I_V4L_SessionData,
//                                       struct Stream_UserData> Test_I_MessageHandler;

#if defined (OLC_PGE_SUPPORT)
typedef Test_I_Module_PGE_T<Test_I_TaskBaseAsynch_t,
                            struct Stream_MediaFramework_V4L_MediaType> Test_I_PGE;
#endif // OLC_PGE_SUPPORT

#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_DirectShow_Source);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_MediaFoundation_Source);           // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_DirectShow_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_MediaFoundation_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_MediaFoundation_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_DirectShow_LibAVResize);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_MediaFoundation_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_MediaFoundation_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                // session event type
                              struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_rgb_hflip_module_name_string,
                              Stream_INotify_t,                                              // stream notification interface type
                              Test_I_DirectShow_HFlip);                                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                     // session event type
                              struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_rgb_hflip_module_name_string,
                              Stream_INotify_t,                                                   // stream notification interface type
                              Test_I_MediaFoundation_HFlip);                                      // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_V4L_Source);                       // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_V4L_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_decoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_I_LibAVDecode);                                       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_I_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_rgb_hflip_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_V4L_HFlip);                                      // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_I_CameraMSA_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_I_DirectShow_Statistic_ReaderTask_t, // reader type
                          Test_I_DirectShow_Statistic_WriterTask_t, // writer type
                          Test_I_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Test_I_CameraMSA_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_I_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Test_I_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Test_I_MediaFoundation_StatisticReport);  // name
#else
DATASTREAM_MODULE_DUPLEX (Test_I_CameraMSA_V4L_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_I_Statistic_ReaderTask_t,            // reader type
                          Test_I_Statistic_WriterTask_t,            // writer type
                          Test_I_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_DirectShow_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Test_I_DirectShow_MessageHandler);        // writer type
//
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_MediaFoundation_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Test_I_MediaFoundation_MessageHandler);   // writer type

#if defined (OLC_PGE_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_pge_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Test_I_DirectShow_PGE);                              // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_pge_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Test_I_MediaFoundation_PGE);                              // writer type
#endif // OLC_PGE_SUPPORT
#else
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_V4L_SessionData,                           // session data type
//                              enum Stream_SessionMessageType,                       // session event type
//                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                     // stream notification interface type
//                              Test_I_MessageHandler);                       // writer type

#if defined (OLC_PGE_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_CameraMSA_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                        // session event type
                              struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_pge_module_name_string,
                              Stream_INotify_t,                                      // stream notification interface type
                              Test_I_PGE);                                  // writer type
#endif // OLC_PGE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
