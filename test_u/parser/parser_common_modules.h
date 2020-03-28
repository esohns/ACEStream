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

#ifndef PARSER_COMMON_MODULES_H
#define PARSER_COMMON_MODULES_H

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_queue_source.h"

#include "stream_stat_statistic_report.h"

#include "parser_stream_common.h"

// declare module(s)
typedef Stream_Module_QueueReader_T <ACE_MT_SYNCH,
                                     Parser_ControlMessage_t,
                                     Parser_Message,
                                     Parser_SessionMessage,
                                     struct Parser_ModuleHandlerConfiguration,
                                     enum Stream_ControlType,
                                     enum Stream_SessionMessageType,
                                     struct Parser_StreamState,
                                     struct Parser_SessionData,
                                     Parser_SessionData_t,
                                     struct Stream_Statistic,
                                     Common_Timer_Manager_t,
                                     struct Stream_UserData> Parser_Source;
DATASTREAM_MODULE_INPUT_ONLY (struct Parser_SessionData,                // session data type
                              enum Stream_SessionMessageType,            // session event type
                              struct Parser_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_misc_queue_module_name_string,
                              Stream_INotify_t,                          // stream notification interface type
                              Parser_Source);                          // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Parser_ModuleHandlerConfiguration,
                                                      Parser_ControlMessage_t,
                                                      Parser_Message,
                                                      Parser_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Parser_SessionData,
                                                      Parser_SessionData_t> Parser_StatisticReport_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Parser_ModuleHandlerConfiguration,
                                                      Parser_ControlMessage_t,
                                                      Parser_Message,
                                                      Parser_SessionMessage,
                                                      int,
                                                      struct Stream_Statistic,
                                                      Common_Timer_Manager_t,
                                                      struct Parser_SessionData,
                                                      Parser_SessionData_t> Parser_StatisticReport_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Parser_SessionData,                // session data type
                          enum Stream_SessionMessageType,            // session event type
                          struct Parser_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Stream_INotify_t,                          // stream notification interface type
                          Parser_StatisticReport_ReaderTask_t,      // reader type
                          Parser_StatisticReport_WriterTask_t,      // writer type
                          Parser_StatisticReport);                  // name

#endif
