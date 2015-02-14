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

#ifndef STREAM_HEADMODULETASK_BASE_H
#define STREAM_HEADMODULETASK_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"
#include "ace/Synch.h"

#include "stream_task_base.h"
#include "stream_istreamcontrol.h"
#include "stream_statemachine_control.h"
#include "stream_session_message.h"
#include "stream_session_message_base.h"
#include "stream_messagequeue.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_MessageBase;
class Stream_IAllocator;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionDataType,
          typename SessionMessageType,
          typename ProtocolMessageType>
class Stream_HeadModuleTaskBase_T
 : public Stream_TaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType>
 , public Stream_IStreamControl
 , public Stream_StateMachine_Control
{
 public:
  virtual ~Stream_HeadModuleTaskBase_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);
  virtual int svc (void);

  // implement (part of) Stream_ITaskBase
  // *NOTE*: these are just default NOP implementations...
  // *WARNING*: need to implement this in the base class to shut up the linker
  // about missing template instantiations...
//   virtual void handleDataMessage (Stream_MessageBase*&, // data message handle
//                                   bool&);               // return value: pass message downstream ?

  // implement Stream_IStreamControl
  virtual void start ();
  virtual void stop (bool = true); // locked access ?
  virtual void pause ();
  virtual void rewind ();
  // *NOTE*: for the time being, this simply waits for any worker threads to join
  virtual void waitForCompletion ();
  virtual bool isRunning () const;

 protected:
  Stream_HeadModuleTaskBase_T (bool = false,  // active object ?
                               bool = false); // auto-start ?

  // override: handle MB_STOP control messages to trigger shutdown
  // of the worker thread...
  virtual void handleControlMessage (ACE_Message_Block*, // control message
                                     bool&,              // return value: stop processing ?
                                     bool&);             // return value: pass message downstream ?

  // convenience methods to send (session-specific) notifications downstream
  // *WARNING*: - handle with care -
  bool putSessionMessage (unsigned int,                                 // session ID
                          Stream_SessionMessageType_t,                  // session message type
                          DataType*,                                    // data
                          const ACE_Time_Value& = ACE_Time_Value::zero, // start of session
                          bool = false) const;                          // user abort ?
  // *NOTE*: session message assumes lifetime responsibility for data
  // --> method implements a "fire-and-forget" strategy !
  bool putSessionMessage (unsigned int,                     // session ID
                          Stream_SessionMessageType_t,      // session message type
                          SessionDataType*&,                // data
                          Stream_IAllocator* = NULL) const; // allocator (NULL ? --> use "new")

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onStateChange (Control_StateType); // new state

  // *NOTE*: functionally, this does the same as stop(), with the
  // difference that stop() will wait for any worker(s)
  // --> i.e. stop() MUST NOT be called within a worker thread itself
  // so it calls this to signal an end
  virtual void finished ();

  // *IMPORTANT NOTE*: children SHOULD set these during initialization !
  Stream_IAllocator*              allocator_;
  bool                            isActive_;
  DataType*                       userData_;

 private:
  typedef Stream_TaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType> inherited;
  typedef Stream_StateMachine_Control inherited2;
  typedef Stream_HeadModuleTaskBase_T<TaskSynchType,
                                      TimePolicyType,
                                      DataType,
                                      SessionDataType,
                                      SessionMessageType,
                                      ProtocolMessageType> own_type;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T (const Stream_HeadModuleTaskBase_T&));
  // *TODO*: apparently, ACE_UNIMPLEMENTED_FUNC gets confused by template arguments...
//   ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T<DataType,SessionConfigType,SessionMessageType>& operator=(const Stream_HeadModuleTaskBase_T<DataType,SessionConfigType,SessionMessageType>&));

  // allow blocking wait in waitForCompletion()
 // ACE_Recursive_Thread_Mutex                lock_;
  //ACE_Condition<ACE_Recursive_Thread_Mutex> condition_;
  bool                            autoStart_;
  ACE_Condition<ACE_Thread_Mutex> condition_;
  unsigned int                    currentNumThreads_;
  ACE_Thread_Mutex                lock_;
  Stream_MessageQueue             queue_;
};

// include template implementation
#include "stream_headmoduletask_base.inl"

#endif
