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

#include <string>

#include "ace/Global_Macros.h"

#include "common_iinitialize.h"

#include "stream_imodule.h"
#include "stream_istreamcontrol.h"
#include "stream_session_message_base.h"
#include "stream_statemachine_control.h"
#include "stream_task_base.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;

template <typename LockType,
          ///////////////////////////////
          typename TaskSynchType,
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
 : public Stream_StateMachine_Control_T<LockType>
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
  virtual int open (void* = NULL); // session data handle
  virtual int close (u_long = 0);
  virtual int module_closed (void);
  virtual int svc (void);

  // implement Stream_IModuleHandler_T
  virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_IStreamControl_T
  virtual void control (Stream_ControlType, // control type
                        bool = false);      // N/A
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;

  virtual void pause ();
  virtual Stream_StateMachine_ControlState status () const;
  // *NOTE*: waits for any worker threads to join
  virtual void waitForCompletion (bool = true,   // wait for any worker
                                                 // thread(s) ?
                                  bool = false); // N/A
  virtual void waitForIdleState () const;

  virtual std::string name () const;
  // *NOTE*: this is just a stub
  virtual const StreamStateType& state () const;

  // implement Common_IInitialize_T
  virtual bool initialize (const StreamStateType&);

 protected:
  Stream_HeadModuleTaskBase_T (LockType*,    // lock handle (state machine)
                               //////////
                               bool = false, // active object ?
                               bool = false, // auto-start ?
                               bool = true,  // run svc() routine on start ? (passive only)
                               bool = true); // generate session messages ?

  // *TODO*: clean this API
  // convenience methods to send (session-specific) notifications downstream
  // *WARNING*: - handle with care -
//  bool putSessionMessage (Stream_SessionMessageType, // session message type
//                          SessionDataType*,          // session data
//                          bool = false) const;       // delete session data ?
  // *NOTE*: message assumes responsibility for the payload data container
  //         --> "fire-and-forget" SessionDataContainerType
  bool putSessionMessage (Stream_SessionMessageType,        // session message type
                          SessionDataContainerType&,        // session data container
                          Stream_IAllocator* = NULL) const; // allocator (NULL ? --> use "new")

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onChange (Stream_StateType_t); // new state

  ConfigurationType*        configuration_;
  //bool              isActive_;
  SessionDataContainerType* sessionData_;
  StreamStateType*          streamState_;

 private:
  typedef Stream_StateMachine_Control_T<LockType> inherited;
  typedef Stream_TaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType> inherited2;

  // convenient types
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      TaskSynchType,
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
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement (part of) Stream_IStreamControl_T
  //virtual void initialize ();
  //virtual void flush (bool = false); // N/A
  virtual void rewind ();
  virtual void upStream (Stream_Base_t*);
  virtual Stream_Base_t* upStream () const;

  bool                      active_;
  bool                      autoStart_;
  bool                      generateSessionMessages_;
  bool                      runSvcRoutineOnStart_;
  bool                      sessionEndSent_;
  ACE_Thread_ID             threadID_;
};

// include template implementation
#include "stream_headmoduletask_base.inl"

#endif
