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

#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            TimerManagerType,
 #if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_Module_QueueReader_T (ISTREAM_T* stream_in)
#else
                            UserDataType>::Stream_Module_QueueReader_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in) // stream handle
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::Stream_Module_QueueReader_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            TimerManagerType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::initialize"));

  if (inherited::isInitialized_)
  { ACE_ASSERT (inherited::msg_queue_);
    inherited::msg_queue_->flush ();
  } // end IF

  MESSAGE_QUEUE_BASE_T* queue_p =
    static_cast<MESSAGE_QUEUE_BASE_T*> (configuration_in.queue);
  if (unlikely (!queue_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no queue provided, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  inherited::msg_queue (queue_p);

  int result = inherited::msg_queue_->activate ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            TimerManagerType,
                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      inherited::change (STREAM_STATE_SESSION_STOPPING);
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    { ACE_ASSERT (inherited::sessionData_);

      // schedule regular statistic collection ?
      // *NOTE*: the runtime-statistic module is responsible for regular
      //         reporting, the head module merely collects information
      // *TODO*: remove type inference
      if (inherited::configuration_->statisticReportingInterval != ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_), // event handler
                                            NULL,                            // asynchronous completion token
                                            COMMON_TIME_NOW + interval,      // first wakeup time
                                            interval);                       // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", returning\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    inherited::timerId_,
                    &interval));
      } // end IF

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (unlikely (inherited::sessionEndProcessed_))
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (inherited::timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer() (id was: %d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
//        if (unlikely (act_p))
//        {
//          delete act_p; act_p = NULL;
//        } // end IF
      } // end IF

      // *NOTE*: cannot deactivate() queue because then stop() (see below) would fail !
      //result = inherited::msg_queue_->deactivate ();
      //if (unlikely (result == -1))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
      //              inherited::mod_->name ()));

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      { Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,  // wait for completion ?
                       false); // high priority ?
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType,
                            TimerManagerType,
                            UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_QueueReader_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  if (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
    Common_Error_Tools::setThreadName (inherited::threadName_,
                                       NULL);
#else
    Common_Error_Tools::setThreadName (inherited::threadName_,
                                       0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT ("")),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? inherited::grp_id_
                                                                                             : -1)));

  ACE_Message_Block* message_block_p = NULL;
  bool release_lock_b = false;
  int result_i = 0;
  SessionDataContainerType* session_data_container_p = inherited::sessionData_;
  const SessionDataType* session_data_p = &inherited::sessionData_->getR ();
  bool stop_processing_b = false;
  bool done_b = false;
  bool finish_b = true;
  bool aborted_b = false;

  do
  {
    message_block_p = NULL;
    result_i = inherited::getq (message_block_p, NULL);
    if (likely (result_i == -1))
    { int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      else // <-- queue has been deactivate()d
      { ACE_ASSERT (inherited::msg_queue_->state () == ACE_Message_Queue_Base::DEACTIVATED);
        result_i = 0;

        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (unlikely (inherited::current () != STREAM_STATE_FINISHED))
          {
            // need to reactivate the queue
            result_i = inherited::msg_queue_->activate ();
            if (unlikely (result_i == -1))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                          inherited::mod_->name ()));
              return -1;
            } // end IF

            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?
            continue;
          } // end IF
        } // end lock scope
      } // end ELSE
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        if (unlikely (inherited::isHighPriorityStop_))
        {
          if (likely (!inherited::abortSent_))
            this->control (STREAM_CONTROL_ABORT,
                           false); // forward upstream ?
        } // end IF

        // *IMPORTANT NOTE*: when close()d manually (i.e. on a user abort),
        //                   the stream may not have finish()ed
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (unlikely (inherited::sessionEndSent_ || inherited::sessionEndProcessed_))
            finish_b = false;
        } // end lock scope
        if (likely (finish_b))
        {
          // enqueue(/process) STREAM_SESSION_END
          inherited::finished (false); // recurse upstream ?
          message_block_p->release (); message_block_p = NULL;
          continue;
        } // end IF

        // *NOTE*: this is racy; the penultimate thread may have left svc() and
        //         not have decremented thr_count_ yet. In this case, the
        //         stop-message might remain in the queue during shutdown (or,
        //         even worse-) during re-initialization...
        // *TODO*: ward against this scenario
        if (unlikely (inherited::thr_count_ > 1))
        {
          result_i =
            (inherited::isHighPriorityStop_ ? inherited::ungetq (message_block_p, NULL)
                                            : inherited::putq (message_block_p, NULL));
          if (unlikely (result_i == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_block_p->release (); message_block_p = NULL;
            return -1;
          } // end IF
        } // end IF
        else
        {
          message_block_p->release (); message_block_p = NULL;
        } // end ELSE
        inherited::isHighPriorityStop_ = false;

        // --> SESSION_END has been processed; leave
        done_b = true;
        break;
      }
      default:
      {
        // grab stream lock if processing must be synchronous
        // *TODO*: there must be a better way to handle this case without this
        //         overhead (e.g. specialized subclass, ...)
        if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
        { ACE_ASSERT (inherited::streamLock_);
          try {
            release_lock_b =
              inherited::streamLock_->lock (true,  // block ?
                                            true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), aborting\n"),
                        inherited::mod_->name ()));
            return -1;
          }
        } // end IF

        inherited::handleMessage (message_block_p,
                                  stop_processing_b);

        if (unlikely (release_lock_b))
        { ACE_ASSERT (inherited::streamLock_);
          try {
            inherited::streamLock_->unlock (false, // unlock ?
                                            true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), aborting\n"),
                        inherited::mod_->name ()));
            return -1;
          }
        } // end IF

        // *IMPORTANT NOTE*: as the session data may change when this stream is
        //                   (un-)link()ed (e.g. inbound network data
        //                   processing), the handle may have to be updated
        if (unlikely (session_data_container_p != inherited::sessionData_))
        {
          session_data_container_p = inherited::sessionData_;
          session_data_p =
            (inherited::sessionData_ ? &inherited::sessionData_->getR ()
                                     : NULL);
          if (session_data_p)
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: updated session data\n"),
                        inherited::mod_->name ()));
        } // end IF

        if (unlikely (stop_processing_b)) // <-- SESSION_END has been processed || serious error
        { stop_processing_b = false; // reset, just in case...
          finish_b = true;
          {
            ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
            if (unlikely (inherited::sessionEndSent_ || inherited::sessionEndProcessed_))
              finish_b = false;
          } // end lock scope
          if (likely (finish_b))
          {
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?
            continue;
          } // end IF
        }   // end IF

        break;
      }
    } // end SWITCH
    // sanity check(s)
    if (unlikely (done_b))
      break;

    // session aborted ?
    // *TODO*: remove type inferences
    if (likely (session_data_p))
    { ACE_ASSERT (session_data_p->lock);
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_p->lock, -1);
      // *TODO*: remove type inferences
      aborted_b = session_data_p->aborted;
      if (unlikely (aborted_b))
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session (id was: %u) aborted\n"),
                    inherited::mod_->name (),
                    session_data_p->sessionId));
    } // end IF/lock scope
    if (unlikely (aborted_b))
    {
      finish_b = true;
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        if (unlikely (inherited::sessionEndSent_ || inherited::sessionEndProcessed_))
          finish_b = false;
      } // end lock scope
      if (likely (finish_b))
      {
        // enqueue(/process) STREAM_SESSION_END
        inherited::finished (false); // recurse upstream ?
        continue;
      } // end IF
    } // end IF
  } while (true);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t) leaving\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT (""))));

  // reset the queue
  //inherited::msg_queue (NULL);

  // *TODO*: in passive mode (!), signal a condition here to unblock any
  //         thread(s) in stop(true) (see above) before returning
  return 0;
}
