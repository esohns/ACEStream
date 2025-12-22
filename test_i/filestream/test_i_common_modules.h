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
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_session_manager.h"
#include "stream_streammodule_base.h"

#include "stream_file_sink.h"
#include "stream_file_source.h"

#include "stream_net_output.h"

#include "stream_stat_statistic_report.h"

#include "test_i_connection_common.h"
#include "test_i_message.h"
#include "test_i_module_eventhandler.h"
#include "test_i_session_message.h"
#include "test_i_source_common.h"
#include "test_i_target_common.h"

// outbound
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Test_I_Source_Message_t,
                                    Test_I_Source_SessionMessage,
                                    struct Test_I_Source_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Test_I_Source_StreamState,
                                    struct Stream_Statistic,
                                    Test_I_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Test_I_FileReader;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Source_SessionData,                // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct Test_I_Source_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Test_I_FileReader);                              // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Source_Message_t,
                                                      Test_I_Source_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Source_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Source_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Source_Message_t,
                                                      Test_I_Source_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Source_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Source_SessionData,                // session data type
                          enum Stream_SessionMessageType,                  // session event type
                          struct Test_I_Source_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                          // stream notification interface type
                          Test_I_Source_Module_Statistic_ReaderTask_t,     // reader type
                          Test_I_Source_Module_Statistic_WriterTask_t,     // writer type
                          Test_I_Source_StatisticReport);                  // name

// outbound
typedef Stream_Module_Net_OutputWriter_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_I_Source_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_I_Source_Message_t,
                                         Test_I_Source_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         Test_I_Source_TCPConnectionManager_t,
                                         struct Stream_UserData> Test_I_Source_Module_TCP_Writer_t;
typedef Stream_Module_Net_OutputReader_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_I_Source_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_I_Source_Message_t,
                                         Test_I_Source_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         Test_I_Source_TCPConnectionManager_t,
                                         struct Stream_UserData> Test_I_Source_Module_TCP_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Source_SessionData,                // session data type
                          enum Stream_SessionMessageType,                  // session event type
                          struct Test_I_Source_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_net_output_module_name_string,
                          Stream_INotify_t,                                // stream notification interface type
                          Test_I_Source_Module_TCP_Reader_t,               // reader type
                          Test_I_Source_Module_TCP_Writer_t,               // writer type
                          Test_I_Source_TCP_Output);                       // name

typedef Stream_Module_Net_OutputWriter_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_I_Source_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_I_Source_Message_t,
                                         Test_I_Source_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         Test_I_Source_UDPConnectionManager_t,
                                         struct Stream_UserData> Test_I_Source_Module_UDP_Writer_t;
typedef Stream_Module_Net_OutputReader_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         struct Test_I_Source_ModuleHandlerConfiguration,
                                         Stream_ControlMessage_t,
                                         Test_I_Source_Message_t,
                                         Test_I_Source_SessionMessage,
                                         enum Stream_ControlType,
                                         enum Stream_SessionMessageType,
                                         Test_I_Source_UDPConnectionManager_t,
                                         struct Stream_UserData> Test_I_Source_Module_UDP_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Source_SessionData,                   // session data type
                          enum Stream_SessionMessageType,                     // session event type
                          struct Test_I_Source_ModuleHandlerConfiguration,    // module handler configuration type
                          libacestream_default_net_output_module_name_string,
                          Stream_INotify_t,                                   // stream notification interface type
                          Test_I_Source_Module_UDP_Reader_t,                  // reader type
                          Test_I_Source_Module_UDP_Writer_t,                  // writer type
                          Test_I_Source_UDP_Output);                          // name

// inbound
typedef Stream_Module_Net_InputWriter_T<ACE_MT_SYNCH,
                                        Stream_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        struct Test_I_Target_StreamState,
                                        struct Stream_Statistic,
                                        Test_I_SessionManager_2,
                                        Common_Timer_Manager_t,
                                        ACE_INET_Addr,
                                        Test_I_Target_TCPConnectionManager_t,
                                        struct Stream_UserData> Test_I_Target_Module_TCP_Writer_t;
typedef Stream_Module_Net_InputReader_T<ACE_MT_SYNCH,
                                        Stream_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        struct Test_I_Target_StreamState,
                                        struct Stream_Statistic,
                                        Test_I_SessionManager_2,
                                        Common_Timer_Manager_t,
                                        ACE_INET_Addr,
                                        Test_I_Target_TCPConnectionManager_t,
                                        struct Stream_UserData> Test_I_Target_Module_TCP_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Target_SessionData,                  // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Test_I_Target_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_net_input_module_name_string,
                          Stream_INotify_t,                                  // stream notification interface type
                          Test_I_Target_Module_TCP_Reader_t,                 // reader type
                          Test_I_Target_Module_TCP_Writer_t,                 // writer type
                          Test_I_Target_TCP_Input);                          // name

typedef Stream_Module_Net_InputWriter_T<ACE_MT_SYNCH,
                                        Stream_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        struct Test_I_Target_StreamState,
                                        struct Stream_Statistic,
                                        Test_I_SessionManager_2,
                                        Common_Timer_Manager_t,
                                        ACE_INET_Addr,
                                        Test_I_Target_UDPConnectionManager_t,
                                        struct Stream_UserData> Test_I_Target_Module_UDP_Writer_t;
typedef Stream_Module_Net_InputReader_T<ACE_MT_SYNCH,
                                        Stream_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        struct Test_I_Target_StreamState,
                                        struct Stream_Statistic,
                                        Test_I_SessionManager_2,
                                        Common_Timer_Manager_t,
                                        ACE_INET_Addr,
                                        Test_I_Target_UDPConnectionManager_t,
                                        struct Stream_UserData> Test_I_Target_Module_UDP_Reader_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Target_SessionData,                  // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Test_I_Target_ModuleHandlerConfiguration,   // module handler configuration type
                          libacestream_default_net_input_module_name_string,
                          Stream_INotify_t,                                  // stream notification interface type
                          Test_I_Target_Module_UDP_Reader_t,                 // reader type
                          Test_I_Target_Module_UDP_Writer_t,                 // writer type
                          Test_I_Target_UDP_Input);                          // name

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Target_Message_t,
                                                      Test_I_Target_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Target_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Test_I_Target_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Test_I_Target_Message_t,
                                                      Test_I_Target_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Test_I_Target_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Test_I_Target_SessionData,                   // session data type
                          enum Stream_SessionMessageType,                     // session event type
                          struct Test_I_Target_ModuleHandlerConfiguration,    // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                             // stream notification interface type
                          Test_I_Target_Module_Statistic_ReaderTask_t,        // reader type
                          Test_I_Target_Module_Statistic_WriterTask_t,        // writer type
                          Test_I_Target_StatisticReport);                     // name

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Test_I_Target_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Test_I_Target_Message_t,
                                   Test_I_Target_SessionMessage> Test_I_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Target_SessionData,                // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct Test_I_Target_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Test_I_FileWriter);                              // writer type

typedef Test_I_Stream_Module_EventHandler_T<struct Stream_ModuleConfiguration,
                                            struct Test_I_Source_ModuleHandlerConfiguration,
                                            Stream_ControlMessage_t,
                                            Test_I_Source_Message_t,
                                            Test_I_Source_SessionMessage,
                                            struct Test_I_Source_SessionData,
                                            Test_I_Source_SessionData_t,
                                            struct Stream_UserData> Test_I_Stream_Source_EventHandler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Source_SessionData,                // session data type
                              enum Stream_SessionMessageType,                  // session event type
                              struct Test_I_Source_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Test_I_Stream_Source_EventHandler);              // writer type

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Test_I_Target_ModuleHandlerConfiguration,
                                       Stream_ControlMessage_t,
                                       Test_I_Target_Message_t,
                                       Test_I_Target_SessionMessage,
                                       struct Test_I_Target_SessionData,
                                       struct Stream_UserData> Test_I_Target_Stream_EventHandler;
DATASTREAM_MODULE_INPUT_ONLY (struct Test_I_Target_SessionData,                            // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Test_I_Target_ModuleHandlerConfiguration,             // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string,
                              Stream_INotify_t,                                            // stream notification interface type
                              Test_I_Target_Stream_EventHandler);                          // writer type

#endif
