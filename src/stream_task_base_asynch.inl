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

#include <limits>
#include <string>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS.h"
#include "ace/Time_Value.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::Stream_TaskBaseAsynch_T ()
 : inherited ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , threadID_ (std::numeric_limits<DWORD>::max (), ACE_INVALID_HANDLE)
#else
 , threadID_ (-1, ACE_INVALID_HANDLE)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::Stream_TaskBaseAsynch_T"));

  inherited::threadCount_ = 1;

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_hthread_t handle = threadID_.handle ();
  if (handle != ACE_INVALID_HANDLE)
    if (!::CloseHandle (handle))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                  handle,
                  ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
#endif
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

  // step1: (re-)activate() the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         - hopefully, this is what is intended
  //         The second time (i.e. the task has been stopped/started, the queue
  //         will have been deactivated in the process, and getq() (see svc()
  //         below) will fail (ESHUTDOWN) --> (re-)activate() the queue
  result = inherited::queue_.activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // step2: (try to) start a new worker thread
  ACE_thread_t thread_ids[1];
  ACE_OS::memset (thread_ids, 0, sizeof (thread_ids));
  ACE_hthread_t thread_handles[1];
  ACE_OS::memset (thread_handles, 0, sizeof (thread_handles));
  char thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
  ACE_OS::strcpy (thread_name, ACE_TEXT_ALWAYS_CHAR (inherited::name ()));
  const char* thread_names[1];
  thread_names[0] = thread_name;
  result =
      inherited::activate ((THR_NEW_LWP      |
                            THR_JOINABLE     |          // *NOTE*: MUST include THR_JOINABLE
                            THR_INHERIT_SCHED),         // flags
                           inherited::threadCount_,     // number of threads
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
  threadID_.handle (thread_handles[0]);
  threadID_.id (thread_ids[0]);

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
//       if (inherited::module ())
//       {
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("\"%s\" worker thread (ID: %t) leaving...\n"),
//                     inherited::mode_->name ()));
//       } // end IF
//       else
//       {
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("worker thread (ID: %t) leaving...\n")));
//       } // end ELSE

      if (inherited::thr_count_ == 0) // last thread ?
      {
        //// *NOTE*: deactivate the queue so it does not accept new data
        //int result = inherited::msg_queue_->deactivate ();
        //if (result == -1)
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
        result = inherited::msg_queue_->flush ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));
        //else if (inherited::mod_)
        //  ACE_DEBUG ((LM_DEBUG,
        //              ACE_TEXT ("\"%s\": flushed %d message(s)...\n"),
        //              inherited::mod_->name (),
        //              result));
      } // end IF

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
    result = inherited::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::wait (): \"%m\", aborting\n")));
  } // end IF

  return result;
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::put (ACE_Message_Block* messageBlock_in,
                                                   ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::put"));

  // drop the message into the queue
  return inherited::putq (messageBlock_in, timeout_in);
}

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
  ACE_Message_Block::ACE_Message_Type message_type;
  int result = 0;
  bool stop_processing = false;
  do
  {
    result = inherited::getq (message_block_p, NULL);
    if (result == -1) break; // error
    ACE_ASSERT (message_block_p);

    message_type = message_block_p->msg_type ();
    if (message_type == ACE_Message_Block::MB_STOP)
    {
      if (inherited::thr_count_ > 0)
      {
        result = inherited::putq (message_block_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                      inherited::name ()));

          // clean up
          message_block_p->release ();
        } // end IF

        goto done; // closing...
      } // end IF

      // clean up
      message_block_p->release ();

      goto done; // done
    } // end IF

    // process manually
    inherited::handleMessage (message_block_p,
                              stop_processing);
    if (stop_processing) // *NOTE*: message_block_p has already been processed
      inherited::shutdown ();

    // initialize
    message_block_p = NULL;
  } while (true);
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
    inherited::queue_.waitForIdleState ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IMessageQueue::waitForIdleState, continuing\n")));
  }
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_TaskBaseAsynch_T<TimePolicyType,
                        SessionMessageType,
                        ProtocolMessageType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TaskBaseAsynch_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_END:
    {
      inherited::shutdown ();
      break;
    }
    default:
      break;
  } // end SWITCH
}
