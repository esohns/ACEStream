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
#include "stdafx.h"

#include "stream_headmoduletask.h"

#include "ace/Message_Block.h"

#include "common_timer_manager.h"

#include "stream_macros.h"
#include "stream_defines.h"
#include "stream_message_base.h"
#include "stream_session_message.h"
#include "stream_iallocator.h"

Stream_HeadModuleTask::Stream_HeadModuleTask (bool autoStart_in)
 : allocator_ (NULL)
 , autoStart_ (autoStart_in)
 , condition_ (lock_)
 , isFinished_ (true)
 , queue_ (STREAM_MAX_QUEUE_SLOTS)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::Stream_HeadModuleTask"));

  // tell the task to use our message queue...
  msg_queue (&queue_);

  // set group ID for worker thread(s)
  grp_id (STREAM_TASK_GROUP_ID);
}

Stream_HeadModuleTask::~Stream_HeadModuleTask ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::~Stream_HeadModuleTask"));

  // flush/deactivate our queue (check whether it was empty...)
  int flushed_messages = 0;
  flushed_messages = queue_.flush ();

  // *IMPORTANT NOTE*: this should NEVER happen !
  // debug info
  if (flushed_messages)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("flushed %d message(s)...\n"),
                flushed_messages));

//   // *TODO*: check if this sequence actually works...
//   queue_.deactivate ();
//   queue_.wait ();
}

int
Stream_HeadModuleTask::put (ACE_Message_Block* mb_in,
                            ACE_Time_Value* tv_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::put"));

  // drop the message into our queue...
  return inherited::putq (mb_in,
                          tv_in);
}

int
Stream_HeadModuleTask::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::open"));

  // step0: init user data
  sessionData_ = reinterpret_cast<Stream_SessionData_t*> (args_in);

  // step1: (re-)activate() our queue
  // *NOTE*: the first time around, our queue will have been open()ed
  // from within the default ctor; this sets it into an ACTIVATED state
  // - hopefully, this is what we want.
  // If we come here a second time (i.e. we have been stopped/started, our queue
  // will have been deactivated in the process, and getq() (see svc()) will fail
  // miserably (ESHUTDOWN) --> (re-)activate() our queue !
  // step1: (re-)activate() our queue
  if (queue_.activate () == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));

    return -1;
  } // end IF

  // standard usecase: being implicitly invoked by ACE_Stream::push()...
  // --> don't do anything, unless we're auto-starting
  if (autoStart_)
  {
    if (inherited::module ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting \"%s\"...\n"),
                  ACE_TEXT (inherited::name ())));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting...\n")));

    try
    {
      start ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in start() method, aborting\n")));

      return -1;
    }
  } // end IF

  return 0;
}

int
Stream_HeadModuleTask::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::close"));

  // *NOTE*: this method may be invoked
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

      break;
    }
    case 1:
    {
      // *WARNING*: SHOULD NEVER GET HERE
      // --> module_closed() hook is implemented below !!!
      ACE_ASSERT (false);

      return -1;
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

int
Stream_HeadModuleTask::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::module_closed"));

  // *NOTE*: this will be a NOP IF the stream was
  // stop()ped BEFORE it is deleted !

  // *NOTE*: this method is invoked by an external thread
  // either from the ACE_Module dtor or during explicit ACE_Module::close()

  // *NOTE*: when we get here, we SHOULD ALREADY BE in a final state...

  // sanity check
  // *WARNING*: this check CANNOT prevent a potential race condition...
  if (isRunning ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("isRunning, continuing\n")));

    try
    {
      stop ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in stop(), aborting\n")));

      return -1;
    }
  } // end IF

  // just wait for OUR worker thread to die
  // *NOTE*: this works because we assume that by the time we get here,
  // we're either stop()ed (see above) or otherwise finished with processing, i.e. our
  // worker thread should be dying/dead by now...
  inherited::wait ();

  return 0;
}

int
Stream_HeadModuleTask::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::svc"));

  ACE_Message_Block* ace_mb          = NULL;
  bool               stop_processing = false;

  // step1: send initial session message downstream...
  if (!putSessionMessage (SESSION_BEGIN,
                          sessionData_,
                          COMMON_TIME_POLICY (), // start of session
                          false))                // N/A
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_BEGIN) failed, aborting\n")));

    // signal the controller
    finished ();

    return -1;
  } // end IF

  // step2: start processing incoming data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));

  while (inherited::getq (ace_mb,
                          NULL) != -1)
  {
    inherited::handleMessage (ace_mb,
                              stop_processing);

    // finished ?
    if (stop_processing)
    {
      // *WARNING*: ace_mb has already been released() at this point !

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("leaving processing loop...\n")));

      // step3: send final session message downstream...
      if (!putSessionMessage (SESSION_END,
                              sessionData_,
                              ACE_Time_Value::zero, // N/A
                              true))                // ALWAYS a user abort...
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("putSessionMessage(SESSION_END) failed, aborting\n")));

        // signal the controller
        finished ();

        return -1;
      } // end IF

      // signal the controller
      finished ();

      return 0;
    } // end IF

    // clean up
    ace_mb = NULL;
  } // end WHILE

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n")));

  // step3: send final session message downstream...
  if (!putSessionMessage (SESSION_END,
                          sessionData_,
                          ACE_Time_Value::zero, // N/A
                          false))               // N/A
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_END) failed, aborting\n")));

  // signal the controller
  finished ();

  return -1;
}

// void
// Stream_HeadModuleTask::handleDataMessage (Stream_MessageBase*& message_inout,
//                                           bool& passMessageDownstream_out)
// {
//STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::handleDataMessage"));
//
//   ACE_UNUSED_ARG (message_inout);
//   ACE_UNUSED_ARG (passMessageDownstream_out);
//
//   // *NOTE*: should NEVER be reached !
//   ACE_ASSERT (false);
//
//   // what else can we do ?
//   ACE_NOTREACHED (return;)
// }

void
Stream_HeadModuleTask::handleControlMessage (ACE_Message_Block* controlMessage_in,
                                             bool& stopProcessing_out,
                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::handleControlMessage"));

  // init return value(s)
  stopProcessing_out = false;

  switch (controlMessage_in->msg_type ())
  {
    // currently, we only handle these...
    // --> send a SESSION_END message downstream to tell the other modules...
    case ACE_Message_Block::MB_STOP:
    {
//      ACE_DEBUG((LM_DEBUG,
//                 ACE_TEXT("received MB_STOP message, shutting down...\n")));

      // clean up --> we DON'T pass these along...
      passMessageDownstream_out = false;
      controlMessage_in->release ();

      // OK: tell our worker thread to stop whatever it's doing ASAP
      // *NOTE*: this triggers forwarding a SESSION_END message to notify any
      // modules downstream !
      stopProcessing_out = true;

      break;
    }
    default:
    {
      // otherwise, just behave like a regular module...
      inherited::handleControlMessage (controlMessage_in,
                                       stopProcessing_out,
                                       passMessageDownstream_out);

      break;
    }
  } // end SWITCH
}

void
Stream_HeadModuleTask::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::start"));

  // (try to) change state
  // --> start a worker thread
  changeState (inherited2::STATE_RUNNING);
}

void
Stream_HeadModuleTask::stop ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::stop"));

  // (try to) change state
  // --> tell the worker thread to die
  changeState (inherited2::STATE_STOPPED);

  // ...and wait for it to happen
  //wait ();
}

void
Stream_HeadModuleTask::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::pause"));

  // (try to) change state
  changeState (inherited2::STATE_PAUSED);
}

void
Stream_HeadModuleTask::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::rewind"));

  // *TODO*: implement this !
  ACE_ASSERT (false);
}

void
Stream_HeadModuleTask::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::waitForCompletion"));

  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

    while (!isFinished_)
      condition_.wait ();
  } // end lock scope

//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("leaving...\n")));
}

bool
Stream_HeadModuleTask::isRunning ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::isRunning"));

  return (inherited2::getState () == inherited2::STATE_RUNNING);
}

void
Stream_HeadModuleTask::finished ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::finished"));

  // (try to) set new state
  changeState (inherited2::STATE_FINISHED);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("leaving finished()...\n")));
}

void
Stream_HeadModuleTask::onStateChange (const Stream_StateType_t& newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::onStateChange"));

  switch (newState_in)
  {
    case inherited2::STATE_INITIALIZED:
    {
      // OK: (re-)initialized
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(re-)initialized...\n")));

      break;
    }
    case inherited2::STATE_RUNNING:
    {
      // *NOTE*: we want to implement tape-recorder logic:
      // PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      // --> check for this condition before we do anything else...
      if (inherited2::getState () == inherited2::STATE_PAUSED)
      {
        // resume our worker thread
        if (inherited::resume () == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to resume(): \"%m\", continuing\n")));
        } // end IF

        // finished !
        break;
      } // end IF

      // OK: start worker thread
      ACE_thread_t thread_ids[1];
      thread_ids[0] = 0;
      ACE_hthread_t thread_handles[1];
      thread_handles[0] = 0;

      // *IMPORTANT NOTE*: MUST be THR_JOINABLE !!!
      int result = 0;
      result = inherited::activate ((THR_NEW_LWP      |
                                     THR_JOINABLE     |
                                     THR_INHERIT_SCHED),         // flags
                                    1,                           // number of threads
                                    0,                           // force spawning
                                    ACE_DEFAULT_THREAD_PRIORITY, // priority
                                    inherited::grp_id (),        // group id --> should have been set by now !
                                    NULL,                        // corresp. task --> use 'this'
                                    thread_handles,              // thread handle(s)
                                    NULL,                        // thread stack(s)
                                    NULL,                        // thread stack size(s)
                                    thread_ids);                 // thread id(s)
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to activate(): \"%m\", aborting\n")));

        // finished !
        break;
      } // end IF

      {
        // synchronize access to myIsFinished
        // *TODO*: synchronize access to state logic to make the API re-entrant...
        ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

        isFinished_ = false;
      } // end lock scope

//       if (inherited::module ())
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("module \"%s\" started worker thread (group: %d, id: %u)...\n"),
//                     ACE_TEXT (inherited::name ()),
//                     inherited::grp_id (),
//                     thread_ids[0]));
//       else
//         ACE_DEBUG ((LM_DEBUG,
//                     ACE_TEXT ("started worker thread (group: %d, id: %u)...\n"),
//                     inherited::grp_id (),
//                     thread_ids[0]));

      break;
    }
    case inherited2::STATE_STOPPED:
    {
      // OK: drop control message into stream...
      // *TODO*: use ACE_Stream::control() instead ?
      ACE_Message_Block* stop_mb = NULL;
      ACE_NEW_NORETURN (stop_mb,
                        ACE_Message_Block (0,                                  // size
                                           ACE_Message_Block::MB_STOP,         // type
                                           NULL,                               // continuation
                                           NULL,                               // data
                                           NULL,                               // buffer allocator
                                           NULL,                               // locking strategy
                                           ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                           ACE_Time_Value::zero,               // execution time
                                           ACE_Time_Value::max_time,           // deadline time
                                           NULL,                               // data block allocator
                                           NULL));                             // message allocator)
      if (!stop_mb)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));

        break;
      } // end IF

      if (inherited::putq (stop_mb, NULL) == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to putq(): \"%m\", continuing\n")));

        // clean up
        stop_mb->release ();
      } // end IF

      break;
    }
    case inherited2::STATE_FINISHED:
    {
      // signal waiting thread(s)
      {
        // grab condition lock...
        ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

        isFinished_ = true;

        condition_.broadcast ();
      } // end lock scope

      // OK: (re-)initialized
//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("finished successfully !\n")));

      break;
    }
    case inherited2::STATE_PAUSED:
    {
      // suspend our worker thread
      if (inherited::suspend () == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to suspend(): \"%m\", continuing\n")));

      break;
    }
    default:
    {
      // *NOTE*: if we get here, an invalid/unknown state change happened...

      std::string currentStateString;
      std::string newStateString;
      ControlState2String (inherited2::getState (),
                           currentStateString);
      ControlState2String (newState_in,
                           newStateString);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid state switch: \"%s\" --> \"%s\", continuing\n"),
                  ACE_TEXT (currentStateString.c_str ()),
                  ACE_TEXT (newStateString.c_str ())));

      break;
    }
  } // end SWITCH
}

bool
Stream_HeadModuleTask::putSessionMessage (Stream_SessionMessageType_t messageType_in,
                                          Stream_SessionData_t* sessionData_in,
                                          Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::putSessionMessage"));

  // create session message
  Stream_SessionMessage* message_p = NULL;
  if (allocator_in)
  {
    try
    {
      message_p =
          static_cast<Stream_SessionMessage*> (allocator_in->malloc (0));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_CRITICAL,
                 ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));

      // clean up
      sessionData_in->decrease ();

      return false;
    }
  } // end IF
  else
  {
//    Stream_SessionData_t* session_data_p = sessionData_in;
    // *NOTE*: session message assumes responsibility for sessionData_in !
    ACE_NEW_NORETURN (message_p,
                      Stream_SessionMessage (messageType_in,
                                             sessionData_in));
  } // end ELSE

  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Stream_SessionMessage: \"%m\", aborting\n")));

    // clean up
    sessionData_in->decrease ();

    return false;
  } // end IF
  if (allocator_in)
  { // *NOTE*: session message assumes responsibility for sessionData_inout !
    message_p->initialize (messageType_in,
                           sessionData_in);
  } // end IF

  // pass message downstream...
  int result =
      const_cast<Stream_HeadModuleTask*> (this)->put_next (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to put_next(): \"%m\", aborting\n")));

    // clean up
    message_p->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));

  return true;
}

bool
Stream_HeadModuleTask::putSessionMessage (Stream_SessionMessageType_t messageType_in,
                                          Stream_SessionData_t* sessionData_in,
                                          const ACE_Time_Value& startOfSession_in,
                                          bool userAbort_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTask::putSessionMessage"));

  // switch
  switch (messageType_in)
  {
    case SESSION_BEGIN:
    case SESSION_STEP:
    case SESSION_END:
      break;
    case SESSION_STATISTICS:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type: %d, aborting\n"),
                  messageType_in));

      return false;
    }
  } // end SWITCH

  // *NOTE*: this API is a "fire-and-forget" for sessionData_in
  return putSessionMessage (messageType_in,
                            sessionData_in,
                            allocator_);
}
