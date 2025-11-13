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
#endif // FFMPEG_SUPPORT
#include "stream_dec_rgb24_hflip.h"

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT

#include "test_i_camera_ar_stream_common.h"
#include "test_i_camera_ar_message.h"
#if defined (OLC_CGE_SUPPORT)
#include "test_i_camera_ar_module_cge.h"
#endif // OLC_CGE_SUPPORT
#if defined (OLC_PGE_SUPPORT)
#include "test_i_camera_ar_module_pge.h"
#endif // OLC_PGE_SUPPORT
#include "test_i_camera_ar_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Stream_CameraAR_DirectShow_SessionData,
                                 struct Stream_CameraAR_StatisticData,
                                 struct Stream_UserData> Test_I_DirectShow_SessionManager_t;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Stream_CameraAR_MediaFoundation_SessionData,
                                 struct Stream_CameraAR_StatisticData,
                                 struct Stream_UserData> Test_I_MediaFoundation_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CameraAR_DirectShow_Message_t,
                               Stream_CameraAR_DirectShow_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CameraAR_DirectShow_Message_t,
                                Stream_CameraAR_DirectShow_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_DirectShow_TaskBaseAsynch_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CameraAR_MediaFoundation_Message_t,
                               Stream_CameraAR_MediaFoundation_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_MediaFoundation_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CameraAR_MediaFoundation_Message_t,
                                Stream_CameraAR_MediaFoundation_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_MediaFoundation_TaskBaseAsynch_t;

typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Stream_ControlMessage_t,
                                           Stream_CameraAR_DirectShow_Message_t,
                                           Stream_CameraAR_DirectShow_SessionMessage_t,
                                           struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Stream_CameraAR_DirectShow_StreamState,
                                           struct Stream_CameraAR_StatisticData,
                                           Test_I_DirectShow_SessionManager_t,
                                           Common_Timer_Manager_t,
                                           struct Stream_UserData,
                                           struct _AMMediaType,
                                           false> Stream_CameraAR_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Stream_ControlMessage_t,
                                                Stream_CameraAR_MediaFoundation_Message_t,
                                                Stream_CameraAR_MediaFoundation_SessionMessage_t,
                                                struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Stream_CameraAR_MediaFoundation_StreamState,
                                                struct Stream_CameraAR_StatisticData,
                                                Test_I_MediaFoundation_SessionManager_t,
                                                Common_Timer_Manager_t,
                                                struct Stream_UserData,
                                                IMFMediaType*> Stream_CameraAR_MediaFoundation_Source;

typedef Stream_Decoder_RGB24_HFlip_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraAR_DirectShow_Message_t,
                                     Stream_CameraAR_DirectShow_SessionMessage_t,
                                     Stream_CameraAR_DirectShow_SessionData_t,
                                     struct _AMMediaType> Stream_CameraAR_DirectShow_HFlip;
typedef Stream_Decoder_RGB24_HFlip_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraAR_MediaFoundation_Message_t,
                                     Stream_CameraAR_MediaFoundation_SessionMessage_t,
                                     Stream_CameraAR_MediaFoundation_SessionData_t,
                                     IMFMediaType*> Stream_CameraAR_MediaFoundation_HFlip;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_DirectShow_TaskBaseSynch_t,
                                        struct _AMMediaType> Stream_CameraAR_DirectShow_LibAVConvert;
typedef Stream_Decoder_LibAVConverter_T<Test_U_MediaFoundation_TaskBaseSynch_t,
                                        IMFMediaType*> Stream_CameraAR_MediaFoundation_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<Test_U_DirectShow_TaskBaseSynch_t,
                                           struct _AMMediaType> Stream_CameraAR_DirectShow_LibAVResize;
typedef Stream_Visualization_LibAVResize_T<Test_U_MediaFoundation_TaskBaseSynch_t,
                                           IMFMediaType*> Stream_CameraAR_MediaFoundation_LibAVResize;
#endif // FFMPEG_SUPPORT
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Stream_CameraAR_V4L_SessionData,
                                 struct Stream_CameraAR_StatisticData,
                                 struct Stream_UserData> Test_I_V4L_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Stream_CameraAR_Message_t,
                               Stream_CameraAR_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Stream_CameraAR_Message_t,
                                Stream_CameraAR_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_TaskBaseAsynch_t;

typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Stream_CameraAR_Message_t,
                                      Stream_CameraAR_SessionMessage_t,
                                      struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Stream_CameraAR_StreamState,
                                      struct Stream_CameraAR_StatisticData,
                                      Test_I_V4L_SessionManager_t,
                                      Common_Timer_Manager_t,
                                      struct Stream_UserData> Stream_CameraAR_V4L_Source;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Stream_CameraAR_LibAVConvert;

typedef Stream_Visualization_LibAVResize_T<Test_U_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_V4L_MediaType> Stream_CameraAR_LibAVResize;
#endif // FFMPEG_SUPPORT

typedef Stream_Decoder_RGB24_HFlip_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Stream_CameraAR_Message_t,
                                     Stream_CameraAR_SessionMessage_t,
                                     Stream_CameraAR_V4L_SessionData_t,
                                     struct Stream_MediaFramework_V4L_MediaType> Stream_CameraAR_V4L_HFlip;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraAR_DirectShow_Message_t,
                                                      Stream_CameraAR_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraAR_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_CameraAR_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraAR_DirectShow_Message_t,
                                                      Stream_CameraAR_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraAR_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_CameraAR_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraAR_MediaFoundation_Message_t,
                                                      Stream_CameraAR_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraAR_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_CameraAR_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraAR_MediaFoundation_Message_t,
                                                      Stream_CameraAR_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraAR_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_CameraAR_MediaFoundation_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraAR_Message_t,
                                                      Stream_CameraAR_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraAR_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_CameraAR_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_CameraAR_Message_t,
                                                      Stream_CameraAR_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CameraAR_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_CameraAR_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_CameraAR_DirectShow_Message_t,
//                                       Stream_CameraAR_DirectShow_SessionMessage_t,
//                                       Stream_CameraAR_DirectShow_SessionData,
//                                       struct Stream_UserData> Stream_CameraAR_DirectShow_MessageHandler;
//
//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_CameraAR_MediaFoundation_Message_t,
//                                       Stream_CameraAR_MediaFoundation_SessionMessage_t,
//                                       Stream_CameraAR_MediaFoundation_SessionData,
//                                       struct Stream_UserData> Stream_CameraAR_MediaFoundation_MessageHandler;

#if defined (OLC_CGE_SUPPORT)
typedef Test_I_CameraAR_Module_CGE_T<Test_U_DirectShow_TaskBaseAsynch_t,
                                     struct _AMMediaType> Stream_CameraAR_DirectShow_CGE;
typedef Test_I_CameraAR_Module_CGE_T<Test_U_MediaFoundation_TaskBaseAsynch_t,
                                     IMFMediaType*> Stream_CameraAR_MediaFoundation_CGE;
#endif // OLC_CGE_SUPPORT
#if defined (OLC_PGE_SUPPORT)
typedef Test_I_CameraAR_Module_PGE_T<Test_U_DirectShow_TaskBaseAsynch_t,
                                     struct _AMMediaType> Stream_CameraAR_DirectShow_PGE;
typedef Test_I_CameraAR_Module_PGE_T<Test_U_MediaFoundation_TaskBaseAsynch_t,
                                     IMFMediaType*> Stream_CameraAR_MediaFoundation_PGE;
#endif // OLC_PGE_SUPPORT
#else
//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_CameraAR_Message_t,
//                                       Stream_CameraAR_SessionMessage_t,
//                                       Stream_CameraAR_V4L_SessionData,
//                                       struct Stream_UserData> Stream_CameraAR_MessageHandler;

#if defined (OLC_PGE_SUPPORT)
typedef Test_I_CameraAR_Module_PGE_T<Test_U_TaskBaseAsynch_t,
                                    struct Stream_MediaFramework_V4L_MediaType> Stream_CameraAR_PGE;
#endif // OLC_PGE_SUPPORT

#if defined (OLC_CGE_SUPPORT)
typedef Test_I_CameraAR_Module_CGE_T<Test_U_TaskBaseAsynch_t,
                                     struct Stream_MediaFramework_V4L_MediaType> Stream_CameraAR_CGE;
#endif // OLC_CGE_SUPPORT

#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_DirectShow_Source);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_MediaFoundation_Source);           // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_DirectShow_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_MediaFoundation_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_DirectShow_LibAVResize);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_MediaFoundation_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_rgb24_hflip_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Stream_CameraAR_DirectShow_HFlip);                            // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_rgb24_hflip_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Stream_CameraAR_MediaFoundation_HFlip);                            // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_V4L_Source);                       // writer type

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_LibAVConvert);                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CameraAR_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                          // session event type
                              struct Stream_CameraAR_V4L_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_dec_rgb24_hflip_module_name_string,
                              Stream_INotify_t,                                        // stream notification interface type
                              Stream_CameraAR_V4L_HFlip);                              // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Stream_CameraAR_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CameraAR_DirectShow_Statistic_ReaderTask_t, // reader type
                          Stream_CameraAR_DirectShow_Statistic_WriterTask_t, // writer type
                          Stream_CameraAR_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Stream_CameraAR_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CameraAR_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Stream_CameraAR_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Stream_CameraAR_MediaFoundation_StatisticReport);  // name
#else
DATASTREAM_MODULE_DUPLEX (Stream_CameraAR_V4L_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CameraAR_Statistic_ReaderTask_t,            // reader type
                          Stream_CameraAR_Statistic_WriterTask_t,            // writer type
                          Stream_CameraAR_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_CameraAR_DirectShow_MessageHandler);        // writer type
//
//DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                   // session event type
//                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                 // stream notification interface type
//                              Stream_CameraAR_MediaFoundation_MessageHandler);   // writer type

#if defined (OLC_CGE_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_cge_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Stream_CameraAR_DirectShow_CGE);                              // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_cge_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Stream_CameraAR_MediaFoundation_CGE);                              // writer type
#endif // OLC_CGE_SUPPORT
#if defined (OLC_PGE_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Stream_CameraAR_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_pge_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Stream_CameraAR_DirectShow_PGE);                              // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_MediaFoundation_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                               // session event type
                              struct Stream_CameraAR_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_pge_module_name_string,
                              Stream_INotify_t,                                             // stream notification interface type
                              Stream_CameraAR_MediaFoundation_PGE);                              // writer type
#endif // OLC_PGE_SUPPORT
#else
//DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                           // session data type
//                              enum Stream_SessionMessageType,                       // session event type
//                              struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                     // stream notification interface type
//                              Stream_CameraAR_MessageHandler);                       // writer type

#if defined (OLC_PGE_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                       // session data type
                             enum Stream_SessionMessageType,                        // session event type
                             struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
                             libacestream_default_pge_module_name_string,
                             Stream_INotify_t,                                      // stream notification interface type
                             Stream_CameraAR_PGE);                                  // writer type
#endif // OLC_PGE_SUPPORT

#if defined (OLC_CGE_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CameraAR_V4L_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                        // session event type
                              struct Stream_CameraAR_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_cge_module_name_string,
                              Stream_INotify_t,                                      // stream notification interface type
                              Stream_CameraAR_CGE);                                  // writer type
#endif // OLC_CGE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
