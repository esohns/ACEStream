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

#ifndef TEST_I_SOURCE_STREAM_H
#define TEST_I_SOURCE_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_streammodule_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_misc_mediafoundation_callback.h"
#endif

#include "stream_module_target.h"

#include "test_i_common_modules.h"
#include "test_i_connection_common.h"
#include "test_i_source_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaSession;
#endif
class ACE_Message_Block;
class Stream_IAllocator;
class Test_I_Source_Stream_Message;
class Test_I_Source_Stream_SessionMessage;

template <typename ConnectorType>
class Test_I_Source_Stream_T
 : public Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Test_I_Source_StreamState,
                        Test_I_Source_StreamConfiguration,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        Test_I_Source_ModuleHandlerConfiguration,
                        Test_I_Source_SessionData,   // session data
                        Test_I_Source_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_I_Source_Stream_Message,
                        Test_I_Source_Stream_SessionMessage>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , public Stream_Misc_MediaFoundation_Callback_T<Test_I_MediaFoundationConfiguration>
#endif
{
 public:
  Test_I_Source_Stream_T ();
  virtual ~Test_I_Source_Stream_T ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // override (part of) Stream_IStreamControl_T
  //virtual Stream_Module_t* find (const std::string&) const; // module name
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
#endif

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_Source_StreamConfiguration&, // configuration
                           bool = true,                              // setup pipeline ?
                           bool = true);                             // reset session data ?

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_Source_Stream_StatisticData&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_ControlType,
                        Stream_SessionMessageType,
                        Stream_StateMachine_ControlState,
                        Test_I_Source_StreamState,
                        Test_I_Source_StreamConfiguration,
                        Test_I_Source_Stream_StatisticData,
                        Stream_ModuleConfiguration,
                        Test_I_Source_ModuleHandlerConfiguration,
                        Test_I_Source_SessionData,   // session data
                        Test_I_Source_SessionData_t, // session data container (reference counted)
                        ACE_Message_Block,
                        Test_I_Source_Stream_Message,
                        Test_I_Source_Stream_SessionMessage> inherited;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef Stream_Misc_MediaFoundation_Callback_T<Test_I_MediaFoundationConfiguration> inherited2;
#endif

  typedef Test_I_Source_Stream_T<ConnectorType> OWN_TYPE_T;
  typedef Stream_Module_Net_Target_T<ACE_MT_SYNCH,
                                     Common_TimePolicy_t,
                                     Test_I_Source_ModuleHandlerConfiguration,
                                     ACE_Message_Block,
                                     Test_I_Source_Stream_Message,
                                     Test_I_Source_Stream_SessionMessage,
                                     Test_I_Source_SessionData_t,
                                     Test_I_Source_InetConnectionManager_t,
                                     ConnectorType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_MT_SYNCH,                                    // task synch type
                                         Common_TimePolicy_t,                             // time policy
                                         Stream_SessionId_t,                              // session id type
                                         Test_I_Source_SessionData,                       // session data type
                                         Stream_SessionMessageType,                       // session event type
                                         Stream_ModuleConfiguration,                      // module configuration type
                                         Test_I_Source_ModuleHandlerConfiguration, // module handler configuration type
                                         Test_I_IStreamNotify_t,                          // stream notification interface type
                                         WRITER_T> TARGET_MODULE_T;                       // writer type

  //ACE_UNIMPLEMENTED_FUNC (Test_I_Source_Stream_T ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_Stream_T (const Test_I_Source_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Source_Stream_T& operator= (const Test_I_Source_Stream_T&))

  // *TODO*: re-consider this API
  void ping ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // media session
  IMFMediaSession* mediaSession_;
#endif
};

// include template definition
#include "test_i_source_stream.inl"

//////////////////////////////////////////

typedef Test_I_Source_Stream_T<Test_I_Source_TCPConnector_t> Test_I_Source_TCPStream_t;
typedef Test_I_Source_Stream_T<Test_I_Source_SSLTCPConnector_t> Test_I_Source_SSLTCPStream_t;
typedef Test_I_Source_Stream_T<Test_I_Source_TCPAsynchConnector_t> Test_I_Source_AsynchTCPStream_t;
typedef Test_I_Source_Stream_T<Test_I_Source_UDPConnector_t> Test_I_Source_UDPStream_t;
typedef Test_I_Source_Stream_T<Test_I_Source_UDPAsynchConnector_t> Test_I_Source_AsynchUDPStream_t;

#endif
