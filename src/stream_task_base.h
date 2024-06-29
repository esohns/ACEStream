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

#ifndef STREAM_TASK_BASE_H
#define STREAM_TASK_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_message_queue_iterator.h"

#include "common_task_base.h"

#include "stream_imodule.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_itask.h"
#include "stream_messagequeue.h"

// forward declarations
class ACE_Message_Block;
template <ACE_SYNCH_DECL,
          class TIME_POLICY>
class ACE_Module;
class ACE_Time_Value;
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename StreamControlType,
          typename SessionEventType,
          ////////////////////////////////
          typename UserDataType>
class Stream_TaskBase_T
 : public Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ACE_Message_Block,
                            ACE_Message_Queue<ACE_SYNCH_USE,
                                              TimePolicyType>,
                            ACE_Task<ACE_SYNCH_USE,
                                     TimePolicyType> >
 , public Stream_ITask
 , public Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType>
 , public Stream_IModuleHandler_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType>
 , public Common_IGetP_T<Stream_IStream_T<ACE_SYNCH_USE,
                                          TimePolicyType> >
 , public Common_IGetR_T<ConfigurationType>
{
  typedef Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ACE_Message_Block,
                            ACE_Message_Queue<ACE_SYNCH_USE,
                                              TimePolicyType>,
                            ACE_Task<ACE_SYNCH_USE,
                                     TimePolicyType> > inherited;

 public:
  // convenient types
  typedef Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ACE_Message_Block,
                            ACE_Message_Queue<ACE_SYNCH_USE,
                                              TimePolicyType>,
                            ACE_Task<ACE_SYNCH_USE,
                                     TimePolicyType> > TASK_BASE_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           TimePolicyType> ISTREAM_T;

  virtual ~Stream_TaskBase_T ();

  // implement (part of) Stream_ITask_T
  // *NOTE*: some default (essentially NOP) definitions
  inline virtual void handleControlMessage (ControlMessageType&) {}
  inline virtual void handleDataMessage (DataMessageType*&, bool&) {}
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass this message downstream ?
  inline virtual void handleUserMessage (ACE_Message_Block*&, bool&) {}
  virtual void handleProcessingError (const ACE_Message_Block* const); // message handle

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);
  inline virtual bool postClone (ACE_Module<ACE_SYNCH_USE, TimePolicyType>*, bool = false) { return true; }

  // implement Common_IGet_T
  virtual const ISTREAM_T* const getP () const;
  inline virtual const ConfigurationType& getR () const { ACE_ASSERT (configuration_);  return *configuration_; }

 protected:
  // convenient types
  typedef ConfigurationType CONFIGURATION_T;
  typedef DataMessageType DATA_MESSAGE_T;
  typedef SessionMessageType SESSION_MESSAGE_T;
  typedef Common_IGetR_T<STREAM_T> IGET_T;
  typedef Stream_MessageQueue_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionMessageType> MESSAGE_QUEUE_T;
  typedef Common_MessageQueueIterator_T<ACE_SYNCH_USE,
                                        TimePolicyType> MESSAGE_QUEUE_ITERATOR_T;

  Stream_TaskBase_T (ISTREAM_T* = NULL,        // stream handle
                     MESSAGE_QUEUE_T* = NULL); // queue handle

  // helper methods
  DataMessageType* allocateMessage (size_t); // (requested) size

  // implement Stream_ITask
  virtual void handleMessage (ACE_Message_Block*, // message handle
                              bool&);             // return value: stop processing ?
  inline virtual bool isAggregator () { return aggregate_; }

  // convenience methods to send (session-specific) notifications downstream
  // *NOTE*: these invoke put(), so the messages are processed by 'this' module
  //         as well
  bool putControlMessage (Stream_SessionId_t, // session id
                          StreamControlType,  // control type
                          bool = false);      // send upstream ? : downstream
  // *NOTE*: "fire-and-forget" the second argument
  bool putSessionMessage (SessionEventType,                      // session message type
                          typename SessionMessageType::DATA_T*&, // session data container
                          UserDataType*,                         // user data handle
                          bool);                                 // expedited ?

  virtual void notify (SessionEventType); // session event

  // helper methods
  // *NOTE*: 'high priority' effectively means that the message is enqueued at
  //         the head end (i.e. will be the next to dequeue), whereas it would
  //         be enqueued at the tail end otherwise
  // *WARNING*: this definition put()s messages, potentially ignoring the
  //            'high-priority' argument
  virtual void control (int,           // message type
                        bool = false); // high-priority ?

  // *TODO*: make this 'private' and use 'friend' access
  bool                                 aggregate_; // support multiple initializations ?
  Stream_IAllocator*                   allocator_;
  // *TODO*: remove ASAP
  ConfigurationType*                   configuration_;
  bool                                 isInitialized_;
  unsigned int                         linked_;

  typename SessionMessageType::DATA_T* sessionData_;

 private:
  // convenient types
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            StreamControlType,
                            SessionEventType,
                            UserDataType> OWN_TYPE_T;
  typedef Stream_ISessionNotify_T<typename SessionMessageType::DATA_T::DATA_T,
                                  SessionEventType> INOTIFY_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T (const Stream_TaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T& operator= (const Stream_TaskBase_T&))

  // override/hide ACE_Task_Base members
  inline virtual int put (ACE_Message_Block* messageBlock_in, ACE_Time_Value* timeValue_in) { ACE_ASSERT (false); return inherited::put_next (messageBlock_in, timeValue_in); }

  bool                                 freeSessionData_;
  // *NOTE*: these apply to 'downstream', iff linked, only
  // *TODO*: move all of this to Stream_HeadModuleTaskBase_T; it's easier to do
  //         the switch in this class though
  typename SessionMessageType::DATA_T* sessionData_2; // backup 'downstream' session data
  ACE_SYNCH_MUTEX*                     sessionDataLock_; // backup 'upstream' lock
};

// include template definition
#include "stream_task_base.inl"

#endif
