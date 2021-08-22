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

#ifndef STREAM_TASK_BASE_ASYNCH_H
#define STREAM_TASK_BASE_ASYNCH_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_messagequeue.h"
#include "stream_task_base.h"

// forward declarations
class ACE_Message_Block;
class ACE_Time_Value;

// *IMPORTANT NOTE*: the message queue needs to be synchronized so that shutdown
//                   can be asynchronous
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionControlType,
          typename SessionEventType,
          ////////////////////////////////
          typename UserDataType>
class Stream_TaskBaseAsynch_T
 : public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionControlType,
                            SessionEventType,
                            UserDataType>
{
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionControlType,
                            SessionEventType,
                            UserDataType> inherited;

 public:
  virtual ~Stream_TaskBaseAsynch_T ();

  // override (part of) ACE_Task_Base
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);

  virtual int put (ACE_Message_Block*,
                   ACE_Time_Value* = NULL);

  // implement Common_IAsynchTask
  // *NOTE*: tests for MB_STOP anywhere in the queue. Note that this does not
  //         block, or dequeue any message
  // *NOTE*: ACE_Message_Queue_Iterator does its own locking, i.e. access
  //         happens in lockstep, which is both inefficient and yields
  //         unpredictable results
  //         --> use Common_MessageQueueIterator_T and lock the queue manually
  virtual bool isShuttingDown () const;
  // enqueue MB_STOP --> stop worker thread(s)
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // high priority ? (i.e. do not wait for queued messages)

  // override (part of) Stream_ITask_T
  inline virtual void waitForIdleState () const { queue_.waitForIdleState (); }

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

 protected:
  // convenient types
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionControlType,
                            SessionEventType,
                            UserDataType> TASK_BASE_T;

  Stream_TaskBaseAsynch_T (typename TASK_BASE_T::ISTREAM_T* = NULL); // stream handle

  typename inherited::MESSAGE_QUEUE_T queue_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseAsynch_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseAsynch_T (const Stream_TaskBaseAsynch_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseAsynch_T& operator= (const Stream_TaskBaseAsynch_T&))

  // helper methods
  // *NOTE*: 'high priority' effectively means that the message is enqueued at
  //         the head end (i.e. will be the next to dequeue), whereas it would
  //         be enqueued at the tail end otherwise
  virtual void control (int,           // message type
                        bool = false); // high-priority ?

  // override (part of) ACE_Task_Base
  virtual int svc (void);
};

// include template definition
#include "stream_task_base_asynch.inl"

#endif
