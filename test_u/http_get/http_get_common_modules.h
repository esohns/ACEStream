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

#ifndef HTTP_GET_COMMON_MODULES_H
#define HTTP_GET_COMMON_MODULES_H

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"

#include "stream_misc_parser.h"

#include "stream_net_output.h"
#include "stream_module_source_http_get.h"

#include "stream_stat_statistic_report.h"

#include "http_module_parser.h"
#include "http_module_streamer.h"

#include "http_get_network.h"
#include "http_get_stream_common.h"

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct HTTPGet_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> HTTPGet_SessionManager_t;

// declare module(s)
typedef Stream_Module_Net_OutputReader_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct HTTPGet_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         HTTPGet_Message,
                                         HTTPGet_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         HTTPGet_ConnectionManager_t,
                                         struct Stream_UserData> HTTPGet_Net_Reader_t;
typedef Stream_Module_Net_OutputWriter_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct HTTPGet_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         HTTPGet_Message,
                                         HTTPGet_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         HTTPGet_ConnectionManager_t,
                                         struct Stream_UserData> HTTPGet_Net_Writer_t;
DATASTREAM_MODULE_DUPLEX (struct HTTPGet_SessionData,                         // session data type
                          enum Stream_SessionMessageType,                     // session event type
                          struct HTTPGet_ModuleHandlerConfiguration,          // module handler configuration type
                          libacestream_default_net_output_module_name_string,
                          Stream_INotify_t,                                   // stream notification interface type
                          HTTPGet_Net_Reader_t,                               // reader type
                          HTTPGet_Net_Writer_t,                               // writer type
                          HTTPGet_Net_Output);                                // name

typedef HTTP_Module_Streamer_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct HTTPGet_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               HTTPGet_Message,
                               HTTPGet_SessionMessage> HTTPGet_HTTPStreamer;
typedef HTTP_Module_ParserH_T<ACE_MT_SYNCH,
                              Common_TimePolicy_t,
                              Stream_ControlMessage_t,
                              HTTPGet_Message,
                              HTTPGet_SessionMessage,
                              struct HTTPGet_ModuleHandlerConfiguration,
                              enum Stream_ControlType,
                              enum Stream_SessionMessageType,
                              struct HTTPGet_StreamState,
                              struct Stream_Statistic,
                              HTTPGet_SessionManager_t,
                              Common_Timer_Manager_t,
                              struct Stream_UserData> HTTPGet_HTTPParser;
DATASTREAM_MODULE_DUPLEX (struct HTTPGet_SessionData,                // session data type
                          enum Stream_SessionMessageType,            // session event type
                          struct HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_misc_parser_module_name_string,
                          Stream_INotify_t,                          // stream notification interface type
                          HTTPGet_HTTPStreamer,                      // reader type
                          HTTPGet_HTTPParser,                        // writer type
                          HTTPGet_HTTPMarshal);                      // name

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct HTTPGet_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      HTTPGet_Message,
                                                      HTTPGet_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> HTTPGet_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct HTTPGet_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      HTTPGet_Message,
                                                      HTTPGet_SessionMessage,
                                                      HTTP_Method_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> HTTPGet_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct HTTPGet_SessionData,                // session data type
                          enum Stream_SessionMessageType,            // session event type
                          struct HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                          // stream notification interface type
                          HTTPGet_StatisticReport_ReaderTask_t,      // reader type
                          HTTPGet_StatisticReport_WriterTask_t,      // writer type
                          HTTPGet_StatisticReport);                  // name

typedef Stream_Module_Net_Source_HTTP_Get_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct HTTPGet_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            HTTPGet_Message,
                                            HTTPGet_SessionMessage> HTTPGet_HTTPGet;
DATASTREAM_MODULE_INPUT_ONLY (struct HTTPGet_SessionData,                // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_net_http_get_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              HTTPGet_HTTPGet);                          // writer type

typedef Stream_Module_FileWriter_2<Common_TimePolicy_t,
                                   struct HTTPGet_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   HTTPGet_Message,
                                   HTTPGet_SessionMessage> HTTPGet_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (struct HTTPGet_SessionData,                // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              HTTPGet_FileWriter);                       // writer type

#endif
