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

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/Time_Value.h"

#include "common_defines.h"
#include "common_ilock.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_error_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_HeadModuleTaskBase_T (ISTREAM_T* stream_in,
#else
                            UserDataType>::Stream_HeadModuleTaskBase_T (typename inherited::ISTREAM_T* stream_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                                        bool autoStart_in,
                                                                        enum Stream_HeadModuleConcurrency concurrency_in,
                                                                        bool generateSessionMessages_in)
 : inherited (stream_in)
 , inherited2 (NULL)
 , concurrency_ (concurrency_in)
 , finishOnDisconnect_ (false)
 , hasReentrantSynchronousSubDownstream_ (true)
 , sessionEndProcessed_ (false)
 , sessionEndSent_ (false)
 , stateMachineLock_ (NULL, // name
                      NULL) // attributes
 , streamLock_ (stream_in)
 , streamState_ (NULL)
 , statisticHandler_ (COMMON_STATISTIC_ACTION_COLLECT,
                      this,
                      false)
 , timerId_ (-1)
 /////////////////////////////////////////
 , autoStart_ (autoStart_in)
 , generateSessionMessages_ (generateSessionMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  inherited::threadCount_ = STREAM_MODULE_DEFAULT_HEAD_THREADS;

  if (unlikely (!inherited2::initialize (stateMachineLock_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_StateMachine_Base_T::initialize(), returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::~Stream_HeadModuleTaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::~Stream_HeadModuleTaskBase_T"));

  int result = -1;

  if (unlikely (timerId_ != -1))
  {
    // sanity check(s)
    ACE_ASSERT (inherited::configuration_);

    typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                 : TIMER_MANAGER_SINGLETON_T::instance ());
    ACE_ASSERT (itimer_manager_p);
    const void* act_p = NULL;
    result = itimer_manager_p->cancel_timer (timerId_,
                                             &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  timerId_));
    else
      ACE_DEBUG ((LM_WARNING, // <-- should happen during STREAM_END_SESSION
                  ACE_TEXT ("%s: cancelled timer in dtor (id was: %d)\n"),
                  inherited::mod_->name (),
                  timerId_));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  int result = -1;

  // use the queue, if necessary
  switch (concurrency_)
  {
    case STREAM_HEADMODULECONCURRENCY_ACTIVE:
    case STREAM_HEADMODULECONCURRENCY_PASSIVE:
    {
      result = inherited::putq (messageBlock_in, timeout_in);
      if (unlikely (result == -1))
      {
        // *NOTE*: most probable reason: link()ed stop(); data arriving after
        //         STREAM_SESSION_END
        int error = ACE_OS::last_error ();
        if (error != ESHUTDOWN)
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
      } // end IF
      return result;
    }
    default:
      break;
  } // end SWITCH

  // --> process 'in-line'

  bool release_lock = false;
  if (unlikely (!hasReentrantSynchronousSubDownstream_))
  { ACE_ASSERT (streamLock_);
    try {
      release_lock = streamLock_->lock (true,  // block ?
                                        true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF

  bool stop_processing = false;
  inherited::handleMessage (messageBlock_in,
                            stop_processing);

  // clean up
  if (unlikely (release_lock))
  { ACE_ASSERT (streamLock_);
    try {
      streamLock_->unlock (false, // unlock ?
                           true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF

  if (unlikely (stop_processing &&
                !sessionEndProcessed_))
  {
    // enqueue(/process) STREAM_SESSION_END
    inherited2::finished ();
  } // end IF

  //return (stop_processing ? -1 : 0);
  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::open (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::open"));

  int result = -1;

  // step0: initialize this
  ACE_ASSERT (!inherited::sessionData_);
  inherited::sessionData_ =
    reinterpret_cast<SessionDataContainerType*> (arg_in);
  if (likely (inherited::sessionData_))
    inherited::sessionData_->increase ();

  // step1: initialize the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state.
  //         The second time around (i.e. the stream has been stopped/started,
  //         the queue will have been deactivate()d in the process, and getq()
  //         (see svc()) would fail (ESHUTDOWN)
  //         --> (re-)activate() the queue
  result = inherited::queue_.activate ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  // standard usecase: being implicitly invoked through ACE_Stream::push()
  // --> don't do anything, unless auto-starting
  if (unlikely (autoStart_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: auto-starting\n"),
                inherited::mod_->name ()));

    try {
      start ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::start(), aborting\n"),
                  inherited::mod_->name ()));
      return -1;
    }
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::close"));

  int result = -1;
  int result_2 = -1;

  // *NOTE*: this method may be invoked by (switch arg_in):
  //         - 1: an external thread closing down the active object; should
  //              NEVER happen as a module
  //         - 0: worker thread(s) returning from svc(); essentially a NOP. Note
  //              that inherited::thr_count_ has already been decremented
  switch (arg_in)
  {
    case 0:
    {
//#if defined (_DEBUG)
      //{ ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, result);
      //  if ((concurrency_ == STREAM_HEADMODULECONCURRENCY_ACTIVE) ||
      //      ACE_OS::thr_equal (ACE_OS::thr_self (),
      //                         inherited::threads_[0].id ()))
      //    ACE_DEBUG ((LM_DEBUG,
      //                ACE_TEXT ("%s: %sthread (id was: %t) leaving\n"),
      //                inherited::mod_->name (),
      //                (concurrency_ == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
      //                                                                     : ACE_TEXT (""))));
      //} // end lock scope
//#endif

      // inherited::thr_count_ has already been decremented at this stage
      // --> there should not be a race condition
      if (inherited::thr_count_ == 0) // last thread ?
      {
        // *NOTE*: this deactivates the queue so it does not accept new data
        //         after the last (worker) thread has left
        result_2 = inherited::msg_queue_->deactivate ();
        if (unlikely (result_2 == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        result_2 = inherited::msg_queue_->flush ();
        if (unlikely (result_2 == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::flush(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        else if (result_2)
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: flushed %d message(s)\n"),
                      inherited::mod_->name (),
                      result_2));

        // clean up
        if (unlikely (inherited::sessionData_))
        {
          inherited::sessionData_->decrease ();
          inherited::sessionData_ = NULL;
        } // end IF
      } // end IF

      result = 0;

      break;
    }
    case 1:
    {
      if (unlikely (inherited::thr_count_ == 0))
      {
        result = 0;
        break; // nothing to do
      } // end IF

      // *NOTE*: make sure there is a message queue
      if (likely (inherited::msg_queue_))
      {
        stop (false, // wait for completion ?
              true); // N/A

        result = 0;
      } // end IF
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: cannot signal %d worker thread(s) (no message queue) --> check implementation\n"),
                    inherited::mod_->name (),
                    inherited::thr_count_));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid argument (was: %u), aborting\n"),
                  inherited::mod_->name (),
                  arg_in));
      break;
    }
  } // end SWITCH

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::module_closed"));

  // *NOTE*: this will be a NOP IF the stream was
  //         stop()ped BEFORE it is deleted !

  // *NOTE*: this method is invoked by an external thread either:
  //         - from the ACE_Module dtor OR
  //         - during explicit ACE_Module::close() (e.g. stream is being
  //           reinitialized --> module reset)

  // *NOTE*: when control flow gets here, the stream should already be in a
  //         final state

  // sanity check
  // *WARNING*: this test CANNOT prevent potential race conditions
  if (unlikely (isRunning ()))
  {
    // *NOTE*: MAY happen after application receives a SIGINT
    //         select() returns -1, reactor invokes:
    //         remove_handler --> remove_reference --> delete this
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: stream still active in Stream_HeadModuleTaskBase_T::module_closed(), continuing\n"),
                inherited::mod_->name ()));
    stop (true,   // wait for completion ?
          false); // N/A
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              (concurrency_ == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                   : ACE_TEXT ("")),
              inherited::grp_id_));
#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  int                            error                    = 0;
  bool                           has_finished             = false;
  ACE_Message_Block*             message_block_p          = NULL;
  bool                           release_lock             = false;
  int                            result                   = -1;
  int                            result_2                 = 0;
  SessionDataContainerType*      session_data_container_p =
    inherited::sessionData_;
  const SessionDataType*         session_data_p           =
    &inherited::sessionData_->getR ();
  bool                           stop_processing          = false;

  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

      if (!has_finished)
      {
        has_finished = true;
        // enqueue(/process) STREAM_SESSION_END
        inherited2::finished ();
      } // end IF

      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage

        // signal the controller ?
        if (!has_finished)
        {
          has_finished = true;
          // enqueue(/process) STREAM_SESSION_END
          inherited2::finished ();
        } // end IF

        if (inherited::thr_count_ > 1)
        {
          result_2 = inherited::putq (message_block_p, NULL);
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_block_p->release (); message_block_p = NULL;
            break;
          } // end IF
        } // end IF
        else
        {
          message_block_p->release (); message_block_p = NULL;
        } // end ELSE

        // has STREAM_SESSION_END been processed ?
        if (!sessionEndProcessed_)
          continue; // continue processing until STREAM_SESSION_END

        // --> STREAM_SESSION_END has been processed, leave

        result = 0;

        goto done;
      }
      default:
      {
        // grab stream lock if processing is 'concurrent'
        if (unlikely (!hasReentrantSynchronousSubDownstream_))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        inherited::handleMessage (message_block_p,
                                  stop_processing);

        // *IMPORTANT NOTE*: as the session data may change when this stream is
        //                   (un-)link()ed (e.g. inbound network data
        //                   processing), the handle may have to be updated
        if (unlikely (inherited::sessionData_ &&
                      (session_data_container_p != inherited::sessionData_)))
          session_data_p = &inherited::sessionData_->getR ();

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        // finished ?
        if (unlikely (stop_processing))
        {
          // *IMPORTANT NOTE*: message_block_p has already been released()

          if (!has_finished)
          {
            has_finished = true;
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished ();
          } // end IF

          continue; // continue processing until STREAM_SESSION_END
        } // end IF

        // iff STREAM_SESSION_END has been processed: flush data ?
        // --> flush all (session-)data; process remaining control messages only
        // *TODO*: stop_processing is set when STREAM_SESSION_END is processed;
        //         this section is currently not being reached
        // *TODO*: an alternative strategy could be to 'lock the queue' (i.e.
        //         modify put()), and 'filter-sort' the remaining messages when
        //         enqueueing the session end message; there would be no need
        //         to 'flush'
        if (unlikely (sessionEndProcessed_ && result))
        {
          result_2 =
            inherited::queue_.flush (true); // flush session messages ?
          if (result_2 == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to Stream_IMessageQueue::flush(true): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
          else if (result_2)
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: session has ended, flushed %u message(s)\n"),
                        inherited::mod_->name (),
                        result_2));
        } // end IF

        break;
      }
    } // end SWITCH
    // sanity check(s)
    if (unlikely (result_2 == -1)) // error (see above)
      break;
    // session aborted ?
    // *TODO*: remove type inferences
    { ACE_ASSERT (session_data_p->lock);
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_p->lock, result);
      if (unlikely (session_data_p->aborted &&
                    !has_finished))
      { // *TODO*: remove type inferences
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session (id was: %u) aborted\n"),
                    inherited::mod_->name (),
                    session_data_p->sessionId));

        has_finished = true;
        // enqueue(/process) STREAM_SESSION_END
        inherited::finished ();
      } // end IF
    } // end lock scope
  } while (true);
  result = -1;

done:
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t) leaving\n"),
              inherited::mod_->name (),
              (concurrency_ == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                   : ACE_TEXT (""))));
#endif // _DEBUG

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      inherited2::change (STREAM_STATE_FINISHED);
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    { ACE_ASSERT (inherited::sessionData_);

      // schedule regular statistic collection ?
      // *NOTE*: the runtime-statistic module is responsible for regular
      //         reporting, the head module merely collects information
      // *TODO*: remove type inference
      if (inherited::configuration_->statisticReportingInterval !=
          ACE_Time_Value::zero)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
        ACE_ASSERT (timerId_ == -1);
        timerId_ =
          itimer_manager_p->schedule_timer (&statisticHandler_, // event handler
                                            NULL,                         // asynchronous completion token
                                            COMMON_TIME_NOW + interval,   // first wakeup time
                                            interval);                    // interval
        if (unlikely (timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", returning\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    timerId_,
                    &interval));
#endif // _DEBUG
      } // end IF

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      if (finishOnDisconnect_)
        inherited2::change (STREAM_STATE_FINISHED);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        if (unlikely (sessionEndProcessed_))
          break; // done
        sessionEndProcessed_ = true;
      } // end lock scope

      if (timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer() (id was: %d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      timerId_));
        timerId_ = -1;
      } // end IF

      if (likely (concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
        inherited::stop (false, // wait for completion ?
                         true); // N/A

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    finishOnDisconnect_ = false;
    sessionEndProcessed_ = false;
    sessionEndSent_ = false;
//    streamLock_ =  NULL;
    streamState_ = NULL;

    if (timerId_ != -1)
    { ACE_ASSERT (inherited::configuration_);
      typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                   : TIMER_MANAGER_SINGLETON_T::instance ());
      ACE_ASSERT (itimer_manager_p);
      const void* act_p = NULL;
      result = itimer_manager_p->cancel_timer (timerId_,
                                               &act_p);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer() (id was: %d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    timerId_));
      timerId_ = -1;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
  concurrency_ = configuration_in.concurrency;
  finishOnDisconnect_ = configuration_in.finishOnDisconnect;
  hasReentrantSynchronousSubDownstream_ =
      configuration_in.hasReentrantSynchronousSubDownstream;

  // *NOTE*: deactivate the queue so it does not accept new data
  result = inherited::msg_queue_->activate ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

  if (unlikely (!inherited::initialize (configuration_in,
                                        allocator_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  inherited2::change (STREAM_STATE_INITIALIZED);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::control (SessionControlType control_in,
                                                    bool /* forwardUpStream_in */)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::control"));

  SessionEventType message_type_e = STREAM_SESSION_MESSAGE_INVALID;

  switch (control_in)
  {
    // control
    case STREAM_CONTROL_ABORT:
    case STREAM_CONTROL_CONNECT:
    case STREAM_CONTROL_DISCONNECT:
    case STREAM_CONTROL_FLUSH:
    case STREAM_CONTROL_RESET:
    case STREAM_CONTROL_STEP:
    case STREAM_CONTROL_STEP_2:
    {
      if (!inherited::putControlMessage (control_in))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putControlMessage(%d), continuing\n"),
                    inherited::name (),
                    control_in));
      break;
    }
    // session notification
    case STREAM_CONTROL_END:
      message_type_e = STREAM_SESSION_MESSAGE_END;
      goto send_session_message;
    case STREAM_CONTROL_LINK:
      message_type_e = STREAM_SESSION_MESSAGE_LINK;
      goto send_session_message;
    case STREAM_CONTROL_RESIZE:
      message_type_e = STREAM_SESSION_MESSAGE_RESIZE;
      goto send_session_message;
    case STREAM_CONTROL_UNLINK:
      message_type_e = STREAM_SESSION_MESSAGE_UNLINK;
      goto send_session_message;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown control (was: %d), returning\n"),
                  inherited::mod_->name (),
                  control_in));
      return;
    }
  } // end SWITCH

  return;

send_session_message:
  // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
  //         condition when the connection is close()d asynchronously
  //         --> see below: line 2015
  bool release_lock = false;
  if (unlikely (!hasReentrantSynchronousSubDownstream_))
  { ACE_ASSERT (streamLock_);
    // *NOTE*: prevent potential deadlocks here; in 'busy' scenarios (i.e. high
    //         contention for message buffers/queue slots), a thread may be
    //         holding
    try {
      release_lock = streamLock_->lock (true,  // block ?
                                        true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF

  SessionDataContainerType* session_data_container_p =
    inherited::sessionData_;
  if (likely (session_data_container_p))
    session_data_container_p->increase ();

  if (unlikely (!inherited::putSessionMessage (message_type_e,
                                               session_data_container_p,
                                               (streamState_ ? streamState_->userData : NULL))))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::name (),
                message_type_e));

  // clean up
  if (unlikely (release_lock))
  { ACE_ASSERT (streamLock_);
    try {
      streamLock_->unlock (false, // unlock ?
                           true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::notify (SessionEventType notification_in,
                                                   bool forwardUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::notify"));

  ACE_UNUSED_ARG (forwardUpStream_in);

  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
      //         condition when the connection is close()d asynchronously
      //         --> see below: line 2015
      bool release_lock = false;
      if (unlikely (!hasReentrantSynchronousSubDownstream_))
      { ACE_ASSERT (streamLock_);
        try {
          release_lock = streamLock_->lock (true,  // block ?
                                            true); // forward upstream (if any) ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,trues), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF

      SessionDataContainerType* session_data_container_p =
        inherited::sessionData_;
      if (likely (session_data_container_p))
      {
        session_data_container_p->increase ();
        SessionDataType& session_data_r =
          const_cast<SessionDataType&> (session_data_container_p->getR ());
        ACE_ASSERT (session_data_r.lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock);
          session_data_r.aborted = true;
        } // end lock scope
        session_data_container_p->decrease (); session_data_container_p = NULL;
      } // end IF

      if (unlikely (release_lock))
      { ACE_ASSERT (streamLock_);
        try {
          streamLock_->unlock (false, // unlock ?
                               true); // forward upstream (if any) ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF
      // *WARNING*: falls through
    }
    default:
    {
      // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
      //         condition when the connection is close()d asynchronously
      //         --> see below: line 2015
      bool release_lock = false;
      if (unlikely (!hasReentrantSynchronousSubDownstream_))
      { ACE_ASSERT (streamLock_);
        try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF

      SessionDataContainerType* session_data_container_p =
        inherited::sessionData_;
      if (likely (session_data_container_p))
        session_data_container_p->increase ();
      // *NOTE*: "fire-and-forget" the second argument
      if (unlikely (!inherited::putSessionMessage (static_cast<enum Stream_SessionMessageType> (notification_in),
                                                   session_data_container_p,
                                                   (streamState_ ? streamState_->userData : NULL))))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                    inherited::name (),
                    notification_in));
      ACE_ASSERT (!session_data_container_p);

      if (unlikely (release_lock))
      { ACE_ASSERT (streamLock_);
        try {
          streamLock_->unlock (false, // unlock ?
                               true); // forward upstream (if any) ?

        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF

      break;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::stop (bool wait_in,
                                                 bool recurseUpstream_in,
                                                 bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (recurseUpstream_in);
  ACE_UNUSED_ARG (lockedAccess_in);

  inherited2::change (STREAM_STATE_STOPPED);

  if (wait_in)
    wait (true,   // wait for worker thread(s) ?
          false,  // N/A
          false); // N/A
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  enum Stream_StateMachine_ControlState status = inherited2::current ();

  return ((status == STREAM_STATE_PAUSED) || (status == STREAM_STATE_RUNNING));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::onLink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onLink"));

  typename inherited::ISTREAM_T* istream_p =
      const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
  ACE_ASSERT (istream_p);

  Stream_ILinkCB* ilink_p = dynamic_cast<Stream_ILinkCB*> (istream_p);
  if (unlikely (!ilink_p))
    return;

  try {
    ilink_p->onLink ();
  } catch (...) {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: caught exception in Stream_ILinkCB::onLink(), continuing\n"),
                ACE_TEXT (istream_p->name ().c_str ()),
                inherited::mod_->name ()));
  }
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::onUnlink ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onUnlink"));

  typename inherited::ISTREAM_T* istream_p =
      const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
  ACE_ASSERT (istream_p);

  Stream_ILinkCB* ilink_p = dynamic_cast<Stream_ILinkCB*> (istream_p);
  if (unlikely (!ilink_p))
    return;

  try {
    ilink_p->onUnlink ();
  } catch (...) {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: caught exception in Stream_ILinkCB::onUnlink(), continuing\n"),
                ACE_TEXT (istream_p->name ().c_str ()),
                inherited::mod_->name ()));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::lock (bool block_in,
                                                 bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::lock"));

  ACE_UNUSED_ARG (forwardUpstream_in);

  int result = -1;
  ACE_SYNCH_MUTEX_T& lock_r = inherited::queue_.lock ();

  result = (block_in ? lock_r.acquire () : lock_r.tryacquire ());
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error == EBUSY)
      return false;
  } // end IF

  return true;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::unlock (bool unlock_in,
                                                   bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::unlock"));

  ACE_UNUSED_ARG (forwardUpstream_in);

  int result = -1;
  ACE_SYNCH_MUTEX_T& lock_r = inherited::queue_.lock ();
  ACE_thread_mutex_t& mutex_r = lock_r.lock ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!ACE_OS::thr_equal (reinterpret_cast<ACE_thread_t> (mutex_r.OwningThread),
                                    ACE_OS::thr_self ())))
#else
  if (unlikely (!ACE_OS::thr_equal (static_cast<ACE_thread_t> (mutex_r.__data.__owner),
                                    ACE_OS::thr_self ())))
#endif // ACE_WIN32 || ACE_WIN64
    return -1;

  do
  {
    result = lock_r.release ();
    if (!unlock_in)
      break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  } while (mutex_r.RecursionCount > 0);
#else
  } while (mutex_r.__data.__count > 0);
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n"),
                inherited::mod_->name ()));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return mutex_r.RecursionCount;
#else
  return mutex_r.__data.__count;
#endif // ACE_WIN32 || ACE_WIN64
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::wait (bool waitForThreads_in,
                                                 bool waitForUpStream_in,
                                                 bool waitForDownStream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::wait"));

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
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (inherited::lock_);

  // *NOTE*: be sure to release the (up-)stream lock to support 'concurrent'
  //         scenarios (e.g. scenarios where upstream delivers data)
  int previous_nesting_level = -1;

  if (streamLock_)
  {
    try {
      previous_nesting_level = streamLock_->unlock (true,
                                                    waitForUpStream_in);
    } catch (...) {
      typename inherited::ISTREAM_T* istream_p =
          const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
      ACE_ASSERT (istream_p);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: caught exception in Stream_ILock_T::unlock(true), continuing\n"),
                  ACE_TEXT (istream_p->name ().c_str ()),
                  inherited::mod_->name ()));
    }
  } // end IF

  // step1: wait for final state
  inherited2::wait (STREAM_STATE_FINISHED,
                    NULL); // <-- block

  // step2: wait for worker(s) to join ?
  if (!waitForThreads_in)
  {
    if (previous_nesting_level > 0)
      STREAM_ILOCK_ACQUIRE_N (streamLock_,
                              previous_nesting_level,
                              waitForUpStream_in);

    return;
  } // end IF

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

  // *NOTE*: pthread_join() returns EDEADLK when the calling thread IS the
  //         thread to join
  //         --> prevent this by comparing thread ids
  // *TODO*: check the whole array
  if (unlikely (inherited::threads_.empty () ||
                ACE_OS::thr_equal (ACE_OS::thr_self (),
                                   inherited::threads_[0].id ())))
    goto continue_;

  switch (concurrency_)
  {
    case STREAM_HEADMODULECONCURRENCY_ACTIVE:
    {
      { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock);
        inherited::wait ();
      } // end lock scope
      break;
    }
    case STREAM_HEADMODULECONCURRENCY_PASSIVE:
    {
      ACE_thread_t thread_id = inherited::threads_[0].id ();
      ACE_THR_FUNC_RETURN status;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_hthread_t handle = inherited::threads_[0].handle ();
      if (likely (handle != ACE_INVALID_HANDLE))
      {
        { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock);
          result = ACE_Thread::join (handle, &status);
        } // end lock scope
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Thread::join(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        else if (likely (result == 0))
        {
          // *NOTE*: successful join()s close the thread handle
          //         (see OS_NS_Thread.inl:2971)
          this_p->inherited::threads_[0].handle (ACE_INVALID_HANDLE);
        } // end IF
        this_p->inherited::threads_[0].id (std::numeric_limits<DWORD>::max ());
      } // end IF
      else
        result = 0;
#else
      if (likely (static_cast<int> (thread_id) != -1))
      {
        { ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard_2, reverse_lock);
          result = ACE_Thread::join (thread_id, NULL, &status);
        } // end lock scope
        this_p->inherited::threads_[0].id (-1);
      } // end IF
      else
        result = 0;
#endif // ACE_WIN32 || ACE_WIN64
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Thread::join(%lu): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    thread_id));
      break;
    }
    default:
      break;
  } // end SWITCH

continue_:
  if (previous_nesting_level > 0)
    STREAM_ILOCK_ACQUIRE_N (streamLock_,
                            previous_nesting_level,
                            waitForUpStream_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::putStatisticMessage (const StatisticContainerType& statisticData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putStatisticMessage"));

  bool result = false;

  // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
  //         condition when the connection is close()d asynchronously
  //         --> see below: line 2015
  bool release_lock = false;
  if (unlikely (!hasReentrantSynchronousSubDownstream_))
  { ACE_ASSERT (streamLock_);
    try {
      release_lock = streamLock_->lock (true,  // block ?
                                        true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true), continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF

  SessionDataContainerType* session_data_container_p =
    inherited::sessionData_;
  if (likely (session_data_container_p))
  {
    // step0: prepare session data object container
    session_data_container_p->increase ();

    // step1: update session state
    SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->getR ());
    // *TODO*: remove type inferences
    session_data_r.statistic = statisticData_in;

    // *TODO*: attach stream state information to the session data
  } // end IF

  // step3: send the statistic data downstream
  // *NOTE*: "fire-and-forget" the second argument
  // *TODO*: remove type inference
  result =
      inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STATISTIC,
                                    session_data_container_p,
                                    (streamState_ ? streamState_->userData : NULL));
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), aborting\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STATISTIC));
  ACE_ASSERT (!session_data_container_p);

  if (unlikely (release_lock))
  { ACE_ASSERT (streamLock_);
    try {
      streamLock_->unlock (false, // unlock ?
                           true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            UserDataType>::onChange (enum Stream_StateMachine_ControlState newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onChange"));

  // sanity check(s)
  ACE_ASSERT (inherited2::stateLock_);

  int result = -1;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T> reverse_lock (*inherited2::stateLock_);

  switch (newState_in)
  {
    case STREAM_STATE_INITIALIZED:
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
      sessionEndSent_ = false;
      sessionEndProcessed_ = false;

      // --> re-initialize ?
      if (unlikely (!inherited::threads_.empty ()))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_hthread_t handle = inherited::threads_[0].handle ();
        if (handle != ACE_INVALID_HANDLE)
          if (!::CloseHandle (handle))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        handle,
                        ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
        inherited::threads_.clear ();
      } // end IF
      break;
    }
    case STREAM_STATE_RUNNING:
    {
      // *NOTE*: implement tape-recorder logic:
      //         transition PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      //         --> check for this condition before doing anything else...
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_);
        if (unlikely (inherited2::state_ == STREAM_STATE_PAUSED))
        {
          // resume worker ?
          switch (concurrency_)
          {
            case STREAM_HEADMODULECONCURRENCY_ACTIVE:
            {
              result = inherited::resume ();
              if (result == -1)
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to ACE_Task::resume(): \"%m\", continuing\n"),
                            inherited::mod_->name ()));
              break;
            } // end IF
            case STREAM_HEADMODULECONCURRENCY_PASSIVE:
            {
              // task object not active --> resume the borrowed thread

              ACE_hthread_t handle;
              { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
                handle = inherited::threads_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
                ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
                result = ACE_Thread::resume (handle);
              } // end lock scope
              if (result == -1)
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to ACE_Thread::resume(): \"%m\", continuing\n"),
                            inherited::mod_->name ()));

              break;
            }
            default:
              break;
          } // end SWITCH

          break;
        } // end IF
      } // end lock scope

      // send initial session message downstream ?
      if (likely (generateSessionMessages_))
      {
        // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
        //         condition when the connection is close()d asynchronously
        //         --> see below: line 2015
        bool release_lock = false;
        if (unlikely (!hasReentrantSynchronousSubDownstream_))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        SessionDataContainerType* session_data_container_p =
          inherited::sessionData_;
        if (likely (session_data_container_p))
          session_data_container_p->increase ();
        else if (streamState_)
        {
          if (streamState_->sessionData)
            ACE_NEW_NORETURN (session_data_container_p,
                              SessionDataContainerType (streamState_->sessionData));
        } // end ELSE IF
        // *NOTE*: "fire-and-forget" the second argument
        if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_BEGIN,          // session message type
                                                     session_data_container_p,              // session data
                                                     (streamState_ ? streamState_->userData
                                                                   : NULL))))               // user data handle
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(STREAM_SESSION_MESSAGE_BEGIN), continuing\n"),
                      inherited::mod_->name ()));
        ACE_ASSERT (!session_data_container_p);

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF
      } // end IF

      switch (concurrency_)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          // spawn a worker thread
          // *TODO*: rewrite for thread counts > 1 (see also: wait() above)
          ACE_ASSERT (inherited::threadCount_ >= 1);

          ACE_thread_t* thread_ids_p = NULL;
          ACE_NEW_NORETURN (thread_ids_p,
                            ACE_thread_t[inherited::threadCount_]);
          if (unlikely (!thread_ids_p))
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("%s: failed to allocate memory (%u), aborting\n"),
                        inherited::mod_->name (),
                        (sizeof (ACE_thread_t) * inherited::threadCount_)));
            return;
          } // end IF
          ACE_OS::memset (thread_ids_p, 0, sizeof (thread_ids_p));
          ACE_hthread_t* thread_handles_p = NULL;
          ACE_NEW_NORETURN (thread_handles_p,
                            ACE_hthread_t[inherited::threadCount_]);
          if (unlikely (!thread_handles_p))
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("%s: failed to allocate memory (%u), aborting\n"),
                        inherited::mod_->name (),
                        (sizeof (ACE_hthread_t) * inherited::threadCount_)));
            delete [] thread_ids_p; thread_ids_p = NULL;
            return;
          } // end IF
          ACE_OS::memset (thread_handles_p, 0, sizeof (thread_handles_p));
          const char** thread_names_p = NULL;
          ACE_NEW_NORETURN (thread_names_p,
                            const char*[inherited::threadCount_]);
          if (unlikely (!thread_names_p))
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("%s: failed to allocate memory (%u), aborting\n"),
                        inherited::mod_->name (),
                        (sizeof (const char*) * inherited::threadCount_)));
            delete [] thread_ids_p; thread_ids_p = NULL;
            delete [] thread_handles_p; thread_handles_p = NULL;
            return;
          } // end IF
          ACE_OS::memset (thread_names_p, 0, sizeof (thread_names_p));
          char* thread_name_p = NULL;
          std::string buffer;
          std::ostringstream converter;
          for (unsigned int i = 0;
               i < inherited::threadCount_;
               ++i)
          {
            thread_name_p = NULL;
            ACE_NEW_NORETURN (thread_name_p,
                              char[BUFSIZ]);
            if (unlikely (!thread_name_p))
            {
              ACE_DEBUG ((LM_CRITICAL,
                          ACE_TEXT ("%s: failed to allocate memory (%u), aborting\n"),
                          inherited::mod_->name (),
                          (sizeof (char) * BUFSIZ)));
              delete [] thread_ids_p; thread_ids_p = NULL;
              delete [] thread_handles_p; thread_handles_p = NULL;
              for (unsigned int j = 0; j < i; j++)
                delete [] thread_names_p[j];
              delete [] thread_names_p; thread_names_p = NULL;
              return;
            } // end IF
            ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
            converter.clear ();
            converter.str (ACE_TEXT_ALWAYS_CHAR (""));
            converter << (i + 1);
            buffer = inherited::threadName_;
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
                                      static_cast<int> (inherited::threadCount_), // # threads
                                      0,                                           // force spawning
                                      ACE_DEFAULT_THREAD_PRIORITY,                 // priority
                                      inherited::grp_id (),                       // group id (see above)
                                      NULL,                                        // corresp. task --> use 'this'
                                      thread_handles_p,                            // thread handle(s)
                                      NULL,                                        // thread stack(s)
                                      NULL,                                        // thread stack size(s)
                                      thread_ids_p,                                // thread id(s)
                                      thread_names_p);                             // thread name(s)
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task_Base::activate(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
            delete[] thread_ids_p; thread_ids_p = NULL;
            delete[] thread_handles_p; thread_handles_p = NULL;
            for (unsigned int i = 0; i < inherited::threadCount_; i++)
              delete[] thread_names_p[i];
            delete[] thread_names_p; thread_names_p = NULL;
            break;
          } // end IF

          std::ostringstream string_stream;
          ACE_Thread_ID thread_id;
          { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
            for (unsigned int i = 0;
                 i < inherited::threadCount_;
                 ++i)
            {
              string_stream << ACE_TEXT_ALWAYS_CHAR ("#") << (i + 1)
                            << ACE_TEXT_ALWAYS_CHAR (" ")
                            << thread_ids_p[i]
                            << ACE_TEXT_ALWAYS_CHAR ("\n");
              delete [] thread_names_p[i];
              thread_id.handle (thread_handles_p[i]);
              thread_id.id (thread_ids_p[i]);
              inherited::threads_.push_back (thread_id);
            } // end FOR
          } // end lock scope
          std::string thread_ids_string = string_stream.str ();
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s/%s spawned %u worker thread(s) (group: %d):\n%s"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::threadName_.c_str ()),
                      inherited::threadCount_,
                      inherited::grp_id (),
                      ACE_TEXT (thread_ids_string.c_str ())));
          delete [] thread_ids_p; thread_ids_p = NULL;
          delete [] thread_handles_p; thread_handles_p = NULL;
          delete [] thread_names_p; thread_names_p = NULL;
          break;
        }
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        {
          // *NOTE*: if the object is 'passive', the whole operation pertaining
          //         to newState_in is processed 'inline' by the calling thread,
          //         i.e. would complete 'before' the state has transitioned to
          //         'running'
          //         --> set the state early
          { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_);
            inherited2::state_ = STREAM_STATE_RUNNING;
          } // end lock scope

          { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, inherited::lock_);
            // sanity check(s)
            ACE_ASSERT (inherited::threads_.empty ());

            ACE_Thread_ID thread_id;
            thread_id.id (ACE_Thread::self ());
            ACE_hthread_t handle = ACE_INVALID_HANDLE;
            ACE_Thread::self (handle);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            HANDLE process_handle = ::GetCurrentProcess ();
            if (unlikely (!::DuplicateHandle (process_handle,
                                              handle,
                                              process_handle,
                                              &handle,
                                              0,
                                              FALSE,
                                              DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS)))
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to DuplicateHandle(0x%@): \"%s\", continuing\n"),
                          inherited::mod_->name (),
                          handle,
                          ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
            thread_id.handle (handle);
            inherited::threads_.push_back (thread_id);
          } // end lock scope

          result = svc ();
          if (unlikely (result == -1)) // *NOTE*: most probable reason: session aborted
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task_Base::svc(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          result = close (0);
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task_Base::close(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));

          break;
        }
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
        {
          // *IMPORTANT NOTE*: this means that there is no worker thread
          //                   driving this module (neither in-line, nor
          //                   dedicated, svc() is not used); data is processed
          //                   in-line in put() by dispatching threads

          // *NOTE*: if any of the modules failed to initialize, signal the
          //         controller

          // *NOTE*: in 'concurrent' (e.g. server-side-)scenarios there is a race
          //         condition when the connection is close()d asynchronously
          //         --> see below: line 2015
          bool release_lock = false;
          if (streamLock_)
          {
            try {
              release_lock = streamLock_->lock (true,  // block ?
                                                true); // forward upstream (if any) ?
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                          inherited::mod_->name ()));
            }
          } // end IF

          SessionDataContainerType* session_data_container_p =
            inherited::sessionData_;
          if (likely (session_data_container_p))
          {
            session_data_container_p->increase ();

            // *TODO*: remove type inferences
            SessionDataType& session_data_r =
                const_cast<SessionDataType&> (session_data_container_p->getR ());
            bool finish_b = false;
            { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, *session_data_r.lock);
              finish_b = session_data_r.aborted;
            } // end lock scope
            if (unlikely (finish_b))
              inherited2::finished ();

            session_data_container_p->decrease ();
          } // end IF

          if (release_lock)
          { ACE_ASSERT (streamLock_);
            try {
              streamLock_->unlock (false, // unlock ?
                                   true); // forward upstream (if any) ?
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                          inherited::mod_->name ()));
            }
          } // end IF

          break;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      // suspend the worker(s) ?
      switch (concurrency_)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          result = inherited::suspend ();
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::suspend(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));

          break;
        } // end IF
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, inherited::lock_);
          // task object not active --> suspend the borrowed thread
          ACE_hthread_t handle = inherited::threads_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
          ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
          result = ACE_Thread::suspend (handle);
          if (unlikely (result == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Thread::suspend(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));

          break;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_STOPPED:
    {
      bool done = false;

      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_);
        switch (inherited2::state_)
        {
          case STREAM_STATE_PAUSED:
          {
            // resume worker ?
            switch (concurrency_)
            {
              case STREAM_HEADMODULECONCURRENCY_ACTIVE:
              {
                result = inherited::resume ();
                if (unlikely (result == -1))
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: failed to ACE_Task::resume(): \"%m\", continuing\n"),
                              inherited::mod_->name ()));

                break;
              } // end IF
              case STREAM_HEADMODULECONCURRENCY_PASSIVE:
              { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, inherited::lock_);
                // task is not 'active' --> resume the calling thread (i.e. the
                // thread that invoked start())
                ACE_hthread_t handle = inherited::threads_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
                ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
                result = ACE_Thread::resume (handle);
                if (unlikely (result == -1))
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: failed to ACE_Thread::resume(): \"%m\", continuing\n"),
                              inherited::mod_->name ()));

                break;
              }
              default:
                break;
            } // end SWITCH

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
        if (unlikely (done))
          break;

        // *NOTE*: in 'passive' mode the finished() method waits for the stream
        //         --> set the (intermediate) state early
        inherited2::state_ = STREAM_STATE_STOPPED;
      } // end lock scope

      switch (concurrency_)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          inherited::stop (false, // wait ?
                           true); // N/A
          break;
        }
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, inherited::lock_);
          ACE_ASSERT (!inherited::threads_.empty ());
          if (!ACE_OS::thr_equal (ACE_OS::thr_self (),
                                  inherited::threads_[0].id ()))
            inherited::stop (false, // wait ?
                             true); // N/A
          //// signal the controller
          //inherited2::finished ();
          break;
        }
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
        {
          // signal the controller
          inherited2::finished ();
          break;
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_);
        // *NOTE*: modules processing the final session message (see below) may
        //         (indirectly) invoke wait() on the stream,
        //         which would deadlock if the implementation is 'passive'
        //         --> set the state early
        // *TODO*: this may not be the best way to handle that case (i.e. it
        //         could introduce other race conditions...)
        inherited2::state_ = STREAM_STATE_FINISHED;
      } // end lock scope

      bool release_lock = false;
      SessionDataContainerType* session_data_container_p = NULL;

      // unlink downstream if necessary
      if (unlikely (inherited::linked_))
      {
        ISTREAM_CONTROL_T* istream_control_p = NULL;
        typename inherited::ISTREAM_T* istream_p =
            const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
        ACE_ASSERT (istream_p);
        istream_p =
          dynamic_cast<typename inherited::ISTREAM_T*> (istream_p->downstream ());
        if (!istream_p)
          goto continue_;

        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s/%s: stream has ended, unlinking downstream\n"),
        //            ACE_TEXT (inherited::stream_->name ().c_str ()),
        //            inherited::mod_->name ()));

        try {
          istream_p->_unlink ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_IStream_T::_unlink(), continuing\n"),
                      ACE_TEXT (istream_p->name ().c_str ())));
        }

        // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
        //         condition when the connection is close()d asynchronously
        //         --> see above: line 2015
        if (unlikely (!hasReentrantSynchronousSubDownstream_))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        // *NOTE*: 'downstream' has been unlinked; notify 'upstream' (i.e.
        //         'this') about this fact as well
        session_data_container_p = inherited::sessionData_;
        if (session_data_container_p)
          session_data_container_p->increase ();
        // *NOTE*: "fire-and-forget" the second argument
        if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_UNLINK,                    // session message type
                                                     session_data_container_p,                         // session data
                                                     (streamState_ ? streamState_->userData : NULL)))) // user data handle
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(STREAM_SESSION_MESSAGE_BEGIN), continuing\n"),
                      inherited::mod_->name ()));
        ACE_ASSERT (!session_data_container_p);

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        istream_control_p = dynamic_cast<ISTREAM_CONTROL_T*> (istream_p);
        ACE_ASSERT (istream_control_p);
        try {
          istream_control_p->control (STREAM_CONTROL_END,
                                      false);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_IStreamControl_T::control(STREAM_CONTROL_END), continuing\n"),
                      ACE_TEXT (istream_p->name ().c_str ())));
        }
      } // end IF

continue_:
      // send final session message downstream ?
      // *IMPORTANT NOTE*: the transition STOPPED --> FINISHED is automatic (see
      //                   above [*NOTE*: in 'active'/svc() based scenarios,
      //                   shutdown() triggers this transition]).
      //                   However, as the stream may be stop()/finished()-ed
      //                   concurrently (e.g. (safety/sanity) precaution is
      //                   required during shutdown, connection reset, ...),
      //                   this transition could trigger several times
      //                   --> ensure that only a single 'session end' message
      //                       is generated and processed per session
      bool send_end_message = false;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        if (!sessionEndSent_ &&
            !sessionEndProcessed_) // already processed upstream 'session end' ?
        {
          sessionEndSent_ = true;
          send_end_message = true;
        } // end IF
      } // end lock scope
      if (likely (generateSessionMessages_ &&
                  send_end_message))
      {
        session_data_container_p = inherited::sessionData_;
        if (likely (session_data_container_p))
          session_data_container_p->increase ();

        release_lock = false;
        if (unlikely (!hasReentrantSynchronousSubDownstream_))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock =
                streamLock_->lock (true,  // block ?
                                   true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        // *NOTE*: "fire-and-forget" the second argument
        if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_END,                       // session message type
                                                     session_data_container_p,                         // session data
                                                     (streamState_ ? streamState_->userData : NULL)))) // user data handle
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(STREAM_SESSION_MESSAGE_END), continuing\n"),
                      inherited::mod_->name ()));
        ACE_ASSERT (!session_data_container_p);

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid state transition: \"%s\" --> \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited2::stateToString (inherited2::state_).c_str ()),
                  ACE_TEXT (inherited2::stateToString (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}
