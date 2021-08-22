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
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::Stream_TaskBaseAsynch_T (typename Stream_TaskBase_T<ACE_SYNCH_USE,
                                                                                           TimePolicyType,
                                                                                           Common_IRecursiveLock_T<ACE_SYNCH_USE>,
                                                                                           ConfigurationType,
                                                                                           ControlMessageType,
                                                                                           DataMessageType,
                                                                                           SessionMessageType,
                                                                                           SessionIdType,
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
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType>
Stream_TaskBaseAsynch_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionIdType,
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
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    queue_.flush ();
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
          typename SessionIdType,
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
                        SessionIdType,
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

  // step2: spawn worker thread(s) ?
  if (!inherited::thr_count_)
  {
    result = inherited::open (NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_TaskBase_T::open(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
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

  goto done;

error:
  result = queue_.deactivate ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

done:
  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::close"));

  int result = -1;

  // *IMPORTANT NOTE*: this method may be invoked
  //                   - by an external thread closing down the active object
  //                     --> should NEVER happen as a module !
  //                   - by the worker thread after returning from svc()
  //                     --> in this case, this should be a NOP
  switch (arg_in)
  {
    // called from ACE_Task_Base on clean-up
    case 0:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: worker thread (id was: %t) stopping...\n"),
                  inherited::mod_->name ()));

      if (inherited::thr_count_ == 0) // last thread ?
      {
        // *NOTE*: deactivate the queue so it does not accept new data
        result = inherited::msg_queue_->deactivate ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
        result = inherited::msg_queue_->flush ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
        else
          if (result)
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: flushed %d message(s)\n"),
                        inherited::mod_->name (),
                        result));
      } // end IF

      // don't (need to) do anything
      break;
    }
    case 1:
    {
      // *IMPORTANT NOTE*: control should never reach here; module_closed() hook
      //                   is implemented (see below)
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (-1);
      ACE_NOTREACHED (return -1;)
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

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
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
                        SessionIdType,
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
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::put (ACE_Message_Block* messageBlock_in,
                                            ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::put"));

  // *TODO*: move this into svc()
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

  // drop the message into the queue
  return inherited::putq (messageBlock_in,
                          timeout_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::isShuttingDown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::isShuttingDown"));

  // sanity check(s)
  if (unlikely (!inherited::msg_queue_))
  {
    if (inherited::mod_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: task has no message queue, returning\n"),
                  inherited::mod_->name ()));
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("task has no message queue, returning\n")));
    return true;
  } // end IF

  bool result = false;
  ACE_Message_Block* message_block_p = NULL;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::msg_queue_->lock (), false);
    for (MESSAGE_QUEUE_ITERATOR_T iterator (*inherited::msg_queue_);
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
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::stop (bool waitForCompletion_in,
                                             bool highPriority_in,
                                             bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

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
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::control (int messageType_in,
                                                bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::control"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // sanity check(s)
  if (unlikely (!inherited::msg_queue_))
  {
    if (inherited::mod_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: task has no message queue, returning\n"),
                  inherited::mod_->name ()));
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("task has no message queue, returning\n")));
    return;
  } // end IF

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
    if (inherited::mod_)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                  inherited::mod_->name ()));
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", returning\n")));
    return;
  } // end IF

  result = (highPriority_in ? inherited::ungetq (message_block_p, NULL)
                            : inherited::putq (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    if (inherited::mod_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", continuing\n")));

    message_block_p->release (); message_block_p = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
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
                        SessionIdType,
                        SessionControlType,
                        SessionEventType,
                        UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::svc"));

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: worker thread %t starting...\n"),
              inherited::mod_->name ()));
#endif // _DEBUG

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  int result_2 = -1;
  int error = -1;
  bool stop_processing = false;

  do
  {
    result = inherited::getq (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      else
        result = 0; // OK, queue has been close()d
      break;
    } // end IF
    ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      result = 0;
      if (inherited::thr_count_ > 1)
      {
        result_2 = inherited::putq (message_block_p, NULL);
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          result = -1;
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
    if (unlikely (stop_processing && inherited::thr_count_))
      this->stop (false, // wait ?
                  true,  // high priority ?
                  true); // locked access ?

    message_block_p = NULL;
  } while (true);

  return result;
}
