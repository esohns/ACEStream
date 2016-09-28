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

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "strmif.h"
#endif

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_cam_source_directshow.h"
#include "stream_dev_cam_source_mediafoundation.h"

//#include "stream_misc_common.h"
#include "stream_misc_directshow_source.h"
#include "stream_misc_mediafoundation_source.h"

//#include "stream_vis_gtk_cairo.h"
#include "stream_vis_target_direct3d.h"
//#include "stream_vis_target_directshow.h"
//#include "stream_vis_target_mediafoundation.h"
#else
#include "stream_dev_cam_source_v4l.h"

#include "stream_vis_gtk_pixbuf.h"
#endif

//#include "stream_dec_avi_decoder.h"

#include "stream_misc_runtimestatistic.h"

#include "stream_module_io.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"

//#include "test_i_module_direct3d.h"
#include "test_i_module_eventhandler.h"
#include "test_i_module_splitter.h"

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

typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_MT_SYNCH,
                                           Test_I_DirectShow_ControlMessage_t,
                                           Test_I_Source_DirectShow_Stream_Message,
                                           Test_I_Source_DirectShow_Stream_SessionMessage,
                                           Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                           Stream_ControlType,
                                           Stream_SessionMessageType,
                                           Test_I_Source_DirectShow_StreamState,
                                           Test_I_Source_DirectShow_SessionData,
                                           Test_I_Source_DirectShow_SessionData_t,
                                           Test_I_Source_Stream_StatisticData> Test_I_Stream_DirectShow_Module_CamSource;
typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_MT_SYNCH,
                                                Test_I_DirectShow_ControlMessage_t,
                                                Test_I_Source_MediaFoundation_Stream_Message,
                                                Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                                Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                                Stream_ControlType,
                                                Stream_SessionMessageType,
                                                Test_I_Source_MediaFoundation_StreamState,
                                                Test_I_Source_MediaFoundation_SessionData,
                                                Test_I_Source_MediaFoundation_SessionData_t,
                                                Test_I_Source_Stream_StatisticData> Test_I_Stream_MediaFoundation_Module_CamSource;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                           // session event type
                              Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                              // stream notification interface type
                              Test_I_Stream_DirectShow_Module_CamSource);          // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                                   // stream notification interface type
                              Test_I_Stream_MediaFoundation_Module_CamSource);          // writer type

typedef Stream_Misc_DirectShow_Source_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_Target_DirectShow_ControlMessage_t,
                                        Test_I_Target_DirectShow_Stream_Message,
                                        Test_I_Target_DirectShow_Stream_SessionMessage,
                                        Test_I_Target_DirectShow_SessionData_t,
                                        Test_I_Target_DirectShow_FilterConfiguration,
                                        Test_I_Target_DirectShow_PinConfiguration,
                                        struct _AMMediaType> Test_I_Target_Module_DirectShowSource;
typedef Stream_Misc_MediaFoundation_Source_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                             Test_I_Target_DirectShow_ControlMessage_t,
                                             Test_I_Target_MediaFoundation_Stream_Message,
                                             Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                             Test_I_Target_MediaFoundation_SessionData_t,
                                             Test_I_Target_MediaFoundation_SessionData,
                                             IMFMediaType*> Test_I_Target_Module_MediaFoundationSource;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                           // session event type
                              Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                              // stream notification interface type
                              Test_I_Target_Module_DirectShowSource);              // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                                   // stream notification interface type
                              Test_I_Target_Module_MediaFoundationSource);              // writer type

#else
typedef Stream_Module_CamSource_V4L_T<ACE_MT_SYNCH,
                                      Test_I_V4L2_ControlMessage_t,
                                      Test_I_Source_V4L2_Stream_Message,
                                      Test_I_Source_V4L2_Stream_SessionMessage,
                                      Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                      Stream_ControlType,
                                      Stream_SessionMessageType,
                                      Test_I_Source_V4L2_StreamState,
                                      Test_I_Source_V4L2_SessionData,
                                      Test_I_Source_V4L2_SessionData_t,
                                      Test_I_Source_Stream_StatisticData> Test_I_Source_V4L2_Module_CamSource;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_V4L2_SessionData,                // session data type
                              Stream_SessionMessageType,                     // session event type
                              Test_I_Source_V4L2_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                        // stream notification interface type
                              Test_I_Source_V4L2_Module_CamSource);          // writer type
#endif

//typedef Stream_Decoder_AVIDecoder_T<Test_I_Target_Stream_SessionMessage,
//                                    Test_I_Target_Stream_Message,
//                                    Test_I_Target_ModuleHandlerConfiguration,
//                                    Test_I_Target_SessionData> Test_I_Target_Stream_Module_AVIDecoder;
//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
//                              Common_TimePolicy_t,                             // time policy
//                              Stream_ModuleConfiguration,                      // module configuration type
//                              Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_Target_Stream_Module_AVIDecoder);         // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
//                                 Common_TimePolicy_t,
//                                 Test_I_Target_DirectShow_ModuleHandlerConfiguration,
//                                 Test_I_Target_MediaFoundation_ControlMessage_t,
//                                 Test_I_Target_DirectShow_Stream_Message,
//                                 Test_I_Target_DirectShow_Stream_SessionMessage,
//                                 Test_I_Target_DirectShow_SessionData> Test_I_Target_DirectShow_Module_Splitter;
//typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
//                                 Common_TimePolicy_t,
//                                 Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
//                                 Test_I_Target_MediaFoundation_ControlMessage_t,
//                                 Test_I_Target_MediaFoundation_Stream_Message,
//                                 Test_I_Target_MediaFoundation_Stream_SessionMessage,
//                                 Test_I_Target_MediaFoundation_SessionData> Test_I_Target_MediaFoundation_Module_Splitter;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                           // session event type
                              Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                              // stream notification interface type
                              Test_I_Target_DirectShow_Module_Splitter);           // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                                   // stream notification interface type
                              Test_I_Target_MediaFoundation_Module_Splitter);           // writer type
#endif
typedef Stream_Module_Splitter_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 Test_I_Target_ModuleHandlerConfiguration,
                                 Test_I_Target_ControlMessage_t,
                                 Test_I_Target_Stream_Message,
                                 Test_I_Target_Stream_SessionMessage,
                                 Test_I_Target_SessionData> Test_I_Target_Module_Splitter;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
                              Stream_SessionMessageType,                // session event type
                              Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                   // stream notification interface type
                              Test_I_Target_Module_Splitter);           // writer type

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_I_DirectShow_ControlMessage_t,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_Stream_SessionMessage,
                                     Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                     Stream_ControlType,
                                     Stream_SessionMessageType,
                                     Test_I_Source_DirectShow_StreamState,
                                     Test_I_Source_DirectShow_SessionData,
                                     Test_I_Source_DirectShow_SessionData_t,
                                     Test_I_Source_Stream_StatisticData,
                                     ACE_INET_Addr,
                                     Test_I_Source_DirectShow_InetConnectionManager_t> Test_I_Source_DirectShow_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                     Test_I_DirectShow_ControlMessage_t,
                                     Test_I_Source_DirectShow_Stream_Message,
                                     Test_I_Source_DirectShow_Stream_SessionMessage,
                                     Test_I_Source_DirectShow_SessionData,
                                     Test_I_Source_DirectShow_SessionData_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_DirectShow_InetConnectionManager_t> Test_I_Source_DirectShow_Module_Net_Reader_t;
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_I_MediaFoundation_ControlMessage_t,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                     Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                     Stream_ControlType,
                                     Stream_SessionMessageType,
                                     Test_I_Source_MediaFoundation_StreamState,
                                     Test_I_Source_MediaFoundation_SessionData,
                                     Test_I_Source_MediaFoundation_SessionData_t,
                                     Test_I_Source_Stream_StatisticData,
                                     ACE_INET_Addr,
                                     Test_I_Source_MediaFoundation_InetConnectionManager_t> Test_I_Source_MediaFoundation_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                     Test_I_MediaFoundation_ControlMessage_t,
                                     Test_I_Source_MediaFoundation_Stream_Message,
                                     Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                     Test_I_Source_MediaFoundation_SessionData,
                                     Test_I_Source_MediaFoundation_SessionData_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_MediaFoundation_InetConnectionManager_t> Test_I_Source_MediaFoundation_Module_Net_Reader_t;

DATASTREAM_MODULE_DUPLEX (Test_I_Source_DirectShow_SessionData,                // session data type
                          Stream_SessionMessageType,                           // session event type
                          Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_IStreamNotify_t,                              // stream notification interface type
                          Test_I_Source_DirectShow_Module_Net_Reader_t,        // reader type
                          Test_I_Source_DirectShow_Module_Net_Writer_t,        // writer type
                          Test_I_Source_DirectShow_Module_Net_IO);             // name
DATASTREAM_MODULE_DUPLEX (Test_I_Source_MediaFoundation_SessionData,                // session data type
                          Stream_SessionMessageType,                                // session event type
                          Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_IStreamNotify_t,                                   // stream notification interface type
                          Test_I_Source_MediaFoundation_Module_Net_Reader_t,        // reader type
                          Test_I_Source_MediaFoundation_Module_Net_Writer_t,        // writer type
                          Test_I_Source_MediaFoundation_Module_Net_IO);             // name

#else
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Test_I_V4L2_ControlMessage_t,
                                     Test_I_Source_V4L2_Stream_Message,
                                     Test_I_Source_V4L2_Stream_SessionMessage,
                                     Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                     Stream_ControlType,
                                     Stream_SessionMessageType,
                                     Test_I_Source_V4L2_StreamState,
                                     Test_I_Source_V4L2_SessionData,
                                     Test_I_Source_V4L2_SessionData_t,
                                     Test_I_Source_Stream_StatisticData,
                                     ACE_INET_Addr,
                                     Test_I_Source_V4L2_InetConnectionManager_t> Test_I_Source_V4L2_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                     Test_I_V4L2_ControlMessage_t,
                                     Test_I_Source_V4L2_Stream_Message,
                                     Test_I_Source_V4L2_Stream_SessionMessage,
                                     Test_I_Source_V4L2_SessionData,
                                     Test_I_Source_V4L2_SessionData_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_V4L2_InetConnectionManager_t> Test_I_Source_V4L2_Module_Net_Reader_t;

DATASTREAM_MODULE_DUPLEX (Test_I_Source_V4L2_SessionData,                // session data type
                          Stream_SessionMessageType,                     // session event type
                          Test_I_Source_V4L2_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_IStreamNotify_t,                        // stream notification interface type
                          Test_I_Source_V4L2_Module_Net_Reader_t,        // reader type
                          Test_I_Source_V4L2_Module_Net_Writer_t,        // writer type
                          Test_I_Source_V4L2_Module_Net_IO);             // name
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                             Test_I_DirectShow_ControlMessage_t,
                                             Test_I_Source_DirectShow_Stream_Message,
                                             Test_I_Source_DirectShow_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_DirectShow_SessionData,
                                             Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                             Test_I_DirectShow_ControlMessage_t,
                                             Test_I_Source_DirectShow_Stream_Message,
                                             Test_I_Source_DirectShow_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_DirectShow_SessionData,
                                             Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Module_Statistic_WriterTask_t;
typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                             Test_I_MediaFoundation_ControlMessage_t,
                                             Test_I_Source_MediaFoundation_Stream_Message,
                                             Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_MediaFoundation_SessionData,
                                             Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                             Test_I_MediaFoundation_ControlMessage_t,
                                             Test_I_Source_MediaFoundation_Stream_Message,
                                             Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_MediaFoundation_SessionData,
                                             Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t;

typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                             Test_I_Target_DirectShow_ControlMessage_t,
                                             Test_I_Target_DirectShow_Stream_Message,
                                             Test_I_Target_DirectShow_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_DirectShow_SessionData,
                                             Test_I_Target_DirectShow_SessionData_t> Test_I_Target_DirectShow_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                             Test_I_Target_DirectShow_ControlMessage_t,
                                             Test_I_Target_DirectShow_Stream_Message,
                                             Test_I_Target_DirectShow_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_DirectShow_SessionData,
                                             Test_I_Target_DirectShow_SessionData_t> Test_I_Target_DirectShow_Module_Statistic_WriterTask_t;
typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                             Test_I_Target_MediaFoundation_ControlMessage_t,
                                             Test_I_Target_MediaFoundation_Stream_Message,
                                             Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_MediaFoundation_SessionData,
                                             Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                             Test_I_Target_MediaFoundation_ControlMessage_t,
                                             Test_I_Target_MediaFoundation_Stream_Message,
                                             Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_MediaFoundation_SessionData,
                                             Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_Statistic_WriterTask_t;

DATASTREAM_MODULE_DUPLEX (Test_I_Source_DirectShow_SessionData,                   // session data type
                          Stream_SessionMessageType,                              // session event type
                          Test_I_Source_DirectShow_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_IStreamNotify_t,                                 // stream notification interface type
                          Test_I_Source_DirectShow_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Source_DirectShow_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Source_DirectShow_Module_RuntimeStatistic);      // name
DATASTREAM_MODULE_DUPLEX (Test_I_Source_MediaFoundation_SessionData,                   // session data type
                          Stream_SessionMessageType,                                   // session event type
                          Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_IStreamNotify_t,                                      // stream notification interface type
                          Test_I_Source_MediaFoundation_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Source_MediaFoundation_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Source_MediaFoundation_Module_RuntimeStatistic);      // name

DATASTREAM_MODULE_DUPLEX (Test_I_Target_DirectShow_SessionData,                   // session data type
                          Stream_SessionMessageType,                              // session event type
                          Test_I_Target_DirectShow_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_IStreamNotify_t,                                 // stream notification interface type
                          Test_I_Target_DirectShow_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Target_DirectShow_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Target_DirectShow_Module_RuntimeStatistic);      // name
DATASTREAM_MODULE_DUPLEX (Test_I_Target_MediaFoundation_SessionData,                   // session data type
                          Stream_SessionMessageType,                                   // session event type
                          Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_IStreamNotify_t,                                      // stream notification interface type
                          Test_I_Target_MediaFoundation_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Target_MediaFoundation_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Target_MediaFoundation_Module_RuntimeStatistic);      // name
#else
typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                             Test_I_V4L2_ControlMessage_t,
                                             Test_I_Source_V4L2_Stream_Message,
                                             Test_I_Source_V4L2_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_V4L2_SessionData,
                                             Test_I_Source_V4L2_SessionData_t> Test_I_Source_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                             Test_I_V4L2_ControlMessage_t,
                                             Test_I_Source_V4L2_Stream_Message,
                                             Test_I_Source_V4L2_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_V4L2_SessionData,
                                             Test_I_Source_V4L2_SessionData_t> Test_I_Source_Module_Statistic_WriterTask_t;

DATASTREAM_MODULE_DUPLEX (Test_I_Source_V4L2_SessionData,                // session data type
                          Stream_SessionMessageType,                     // session event type
                          Test_I_Source_V4L2_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_IStreamNotify_t,                        // stream notification interface type
                          Test_I_Source_Module_Statistic_ReaderTask_t,   // reader type
                          Test_I_Source_Module_Statistic_WriterTask_t,   // writer type
                          Test_I_Source_V4L2_Module_RuntimeStatistic);   // name

#endif
typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_ModuleHandlerConfiguration,
                                             Test_I_Target_ControlMessage_t,
                                             Test_I_Target_Stream_Message,
                                             Test_I_Target_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_SessionData,
                                             Test_I_Target_SessionData_t> Test_I_Target_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_ModuleHandlerConfiguration,
                                             Test_I_Target_ControlMessage_t,
                                             Test_I_Target_Stream_Message,
                                             Test_I_Target_Stream_SessionMessage,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_SessionData,
                                             Test_I_Target_SessionData_t> Test_I_Target_Module_Statistic_WriterTask_t;

DATASTREAM_MODULE_DUPLEX (Test_I_Target_SessionData,                   // session data type
                          Stream_SessionMessageType,                   // session event type
                          Test_I_Target_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_IStreamNotify_t,                      // stream notification interface type
                          Test_I_Target_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Target_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Target_Module_RuntimeStatistic);      // name

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_DirectShow_Target_Direct3D_T<ACE_MT_SYNCH,
                                                Common_TimePolicy_t,
                                                Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                                Test_I_DirectShow_ControlMessage_t,
                                                Test_I_Source_DirectShow_Stream_Message,
                                                Test_I_Source_DirectShow_Stream_SessionMessage,
                                                Test_I_Source_DirectShow_SessionData,
                                                Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Module_Display;
typedef Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_MT_SYNCH,
                                                     Common_TimePolicy_t,
                                                     Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                                     Test_I_MediaFoundation_ControlMessage_t,
                                                     Test_I_Source_MediaFoundation_Stream_Message,
                                                     Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                                     Test_I_Source_MediaFoundation_SessionData,
                                                     Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_Display;
//typedef Stream_Vis_Target_DirectShow_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       Test_I_Source_DirectShow_ModuleHandlerConfiguration,
//                                       Test_I_DirectShow_ControlMessage_t,
//                                       Test_I_Source_DirectShow_Stream_Message,
//                                       Test_I_Source_DirectShow_Stream_SessionMessage,
//                                       Test_I_Source_DirectShow_SessionData_t,
//                                       Test_I_Source_DirectShow_SessionData> Test_I_Source_DirectShow_Module_Display;
//typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
//                                            Test_I_MediaFoundation_ControlMessage_t,
//                                            Test_I_Source_MediaFoundation_Stream_Message,
//                                            Test_I_Source_MediaFoundation_Stream_SessionMessage,
//                                            Test_I_Source_MediaFoundation_SessionData,
//                                            Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_Display;
//typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            Test_I_MediaFoundation_ControlMessage_t,
//                                            Test_I_Source_MediaFoundation_Stream_Message,
//                                            Test_I_Source_MediaFoundation_Stream_SessionMessage,
//                                            Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
//                                            Test_I_Source_MediaFoundation_SessionData,
//                                            Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_DisplayNull;

//typedef Stream_Vis_DirectShow_Target_Direct3D_T<ACE_MT_SYNCH,
//                                                Common_TimePolicy_t,
//                                                Test_I_Target_DirectShow_ModuleHandlerConfiguration,
//                                                Test_I_DirectShow_ControlMessage_t,
//                                                Test_I_Target_DirectShow_Stream_Message,
//                                                Test_I_Target_DirectShow_Stream_SessionMessage,
//                                                Test_I_Target_DirectShow_SessionData,
//                                                Test_I_Target_DirectShow_SessionData_t> Test_I_Target_DirectShow_Module_Display;
//typedef Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_MT_SYNCH,
//                                                     Common_TimePolicy_t,
//                                                     Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
//                                                     Test_I_MediaFoundation_ControlMessage_t,
//                                                     Test_I_Target_MediaFoundation_Stream_Message,
//                                                     Test_I_Target_MediaFoundation_Stream_SessionMessage,
//                                                     Test_I_Target_MediaFoundation_SessionData,
//                                                     Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_Display;
typedef Stream_Vis_Target_Direct3D_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     Test_I_Target_ModuleHandlerConfiguration,
                                     Test_I_Target_ControlMessage_t,
                                     Test_I_Target_Stream_Message,
                                     Test_I_Target_Stream_SessionMessage,
                                     Test_I_Target_SessionData,
                                     Test_I_Target_SessionData_t> Test_I_Target_Module_Display;
//typedef Stream_Vis_Target_MediaFoundation_T<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
//                                            Test_I_Target_MediaFoundation_ControlMessage_t,
//                                            Test_I_Target_MediaFoundation_Stream_Message,
//                                            Test_I_Target_MediaFoundation_Stream_SessionMessage,
//                                            Test_I_Target_MediaFoundation_SessionData,
//                                            Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_Display;
//typedef Stream_Vis_Target_MediaFoundation_2<ACE_MT_SYNCH,
//                                            Common_TimePolicy_t,
//                                            Test_I_Target_MediaFoundation_ControlMessage_t,
//                                            Test_I_Target_MediaFoundation_Stream_Message,
//                                            Test_I_Target_MediaFoundation_Stream_SessionMessage,
//                                            Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
//                                            Test_I_Target_MediaFoundation_SessionData,
//                                            Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_Stream_Module_DisplayNull;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                           // session event type
                              Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                              // stream notification interface type
                              Test_I_Source_DirectShow_Module_Display);            // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                                   // stream notification interface type
                              Test_I_Source_MediaFoundation_Module_Display);            // writer type

//DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
//                              Stream_SessionMessageType,                           // session event type
//                              Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_IStreamNotify_t,                              // stream notification interface type
//                              Test_I_Target_DirectShow_Module_Display);            // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
//                              Stream_SessionMessageType,                                // session event type
//                              Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_IStreamNotify_t,                                   // stream notification interface type
//                              Test_I_Target_MediaFoundation_Module_Display);            // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
                              Stream_SessionMessageType,                // session event type
                              Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                   // stream notification interface type
                              Test_I_Target_Module_Display);            // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_SessionData,                // session data type
//                              Stream_SessionMessageType,                // session event type
//                              Test_I_Source_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_IStreamNotify_t,                   // stream notification interface type
//                              Test_I_Source_Stream_Module_DisplayNull); // writer type
//DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
//                              Stream_SessionMessageType,                // session event type
//                              Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_IStreamNotify_t,                   // stream notification interface type
//                              Test_I_Target_Stream_Module_DisplayNull); // writer type

typedef Test_I_Stream_Module_EventHandler_T<Stream_ModuleConfiguration,
                                            Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                            Test_I_DirectShow_ControlMessage_t,
                                            Test_I_Source_DirectShow_Stream_Message,
                                            Test_I_Source_DirectShow_Stream_SessionMessage,
                                            Test_I_Source_DirectShow_SessionData,
                                            Test_I_Source_DirectShow_SessionData_t> Test_I_Source_DirectShow_Module_EventHandler;
typedef Test_I_Stream_Module_EventHandler_T<Stream_ModuleConfiguration,
                                            Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_I_MediaFoundation_ControlMessage_t,
                                            Test_I_Source_MediaFoundation_Stream_Message,
                                            Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                            Test_I_Source_MediaFoundation_SessionData,
                                            Test_I_Source_MediaFoundation_SessionData_t> Test_I_Source_MediaFoundation_Module_EventHandler;

typedef Test_I_Stream_Module_EventHandler_T<Stream_ModuleConfiguration,
                                            Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                            Test_I_Target_DirectShow_ControlMessage_t,
                                            Test_I_Target_DirectShow_Stream_Message,
                                            Test_I_Target_DirectShow_Stream_SessionMessage,
                                            Test_I_Target_DirectShow_SessionData,
                                            Test_I_Target_DirectShow_SessionData_t> Test_I_Target_DirectShow_Module_EventHandler;
typedef Test_I_Stream_Module_EventHandler_T<Stream_ModuleConfiguration,
                                            Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                            Test_I_Target_MediaFoundation_ControlMessage_t,
                                            Test_I_Target_MediaFoundation_Stream_Message,
                                            Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                            Test_I_Target_MediaFoundation_SessionData,
                                            Test_I_Target_MediaFoundation_SessionData_t> Test_I_Target_MediaFoundation_Module_EventHandler;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                           // session event type
                              Test_I_Source_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                              // stream notification interface type
                              Test_I_Source_DirectShow_Module_EventHandler);       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_I_Source_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                                   // stream notification interface type
                              Test_I_Source_MediaFoundation_Module_EventHandler);       // writer type

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_DirectShow_SessionData,                // session data type
                              Stream_SessionMessageType,                           // session event type
                              Test_I_Target_DirectShow_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                              // stream notification interface type
                              Test_I_Target_DirectShow_Module_EventHandler);       // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_MediaFoundation_SessionData,                // session data type
                              Stream_SessionMessageType,                                // session event type
                              Test_I_Target_MediaFoundation_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                                   // stream notification interface type
                              Test_I_Target_MediaFoundation_Module_EventHandler);       // writer type
#else
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                       Test_I_V4L2_ControlMessage_t,
                                       Test_I_Source_V4L2_Stream_Message,
                                       Test_I_Source_V4L2_Stream_SessionMessage,
                                       Test_I_Source_V4L2_SessionData_t> Test_I_Source_V4L2_Module_Display;
typedef Stream_Module_Vis_GTK_Pixbuf_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       Test_I_Target_ModuleHandlerConfiguration,
                                       Test_I_Target_ControlMessage_t,
                                       Test_I_Target_Stream_Message,
                                       Test_I_Target_Stream_SessionMessage,
                                       Test_I_Target_SessionData_t> Test_I_Target_Module_Display;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_V4L2_SessionData,                // session data type
                              Stream_SessionMessageType,                     // session event type
                              Test_I_Source_V4L2_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                        // stream notification interface type
                              Test_I_Source_V4L2_Module_Display);            // writer type
DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
                              Stream_SessionMessageType,                // session event type
                              Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                   // stream notification interface type
                              Test_I_Target_Module_Display);            // writer type

typedef Test_I_Stream_Module_EventHandler_T<Stream_ModuleConfiguration,
                                            Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                            Test_I_V4L2_ControlMessage_t,
                                            Test_I_Source_V4L2_Stream_Message,
                                            Test_I_Source_V4L2_Stream_SessionMessage,
                                            Test_I_Source_V4L2_SessionData,
                                            Test_I_Source_V4L2_SessionData_t> Test_I_Source_V4L2_Module_EventHandler;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Source_V4L2_SessionData,                // session data type
                              Stream_SessionMessageType,                     // session event type
                              Test_I_Source_V4L2_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                        // stream notification interface type
                              Test_I_Source_V4L2_Module_EventHandler);       // writer type
#endif
typedef Test_I_Stream_Module_EventHandler_T<Stream_ModuleConfiguration,
                                            Test_I_Target_ModuleHandlerConfiguration,
                                            Test_I_Target_ControlMessage_t,
                                            Test_I_Target_Stream_Message,
                                            Test_I_Target_Stream_SessionMessage,
                                            Test_I_Target_SessionData,
                                            Test_I_Target_SessionData_t> Test_I_Target_Module_EventHandler;

DATASTREAM_MODULE_INPUT_ONLY (Test_I_Target_SessionData,                // session data type
                              Stream_SessionMessageType,                // session event type
                              Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_IStreamNotify_t,                   // stream notification interface type
                              Test_I_Target_Module_EventHandler);       // writer type

#endif
