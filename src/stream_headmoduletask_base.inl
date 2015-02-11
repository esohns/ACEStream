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

#include "common_defines.h"
#include "common_timer_manager.h"

#include "stream_defines.h"
#include "stream_iallocator.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::Stream_HeadModuleTaskBase_T (bool isActive_in,
                                                                               bool autoStart_in)
 : allocator_ (NULL)
 , sessionID_ (0)
 , isActive_ (isActive_in)
 , condition_ (lock_)
 , currentNumThreads_ (STREAM_DEF_NUM_STREAM_HEAD_THREADS)
 , queue_ (STREAM_MAX_QUEUE_SLOTS)
 , autoStart_ (autoStart_in)
// , userData_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  // init user data
  ACE_OS::memset (&userData_, 0, sizeof (userData_));

  // tell the task to use our message queue...
  inherited::msg_queue (&queue_);

  // set group ID for worker thread(s)
  inherited::grp_id (STREAM_TASK_GROUP_ID);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::~Stream_HeadModuleTaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::~Stream_HeadModuleTaskBase_T"));

  // flush/deactivate our queue (check whether it was empty...)
  int flushed_messages = 0;
  flushed_messages = queue_.flush ();

  if (flushed_messages)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("flushed %d message(s)...\n"),
                flushed_messages));

//   // *TODO*: check if this sequence actually works...
//   queue_.deactivate ();
//   queue_.wait ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::put (ACE_Message_Block* mb_in,
                                                       ACE_Time_Value* tv_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  // if active, simply drop the message into the queue...
  if (isActive_)
    return inherited::putq (mb_in, tv_in);

  // otherwise, process manually...
  bool stop_processing = false;
  inherited::handleMessage (mb_in,
                            stop_processing);

  // finished ?
  if (stop_processing)
  {
    // *WARNING*: mb_in has already been released() at this point !

    stop ();
  } // end IF

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::open"));

  // sanity check
  // *WARNING*: DataType == void* --> args_in COULD be NULL...
//   ACE_ASSERT(args_in);

  // step0: init user data
  userData_ = *static_cast<DataType*> (args_in);

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
  // --> don't do anything, unless auto-starting
  if (autoStart_)
  {
    if (inherited::module ())
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting \"%s\"...\n"),
                  ACE_TEXT (inherited::name ())));
    } // end IF
    else
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting...\n")));
    } // end ELSE

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

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::close"));

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
//       // debug info
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
      // --> refer to module_closed () hook
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

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::module_closed"));

  // *NOTE*: this will be a NOP IF the stream was
  // stop()ped BEFORE it is deleted !

  // *NOTE*: this method is invoked by an external thread
  // either from the ACE_Module dtor or during explicit ACE_Module::close()

  // *NOTE*: when we get here, we SHOULD ALREADY BE in a final state...

  // sanity check
  // *WARNING*: this check CAN NOT prevent a potential race condition...
  if (isRunning ())
  {
    // *NOTE*: MAY happen after application receives a SIGINT
    // select() returns -1, reactor invokes remove_handler --> remove_reference --> delete this
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("stream is still running --> check implementation !, continuing\n")));

    stop ();
  } // end IF

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

  ACE_Message_Block* ace_mb          = NULL;
  bool               stop_processing = false;

  // step0: increment thread count
  {
//    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(myLock);
    ACE_Guard<ACE_Thread_Mutex> aGuard (lock_);

    currentNumThreads_++;
  } // end IF

  // step1: send initial session message downstream...
  if (!putSessionMessage (sessionID_,
                          Stream_SessionMessage::MB_STREAM_SESSION_BEGIN,
                          userData_,
                          COMMON_TIME_POLICY(), // timestamp: start of session
                          false))               // N/A
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
      if (!putSessionMessage (sessionID_,
                              Stream_SessionMessage::MB_STREAM_SESSION_END,
                              userData_,
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

      // done
      return 0;
    } // end IF

    // clean up
    ace_mb = NULL;
  } // end WHILE

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n")));

  // step3: send final session message downstream...
  if (!putSessionMessage (sessionID_,
                          Stream_SessionMessage::MB_STREAM_SESSION_END,
                          userData_,
                          ACE_Time_Value::zero, // N/A
                          false))               // N/A
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_END) failed, aborting\n")));

  // signal the controller
  finished ();

  return -1;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::handleControlMessage (ACE_Message_Block* controlMessage_in,
                                                                        bool& stopProcessing_out,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleControlMessage"));

  // init return value(s)
  stopProcessing_out = false;

  switch (controlMessage_in->msg_type ())
  {
    case ACE_Message_Block::MB_STOP:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MB_STOP message, shutting down...\n")));

      // clean up --> we DON'T pass these along...
      passMessageDownstream_out = false;
      controlMessage_in->release ();

      // *NOTE*: forward a SESSION_END message to notify any modules downstream
      stopProcessing_out = true;

      break;
    }
    default:
    {
      // ...otherwise, behave like a regular module...
      inherited::handleControlMessage (controlMessage_in,
                                       stopProcessing_out,
                                       passMessageDownstream_out);

      break;
    }
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::start"));

  // --> start a worker thread, if active
  changeState (inherited2::STATE_RUNNING);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::stop (bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  // (try to) change state
  changeState (inherited2::STATE_STOPPED);

  waitForCompletion ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::pause"));

  // (try to) change state
  changeState (inherited2::STATE_PAUSED);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::rewind"));

  // *TODO*: implement this !
  ACE_ASSERT (false);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::waitForCompletion"));

  if (isActive_)
  {
    // step1: wait for workers to finish
    {
//    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(myLock);
      ACE_Guard<ACE_Thread_Mutex> aGuard (lock_);

      while (currentNumThreads_)
        condition_.wait ();
    } // end IF

    // step2: wait for workers to join
    if (inherited::wait () == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
  } // end IF
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  return (inherited2::getState () == inherited2::STATE_RUNNING);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::finished ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::finished"));

  // (try to) set new state
  changeState (inherited2::STATE_FINISHED);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("leaving finished()...\n")));
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::onStateChange (const Control_StateType& newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onStateChange"));

  switch (newState_in)
  {
    case inherited2::STATE_INIT:
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
        // resume worker ?
        if (isActive_)
          if (inherited::resume () == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to resume(): \"%m\", continuing\n")));

        break;
      } // end IF

      if (isActive_)
      {
        // OK: start worker
        ACE_hthread_t thread_handles[1];
        thread_handles[0] = 0;
        ACE_thread_t thread_ids[1];
        thread_ids[0] = 0;
        char thread_name[COMMON_BUFSIZE];
        ACE_OS::memset (thread_name, 0, sizeof (thread_name));
        ACE_OS::strcpy (thread_name, STREAM_DEF_HANDLER_THREAD_NAME);
        const char* thread_names[1];
        thread_names[0] = thread_name;
        if (inherited::activate ((THR_NEW_LWP      |
                                  THR_JOINABLE     |
                                  THR_INHERIT_SCHED),         // flags
                                 1,                           // number of threads
                                 0,                           // force spawning
                                 ACE_DEFAULT_THREAD_PRIORITY, // priority
                                 inherited::grp_id (),        // group id (see above)
                                 NULL,                        // corresp. task --> use 'this'
                                 thread_handles,              // thread handle(s)
                                 NULL,                        // thread stack(s)
                                 NULL,                        // thread stack size(s)
                                 thread_ids,                  // thread id(s)
                                 thread_names) == -1)         // thread name(s)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task_Base::activate(): \"%m\", aborting\n")));

          break;
        } // end IF
      } // end IF
      else
      {
        // send initial session message downstream...
        if (!putSessionMessage (sessionID_,
                                Stream_SessionMessage::MB_STREAM_SESSION_BEGIN,
                                userData_,
                                COMMON_TIME_POLICY (), // timestamp: start of session
                                false))                // N/A
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_BEGIN) failed, aborting\n")));

          break;
        } // end IF
      } // end ELSE

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
      if (isActive_)
      {
        // OK: drop a control message into the queue...
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
                                             NULL));                             // message allocator
        if (!stop_mb)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));

          break;
        } // end IF

        if (inherited::putq (stop_mb, NULL) == -1)
        {
          ACE_DEBUG((LM_ERROR,
                     ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", continuing\n")));

          // clean up
          stop_mb->release ();
        } // end IF
      } // end IF
      else
      {
        // send final session message downstream...
        if (!putSessionMessage (sessionID_,
                                Stream_SessionMessage::MB_STREAM_SESSION_END,
                                userData_,
                                ACE_Time_Value::zero, // N/A
                                false))               // N/A
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_END) failed, aborting\n")));

        // signal the controller
        finished ();
      } // end ELSE

      break;
    }
    case inherited2::STATE_FINISHED:
    {
      // signal waiting thread(s)
      {
//        ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);
        ACE_Guard<ACE_Thread_Mutex> aGuard (lock_);

        currentNumThreads_--;

        condition_.broadcast ();
      } // end lock scope

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("finished successfully !\n")));

      break;
    }
    case inherited2::STATE_PAUSED:
    {
      // suspend the worker ?
      if (isActive_)
        if (inherited::suspend () == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to suspend(): \"%m\", continuing\n")));

      break;
    }
    default:
    {
      // *NOTE*: if we get here, an invalid/unknown state change happened...

      // debug info
      std::string currentStateString;
      std::string newStateString;
      ControlState2String (getState (),
                           currentStateString);
      ControlState2String (newState_in,
                           newStateString);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid state switch: \"%s\" --> \"%s\", continuing\n"),
                  ACE_TEXT (currentStateString.c_str()),
                  ACE_TEXT (newStateString.c_str())));

      break;
    }
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::putSessionMessage (unsigned int sessionID_in,
                                                                     const Stream_SessionMessageType& messageType_in,
                                                                     SessionConfigurationType*& configuration_inout,
                                                                     Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // create session message
  SessionMessageType* message = NULL;
  if (allocator_in)
  {
    try
    {
      // *IMPORTANT NOTE*: 0 --> session message !
      message = static_cast<SessionMessageType*> (allocator_in->malloc (0));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("caught exception in RPG_Stream_IAllocator::malloc(0), aborting\n")));

      // clean up
      configuration_inout->decrease ();

      return false;
    }
  } // end IF
  else
  { // *IMPORTANT NOTE*: session message assumes responsibility for session_config
    ACE_NEW_NORETURN (message,
                      SessionMessageType (sessionID_in,
                                          messageType_in,
                                          configuration_inout));
  } // end ELSE

  if (!message)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to allocate SessionMessageType: \"%m\", aborting\n")));

    // clean up
    configuration_inout->decrease ();

    return false;
  } // end IF
  if (allocator_in)
  { // *IMPORTANT NOTE*: session message assumes responsibility for session_config
    message->init (sessionID_in,
                   messageType_in,
                   configuration_inout);
  } // end IF

  // pass message downstream...
  if (const_cast<own_type*> (this)->put_next (message, NULL) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to put_next(): \"%m\", aborting\n")));

    // clean up
    message->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename DataType,
          typename SessionConfigurationType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            DataType,
                            SessionConfigurationType,
                            SessionMessageType,
                            ProtocolMessageType>::putSessionMessage (unsigned int sessionID_in,
                                                                     const Stream_SessionMessageType& messageType_in,
                                                                     const DataType& userData_in,
                                                                     const ACE_Time_Value& startOfSession_in,
                                                                     bool userAbort_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // create session data
  SessionConfigurationType* configuration_p = NULL;

  // switch
  switch (messageType_in)
  {
    case Stream_SessionMessage::MB_STREAM_SESSION_BEGIN:
    case Stream_SessionMessage::MB_STREAM_SESSION_STEP:
    case Stream_SessionMessage::MB_STREAM_SESSION_END:
    {
      ACE_NEW_NORETURN (configuration_p,
                        SessionConfigurationType (userData_in,
                                           startOfSession_in,
                                           userAbort_in));
      if (!configuration_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to allocate SessionConfigurationType: \"%m\", aborting\n")));

        return false;
      } // end IF

      break;
    }
    case Stream_SessionMessage::MB_STREAM_SESSION_STATISTICS:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type: %d, aborting\n"),
                  messageType_in));

      return false;
    }
  } // end SWITCH

  // *IMPORTANT NOTE*: "fire-and-forget"-API for session_config
  return putSessionMessage (sessionID_in,
                            messageType_in,
                            configuration_p,
                            allocator_);
}
