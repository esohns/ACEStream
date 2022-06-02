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

#ifndef TEST_I_POP_RECEIVE_STREAM_H
#define TEST_I_POP_RECEIVE_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_statemachine_control.h"
#include "stream_streammodule_base.h"

#include "stream_stat_common.h"
#include "stream_stat_statistic_report.h"

#include "pop_common.h"
#include "pop_module_parser.h"
#include "pop_module_streamer.h"
#include "pop_stream.h"
#include "pop_stream_common.h"

#include "test_i_pop_receive_common_modules.h"

class Test_I_POPReceive_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        libacenetwork_default_pop_stream_name_string,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct POP_StreamState,
                        struct POP_StreamConfiguration,
                        POP_Statistic_t,
                        struct Stream_POPReceive_ModuleHandlerConfiguration,
                        struct POP_Stream_SessionData,
                        POP_Stream_SessionData_t,
                        Stream_ControlMessage_t,
                        POP_Message_t,
                        POP_SessionMessage_t>
{
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        libacenetwork_default_pop_stream_name_string,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct POP_StreamState,
                        struct POP_StreamConfiguration,
                        POP_Statistic_t,
                        struct Stream_POPReceive_ModuleHandlerConfiguration,
                        struct POP_Stream_SessionData,
                        POP_Stream_SessionData_t,
                        Stream_ControlMessage_t,
                        POP_Message_t,
                        POP_SessionMessage_t> inherited;

 public:
  Test_I_POPReceive_Stream ();
  inline virtual ~Test_I_POPReceive_Stream () { inherited::shutdown (); }

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // implement Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&); // configuration
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&); // configuration
#endif // ACE_WIN32 || ACE_WIN64

  // implement Common_IStatistic_T
  // *NOTE*: delegate this to rntimeStatistic_
  virtual bool collect (POP_Statistic_t&); // return value: statistic data
  // this is just a dummy (use statisticsReportingInterval instead)
  virtual void report () const;

 private:
  typedef POP_Module_Streamer_T<ACE_MT_SYNCH,
                                 Common_TimePolicy_t,
                                 struct Stream_POPReceive_ModuleHandlerConfiguration,
                                 Stream_ControlMessage_t,
                                 POP_Message_t,
                                 POP_SessionMessage_t> STREAMER_T;
  //typedef POP_Module_Parser_T<ACE_MT_SYNCH,
  //                             Common_TimePolicy_t,
  //                             POP_SessionMessage_t,
  //                             POP_Message_t> PARSER_T;
  typedef POP_Module_Parser_T<ACE_MT_SYNCH,
                               Common_TimePolicy_t,
                               struct Stream_POPReceive_ModuleHandlerConfiguration,
                               Stream_ControlMessage_t,
                               POP_Message_t,
                               POP_SessionMessage_t> PARSER_T;
  //typedef Stream_StreamModule_T<ACE_MT_SYNCH,
  //                              Common_TimePolicy_t,
  //                              struct Stream_ModuleConfiguration,
  //                              ModuleHandlerConfigurationType,
  //                              STREAMER_T,
  //                              BISECTOR_T> MODULE_MARSHAL_T;
  //typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,
  //                                       Common_TimePolicy_t,
  //                                       Stream_ModuleConfiguration,
  //                                       ModuleHandlerConfigurationType,
  //                                       PARSER_T> MODULE_PARSER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct POP_Stream_SessionData,    // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                struct Stream_POPReceive_ModuleHandlerConfiguration,
                                libacenetwork_default_pop_marshal_module_name_string,
                                Stream_INotify_t,                  // stream notification interface type
                                STREAMER_T,
                                PARSER_T> MODULE_MARSHAL_T;

  typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Stream_POPReceive_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        POP_Message_t,
                                                        POP_SessionMessage_t,
                                                        Stream_CommandType_t,
                                                        POP_Statistic_t,
                                                        Common_Timer_Manager_t,
                                                        struct POP_Stream_SessionData,
                                                        POP_Stream_SessionData_t> STATISTIC_READER_T;
  typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                        Common_TimePolicy_t,
                                                        struct Stream_POPReceive_ModuleHandlerConfiguration,
                                                        Stream_ControlMessage_t,
                                                        POP_Message_t,
                                                        POP_SessionMessage_t,
                                                        Stream_CommandType_t,
                                                        POP_Statistic_t,
                                                        Common_Timer_Manager_t,
                                                        struct POP_Stream_SessionData,
                                                        POP_Stream_SessionData_t> STATISTIC_WRITER_T;
  typedef Stream_StreamModule_T<ACE_MT_SYNCH,
                                Common_TimePolicy_t,
                                struct POP_Stream_SessionData,    // session data type
                                enum Stream_SessionMessageType,    // session event type
                                struct Stream_ModuleConfiguration,
                                struct Stream_POPReceive_ModuleHandlerConfiguration,
                                libacestream_default_stat_report_module_name_string,
                                Stream_INotify_t,                  // stream notification interface type
                                STATISTIC_READER_T,
                                STATISTIC_WRITER_T> MODULE_STATISTIC_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_POPReceive_Stream (const Test_I_POPReceive_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_POPReceive_Stream& operator= (const Test_I_POPReceive_Stream&))

  // modules
  Stream_POPReceive_NetSource_Module       source_;
  Stream_POPReceive_AsynchNetSource_Module asynchSource_;
#if defined (SSL_SUPPORT)
  Stream_POPReceive_SSLNetSource_Module    SSLSource_;
#endif // SSL_SUPPORT
  //MODULE_PARSER_T    parser_;
  MODULE_MARSHAL_T                       marshal_;
  MODULE_STATISTIC_T                     statistic_;
  Stream_POPReceive_ProtocolHandler_Module protocolHandler_;
};

#endif
