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

#ifndef STREAM_NET_SERVER_STREAM_H
#define STREAM_NET_SERVER_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Time_Policy.h"

#include "common_iget.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_inotify.h"
#include "stream_streammodule_base.h"

#include "stream_net_listener.h"

// forward declarations
template <ACE_SYNCH_DECL,
          class TIME_POLICY>
class ACE_Module;

// global variables
extern const char libacestream_default_net_server_stream_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName, // *TODO*: use a variadic character array
          ////////////////////////////////
          typename ControlType,
          typename NotificationType,
          typename StatusType,       // state machine-
          typename StateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType,       // implements Common_ITimer
          ///////////////////////////////
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename SessionManagerType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ListenerType, // implements Net_IListener_T
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_Server_Stream_T
 : public Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

 public:
  // convenient types
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> STREAM_BASE_T;

  Stream_Module_Net_Server_Stream_T ();
  inline virtual ~Stream_Module_Net_Server_Stream_T () { inherited::shutdown (); }

  // override (part of) Stream_IStream_T
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

 protected:
  // convenient types
  typedef Stream_INotify_T<NotificationType> INOTIFY_T;
  typedef Stream_Module_Net_Listener_Reader_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              HandlerConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType> READER_T;
  typedef Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       HandlerConfigurationType,
                                       ControlType,
                                       NotificationType,
                                       StateType,
                                       StatisticContainerType,
                                       SessionManagerType,
                                       TimerManagerType,
                                       ListenerType,
                                       UserDataType> WRITER_T;
  typedef typename SessionMessageType::DATA_T::DATA_T SESSION_DATA_T;
  typedef Stream_StreamModule_T<ACE_SYNCH_USE,                               // task synch type
                                TimePolicyType,                              // time policy
                                SESSION_DATA_T,                              // session data type
                                enum Stream_SessionMessageType,              // session event type
                                struct Stream_ModuleConfiguration,           // module configuration type
                                HandlerConfigurationType,                    // module handler configuration type
                                libacestream_default_net_listener_module_name_string, // name
                                INOTIFY_T,                                   // stream notification interface type
                                READER_T,                                    // reader type
                                WRITER_T> LISTENER_MODULE_T;                 // writer type

 private:
  // convenient types
  typedef Stream_Module_Net_Server_Stream_T<ACE_SYNCH_USE,
                                            TimePolicyType,
                                            StreamName,
                                            ControlType,
                                            NotificationType,
                                            StatusType,
                                            StateType,
                                            ConfigurationType,
                                            StatisticContainerType,
                                            TimerManagerType,
                                            HandlerConfigurationType,
                                            SessionManagerType,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ListenerType,
                                            UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Server_Stream_T (const Stream_Module_Net_Server_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_Server_Stream_T& operator= (const Stream_Module_Net_Server_Stream_T&))
};

// include template definition
#include "stream_net_server_stream.inl"

#endif
