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

#ifndef BRANCH_COMMON_MODULES_H
#define BRANCH_COMMON_MODULES_H

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_aggregator.h"
#include "stream_misc_queue_source.h"

#include "stream_stat_statistic_report.h"

#include "branch_stream_common.h"

// declare module(s)
typedef Stream_Module_QueueReader_T <ACE_MT_SYNCH,
                                     Stream_ControlMessage_t,
                                     Branch_Message,
                                     Branch_SessionMessage,
                                     struct Branch_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Branch_StreamState,
                                     struct Branch_SessionData,
                                     Branch_SessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     struct Stream_UserData> Branch_Source;
DATASTREAM_MODULE_INPUT_ONLY (struct Branch_SessionData,                // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct Branch_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_queue_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Branch_Source);                          // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Branch_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Branch_Message,
                                                      Branch_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Branch_SessionData,
                                                      Branch_SessionData_t> Branch_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Branch_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Branch_Message,
                                                      Branch_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Branch_SessionData,
                                                      Branch_SessionData_t> Branch_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Branch_SessionData,                // session data type
                          enum Stream_SessionMessageType,            // session event type
                          struct Branch_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                          // stream notification interface type
                          Branch_StatisticReport_ReaderTask_t,      // reader type
                          Branch_StatisticReport_WriterTask_t,      // writer type
                          Branch_StatisticReport);                  // name

typedef Stream_Miscellaneous_Distributor_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      Branch_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Branch_Message,
                                                      Branch_SessionMessage,
                                                      Branch_SessionData_t> Branch_Distributor_Reader_t;
typedef Stream_Miscellaneous_Distributor_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      Branch_ModuleHandlerConfiguration,
                                                      Stream_ControlMessage_t,
                                                      Branch_Message,
                                                      Branch_SessionMessage,
                                                      Branch_SessionData_t> Branch_Distributor_Writer_t;
DATASTREAM_MODULE_DUPLEX (struct Branch_SessionData,                 // session data type
                          enum Stream_SessionMessageType,            // session event type
                          struct Branch_ModuleHandlerConfiguration,  // module handler configuration type
                          libacestream_default_misc_distributor_module_name_string,
                          Stream_INotify_t,                          // stream notification interface type
                          Branch_Distributor_Reader_t,               // reader type
                          Branch_Distributor_Writer_t,               // writer type
                          Branch_Distributor);                       // name

//////////////////////////////////////////

typedef Stream_Module_Aggregator_WriterTask_2<ACE_MT_SYNCH,
                                              Common_TimePolicy_t,
                                              Branch_ModuleHandlerConfiguration,
                                              Stream_ControlMessage_t,
                                              Branch_Message,
                                              Branch_SessionMessage> Branch_Aggregator_Writer_t;
DATASTREAM_MODULE_DUPLEX_A (struct Branch_SessionData,                 // session data type
                            enum Stream_SessionMessageType,            // session event type
                            struct Branch_ModuleHandlerConfiguration,  // module handler configuration type
                            libacestream_default_misc_aggregator_module_name_string,
                            Stream_INotify_t,                          // stream notification interface type
                            Branch_Aggregator_Writer_t::READER_TASK_T, // reader type
                            Branch_Aggregator_Writer_t,                // writer type
                            Branch_Aggregator);                        // name

#endif
