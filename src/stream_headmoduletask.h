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

#include "stream_common.h"
#include "stream_istreamcontrol.h"
#include "stream_statemachine_control.h"
#include "stream_session_message.h"
#include "stream_task.h"
#include "stream_messagequeue.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_MessageBase;
class Stream_IAllocator;

class Stream_HeadModuleTask
 : public Stream_Task_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t>
 , public Stream_IStreamControl
 , public Stream_StateMachine_Control
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
  virtual bool isRunning ();

 protected:
  Stream_HeadModuleTask (bool); // auto-start ?

  // override: handle MB_STOP control messages to trigger shutdown of the stream...
  virtual void handleControlMessage (ACE_Message_Block*, // control message
                                     bool&,              // return value: stop processing ?
                                     bool&);             // return value: pass message downstream ?

  // convenience method: send (session-specific) notifications downstream
  bool putSessionMessage (Stream_SessionMessageType_t,      // session message type
                          Stream_SessionData_t*,            // session data
                          Stream_IAllocator* = NULL) const; // allocator (NULL ? --> use "new")
  // *NOTE*: session message assumes lifetime responsibility for data
  // --> method implements a "fire-and-forget" strategy
  bool putSessionMessage (Stream_SessionMessageType_t,                  // session message type
                          Stream_SessionData_t*,                        // session data
                          const ACE_Time_Value& = ACE_Time_Value::zero, // start of session
                          bool = false) const;                          // user abort ?

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onStateChange (const Stream_StateType_t&); // new state

  // *TODO*: remove this ASAP
  // *NOTE*: functionally, this does the same as stop(), with the
  // difference, that stop() will blocking wait for the worker
  // thread to finish...
  // --> i.e. stop() MUST NOT be called from WITHIN the worker thread
  // --> workers call this function to signal task completion
  virtual void finished ();

  // *WARNING*: children need to set this during initialization !
  Stream_IAllocator*                        allocator_;

 private:
  typedef Stream_Task_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t> inherited;
  typedef Stream_StateMachine_Control inherited2;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTask ());
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTask (const Stream_HeadModuleTask&));
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTask& operator= (const Stream_HeadModuleTask&));

  // allow blocking wait in waitForCompletion()
  bool                                      autoStart_;
  ACE_Condition<ACE_Recursive_Thread_Mutex> condition_;
  bool                                      isFinished_;
  ACE_Recursive_Thread_Mutex                lock_;
  Stream_MessageQueue                       queue_;
  Stream_SessionData_t*                     sessionData_;
  Stream_State_t*                           state_;
};

#endif
