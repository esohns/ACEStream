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

//#include "stream_misc_common.h"
#include "stream_misc_directshow_source.h"

#include "stream_vis_target_directshow.h"
#else
#include "stream_dev_cam_source_v4l.h"

#include "stream_vis_gtk_drawingarea.h"
#endif

//#include "stream_dec_avi_decoder.h"

#include "stream_misc_runtimestatistic.h"
#include "stream_misc_splitter.h"

#include "stream_module_io.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"
#include "test_i_message.h"
#include "test_i_session_message.h"

#include "test_i_source_common.h"

#include "test_i_target_common.h"

// declare module(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Dev_Cam_Source_DirectShow_T<ACE_SYNCH_MUTEX,

                                           Test_I_Source_Stream_SessionMessage,
                                           Test_I_Source_Stream_Message,

                                           Test_I_Source_Stream_ModuleHandlerConfiguration,

                                           Test_I_Source_StreamState,

                                           Test_I_Source_Stream_SessionData,
                                           Test_I_Source_Stream_SessionData_t,

                                           Test_I_Source_Stream_StatisticData> Test_I_Stream_Module_CamSource;
typedef Stream_Misc_DirectShow_Source_T<Test_I_Target_Stream_SessionMessage,
                                        Test_I_Target_Stream_Message,

                                        Test_I_Target_Stream_ModuleHandlerConfiguration,

                                        Test_I_Target_Stream_SessionData_t,

                                        Test_I_Target_DirectShow_FilterConfiguration,
                                        Test_I_Target_DirectShow_PinConfiguration,
                                        struct _AMMediaType> Test_I_Target_Stream_Module_DirectShowSource;
#else
typedef Stream_Module_CamSource_V4L_T<ACE_SYNCH_MUTEX,

                                      Test_I_Source_Stream_SessionMessage,
                                      Test_I_Source_Stream_Message,

                                      Test_I_Source_Stream_ModuleHandlerConfiguration,

                                      Test_I_Source_StreamState,

                                      Test_I_Source_Stream_SessionData,
                                      Test_I_Source_Stream_SessionData_t,

                                      Test_I_Source_Stream_StatisticData> Test_I_Stream_Module_CamSource;
#endif
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
                              Common_TimePolicy_t,                             // time policy
                              Stream_ModuleConfiguration,                      // module configuration type
                              Test_I_Source_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Stream_Module_CamSource);                 // writer type
#if defined (ACE_WIN32) || defined (ACE_WIN64)
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
                              Common_TimePolicy_t,                             // time policy
                              Stream_ModuleConfiguration,                      // module configuration type
                              Test_I_Target_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Target_Stream_Module_DirectShowSource);   // writer type
#endif

//typedef Stream_Decoder_AVIDecoder_T<Test_I_Target_Stream_SessionMessage,
//                                    Test_I_Target_Stream_Message,

//                                    Test_I_Target_Stream_ModuleHandlerConfiguration,

//                                    Test_I_Target_Stream_SessionData> Test_I_Target_Stream_Module_AVIDecoder;
//DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
//                              Common_TimePolicy_t,                             // time policy
//                              Stream_ModuleConfiguration,                      // module configuration type
//                              Test_I_Target_Stream_ModuleHandlerConfiguration, // module handler configuration type
//                              Test_I_Target_Stream_Module_AVIDecoder);         // writer type

typedef Stream_Module_SplitterH_T<ACE_SYNCH_MUTEX,
                                  ////
                                  Test_I_Target_Stream_SessionMessage,
                                  Test_I_Target_Stream_Message,
                                  ////
                                  Test_I_Target_Stream_ModuleHandlerConfiguration,
                                  ////
                                  Test_I_Target_StreamState,
                                  ////
                                  Test_I_Target_Stream_SessionData,
                                  Test_I_Target_Stream_SessionData_t,
                                  ////
                                  Test_I_RuntimeStatistic_t> Test_I_Target_Stream_Module_Splitter;
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
                              Common_TimePolicy_t,                             // time policy
                              Stream_ModuleConfiguration,                      // module configuration type
                              Test_I_Target_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Target_Stream_Module_Splitter);           // writer type

typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_MUTEX,
                                     ////
                                     Test_I_Source_Stream_SessionMessage,
                                     Test_I_Source_Stream_Message,
                                     ////
                                     Test_I_Source_Stream_ModuleHandlerConfiguration,
                                     ////
                                     Test_I_Source_StreamState,
                                     ////
                                     Test_I_Source_Stream_SessionData,
                                     Test_I_Source_Stream_SessionData_t,
                                     ////
                                     Test_I_Source_Stream_StatisticData,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Source_InetConnectionManager_t> Test_I_Source_Stream_Module_Net_Writer_t;
typedef Stream_Module_Net_IOReader_T<Test_I_Source_Stream_SessionMessage,
                                     Test_I_Source_Stream_Message,
                                     ////
                                     Test_I_Source_Configuration,
                                     ////
                                     Test_I_Source_Stream_ModuleHandlerConfiguration,
                                     ////
                                     Test_I_Source_Stream_SessionData,
                                     Test_I_Source_Stream_SessionData_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Source_InetConnectionManager_t> Test_I_Source_Stream_Module_Net_Reader_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                    // task synch type
                          Common_TimePolicy_t,                             // time policy
                          Stream_ModuleConfiguration,                      // module configuration type
                          Test_I_Source_Stream_ModuleHandlerConfiguration, // module handler configuration type
                          Test_I_Source_Stream_Module_Net_Reader_t,        // reader type
                          Test_I_Source_Stream_Module_Net_Writer_t,        // writer type
                          Test_I_Source_Stream_Module_Net_IO);             // name

//typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_MUTEX,
//                                     ////
//                                     Test_I_Target_Stream_SessionMessage,
//                                     Test_I_Target_Stream_Message,
//                                     ////
//                                     Test_I_Target_Stream_ModuleHandlerConfiguration,
//                                     ////
//                                     Test_I_Target_StreamState,
//                                     ////
//                                     Test_I_Target_Stream_SessionData,
//                                     Test_I_Target_Stream_SessionData_t,
//                                     ////
//                                     Test_I_RuntimeStatistic_t,
//                                     ////
//                                     ACE_INET_Addr,
//                                     Test_I_Target_InetConnectionManager_t> Test_I_Target_Stream_Module_Net_Writer_t;
//typedef Stream_Module_Net_IOReader_T<Test_I_Target_Stream_SessionMessage,
//                                     Test_I_Target_Stream_Message,
//                                     ////
//                                     Test_I_Target_Configuration,
//                                     ////
//                                     Test_I_Target_Stream_ModuleHandlerConfiguration,
//                                     ////
//                                     Test_I_Target_Stream_SessionData,
//                                     Test_I_Target_Stream_SessionData_t,
//                                     ////
//                                     ACE_INET_Addr,
//                                     Test_I_Target_InetConnectionManager_t> Test_I_Target_Stream_Module_Net_Reader_t;
//DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                    // task synch type
//                          Common_TimePolicy_t,                             // time policy
//                          Stream_ModuleConfiguration,                      // module configuration type
//                          Test_I_Target_Stream_ModuleHandlerConfiguration, // module handler configuration type
//                          Test_I_Target_Stream_Module_Net_Reader_t,        // reader type
//                          Test_I_Target_Stream_Module_Net_Writer_t,        // writer type
//                          Test_I_Target_Stream_Module_Net_IO);             // name

typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_Stream_SessionMessage,
                                             Test_I_Source_Stream_Message,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_Stream_SessionData,
                                             Test_I_Source_Stream_SessionData_t> Test_I_Source_Stream_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Source_Stream_SessionMessage,
                                             Test_I_Source_Stream_Message,
                                             Test_I_CommandType_t,
                                             Test_I_Source_Stream_StatisticData,
                                             Test_I_Source_Stream_SessionData,
                                             Test_I_Source_Stream_SessionData_t> Test_I_Source_Stream_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                       // task synch type
                          Common_TimePolicy_t,                                // time policy type
                          Stream_ModuleConfiguration,                         // module configuration type
                          Test_I_Source_Stream_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_Source_Stream_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Source_Stream_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Source_Stream_Module_RuntimeStatistic);      // name
typedef Stream_Module_Statistic_ReaderTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_Stream_SessionMessage,
                                             Test_I_Target_Stream_Message,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_Stream_SessionData,
                                             Test_I_Target_Stream_SessionData_t> Test_I_Target_Stream_Module_Statistic_ReaderTask_t;
typedef Stream_Module_Statistic_WriterTask_T<ACE_MT_SYNCH,
                                             Common_TimePolicy_t,
                                             Test_I_Target_Stream_SessionMessage,
                                             Test_I_Target_Stream_Message,
                                             Test_I_CommandType_t,
                                             Test_I_RuntimeStatistic_t,
                                             Test_I_Target_Stream_SessionData,
                                             Test_I_Target_Stream_SessionData_t> Test_I_Target_Stream_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (ACE_MT_SYNCH,                                       // task synch type
                          Common_TimePolicy_t,                                // time policy type
                          Stream_ModuleConfiguration,                         // module configuration type
                          Test_I_Target_Stream_ModuleHandlerConfiguration,    // module handler configuration type
                          Test_I_Target_Stream_Module_Statistic_ReaderTask_t, // reader type
                          Test_I_Target_Stream_Module_Statistic_WriterTask_t, // writer type
                          Test_I_Target_Stream_Module_RuntimeStatistic);      // name

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Vis_Target_DirectShow_T<Test_I_Source_Stream_SessionMessage,
                                       Test_I_Source_Stream_Message,
                                       ///
                                       Test_I_Source_Stream_ModuleHandlerConfiguration,
                                       ///
                                       Test_I_Source_Stream_SessionData,
                                       Test_I_Source_Stream_SessionData_t> Test_I_Source_Stream_Module_Display;
typedef Stream_Vis_Target_DirectShow_T<Test_I_Target_Stream_SessionMessage,
                                       Test_I_Target_Stream_Message,
                                       ///
                                       Test_I_Target_Stream_ModuleHandlerConfiguration,
                                       ///
                                       Test_I_Target_Stream_SessionData,
                                       Test_I_Target_Stream_SessionData_t> Test_I_Target_Stream_Module_Display;
#else
typedef Stream_Module_Vis_GTK_DrawingArea_T<Test_I_Source_Stream_SessionMessage,
                                            Test_I_Source_Stream_Message,

                                            Test_I_Source_Stream_ModuleHandlerConfiguration,

                                            Test_I_Source_Stream_SessionData,
                                            Test_I_Source_Stream_SessionData_t> Test_I_Source_Stream_Module_Display;
typedef Stream_Module_Vis_GTK_DrawingArea_T<Test_I_Target_Stream_SessionMessage,
                                            Test_I_Target_Stream_Message,

                                            Test_I_Target_Stream_ModuleHandlerConfiguration,

                                            Test_I_Target_Stream_SessionData,
                                            Test_I_Target_Stream_SessionData_t> Test_I_Target_Stream_Module_Display;
#endif
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
                              Common_TimePolicy_t,                             // time policy
                              Stream_ModuleConfiguration,                      // module configuration type
                              Test_I_Source_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Source_Stream_Module_Display);            // writer type
DATASTREAM_MODULE_INPUT_ONLY (ACE_MT_SYNCH,                                    // task synch type
                              Common_TimePolicy_t,                             // time policy
                              Stream_ModuleConfiguration,                      // module configuration type
                              Test_I_Target_Stream_ModuleHandlerConfiguration, // module handler configuration type
                              Test_I_Target_Stream_Module_Display);            // writer type

#endif
