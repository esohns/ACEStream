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

#ifndef STREAM_HEADMODULETASK_BASE_T_H
#define STREAM_HEADMODULETASK_BASE_T_H

#include <string>

#include <ace/Global_Macros.h>

#include "common_iinitialize.h"
#include "common_istatistic.h"

#include "stream_ilink.h"
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

template <ACE_SYNCH_DECL, // state machine-/task
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_HeadModuleTaskBase_T
 : public Stream_StateMachine_Control_T<ACE_SYNCH_USE>
 , public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            SessionControlType,
                            SessionEventType,
                            UserDataType>
 , public Stream_IStreamControl_T<SessionControlType,
                                  SessionEventType,
                                  enum Stream_StateMachine_ControlState,
                                  StreamStateType>
 , public Stream_ILink
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
  inline virtual void waitForIdleState () const { inherited2::queue_.waitForIdleState (); };

//  // implement Stream_IModuleHandler_T
//  virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_IStreamControl_T
  virtual void start ();
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true); // locked access ?
  virtual bool isRunning () const;

  inline virtual void pause () { inherited::change (STREAM_STATE_PAUSED); };
  virtual void wait (bool = true,   // wait for any worker thread(s) ?
                     bool = false,  // N/A
                     bool = false); // N/A

  inline virtual std::string name () const { std::string result = ACE_TEXT_ALWAYS_CHAR (inherited2::name ()); return result; };

  virtual void control (SessionControlType, // control type
                        bool = false);     // N/A
  // *WARNING*: currently, the default stream implementation forwards all
  //            notifications to the head module. This implementation generates
  //            session messages for all events except 'abort'
  //            --> make sure there are no session message 'loops'
  virtual void notify (SessionEventType, // notification type
                       bool = false);          // N/A
  inline virtual const StreamStateType& state () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (StreamStateType ()); ACE_NOTREACHED (return StreamStateType ();) };
  inline virtual Stream_StateMachine_ControlState status () const { Stream_StateMachine_ControlState result = inherited::current (); return result; };

  // implement Stream_ILink
  virtual void link ();
  virtual void unlink ();

  // implement Stream_ILock_T
  // *WARNING*: on Windows, 'critical sections' (such as this) are 'recursive',
  //            so lock() increases the count, and unlock needs to be called
  //            an equal number of times. Note how lock/unlock does not keep
  //            track of the recursion counter
  virtual bool lock (bool = true); // block ?
  virtual int unlock (bool = false); // unlock ?
  inline virtual ACE_SYNCH_RECURSIVE_MUTEX& getLock () { ACE_ASSERT (false); ACE_SYNCH_RECURSIVE_MUTEX dummy; ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) };
  // *TODO*: this isn't nearly accurate enough
  inline virtual bool hasLock () { return concurrent_; };

  // implement Common_IInitialize_T
  virtual bool initialize (const StreamStateType&);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  inline virtual bool collect (StatisticContainerType&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 protected:
  Stream_HeadModuleTaskBase_T (ACE_SYNCH_MUTEX_T* = NULL,                                                // lock handle (state machine)
                               bool = false,                                                             // auto-start ? (active mode only)
                               enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency mode
                               bool = true);                                                             // generate session messages ?

  // convenient types
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            SessionControlType,
                            SessionEventType,
                            UserDataType> TASK_BASE_T;
  typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> COLLECTION_HANDLER_T;

//  using TASK_BASE_T::shutdown;

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&); // statistic information

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onChange (Stream_StateType_t); // new state

  // *NOTE*: valid operating modes:
  //         active    : dedicated worker thread(s) running svc()
  //         concurrent: in-line processing (i.e. concurrent put(), no workers)
  //                     [Data is supplied externally, e.g. event dispatch]
  //         passive   : in-line (invokes svc() on start())
  //                     [Note that in this case, stream processing is already
  //                     finished once the thread returns from start(), i.e.
  //                     there is no point in calling waitForCompletion().]
  enum Stream_HeadModuleConcurrency concurrency_;
  // *NOTE*: applies to 'concurrent' modules (see below). This enforces that all
  //         messages pass through the stream strictly sequentially, which may
  //         be necessary for asynchronously-supplied (i.e. 'concurrent')
  //         scenarios with non-reentrant module (i.e. most modules that
  //         maintain some kind of internal state, such as push-parsers) states,
  //         or streams that react to asynchronous events (such as connection
  //         resets, user aborts, signals, etc)
  // *WARNING*: when disabled, this 'locks down' the pipeline head module.
  //            Threads will then hold the 'stream lock' during message
  //            processing to support (down)stream synchronization. This really
  //            only makes sense in fully synchronous layouts, or 'concurrent'
  //            scenarios with non-reentrant modules
  //            --> disable only if you know what you are doing
  // *TODO*: find a way to by-pass the additional overhead when 'true'
  bool                              concurrent_;
  bool                              isInitialized_;

  bool                              sessionEndProcessed_;
  bool                              sessionEndSent_;
  StreamStateType*                  streamState_;

  // timer
  COLLECTION_HANDLER_T              statisticCollectionHandler_;
  long                              timerID_;

 private:
  typedef Stream_StateMachine_Control_T<ACE_SYNCH_USE> inherited;
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            SessionControlType,
                            SessionEventType,
                            UserDataType> inherited2;

  // convenient types
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      SessionControlType,
                                      SessionEventType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T (const Stream_HeadModuleTaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T& operator= (const Stream_HeadModuleTaskBase_T&))

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement (part of) Stream_IStreamControl_T
  inline virtual const Stream_Module_t* find (const std::string&) const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) };
  inline virtual bool load (Stream_ModuleList_t&, bool&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  inline virtual void flush (bool = true,   // flush inbound data ?
                             bool = false,  // flush session messages ?
                             bool = false) { inherited2::putControlMessage (STREAM_CONTROL_FLUSH); };
  inline virtual void rewind () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  inline virtual void upStream (Stream_Base_t*) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  inline virtual Stream_Base_t* upStream () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) };

  // *NOTE*: starts a worker thread in open (), i.e. when push()ed onto a stream
  bool                              autoStart_;
  bool                              generateSessionMessages_;
  ACE_SYNCH_MUTEX*                  sessionDataLock_;
};

// include template definition
#include "stream_headmoduletask_base.inl"

#endif
