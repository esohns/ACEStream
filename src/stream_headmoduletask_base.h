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
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_istatistic.h"
#include "common_statistic_handler.h"

#include "stream_ilink.h"
#include "stream_ilock.h"
#include "stream_imodule.h"
#include "stream_istreamcontrol.h"
#include "stream_messagequeue.h"
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType>
class Stream_HeadModuleTaskBase_T
 : public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            StreamControlType,
                            SessionEventType,
                            UserDataType>
 , public Stream_StateMachine_Control_T<ACE_SYNCH_USE>
 , public Stream_IStreamControl_T<StreamControlType,
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
                            StreamControlType,
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

  // implement Common_IAsynchTask
  // *NOTE*: tests for MB_STOP anywhere in the queue. Note that this does not
  //         block, or dequeue any message
  // *NOTE*: ACE_Message_Queue_Iterator does its own locking, i.e. access
  //         happens in lockstep, which is both inefficient and yields
  //         unpredictable results
  //         --> use Common_MessageQueueIterator_T and lock the queue manually
  virtual bool isShuttingDown () const;

  // override (part of) Stream_TaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message handle

  // implement (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_IStreamControl_T
  inline virtual void start () { inherited2::change (STREAM_STATE_SESSION_STARTING); }
  virtual void stop (bool = true,   // wait for completion ?
                     bool = true,   // N/A
                     bool = false); // high priority ?
  virtual bool isRunning () const;
  inline virtual void pause () { inherited2::change (STREAM_STATE_PAUSED); }
  inline virtual void idle (bool waitForever_in = true,
                            bool = true) const { queue_.waitForIdleState (waitForever_in); }
  virtual void wait (bool = true,         // wait for any worker thread(s) ?
                     bool = false,        // N/A
                     bool = false) const; // N/A

  virtual void control (StreamControlType, // control type
                        bool = false);      // N/A
  // *WARNING*: currently, the default stream implementation forwards all
  //            notifications to the head module. This implementation generates
  //            session messages for all events except 'abort'
  //            --> make sure there are no session message 'loops'
  virtual void notify (SessionEventType, // notification type
                       bool = false,     // N/A
                       bool = false);    // expedite ?
  inline virtual const StreamStateType& state () const { ACE_ASSERT (streamState_); return *streamState_; }
  inline virtual enum Stream_StateMachine_ControlState status () const { enum Stream_StateMachine_ControlState result = inherited2::current (); return result; }

  // implement Stream_ILock_T
  // *NOTE*: these just use queue_.lock_
  virtual bool lock (bool = true,  // block ?
                     bool = true); // N/A
  virtual int unlock (bool = false, // unblock ?
                      bool = true); // N/A
  inline virtual ACE_SYNCH_MUTEX_T& getLock (bool = true) { static ACE_SYNCH_MUTEX_T dummy;  ACE_ASSERT (false); ACE_NOTSUP_RETURN (dummy); ACE_NOTREACHED (return dummy;) } // forward upstream (if any) ?
  // *TODO*: this isn't nearly accurate enough
  inline virtual bool hasLock (bool = true) { ACE_ASSERT (false); ACE_ASSERT (inherited::configuration_); return !inherited::configuration_->hasReentrantSynchronousSubDownstream; } // forward upstream (if any) ?

  // implement Common_ISet_T
  inline virtual void setP (StreamStateType* streamState_in) { streamState_ = streamState_in; }

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  inline virtual bool collect (StatisticContainerType&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual void update (const ACE_Time_Value&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

 protected:
  // convenient types
  typedef ACE_Singleton<TimerManagerType,
                        ACE_SYNCH_MUTEX> TIMER_MANAGER_SINGLETON_T;
  typedef Common_StatisticHandler_T<StatisticContainerType> STATISTIC_HANDLER_T;
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            StreamControlType,
                            SessionEventType,
                            UserDataType> TASK_BASE_T;
  typedef Stream_StateMachine_Control_T<ACE_SYNCH_USE> STATE_MACHINE_T;
  typedef Stream_IStreamControl_T<StreamControlType,
                                  SessionEventType,
                                  enum Stream_StateMachine_ControlState,
                                  StreamStateType> ISTREAM_CONTROL_T;
  typedef Stream_INotify_T<SessionEventType> INOTIFY_T;
  typedef Stream_ILock_T<ACE_SYNCH_USE> ILOCK_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  typedef typename TASK_BASE_T::ISTREAM_T ISTREAM_T;
  Stream_HeadModuleTaskBase_T (ISTREAM_T*); // stream handle
#else
  Stream_HeadModuleTaskBase_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64

  // helper methods
  bool putStatisticMessage (const StatisticContainerType&); // statistic information

  // implement state machine callback
  // *NOTE*: this method is threadsafe
  virtual bool onChange (enum Stream_StateMachine_ControlState); // new state

  // hide part of Common_ITask
  virtual void stop (bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)

  // implement/hide (part of) Stream_IStreamControl_T
  virtual void finished (bool = true); // recurse upstream (if any) ?

  // disambiguate Stream_TaskBase_T and Common_StateMachine_Base_T
  using inherited::isInitialized_;

  bool                                abortSent_;
  bool                                endSeen_;
  bool                                isHighPriorityStop_;
  typename inherited::MESSAGE_QUEUE_T queue_;
  bool                                sessionEndProcessed_;
  bool                                sessionEndSent_;
  ACE_SYNCH_MUTEX_T                   stateMachineLock_;
  ILOCK_T*                            streamLock_;
  StreamStateType*                    streamState_;
  // timer
  StatisticContainerType              statistic_;
  STATISTIC_HANDLER_T                 statisticHandler_;
  long                                timerId_;

 private:
  // convenient types
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      TimePolicyType,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      SessionEventType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T (const Stream_HeadModuleTaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadModuleTaskBase_T& operator= (const Stream_HeadModuleTaskBase_T&))

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?
  // implement (part of) Stream_ITask_T
  inline virtual void waitForIdleState (bool waitForever_in = true) const { queue_.waitForIdleState (waitForever_in); }

  // implement Stream_ILinkCB
  virtual void onLink ();
  virtual void onUnlink ();

  // implement/hide (part of) Stream_IStreamControl_T
  virtual Stream_SessionId_t id () const;
  virtual unsigned int flush (bool = true,   // N/A
                              bool = false,  // N/A
                              bool = false); // N/A
  inline virtual void rewind () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
};

// include template definition
#include "stream_headmoduletask_base.inl"

#endif
