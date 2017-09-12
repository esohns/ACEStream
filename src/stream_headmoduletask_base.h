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

#include "ace/Global_Macros.h"

#include "common_iget.h"
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
          typename StatisticHandlerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_HeadModuleTaskBase_T
 : public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            SessionControlType,
                            SessionEventType,
                            UserDataType>
 , public Stream_StateMachine_Control_T<ACE_SYNCH_USE>
 , public Stream_IStreamControl_T<SessionControlType,
                                  SessionEventType,
                                  enum Stream_StateMachine_ControlState,
                                  StreamStateType>
 , public Stream_ILinkCB
 , public Stream_ILock_T<ACE_SYNCH_USE>
 , public Common_ISetP_T<StreamStateType>
 , public Common_IStatistic_T<StatisticContainerType>
{
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            Stream_SessionId_t,
                            SessionControlType,
                            SessionEventType,
                            UserDataType> inherited;
  typedef Stream_StateMachine_Control_T<ACE_SYNCH_USE> inherited2;

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
  inline virtual void waitForIdleState () const { inherited::queue_.waitForIdleState (); };

  // implement Stream_IModuleHandler_T
//  virtual const ConfigurationType& get () const;
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_IStreamControl_T
  inline virtual void start () { inherited2::change (STREAM_STATE_RUNNING); };
  virtual void stop (bool = true,  // wait for completion ?
                     bool = true,  // N/A
                     bool = true); // N/A
  virtual bool isRunning () const;

  inline virtual void pause () { inherited2::change (STREAM_STATE_PAUSED); };
  virtual void wait (bool = true,   // wait for any worker thread(s) ?
                     bool = false,  // N/A
                     bool = false); // N/A

  //inline virtual std::string name () const { std::string result = ACE_TEXT_ALWAYS_CHAR (inherited2::name ()); return result; };

  virtual void control (SessionControlType, // control type
                        bool = false);     // N/A
  // *WARNING*: currently, the default stream implementation forwards all
  //            notifications to the head module. This implementation generates
  //            session messages for all events except 'abort'
  //            --> make sure there are no session message 'loops'
  virtual void notify (SessionEventType, // notification type
                       bool = false);    // N/A
  inline virtual const StreamStateType& state () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (StreamStateType ()); ACE_NOTREACHED (return StreamStateType ();) };
  inline virtual enum Stream_StateMachine_ControlState status () const { enum Stream_StateMachine_ControlState result = inherited2::current (); return result; };

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

  // implement Common_ISet_T
  inline virtual void setP (StreamStateType* streamState_in) { ACE_ASSERT (!streamState_); streamState_ = streamState_in; };

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  inline virtual bool collect (StatisticContainerType&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 protected:
  // convenient types
  typedef Stream_StateMachine_Control_T<ACE_SYNCH_USE> STATE_MACHINE_T;
  //typedef Stream_StatisticHandler_Reactor_T<StatisticContainerType> COLLECTION_HANDLER_T;
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
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_HeadModuleTaskBase_T (ISTREAM_T*,                                                               // stream handle
#else
  Stream_HeadModuleTaskBase_T (typename inherited::ISTREAM_T*,                                           // stream handle
#endif
                               bool = false,                                                             // auto-start ? (active mode only)
                               enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency mode
                               bool = true);                                                             // generate session messages ?

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&); // statistic information

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual void onChange (Stream_StateType_t); // new state

  // disambiguate Common_TaskBase_T and Common_StateMachine_Base_T
  using inherited2::finished;
  // disambiguate Stream_TaskBase_T and Common_StateMachine_Base_T
  using inherited::isInitialized_;

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
  //         scenarios with non-reentrant module states (i.e. most modules that
  //         maintain some kind of internal state, such as push-parsers),
  //         or streams that react to asynchronous events (such as connection
  //         resets, user aborts, signals, etc)
  // *WARNING*: when disabled, this 'locks down' the pipeline head module.
  //            Threads will then hold the 'stream lock' during message
  //            processing to support (down)stream synchronization. This really
  //            only makes sense in fully synchronous layouts, or 'concurrent'
  //            scenarios with non-reentrant modules
  //            --> disable only if you know what you are doing
  // *TODO*: find a way to by-pass this processing overhead when 'true'
  bool                              concurrent_;

  bool                              sessionEndProcessed_;
  bool                              sessionEndSent_;
  ACE_SYNCH_MUTEX_T                 stateMachineLock_;
  StreamStateType*                  streamState_;

  // timer
  StatisticHandlerType              statisticCollectionHandler_;
  long                              timerID_;

 private:
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
                                      StatisticHandlerType,
                                      UserDataType> OWN_TYPE_T;
  typedef Stream_IStreamControl_T<SessionControlType,
                                  SessionEventType,
                                  enum Stream_StateMachine_ControlState,
                                  StreamStateType> ISTREAM_CONTROL_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T (const Stream_HeadModuleTaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T& operator= (const Stream_HeadModuleTaskBase_T&))

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_ILinkCB
  virtual void onLink ();
  virtual void onUnlink ();

  // implement (part of) Stream_IStreamControl_T
  inline virtual Stream_SessionId_t id () const { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) };
  inline virtual void finished (bool = true) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };
  inline virtual void flush (bool = true, bool = false, bool = false) { inherited::putControlMessage (STREAM_CONTROL_FLUSH); };
  inline virtual void rewind () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // *NOTE*: starts a worker thread in open (), i.e. when push()ed onto a stream
  bool                              autoStart_;
  bool                              generateSessionMessages_;
};

// include template definition
#include "stream_headmoduletask_base.inl"

#endif
