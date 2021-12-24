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

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::Stream_TaskBaseAsynch_T (typename Stream_TaskBase_T<ACE_SYNCH_USE,
                                                                                           TimePolicyType,
                                                                                           ConfigurationType,
                                                                                           ControlMessageType,
                                                                                           DataMessageType,
                                                                                           SessionMessageType,
                                                                                           SessionControlType,
                                                                                           SessionEventType,
                                                                                           UserDataType>::ISTREAM_T* stream_in)
 : inherited (stream_in,
              // *TODO*: this looks dodgy, but seems to work nonetheless...
              &queue_)   // queue handle
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::Stream_TaskBaseAsynch_T"));

  // *TODO*: pass this in from outside
  inherited::threadCount_ = 1;

  // set group id for worker thread(s)
  // *TODO*: pass this in from outside
  inherited::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}


template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::~Stream_TaskBaseAsynch_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::~Stream_TaskBaseAsynch_T"));

  int result = -1;

  //   // *TODO*: check whether this sequence works
  //   queue_.deactivate ();
  //   queue_.wait ();

  // *NOTE*: deactivate the queue so it does not accept new data
  result = queue_.deactivate ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));

  result = queue_.flush ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
  else if (result)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: flushed %d message(s)\n"),
                inherited::mod_->name (),
                result));
  } // end ELSE IF
  inherited::msg_queue (NULL);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    queue_.flush (true);
  } // end IF

  if (unlikely (queue_.deactivated ()))
  {
    result = queue_.activate ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
int
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::open"));

  ACE_UNUSED_ARG (args_in);

  int result = -1;

  // step1: (re-)activate() the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         (hopefully, this is what is intended)
  //         The second time (i.e. the task has been stopped/started, the queue
  //         will have been deactivated in the process, and getq() (see svc()
  //         below) will fail (ESHUTDOWN)
  //         --> (re-)activate() the queue
  result = queue_.activate ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  // step2: spawn worker thread(s)
  ACE_ASSERT (!inherited::thr_count_);
  result = inherited::open (NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_TaskBase_T::open(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  //for (unsigned int i = 0;
  //     i < inherited::threadIds_.size ();
  //     ++i)
  //  string_stream << ACE_TEXT_ALWAYS_CHAR ("#") << (i + 1)
  //                << ACE_TEXT_ALWAYS_CHAR (" ")
  //                << inherited::threadIds_[i].thread_id ()
  //                << ACE_TEXT_ALWAYS_CHAR ("\n");
  //if (inherited::mod_)
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s: spawned worker thread(s) (\"%s\", group: %d):\n%s"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (inherited::threadName_.c_str ()),
  //              inherited::grp_id (),
  //              ACE_TEXT (string_stream.str ().c_str ())));
  //else
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("spawned worker thread(s) (\"%s\", group: %d):\n%s"),
  //              ACE_TEXT (inherited::threadName_.c_str ()),
  //              inherited::grp_id (),
  //              ACE_TEXT (string_stream.str ().c_str ())));

  return 0;

error:
  result = queue_.deactivate ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

  return -1;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
int
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::close"));

  int result = 0;

  // *NOTE*: this method may be invoked
  //         - by external threads shutting down the active object (arg_in: 1)
  //         - by worker thread(s) upon returning from svc() (arg_in: 0)
  switch (arg_in)
  {
    case 0:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::msg_queue_);

      ACE_thread_t handle = ACE_OS::thr_self ();
      // final thread ? --> clean up
      if (likely (ACE_OS::thr_equal (handle,
                                     inherited::last_thread ())))
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
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                            handle_2,
                            ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
            } // end FOR
#endif // ACE_WIN32 || ACE_WIN64
          inherited::threadIds_.clear ();
        } // end lock scope

        // *NOTE*: this deactivates the queue so it does not accept new data
        //         after the last (worker) thread has left
        int result_2 = inherited::msg_queue_->deactivate ();
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
        else if (unlikely (result_2))
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: flushed %u message(s)\n"),
                      inherited::mod_->name (),
                      result_2));
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
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
int
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::module_closed"));

//  if (inherited::thr_count_ > 0)
//    inherited::stop (true,  // wait ?
//                     false, // high priority ?
//                     true); // locked access ?

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
int
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                            ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::put"));

  // *TODO*: move this into svc() ?
  if (unlikely (messageBlock_in->msg_type () == ACE_Message_Block::MB_FLUSH))
  {
    // *TODO*: support selective flushing via ControlMessageType
    int result = inherited::msg_queue_->flush ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_T::flush(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    else
      if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: flushed %d messages\n"),
                    inherited::mod_->name (),
                    result));
    messageBlock_in->release ();
    return (result > 0 ? 0 : result);
  } // end IF

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
    {
      ControlMessageType* message_p =
        static_cast<ControlMessageType*> (messageBlock_in);

      switch (message_p->type ())
      {
        case STREAM_CONTROL_MESSAGE_ABORT:
        {
          // *IMPORTANT NOTE*: make sure the message is actually processed
          { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, queue_.lock (), -1);
            if (unlikely (!inherited::thr_count_ ||
                          queue_.isShuttingDown ()))
            {
              // *IMPORTANT NOTE*: it is either too early or too late to process
              //                   this message by this (and (!) subsequent
              //                   synchronous downstream-) task(s)
              //                   --> do it manually
              bool stop_processing = false;
              inherited::handleMessage (messageBlock_in,
                                        stop_processing);
              return 0;
            } // end IF
            return queue_.enqueue_head_i (messageBlock_in, NULL);
          } // end lock scope

          ACE_NOTREACHED (break;)
        }
        default:
          break;
      } // end SWITCH

      break;
    }
    default:
      break;
  } // end SWITCH

  // drop the message into the queue
  return inherited::putq (messageBlock_in, timeout_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
bool
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::isShuttingDown () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::isShuttingDown"));

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  ACE_Message_Block* message_block_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::msg_queue_->lock (), false); // <-- *TODO*: false negative !
    for (typename inherited::MESSAGE_QUEUE_ITERATOR_T iterator (*inherited::msg_queue_);
         iterator.next (message_block_p);
         iterator.advance ())
    { ACE_ASSERT (message_block_p);
      if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
        return true;
      message_block_p = NULL;
    } // end FOR
  } // end lock scope

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
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
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::stop (bool waitForCompletion_in,
                                             bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::stop"));

  control (ACE_Message_Block::MB_STOP,
           highPriority_in);

  if (waitForCompletion_in)
    this->wait (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
void
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::control (int messageType_in,
                                                bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::control"));

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // enqueue a control message
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       messageType_in,                     // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = (highPriority_in ? inherited::ungetq (message_block_p, NULL)
                            : inherited::putq (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::%s(): \"%m\", continuing\n"),
                inherited::mod_->name (),
                (highPriority_in ? ACE_TEXT ("ungetq") : ACE_TEXT ("putq"))));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
int
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  int error = -1;
  bool stop_processing = false;

  do
  {
    result = inherited::getq (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF

    ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      if (unlikely (inherited::thr_count_ > 1))
      {
        result = inherited::putq (message_block_p, NULL);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          return -1;
        } // end IF
        message_block_p = NULL;
      } // end IF
      // clean up ?
      if (message_block_p)
      {
        message_block_p->release (); message_block_p = NULL;
      } // end IF
      break; // done
    } // end IF

    // process manually
    inherited::handleMessage (message_block_p,
                              stop_processing);
    if (unlikely (stop_processing))
      this->stop (false, // wait ?
                  true); // high priority ?

    message_block_p = NULL;
  } while (true);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}
