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

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"
//#include "stream_vis_target_directshow.h"
#include "stream_vis_target_mediafoundation.h"
#include "stream_vis_target_direct3d.h"
#else
#include "stream_dev_cam_source_v4l.h"
#endif
#include "stream_dec_avi_encoder.h"
#include "stream_file_sink.h"
#include "stream_misc_statistic_report.h"
#include "stream_vis_gtk_cairo.h"

#include "test_u_camsave_common.h"
#include "test_u_camsave_message.h"
#include "test_u_camsave_session_message.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
//                                           Stream_CamSave_SessionMessage,
//                                           Stream_CamSave_Message,
//                                           struct Stream_CamSave_ModuleHandlerConfiguration,
//                                           struct Stream_CamSave_StreamState,
//                                           struct Stream_CamSave_SessionData,
//                                           Stream_CamSave_SessionData_t,
//                                           struct Stream_CamSave_StatisticData> Stream_CamSave_Source;
typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Test_U_ControlMessage_t,
                                                Stream_CamSave_Message,
                                                Stream_CamSave_SessionMessage,
                                                struct Stream_CamSave_ModuleHandlerConfiguration,
                                                enum Stream_ControlType,
                                                enum Stream_SessionMessageType,
                                                struct Stream_CamSave_StreamState,
                                                struct Stream_CamSave_SessionData,
                                                Stream_CamSave_SessionData_t,
                                                struct Stream_CamSave_StatisticData,
                                                struct Stream_CamSave_UserData> Stream_CamSave_Source;
#else
typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_Message,
                                      Stream_CamSave_SessionMessage,
                                      struct Stream_CamSave_ModuleHandlerConfiguration,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      struct Stream_CamSave_StreamState,
                                      struct Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t,
                                      struct Stream_CamSave_StatisticData,
                                      struct Stream_CamSave_UserData> Stream_CamSave_Source;
#endif
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_IStreamNotify_t,                   // stream notification interface type
                              Stream_CamSave_Source);                           // writer type

typedef Stream_Module_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Stream_CamSave_ModuleHandlerConfiguration,
                                                   Test_U_ControlMessage_t,
                                                   Stream_CamSave_Message,
                                                   Stream_CamSave_SessionMessage,
                                                   Stream_CommandType_t,
                                                   struct Stream_CamSave_StatisticData,
                                                   struct Stream_CamSave_SessionData,
                                                   Stream_CamSave_SessionData_t> Stream_CamSave_Statistic_ReaderTask_t;
typedef Stream_Module_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                   Common_TimePolicy_t,
                                                   struct Stream_CamSave_ModuleHandlerConfiguration,
                                                   Test_U_ControlMessage_t,
                                                   Stream_CamSave_Message,
                                                   Stream_CamSave_SessionMessage,
                                                   Stream_CommandType_t,
                                                   struct Stream_CamSave_StatisticData,
                                                   struct Stream_CamSave_SessionData,
                                                   Stream_CamSave_SessionData_t> Stream_CamSave_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Stream_CamSave_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                          Stream_CamSave_IStreamNotify_t,                   // stream notification interface type
                          Stream_CamSave_Statistic_ReaderTask_t,            // reader type
                          Stream_CamSave_Statistic_WriterTask_t,            // writer type
                          Stream_CamSave_StatisticReport);                  // name

typedef Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               Stream_CamSave_SessionData_t,
                                               struct Stream_CamSave_SessionData> Stream_CamSave_AVIEncoder_ReaderTask_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
//                                               Common_TimePolicy_t,
//                                               struct Stream_CamSave_ModuleHandlerConfiguration,
//                                               Test_U_ControlMessage_t,
//                                               Stream_CamSave_Message,
//                                               Stream_CamSave_SessionMessage,
//                                               Stream_CamSave_SessionData_t,
//                                               struct Stream_CamSave_SessionData,
//                                               struct _AMMediaType,
//                                               struct Stream_CamSave_UserData> Stream_CamSave_AVIEncoder_DirectShow_WriterTask_t;
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_Message,
                                               Stream_CamSave_SessionMessage,
                                               Stream_CamSave_SessionData_t,
                                               struct Stream_CamSave_SessionData,
                                               IMFMediaType,
                                               struct Stream_CamSave_UserData> Stream_CamSave_AVIEncoder_MediaFoundation_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Stream_CamSave_SessionData,                      // session data type
                          enum Stream_SessionMessageType,                         // session event type
                          struct Stream_CamSave_ModuleHandlerConfiguration,       // module handler configuration type
                          Stream_CamSave_IStreamNotify_t,                         // stream notification interface type
                          Stream_CamSave_AVIEncoder_ReaderTask_t,                 // reader type
                          Stream_CamSave_AVIEncoder_MediaFoundation_WriterTask_t, // writer type
                          Stream_CamSave_MediaFoundation_AVIEncoder);             // name
#else
typedef Stream_Decoder_AVIEncoder_WriterTask_T<ACE_MT_SYNCH,
                                               Common_TimePolicy_t,
                                               struct Stream_CamSave_ModuleHandlerConfiguration,
                                               Test_U_ControlMessage_t,
                                               Stream_CamSave_Message,
                                               Stream_CamSave_SessionMessage,
                                               Stream_CamSave_SessionData_t,
                                               struct Stream_CamSave_SessionData,
                                               struct v4l2_format,
                                               struct Stream_CamSave_UserData> Stream_CamSave_V4L2_AVIEncoder_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Stream_CamSave_SessionData,                // session data type
                          enum Stream_SessionMessageType,                   // session event type
                          struct Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                          Stream_CamSave_IStreamNotify_t,                   // stream notification interface type
                          Stream_CamSave_AVIEncoder_ReaderTask_t,           // reader type
                          Stream_CamSave_V4L2_AVIEncoder_WriterTask_t,      // writer type
                          Stream_CamSave_V4L2_AVIEncoder);                  // name
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Vis_Target_DirectShow_T<Stream_CamSave_SessionMessage,
//                                       Stream_CamSave_Message,
//                                       struct Stream_CamSave_ModuleHandlerConfiguration,
//                                       struct Stream_CamSave_SessionData,
//                                       Stream_CamSave_SessionData_t> Stream_CamSave_Display;
//typedef Stream_Vis_Target_MediaFoundation_T<Stream_CamSave_SessionMessage,
//                                            Stream_CamSave_Message,
//                                            struct Stream_CamSave_ModuleHandlerConfiguration,
//                                            struct Stream_CamSave_SessionData,
//                                            Stream_CamSave_SessionData_t> Stream_CamSave_Display;
typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Stream_CamSave_ModuleHandlerConfiguration,
                                            Test_U_ControlMessage_t,
                                            Stream_CamSave_Message,
                                            Stream_CamSave_SessionMessage,
                                            struct Stream_CamSave_SessionData,
                                            Stream_CamSave_SessionData_t> Stream_CamSave_DisplayNull;
//typedef Stream_Vis_Target_Direct3D_T<Stream_CamSave_SessionMessage,
//                                     Stream_CamSave_Message,
//                                     struct Stream_CamSave_ModuleHandlerConfiguration,
//                                     struct Stream_CamSave_SessionData,
//                                     Stream_CamSave_SessionData_t> Stream_CamSave_Display;
#endif
typedef Stream_Module_Vis_GTK_Cairo_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      struct Stream_CamSave_ModuleHandlerConfiguration,
                                      Test_U_ControlMessage_t,
                                      Stream_CamSave_Message,
                                      Stream_CamSave_SessionMessage,
                                      struct Stream_CamSave_SessionData,
                                      Stream_CamSave_SessionData_t> Stream_CamSave_Display;
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_IStreamNotify_t,                   // stream notification interface type
                              Stream_CamSave_Display);                          // writer type
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_IStreamNotify_t,                   // stream notification interface type
                              Stream_CamSave_DisplayNull);                      // writer type
#endif

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_CamSave_ModuleHandlerConfiguration,
                                   Test_U_ControlMessage_t,
                                   Stream_CamSave_Message,
                                   Stream_CamSave_SessionMessage,
                                   struct Stream_CamSave_SessionData> Stream_CamSave_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_CamSave_SessionData,                // session data type
                              enum Stream_SessionMessageType,                   // session event type
                              struct Stream_CamSave_ModuleHandlerConfiguration, // module handler configuration type
                              Stream_CamSave_IStreamNotify_t,                   // stream notification interface type
                              Stream_CamSave_FileWriter);                       // writer type

#endif
