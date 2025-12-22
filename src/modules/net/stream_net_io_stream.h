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

#include "stream_net_input.h"
#include "stream_net_output.h"

// forward declarations
template <ACE_SYNCH_DECL,
          class TIME_POLICY>
class ACE_Module;

// global variables
extern const char libacestream_default_net_io_stream_name_string[];

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
                        HandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        UserDataType>
 //, public Stream_IMessageQueue
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
                        HandlerConfigurationType,
                        SessionManagerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        UserDataType> inherited;

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
                        SessionMessageType,
                        UserDataType> STREAM_BASE_T;

  Stream_Module_Net_IO_Stream_T ();
  virtual ~Stream_Module_Net_IO_Stream_T ();

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
  virtual bool initialize (const typename inherited::CONFIGURATION_T&,
                           ACE_HANDLE);                                // socket handle

  // override (part of) Stream_IStream_T
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  // override (part of) Stream_IStreamControl_T
  virtual void start ();
  virtual void stop (bool = true,      // wait for completion ?
                     bool = true,   // recurse upstream (if any) ?
                     bool = false); // high priority ?
  virtual bool isRunning () const;
  virtual void finished (bool = true); // recurse upstream (if any) ?
  virtual unsigned int flush (bool = true,   // flush inbound data ?
                              bool = false,  // flush session messages ?
                              bool = false); // flush upstream (if any) ?
  virtual void idle (bool = true,            // wait forever ?
                     bool = true) const;     // recurse upstream (if any) ?
  virtual void wait (bool = true,            // wait for any worker thread(s) ?
                     bool = false,           // wait for upstream (if any) ?
                     bool = false) const;    // wait for downstream (if any) ?
  virtual void pause ();
  virtual void rewind ();
  virtual void control (ControlType,   // control type
                        bool = false); // recurse upstream (if any) ?
  // *NOTE*: the default implementation forwards calls to the head module
  virtual void notify (NotificationType, // notification type
                       bool = false,     // recurse upstream (if any) ?
                       bool = false);    // expedite ?
  virtual StatusType status () const;

  //inline virtual const SessionDataContainerType& getR_2 () const { ACE_ASSERT (inherited::sessionData_); return *inherited::sessionData_; }

  //// implement Stream_IMessageQueue
  //// *IMPORTANT NOTE*: these manipulate the 'outbound' queue only
  //inline virtual int enqueue_head_i (ACE_Message_Block*, ACE_Time_Value* = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); }
  //inline virtual unsigned int flush (bool flushSessionMessages_in = false) { return inherited::messageQueue_.flush (flushSessionMessages_in); }
  //inline virtual void reset () { ACE_ASSERT (false); ACE_NOTSUP; }
  //inline virtual bool isShuttingDown () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); }
  //inline virtual void waitForIdleState () const { inherited::messageQueue_.waitForIdleState (); }

  // implement Stream_IOutboundDataNotify
  virtual const ACE_Notification_Strategy* const getP (bool = false) const; // recurse upstream ?
  virtual bool initialize_2 (ACE_Notification_Strategy*,                                     // strategy handle
                             const std::string& = ACE_TEXT_ALWAYS_CHAR ("ACE_Stream_Head")); // module name

  // *TODO*: remove this and use Stream_IStream_T interface
  // implement Common_ISetR_T
  inline virtual void setR (const std::string& name_in) { inherited::name_ = name_in; }

 protected:
  // convenient types
  typedef Stream_INotify_T<NotificationType> INOTIFY_T;
  typedef Stream_Module_Net_InputReader_T<ACE_SYNCH_USE,
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
                                          AddressType,
                                          ConnectionManagerType,
                                          UserDataType> I_READER_T;
  typedef Stream_Module_Net_InputWriter_T<ACE_SYNCH_USE,
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
                                          AddressType,
                                          ConnectionManagerType,
                                          UserDataType> I_WRITER_T;
  typedef Stream_StreamModule_T<ACE_SYNCH_USE,                                     // task synch type
                                TimePolicyType,                                    // time policy
                                typename SessionMessageType::DATA_T::DATA_T,       // session data type
                                enum Stream_SessionMessageType,                    // session event type
                                struct Stream_ModuleConfiguration,                 // module configuration type
                                HandlerConfigurationType,                          // module handler configuration type
                                libacestream_default_net_input_module_name_string, // name
                                INOTIFY_T,                                         // stream notification interface type
                                I_READER_T,                                        // reader type
                                I_WRITER_T> INPUT_MODULE_T;                        // writer type

  typedef Stream_Module_Net_OutputReader_T<ACE_SYNCH_USE,
                                           TimePolicyType,
                                           HandlerConfigurationType,
                                           ControlMessageType,
                                           DataMessageType,
                                           SessionMessageType,
                                           ControlType,
                                           NotificationType,
                                           ConnectionManagerType,
                                           UserDataType> O_READER_T;
  typedef Stream_Module_Net_OutputWriter_T<ACE_SYNCH_USE,
                                           TimePolicyType,
                                           HandlerConfigurationType,
                                           ControlMessageType,
                                           DataMessageType,
                                           SessionMessageType,
                                           ControlType,
                                           NotificationType,
                                           ConnectionManagerType,
                                           UserDataType> O_WRITER_T;
  typedef Stream_StreamModule_T<ACE_SYNCH_USE,                                      // task synch type
                                TimePolicyType,                                     // time policy
                                typename SessionMessageType::DATA_T::DATA_T,        // session data type
                                enum Stream_SessionMessageType,                     // session event type
                                struct Stream_ModuleConfiguration,                  // module configuration type
                                HandlerConfigurationType,                           // module handler configuration type
                                libacestream_default_net_output_module_name_string, // name
                                INOTIFY_T,                                          // stream notification interface type
                                O_READER_T,                                         // reader type
                                O_WRITER_T> OUTPUT_MODULE_T;                        // writer type

  ACE_HANDLE  handle_; // socket-

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
                                        HandlerConfigurationType,
                                        SessionManagerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        AddressType,
                                        ConnectionManagerType,
                                        UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T (const Stream_Module_Net_IO_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T& operator= (const Stream_Module_Net_IO_Stream_T&))

  // override Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&);

  // override (part of) Stream_ILinkCB
  virtual void onLink ();
};

// include template definition
#include "stream_net_io_stream.inl"

#endif
