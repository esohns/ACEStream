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

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"

#include "stream_misc_parser.h"

#include "stream_module_htmlparser.h"

#include "stream_net_io.h"
#include "stream_module_source_http_get.h"

#include "stream_stat_statistic_report.h"

#include "http_module_parser.h"
#include "http_module_streamer.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"
#include "test_i_message.h"
//#include "test_i_module_httpparser.h"
#include "test_i_session_message.h"

// declare module(s)
typedef Stream_Module_Net_IOReader_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage,
                                     struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_HTTPGet_StreamState,
                                     struct Test_I_Stream_SessionData,
                                     Test_I_Stream_SessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Stream_InetConnectionManager_t,
                                     struct Stream_UserData> Test_I_Net_Reader_t;
typedef Stream_Module_Net_IOWriter_T<ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Test_I_Stream_Message,
                                     Test_I_Stream_SessionMessage,
                                     struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Test_I_HTTPGet_StreamState,
                                     struct Test_I_Stream_SessionData,
                                     Test_I_Stream_SessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     ACE_INET_Addr,
                                     Test_I_Stream_InetConnectionManager_t,
                                     struct Stream_UserData> Test_I_Net_Writer_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Stream_SessionData,         // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_io_module_name_string,
                          Stream_INotify_t,                         // stream notification interface type
                          Test_I_Net_Reader_t,                      // reader type
                          Test_I_Net_Writer_t,                      // writer type
                          Test_I_Net_IO);                           // name

typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               Test_I_Stream_Message,
                               Test_I_Stream_SessionMessage> Test_I_HTTPStreamer;
typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Stream_ControlMessage_t,
                              Test_I_Stream_Message,
                              Test_I_Stream_SessionMessage,
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                              enum Stream_ControlType,
                              enum Stream_SessionMessageType,
                              struct Test_I_HTTPGet_StreamState,
                              struct Test_I_Stream_SessionData,
                              Test_I_Stream_SessionData_t,
                              struct Stream_Statistic,
                              Common_Timer_Manager_t,
                              struct Stream_UserData> Test_I_HTTPParser;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Stream_SessionData,         // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_parser_module_name_string,
                          Stream_INotify_t,                         // stream notification interface type
                          Test_I_HTTPStreamer,                      // reader type
                          Test_I_HTTPParser,                        // writer type
                          Test_I_HTTPMarshal);                      // name

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Test_I_Stream_SessionData,
                                                      Test_I_Stream_SessionData_t> Test_I_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Stream_Message,
                                                      Test_I_Stream_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Test_I_Stream_SessionData,
                                                      Test_I_Stream_SessionData_t> Test_I_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Stream_SessionData,         // session data type
                          enum Stream_SessionMessageType,           // session event type
                          struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                         // stream notification interface type
                          Test_I_Statistic_ReaderTask_t,            // reader type
                          Test_I_Statistic_WriterTask_t,            // writer type
                          Test_I_StatisticReport);                  // name

typedef Stream_Module_Net_Source_HTTP_Get_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_Stream_Message,
                                            Test_I_Stream_SessionMessage> Test_I_HTTPGet;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_stat_report_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_HTTPGet);                          // writer type

//typedef Stream_Module_HTMLParser_T<ACE_MT_SYNCH,
//                                   Common_TimePolicy_t,
//                                   struct Test_I_HTTPGet_ModuleHandlerConfiguration,
//                                   Stream_ControlMessage_t,
//                                   Test_I_Stream_Message,
//                                   Test_I_Stream_SessionMessage,
//                                   Test_I_Stream_SessionData_t,
//                                   struct Test_I_Stream_SessionData,
//                                   struct Test_I_SAXParserContext> Test_I_HTMLParser;
//DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,         // session data type
//                              enum Stream_SessionMessageType,           // session event type
//                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_html_parser_module_name_string,
//                              Stream_INotify_t,                         // stream notification interface type
//                              Test_I_HTMLParser);                       // writer type

typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Stream_Message,
                                   Test_I_Stream_SessionMessage> Test_I_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Stream_SessionData,         // session data type
                              enum Stream_SessionMessageType,           // session event type
                              struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                         // stream notification interface type
                              Test_I_FileWriter);                       // writer type

#endif
