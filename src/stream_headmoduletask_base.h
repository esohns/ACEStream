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
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

#include "stream_imodule.h"
#include "stream_istreamcontrol.h"
#include "stream_messagequeue.h"
#include "stream_session_message_base.h"
#include "stream_statemachine_control.h"
#include "stream_task_base.h"

// forward declaration(s)
class ACE_Message_Block;
class Stream_IAllocator;

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename StreamStateType,
          ///////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
class Stream_HeadModuleTaskBase_T
 : public Stream_StateMachine_Control
 , public Stream_TaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType>
 , public Stream_IModuleHandler_T<ConfigurationType>
 , public Stream_IStreamControl_T<Stream_StateMachine_ControlState,
                                  StreamStateType>
 , public Common_IInitialize_T<StreamStateType>
{
 public:
  virtual ~Stream_HeadModuleTaskBase_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  // *IMPORTANT NOTE*: (if any,) the argument is assumed to be of type
  //                   SessionDataType* !
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);
  virtual int svc (void);

  // implement Stream_IModuleHandler_T
  virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&);

  // implement Stream_IStreamControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;
  virtual void pause ();
  virtual void rewind ();
  virtual const Stream_StateMachine_ControlState& status () const;
  // *NOTE*: waits for any worker threads to join
  virtual void waitForCompletion ();
  // *NOTE*: this is just a stub
  virtual const StreamStateType& state () const;

  // implement Common_IInitialize_T
  virtual bool initialize (const StreamStateType&);

 protected:
  Stream_HeadModuleTaskBase_T (bool = false, // active object ?
                               bool = false, // auto-start ?
                               bool = true); // run svc() routine on start ? (passive only)

  // *NOTE*: override: handle MB_STOP control messages to trigger shutdown of
  //         the worker thread
  virtual void handleControlMessage (ACE_Message_Block*, // control message
                                     bool&,              // return value: stop processing ?
                                     bool&);             // return value: pass message downstream ?

  // *TODO*: clean this API
  // convenience methods to send (session-specific) notifications downstream
  // *WARNING*: - handle with care -
  bool putSessionMessage (Stream_SessionMessageType, // session message type
                          SessionDataType*,          // data
                          bool = false) const;       // delete session data ?
  // *NOTE*: message assumes responsibility for the payload data container
  //         --> "fire-and-forget" SessionDataContainerType
  bool putSessionMessage (Stream_SessionMessageType,        // session message type
                          SessionDataContainerType*&,       // data container
                          Stream_IAllocator* = NULL) const; // allocator (NULL ? --> use "new")

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onChange (Stream_StateType_t); // new state

  // *NOTE*: functionally, this does the same as stop(), with the
  //         difference that stop() will wait for any worker(s)
  //         --> i.e. stop() MUST NOT be called within a worker thread itself
  //             so it calls this to signal an end
  virtual void finished ();

  ConfigurationType   configuration_;
//  bool                isActive_;
  SessionDataType*    sessionData_;
  StreamStateType*    state_;

  ACE_SYNCH_MUTEX     lock_;
  Stream_MessageQueue queue_;

 private:
  typedef Stream_StateMachine_Control inherited;
  typedef Stream_TaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType> inherited2;

  // convenient types
  typedef Stream_HeadModuleTaskBase_T<TaskSynchType,
                                      TimePolicyType,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T (const Stream_HeadModuleTaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T& operator= (const Stream_HeadModuleTaskBase_T&))

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
                                  bool&);                // return value: pass message downstream ?

  // allow blocking wait in waitForCompletion()
  bool                autoStart_;
  ACE_SYNCH_CONDITION condition_;
  bool                runSvcRoutineOnStart_;
};

// include template implementation
#include "stream_headmoduletask_base.inl"

#endif
