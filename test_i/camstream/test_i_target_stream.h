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

#ifndef TEST_I_TARGET_STREAM_H
#define TEST_I_TARGET_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "stream_module_io_stream.h"

#include "net_connection_manager.h"

#include "test_i_target_common.h"

// forward declarations
class Stream_IAllocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Target_DirectShow_Configuration,
                                 Test_I_Target_DirectShow_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_InetConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Target_MediaFoundation_Configuration,
                                 Test_I_Target_MediaFoundation_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_InetConnectionManager_t;
#else
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Target_Configuration,
                                 Test_I_Target_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 Test_I_Target_UserData> Test_I_Target_InetConnectionManager_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Target_DirectShow_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Stream_ControlType,
                                        Stream_SessionMessageType,
                                        Stream_StateMachine_ControlState,
                                        Test_I_Target_DirectShow_StreamState,
                                        Test_I_Target_DirectShow_StreamConfiguration,
                                        Test_I_RuntimeStatistic_t,
                                        Stream_ModuleConfiguration,
                                        Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_Target_DirectShow_SessionData,
                                        Test_I_Target_DirectShow_SessionData_t,
                                        ACE_Message_Block,
                                        Test_I_Target_DirectShow_Stream_Message,
                                        Test_I_Target_DirectShow_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_DirectShow_InetConnectionManager_t>
{
 public:
  Test_I_Target_DirectShow_Stream (const std::string&); // name
  virtual ~Test_I_Target_DirectShow_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_Target_DirectShow_StreamConfiguration&, // configuration
                           bool = true,                                         // setup pipeline ?
                           bool = true);                                        // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Stream_ControlType,
                                        Stream_SessionMessageType,
                                        Stream_StateMachine_ControlState,
                                        Test_I_Target_DirectShow_StreamState,
                                        Test_I_Target_DirectShow_StreamConfiguration,
                                        Test_I_RuntimeStatistic_t,
                                        Stream_ModuleConfiguration,
                                        Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_Target_DirectShow_SessionData,
                                        Test_I_Target_DirectShow_SessionData_t,
                                        ACE_Message_Block,
                                        Test_I_Target_DirectShow_Stream_Message,
                                        Test_I_Target_DirectShow_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_DirectShow_InetConnectionManager_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Stream ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Stream (const Test_I_Target_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Stream& operator= (const Test_I_Target_DirectShow_Stream&))

  IGraphBuilder* graphBuilder_;
};

class Test_I_Target_MediaFoundation_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Stream_ControlType,
                                        Stream_SessionMessageType,
                                        Stream_StateMachine_ControlState,
                                        Test_I_Target_MediaFoundation_StreamState,
                                        Test_I_Target_MediaFoundation_StreamConfiguration,
                                        Test_I_RuntimeStatistic_t,
                                        Stream_ModuleConfiguration,
                                        Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_Target_MediaFoundation_SessionData,
                                        Test_I_Target_MediaFoundation_SessionData_t,
                                        ACE_Message_Block,
                                        Test_I_Target_MediaFoundation_Stream_Message,
                                        Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_MediaFoundation_InetConnectionManager_t>
{
 public:
  Test_I_Target_MediaFoundation_Stream (const std::string&); // name
  virtual ~Test_I_Target_MediaFoundation_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_Target_MediaFoundation_StreamConfiguration&, // configuration
                           bool = true,                                              // setup pipeline ?
                           bool = true);                                             // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Stream_ControlType,
                                        Stream_SessionMessageType,
                                        Stream_StateMachine_ControlState,
                                        Test_I_Target_MediaFoundation_StreamState,
                                        Test_I_Target_MediaFoundation_StreamConfiguration,
                                        Test_I_RuntimeStatistic_t,
                                        Stream_ModuleConfiguration,
                                        Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_Target_MediaFoundation_SessionData,
                                        Test_I_Target_MediaFoundation_SessionData_t,
                                        ACE_Message_Block,
                                        Test_I_Target_MediaFoundation_Stream_Message,
                                        Test_I_Target_MediaFoundation_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_MediaFoundation_InetConnectionManager_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Stream ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Stream (const Test_I_Target_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Stream& operator= (const Test_I_Target_MediaFoundation_Stream&))

  // media session
  IMFMediaSession* mediaSession_;
  ULONG            referenceCount_;
};
#else
class Test_I_Target_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Stream_ControlType,
                                        Stream_SessionMessageType,
                                        Stream_StateMachine_ControlState,
                                        Test_I_Target_StreamState,
                                        Test_I_Target_StreamConfiguration,
                                        Test_I_RuntimeStatistic_t,
                                        Stream_ModuleConfiguration,
                                        Test_I_Target_ModuleHandlerConfiguration,
                                        Test_I_Target_SessionData,
                                        Test_I_Target_SessionData_t,
                                        ACE_Message_Block,
                                        Test_I_Target_Stream_Message,
                                        Test_I_Target_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_InetConnectionManager_t>
{
 public:
  Test_I_Target_Stream (const std::string&); // name
  virtual ~Test_I_Target_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const Test_I_Target_StreamConfiguration&, // configuration
                           bool = true,                              // setup pipeline ?
                           bool = true);                             // reset session data ?

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Test_I_RuntimeStatistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        Stream_ControlType,
                                        Stream_SessionMessageType,
                                        Stream_StateMachine_ControlState,
                                        Test_I_Target_StreamState,
                                        Test_I_Target_StreamConfiguration,
                                        Test_I_RuntimeStatistic_t,
                                        Stream_ModuleConfiguration,
                                        Test_I_Target_ModuleHandlerConfiguration,
                                        Test_I_Target_SessionData,
                                        Test_I_Target_SessionData_t,
                                        ACE_Message_Block,
                                        Test_I_Target_Stream_Message,
                                        Test_I_Target_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_InetConnectionManager_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream (const Test_I_Target_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream& operator= (const Test_I_Target_Stream&))
};
#endif

#endif
