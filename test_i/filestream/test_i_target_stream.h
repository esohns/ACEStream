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
#include "ace/Synch_Traits.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "stream_net_io_stream.h"

#include "test_i_common_modules.h"
#include "test_i_session_message.h"
#include "test_i_target_common.h"

// forward declarations
class Stream_IAllocator;

class Test_I_Target_TCPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_StreamState,
                                        struct Test_I_Target_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Common_FlexParserAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        struct Test_I_Target_SessionData, // session data
                                        Test_I_Target_SessionData_t,      // session data container (reference counted)
                                        Test_I_Target_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_TCPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_StreamState,
                                        struct Test_I_Target_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Common_FlexParserAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        struct Test_I_Target_SessionData, // session data
                                        Test_I_Target_SessionData_t,      // session data container (reference counted)
                                        Test_I_Target_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_TCPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_Target_TCPStream ();
  virtual ~Test_I_Target_TCPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE); // socket handle

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_TCPStream (const Test_I_Target_TCPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_TCPStream& operator= (const Test_I_Target_TCPStream&))

  // modules
  Test_I_Target_TCP_IO_Module          netIO_;
  //Test_I_Target_StatisticReport_Module statisticReport_;
  Test_I_FileWriter_Module             fileWriter_;
};

class Test_I_Target_UDPStream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_StreamState,
                                        struct Test_I_Target_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Common_FlexParserAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        struct Test_I_Target_SessionData, // session data
                                        Test_I_Target_SessionData_t,      // session data container (reference counted)
                                        Test_I_Target_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_UDPConnectionManager_t,
                                        struct Stream_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_StreamState,
                                        struct Test_I_Target_StreamConfiguration,
                                        struct Stream_Statistic,
                                        Common_Timer_Manager_t,
                                        struct Common_FlexParserAllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        struct Test_I_Target_SessionData, // session data
                                        Test_I_Target_SessionData_t,      // session data container (reference counted)
                                        Test_I_Target_ControlMessage_t,
                                        Test_I_Target_Message_t,
                                        Test_I_Target_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_UDPConnectionManager_t,
                                        struct Stream_UserData> inherited;

 public:
  Test_I_Target_UDPStream ();
  virtual ~Test_I_Target_UDPStream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ILayout*,
                     bool&);          // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE); // socket handle

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (struct Stream_Statistic&); // return value: statistic data
  virtual void report () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_UDPStream (const Test_I_Target_UDPStream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_UDPStream& operator= (const Test_I_Target_UDPStream&))

  // modules
  Test_I_Target_UDP_IO_Module          netIO_;
  //Test_I_Target_StatisticReport_Module statisticReport_;
  Test_I_FileWriter_Module             fileWriter_;
};

#endif
