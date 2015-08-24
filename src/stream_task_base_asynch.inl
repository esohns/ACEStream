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

#include <string>

#include "ace/OS.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/Time_Value.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::Stream_TaskBaseAsynch_T ()
 : threadID_ (0)
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::Stream_TaskBaseAsynch_T"));

  // tell the task to use this message queue
  inherited::msg_queue (&queue_);

  // set group ID for worker thread(s)
  // *TODO*: pass this in from outside...
  inherited::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::~Stream_TaskBaseAsynch_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::~Stream_TaskBaseAsynch_T"));

  // flush/deactivate our queue (check whether it was empty)
  int flushed_messages = 0;
  flushed_messages = queue_.flush ();

  // *NOTE*: this should probably NEVER happen !
  // debug info
  if (flushed_messages)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("flushed %d message(s)...\n"),
                flushed_messages));
  } // end IF

//   // *TODO*: check if this sequence actually works...
//   queue_.deactivate ();
//   queue_.wait ();
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::open"));

  ACE_UNUSED_ARG (args_in);

  int result = -1;

  // step1: (re-)activate() the queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         - hopefully, this is what is intended
  //         The second time (i.e. the task has been stopped/started, the queue
  //         will have been deactivated in the process, and getq() (see svc()
  //         below) will fail (ESHUTDOWN) --> (re-)activate() the queue
  result = queue_.activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // step2: (try to) start a new worker thread
  ACE_thread_t thread_ids[1];
  thread_ids[0] = 0;
  ACE_hthread_t thread_handles[1];
  thread_handles[0] = 0;
  char thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  ACE_OS::strcpy (thread_name, ACE_TEXT_ALWAYS_CHAR (inherited::name ()));
  const char* thread_names[1];
  thread_names[0] = thread_name;
  result =
      inherited::activate ((THR_NEW_LWP      |
                            THR_JOINABLE     |          // *NOTE*: MUST include THR_JOINABLE
                            THR_INHERIT_SCHED),         // flags
                           1,                           // number of threads
                           0,                           // force spawning
                           ACE_DEFAULT_THREAD_PRIORITY, // priority
                           inherited::grp_id (),        // group id --> should have been set by now !
                           NULL,                        // corresp. task --> use 'this'
                           thread_handles,              // thread handle(s)
                           NULL,                        // thread stack(s)
                           NULL,                        // thread stack size(s)
                           thread_ids,                  // thread id(s)
                           thread_names);               // thread name(s)
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF
  threadID_ = thread_ids[0];

//   if (inherited::module ())
//   {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("module \"%s\" started worker thread (group: %d, id: %u)...\n"),
//                 inherited::name (),
//                 inherited::grp_id (),
//                 threadID_));
//   } // end IF
//   else
//   {
//     ACE_DEBUG ((LM_DEBUG,
//                 ACE_TEXT ("started worker thread (group: %d, id: %u)...\n"),
//                 inherited::grp_id(),
//                 threadID_));
//   } // end IF

  return result;
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::close"));

  // *IMPORTANT NOTE*: this method may be invoked
  // - by an external thread closing down the active object
  //    --> should NEVER happen as a module !
  // - by the worker thread which calls this after returning from svc()
  //    --> in this case, this should be a NOP...
  switch (arg_in)
  {
    // called from ACE_Task_Base on clean-up
    case 0:
    {
//       if (inherited::module ())
//       {
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
//                     ACE_TEXT (inherited::name ())));
//       } // end IF
//       else
//       {
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));
//       } // end ELSE

      // don't (need to) do anything...
      break;
    }
    case 1:
    {
      // *IMPORTANT NOTE*: SHOULD NEVER GET HERE
      // --> module_closed() hook is implemented below !!!
      ACE_ASSERT (false);
      ACE_NOTREACHED (return -1;)
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument: %u, aborting\n"),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::module_closed"));

  int result = 0;

  if (inherited::thr_count_ > 0)
  {
    inherited::shutdown ();

    // wait for the worker thread(s) to join
    int result = inherited::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::wait (): \"%m\", aborting\n")));
  } // end IF

  return result;
}

//template <typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType>
//int
//Stream_TaskBaseAsynch_T<TaskSynchType,
//                        TimePolicyType,
//                        SessionMessageType,
//                        ProtocolMessageType>::put (ACE_Message_Block* messageBlock_in,
//                                                   ACE_Time_Value* timeout_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::put"));
//
//  // drop the message into the queue
//  return inherited::putq (messageBlock_in, timeout_in);
//}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::svc"));

  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("(%t) worker starting...\n")));

  ACE_Message_Block* message_block_p = NULL;
  int result = 0;
  bool stop_processing = false;
  while (inherited::getq (message_block_p, NULL) != -1) // blocking wait
  {
    if (message_block_p->msg_type () == ACE_Message_Block::MB_STOP)
    {
      if (inherited::thr_count_ > 1)
      {
        result = inherited::putq (message_block_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));

          // clean up
          message_block_p->release ();
        } // end IF
      } // end IF
      else
      {
        // clean up
        message_block_p->release ();
      } // end ELSE

      goto done;
    } // end IF

    inherited::handleMessage (message_block_p,
                              stop_processing);

    // finished ?
    if (stop_processing)
    {
      // *WARNING*: message_block_p has already been released() at this point !

      goto done;
    } // end IF

    // initialize
    message_block_p = NULL;
  } // end WHILE
  result = -1;

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n")));

done:
  return result;
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::waitForIdleState"));

  // delegate this to the queue
  try
  {
    queue_.waitForIdleState ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IMessageQueue::waitForIdleState, continuing\n")));
  }
}

//template <typename TaskSynchType,
//          typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType>
//void
//Stream_TaskBaseAsynch_T<TaskSynchType,
//                        TimePolicyType,
//                        SessionMessageType,
//                        ProtocolMessageType>::shutdown ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::shutdown"));
//
//  int result = -1;
//
////   if (inherited::module ())
////   {
////     ACE_DEBUG ((LM_DEBUG,
////                 ACE_TEXT ("initiating shutdown of worker thread(s) for module \"%s\"...\n"),
////                 inherited::name ()));
////   } // end IF
////   else
////   {
////     ACE_DEBUG ((LM_DEBUG,
////                 ACE_TEXT ("initiating shutdown of worker thread(s)...\n")));
////   } // end ELSE
//
//  ACE_Message_Block* message_block_p = NULL;
//  ACE_NEW_NORETURN (message_block_p,
//                    ACE_Message_Block (0,                                  // size
//                                       ACE_Message_Block::MB_STOP,         // type
//                                       NULL,                               // continuation
//                                       NULL,                               // data
//                                       NULL,                               // buffer allocator
//                                       NULL,                               // locking strategy
//                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
//                                       ACE_Time_Value::zero,               // execution time
//                                       ACE_Time_Value::max_time,           // deadline time
//                                       NULL,                               // data block allocator
//                                       NULL));                             // message allocator)
//  if (!message_block_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
//    return;
//  } // end IF
//
//  result = inherited::putq (message_block_p, NULL);
//  if (result == -1)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", continuing\n")));
//
//    // clean up
//    message_block_p->release ();
//  } // end IF
//
////   if (inherited::module ())
////   {
////     ACE_DEBUG ((LM_DEBUG,
////                 ACE_TEXT ("initiating shutdown of worker thread(s) for module \"%s\"...DONE\n"),
////                 ACE_TEXT (inherited::name ())));
////   } // end IF
////   else
////   {
////     ACE_DEBUG ((LM_DEBUG,
////                 ACE_TEXT ("initiating shutdown of worker thread(s)...DONE\n")));
////   } // end ELSE
//}
