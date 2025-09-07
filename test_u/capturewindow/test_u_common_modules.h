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
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_encoder.h"
#endif // FFMPEG_SUPPORT

#include "stream_misc_defines.h"
#include "stream_misc_distributor.h"
#include "stream_misc_messagehandler.h"
#include "stream_misc_window_source.h"

#include "stream_stat_statistic_report.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_vis_libav_resize.h"
#endif // FFMPEG_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_gdi.h"
#include "stream_vis_target_direct2d.h"
#else
#include "stream_vis_wayland_window.h"
#include "stream_vis_x11_window.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_capturewindow_common.h"
#include "test_u_message.h"
#include "test_u_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_U_CaptureWindow_DirectShow_SessionData,
                                 struct Test_U_StatisticData,
                                 struct Stream_UserData> Test_U_DirectShow_SessionManager_t;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 Test_U_CaptureWindow_MediaFoundation_SessionData,
                                 struct Test_U_StatisticData,
                                 struct Stream_UserData> Test_U_MediaFoundation_SessionManager_t;

typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_DirectShow_Message_t,
                               Test_U_DirectShow_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_DirectShow_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_DirectShow_Message_t,
                                Test_U_DirectShow_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_DirectShow_TaskBaseAsynch_t;

typedef Stream_Module_Window_Source_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Test_U_DirectShow_Message_t,
                                      Test_U_DirectShow_SessionMessage_t,
                                      struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_U_DirectShow_StreamState,
                                      struct Test_U_StatisticData,
                                      Test_U_DirectShow_SessionManager_t,
                                      Common_Timer_Manager_t,
                                      struct _AMMediaType> Test_U_DirectShow_WindowSource;

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message_t,
                                                      Test_U_DirectShow_SessionMessage_t,
                                                      Test_U_CaptureWindow_DirectShow_SessionData_t> Test_U_DirectShow_Distributor_WriterTask_t;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_DirectShow_TaskBaseSynch_t,
                                        struct _AMMediaType> Test_U_DirectShow_LibAVConvert;

typedef Stream_Decoder_LibAVEncoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_U_DirectShow_Message_t,
                                      Test_U_DirectShow_SessionMessage_t,
                                      Test_U_CaptureWindow_DirectShow_SessionData_t,
                                      struct _AMMediaType> Test_U_DirectShow_LibAVEncoder;

typedef Stream_Visualization_LibAVResize_T<Test_U_DirectShow_TaskBaseSynch_t,
                                           struct _AMMediaType> Test_U_DirectShow_LibAVResize;
#endif // FFMPEG_SUPPORT
#else
typedef Stream_TaskBaseSynch_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_U_Message_t,
                               Test_U_SessionMessage_t,
                               enum Stream_ControlType,
                               enum Stream_SessionMessageType,
                               struct Stream_UserData> Test_U_TaskBaseSynch_t;
typedef Stream_TaskBaseAsynch_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_Message_t,
                                Test_U_SessionMessage_t,
                                enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                struct Stream_UserData> Test_U_TaskBaseAsynch_t;

typedef Stream_Module_Window_Source_T<ACE_MT_SYNCH,
                                      Stream_ControlMessage_t,
                                      Test_U_Message_t,
                                      Test_U_SessionMessage_t,
                                      struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_U_StreamState,
                                      Test_U_CaptureWindow_SessionData,
                                      Test_U_CaptureWindow_SessionData_t,
                                      struct Test_U_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_WindowSource;

typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message_t,
                                                      Test_U_SessionMessage_t,
                                                      Test_U_CaptureWindow_SessionData_t> Test_U_Distributor_WriterTask_t;

#if defined (FFMPEG_SUPPORT)
typedef Stream_Decoder_LibAVConverter_T<Test_U_TaskBaseSynch_t,
                                        struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_LibAVConvert;

typedef Stream_Decoder_LibAVEncoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                      Stream_ControlMessage_t,
                                      Test_U_Message_t,
                                      Test_U_SessionMessage_t,
                                      Test_U_CaptureWindow_SessionData_t,
                                      struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_LibAVEncoder;

typedef Stream_Visualization_LibAVResize_T<Test_U_TaskBaseSynch_t,
                                           struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_LibAVResize;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message_t,
                                                      Test_U_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CaptureWindow_DirectShow_SessionData,
                                                      Test_U_CaptureWindow_DirectShow_SessionData_t> Test_U_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_DirectShow_Message_t,
                                                      Test_U_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CaptureWindow_DirectShow_SessionData,
                                                      Test_U_CaptureWindow_DirectShow_SessionData_t> Test_U_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_MediaFoundation_Message_t,
                                                      Test_U_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CaptureWindow_MediaFoundation_SessionData,
                                                      Test_U_CaptureWindow_MediaFoundation_SessionData_t> Test_U_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_MediaFoundation_Message_t,
                                                      Test_U_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CaptureWindow_MediaFoundation_SessionData,
                                                      Test_U_CaptureWindow_MediaFoundation_SessionData_t> Test_U_MediaFoundation_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message_t,
                                                      Test_U_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CaptureWindow_SessionData,
                                                      Test_U_CaptureWindow_SessionData_t> Test_U_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_U_Message_t,
                                                      Test_U_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Test_U_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_U_CaptureWindow_SessionData,
                                                      Test_U_CaptureWindow_SessionData_t> Test_U_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct2D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlMessage_t,
                                     Test_U_DirectShow_Message_t,
                                     Test_U_DirectShow_SessionMessage_t,
                                     Test_U_CaptureWindow_DirectShow_SessionData,
                                     Test_U_CaptureWindow_DirectShow_SessionData_t,
                                     struct _AMMediaType> Test_U_DirectShow_Direct2D_Display;

typedef Stream_Vis_Target_GDI_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_DirectShow_Message_t,
                                Test_U_DirectShow_SessionMessage_t,
                                Test_U_CaptureWindow_DirectShow_SessionData,
                                Test_U_CaptureWindow_DirectShow_SessionData_t,
                                struct _AMMediaType> Test_U_DirectShow_GDI_Display;

typedef Stream_Vis_Target_GDI_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration,
                                Stream_ControlMessage_t,
                                Test_U_MediaFoundation_Message_t,
                                Test_U_MediaFoundation_SessionMessage_t,
                                Test_U_CaptureWindow_MediaFoundation_SessionData,
                                Test_U_CaptureWindow_MediaFoundation_SessionData_t,
                                IMFMediaType*> Test_U_MediaFoundation_GDI_Display;
#else
typedef Stream_Module_Vis_Wayland_Window_T<ACE_MT_SYNCH,
                                           Common_TimePolicy_t,
                                           struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                           Stream_ControlMessage_t,
                                           Test_U_Message_t,
                                           Test_U_SessionMessage_t,
                                           Test_U_CaptureWindow_SessionData_t,
                                           struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_Wayland_Display;
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message_t,
                                       Test_U_SessionMessage_t,
                                       Test_U_CaptureWindow_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_VideoMediaType> Test_U_X11_Display;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_DirectShow_Message_t,
                                       Test_U_DirectShow_SessionMessage_t,
                                       Test_U_CaptureWindow_DirectShow_SessionData,
                                       struct Stream_UserData> Test_U_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_MediaFoundation_Message_t,
                                       Test_U_MediaFoundation_SessionMessage_t,
                                       Test_U_CaptureWindow_MediaFoundation_SessionData,
                                       struct Stream_UserData> Test_U_MediaFoundation_MessageHandler;
#else
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_U_Message_t,
                                       Test_U_SessionMessage_t,
                                       Test_U_CaptureWindow_SessionData,
                                       struct Stream_UserData> Test_U_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_window_source_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_U_DirectShow_WindowSource);                        // writer type

DATASTREAM_MODULE_DUPLEX (Test_U_CaptureWindow_DirectShow_SessionData,                            // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                          // stream notification interface type
                          Test_U_DirectShow_Distributor_WriterTask_t::READER_TASK_T, // reader type
                          Test_U_DirectShow_Distributor_WriterTask_t,                // writer type
                          Test_U_DirectShow_Distributor);                            // name

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,                    // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_LibAVConvert);                  // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,      // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_encoder_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_LibAVEncoder);                  // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,                   // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_LibAVResize);                      // writer type
#endif // FFMPEG_SUPPORT
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_misc_window_source_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_WindowSource);                                      // writer type

DATASTREAM_MODULE_DUPLEX (Test_U_CaptureWindow_SessionData,                         // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                                         // stream notification interface type
                          Test_U_Distributor_WriterTask_t::READER_TASK_T,           // reader type
                          Test_U_Distributor_WriterTask_t,                          // writer type
                          Test_U_Distributor);                                      // name

#if defined (FFMPEG_SUPPORT)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_U_LibAVConvert);                                        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                            // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,  // module handler configuration type
                              libacestream_default_dec_libav_encoder_module_name_string,
                              Stream_INotify_t,                                          // stream notification interface type
                              Test_U_LibAVEncoder);                                      // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_libav_resize_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_U_LibAVResize);                                      // writer type
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_U_CaptureWindow_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_U_DirectShow_Statistic_ReaderTask_t, // reader type
                          Test_U_DirectShow_Statistic_WriterTask_t, // writer type
                          Test_U_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Test_U_CaptureWindow_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Test_U_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Test_U_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Test_U_MediaFoundation_StatisticReport);  // name
#else
DATASTREAM_MODULE_DUPLEX (Test_U_CaptureWindow_SessionData,                         // session data type
                          enum Stream_SessionMessageType,                           // session event type
                          struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                         // stream notification interface type
                          Test_U_Statistic_ReaderTask_t,                            // reader type
                          Test_U_Statistic_WriterTask_t,                            // writer type
                          Test_U_StatisticReport);                                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                                    // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct2d_module_name_string,
                              Stream_INotify_t,                                                  // stream notification interface type
                              Test_U_DirectShow_Direct2D_Display);                               // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gdi_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_GDI_Display);                 // writer type


DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_MediaFoundation_SessionData,       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gdi_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_GDI_Display); // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_wayland_window_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_U_Wayland_Display);                                    // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                         // session data type
                              enum Stream_SessionMessageType,                           // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                         // stream notification interface type
                              Test_U_X11_Display);                                      // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_DirectShow_MessageHandler);        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Test_U_MediaFoundation_MessageHandler);   // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_U_CaptureWindow_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration,    // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_U_MessageHandler);                                      // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
