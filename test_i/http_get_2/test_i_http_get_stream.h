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

#ifndef TEST_I_HTTP_GET_STREAM_H
#define TEST_I_HTTP_GET_STREAM_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "stream_module_source.h"

#include "http_module_parser.h"

#include "test_i_common.h"
#include "test_i_common_modules.h"
#include "test_i_message.h"
#include "test_i_module_htmlparser.h"
#include "test_i_module_httpget.h"
#include "test_i_module_spreadsheetwriter.h"
#include "test_i_session_message.h"

#include "test_i_http_get_common.h"

// forward declarations
class Stream_IAllocator;

template <typename ConnectorType>
class Test_I_HTTPGet_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_HTTPGet_StreamState,
                        struct Test_I_HTTPGet_StreamConfiguration,
                        Test_I_RuntimeStatistic_t,
                        struct Stream_ModuleConfiguration,
                        struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                        struct Test_I_HTTPGet_SessionData,
                        Test_I_HTTPGet_SessionData_t,
                        Test_I_ControlMessage_t,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage>
{
 public:
  Test_I_HTTPGet_Stream_T ();
  virtual ~Test_I_HTTPGet_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_HTTPGet_StreamConfiguration&, // configuration
                           bool = true,                               // setup pipeline ?
                           bool = true);                              // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        enum Stream_ControlType,
                        enum Stream_SessionMessageType,
                        enum Stream_StateMachine_ControlState,
                        struct Test_I_HTTPGet_StreamState,
                        struct Test_I_HTTPGet_StreamConfiguration,
                        Test_I_RuntimeStatistic_t,
                        struct Stream_ModuleConfiguration,
                        struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                        struct Test_I_HTTPGet_SessionData,
                        Test_I_HTTPGet_SessionData_t,
                        Test_I_ControlMessage_t,
                        Test_I_Stream_Message,
                        Test_I_Stream_SessionMessage> inherited;

  typedef Stream_Module_Net_Source_Writer_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            struct Test_I_HTTPGet_ModuleHandlerConfiguration,
                                            Test_I_ControlMessage_t,
                                            Test_I_Stream_Message,
                                            Test_I_Stream_SessionMessage,
                                            ACE_INET_Addr,
                                            struct Test_I_HTTPGet_SocketHandlerConfiguration,
                                            Test_I_HTTPGet_InetConnectionManager_t,
                                            ConnectorType> SOURCE_WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                                     // task synch type
                                         Common_TimePolicy_t,                              // time policy
                                         Stream_SessionId_t,                               // session id type
                                         struct Test_I_HTTPGet_SessionData,                // session data type
                                         enum Stream_SessionMessageType,                   // session event type
                                         struct Stream_ModuleConfiguration,                // module configuration type
                                         struct Test_I_HTTPGet_ModuleHandlerConfiguration, // module handler configuration type
                                         Test_I_IStreamNotify_t,                           // stream notification interface type
                                         SOURCE_WRITER_T> SOURCE_MODULE_T;                 // writer type

  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPGet_Stream_T (const Test_I_HTTPGet_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_HTTPGet_Stream_T& operator= (const Test_I_HTTPGet_Stream_T&))
};

// include template definition
#include "test_i_http_get_stream.inl"

//////////////////////////////////////////

typedef Test_I_HTTPGet_Stream_T<Test_I_HTTPGet_TCPConnector_t> Test_I_HTTPGet_Stream_t;
typedef Test_I_HTTPGet_Stream_T<Test_I_HTTPGet_SSLTCPConnector_t> Test_I_HTTPGet_SSL_Stream_t;
typedef Test_I_HTTPGet_Stream_T<Test_I_HTTPGet_TCPAsynchConnector_t> Test_I_HTTPGet_AsynchStream_t;

#endif
