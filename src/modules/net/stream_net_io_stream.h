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

#ifndef STREAM_NET_IO_STREAM_H
#define STREAM_NET_IO_STREAM_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Time_Policy.h"

#include "common_iget.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_inotify.h"
#include "stream_streammodule_base.h"

#include "stream_net_exports.h"
#include "stream_net_io.h"

// forward declarations
template <ACE_SYNCH_DECL,
          class TIME_POLICY>
class ACE_Module;

// global variables
extern STREAM_NET_Export const char libacestream_default_net_io_module_name_string[];
extern STREAM_NET_Export const char libacestream_default_net_stream_name_string[];

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
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename AddressType,
          typename ConnectionManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_IO_Stream_T
 : public Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        AllocatorConfigurationType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
 , public Stream_IMessageQueue
 , public Stream_IOutboundDataNotify
 , public Common_ISetR_T<std::string>
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
                        AllocatorConfigurationType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

 public:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;

  Stream_Module_Net_IO_Stream_T ();
  inline virtual ~Stream_Module_Net_IO_Stream_T () { inherited::shutdown (); };

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&,
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
#endif
                           ACE_HANDLE);                                // socket handle

  // override (part of) Stream_IStream_T
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?
  inline virtual std::string name () const { std::string name_s = StreamName; return (name_.empty () ? name_s : name_); };

  // override (part of) Stream_IStreamControl_T
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true,  // recurse upstream (if any) ?
                     bool = true); // locked access ?
  virtual void finished (bool = true); // recurse upstream (if any) ?
  using inherited::flush;
  using inherited::getR;

  // override Common_IStatistic_T
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // implement Stream_IMessageQueue
  // *IMPORTANT NOTE*: these manipulate the 'outbound' queue only
  inline virtual unsigned int flush (bool flushSessionMessages_in = false) { return inherited::messageQueue_.flush (flushSessionMessages_in); };
  inline virtual void waitForIdleState () const { inherited::messageQueue_.waitForIdleState (); };

  // implement Stream_IOutboundDataNotify
  virtual bool initialize (ACE_Notification_Strategy*,                                     // strategy handle
                           const std::string& = ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head")); // module name

  // implement Common_ISetR_T
  inline virtual void setR (const std::string& name_in) { name_ = name_in; };

 protected:
  // convenient types
  typedef Stream_INotify_T<NotificationType> INOTIFY_T;
  typedef Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       HandlerConfigurationType,
                                       ControlType,
                                       NotificationType,
                                       StateType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       StatisticContainerType,
                                       TimerManagerType,
                                       AddressType,
                                       ConnectionManagerType,
                                       UserDataType> READER_T;
  typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       HandlerConfigurationType,
                                       ControlType,
                                       NotificationType,
                                       StateType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       StatisticContainerType,
                                       TimerManagerType,
                                       AddressType,
                                       ConnectionManagerType,
                                       UserDataType> WRITER_T;
  typedef Stream_StreamModule_T<ACE_SYNCH_USE,             // task synch type
                                TimePolicyType,            // time policy
                                Stream_SessionId_t,        // session id type
                                SessionDataType,           // session data type
                                Stream_SessionMessageType, // session event type
                                ModuleConfigurationType,   // module configuration type
                                HandlerConfigurationType,  // module handler configuration type
                                libacestream_default_net_io_module_name_string, // name
                                INOTIFY_T,                 // stream notification interface type
                                READER_T,                  // reader type
                                WRITER_T> IO_MODULE_T;     // writer type

 ACE_HANDLE  handle_; // socket-
 std::string name_;

 private:
  // convenient types
  typedef Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        StreamName,
                                        ControlType,
                                        NotificationType,
                                        StatusType,
                                        StateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        TimerManagerType,
                                        AllocatorConfigurationType,
                                        ModuleConfigurationType,
                                        HandlerConfigurationType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        AddressType,
                                        ConnectionManagerType,
                                        UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T (const Stream_Module_Net_IO_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T& operator= (const Stream_Module_Net_IO_Stream_T&))

  // override Common_IInitialize_T
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  virtual bool initialize (const CONFIGURATION_T&);
#else
  virtual bool initialize (const typename inherited::CONFIGURATION_T&);
#endif
};

// include template definition
#include "stream_net_io_stream.inl"

#endif
