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

#ifndef TEST_U_CAMSAVE_COMMON_MODULES_H
#define TEST_U_CAMSAVE_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// *NOTE*: wxWidgets may have #defined __WXDEBUG__
//#undef __WXDEBUG__
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
#else
#include "stream_dec_libav_converter.h"
#include "stream_dec_libav_decoder.h"

#include "stream_dev_cam_source_v4l.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_dec_avi_encoder.h"

#include "stream_file_sink.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GTK_USE)
#include "stream_vis_gtk_cairo.h"
#elif defined (WXWIDGETS_USE)
#include "stream_vis_x11_window.h"
#endif
#endif // GUI_SUPPORT

#include "test_u_camsave_common.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Test_U_ControlMessage_t,
                                           Stream_CamSave_DirectShow_Message_t,
                                           Stream_CamSave_DirectShow_SessionMessage_t,
                                           struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Stream_CamSave_StreamState,
                                           Stream_CamSave_SessionData,
                                           Stream_CamSave_SessionData_t,
                                           struct Stream_CamSave_StatisticData,
                                           Common_Timer_Manager_t,
                                           struct Stream_CamSave_UserData> Stream_CamSave_DirectShow_Source;

typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Test_U_ControlMessage_t,
                                                Stream_CamSave_MediaFoundation_Message_t,
                                                Stream_CamSave_MediaFoundation_SessionMessage_t,
                                                struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Stream_CamSave_StreamState,
                                                Stream_CamSave_SessionData,
                                                Stream_CamSave_SessionData_t,
                                                struct Stream_CamSave_StatisticData,
                                                Common_Timer_Manager_t,
                                                struct Stream_CamSave_UserData> Stream_CamSave_MediaFoundation_Source;
#else
typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_Message_t,
                                      Stream_CamSave_SessionMessage_t,
                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Stream_CamSave_StreamState,
                                      Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t,
                                      struct Stream_CamSave_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Stream_CamSave_UserData> Stream_CamSave_V4L_Source;

typedef Stream_Decoder_LibAVConverter_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                        Test_U_ControlMessage_t,
                                        Stream_CamSave_Message_t,
                                        Stream_CamSave_SessionMessage_t,
                                        Stream_CamSave_SessionData_t,
                                        struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_LibAVConverter;
typedef Stream_Decoder_LibAVDecoder_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_Message_t,
                                      Stream_CamSave_SessionMessage_t,
                                      Stream_CamSave_SessionData_t> Stream_CamSave_LibAVDecoder;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Stream_CamSave_DirectShow_Message_t,
                                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_SessionData,
                                                      Stream_CamSave_SessionData_t> Stream_CamSave_DirectShow_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Stream_CamSave_DirectShow_Message_t,
                                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_SessionData,
                                                      Stream_CamSave_SessionData_t> Stream_CamSave_DirectShow_Statistic_WriterTask_t;

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Stream_CamSave_MediaFoundation_Message_t,
                                                      Stream_CamSave_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_SessionData,
                                                      Stream_CamSave_SessionData_t> Stream_CamSave_MediaFoundation_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Stream_CamSave_MediaFoundation_Message_t,
                                                      Stream_CamSave_MediaFoundation_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_SessionData,
                                                      Stream_CamSave_SessionData_t> Stream_CamSave_MediaFoundation_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Stream_CamSave_Message_t,
                                                      Stream_CamSave_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_SessionData,
                                                      Stream_CamSave_SessionData_t> Stream_CamSave_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                                      Test_U_ControlMessage_t,
                                                      Stream_CamSave_Message_t,
                                                      Stream_CamSave_SessionMessage_t,
                                                      Stream_CommandType_t,
                                                      struct Stream_CamSave_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Stream_CamSave_SessionData,
                                                      Stream_CamSave_SessionData_t> Stream_CamSave_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_DirectShow_Message_t,
                                               Stream_CamSave_DirectShow_SessionMessage_t,
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData,
                                               struct _AMMediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_DirectShow_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_DirectShow_Message_t,
                                               Stream_CamSave_DirectShow_SessionMessage_t,
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData,
                                               struct _AMMediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_DirectShow_AVIEncoder_WriterTask_t;

typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_MediaFoundation_Message_t,
                                               Stream_CamSave_MediaFoundation_SessionMessage_t,
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData,
                                               IMFMediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_MediaFoundation_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_MediaFoundation_Message_t,
                                               Stream_CamSave_MediaFoundation_SessionMessage_t,
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData,
                                               IMFMediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_MediaFoundation_AVIEncoder_WriterTask_t;
#else
typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_Message_t,
                                               Stream_CamSave_SessionMessage_t,
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData,
                                               struct Stream_MediaFramework_V4L_MediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_AVIEncoder_ReaderTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_Message_t,
                                               Stream_CamSave_SessionMessage_t,
                                               Stream_CamSave_SessionData_t,
                                               Stream_CamSave_SessionData,
                                               struct Stream_MediaFramework_V4L_MediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_V4L2_AVIEncoder_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                     Test_U_ControlMessage_t,
                                     Stream_CamSave_DirectShow_Message_t,
                                     Stream_CamSave_DirectShow_SessionMessage_t,
                                     Stream_CamSave_SessionData,
                                     Stream_CamSave_SessionData_t> Stream_CamSave_DirectShow_Direct3DDisplay;
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                     Test_U_ControlMessage_t,
                                     Stream_CamSave_MediaFoundation_Message_t,
                                     Stream_CamSave_MediaFoundation_SessionMessage_t,
                                     Stream_CamSave_SessionData,
                                     Stream_CamSave_SessionData_t> Stream_CamSave_MediaFoundation_Direct3DDisplay;

struct Stream_CamSave_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Stream_CamSave_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Common_TimePolicy_t,
                                                         Stream_CamSave_DirectShow_SessionMessage_t,
                                                         Stream_CamSave_DirectShow_Message_t,
                                                         struct Stream_CamSave_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                         struct _AMMediaType> Stream_CamSave_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
                                                                Stream_CamSave_DirectShow_SessionMessage_t,
                                                                Stream_CamSave_DirectShow_Message_t,
                                                                struct Stream_CamSave_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                                struct _AMMediaType> Stream_CamSave_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                       Test_U_ControlMessage_t,
                                       Stream_CamSave_DirectShow_Message_t,
                                       Stream_CamSave_DirectShow_SessionMessage_t,
                                       Stream_CamSave_SessionData_t,
                                       Stream_CamSave_SessionData,
                                       struct Stream_CamSave_DirectShow_FilterConfiguration,
                                       struct Stream_CamSave_DirectShow_PinConfiguration,
                                       Stream_CamSave_DirectShowFilter_t> Stream_CamSave_DirectShow_DirectShowDisplay;

typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_U_ControlMessage_t,
                                            Stream_CamSave_MediaFoundation_Message_t,
                                            Stream_CamSave_MediaFoundation_SessionMessage_t,
                                            Stream_CamSave_SessionData,
                                            Stream_CamSave_SessionData_t,
                                            struct Stream_CamSave_UserData> Stream_CamSave_MediaFoundation_MediaFoundationDisplay;
typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_U_ControlMessage_t,
                                            Stream_CamSave_MediaFoundation_Message_t,
                                            Stream_CamSave_MediaFoundation_SessionMessage_t,
                                            Stream_CamSave_SessionData,
                                            Stream_CamSave_SessionData_t> Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull;

#if defined (GTK_USE)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_DirectShow_Message_t,
                                      Stream_CamSave_DirectShow_SessionMessage_t,
                                      Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t> Stream_CamSave_DirectShow_GTKCairoDisplay;
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_MediaFoundation_Message_t,
                                      Stream_CamSave_MediaFoundation_SessionMessage_t,
                                      Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t> Stream_CamSave_MediaFoundation_GTKCairoDisplay;
#endif // GTK_USE
#else
#if defined (GTK_USE)
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_Message_t,
                                      Stream_CamSave_SessionMessage_t,
                                      Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t,
                                      struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_GTKCairoDisplay;
#elif defined (WXWIDGETS_USE)
typedef Stream_Module_Vis_X11_Window_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                       Test_U_ControlMessage_t,
                                       Stream_CamSave_Message_t,
                                       Stream_CamSave_SessionMessage_t,
                                       Stream_CamSave_SessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Stream_CamSave_X11WindowDisplay;
#endif
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                   Test_U_ControlMessage_t,
                                   Stream_CamSave_DirectShow_Message_t,
                                   Stream_CamSave_DirectShow_SessionMessage_t,
                                   Stream_CamSave_SessionData> Stream_CamSave_DirectShow_FileWriter;

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                   Test_U_ControlMessage_t,
                                   Stream_CamSave_MediaFoundation_Message_t,
                                   Stream_CamSave_MediaFoundation_SessionMessage_t,
                                   Stream_CamSave_SessionData> Stream_CamSave_MediaFoundation_FileWriter;
#else
typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                   Test_U_ControlMessage_t,
                                   Stream_CamSave_Message_t,
                                   Stream_CamSave_SessionMessage_t,
                                   Stream_CamSave_SessionData> Stream_CamSave_FileWriter;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration,
                                       Test_U_ControlMessage_t,
                                       Stream_CamSave_DirectShow_Message_t,
                                       Stream_CamSave_DirectShow_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_CamSave_SessionData,
                                       struct Stream_CamSave_UserData> Stream_CamSave_DirectShow_MessageHandler;

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration,
                                       Test_U_ControlMessage_t,
                                       Stream_CamSave_MediaFoundation_Message_t,
                                       Stream_CamSave_MediaFoundation_SessionMessage_t,
                                       Stream_SessionId_t,
                                       Stream_CamSave_SessionData,
                                       struct Stream_CamSave_UserData> Stream_CamSave_MediaFoundation_MessageHandler;
#else
typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Stream_CamSave_V4L_ModuleHandlerConfiguration,
                                       Test_U_ControlMessage_t,
                                       Stream_CamSave_Message_t,
                                       Stream_CamSave_SessionMessage_t,
                                       Stream_SessionId_t,
                                       struct Stream_CamSave_SessionData,
                                       struct Stream_CamSave_UserData> Stream_CamSave_MessageHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_Source);                // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_Source);           // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_V4L_Source);                       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibAVConverter);                   // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dec_libav_converter_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_LibAVDecoder);                     // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_DirectShow_Statistic_ReaderTask_t, // reader type
                          Stream_CamSave_DirectShow_Statistic_WriterTask_t, // writer type
                          Stream_CamSave_DirectShow_StatisticReport);       // name

DATASTREAM_MODULE_DUPLEX (Stream_CamSave_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_MediaFoundation_Statistic_ReaderTask_t, // reader type
                          Stream_CamSave_MediaFoundation_Statistic_WriterTask_t, // writer type
                          Stream_CamSave_MediaFoundation_StatisticReport);  // name
#else
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_Statistic_ReaderTask_t,            // reader type
                          Stream_CamSave_Statistic_WriterTask_t,            // writer type
                          Stream_CamSave_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_SessionData,                           // session data type
                          enum Stream_SessionMessageType,                              // session event type
                          struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                            // stream notification interface type
                          Stream_CamSave_DirectShow_AVIEncoder_ReaderTask_t,           // reader type
                          Stream_CamSave_DirectShow_AVIEncoder_WriterTask_t,           // writer type
                          Stream_CamSave_DirectShow_AVIEncoder);                       // name

DATASTREAM_MODULE_DUPLEX (Stream_CamSave_SessionData,                                // session data type
                          enum Stream_SessionMessageType,                                   // session event type
                          struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                                 // stream notification interface type
                          Stream_CamSave_MediaFoundation_AVIEncoder_ReaderTask_t,           // reader type
                          Stream_CamSave_MediaFoundation_AVIEncoder_WriterTask_t,           // writer type
                          Stream_CamSave_MediaFoundation_AVIEncoder);                       // name
#else
DATASTREAM_MODULE_DUPLEX (Stream_CamSave_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_dec_avi_encoder_module_name_string,
                          Stream_INotify_t,                                 // stream notification interface type
                          Stream_CamSave_AVIEncoder_ReaderTask_t,           // reader type
                          Stream_CamSave_V4L2_AVIEncoder_WriterTask_t,      // writer type
                          Stream_CamSave_V4L2_AVIEncoder);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_Direct3DDisplay);       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_Direct3DDisplay);  // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_DirectShowDisplay);     // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                      // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Stream_CamSave_MediaFoundation_MediaFoundationDisplay); // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                          // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Stream_CamSave_MediaFoundation_MediaFoundationDisplayNull); // writer type

#if defined (GTK_USE)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_GTKCairoDisplay);       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_GTKCairoDisplay);  // writer type
#endif // GTK_USE
#else
#if defined (GTK_USE)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_gtk_cairo_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_CamSave_GTKCairoDisplay);                      // writer type
#elif defined (WXWIDGETS_USE)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_x11_window_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_X11WindowDisplay);                 // writer type
#endif
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_FileWriter);            // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_FileWriter);       // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_FileWriter);                       // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_DirectShow_MessageHandler);        // writer type

DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                 // stream notification interface type
                              Stream_CamSave_MediaFoundation_MessageHandler);   // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Stream_CamSave_SessionData,                           // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Stream_CamSave_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Stream_CamSave_MessageHandler);                       // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
