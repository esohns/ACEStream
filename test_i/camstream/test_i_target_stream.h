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
#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_statemachine_control.h"

#include "stream_net_io_stream.h"

#include "net_connection_manager.h"

#include "test_i_target_common.h"
#include "test_i_target_message.h"

// forward declarations
class Stream_IAllocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_DirectShow_ConnectionConfiguration_t,
                                 struct Test_I_Target_DirectShow_ConnectionState,
                                 Test_I_Statistic_t,
                                 struct Test_I_Target_UserData> Test_I_Target_DirectShow_InetConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_MediaFoundation_ConnectionConfiguration_t,
                                 struct Test_I_Target_MediaFoundation_ConnectionState,
                                 Test_I_Statistic_t,
                                 struct Test_I_Target_UserData> Test_I_Target_MediaFoundation_InetConnectionManager_t;
#else
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_ConnectionConfiguration_t,
                                 struct Test_I_Target_ConnectionState,
                                 Test_I_Statistic_t,
                                 struct Test_I_Target_UserData> Test_I_Target_InetConnectionManager_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_Target_DirectShow_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_DirectShow_StreamState,
                                        struct Test_I_Target_DirectShow_StreamConfiguration,
                                        Test_I_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_Target_DirectShow_SessionData,
                                        Test_I_Target_DirectShow_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Target_DirectShow_Stream_Message,
                                        Test_I_Target_DirectShow_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_DirectShow_InetConnectionManager_t,
                                        struct Test_I_Target_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_DirectShow_StreamState,
                                        struct Test_I_Target_DirectShow_StreamConfiguration,
                                        Test_I_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_DirectShow_ModuleHandlerConfiguration,
                                        Test_I_Target_DirectShow_SessionData,
                                        Test_I_Target_DirectShow_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Target_DirectShow_Stream_Message,
                                        Test_I_Target_DirectShow_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_DirectShow_InetConnectionManager_t,
                                        struct Test_I_Target_UserData> inherited;

 public:
  Test_I_Target_DirectShow_Stream ();
  virtual ~Test_I_Target_DirectShow_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE);            // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Stream (const Test_I_Target_DirectShow_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_DirectShow_Stream& operator= (const Test_I_Target_DirectShow_Stream&))

  // helper methods
  void setFormat (IGraphBuilder*,              // builder handle
                  const std::wstring&,         // filter name
                  const struct _AMMediaType&); // media type

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

class Test_I_Target_MediaFoundation_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_MediaFoundation_StreamState,
                                        struct Test_I_Target_MediaFoundation_StreamConfiguration,
                                        Test_I_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_Target_MediaFoundation_SessionData,
                                        Test_I_Target_MediaFoundation_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Target_MediaFoundation_Stream_Message,
                                        Test_I_Target_MediaFoundation_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_MediaFoundation_InetConnectionManager_t,
                                        struct Test_I_Target_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_MediaFoundation_StreamState,
                                        struct Test_I_Target_MediaFoundation_StreamConfiguration,
                                        Test_I_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration,
                                        Test_I_Target_MediaFoundation_SessionData,
                                        Test_I_Target_MediaFoundation_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Target_MediaFoundation_Stream_Message,
                                        Test_I_Target_MediaFoundation_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_MediaFoundation_InetConnectionManager_t,
                                        struct Test_I_Target_UserData> inherited;

 public:
  Test_I_Target_MediaFoundation_Stream ();
  virtual ~Test_I_Target_MediaFoundation_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const CONFIGURATION_T&,
                           ACE_HANDLE); // socket handle

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Stream (const Test_I_Target_MediaFoundation_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_MediaFoundation_Stream& operator= (const Test_I_Target_MediaFoundation_Stream&))

  // *TODO*: re-consider this API
  inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // media session
  IMFMediaSession* mediaSession_;
  ULONG            referenceCount_;
};
#else
class Test_I_Target_Stream
 : public Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_StreamState,
                                        struct Test_I_Target_StreamConfiguration,
                                        Test_I_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        Test_I_Target_SessionData,
                                        Test_I_Target_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Target_Stream_Message,
                                        Test_I_Target_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_InetConnectionManager_t,
                                        struct Test_I_Target_UserData>
{
  typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                        Common_TimePolicy_t,
                                        stream_name_string_,
                                        enum Stream_ControlType,
                                        enum Stream_SessionMessageType,
                                        enum Stream_StateMachine_ControlState,
                                        struct Test_I_Target_StreamState,
                                        struct Test_I_Target_StreamConfiguration,
                                        Test_I_Statistic_t,
                                        Common_Timer_Manager_t,
                                        struct Test_I_AllocatorConfiguration,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration,
                                        Test_I_Target_SessionData,
                                        Test_I_Target_SessionData_t,
                                        Test_I_ControlMessage_t,
                                        Test_I_Target_Stream_Message,
                                        Test_I_Target_Stream_SessionMessage,
                                        ACE_INET_Addr,
                                        Test_I_Target_InetConnectionManager_t,
                                        struct Test_I_Target_UserData> inherited;

 public:
  Test_I_Target_Stream ();
  virtual ~Test_I_Target_Stream ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);                                // socket handle

  // *TODO*: re-consider this API
  void ping ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream (const Test_I_Target_Stream&))
  ACE_UNIMPLEMENTED_FUNC (Test_I_Target_Stream& operator= (const Test_I_Target_Stream&))
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
