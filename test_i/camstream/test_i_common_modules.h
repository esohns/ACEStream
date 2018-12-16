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

#ifndef TEST_I_COMMON_MODULES_H
#define TEST_I_COMMON_MODULES_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <strmif.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"

#include "stream_lib_directshow_asynch_source_filter.h"
#include "stream_lib_directshow_source_filter.h"
#include "stream_lib_directshow_target.h"
#include "stream_lib_mediafoundation_target.h"

#include "stream_vis_target_direct3d.h"
#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#else
#include "stream_dev_cam_source_v4l.h"

#include "stream_lib_v4l_common.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "stream_vis_gtk_pixbuf.h"
#endif // GTK_USE
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_lib_ffmpeg_common.h"

#include "stream_misc_splitter.h"

#include "stream_net_io.h"

#include "stream_stat_statistic_report.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"

#include "test_i_module_eventhandler.h"

#include "test_i_source_common.h"
#include "test_i_source_message.h"
#include "test_i_source_session_message.h"

#include "test_i_target_common.h"
#include "test_i_target_message.h"
#include "test_i_target_session_message.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
// forward declarations
struct Test_I_Target_DirectShow_FilterConfiguration;
struct Test_I_Target_DirectShow_PinConfiguration;

// source
typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Test_I_ControlMessage_t,
                                           Test_I_Source_DirectShow_Stream_Message,
                                           Test_I_Source_DirectShow_SessionMessage,
                                           struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                           enum Stream_ControlType,
                                           enum Stream_SessionMessageType,
                                           struct Test_I_Source_DirectShow_StreamState,
                                           Test_I_Source_DirectShow_SessionData,
                                           Test_I_Source_DirectShow_SessionData_t,
                                           struct Test_I_Source_Stream_StatisticData,
                                           Common_Timer_Manager_t,
                                           struct Test_I_Source_DirectShow_UserData> Test_I_Stream_DirectShow_CamSource;
typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Test_I_ControlMessage_t,
                                                Test_I_Source_MediaFoundation_Stream_Message,
                                                Test_I_Source_MediaFoundation_SessionMessage,
                                                struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Test_I_Source_MediaFoundation_StreamState,
                                                Test_I_Source_MediaFoundation_SessionData,
                                                Test_I_Source_MediaFoundation_SessionData_t,
                                                struct Test_I_Source_Stream_StatisticData,
                                                Common_Timer_Manager_t,
                                                struct Test_I_Source_MediaFoundation_UserData> Test_I_Stream_MediaFoundation_CamSource;
#else
typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Test_I_ControlMessage_t,
                                      Test_I_Source_V4L_Stream_Message,
                                      Test_I_Source_V4L_SessionMessage,
                                      struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Test_I_Source_V4L_StreamState,
                                      Test_I_Source_V4L_SessionData,
                                      Test_I_Source_V4L_SessionData_t,
                                      struct Test_I_Source_Stream_StatisticData,
                                      Common_Timer_Manager_t,
                                      struct Test_I_Source_V4L_UserData> Test_I_Source_V4L_CamSource;
#endif // ACE_WIN32 || ACE_WIN64

//typedef Stream_Decoder_AVIDecoder_T<Test_I_Target_SessionMessage,
//                                    Test_I_Target_Stream_Message,
//                                    struct Test_I_Target_ModuleHandlerConfiguration,
//                                    Test_I_Target_SessionData> Test_I_Target_Stream_AVIDecoder;
//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
//                              Common_TimePolicy_t,                             // time policy
//                              struct Stream_ModuleConfiguration,               // module configuration type
//                              struct Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_Target_Stream_AVIDecoder);                // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                 Test_I_ControlMessage_t,
                                 Test_I_Target_DirectShow_Stream_Message,
                                 Test_I_Target_DirectShow_SessionMessage,
                                 Test_I_Target_DirectShow_SessionData,
                                 struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_Target_DirectShow_Splitter;
typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                 Test_I_ControlMessage_t,
                                 Test_I_Target_MediaFoundation_Stream_Message,
                                 Test_I_Target_MediaFoundation_SessionMessage,
                                 Test_I_Target_MediaFoundation_SessionData,
                                 struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_Target_MediaFoundation_Splitter;
#else
typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Test_I_Target_ModuleHandlerConfiguration,
                                 Test_I_ControlMessage_t,
                                 Test_I_Target_Stream_Message,
                                 Test_I_Target_SessionMessage,
                                 Test_I_Target_SessionData,
                                 struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_Target_Splitter;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_I_ControlMessage_t,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_SessionMessage,
                                     struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_Source_DirectShow_StreamState,
                                     Test_I_Source_DirectShow_SessionData,
                                     Test_I_Source_DirectShow_SessionData_t,
                                     struct Test_I_Source_Stream_StatisticData,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_DirectShow_InetConnectionManager_t,
                                     struct Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Test_I_ControlMessage_t,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_SessionMessage,
                                     struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Stream_State,
                                     Test_I_Source_DirectShow_SessionData,
                                     Test_I_Source_DirectShow_SessionData_t,
                                     struct Test_I_Source_Stream_StatisticData,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_DirectShow_InetConnectionManager_t,
                                     struct Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_Module_Net_Reader_t;
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_I_ControlMessage_t,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_SessionMessage,
                                     struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_Source_MediaFoundation_StreamState,
                                     Test_I_Source_MediaFoundation_SessionData,
                                     Test_I_Source_MediaFoundation_SessionData_t,
                                     struct Test_I_Source_Stream_StatisticData,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                     struct Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Test_I_ControlMessage_t,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_SessionMessage,
                                     struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_Source_MediaFoundation_StreamState,
                                     Test_I_Source_MediaFoundation_SessionData,
                                     Test_I_Source_MediaFoundation_SessionData_t,
                                     struct Test_I_Source_Stream_StatisticData,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_MediaFoundation_InetConnectionManager_t,
                                     struct Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_Module_Net_Reader_t;
#else
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_I_ControlMessage_t,
                                     Test_I_Source_V4L_Stream_Message,
                                     Test_I_Source_V4L_SessionMessage,
                                     struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_Source_V4L_StreamState,
                                     Test_I_Source_V4L_SessionData,
                                     Test_I_Source_V4L_SessionData_t,
                                     struct Test_I_Source_Stream_StatisticData,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_V4L_InetConnectionManager_t,
                                     struct Test_I_Source_V4L_UserData> Test_I_Source_V4L_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Test_I_ControlMessage_t,
                                     Test_I_Source_V4L_Stream_Message,
                                     Test_I_Source_V4L_SessionMessage,
                                     struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_Source_V4L_StreamState,
                                     Test_I_Source_V4L_SessionData,
                                     Test_I_Source_V4L_SessionData_t,
                                     struct Test_I_Source_Stream_StatisticData,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_V4L_InetConnectionManager_t,
                                     struct Test_I_Source_V4L_UserData> Test_I_Source_V4L_Net_Reader_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Source_DirectShow_Stream_Message,
                                                      Test_I_Source_DirectShow_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      struct Test_I_Source_Stream_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Source_DirectShow_SessionData,
                                                      Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Source_DirectShow_Stream_Message,
                                                      Test_I_Source_DirectShow_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      struct Test_I_Source_Stream_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Source_DirectShow_SessionData,
                                                      Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Module_Statistic_WriterTask_t;
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Source_MediaFoundation_Stream_Message,
                                                      Test_I_Source_MediaFoundation_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      struct Test_I_Source_Stream_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Source_MediaFoundation_SessionData,
                                                      Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Source_MediaFoundation_Stream_Message,
                                                      Test_I_Source_MediaFoundation_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      struct Test_I_Source_Stream_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Source_MediaFoundation_SessionData,
                                                      Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Source_V4L_Stream_Message,
                                                      Test_I_Source_V4L_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      struct Test_I_Source_Stream_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Source_V4L_SessionData,
                                                      Test_I_Source_V4L_SessionData_t> Test_I_Source_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Source_V4L_Stream_Message,
                                                      Test_I_Source_V4L_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      struct Test_I_Source_Stream_StatisticData,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Source_V4L_SessionData,
                                                      Test_I_Source_V4L_SessionData_t> Test_I_Source_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Vis_DirectShow_Target_Direct3D_T<ACE_MT_SYNCH,
//                                                Common_TimePolicy_t,
//                                                struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
//                                                Test_I_ControlMessage_t,
//                                                Test_I_Source_DirectShow_Stream_Message,
//                                                Test_I_Source_DirectShow_SessionMessage,
//                                                Test_I_Source_DirectShow_SessionData,
//                                                Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Direct3D_Display;

struct Test_I_Source_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Test_I_Source_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   //, format (NULL)
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  // *TODO*: specify this as part of the network protocol header/handshake
  //struct _AMMediaType*                                           format; // handle
  Stream_Module_t*                                                module; // handle
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration; // handle
};
typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Common_TimePolicy_t,
                                                         Test_I_Source_DirectShow_SessionMessage,
                                                         Test_I_Source_DirectShow_Stream_Message,
                                                         struct Test_I_Source_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                         struct _AMMediaType> Test_I_Source_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
                                                                Test_I_Source_DirectShow_SessionMessage,
                                                                Test_I_Source_DirectShow_Stream_Message,
                                                                struct Test_I_Source_DirectShow_FilterConfiguration,
                                                                struct Test_I_Source_DirectShow_FilterPinConfiguration,
                                                                struct _AMMediaType> Test_I_Source_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                       Test_I_ControlMessage_t,
                                       Test_I_Source_DirectShow_Stream_Message,
                                       Test_I_Source_DirectShow_SessionMessage,
                                       Test_I_Source_DirectShow_SessionData_t,
                                       Test_I_Source_DirectShow_SessionData,
                                       struct Test_I_Source_DirectShow_FilterConfiguration,
                                       struct Test_I_Source_DirectShow_PinConfiguration,
                                       Test_I_Source_DirectShowFilter_t> Test_I_Source_DirectShow_Display;
typedef Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_MT_SYNCH,
                                                     Common_TimePolicy_t,
                                                     struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                                     Test_I_ControlMessage_t,
                                                     Test_I_Source_MediaFoundation_Stream_Message,
                                                     Test_I_Source_MediaFoundation_SessionMessage,
                                                     Test_I_Source_MediaFoundation_SessionData,
                                                     Test_I_Source_MediaFoundation_SessionData_t,
                                                     IMFMediaType*> Test_I_Source_MediaFoundation_Display;
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                       Test_I_ControlMessage_t,
                                       Test_I_Source_V4L_Stream_Message,
                                       Test_I_Source_V4L_SessionMessage,
                                       Test_I_Source_V4L_SessionData_t,
                                       struct Stream_MediaFramework_V4L_MediaType> Test_I_Source_V4L_Display;
#endif // GTK_USE
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Source_DirectShow_Stream_Message,
                                            Test_I_Source_DirectShow_SessionMessage,
                                            Test_I_Source_DirectShow_SessionData,
                                            Test_I_Source_DirectShow_SessionData_t,
                                            struct Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_EventHandler;
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Source_MediaFoundation_Stream_Message,
                                            Test_I_Source_MediaFoundation_SessionMessage,
                                            Test_I_Source_MediaFoundation_SessionData,
                                            Test_I_Source_MediaFoundation_SessionData_t,
                                            struct Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_EventHandler;
#else
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Source_V4L_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Source_V4L_Stream_Message,
                                            Test_I_Source_V4L_SessionMessage,
                                            Test_I_Source_V4L_SessionData,
                                            Test_I_Source_V4L_SessionData_t,
                                            struct Test_I_Source_V4L_UserData> Test_I_Source_V4L_Module_EventHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

// target
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Target_DirectShow_Stream_Message,
                                                      Test_I_Target_DirectShow_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      Test_I_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Target_DirectShow_SessionData,
                                                      Test_I_Target_DirectShow_SessionData_t> Test_I_Target_DirectShow_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Target_DirectShow_Stream_Message,
                                                      Test_I_Target_DirectShow_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      Test_I_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Target_DirectShow_SessionData,
                                                      Test_I_Target_DirectShow_SessionData_t> Test_I_Target_DirectShow_Module_Statistic_WriterTask_t;
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Target_MediaFoundation_Stream_Message,
                                                      Test_I_Target_MediaFoundation_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      Test_I_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Target_MediaFoundation_SessionData,
                                                      Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Target_MediaFoundation_Stream_Message,
                                                      Test_I_Target_MediaFoundation_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      Test_I_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Target_MediaFoundation_SessionData,
                                                      Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_Statistic_WriterTask_t;
#else
typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Target_Stream_Message,
                                                      Test_I_Target_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      Test_I_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Target_SessionData,
                                                      Test_I_Target_SessionData_t> Test_I_Target_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_ModuleHandlerConfiguration,
                                                      Test_I_ControlMessage_t,
                                                      Test_I_Target_Stream_Message,
                                                      Test_I_Target_SessionMessage,
                                                      Test_I_CommandType_t,
                                                      Test_I_Statistic_t,
                                                      Common_Timer_Manager_t,
                                                      Test_I_Target_SessionData,
                                                      Test_I_Target_SessionData_t> Test_I_Target_Statistic_WriterTask_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                     Test_I_ControlMessage_t,
                                     Test_I_Target_DirectShow_Stream_Message,
                                     Test_I_Target_DirectShow_SessionMessage,
                                     Test_I_Target_DirectShow_SessionData,
                                     Test_I_Target_DirectShow_SessionData_t,
                                     struct _AMMediaType> Test_I_Target_Display;

typedef Stream_MediaFramework_DirectShow_Source_Filter_T<Common_TimePolicy_t,
                                                         Test_I_Target_DirectShow_SessionMessage,
                                                         Test_I_Target_DirectShow_Stream_Message,
                                                         struct Test_I_Target_DirectShow_FilterConfiguration,
                                                         struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                         struct _AMMediaType> Test_I_Target_DirectShowFilter_t;
typedef Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
                                                                Test_I_Target_DirectShow_SessionMessage,
                                                                Test_I_Target_DirectShow_Stream_Message,
                                                                struct Test_I_Target_DirectShow_FilterConfiguration,
                                                                struct Stream_MediaFramework_DirectShow_FilterPinConfiguration,
                                                                struct _AMMediaType> Test_I_Target_AsynchDirectShowFilter_t;
typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                       Test_I_ControlMessage_t,
                                       Test_I_Target_DirectShow_Stream_Message,
                                       Test_I_Target_DirectShow_SessionMessage,
                                       Test_I_Target_DirectShow_SessionData_t,
                                       Test_I_Target_DirectShow_SessionData,
                                       struct Test_I_Target_DirectShow_FilterConfiguration,
                                       struct Test_I_Target_DirectShow_PinConfiguration,
                                       Test_I_Target_DirectShowFilter_t> Test_I_Target_DirectShow_Display;
typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Target_MediaFoundation_Stream_Message,
                                            Test_I_Target_MediaFoundation_SessionMessage,
                                            Test_I_Target_MediaFoundation_SessionData,
                                            Test_I_Target_MediaFoundation_SessionData_t,
                                            struct Test_I_Target_UserData> Test_I_Target_MediaFoundation_Display;
//typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            Test_I_ControlMessage_t,
//                                            Test_I_Target_MediaFoundation_Stream_Message,
//                                            Test_I_Target_MediaFoundation_SessionMessage,
//                                            Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
//                                            Test_I_Target_MediaFoundation_SessionData,
//                                            Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_Stream_DisplayNull;
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_Target_ModuleHandlerConfiguration,
                                       Test_I_ControlMessage_t,
                                       Test_I_Target_Stream_Message,
                                       Test_I_Target_SessionMessage,
                                       Test_I_Target_SessionData_t,
                                       struct Stream_MediaFramework_FFMPEG_MediaType> Test_I_Target_Display;
#endif // GTK_USE
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Target_DirectShow_Stream_Message,
                                            Test_I_Target_DirectShow_SessionMessage,
                                            Test_I_Target_DirectShow_SessionData,
                                            Test_I_Target_DirectShow_SessionData_t,
                                            struct Test_I_Target_UserData> Test_I_Target_DirectShow_EventHandler;
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Target_MediaFoundation_Stream_Message,
                                            Test_I_Target_MediaFoundation_SessionMessage,
                                            Test_I_Target_MediaFoundation_SessionData,
                                            Test_I_Target_MediaFoundation_SessionData_t,
                                            struct Test_I_Target_UserData> Test_I_Target_MediaFoundation_EventHandler;
#else
typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Target_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Target_Stream_Message,
                                            Test_I_Target_SessionMessage,
                                            Test_I_Target_SessionData,
                                            Test_I_Target_SessionData_t,
                                            struct Test_I_Target_UserData> Test_I_Target_Module_EventHandler;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_directshow_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Stream_DirectShow_CamSource);                        // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Stream_MediaFoundation_CamSource);                        // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_V4L_SessionData,                // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_I_Source_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_dev_cam_source_v4l_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_I_Source_V4L_CamSource);                        // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_splitter_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Target_DirectShow_Splitter);                         // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_splitter_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Target_MediaFoundation_Splitter);                         // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_splitter_module_name_string,
                              Stream_INotify_t,                                // stream notification interface type
                              Test_I_Target_Splitter);                         // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_I_Source_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                             // session event type
                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                           // stream notification interface type
                          Test_I_Source_DirectShow_Module_Net_Reader_t,               // reader type
                          Test_I_Source_DirectShow_Module_Net_Writer_t,               // writer type
                          Test_I_Source_DirectShow_Module_Net_IO);                    // name
DATASTREAM_MODULE_DUPLEX (Test_I_Source_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                                  // session event type
                          struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                                // stream notification interface type
                          Test_I_Source_MediaFoundation_Module_Net_Reader_t,               // reader type
                          Test_I_Source_MediaFoundation_Module_Net_Writer_t,               // writer type
                          Test_I_Source_MediaFoundation_Module_Net_IO);                    // name
#else
DATASTREAM_MODULE_DUPLEX (Test_I_Source_V4L_SessionData,                // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Test_I_Source_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                                     // stream notification interface type
                          Test_I_Source_V4L_Net_Reader_t,                      // reader type
                          Test_I_Source_V4L_Net_Writer_t,                      // writer type
                          Test_I_Source_V4L_Net_IO);                           // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_I_Source_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                             // session event type
                          struct Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                           // stream notification interface type
                          Test_I_Source_DirectShow_Module_Statistic_ReaderTask_t,     // reader type
                          Test_I_Source_DirectShow_Module_Statistic_WriterTask_t,     // writer type
                          Test_I_Source_DirectShow_StatisticReport);                  // name
DATASTREAM_MODULE_DUPLEX (Test_I_Source_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                                  // session event type
                          struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                                // stream notification interface type
                          Test_I_Source_MediaFoundation_Module_Statistic_ReaderTask_t,     // reader type
                          Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t,     // writer type
                          Test_I_Source_MediaFoundation_StatisticReport);                  // name
#else
DATASTREAM_MODULE_DUPLEX (Test_I_Source_V4L_SessionData,                // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Test_I_Source_V4L_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                     // stream notification interface type
                          Test_I_Source_Statistic_ReaderTask_t,                 // reader type
                          Test_I_Source_Statistic_WriterTask_t,                 // writer type
                          Test_I_Source_V4L_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Source_DirectShow_Display);                          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Source_MediaFoundation_Display);                          // writer type
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_V4L_SessionData,                  // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_Source_V4L_ModuleHandlerConfiguration,   // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Source_V4L_Display);                            // writer type
#endif // GTK_USE
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Source_DirectShow_EventHandler);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Source_MediaFoundation_EventHandler);                     // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_V4L_SessionData,                // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Test_I_Source_V4L_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                     // stream notification interface type
                              Test_I_Source_V4L_Module_EventHandler);              // writer type
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_DUPLEX (Test_I_Target_DirectShow_SessionData,                // session data type
                          enum Stream_SessionMessageType,                             // session event type
                          struct Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                           // stream notification interface type
                          Test_I_Target_DirectShow_Module_Statistic_ReaderTask_t,     // reader type
                          Test_I_Target_DirectShow_Module_Statistic_WriterTask_t,     // writer type
                          Test_I_Target_DirectShow_StatisticReport);                  // name
DATASTREAM_MODULE_DUPLEX (Test_I_Target_MediaFoundation_SessionData,                // session data type
                          enum Stream_SessionMessageType,                                  // session event type
                          struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                                // stream notification interface type
                          Test_I_Target_MediaFoundation_Module_Statistic_ReaderTask_t,     // reader type
                          Test_I_Target_MediaFoundation_Module_Statistic_WriterTask_t,     // writer type
                          Test_I_Target_MediaFoundation_StatisticReport);                  // name
#else
DATASTREAM_MODULE_DUPLEX (Test_I_Target_SessionData,                // session data type
                          enum Stream_SessionMessageType,                  // session event type
                          struct Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                // stream notification interface type
                          Test_I_Target_Statistic_ReaderTask_t,            // reader type
                          Test_I_Target_Statistic_WriterTask_t,            // writer type
                          Test_I_Target_StatisticReport);                  // name
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_direct3d_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Target_Display);                                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_directshow_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Target_DirectShow_Display);                          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_vis_mediafoundation_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Target_MediaFoundation_Display);                          // writer type
#else
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                         // session event type
                              struct Test_I_Target_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_vis_gtk_pixbuf_module_name_string,
                              Stream_INotify_t,                                       // stream notification interface type
                              Test_I_Target_Display);                                 // writer type
#endif // GTK_USE
#endif // GUI_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              enum Stream_SessionMessageType,                             // session event type
                              struct Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                           // stream notification interface type
                              Test_I_Target_DirectShow_EventHandler);                     // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
                              enum Stream_SessionMessageType,                                  // session event type
                              struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                                // stream notification interface type
                              Test_I_Target_MediaFoundation_EventHandler);                     // writer type
#else
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                // stream notification interface type
                              Test_I_Target_Module_EventHandler);              // writer type
#endif // ACE_WIN32 || ACE_WIN64

#endif
