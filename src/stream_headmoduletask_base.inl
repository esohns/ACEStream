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

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>

#include "common_defines.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::Stream_HeadModuleTaskBase_T (typename LockType::MUTEX* lock_in,
                                                                                  bool autoStart_in,
                                                                                  bool generateSessionMessages_in)
 : inherited (lock_in)
 , inherited2 ()
 // *WARNING*: when disabled, this 'locks down' the pipeline head module. It
 //            will then hold the 'stream lock' during message processing to
 //            support (down)stream synchronization. This really only makes
 //            sense in fully synchronous layouts, or 'concurrent' scenarios
 //            with non-reentrant modules
 //            --> disable only if you know what you are doing
 , concurrent_ (true)
// , configuration_ (NULL)
 , isInitialized_ (false)
 , runSvcOnStart_ (false)
 , sessionEndProcessed_ (false)
 , streamState_ (NULL)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
 , active_ (false)
 , autoStart_ (autoStart_in)
 , generateSessionMessages_ (generateSessionMessages_in)
 , sessionEndSent_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  inherited2::threadCount_ = STREAM_MODULE_DEFAULT_HEAD_THREADS;

  // set group ID for worker thread(s)
  inherited2::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::~Stream_HeadModuleTaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::~Stream_HeadModuleTaskBase_T"));

  int result = -1;

  if (timerID_ != -1)
  {
    const void* act_p = NULL;
    result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  timerID_));
    else
      ACE_DEBUG ((LM_WARNING, // <-- should happen in STREAM_END_SESSION
                  ACE_TEXT ("cancelled timer in Stream_HeadModuleTaskBase_T dtor (id was: %d)\n"),
                  timerID_));
  } // end IF
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::put (ACE_Message_Block* messageBlock_in,
                                                          ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  int result = -1;

  // use the queue, if in use
  if (inherited2::thr_count_ || runSvcOnStart_)
  {
    result = inherited2::putq (messageBlock_in, timeout_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited2::name ()));
    return result;
  } // end IF

  // otherwise, process 'in-line'

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->ilock);

  bool release_lock = false;
  if (!concurrent_) release_lock =
      inherited2::configuration_->ilock->lock (true);
  else
  {
    // *NOTE*: check the status first to intercept (primarily data) messages
    //         arriving while the session is ending. Note that this cannot
    //         prevent race conditions, primarily between the 'session end'
    //         message and data message processing
  } // end ELSE

  bool stop_processing = false;
  inherited2::handleMessage (messageBlock_in,
                             stop_processing);

  if (release_lock) inherited2::configuration_->ilock->unlock (false);

  //return (stop_processing ? -1 : 0);
  return 0;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::open (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::open"));

  int result = -1;

  //// step-1: *WORKAROUND*: for some odd reason, inherited::next_ is not updated
  ////                       correctly
  //if (inherited::mod_)
  //{
  //  Stream_Module_t* module_p = inherited::mod_->next ();
  //  if (module_p)
  //    inherited::next_ = module_p->writer ();
  //} // end IF

  // step0: initialize this
  ACE_ASSERT (!inherited2::sessionData_);
  inherited2::sessionData_ = reinterpret_cast<SessionDataContainerType*> (arg_in);
  if (inherited2::sessionData_)
    inherited2::sessionData_->increase ();

  // step1: (re-)activate() the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         The second time around (i.e. the stream has been stopped/started,
  //         the queue will have been deactivate()d in the process, and getq()
  //         (see svc()) would fail (ESHUTDOWN)
  //         --> (re-)activate() the queue here !
  // step1: (re-)activate() message queue
  result = inherited2::queue_.activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // standard usecase: being implicitly invoked through ACE_Stream::push()
  // --> don't do anything, unless auto-starting
  if (autoStart_)
  {
    if (inherited2::module ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting \"%s\"...\n"),
                  inherited2::name ()));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting...\n")));

    try {
      start ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IStreamControl_T::start(), aborting\n")));
      return -1;
    }
  } // end IF

  return 0;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::close"));

  int result = -1;

  // *NOTE*: this method may be invoked
  //         - by an external thread closing down the active object
  //           --> should NEVER happen as a module !
  //         - by the worker thread which calls this after returning from svc()
  //           --> in this case, this should be a NOP...
  switch (arg_in)
  {
    // called from ACE_Task_Base on clean-up
    case 0:
    {
      if (!runSvcOnStart_)
      {
        if (inherited2::mod_)
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: worker thread (ID: %t) leaving...\n"),
                      inherited2::mod_->name ()));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("worker thread (ID: %t) leaving...\n")));
      } // end IF

      if (inherited2::thr_count_ == 0) // last thread ?
      {
        // *NOTE*: deactivate the queue so it does not accept new data
        result = inherited2::msg_queue_->deactivate ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
        result = inherited2::msg_queue_->flush ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
        else if (result &&
                 inherited2::mod_)
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: flushed %d message(s)...\n"),
                      inherited2::mod_->name (),
                      result));
      } // end IF

      // don't (need to) do anything
      break;
    }
    case 1:
    {
      // *WARNING*: SHOULD NEVER GET HERE
      // --> refer to module_closed () hook
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (-1);

      ACE_NOTREACHED (return -1;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::module_closed"));

  // *NOTE*: this will be a NOP IF the stream was
  //         stop()ped BEFORE it is deleted !

  // *NOTE*: this method is invoked by an external thread either:
  //         - from the ACE_Module dtor OR
  //         - during explicit ACE_Module::close()

  // *NOTE*: when control flow gets here, the stream SHOULD (!) already be in a
  //         final state...

  // sanity check
  // *WARNING*: this check CAN NOT prevent a potential race condition...
  if (isRunning ())
  {
    // *NOTE*: MAY happen after application receives a SIGINT
    //         select() returns -1, reactor invokes:
    //         remove_handler --> remove_reference --> delete this
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("stream still active in module_closed() --> check implementation !, continuing\n")));
    stop (true,  // wait for completion ?
          true); // locked access ?
  } // end IF

  return 0;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

  if (!runSvcOnStart_)
  {
    if (inherited2::mod_)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: worker thread (ID: %t) starting...\n"),
                  inherited2::mod_->name ()));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(%s): worker thread (ID: %t) starting...\n"),
                  ACE_TEXT (inherited2::threadName_.c_str ())));
  } // end IF

  // sanity check(s)
  ACE_ASSERT (inherited2::sessionData_);

  int                    error           = -1;
  bool                   has_finished    = false;
  ACE_Message_Block*     message_block_p = NULL;
  bool                   release_lock    = false;
  int                    result          = -1;
  const SessionDataType& session_data_r  = inherited2::sessionData_->get ();
  bool                   stop_processing = false;
  //ACE_hthread_t          thread_handle   = ACE_INVALID_HANDLE;
  //ACE_OS::thr_self (thread_handle);

  // step1: start processing data
  do
  {
    message_block_p = NULL;
    result = inherited2::getq (message_block_p,
                               NULL);
    if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited2::mod_->name ()));

        if (!has_finished)
        {
          has_finished = true;
          // enqueue(/process) STREAM_SESSION_END
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        // clean up
        message_block_p->release ();
        message_block_p = NULL;

        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage

        // signal the controller ?
        if (!has_finished)
        {
          has_finished = true;
          // enqueue(/process) STREAM_SESSION_END
          inherited::finished ();

          // has STREAM_SESSION_END been processed ? --> done
          if (!inherited2::thr_count_ && !runSvcOnStart_) goto done;

          continue; // process STREAM_SESSION_END
        } // end IF

done:
        result = 0;

        goto continue_; // STREAM_SESSION_END has been processed
      }
      default:
      {
        // sanity check(s)
        ACE_ASSERT (inherited2::configuration_);
        ACE_ASSERT (inherited2::configuration_->ilock);

        // grab lock if processing is 'non-concurrent'
        if (!concurrent_)
          release_lock = inherited2::configuration_->ilock->lock (true);

        inherited2::handleMessage (message_block_p,
                                   stop_processing);

        if (release_lock) inherited2::configuration_->ilock->unlock (false);

        // finished ?
        if (stop_processing)
        {
          // *IMPORTANT NOTE*: message_block_p has already been released() !

          if (!has_finished)
          {
            has_finished = true;
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished ();
          } // end IF

          continue;
        } // end IF

        break;
      }
    } // end SWITCH

    // session aborted ?
    // sanity check(s)
    // *TODO*: remove type inferences
    ACE_ASSERT (session_data_r.lock);
    {
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock, result);

      if (session_data_r.aborted &&
          !has_finished)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("session aborted\n")));

        has_finished = true;
        // enqueue(/process) STREAM_SESSION_END
        inherited::finished ();
      } // end IF
    } // end lock scope
  } while (true);
  result = -1;

continue_:
  return result;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::waitForIdleState"));

  // delegate this to the queue
  try {
    inherited2::queue_.waitForIdleState ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IMessageQueue::waitForIdleState, continuing\n")));
  }
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited2::mod_);
  ACE_ASSERT (inherited2::configuration_);
  //ACE_ASSERT (inherited2::sessionData_);

  //SessionDataType& session_data_r =
  //  const_cast<SessionDataType&> (inherited2::sessionData_->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *TODO*: remove type inference
      // sanity check(s)
      ACE_ASSERT (inherited2::configuration_->streamConfiguration);

      // schedule regular statistic collection ?
      // *NOTE*: the runtime-statistic module is responsible for regular
      //         reporting, the head module merely collects information
      if (inherited2::configuration_->streamConfiguration->statisticReportingInterval !=
          ACE_Time_Value::zero)
      {
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
        ACE_ASSERT (timerID_ == -1);
        ACE_Event_Handler* handler_p = &statisticCollectionHandler_;
        timerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                      NULL,                       // argument
                                                                      COMMON_TIME_NOW + interval, // first wakeup time
                                                                      interval);                  // interval
        if (timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Timer_Manager::schedule_timer(): \"%m\", returning\n"),
                      inherited2::mod_->name ()));
          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    timerID_,
//                    &interval));
      } // end IF

      break;
    }
    //// *NOTE*: the stream has been link()ed, the message contains the (merged)
    ////         upstream session data --> retain a reference
    //case STREAM_SESSION_MESSAGE_LINK:
    //{
    //  // *NOTE*: relax the concurrency policy in this case
    //  // *TODO*: this isn't very reliable and needs to be reset on unlink
    //  concurrent_ = true;

    //  //SessionDataContainerType& session_data_container_r =
    //  //  const_cast<SessionDataContainerType&> (message_inout->get ());
    //  //session_data_container_r.increase ();
    //  //sessionData_->decrease ();
    //  //sessionData_ = &session_data_container_r;

    //  break;
    //}
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      {
        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::lock_);

        if (sessionEndProcessed_) break; // done
        sessionEndProcessed_ = true;
      } // end lock scope

      if (timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                    &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      inherited2::mod_->name (),
                      timerID_));
        timerID_ = -1;
      } // end IF

      // *NOTE*: in passive 'concurrent' scenarios, there is no 'worker' thread
      //         running svc()
      //         --> do not signal completion in this case
      // *TODO*: remove type inference
      if (inherited2::thr_count_ || runSvcOnStart_)
        inherited2::stop (false); // wait ?

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  int result = -1;

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.stateMachineLock);

  if (isInitialized_)
  {
    if (timerID_ != -1)
    {
      const void* act_p = NULL;
      result =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                  &act_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    timerID_));
    } // end IF
    timerID_ = -1;

    isInitialized_ = false;
  } // end IF

  concurrent_ = configuration_in.concurrent;

  // *TODO*: remove type inferences
  active_ = configuration_in.active;
  runSvcOnStart_ = !active_;

  if (!inherited2::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_TaskBase_T::initialize(), aborting\n")));
    return false;
  } // end IF

  // *TODO*: remove type inference
  //inherited2::configuration_->ilock = this;
  isInitialized_ =
      inherited::initialize (*inherited2::configuration_->stateMachineLock);
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_StateMachine_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  inherited::change (STREAM_STATE_INITIALIZED);

  return true;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::collect"));

  ACE_UNUSED_ARG (data_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::control (StreamControlType control_in,
                                                              bool /* forwardUpStream_in */)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::control"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  enum Stream_SessionMessageType message_type = STREAM_SESSION_MESSAGE_STEP;
  switch (control_in)
  {
    case STREAM_CONTROL_FLUSH:
    {
      if (!putControlMessage (STREAM_CONTROL_FLUSH,
                              inherited2::configuration_->streamConfiguration->messageAllocator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putControlMessage(%d), returning\n"),
                    inherited2::name (),
                    control_in));
      break;
    }
    case STREAM_CONTROL_LINK:
      message_type = STREAM_SESSION_MESSAGE_LINK;
    case STREAM_CONTROL_STEP:
    {
      // sanity check(s)
      ACE_ASSERT (inherited2::sessionData_);

      inherited2::sessionData_->increase ();
      SessionDataContainerType* session_data_container_p =
          inherited2::sessionData_;

      if (!putSessionMessage (message_type,
                              session_data_container_p,
                              inherited2::configuration_->streamConfiguration->messageAllocator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putSessionMessage(%d), returning\n"),
                    inherited2::name (),
                    message_type));

      break;
    }
    case STREAM_CONTROL_UNLINK:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown control (was: %d), returning\n"),
                  control_in));
      return;
    }
  } // end SWITCH
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::notify (StreamNotificationType notification_in,
                                                             bool /* forwardUpStream_in */)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::notify"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // sanity check(s)
      if (inherited2::sessionData_)
      {
        SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited2::sessionData_->get ());

        ACE_ASSERT (session_data_r.lock);
        {
          // *TODO*: remove type inferences
          ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock);

          session_data_r.aborted = true;
        } // end lock scope
      } // end IF

      // *IMPORTANT NOTE*: in concurrent scenarios, (session) message processing
      //                   happens out of order. Control cannot be maintained
      //                   effectively without state machine- (or similar)
      //                   framework
      // *NOTE*: when the session is aborted asynchronously, no further
      //         (session) messages should be processed after the 'session end'
      //         message generated by stop()
      //         --> grab the stream lock, run stop(), and flush any queued
      //             messages
      // *TODO*: Note that the implied policy cannot be really enforced like
      //         this, as other threads may be processing messages at this point
      //         --> more synchronization is needed (e.g. wait for all stream
      //             processing queues and active worker threads to idle first)

      // *WARNING*: falls through
    }
    default:
    {
      SessionDataContainerType* session_data_container_p =
          inherited2::sessionData_;
      if (session_data_container_p)
        session_data_container_p->increase ();

      if (!putSessionMessage (static_cast<Stream_SessionMessageType> (notification_in),
                              session_data_container_p,
                              inherited2::configuration_->streamConfiguration->messageAllocator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putSessionMessage(%d), returning\n"),
                    inherited2::name (),
                    notification_in));
  
      break;
    }
  } // end SWITCH
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::start"));

  if (inherited2::sessionData_)
  {
    // *TODO*: remove type inference
    SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited2::sessionData_->get ());

    {
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);

      session_data_r.startOfSession = COMMON_TIME_NOW;
    } // end lock scope
  } // end IF

  // --> start a worker thread, if active
  inherited::change (STREAM_STATE_RUNNING);
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::stop (bool wait_in,
                                                           bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  // (try to) change state
  inherited::change (STREAM_STATE_STOPPED);

  if (wait_in)
    wait (true,
          false,
          false);
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  Stream_StateMachine_ControlState status = inherited::current ();

  return ((status == STREAM_STATE_PAUSED) || (status == STREAM_STATE_RUNNING));
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::load (Stream_ModuleList_t& modules_out,
                                                           bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::load"));

  ACE_UNUSED_ARG (modules_out);
  ACE_UNUSED_ARG (delete_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::flush (bool /* flushInbound_in */,
                                                            bool /* flushSessionMessages_in */,
                                                            bool /* flushUpStream_in */)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::flush"));

  //ACE_UNUSED_ARG (flushInbound_in);
  //ACE_UNUSED_ARG (flushSessionMessages_in);
  //ACE_UNUSED_ARG (flushUpStream_in);

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration->messageAllocator);

  // create control message
  ACE_Message_Block* message_block_p = NULL;
  if (inherited2::configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try {
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<ACE_Message_Block*> (inherited2::configuration_->streamConfiguration->messageAllocator->calloc ()),
                               ACE_Message_Block (0,
                                                  ACE_Message_Block::MB_FLUSH,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                                  ACE_Time_Value::zero,
                                                  ACE_Time_Value::max_time,
                                                  NULL,
                                                  NULL));
    }
    catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(), returning\n")));
      return;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited2::configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (0,
                                         ACE_Message_Block::MB_FLUSH,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                         ACE_Time_Value::zero,
                                         ACE_Time_Value::max_time,
                                         NULL,
                                         NULL));
  if (!message_block_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", continuing\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", returning\n")));
    return;
  } // end IF

  // enqueue the message at the stream head
  typename inherited2::TASK_T*
      stream_head_p = inherited2::sibling ()->next ()-> sibling ();
  result = stream_head_p->put (message_block_p,
                               NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", returning\n")));

    // clean up
    message_block_p->release ();

    return;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued control message...\n")));
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::pause"));

  // (try to) change state
  inherited::change (STREAM_STATE_PAUSED);
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
DataMessageType*
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (inherited2::configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try {
      // *TODO*: remove type inference
      message_p =
          static_cast<DataMessageType*> (inherited2::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !inherited2::configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (inherited2::configuration_->streamConfiguration->messageAllocator)
    {
      if (inherited2::configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                    requestedSize_in));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                  requestedSize_in));
  } // end IF

  return message_p;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::rewind"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::upStream (Stream_Base_t* upStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::upStream"));

  ACE_UNUSED_ARG (upStream_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Base_t*
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::upStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::upStream"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_StateMachine_ControlState
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::status () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::status"));

  Stream_StateMachine_ControlState result = inherited::current ();

  return result;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::lock (bool block_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::lock"));

  int result = -1;
  ACE_SYNCH_MUTEX_T& lock_r = inherited2::queue_.lock ();

  result = (block_in ? lock_r.acquire () : lock_r.tryacquire ());
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error == EBUSY)
      return false; 
  } // end IF

  return true;
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::unlock (bool unlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::unlock"));

  int result = -1;
  ACE_SYNCH_MUTEX_T& lock_r = inherited2::queue_.lock ();
  ACE_thread_mutex_t& mutex_r = lock_r.lock ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!ACE_OS::thr_equal (reinterpret_cast<ACE_thread_t> (mutex_r.OwningThread),
                          ACE_OS::thr_self ()))
    return -1;
#else
  if (!ACE_OS::thr_equal (static_cast<ACE_thread_t> (mutex_r.__data.__owner),
                          ACE_OS::thr_self ()))
    return -1;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  do
  {
    result = lock_r.release ();
    if (!unlock_in) break;
  } while (mutex_r.RecursionCount > 0);
#else
  do
  {
    result = lock_r.release ();
    if (!unlock_in) break;
  } while (mutex_r.__data.__count > 0);
#endif
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                inherited2::name ()));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return mutex_r.RecursionCount;
#else
  return mutex_r.__data.__count;
#endif
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ACE_SYNCH_RECURSIVE_MUTEX&
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::getLock ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::getLock"));

  ACE_SYNCH_RECURSIVE_MUTEX dummy;

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (dummy);

  ACE_NOTREACHED (return dummy;)
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::hasLock ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::hasLock"));

  // *TODO*: this isn't nearly accurate enough...
  return concurrent_;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::wait (bool waitForThreads_in,
                                                           bool waitForUpStream_in,
                                                           bool waitForDownStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::wait"));

  ACE_UNUSED_ARG (waitForUpStream_in);
  ACE_UNUSED_ARG (waitForDownStream_in);

  // *IMPORTANT NOTE*: when a connection is close()d, a race condition may
  //                   arise here between any of the following actors:
  // - the application (main) thread waiting in Stream_Base_T::waitForCompletion
  // - a (network) event dispatching thread (connection hanndler calling
  //   handle_close() --> wait() of the (network) data processing
  //   (sub-)stream)
  // - a stream head module thread pushing the SESSION_END message (i.e.
  //   processing in the 'Net Source/Target' module)
  // - a 'Net IO' module thread processing the SESSION_END message

  int result = -1;

  // *NOTE*: be sure to release the stream lock to support 'concurrent'
  //         scenarios (e.g. scenarios where upstream generates the data)
  int nesting_level = -1;
  ACE_ASSERT (inherited2::configuration_->ilock);
  //ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (configuration_->ilock->getLock ());
  //ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock);
  nesting_level = inherited2::configuration_->ilock->unlock (true);

  // step1: wait for final state
  inherited::wait (STREAM_STATE_FINISHED,
                   NULL); // <-- block

  // step2: wait for worker(s) to join ?
  if (waitForThreads_in)
  {
    // *NOTE*: pthread_join() returns EDEADLK when the calling thread IS the
    //         thread to join
    //         --> prevent this by comparing thread ids
    // *TODO*: find a way to handle multi-thread modules
    if (ACE_OS::thr_equal (ACE_OS::thr_self (),
                           inherited2::threadIDs_[0].id ()))
      goto continue_;

    // *IMPORTANT NOTE*: (on Win32) only one thread may inherited2::wait(),
    //                   because ::CloseHandle() was being called twice on the
    //                   same handle, throwing exceptions
    // *TODO*: This is a bug in ACE being worked around here
    //         --> clarify the issue and submit a patch
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::lock_);

    if (inherited2::thr_count_)
    {
      // *NOTE*: the task has a dedicated worker thread

      ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (inherited2::lock_);
      {
        ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock);
        result = inherited2::wait ();
      } // end lock scope
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
    } // end IF
    else if (runSvcOnStart_)
    {
      // *NOTE*: the stream head module is using the calling thread

      ACE_thread_t thread_id = inherited2::threadIDs_[0].id ();
      ACE_THR_FUNC_RETURN status;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_hthread_t handle = inherited2::threadIDs_[0].handle ();
      if (handle != ACE_INVALID_HANDLE)
      {
        result = ACE_Thread::join (handle, &status);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Thread::join(): \"%m\", continuing\n")));
        else if (result == 0)
        {
          // *NOTE*: successful join()s close the thread handle
          //         (see OS_NS_Thread.inl:2971)
          inherited2::threadIDs_[0].handle (ACE_INVALID_HANDLE);
        } // end IF
        inherited2::threadIDs_[0].id (std::numeric_limits<DWORD>::max ());
      } // end IF
      else
        result = 0;
#else
      if (static_cast<int> (thread_id) != -1)
      {
        result = ACE_Thread::join (thread_id, NULL, &status);
        inherited2::threadIDs_[0].id (-1);
      } // end IF
      else
        result = 0;
#endif
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread::join(%lu): \"%m\", continuing\n"),
                    thread_id));
    } // end IF
  } // end IF

continue_:
  if (nesting_level >= 0)
    COMMON_ILOCK_ACQUIRE_N (inherited2::configuration_->ilock,
                            nesting_level + 1);

  return;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
const Stream_Module_t*
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
std::string
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::name () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::name"));

  std::string result = ACE_TEXT_ALWAYS_CHAR (inherited2::name ());
  return result;
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
const StreamStateType&
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::state"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (StreamStateType ());

  ACE_NOTREACHED (return StreamStateType ());
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::initialize (const StreamStateType& streamState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  streamState_ = &const_cast<StreamStateType&> (streamState_in);
//  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

  inherited::change (STREAM_STATE_INITIALIZED);

  return true;
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  ACE_ASSERT (inherited2::sessionData_);

  // step1: update session state
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited2::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

  // step2: prepare session data object container
  inherited2::sessionData_->increase ();
  SessionDataContainerType* session_data_container_p =
      inherited2::sessionData_;

  // step3: send the statistic data downstream
  //  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return putSessionMessage (STREAM_SESSION_MESSAGE_STATISTIC,
                            session_data_container_p,
                            inherited2::configuration_->streamConfiguration->messageAllocator);
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::onChange (Stream_StateType_t newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onChange"));

  // sanity check(s)
  ACE_ASSERT (inherited2::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited2::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::stateLock_);

  int result = -1;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T> reverse_lock (*inherited::stateLock_);

  switch (newState_in)
  {
    case STREAM_STATE_INITIALIZED:
    {
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::lock_);

      sessionEndSent_ = false;
      sessionEndProcessed_ = false;

      // --> re-initialize ?
      if (!inherited2::threadIDs_.empty ())
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_hthread_t handle = inherited2::threadIDs_[0].handle ();
        if (handle != ACE_INVALID_HANDLE)
          if (!::CloseHandle (handle))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                        handle,
                        ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
#endif
        inherited2::threadIDs_.clear ();
      } // end IF

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: head module (re-)initialized...\n"),
      //            inherited2::mod_->name ()));

      break;
    }
    case STREAM_STATE_RUNNING:
    {
      // *NOTE*: implement tape-recorder logic:
      //         transition PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      //         --> check for this condition before doing anything else...
      ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited::stateLock_);

      if (inherited::state_ == STREAM_STATE_PAUSED)
      {
        // resume worker ?
        if (inherited2::thr_count_ > 0)
        {
          result = inherited2::resume ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task::resume(): \"%m\", continuing\n")));
        } // end IF
        else
        {
          // task object not active --> resume the borrowed thread
          ACE_hthread_t handle = inherited2::threadIDs_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
          ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif
          result = ACE_Thread::resume (handle);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Thread::resume(): \"%m\", continuing\n")));
        } // end ELSE

        break;
      } // end IF

      // send initial session message downstream ?
      if (generateSessionMessages_)
      {
        ACE_ASSERT (inherited2::sessionData_);

        inherited2::sessionData_->increase ();
        SessionDataContainerType* session_data_container_p =
            inherited2::sessionData_;

        if (!putSessionMessage (STREAM_SESSION_MESSAGE_BEGIN,                                       // type
                                session_data_container_p,                                           // session data
                                inherited2::configuration_->streamConfiguration->messageAllocator)) // allocator
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_BEGIN) failed, continuing\n")));
          break;
        } // end IF
      } // end IF

      if (active_)
      {
        // spawn a worker thread
        // *TODO*: rewrite for thread counts > 1 (see also: wait() above)
        {
          ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, inherited2::lock_);

          ACE_ASSERT (inherited2::threadCount_ >= 1);

          ACE_thread_t* thread_ids_p = NULL;
          ACE_NEW_NORETURN (thread_ids_p,
                            ACE_thread_t[inherited2::threadCount_]);
          if (!thread_ids_p)
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                        (sizeof (ACE_thread_t) * inherited2::threadCount_)));
            return;
          } // end IF
          ACE_OS::memset (thread_ids_p, 0, sizeof (thread_ids_p));
          ACE_hthread_t* thread_handles_p = NULL;
          ACE_NEW_NORETURN (thread_handles_p,
                            ACE_hthread_t[inherited2::threadCount_]);
          if (!thread_handles_p)
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                        (sizeof (ACE_hthread_t) * inherited2::threadCount_)));

            // clean up
            delete [] thread_ids_p;

            return;
          } // end IF
          ACE_OS::memset (thread_handles_p, 0, sizeof (thread_handles_p));
          const char** thread_names_p = NULL;
          ACE_NEW_NORETURN (thread_names_p,
                            const char*[inherited2::threadCount_]);
          if (!thread_names_p)
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                        (sizeof (const char*) * inherited2::threadCount_)));

            // clean up
            delete [] thread_ids_p;
            delete [] thread_handles_p;

            return;
          } // end IF
          ACE_OS::memset (thread_names_p, 0, sizeof (thread_names_p));
          char* thread_name_p = NULL;
          std::string buffer;
          std::ostringstream converter;
          for (unsigned int i = 0;
               i < inherited2::threadCount_;
               i++)
          {
            thread_name_p = NULL;
            ACE_NEW_NORETURN (thread_name_p,
                              char[BUFSIZ]);
            if (!thread_name_p)
            {
              ACE_DEBUG ((LM_CRITICAL,
                          ACE_TEXT ("failed to allocate memory (%u), aborting\n"),
                          (sizeof (char) * BUFSIZ)));

              // clean up
              delete [] thread_ids_p;
              delete [] thread_handles_p;
              for (unsigned int j = 0; j < i; j++)
                delete [] thread_names_p[j];
              delete [] thread_names_p;

              return;
            } // end IF
            ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << (i + 1);
            buffer = inherited2::threadName_;
            buffer += ACE_TEXT_ALWAYS_CHAR (" #");
            buffer += converter.str ();
            ACE_OS::strcpy (thread_name_p,
                            buffer.c_str ());
            thread_names_p[i] = thread_name_p;
          } // end FOR
          result =
            ACE_Task_Base::activate ((THR_NEW_LWP      |
                                      THR_JOINABLE     |
                                      THR_INHERIT_SCHED),                         // flags
                                     static_cast<int> (inherited2::threadCount_), // # threads
                                     0,                                           // force spawning
                                     ACE_DEFAULT_THREAD_PRIORITY,                 // priority
                                     inherited2::grp_id (),                       // group id (see above)
                                     NULL,                                        // corresp. task --> use 'this'
                                     thread_handles_p,                            // thread handle(s)
                                     NULL,                                        // thread stack(s)
                                     NULL,                                        // thread stack size(s)
                                     thread_ids_p,                                // thread id(s)
                                     thread_names_p);                             // thread name(s)
          if (result == -1)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task_Base::activate(): \"%m\", continuing\n")));

            // clean up
            delete[] thread_ids_p;
            delete[] thread_handles_p;
            for (unsigned int i = 0; i < inherited2::threadCount_; i++)
              delete[] thread_names_p[i];
            delete[] thread_names_p;

            break;
          } // end IF

          std::ostringstream string_stream;
          ACE_Thread_ID thread_id;
          for (unsigned int i = 0;
               i < inherited2::threadCount_;
               i++)
          {
            string_stream << ACE_TEXT_ALWAYS_CHAR ("#") << (i + 1)
                          << ACE_TEXT_ALWAYS_CHAR (" ")
                          << thread_ids_p[i]
                          << ACE_TEXT_ALWAYS_CHAR ("\n");

            // clean up
            delete [] thread_names_p[i];

            thread_id.handle (thread_handles_p[i]);
            thread_id.id (thread_ids_p[i]);
            inherited2::threadIDs_.push_back (thread_id);
          } // end FOR
          std::string thread_ids_string = string_stream.str ();
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("(%s) spawned %u worker thread(s) (group: %d):\n%s"),
                      ACE_TEXT (inherited2::threadName_.c_str ()),
                      inherited2::threadCount_,
                      inherited2::grp_id (),
                      ACE_TEXT (thread_ids_string.c_str ())));

          // clean up
          delete[] thread_ids_p;
          delete[] thread_handles_p;
          delete[] thread_names_p;
        } // end lock scope
      } // end IF
      else if (runSvcOnStart_)
      {
        // *NOTE*: if the object is 'passive', the whole operation pertaining
        //         to newState_in is processed 'inline' by the calling thread,
        //         i.e. would complete 'before' the state has transitioned to
        //         'running'
        //         --> set the state early
        inherited::state_ = STREAM_STATE_RUNNING;

        {
          ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, inherited2::lock_);

          // sanity check(s)
          ACE_ASSERT (inherited2::threadIDs_.empty ());

          ACE_Thread_ID thread_id;
          thread_id.id (ACE_Thread::self ());
          ACE_hthread_t handle = ACE_INVALID_HANDLE;
          ACE_Thread::self (handle);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          HANDLE process_handle = ::GetCurrentProcess ();
          if (!::DuplicateHandle (process_handle,
                                  handle,
                                  process_handle,
                                  &handle,
                                  0,
                                  FALSE,
                                  DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to DuplicateHandle(0x%@): \"%s\", continuing\n"),
                        handle,
                        ACE_TEXT (Common_Tools::error2String (GetLastError ()).c_str ())));
#endif
          thread_id.handle (handle);
          inherited2::threadIDs_.push_back (thread_id);
        } // end lock scope

        {
          ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock);

          result = svc ();
          if (result == -1) // *NOTE*: most probable reason: session aborted
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task_Base::svc(): \"%m\", continuing\n")));
          result = close (0);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task_Base::close(): \"%m\", continuing\n")));
        } // end lock scope

//        // send initial session message downstream
//        if (!putSessionMessage (STREAM_SESSION_END,
//                                sessionData_,
//                                false))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));
//          break;
//        } // end IF
      } // end ELSE IF
      else
      {
        // *IMPORTANT NOTE*: this means that there is no worker thread
        //                   driving this module; neither in-line, nor
        //                   dedicated

        // *NOTE*: check if any of the modules failed to initialize
        //         --> just signal the controller

        // sanity check(s)
        ACE_ASSERT (inherited2::sessionData_);
        // *TODO*: remove type inferences
        SessionDataType& session_data_r =
            const_cast<SessionDataType&> (inherited2::sessionData_->get ());

        {
          ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, *session_data_r.lock);
          if (session_data_r.aborted)
            this->finished ();
        } // end lock scope
      } // end IF

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      // suspend the worker(s) ?
      if (inherited2::thr_count_ > 0)
      {
        result = inherited2::suspend ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::suspend(): \"%m\", continuing\n")));
      } // end IF
      else
      {
        // task object not active --> suspend the borrowed thread
        ACE_hthread_t handle = inherited2::threadIDs_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
        ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif
        result = ACE_Thread::suspend (handle);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Thread::suspend(): \"%m\", continuing\n")));
      } // end ELSE

      break;
    }
    case STREAM_STATE_STOPPED:
    {
      bool done = false;

      {
        ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited::stateLock_);

        switch (inherited::state_)
        {
          case STREAM_STATE_PAUSED:
          {
            // resume worker ?
            if (inherited2::thr_count_ > 0)
            {
              result = inherited2::resume ();
              if (result == -1)
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to ACE_Task::resume(): \"%m\", continuing\n")));
            } // end IF
            else
            {
              // task is not 'active' --> resume the calling thread (i.e. the
              // thread that invoked start())
              ACE_hthread_t handle = inherited2::threadIDs_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
              ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif
              result = ACE_Thread::resume (handle);
              if (result == -1)
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to ACE_Thread::resume(): \"%m\", continuing\n")));
            } // end ELSE
            break;
          }
          case STREAM_STATE_FINISHED:
          {
            done = true;
            break;
          }
          default:
            break;
        } // end SWITCH
        if (done) break;

        // *NOTE*: in 'passive' mode the finished() method waits for the stream
        //         --> set the (intermediate) state early
        inherited::state_ = STREAM_STATE_STOPPED;
      } // end lock scope

      // *TODO*: remove type inference
      if ((inherited2::thr_count_ > 0) ||
          (runSvcOnStart_  &&
           !ACE_OS::thr_equal (ACE_OS::thr_self (),
                               inherited2::threadIDs_[0].id ())))
        inherited2::stop (false); // wait ?
      else
      {
        //if (runSvcOnStart_)
        //{
        //  ACE_ASSERT (threadID_ != -1);
        //  result = ACE_Thread::kill (threadID_, SIGKILL);
        //  if (result == -1)
        //    ACE_DEBUG ((LM_ERROR,
        //                ACE_TEXT ("ACE_Thread::kill(%d, \"%S\") failed: \"%m\", continuing\n"),
        //                threadID_, SIGKILL));
        //} // end IF

        // signal the controller
        this->finished ();
      } // end ELSE

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      {
        ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited::stateLock_);

        // *NOTE*: modules processing the final session message (see below) may
        //         (indirectly) invoke wait() on the stream,
        //         which would deadlock if the implementation is 'passive'
        //         --> set the state early
        // *TODO*: this may not be the best way to handle that case (i.e. it
        //         could introduce other race conditions...)
        inherited::state_ = STREAM_STATE_FINISHED;
      } // end lock scope

      // send final session message downstream ?
      // *IMPORTANT NOTE*: the transition STOPPED --> FINISHED is automatic (see
      //                   above [*NOTE*: in 'active'/svc() based scenarios,
      //                   shutdown() triggers this transition]).
      //                   However, as the stream may be stop()/finished()-ed
      //                   concurrently (e.g. (safety/sanity) precaution during
      //                   shutdown, connection reset, ...), this transition
      //                   could trigger several times
      //                   --> ensure that only a single 'session end' message
      //                       is generated per session
      bool send_end_message = false;
      {
        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited2::lock_);

        if (!sessionEndSent_ &&
            !sessionEndProcessed_) // already processed upstream 'session end' ?
        {
          sessionEndSent_ = true;
          send_end_message = true;
        } // end IF
      } // end lock scope
      if (generateSessionMessages_ &&
          send_end_message)
      {
        ACE_ASSERT (inherited2::sessionData_);

        inherited2::sessionData_->increase ();
        SessionDataContainerType* session_data_container_p =
            inherited2::sessionData_;

        if (!putSessionMessage (STREAM_SESSION_MESSAGE_END,                                         // session message type
                                session_data_container_p,                                           // session data
                                inherited2::configuration_->streamConfiguration->messageAllocator)) // allocator
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));
      } // end IF

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("stream processing complete\n")));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid state transition: \"%s\" --> \"%s\", continuing\n"),
                  ACE_TEXT (inherited::state2String (inherited::state_).c_str ()),
                  ACE_TEXT (inherited::state2String (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}

template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::putControlMessage (StreamControlType messageType_in,
                                                                        Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putControlMessage"));

  int result = -1;

  // create control message
  ACE_Message_Block* message_block_p = NULL;
  if (allocator_in)
  {
allocate:
    try {
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<ACE_Message_Block*> (allocator_in->calloc ()),
                               ACE_Message_Block (0,
                                                  messageType_in,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  NULL,
                                                  ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                                  ACE_Time_Value::zero,
                                                  ACE_Time_Value::max_time,
                                                  NULL,
                                                  NULL));
    }
    catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::calloc(), aborting\n")));
      return false;
    }

    // keep retrying ?
    if (!message_block_p &&
        !allocator_in->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      ACE_Message_Block (0,
                                         messageType_in,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                         ACE_Time_Value::zero,
                                         ACE_Time_Value::max_time,
                                         NULL,
                                         NULL));
  if (!message_block_p)
  {
    if (allocator_in)
    {
      if (allocator_in->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
    return false;
  } // end IF

  // pass message downstream
  result = const_cast<OWN_TYPE_T*> (this)->put (message_block_p,
                                                NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", aborting\n")));

    // clean up
    message_block_p->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued control message...\n")));

  return true;
}
template <typename LockType,
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
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
                            StatisticContainerType>::putSessionMessage (Stream_SessionMessageType messageType_in,
                                                                        SessionDataContainerType*& sessionData_inout,
                                                                        Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // sanity check(s)
  ACE_ASSERT (streamState_);

  int result = -1;

  // create a session message
  SessionMessageType* session_message_p = NULL;
  if (allocator_in)
  {
allocate:
    try {
      // *IMPORTANT NOTE*: 0 --> session message !
      session_message_p =
        static_cast<SessionMessageType*> (allocator_in->malloc (0));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));

      // clean
      if (sessionData_inout)
      {
        sessionData_inout->decrease ();
        sessionData_inout = NULL;
      } // end IF

      return false;
    }

    // keep retrying ?
    if (!session_message_p &&
        !allocator_in->block ())
      goto allocate;
  } // end IF
  else
  {
    // *TODO*: remove type inference
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (messageType_in,
                                          sessionData_inout,
                                          streamState_->userData));
  } // end ELSE
  if (!session_message_p)
  {
    if (allocator_in)
    {
      if (allocator_in->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));

    // clean
    if (sessionData_inout)
    {
      sessionData_inout->decrease ();
      sessionData_inout = NULL;
    } // end IF

    return false;
  } // end IF
  if (allocator_in)
  {
    // *TODO*: remove type inference
    session_message_p->initialize (messageType_in,
                                   sessionData_inout,
                                   streamState_->userData);
  } // end IF

  // pass message downstream
  result = const_cast<OWN_TYPE_T*> (this)->put (session_message_p,
                                                NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", aborting\n")));

    // clean up
    session_message_p->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));

  return true;
}
