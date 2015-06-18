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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_defines.h"
#include "common_timer_manager.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::Stream_HeadModuleTaskBase_T (bool isActive_in,
                                                                               bool autoStart_in)
 : allocator_ (NULL)
 , isActive_ (isActive_in)
 , sessionData_ (NULL)
 , state_ (NULL)
 , autoStart_ (autoStart_in)
 , condition_ (lock_)
 , currentNumThreads_ (0)
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  // tell the task to use our message queue...
  inherited::msg_queue (&queue_);

  // set group ID for worker thread(s)
  inherited::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::~Stream_HeadModuleTaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::~Stream_HeadModuleTaskBase_T"));

  // flush/deactivate our queue (check whether it was empty...)
  int flushed_messages = queue_.flush ();
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
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::put (ACE_Message_Block* mb_in,
                                                       ACE_Time_Value* tv_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  int result = 0;

  // if active, simply drop the message into the queue...
  if (isActive_)
  {
    result = inherited::putq (mb_in, tv_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));
    return result;
  } // end IF

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

  return result;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::open"));

  int result = -1;

  // sanity check
  // *NOTE*: session/user data could be empty...
//   ACE_ASSERT(args_in);

  //// step-1: *WORKAROUND*: for some odd reason, inherited::next_ is not updated
  ////                       correctly
  //if (inherited::mod_)
  //{
  //  Stream_Module_t* module_p = inherited::mod_->next ();
  //  if (module_p)
  //    inherited::next_ = module_p->writer ();
  //} // end IF

  // step0: initialize this
  sessionData_ = reinterpret_cast<SessionDataType*> (args_in);

  // step1: (re-)activate() our queue
  // *NOTE*: the first time around, our queue will have been open()ed
  // from within the default ctor; this sets it into an ACTIVATED state
  // - hopefully, this is what we want.
  // If we come here a second time (i.e. we have been stopped/started, our queue
  // will have been deactivated in the process, and getq() (see svc()) will fail
  // miserably (ESHUTDOWN) --> (re-)activate() our queue !
  // step1: (re-)activate() our queue
  result = queue_.activate ();
  if (result == -1)
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

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
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
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
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
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

  ACE_Message_Block* ace_mb          = NULL;
  bool               stop_processing = false;

  // step0: increment thread count
  {
//    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(myLock);
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    currentNumThreads_++;
  } // end IF

  // step1: send initial session message downstream...
  if (!putSessionMessage (SESSION_BEGIN,
                          sessionData_,
                          false))
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
                              false))
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
  if (!putSessionMessage (SESSION_END,
                          sessionData_,
                          false))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_END) failed, aborting\n")));

  // signal the controller
  finished ();

  return -1;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
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
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::start"));

  if (state_)
    state_->startOfSession = COMMON_TIME_NOW;

  // --> start a worker thread, if active
  inherited2::change (STREAM_STATE_RUNNING);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::stop (bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  // (try to) change state
  inherited2::change (STREAM_STATE_STOPPED);

  waitForCompletion ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  return (inherited2::current () == STREAM_STATE_RUNNING);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::pause"));

  // (try to) change state
  inherited2::change (STREAM_STATE_PAUSED);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::rewind"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

#if defined (_MSC_VER)
  ACE_NOTREACHED (true);
#endif
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::waitForCompletion"));

  int result = -1;

  if (isActive_)
  {
    // step1: wait for workers to finish
    {
//    ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard(myLock);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

      while (currentNumThreads_)
      {
        result = condition_.wait ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Condition::wait(): \"%m\", continuing\n")));
      } // end WHILE
    } // end IF

    // step2: wait for workers to join
    result = inherited::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
  } // end IF
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
const StreamStateType*
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::getState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::getState"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::initialize (const StreamStateType& state_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  state_ = &const_cast<StreamStateType&> (state_in);

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::finished ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::finished"));

  // (try to) set new state
  inherited2::change (STREAM_STATE_FINISHED);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::onChange (Stream_StateType_t newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onChange"));

  int result = -1;

  switch (newState_in)
  {
    case STREAM_STATE_INITIALIZED:
    {
      // OK: (re-)initialized
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("(re-)initialized...\n")));
      break;
    }
    case STREAM_STATE_RUNNING:
    {
      // *NOTE*: we want to implement tape-recorder logic:
      // PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      // --> check for this condition before we do anything else...
      if (inherited2::current () == STREAM_STATE_PAUSED)
      {
        // resume worker ?
        if (isActive_)
        {
          result = inherited::resume ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task::resume(): \"%m\", continuing\n")));
        } // end IF

        break;
      } // end IF

      if (isActive_)
      {
        // OK: start worker
        ACE_hthread_t thread_handles[1];
        thread_handles[0] = 0;
        ACE_thread_t thread_ids[1];
        thread_ids[0] = 0;
        char thread_name[BUFSIZ];
        ACE_OS::memset (thread_name, 0, sizeof (thread_name));
        ACE_OS::strcpy (thread_name, STREAM_MODULE_DEFAULT_HEAD_THREAD_NAME);
        const char* thread_names[1];
        thread_names[0] = thread_name;
        result =
          inherited::activate ((THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),                // flags
                               STREAM_MODULE_DEFAULT_HEAD_THREADS, // number of threads
                               0,                                  // force spawning
                               ACE_DEFAULT_THREAD_PRIORITY,        // priority
                               inherited::grp_id (),               // group id (see above)
                               NULL,                               // corresp. task --> use 'this'
                               thread_handles,                     // thread handle(s)
                               NULL,                               // thread stack(s)
                               NULL,                               // thread stack size(s)
                               thread_ids,                         // thread id(s)
                               thread_names);                      // thread name(s)
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task_Base::activate(): \"%m\", continuing\n")));
          break;
        } // end IF
      } // end IF
      else
      {
        // send initial session message downstream...
        if (!putSessionMessage (SESSION_BEGIN,
                                sessionData_,
                                false))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_BEGIN) failed, continuing\n")));
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
    case STREAM_STATE_STOPPED:
    {
      if (isActive_)
      {
        // OK: drop a control message into the queue...
        // *TODO*: use ACE_Stream::control() instead ?
        ACE_Message_Block* message_block_p = NULL;
        ACE_NEW_NORETURN (message_block_p,
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
        if (!message_block_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate memory: \"%m\", continuing\n")));
          break;
        } // end IF

        result = inherited::putq (message_block_p, NULL);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", continuing\n")));

          // clean up
          message_block_p->release ();
        } // end IF
      } // end IF
      else
      {
        // send final session message downstream...
        if (!putSessionMessage (SESSION_END,
                                sessionData_,
                                false))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));

        // signal the controller
        finished ();
      } // end ELSE

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      // signal waiting thread(s)
      {
//        ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

        currentNumThreads_--;

        result = condition_.broadcast ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));
      } // end lock scope

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("finished successfully !\n")));

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      // suspend the worker ?
      if (isActive_)
      {
        result = inherited::suspend ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::suspend(): \"%m\", continuing\n")));
      } // end IF

      break;
    }
    default:
    {
      // *NOTE*: an invalid/unknown state transition occurred...
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid state switch: \"%s\" --> \"%s\", continuing\n"),
                  ACE_TEXT (inherited2::state2String (inherited2::current ()).c_str ()),
                  ACE_TEXT (inherited2::state2String (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::putSessionMessage (Stream_SessionMessageType messageType_in,
                                                                     SessionDataContainerType*& sessionData_inout,
                                                                     Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  int result = -1;

  // create session message
  SessionMessageType* session_message_p = NULL;
  if (allocator_in)
  {
allocate:
    try
    {
      // *IMPORTANT NOTE*: 0 --> session message !
      session_message_p =
        static_cast<SessionMessageType*> (allocator_in->malloc (0));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(0), aborting\n")));

      // clean up
      sessionData_inout->decrease ();
      sessionData_inout = NULL;

      return false;
    }

    // keep retrying ?
    if (!session_message_p &&
        !allocator_in->block ())
      goto allocate;
  } // end IF
  else
  { // *IMPORTANT NOTE*: session message assumes responsibility for
    //                   sessionData_inout
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (messageType_in,
                                          state_,
                                          sessionData_inout));
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

    // clean up
    sessionData_inout->decrease ();
    sessionData_inout = NULL;

    return false;
  } // end IF
  if (allocator_in)
  { // *IMPORTANT NOTE*: session message assumes responsibility for
    //                   sessionData_inout
    session_message_p->initialize (messageType_in,
                                   state_,
                                   sessionData_inout);
  } // end IF

  // pass message downstream...
  result = const_cast<OWN_TYPE_T*> (this)->put_next (session_message_p,
                                                     NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

    // clean up
    session_message_p->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename SessionMessageType,
          typename ProtocolMessageType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            SessionMessageType,
                            ProtocolMessageType>::putSessionMessage (Stream_SessionMessageType messageType_in,
                                                                     SessionDataType* sessionData_in,
                                                                     bool deleteSessionData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // create session data
  SessionDataContainerType* session_data_p = NULL;

  // switch
  switch (messageType_in)
  {
    case SESSION_BEGIN:
    case SESSION_STEP:
    case SESSION_END:
    {
      ACE_NEW_NORETURN (session_data_p,
                        SessionDataContainerType (sessionData_in,
                                                  deleteSessionData_in));
      if (!session_data_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
        return false;
      } // end IF

      break;
    }
    case SESSION_STATISTICS:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type: %d, aborting\n"),
                  messageType_in));
      return false;
    }
  } // end SWITCH

  // *IMPORTANT NOTE*: "fire-and-forget"-API for session_data_p
  return putSessionMessage (messageType_in,
                            session_data_p,
                            allocator_);
}
