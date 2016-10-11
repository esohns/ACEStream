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

#include <ace/Global_Macros.h>

#include "common_iinitialize.h"
#include "common_istatistic.h"

#include "stream_ilock.h"
#include "stream_imodule.h"
#include "stream_istreamcontrol.h"
#include "stream_session_message_base.h"
#include "stream_statemachine_control.h"
#include "stream_task_base.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;

template <typename LockType,                 // state machine
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType>
class Stream_HeadModuleTaskBase_T
 : public Stream_StateMachine_Control_T<LockType>
 , public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            StreamNotificationType>
 //, public Stream_IModuleHandler_T<ConfigurationType>
 , public Stream_IStreamControl_T<StreamControlType,
                                  StreamNotificationType,
                                  Stream_StateMachine_ControlState,
                                  StreamStateType>
 , public Stream_ILock_T<ACE_SYNCH_USE>
 , public Common_IInitialize_T<StreamStateType>
 , public Common_IStatistic_T<StatisticContainerType>
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

  // implement (part of) Stream_ITask_T
  virtual void waitForIdleState () const;

//  // implement Stream_IModuleHandler_T
//  virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_IStreamControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;

  virtual void pause ();
  virtual void wait (bool = true,   // wait for any worker thread(s) ?
                     bool = false,  // N/A
                     bool = false); // N/A

  // *NOTE*: just a stub
  virtual std::string name () const;

  virtual void control (StreamControlType, // control type
                        bool = false);     // N/A
  // *WARNING*: currently, the default stream implementation forwards all
  //            notifications to the head module. This implementation generates
  //            session messages for all events except 'abort'
  //            --> make sure there are no session message 'loops'
  virtual void notify (StreamNotificationType, // notification type
                       bool = false);          // N/A
    // *NOTE*: just a stub
  virtual const StreamStateType& state () const;
  virtual Stream_StateMachine_ControlState status () const;

  // implement Stream_ILock_T
  // *IMPORTANT NOTE*: on Windows, 'critical sections' (such as this) are
  //                   recursive, and lock increases the count, so unlock may
  //                   need to be called several times. Note how lock/unlock
  //                   does not keep track of the recursion counter
  //                   --> handle with care !
  virtual bool lock (bool = true); // block ?
  virtual int unlock (bool = false); // unlock ?
  virtual ACE_SYNCH_RECURSIVE_MUTEX& getLock ();
  virtual bool hasLock ();

  // implement Common_IInitialize_T
  virtual bool initialize (const StreamStateType&);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  virtual void report () const;

 protected:
  Stream_HeadModuleTaskBase_T (typename LockType::MUTEX* = NULL, // lock handle (state machine)
                               bool = false,                     // auto-start ?
                               bool = true);                     // generate session messages ?

  // convenient types
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            StreamNotificationType> TASK_BASE_T;
  typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> COLLECTION_HANDLER_T;

//  using TASK_BASE_T::shutdown;

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // (requested) size
  // convenience methods to send (session-specific) notifications downstream
  bool putControlMessage (StreamControlType,                // control type
                          Stream_IAllocator* = NULL) const; // allocator (NULL ? --> use "new")
  // *NOTE*: message assumes responsibility for the payload data container
  //         --> "fire-and-forget" SessionDataContainerType
  bool putSessionMessage (Stream_SessionMessageType,        // session message type
                          SessionDataContainerType*&,       // session data container
                          Stream_IAllocator* = NULL) const; // allocator (NULL ? --> use "new")
  bool putStatisticMessage (const StatisticContainerType&) const; // statistic information

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onChange (Stream_StateType_t); // new state

  // *NOTE*: applies only to '!active' modules that do not run svc() on start
  //         (see below). This enforces that all messages pass through the
  //         stream strictly sequentially, which may be necessary for
  //         asynchronously-supplied (i.e. 'concurrent') scenarios with
  //         non-reentrant module (i.e. most modules that maintain some kind of
  //         internal state, such as push-parsers) configurations, or streams
  //         that react to asynchronous events (such as connection resets, user
  //         aborts, signals, etc)
  // *TODO*: find a way to by-pass this additional overhead if 'true'
  bool                 concurrent_;
//  ConfigurationType*   configuration_;
  bool                 isInitialized_;

  // *NOTE*: default behaviour for '!active' modules (i.e. modules that have no
  //         dedicated worker thread), which 'borrows' the thread calling
  //         start() to do the processing. [Note that in this case, stream
  //         processing is already finished once the thread returns from
  //         start(), i.e. there is no point in calling waitForCompletion().]
  //         However, this behaviour may not be intended for passive modules
  //         that don't do their processing via start(), but are 'supplied'
  //         externally (e.g. network source modules)
  bool                 runSvcOnStart_;
  bool                 sessionEndProcessed_;
  StreamStateType*     streamState_;

  // timer
  COLLECTION_HANDLER_T statisticCollectionHandler_;
  long                 timerID_;

 private:
  typedef Stream_StateMachine_Control_T<LockType> inherited;
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            StreamNotificationType> inherited2;

  // convenient types
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T (const Stream_HeadModuleTaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T& operator= (const Stream_HeadModuleTaskBase_T&))

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement (part of) Stream_IStreamControl_T
  virtual const Stream_Module_t* find (const std::string&) const; // module name
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?
  virtual void flush (bool = true,   // flush inbound data ?
                      bool = false,  // flush session messages ?
                      bool = false); // flush upstream (if any) ?
  virtual void rewind ();
  virtual void upStream (Stream_Base_t*);
  virtual Stream_Base_t* upStream () const;

  bool                 active_;
  // *NOTE*: starts a worker thread in open (), i.e. when push()ed onto a stream
  bool                 autoStart_;
  bool                 generateSessionMessages_;
  bool                 sessionEndSent_;
};

// include template definition
#include "stream_headmoduletask_base.inl"

#endif
