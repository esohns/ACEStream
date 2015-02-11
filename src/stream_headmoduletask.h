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

#ifndef STREAM_HEADMODULETASK_H
#define STREAM_HEADMODULETASK_H

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"
#include "ace/Synch_Traits.h"

#include "common.h"

#include "stream_task.h"
#include "stream_istreamcontrol.h"
#include "stream_statemachine_control.h"
#include "stream_session_message.h"
#include "stream_messagequeue.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_MessageBase;
class Stream_SessionConfiguration;
class Stream_IAllocator;

class Stream_HeadModuleTask
 : public Stream_Task_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t>,
   public Stream_IStreamControl,
   public Stream_StateMachine_Control
{
 public:
  virtual ~Stream_HeadModuleTask ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);
  virtual int svc (void);

  // implement (part of) Stream_ITask
  // *NOTE*: this is just default NOP implementation...
//   virtual void handleDataMessage(Stream_MessageBase*&, // data message handle
//                                  bool&);               // return value: pass message downstream ?

  // implement Stream_IStreamControl
  virtual void start ();
  virtual void stop ();
  virtual void pause ();
  virtual void rewind ();
  virtual void waitForCompletion ();
  virtual const bool isRunning ();

 protected:
  Stream_HeadModuleTask (bool); // auto-start ?

  // override: handle MB_STOP control messages to trigger shutdown of the stream...
  virtual void handleControlMessage (ACE_Message_Block*, // control message
                                     bool&,              // return value: stop processing ?
                                     bool&);             // return value: pass message downstream ?

  // convenience methods to send (session-specific) notifications downstream
  // *WARNING*: - handle with care -
  bool putSessionMessage (unsigned int,                                     // session ID
                          const Stream_SessionMessage::SessionMessageType&, // session message type
                          Stream_SessionConfiguration*&,                    // data
                          Stream_IAllocator* = NULL) const;                 // allocator (NULL ? --> use "new")
  // *NOTE*: session message assumes lifetime responsibility for data
  // --> method implements a "fire-and-forget" strategy !
  bool putSessionMessage (unsigned int,                                     // session ID
                          const Stream_SessionMessage::SessionMessageType&, // session message type
                          const void* = NULL,                               // user data
                          const ACE_Time_Value& = ACE_Time_Value::zero,     // start of session
                          bool = false) const;                              // user abort ?

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onStateChange (const Control_StateType&); // new state

  // *TODO*: try to remove this !
  // functionally, this does the same as stop(), with the
  // difference, that stop() will blocking wait for our worker
  // thread to die...
  // --> i.e. stop() MUST NOT be called from WITHIN the worker thread !
  // but what if we need to do exactly that ?
  // That's right --> use this !
  virtual void finished ();

  // *WARNING*: children need to set this during initialization !
  Stream_IAllocator*                        allocator_;
  unsigned int                              sessionID_;

 private:
  typedef Stream_Task_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t> inherited;
  typedef Stream_StateMachine_Control inherited2;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTask ());
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTask (const Stream_HeadModuleTask&));
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTask& operator= (const Stream_HeadModuleTask&));

  // allow blocking wait in waitForCompletion()
  ACE_Condition<ACE_Recursive_Thread_Mutex> condition_;
  ACE_Recursive_Thread_Mutex                lock_;
  bool                                      isFinished_;
  Stream_MessageQueue                       queue_;
  bool                                      autoStart_;
  const void*                               userData_;
};

#endif
