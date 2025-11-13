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

#ifndef TEST_U_FILECOPY_COMMON_MODULES_H
#define TEST_U_FILECOPY_COMMON_MODULES_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"
#include "stream_session_manager.h"

#include "stream_file_sink.h"
#include "stream_file_source.h"

//#include "stream_misc_messagehandler.h"

#include "stream_stat_statistic_report.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_message.h"
#include "test_u_filecopy_session_message.h"

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Stream_Filecopy_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_U_SessionManager_t;

// declare module(s)
typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
                                    Stream_ControlMessage_t,
                                    Stream_Filecopy_Message,
                                    Stream_Filecopy_SessionMessage,
                                    Stream_Filecopy_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Stream_Filecopy_StreamState,
                                    struct Stream_Statistic,
                                    Test_U_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    struct Stream_UserData> Stream_Filecopy_FileReader;
//typedef Stream_Module_FileReaderH_T<ACE_MT_SYNCH,
//                                    Stream_ControlMessage_t,
//                                    Stream_Filecopy_Message,
//                                    Stream_Filecopy_SessionMessage,
//                                    Stream_Filecopy_ModuleHandlerConfiguration,
//                                    enum Stream_ControlType,
//                                    enum Stream_SessionMessageType,
//                                    struct Stream_Filecopy_StreamState,
//                                    struct Stream_Filecopy_SessionData,
//                                    Stream_Filecopy_SessionData_t,
//                                    struct Stream_Statistic,
//                                    Test_U_StatisticHandlerProactor_t,
//                                    struct Stream_UserData> Stream_Filecopy_AsynchFileReader;
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_source_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              Stream_Filecopy_FileReader);                       // writer type
//DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                    // session event type
//                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
//                              Stream_Filecopy_IStreamNotify_t,                   // stream notification interface type
//                              Stream_Filecopy_AsynchFileReader);                 // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_Filecopy_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_Filecopy_Message,
                                                      Stream_Filecopy_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_Filecopy_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Stream_Filecopy_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Stream_Filecopy_Message,
                                                      Stream_Filecopy_SessionMessage,
                                                      Stream_CommandType_t,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Stream_UserData> Stream_Filecopy_Module_Statistic_WriterTask_t;
//typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_Filecopy_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_Filecopy_Message,
//                                                      Stream_Filecopy_SessionMessage,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Test_U_StatisticHandlerProactor_t,
//                                                      struct Stream_UserData> Stream_Filecopy_Module_Statistic_AsynchReaderTask_t;
//typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
//                                                      Common_TimePolicy_t,
//                                                      struct Stream_Filecopy_ModuleHandlerConfiguration,
//                                                      Stream_ControlMessage_t,
//                                                      Stream_Filecopy_Message,
//                                                      Stream_Filecopy_SessionMessage,
//                                                      Stream_CommandType_t,
//                                                      struct Stream_Statistic,
//                                                      Test_U_StatisticHandlerProactor_t,
//                                                      struct Stream_UserData> Stream_Filecopy_Module_Statistic_AsynchWriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Stream_Filecopy_SessionData,                // session data type
                          enum Stream_SessionMessageType,                    // session event type
                          struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                                  // stream notification interface type
                          Stream_Filecopy_Module_Statistic_ReaderTask_t,     // reader type
                          Stream_Filecopy_Module_Statistic_WriterTask_t,     // writer type
                          Stream_Filecopy_StatisticReport);                  // name
//DATASTREAM_MODULE_DUPLEX (struct Stream_Filecopy_SessionData,                  // session data type
//                          enum Stream_SessionMessageType,                      // session event type
//                          struct Stream_Filecopy_ModuleHandlerConfiguration,   // module handler configuration type
//                          Stream_Filecopy_IStreamNotify_t,                     // stream notification interface type
//                          Stream_Filecopy_Module_Statistic_AsynchReaderTask_t, // reader type
//                          Stream_Filecopy_Module_Statistic_AsynchWriterTask_t, // writer type
//                          Stream_Filecopy_AsynchStatisticReport);              // name

typedef Stream_Module_FileWriter_T<ACE_MT_SYNCH,
                                   Common_TimePolicy_t,
                                   struct Stream_Filecopy_ModuleHandlerConfiguration,
                                   Stream_ControlMessage_t,
                                   Stream_Filecopy_Message,
                                   Stream_Filecopy_SessionMessage> Stream_Filecopy_FileWriter;
DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
                              enum Stream_SessionMessageType,                    // session event type
                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_file_sink_module_name_string,
                              Stream_INotify_t,                                  // stream notification interface type
                              Stream_Filecopy_FileWriter);                       // writer type

//typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
//                                       Common_TimePolicy_t,
//                                       struct Stream_Filecopy_ModuleHandlerConfiguration,
//                                       Stream_ControlMessage_t,
//                                       Stream_Filecopy_Message,
//                                       Stream_Filecopy_SessionMessage,
//                                       struct Stream_Filecopy_SessionData,
//                                       struct Stream_UserData> Stream_Filecopy_MessageHandler;
//DATASTREAM_MODULE_INPUT_ONLY (struct Stream_Filecopy_SessionData,                // session data type
//                              enum Stream_SessionMessageType,                    // session event type
//                              struct Stream_Filecopy_ModuleHandlerConfiguration, // module handler configuration type
//                              libacestream_default_misc_messagehandler_module_name_string,
//                              Stream_INotify_t,                                  // stream notification interface type
//                              Stream_Filecopy_MessageHandler);                       // writer type

#endif
