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
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::Stream_HeadModuleTaskBase_T (bool isActive_in,
                                                                                    bool autoStart_in,
                                                                                    bool runSvcRoutineOnStart_in)
 : configuration_ ()
// , isActive_ (isActive_in)
 , sessionData_ (NULL)
 , state_ (NULL)
 , lock_ ()
 , queue_ (STREAM_QUEUE_MAX_SLOTS)
 , autoStart_ (autoStart_in)
 , condition_ (lock_)
 , runSvcRoutineOnStart_ (runSvcRoutineOnStart_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  // *TODO*: remove type inference
  configuration_.active = isActive_in;

  // tell the task to use our message queue...
  inherited2::msg_queue (&queue_);

  // set group ID for worker thread(s)
  inherited2::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::~Stream_HeadModuleTaskBase_T ()
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
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::put (ACE_Message_Block* messageBlock_in,
                                                            ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  int result = -1;

  // if active, simply drop the message into the queue...
  // *TODO*: remove type inference
  if (configuration_.active)
  {
    result = inherited2::putq (messageBlock_in, timeout_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::putq(): \"%m\", aborting\n")));
    return result;
  } // end IF

  // otherwise, process manually...
  bool stop_processing = false;
  inherited2::handleMessage (messageBlock_in,
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
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::open (void* args_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::open"));

  int result = -1;

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

  // step1: (re-)activate() the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         The second time around (i.e. the stream has been stopped/started,
  //         the queue will have been deactivate()d in the process, and getq()
  //         (see svc()) would fail (ESHUTDOWN)
  //         --> (re-)activate() the queue here !
  // step1: (re-)activate() message queue
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
    if (inherited2::module ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting \"%s\"...\n"),
                  ACE_TEXT (inherited2::name ())));
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
                  ACE_TEXT ("caught exception in Stream_IStreamControl_T::start(), aborting\n")));
      return -1;
    }
  } // end IF

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::close (u_long arg_in)
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

      ACE_NOTSUP_RETURN (-1);
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid argument (was: %u), aborting\n"),
                  arg_in));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::module_closed (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::module_closed"));

  // *NOTE*: this will be a NOP IF the stream was
  //         stop()ped BEFORE it is deleted !

  // *NOTE*: this method is invoked by an external thread either:
  //         - from the ACE_Module dtor OR
  //         - during explicit ACE_Module::close()

  // *NOTE*: when control flow gets here, the stream SHOULD (!) already be in a
  //         final state...

  // sanity check
  // *WARNING*: this check CAN NOT prevent a potential race condition...
  if (isRunning ())
  {
    // *NOTE*: MAY happen after application receives a SIGINT
    //         select() returns -1, reactor invokes:
    //         remove_handler --> remove_reference --> delete this
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("stream still active in module_closed() --> check implementation !, continuing\n")));
    stop ();
  } // end IF

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool               stop_processing = false;

  // step0: increment thread count
  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    inherited2::threadCount_++;
  } // end IF

  // step1: start processing incoming data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));

  while (inherited2::getq (message_block_p,
                           NULL) != -1)
  {
    inherited2::handleMessage (message_block_p,
                               stop_processing);

    // finished ?
    if (stop_processing)
    {
      // *IMPORTANT NOTE*: message_block_p has already been released() !

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("leaving processing loop...\n")));

      result = 0;

      goto session_finished;
    } // end IF

    // clean up
    message_block_p = NULL;
  } // end WHILE
  result = -1;

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n")));

session_finished:
  // step2: send final session message downstream...
  if (!putSessionMessage (STREAM_SESSION_END,
                          sessionData_,
                          false))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));

  // signal the controller
  finished ();

  return result;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::handleControlMessage (ACE_Message_Block* controlMessage_in,
                                                                             bool& stopProcessing_out,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleControlMessage"));

  // initialize return value(s)
  stopProcessing_out = false;

  switch (controlMessage_in->msg_type ())
  {
    case ACE_Message_Block::MB_STOP:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("received MB_STOP message, shutting down...\n")));

      // clean up
      passMessageDownstream_out = false;
      controlMessage_in->release ();

      // *NOTE*: forward a SESSION_END message to notify any modules downstream
      stopProcessing_out = true;

      break;
    }
    default:
    {
      // ...otherwise, behave like a regular module...
      inherited2::handleControlMessage (controlMessage_in,
                                        stopProcessing_out,
                                        passMessageDownstream_out);

      break;
    }
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleDataMessage"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
const ConfigurationType&
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::get"));

  return configuration_;
}
template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  configuration_ = configuration_in;

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::start"));

  // *TODO*: remove type inferences
  if (sessionData_)
    sessionData_->startOfSession = COMMON_TIME_NOW;

  // --> start a worker thread, if active
  inherited::change (STREAM_STATE_RUNNING);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::stop (bool waitForCompletion_in,
                                                             bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  // (try to) change state
  inherited::change (STREAM_STATE_STOPPED);

  if (waitForCompletion_in)
    waitForCompletion ();
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  return (inherited::current () == STREAM_STATE_RUNNING);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::pause"));

  // (try to) change state
  inherited::change (STREAM_STATE_PAUSED);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::rewind ()
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
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::waitForCompletion ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::waitForCompletion"));

  int result = -1;

  // *TODO*: remove type inference
  if (configuration_.active)
  {
    // step1: wait for workers to finish
    {
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

      while (inherited2::threadCount_)
      {
        result = condition_.wait ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::wait(): \"%m\", continuing\n")));
      } // end WHILE
    } // end IF

    // step2: wait for workers to join
    result = inherited2::wait ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
  } // end IF
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
const StreamStateType&
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::state"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (StreamStateType ());

  ACE_NOTREACHED (return StreamStateType ());
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::initialize (const StreamStateType& state_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  state_ = &const_cast<StreamStateType&> (state_in);
//  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

  return true;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::finished ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::finished"));

  // (try to) set new state
  inherited::change (STREAM_STATE_FINISHED);
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::onChange (Stream_StateType_t newState_in)
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
      // *NOTE*: implement tape-recorder logic:
      //         transition PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      //         --> check for this condition before doing anything else...
      if (inherited::current () == STREAM_STATE_PAUSED)
      {
        // resume worker ?
        // *TODO*: remove type inference
        if (configuration_.active)
        {
          result = inherited2::resume ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task::resume(): \"%m\", continuing\n")));
        } // end IF

        break;
      } // end IF

      // send initial session message downstream...
      if (!putSessionMessage (STREAM_SESSION_BEGIN,
                              sessionData_,
                              false))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("putSessionMessage(SESSION_BEGIN) failed, continuing\n")));
        break;
      } // end IF

      // *TODO*: remove type inference
      if (configuration_.active)
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
          inherited2::activate ((THR_NEW_LWP      |
                                 THR_JOINABLE     |
                                 THR_INHERIT_SCHED),                // flags
                                STREAM_MODULE_DEFAULT_HEAD_THREADS, // number of threads
                                0,                                  // force spawning
                                ACE_DEFAULT_THREAD_PRIORITY,        // priority
                                inherited2::grp_id (),              // group id (see above)
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
      } // end IF
      else if (runSvcRoutineOnStart_)
      {
        result = svc ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task_Base::svc(): \"%m\", continuing\n")));

//        // send initial session message downstream...
//        if (!putSessionMessage (STREAM_SESSION_END,
//                                sessionData_,
//                                false))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));
//          break;
//        } // end IF
      } // end IF

      break;
    }
    case STREAM_STATE_STOPPED:
    {
      // *TODO*: remove type inference
      if (configuration_.active)
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

        result = inherited2::putq (message_block_p, NULL);
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
        if (!putSessionMessage (STREAM_SESSION_END,
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
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

        inherited2::threadCount_--;

        result = condition_.broadcast ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
      } // end lock scope

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("finished successfully !\n")));

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      // suspend the worker(s) ?
      // *TODO*: remove type inference
      if (configuration_.active)
      {
        result = inherited2::suspend ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::suspend(): \"%m\", continuing\n")));
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid state transition: \"%s\" --> \"%s\", continuing\n"),
                  ACE_TEXT (inherited::state2String (inherited::current ()).c_str ()),
                  ACE_TEXT (inherited::state2String (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::putSessionMessage (Stream_SessionMessageType messageType_in,
                                                                          SessionDataContainerType*& sessionData_inout,
                                                                          Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // sanity check(s)
  ACE_ASSERT (state_);

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
  {
    // *IMPORTANT NOTE*: session message assumes responsibility for
    //                   sessionData_inout
    // *TODO*: remove type inference
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (messageType_in,
                                          sessionData_inout,
                                          state_->userData));
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
  {
    // *IMPORTANT NOTE*: session message assumes responsibility for
    //                   sessionData_inout
    // *TODO*: remove type inference
    session_message_p->initialize (messageType_in,
                                   sessionData_inout,
                                   state_->userData);
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
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_HeadModuleTaskBase_T<TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType>::putSessionMessage (Stream_SessionMessageType messageType_in,
                                                                          SessionDataType* sessionData_in,
                                                                          bool deleteSessionData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_.streamConfiguration);

  // create session data
  SessionDataContainerType* session_data_p = NULL;
  switch (messageType_in)
  {
    case STREAM_SESSION_BEGIN:
    case STREAM_SESSION_STEP:
    case STREAM_SESSION_END:
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
    case STREAM_SESSION_STATISTIC:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), aborting\n"),
                  messageType_in));
      return false;
    }
  } // end SWITCH

  // *IMPORTANT NOTE*: "fire-and-forget" session_data_p
  // *TODO*: remove type inference
  return putSessionMessage (messageType_in,
                            session_data_p,
                            configuration_.streamConfiguration->messageAllocator);
}
