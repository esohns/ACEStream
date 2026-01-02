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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_HeadModuleTaskBase_T (ISTREAM_T* stream_in)
#else
                            UserDataType>::Stream_HeadModuleTaskBase_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in,
              NULL) // queue handle (see below)
 , inherited2 (NULL)
 , abortSent_ (false)
 , endSeenFromUpstream_ (false)
 , isHighPriorityStop_ (false)
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // maximum #slots
           NULL)                   // notification handle
 , sessionEndProcessed_ (false)
 , sessionEndSent_ (false)
 , stateMachineLock_ (NULL, // name
                      NULL) // attributes
 , streamId_ (stream_in->id ())
 , streamLock_ (stream_in)
 , streamState_ (NULL)
 , statistic_ ()
 , statisticHandler_ (COMMON_STATISTIC_ACTION_COLLECT, // handler action
                      this,                            // interface handle
                      false)                           // report on collect ?
 , timerId_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  inherited::msg_queue (&queue_);

  if (unlikely (!inherited2::initialize (stateMachineLock_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_StateMachine_Base_T::initialize(), returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  ACE_ASSERT (stream_in);
  ISTREAM_CONTROL_T* istream_control_p =
    dynamic_cast<ISTREAM_CONTROL_T*> (stream_in);
  ACE_ASSERT (istream_control_p);
  streamState_ = &const_cast<StreamStateType&> (istream_control_p->state ());
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
//    if (unlikely (act_p))
//    {
//      delete act_p; act_p = NULL;
//    } // end IF
  } // end IF

  if (unlikely (isRunning ()))
  {
    stop (true,  // wait ?
          false, // recurse ?
          true); // high priority ?
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: stream still active in dtor, continuing\n"),
                inherited::mod_->name ()));
  } // end IF

  inherited::msg_queue (NULL);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // use the queue, if necessary
  switch (inherited::configuration_->concurrency)
  {
    case STREAM_HEADMODULECONCURRENCY_ACTIVE:
    case STREAM_HEADMODULECONCURRENCY_PASSIVE:
    {
      switch (messageBlock_in->msg_type ())
      {
        case ACE_Message_Block::MB_STOP:
        {
          if (likely (!isHighPriorityStop_))
            break;

          // sanity check(s)
          ACE_ASSERT (inherited::msg_queue_);

          // *IMPORTANT NOTE*: make sure the message is actually processed
          { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::msg_queue_->lock (), -1);
            if (likely (!inherited::threadIds_.empty ()))
            {
              Stream_IMessageQueue* imessage_queue_p =
                dynamic_cast<Stream_IMessageQueue*> (inherited::msg_queue_);
              ACE_ASSERT (imessage_queue_p);
              return imessage_queue_p->enqueue_head_i (messageBlock_in,
                                                       NULL);
            } // end IF
          } // end lock scope
          isHighPriorityStop_ = false;

          // *IMPORTANT NOTE*: it is either too early or too late to process
          //                   this message by this (and (!) subsequent
          //                   synchronous downstream-) task(s)
          //                   --> do it manually
          control (STREAM_CONTROL_ABORT,
                   false); // forward upstream ?
          messageBlock_in->release ();
          return 0;
        }
        case STREAM_MESSAGE_CONTROL:
        {
          ControlMessageType* message_p =
            static_cast<ControlMessageType*> (messageBlock_in);

          switch (message_p->type ())
          {
            case STREAM_CONTROL_MESSAGE_ABORT:
            { // *IMPORTANT NOTE*: try to ensure the message is actually processed
              bool enqueue_b = false; // fallback
              { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::msg_queue_->lock (), -1);
                // *NOTE*: cannot just use queue_, because msg_queue_ may not
                //         be the same as queue_ (e.g. when 'this' is a 'queue
                //         source')
                typename inherited::MESSAGE_QUEUE_T* queue_p =
                    dynamic_cast<typename inherited::MESSAGE_QUEUE_T*> (inherited::msg_queue_);
                if (likely (inherited::thr_count_      &&
                            queue_p                    &&
                            !queue_p->isShuttingDown ()))
                  return queue_p->enqueue_head_i (messageBlock_in, NULL);
                else if (!queue_p)
                  enqueue_b = true; // --> try fallback
              } // end lock scope
              if (unlikely (enqueue_b)) // *WARNING*: race condition here
                return inherited::msg_queue_->enqueue_head (messageBlock_in, NULL);

              // *IMPORTANT NOTE*: it is either too early or too late to process
              //                   this message by this (and (!) subsequent
              //                   synchronous downstream-) task(s)
              //                   --> process it 'inline'
              bool stop_processing = false;
              inherited::handleMessage (messageBlock_in,
                                        stop_processing);
              return 0;
            }
            default:
              break;
          } // end SWITCH

          break;
        }
        case STREAM_MESSAGE_SESSION:
        {
          SessionMessageType* message_p =
            static_cast<SessionMessageType*> (messageBlock_in);
          if (unlikely (message_p->expedited ()))
            return inherited::msg_queue_->enqueue_head (messageBlock_in, NULL);
          break;
        }
        default:
          break;
      } // end SWITCH

      result = inherited::putq (messageBlock_in, timeout_in);
      if (unlikely (result == -1))
      {
        // *NOTE*: most probable reason: link()ed stop(); data arriving after
        //         STREAM_SESSION_END
        int error_i = ACE_OS::last_error ();
        if (error_i != ESHUTDOWN)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
      } // end IF
      return result;
    }
    case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown concurrency type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->concurrency));
      return -1;
    }
  } // end SWITCH

  // --> process 'in-line'

  bool release_lock = false;
  if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
  { ACE_ASSERT (streamLock_);
    try {
      release_lock = streamLock_->lock (true,  // block ?
                                        true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return -1;
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
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return -1;
    }
  } // end IF

  bool is_error_state_b = false;
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    is_error_state_b = stop_processing && !sessionEndProcessed_;
  } // end lock scope
  if (unlikely (is_error_state_b))
    finished (false); // recurse upstream ?

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::open (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::open"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::msg_queue_);
  ACE_UNUSED_ARG (arg_in);

  // step2: initialize the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state.
  //         The second time around (i.e. the stream has been stopped/started,
  //         the queue will have been deactivate()d in the process, and getq()
  //         (see svc()) would fail (ESHUTDOWN)
  //         --> (re-)activate() the queue
  result = inherited::msg_queue_->activate ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  // *NOTE*: being implicitly invoked through ACE_Stream::push()
  //         --> don't do anything, unless auto-starting
  if (unlikely (inherited::configuration_->autoStart))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: auto-starting...\n"),
                inherited::mod_->name ()));

    // initialize 'this'
    SessionManagerType* session_manager_p =
      SessionManagerType::SINGLETON_T::instance ();
    ACE_ASSERT (session_manager_p);
    typename SessionMessageType::DATA_T::DATA_T* session_data_p =
      &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (streamId_));

    ACE_ASSERT (session_data_p->lock);
    inherited::sessionDataLock_ = session_data_p->lock;

    ACE_ASSERT (!inherited::sessionData_);
    ACE_NEW_NORETURN (inherited::sessionData_,
                      typename SessionMessageType::DATA_T (session_data_p,
                                                           false)); // *NOTE*: do NOT delete the session data when the container is destroyed
    if (unlikely (!inherited::sessionData_))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                  inherited::mod_->name ()));
      return -1;
    } // end IF

    try {
      inherited::start (NULL);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Common_TaskBase_T::start(), aborting\n"),
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::close"));

  int result = 0;
  int result_2 = 0;

  // *NOTE*: this method may be invoked
  //         - by external threads shutting down the active object (arg_in: 1)
  //         - by worker thread(s) upon returning from svc() (arg_in: 0)
  switch (arg_in)
  {
    case 0:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

      ACE_thread_t handle = ACE_OS::thr_self ();
      bool is_last_thread_b = false;
      bool free_session_data_b = false;
      switch (inherited::configuration_->concurrency)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          is_last_thread_b = ACE_OS::thr_equal (handle,
                                                inherited::last_thread ());
          free_session_data_b = is_last_thread_b;
          break;
        }
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        {
          is_last_thread_b = true;
          break;
        }
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown concurrency type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->concurrency));
          return -1;
        }
      } // end SWITCH

      if (is_last_thread_b) // last thread ?
      {
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_hthread_t handle_2 = ACE_INVALID_HANDLE;
          if (unlikely (inherited::closeHandles_))
            for (THREAD_IDS_CONSTITERATOR_T iterator = inherited::threadIds_.begin ();
                 iterator != inherited::threadIds_.end ();
                 ++iterator)
            {
              handle_2 = (*iterator).handle ();
              ACE_ASSERT (handle_2 != ACE_INVALID_HANDLE);
              if (!::CloseHandle (handle_2))
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                            handle_2,
                            ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
                result = -1;
              } // end IF
            } // end FOR
#endif // ACE_WIN32 || ACE_WIN64
          inherited::threadIds_.clear ();
        } // end lock scope

        if (unlikely (!inherited::msg_queue_))
          goto continue_;
        // *NOTE*: deactivate the message queue so it does not accept new data
        //         after the last (worker) thread has left
        result_2 = inherited::msg_queue_->deactivate ();
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
          result = -1;
        } // end IF
        result_2 = inherited::msg_queue_->flush ();
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::flush(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
          result = -1;
        } // end IF
        else if (result_2)
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: flushed %d message(s)\n"),
                      inherited::mod_->name (),
                      result_2));

continue_:
        if (free_session_data_b)
        { ACE_ASSERT (inherited::sessionData_);
          inherited::sessionData_->decrease (); inherited::sessionData_ = NULL;
        } // end IF
      } // end IF
      else
      {
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          for (typename inherited::THREAD_IDS_ITERATOR_T iterator = inherited::threadIds_.begin ();
               iterator != inherited::threadIds_.end ();
               ++iterator)
            if ((*iterator).id () == handle)
            {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
              ACE_hthread_t handle_2 = ACE_INVALID_HANDLE;
              handle_2 = (*iterator).handle ();
              ACE_ASSERT (handle_2 != ACE_INVALID_HANDLE);
              if (unlikely (inherited::closeHandles_))
                if (!::CloseHandle (handle_2))
                {
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                              handle_2,
                              ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
                  result = -1;
                } // end IF
#endif // ACE_WIN32 || ACE_WIN64
              inherited::threadIds_.erase (iterator);
              break;
            } // end IF
        } // end lock scope
      } // end ELSE
      break;
    }
    case 1:
    {
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        if (unlikely (inherited::threadIds_.empty ()))
          break; // nothing to do
      } // end lock scope

      Common_ITask* itask_p = this;
      itask_p->stop (false,  // wait for completion ?
                     false); // high priority ?

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid argument (was: %u), aborting\n"),
                  inherited::mod_->name (),
                  arg_in));
      return -1;
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
    stop (true,  // wait ?
          false, // recurse ?
          true); // high priority ?
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  if (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
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
  bool stop_processing_b = false;
  bool done_b = false;

  do
  {
    message_block_p = NULL;
    result_i = inherited::getq (message_block_p, NULL);
    if (unlikely (result_i == -1))
    { int error_i = ACE_OS::last_error ();
      if (unlikely (error_i != ESHUTDOWN))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      else // <-- queue has been deactivate()d
      { ACE_ASSERT (inherited::msg_queue_->state () == ACE_Message_Queue_Base::DEACTIVATED);
        result_i = 0;

        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (unlikely (inherited2::current () != STREAM_STATE_FINISHED))
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
            finished (false); // recurse upstream ?
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
        if (unlikely (isHighPriorityStop_))
        {
          if (likely (!abortSent_))
            control (STREAM_CONTROL_ABORT,
                     false); // forward upstream ?
        } // end IF

        // *IMPORTANT NOTE*: when close()d manually (i.e. on a user abort),
        //                   the stream may not have finish()ed at this point
        bool finish_b = true;
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (sessionEndSent_ || sessionEndProcessed_)
            finish_b = false;
        } // end lock scope
        if (likely (finish_b))
        {
          // enqueue(/process) STREAM_SESSION_END
          finished (false); // recurse upstream ?
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
            (isHighPriorityStop_ ? inherited::ungetq (message_block_p, NULL)
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
        isHighPriorityStop_ = false;

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
        { ACE_ASSERT (streamLock_);
          try {
            release_lock_b = streamLock_->lock (true,  // block ?
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
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), aborting\n"),
                        inherited::mod_->name ()));
            return -1;
          }
        } // end IF

        if (unlikely (stop_processing_b)) // <-- SESSION_END has been processed || finished || serious error
        { stop_processing_b = false; // reset, just in case...
          bool finish_b = true;
          { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
            if (unlikely (sessionEndSent_ || sessionEndProcessed_))
              finish_b = false;
          } // end lock scope
          if (likely (finish_b))
          {
            // enqueue(/process) STREAM_SESSION_END
            finished (false); // recurse upstream ?
            continue;
          } // end IF
        } // end IF

        break;
      }
    } // end SWITCH
    // sanity check(s)
    if (unlikely (done_b))
      break;
  } while (true);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t) leaving\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT (""))));

  // *TODO*: in passive mode (!), signal a condition here to unblock any
  //         thread(s) in stop(true) (see above) before returning
  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::isShuttingDown () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isShuttingDown"));

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);
  ACE_ASSERT (inherited::configuration_);

  switch (inherited::configuration_->concurrency)
  {
    case STREAM_HEADMODULECONCURRENCY_ACTIVE:
    case STREAM_HEADMODULECONCURRENCY_PASSIVE:
      break;
    case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
      return false;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                  inherited::configuration_->concurrency));
      return false; // *WARNING*: false negative !
    }
  } // end SWITCH

  bool result = false;
  ACE_Message_Block* message_block_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::msg_queue_->lock (), false); // *WARNING*: false negative !
    for (typename inherited::MESSAGE_QUEUE_ITERATOR_T iterator (*inherited::msg_queue_);
         iterator.next (message_block_p);
         iterator.advance ())
    { ACE_ASSERT (message_block_p);
      if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
      {
        result = true;
        break;
      } // end IF
      message_block_p = NULL;
    } // end FOR
  } // end lock scope

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_END:
    {
      endSeenFromUpstream_ = true;

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received end from upstream\n"),
                  inherited::mod_->name ()));
      break;
    }
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      unsigned int result = 0; 
      typename inherited::MESSAGE_QUEUE_T* queue_p =
        dynamic_cast<typename inherited::MESSAGE_QUEUE_T*> (inherited::msg_queue_);
      if (likely (queue_p))
        result = queue_p->flush (false); // flush all data messages
      else
      { ACE_ASSERT (false); // *TODO*
        result = inherited::msg_queue_->flush ();
      } // end ELSE
      if (unlikely (result == static_cast<unsigned int> (-1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
      else if (result)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u data messages\n"),
                    inherited::mod_->name (),
                    result));
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::stop (bool waitForCompletion_in,
                                                 bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  // sanity check(s)
  // *NOTE*: inherited::control() ignores highPriority_in, but invokes
  //         this->put(), which re-evaluates isHighPriorityStop_

  inherited::control (ACE_Message_Block::MB_STOP,
                      highPriority_in);

  if (waitForCompletion_in)
    this->wait (true, false, false);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      isHighPriorityStop_ = true;
      inherited2::change (STREAM_STATE_SESSION_STOPPING);
      goto end;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      if (endSeenFromUpstream_ && // <-- there was (!) an upstream
          inherited::configuration_->stopOnUnlink)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: received unlink from upstream, updating state\n"),
                    inherited::mod_->name ()));
        inherited2::change (STREAM_STATE_SESSION_STOPPING);
      } // end IF
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    { ACE_ASSERT (inherited::sessionData_);

      // schedule regular statistic collection ?
      // *NOTE*: the runtime-statistic module is responsible for regular
      //         reporting, the head module merely collects information
      // *TODO*: remove type inference
      if (inherited::configuration_->statisticReportingInterval != ACE_Time_Value::zero)
      { ACE_ASSERT (timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    timerId_,
                    &interval));
      } // end IF

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
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
//        if (unlikely (act_p))
//        {
//          delete act_p; act_p = NULL;
//        } // end IF
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      { Common_ITask* itask_p = this; // *TODO*: is there no other way ?
        itask_p->stop (false,                // wait for completion ?
                       isHighPriorityStop_); // high priority ?
      } // end IF

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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    abortSent_ = false;
    endSeenFromUpstream_ = false;
    isHighPriorityStop_ = false;
    // *NOTE*: sessionEndProcessed_ and sessionEndSent_ are reset in onChange()

    typename inherited::ISTREAM_T* istream_p =
      const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
    ACE_ASSERT (istream_p);
    streamLock_ =  istream_p;
    ISTREAM_CONTROL_T* istream_control_p = dynamic_cast<ISTREAM_CONTROL_T*> (istream_p);
    ACE_ASSERT (istream_control_p);
    streamState_ = &const_cast<StreamStateType&> (istream_control_p->state ());

    if (unlikely (timerId_ != -1))
    { ACE_ASSERT (inherited::configuration_);
      typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                   : TIMER_MANAGER_SINGLETON_T::instance ());
      ACE_ASSERT (itimer_manager_p);
      const void* act_p = NULL;
      result = itimer_manager_p->cancel_timer (timerId_,
                                               &act_p);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer() (id was: %d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    timerId_));
        return false;
      } // end IF
      timerId_ = -1;
//      if (unlikely (act_p))
//      {
//        delete act_p; act_p = NULL;
//      } // end IF
    } // end IF
  } // end IF

  result = inherited::msg_queue_->activate ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::control (StreamControlType control_in,
                                                    bool forwardUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::control"));

  SessionEventType message_type_e = STREAM_SESSION_MESSAGE_INVALID;

  // *NOTE*: support sending control messages even when stream is finished...
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;
  bool release_session_data_container_b = false;
  if (unlikely (!session_data_container_p))
  {
    SessionManagerType* session_manager_p =
      SessionManagerType::SINGLETON_T::instance ();
    ACE_ASSERT (session_manager_p);
    typename SessionMessageType::DATA_T::DATA_T* session_data_p =
      &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (streamId_));

    ACE_NEW_NORETURN (session_data_container_p,
                      typename SessionMessageType::DATA_T (session_data_p,
                                                           false)); // *NOTE*: do NOT delete the session data when the container is destroyed
    if (unlikely (!session_data_container_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    release_session_data_container_b = true;
  } // end IF

  switch (control_in)
  { // control
    case STREAM_CONTROL_END:
    case STREAM_CONTROL_ABORT:
    case STREAM_CONTROL_CONNECT:
    case STREAM_CONTROL_DISCONNECT:
    case STREAM_CONTROL_FLUSH:
    case STREAM_CONTROL_RESET:
    case STREAM_CONTROL_STEP:
    case STREAM_CONTROL_STEP_2:
    { ACE_ASSERT (session_data_container_p);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        session_data_container_p->getR ();
      if (!inherited::putControlMessage (session_data_r.sessionId,
                                         control_in,
                                         forwardUpStream_in))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putControlMessage(%u,%d), continuing\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId,
                    control_in));
      else if (control_in == STREAM_CONTROL_ABORT)
        abortSent_ = true;
      break;
    }
    // session notification
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
                  ACE_TEXT ("%s: invalid/unknown control (was: %d), continuing\n"),
                  inherited::mod_->name (),
                  control_in));
      break;
    }
  } // end SWITCH

  if (release_session_data_container_b)
    session_data_container_p->decrease ();

  return;

send_session_message:
  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (session_data_container_p);
  ACE_ASSERT (streamState_);

  // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
  //         condition when the connection is close()d asynchronously
  //         --> see below: line 2015
  bool release_lock = false;
  if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
  { ACE_ASSERT (streamLock_);
    // *NOTE*: prevent potential deadlocks here; in 'busy' scenarios (i.e. high
    //         contention for message buffers/queue slots), a thread may be
    //         holding
    try {
      release_lock = streamLock_->lock (true,  // block ?
                                        true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), returning\n"),
                  inherited::mod_->name ()));
      return;
    }
  } // end IF

  session_data_container_p->increase ();
  typename SessionMessageType::DATA_T* session_data_container_2 =
    session_data_container_p;
  if (unlikely (!inherited::putSessionMessage (message_type_e,
                                               session_data_container_2,
                                               streamState_->userData,
                                               false))) // expedited ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::mod_->name (),
                message_type_e));

  // clean up
  if (unlikely (release_lock))
  { ACE_ASSERT (streamLock_);
    try {
      streamLock_->unlock (false, // unlock ?
                           true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), returning\n"),
                  inherited::mod_->name ()));
      return;
    }
  } // end IF

  if (release_session_data_container_b)
    session_data_container_p->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::notify (SessionEventType notification_in,
                                                   bool forwardUpStream_in,
                                                   bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::notify"));

  ACE_UNUSED_ARG (forwardUpStream_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (notification_in)
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    { ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock);
        session_data_r.aborted = true;
      } // end lock scope

      // *NOTE*: there is no SESSION_END message in this scenario
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        sessionEndSent_ = true;
        sessionEndProcessed_ = true;
      } // end lock scope

      if (likely (!abortSent_))
        control (STREAM_CONTROL_ABORT,
                 false); // forward upstream ?

      // *WARNING*: falls through
      ACE_FALLTHROUGH;
    }
    default:
    {
      // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
      //         condition when the connection is close()d asynchronously
      //         --> see below: line 2015
      bool release_lock = false;
      if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
      { ACE_ASSERT (streamLock_);
        try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), returning\n"),
                      inherited::mod_->name ()));
          return;
        }
      } // end IF

      typename SessionMessageType::DATA_T* session_data_container_p =
        inherited::sessionData_;
      // bool release_session_data_container_b = false;
      if (unlikely (!session_data_container_p))
      {
        SessionManagerType* session_manager_p =
          SessionManagerType::SINGLETON_T::instance ();
        ACE_ASSERT (session_manager_p);
        typename SessionMessageType::DATA_T::DATA_T* session_data_p =
          &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (streamId_));

        ACE_NEW_NORETURN (session_data_container_p,
                          typename SessionMessageType::DATA_T (session_data_p,
                                                               false)); // *NOTE*: do NOT delete the session data when the container is destroyed
        if (unlikely (!session_data_container_p))
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory, returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        // release_session_data_container_b = true;
      } // end IF
      else
        inherited::sessionData_->increase ();
      ACE_ASSERT (streamState_);
      // *NOTE*: "fire-and-forget" the second argument
      if (unlikely (!inherited::putSessionMessage (notification_in,
                                                   session_data_container_p,
                                                   streamState_->userData,
                                                   expedite_in))) // expedited ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                    inherited::name (),
                    notification_in));

      if (unlikely (release_lock))
      { ACE_ASSERT (streamLock_);
        try {
          streamLock_->unlock (false, // unlock ?
                               true); // forward upstream (if any) ?

        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), returning\n"),
                      inherited::mod_->name ()));
          return;
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::stop (bool wait_in,
                                                 bool recurseUpstream_in,
                                                 bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (recurseUpstream_in);

  isHighPriorityStop_ = highPriority_in;

  int result = -1;
  enum Stream_StateMachine_ControlState next_state_e = STREAM_STATE_INVALID;
  ACE_Time_Value timeout (STREAM_STATEMACHINE_CONTROL_STOP_TIMEOUT_S, 0);

  { ACE_GUARD (ACE_Thread_Mutex, aGuard, stateMachineLock_);
retry:
    switch (inherited2::state_)
    {
      case STREAM_STATE_INVALID:
      case STREAM_STATE_INITIALIZED:
        return; // nothing to do
      case STREAM_STATE_SESSION_STARTING:
        goto wait;
      case STREAM_STATE_RUNNING:
      case STREAM_STATE_PAUSED:
        next_state_e = STREAM_STATE_SESSION_STOPPING;
        break;
      case STREAM_STATE_SESSION_STOPPING:
      case STREAM_STATE_STOPPED:
      case STREAM_STATE_FINISHED:
      {
        if (unlikely (isHighPriorityStop_))
        {
          if (likely (!abortSent_))
            control (STREAM_CONTROL_ABORT,
                     false); // forward upstream ?
        } // end IF

        goto wait_2; // wait for any remaining workers ?
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown state (was: %d), returning\n"),
                    inherited::mod_->name (),
                    inherited2::state_));
        return;
      }
    } // end SWITCH
    goto continue_;

wait: // (try to) wait a little while until the state settles down
    while (inherited2::state_ < STREAM_STATE_RUNNING)
    { ACE_ASSERT (inherited2::condition_);
      result = inherited2::condition_->wait (&timeout);
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        if (unlikely (error != ETIME))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Condition::wait(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: failed to ACE_Condition::wait(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
        next_state_e = STREAM_STATE_SESSION_STOPPING;
        goto continue_;
      } // end IF
    } // end WHILE
    goto retry;
  } // end lock scope
continue_:
  inherited2::change (next_state_e);

wait_2:
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  enum Stream_StateMachine_ControlState status_e = inherited2::current ();
  switch (status_e)
  {
    case STREAM_STATE_INVALID:
    case STREAM_STATE_INITIALIZED:
      return false;
    case STREAM_STATE_SESSION_STARTING:
    case STREAM_STATE_RUNNING:
    case STREAM_STATE_PAUSED:
      return true;
    case STREAM_STATE_SESSION_STOPPING:
    case STREAM_STATE_STOPPED:
    case STREAM_STATE_FINISHED:
      break; // left to check: (still) processing data ?
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown state (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  status_e));
      return false; // <-- *TODO*: false negative
    }
  } // end SWITCH

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false); // <-- *TODO*: false negative
    if (!inherited::threadIds_.empty () &&
        !inherited::msg_queue_->is_empty ())
     return true;
  } // end lock scope

  for (Stream_Task_t* task_p = inherited::next_;
       task_p;
       task_p = task_p->next_)
  { ACE_ASSERT (task_p->msg_queue_);
    if (task_p->thr_count () &&
        !task_p->msg_queue_->is_empty ())
      return true;
  } // end FOR

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
    ACE_DEBUG ((LM_ERROR,
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
    ACE_DEBUG ((LM_ERROR,
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
unsigned int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::flush (bool,
                                                  bool,
                                                  bool)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::flush"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    inherited::sessionData_->getR ();

  if (unlikely (!inherited::putControlMessage (session_data_r.sessionId,
                                               STREAM_CONTROL_FLUSH)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putControlMessage(STREAM_CONTROL_FLUSH), aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::lock (bool block_in,
                                                 bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::lock"));

  ACE_UNUSED_ARG (forwardUpstream_in);

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  int result = -1;
  ACE_SYNCH_MUTEX_T& lock_r = inherited::msg_queue_->lock ();

  result = (block_in ? lock_r.acquire () : lock_r.tryacquire ());
  if (unlikely (result == -1))
  { int error_i = ACE_OS::last_error ();
    if (error_i == EBUSY)
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
int
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::unlock (bool unlock_in,
                                                   bool forwardUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::unlock"));

  ACE_UNUSED_ARG (forwardUpstream_in);

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  int result = -1;
  ACE_SYNCH_MUTEX_T& lock_r = inherited::msg_queue_->lock ();
  ACE_thread_mutex_t& mutex_r = lock_r.lock ();

  // sanity check(s)
  // *TODO*: on Windows platforms, the current ACE implementation does not
  //         support ACE_Thread_Mutex::get_thread_id(), although the
  //         data type 'struct _RTL_CRITICAL_SECTION' contains the necessary
  //         information ('OwningThread')
  //         --> submit a bug report
  //ACE_thread_t thread_id = lock_r.get_thread_id ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_hthread_t thread_h = NULL;
  ACE_OS::thr_self (thread_h);
  ACE_ASSERT (thread_h);
  if (unlikely (!ACE_OS::thr_cmp (mutex_r.OwningThread, thread_h)))
#else
  if (unlikely (!ACE_OS::thr_equal (mutex_r.__data.__owner, ACE_OS::thr_self ())))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_OS::last_error (EACCES);
    return -1;
  } // end IF

  do
  {
    result = lock_r.release ();
    if (likely (!unlock_in))
      break;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  } while (mutex_r.RecursionCount > 0);
#else
  } while (mutex_r.__data.__count > 0);
#endif // ACE_WIN32 || ACE_WIN64
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_SYNCH_MUTEX_T::release(): \"%m\", continuing\n"),
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::wait (bool waitForThreads_in,
                                                 bool waitForUpStream_in,
                                                 bool waitForDownStream_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::wait"));

  ACE_UNUSED_ARG (waitForDownStream_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;
  ACE_Reverse_Lock<ACE_Thread_Mutex> reverse_lock (inherited::lock_);
  static ACE_Time_Value timeout (STREAM_STATEMACHINE_WAIT_TIMEOUT_S, 0);
  ACE_Time_Value deadline = ACE_OS::gettimeofday () + timeout;

  // step1: wait for final state
  if (unlikely (!inherited2::wait (STREAM_STATE_FINISHED, &deadline)))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_StateMachine_Base_T::wait(%s,%#T): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited2::stateToString (STREAM_STATE_FINISHED).c_str ()),
                &timeout,
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_StateMachine_Base_T::wait(%s,%#T): \"%m\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited2::stateToString (STREAM_STATE_FINISHED).c_str ()),
                &timeout));
#endif
  } // end IF

  // step2: wait for worker(s) to join ?
  if (unlikely (!waitForThreads_in))
    return;

  ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);

  // sanity check(s)
  if (unlikely (inherited::threadIds_.empty ()))
    goto continue_; // --> stream not started ?
  // *NOTE*: pthread_join() returns EDEADLK when the calling thread IS the
  //         thread to join --> prevent this by comparing thread ids
  for (typename inherited::THREAD_IDS_CONSTITERATOR_T iterator = inherited::threadIds_.begin ();
       iterator != inherited::threadIds_.end ();
       ++iterator)
    if (ACE_OS::thr_equal (ACE_OS::thr_self (),
                           (*iterator).id ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%t: prevented deadlock; stream dispatch thread cannot wait for itself, returning\n"),
                  inherited::mod_->name ()));
      goto continue_;
    } // end IF

  switch (inherited::configuration_->concurrency)
  {
    case STREAM_HEADMODULECONCURRENCY_ACTIVE:
    {
      { ACE_GUARD (ACE_Reverse_Lock<ACE_Thread_Mutex>, aGuard_2, reverse_lock);
        inherited::wait ();
      } // end lock scope
      break;
    }
    case STREAM_HEADMODULECONCURRENCY_PASSIVE:
    {
      ACE_thread_t thread_id = inherited::threadIds_[0].id ();
      ACE_THR_FUNC_RETURN status;
      // *TODO*: do not join() here; instead signal a condition upon leaving
      //         svc()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_hthread_t handle = inherited::threadIds_[0].handle ();
      if (likely (handle != ACE_INVALID_HANDLE))
      {
        { ACE_GUARD (ACE_Reverse_Lock<ACE_Thread_Mutex>, aGuard_2, reverse_lock);
          result = ACE_Thread::join (handle, &status);
        } // end lock scope
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Thread::join(%u/0x%@): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      thread_id, handle));
      } // end IF
#else
      if (likely (static_cast<int> (thread_id) != -1))
      {
        { ACE_GUARD (ACE_Reverse_Lock<ACE_Thread_Mutex>, aGuard_2, reverse_lock);
          result = ACE_Thread::join (thread_id, NULL, &status);
        } // end lock scope
      } // end IF
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Thread::join(%lu): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    thread_id));
#endif // ACE_WIN32 || ACE_WIN64
      break;
    }
    default:
      break;
  } // end SWITCH

continue_:
  ;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::putStatisticMessage (const StatisticContainerType& statisticData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (streamState_);

  bool result = false;

  // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
  //         condition when the connection is close()d asynchronously
  //         --> see below: line 2015
  bool release_lock = false;
  if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
  { ACE_ASSERT (streamLock_);
    try {
      release_lock = streamLock_->lock (true,  // block ?
                                        true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    }
  } // end IF

  inherited::sessionData_->increase ();
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inferences
  session_data_r.statistic = statisticData_in;
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;
  // *NOTE*: "fire-and-forget" the second argument
  // *TODO*: remove type inference
  result =
      inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STATISTIC,
                                    session_data_container_p,
                                    streamState_->userData,
                                    false); // expedited ?
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), aborting\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STATISTIC));

  if (unlikely (release_lock))
  { ACE_ASSERT (streamLock_);
    try {
      streamLock_->unlock (false, // unlock ?
                           true); // forward upstream (if any) ?
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), aborting\n"),
                  inherited::mod_->name ()));
      return false;
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
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
bool
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::onChange (enum Stream_StateMachine_ControlState newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onChange"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited2::stateLock_);

  // initialize return value
  bool result = true; // --> caller will update the state

  int result_2 = -1;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T> reverse_lock (*inherited2::stateLock_);

  switch (newState_in)
  {
    case STREAM_STATE_INITIALIZED:
    {
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false);
        sessionEndSent_ = false;
        sessionEndProcessed_ = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
        if (inherited::closeHandles_)
        {
          ACE_hthread_t handle = ACE_INVALID_HANDLE;
          for (THREAD_IDS_ITERATOR_T iterator = inherited::threadIds_.begin ();
               iterator != inherited::threadIds_.end ();
               ++iterator)
          {
            handle = (*iterator).handle ();
            if (unlikely (handle != ACE_INVALID_HANDLE))
              if (!::CloseHandle (handle))
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                            inherited::mod_->name (),
                            handle,
                            ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          } // end FOR
          inherited::closeHandles_ = false;
        } // end IF
#endif // ACE_WIN32 || ACE_WIN64
        inherited::threadIds_.clear ();
      } // end lock scope

      switch (inherited::configuration_->concurrency)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          inherited::threadCount_ = STREAM_MODULE_DEFAULT_HEAD_THREADS;

          break;
        }
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->concurrency));
          return false;
        }
      } // end SWITCH

      break;
    }
    case STREAM_STATE_SESSION_STARTING:
    {
      // initialize 'this'
      SessionManagerType* session_manager_p =
        SessionManagerType::SINGLETON_T::instance ();
      ACE_ASSERT (session_manager_p);
      typename SessionMessageType::DATA_T::DATA_T* session_data_p =
        &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (streamId_));

      ACE_ASSERT (session_data_p->lock);
      inherited::sessionDataLock_ = session_data_p->lock;

      ACE_ASSERT (!inherited::sessionData_);
      ACE_NEW_NORETURN (inherited::sessionData_,
                        typename SessionMessageType::DATA_T (session_data_p,
                                                             false)); // *NOTE*: do NOT delete the session data when the container is destroyed
      if (unlikely (!inherited::sessionData_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                    inherited::mod_->name ()));
        return false;
      } // end IF

      // *NOTE*: if the object is 'passive/concurrent', the session-begin
      //         message may be processed earlier than 'this' returns, i.e.
      //         the transition 'starting' --> 'running' would fail
      //         --> set the state early
      result = false; // <-- caller will not set the state
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_, false);
        inherited2::state_ = STREAM_STATE_SESSION_STARTING;
      } // end lock scope
      inherited2::signal ();

      // send initial session message downstream ?
      // *NOTE*: this is currently pushed (inline/queue) by the calling thread
      if (likely (inherited::configuration_->generateSessionMessages))
      {
        // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
        //         condition when the connection is close()d asynchronously
        //         --> see below: line 2015
        bool release_lock = false;
        if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), aborting\n"),
                        inherited::mod_->name ()));
            return false;
          }
        } // end IF

        ACE_ASSERT (inherited::sessionData_);
        inherited::sessionData_->increase ();
        typename SessionMessageType::DATA_T* session_data_container_p =
          inherited::sessionData_;
        ACE_ASSERT (session_data_container_p);
        ACE_ASSERT (streamState_);
        // *NOTE*: "fire-and-forget" the second argument
        if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_BEGIN, // session message type
                                                     session_data_container_p,     // session data
                                                     streamState_->userData,       // user data handle
                                                     false)))                      // expedited ?
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(STREAM_SESSION_MESSAGE_BEGIN), continuing\n"),
                      inherited::mod_->name ()));

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), aborting\n"),
                        inherited::mod_->name ()));
            return false;
          }
        } // end IF
      } // end IF

      switch (inherited::configuration_->concurrency)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          result_2 = inherited::open (NULL);
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task_Base::open(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            return false;
          } // end IF

          break;
        }
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        {
          // *NOTE*: if the object is 'passive', the whole operation pertaining
          //         to newState_in is processed 'inline' by the calling thread,
          //         i.e. would complete 'before' the state has transitioned to
          //         'running' --> set the state early
          if (result)
          {
            { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_, false);
              result = false; // <-- caller will not set the state
              inherited2::state_ = STREAM_STATE_SESSION_STARTING;
            } // end lock scope
            inherited2::signal ();
          } // end IF

          { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard_2, inherited::lock_, false);
            // sanity check(s)
            ACE_ASSERT (inherited::threadIds_.empty ());

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
                                              TRUE,
                                              DUPLICATE_SAME_ACCESS)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to DuplicateHandle(0x%@): \"%s\", aborting\n"),
                          inherited::mod_->name (),
                          handle,
                          ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false).c_str ())));
              return false;
            } // end IF
            inherited::closeHandles_ = true;
#endif // ACE_WIN32 || ACE_WIN64
            thread_id.handle (handle);
            inherited::threadIds_.push_back (thread_id);
          } // end lock scope

          result_2 = svc ();
          if (unlikely (result_2 == -1))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task_Base::svc(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          result_2 = close (0);
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task_Base::close(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            return false;
          } // end IF

          break;
        }
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
        {
          // *IMPORTANT NOTE*: this means that there is no worker thread
          //                   driving this module (neither in-line, nor
          //                   dedicated: svc() is not used); data is processed
          //                   in-line in put() by dispatching threads

          bool release_lock = false;
          ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), aborting\n"),
                        inherited::mod_->name ()));
            return false;
          }

          // *NOTE*: if any of the modules failed to initialize, signal the
          //         controller
          // *NOTE*: this works only as long as the stream is fully synchronous,
          //         i.e. has no asynchronous sub-downstream; otherwise there is
          //         a race condition
          ACE_ASSERT (inherited::sessionData_);
          typename SessionMessageType::DATA_T::DATA_T& session_data_r =
              const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
          // *TODO*: remove type inferences
          ACE_ASSERT (session_data_r.lock);
          bool aborted_b = false;
          { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock, false);
            aborted_b = session_data_r.aborted;
          } // end lock scope

          if (release_lock)
          { ACE_ASSERT (streamLock_);
            try {
              streamLock_->unlock (false, // unlock ?
                                   true); // forward upstream (if any) ?
            } catch (...) {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), aborting\n"),
                          inherited::mod_->name ()));
              return false;
            }
          } // end IF

          if (unlikely (aborted_b))
          { result = false; // <-- caller will not set the state
            inherited2::change (STREAM_STATE_SESSION_STOPPING);
          } // end IF

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->concurrency));
          return false;
        }
      } // end SWITCH

      break;
    }
    case STREAM_STATE_RUNNING:
    {
      // *NOTE*: implement tape-recorder logic:
      //         transition PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      //         --> check for this condition before doing anything else
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_, false);
        if (unlikely (inherited2::state_ == STREAM_STATE_PAUSED))
        {
          // resume worker ?
          switch (inherited::configuration_->concurrency)
          {
            case STREAM_HEADMODULECONCURRENCY_ACTIVE:
            {
              inherited::resume ();

              break;
            } // end IF
            case STREAM_HEADMODULECONCURRENCY_PASSIVE:
            {
              // task object not active --> resume the borrowed thread

              ACE_hthread_t handle;
              { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, false);
                handle = inherited::threadIds_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
                ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
                result_2 = ACE_Thread::resume (handle);
              } // end lock scope
              if (unlikely (result_2 == -1))
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to ACE_Thread::resume(): \"%m\", aborting\n"),
                            inherited::mod_->name ()));
                return false;
              } // end IF

              break;
            }
            case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
              break;
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                          inherited::mod_->name (),
                          inherited::configuration_->concurrency));
              return false;
            }
          } // end SWITCH

          break;
        } // end IF
      } // end lock scope

      break;
    }
    case STREAM_STATE_SESSION_STOPPING:
    {
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_, false);
        switch (inherited2::state_)
        {
          case STREAM_STATE_PAUSED:
          {
            // resume worker ?
            switch (inherited::configuration_->concurrency)
            {
              case STREAM_HEADMODULECONCURRENCY_ACTIVE:
              {
                inherited::resume ();

                break;
              } // end IF
              case STREAM_HEADMODULECONCURRENCY_PASSIVE:
              { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard_2, inherited::lock_, false);
                // task is not 'active' --> resume the calling thread (i.e. the
                // thread that invoked start())
                ACE_hthread_t handle = inherited::threadIds_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
                ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
                result_2 = ACE_Thread::resume (handle);
                if (unlikely (result_2 == -1))
                {
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: failed to ACE_Thread::resume(): \"%m\", aborting\n"),
                              inherited::mod_->name ()));
                  return false;
                } // end IF

                break;
              }
              case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
                break;
              default:
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                            inherited::mod_->name (),
                            inherited::configuration_->concurrency));
                return false;
              }
            } // end SWITCH

            break;
          }
          default:
            break;
        } // end SWITCH

        result = false; // <-- caller will not set the state
        inherited2::state_ = STREAM_STATE_SESSION_STOPPING;
      } // end lock scope
      inherited2::signal ();

      // *WARNING*: if the stream contains an asynchronous module, the session
      //            data may already have been reset at this stage !
      bool aborted_b = abortSent_;
      if (likely (!aborted_b))
      { ACE_ASSERT (inherited::sessionData_);
        const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
        // *TODO*: remove type inferences
        { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, *session_data_r.lock, false);
          aborted_b = session_data_r.aborted;
        } // end lock scope
      } // end IF

      bool release_lock = false;
      // typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      typename inherited::ISTREAM_T* istream_p =
        const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
      typename inherited::STREAM_T* downstream_p =
        (istream_p ? istream_p->downstream () : NULL);

      // *IMPORTANT NOTE*: notify any downstream head modules of the state
      //                   change
      if (unlikely (inherited::linked_ &&
                    downstream_p       &&
                    !endSeenFromUpstream_)) // --> send ONCE (from upstream head) only
      {
        ISTREAM_CONTROL_T* istream_control_p =
          dynamic_cast<ISTREAM_CONTROL_T*> (downstream_p);
        if (unlikely (!istream_control_p))
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s:%s: downstream does not implement Stream_IStreamControl_T; cannot notify state change, continuing\n"),
                      ACE_TEXT (istream_p->name ().c_str ()),
                      inherited::mod_->name ()));
          goto continue_;
        } // end IF

        try {
          istream_control_p->control (STREAM_CONTROL_END,
                                      false); // recurse upstream ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s:%s: caught exception in Stream_IStreamControl_T::control(STREAM_CONTROL_END), aborting\n"),
                      ACE_TEXT (istream_p->name ().c_str ()),
                      inherited::mod_->name ()));
          return false;
        }
      } // end IF
continue_:

      // unlink downstream if necessary
      if (unlikely (inherited::linked_ &&
                    downstream_p))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: stream has ended, unlinking downstream\n"),
                    ACE_TEXT (istream_p->name ().c_str ()),
                    inherited::mod_->name ()));

        // step1: unlink downstream
        typename inherited::ISTREAM_T* istream_2 =
          dynamic_cast<typename inherited::ISTREAM_T*> (downstream_p);
        if (unlikely (!istream_2))
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s/%s: downstream does not implement Stream_IStream_T; cannot unlink, continuing\n"),
                      ACE_TEXT (istream_p->name ().c_str ()),
                      inherited::mod_->name ()));
          goto continue_2;
        } // end IF

        // *NOTE*: this notifies downstream
        try {
          istream_2->_unlink ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_IStream_T::_unlink(), aborting\n"),
                      ACE_TEXT (istream_2->name ().c_str ())));
          return false;
        }

continue_2:
        //// step2: notify downstream
        //INOTIFY_T* inotify_p = dynamic_cast<INOTIFY_T*> (downstream_p);
        //if (unlikely (!inotify_p))
        //{
        //  ACE_DEBUG ((LM_WARNING,
        //              ACE_TEXT ("%s:%s: downstream does not implement Stream_INotify_T; cannot notify unlink, continuing\n"),
        //              ACE_TEXT (istream_p->name ().c_str ()),
        //              inherited::mod_->name ()));
        //  goto continue_3;
        //} // end IF

        //try {
        //  inotify_p->notify (STREAM_SESSION_MESSAGE_UNLINK,
        //                     false,  // recurse upstream ?
        //                     false); // expedite ?
        //} catch (...) {
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("%s:%s: caught exception in Stream_INotify_T::notify(STREAM_SESSION_MESSAGE_UNLINK), aborting\n"),
        //              ACE_TEXT (istream_p->name ().c_str ()),
        //              inherited::mod_->name ()));
        //  return false;
        //}

//continue_3:
        // step3: 'downstream' has been unlinked; notify 'upstream' (i.e.
        //         'this') about this fact as well

        // *NOTE*: in 'concurrent' (server-side-)scenarios there is a race
        //         condition when the connection is close()d asynchronously
        if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), aborting\n"),
                        inherited::mod_->name ()));
            return false;
          }
        } // end IF

        // *NOTE*: support sending control messages even when stream is finished...
        typename SessionMessageType::DATA_T* session_data_container_p =
          inherited::sessionData_;
        // bool release_session_data_container_b = false;
        if (unlikely (!session_data_container_p))
        {
          SessionManagerType* session_manager_p =
            SessionManagerType::SINGLETON_T::instance ();
          ACE_ASSERT (session_manager_p);
          typename SessionMessageType::DATA_T::DATA_T* session_data_p =
            &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_manager_p->getR (streamId_));

          ACE_NEW_NORETURN (session_data_container_p,
                            typename SessionMessageType::DATA_T (session_data_p,
                                                                 false)); // *NOTE*: do NOT delete the session data when the container is destroyed
          if (unlikely (!session_data_container_p))
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                        inherited::mod_->name ()));
            if (unlikely (release_lock))
              streamLock_->unlock (false, // unlock ?
                                   true); // forward upstream (if any) ?
            return false;
          } // end IF
          // release_session_data_container_b = true;
        } // end IF
        else
          inherited::sessionData_->increase ();
        ACE_ASSERT (streamState_);
        // *NOTE*: "fire-and-forget" the second argument
        if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_UNLINK, // session message type
                                                     session_data_container_p,      // session data
                                                     streamState_->userData,        // user data handle
                                                     false)))                       // expedited ?
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(STREAM_SESSION_MESSAGE_UNLINK), continuing\n"),
                      inherited::mod_->name ()));

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), aborting\n"),
                        inherited::mod_->name ()));
            return false;
          }
        } // end IF
      } // end IF

      switch (inherited::configuration_->concurrency)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        { Common_ITask* itask_p = this;
          itask_p->stop (false,                // wait ?
                         isHighPriorityStop_); // high priority ?

          break;
        }
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
        {
          finished (false); // recurse upstream ?
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->concurrency));
          return false;
        }
      } // end SWITCH

      if (unlikely (aborted_b)) // *NOTE*: there will be no session-end message; transition manually
        inherited2::change (STREAM_STATE_STOPPED);

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      // suspend the worker(s) ?
      switch (inherited::configuration_->concurrency)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
        {
          result_2 = inherited::suspend ();
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::suspend(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            return false;
          } // end IF

          break;
        } // end IF
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard_2, inherited::lock_, false);
          // task object not active --> suspend the borrowed thread
          ACE_hthread_t handle = inherited::threadIds_[0].handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
          ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif // ACE_WIN32 || ACE_WIN64
          result_2 = ACE_Thread::suspend (handle);
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Thread::suspend(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            return false;
          } // end IF

          break;
        }
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->concurrency));
          return false;
        }
      } // end SWITCH

      break;
    }
    case STREAM_STATE_STOPPED:
    {
      // *IMPORTANT NOTE*: the transition STOPPED --> FINISHED is automatic; set
      //                   state early to facilitate this transition
      { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *inherited2::stateLock_, false);
        result = false; // <-- caller will not set the state
        inherited2::state_ = STREAM_STATE_STOPPED;
      } // end lock scope
      inherited2::signal ();

      inherited2::change (STREAM_STATE_FINISHED);

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      switch (inherited::configuration_->concurrency)
      {
        case STREAM_HEADMODULECONCURRENCY_ACTIVE:
          break;
        case STREAM_HEADMODULECONCURRENCY_PASSIVE:
        case STREAM_HEADMODULECONCURRENCY_CONCURRENT:
        {
          // *NOTE*: passive/concurrent thread(s) never reach close(0)
          //         --> release session data here
          if (likely (inherited::sessionData_))
          {
            inherited::sessionData_->decrease (); inherited::sessionData_ = NULL;
          } // end IF

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown concurrency mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->concurrency));
          return false;
        }
      } // end SWITCH

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

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType>
void
Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
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
                            UserDataType>::finished (bool recurseUpstream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::finished"));

  ACE_UNUSED_ARG (recurseUpstream_in);

  // sanity check(s)
  enum Stream_StateMachine_ControlState state_e = inherited2::current ();
  if (state_e == STREAM_STATE_FINISHED)
    return; // nothing to do
  if (state_e != STREAM_STATE_SESSION_STOPPING)
  { ACE_ASSERT (false);
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: should never send 'end' in current state (was: \"%s\"), continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited2::stateToString (state_e).c_str ())));
  } // end IF
  ACE_ASSERT (inherited::configuration_);

  // send final session message downstream ?
  // --> this triggers the state transition STOPPING --> STOPPED
  // *IMPORTANT NOTE*: the transition STOPPED --> FINISHED is automatic
  //                   However, as the stream may be stop()/finished()-ed
  //                   concurrently, this transition could trigger several
  //                   times --> ensure that only a single 'session end' message
  //                   is generated and processed per session
  bool send_end_message = true;
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    if (likely (!sessionEndSent_))
    {
      sessionEndSent_ = true;
      if (unlikely (sessionEndProcessed_)) // already processed upstream 'session end' ?
        send_end_message = false;
    } // end IF
  } // end lock scope

  if (likely (inherited::configuration_->generateSessionMessages))
  {
    if (likely (send_end_message))
    {
      bool release_lock = false;

      if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
      { ACE_ASSERT (streamLock_);
        try {
          release_lock =
              streamLock_->lock (true,  // block ?
                                 true); // forward upstream (if any) ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), returning\n"),
                      inherited::mod_->name ()));
          return;
        }
      } // end IF

      ACE_ASSERT (inherited::sessionData_);
      inherited::sessionData_->increase ();
      typename SessionMessageType::DATA_T* session_data_container_p =
        inherited::sessionData_;
      ACE_ASSERT (streamState_);
      // *NOTE*: "fire-and-forget" the second argument
      if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_END, // session message type
                                                   session_data_container_p,   // session data
                                                   streamState_->userData,     // user data handle
                                                   false)))                    // expedited ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(STREAM_SESSION_MESSAGE_END), continuing\n"),
                    inherited::mod_->name ()));

      if (unlikely (release_lock))
      { ACE_ASSERT (streamLock_);
        try {
          streamLock_->unlock (false, // unlock ?
                               true); // forward upstream (if any) ?
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), returning\n"),
                      inherited::mod_->name ()));
          return;
        }
      } // end IF
    } // end IF
  } // end IF
  else // <-- cannot send 'session end' message, so transition manually
    inherited2::finished ();
}
