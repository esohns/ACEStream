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

#include "common_defines.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::Stream_HeadModuleTaskBase_T (LockType* lock_in,
                                                                                  bool active_in,
                                                                                  bool autoStart_in,
                                                                                  bool runSvcRoutineOnStart_in,
                                                                                  bool generateSessionMessages_in)
 : inherited (lock_in)
 , inherited2 ()
 , configuration_ (NULL)
 , initialized_ (false)
 , sessionData_ (NULL)
 , streamState_ (NULL)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
 , active_ (active_in)
 , autoStart_ (autoStart_in)
 , generateSessionMessages_ (generateSessionMessages_in)
 , runSvcRoutineOnStart_ (runSvcRoutineOnStart_in)
 , sessionEndSent_ (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , threadID_ (std::numeric_limits<DWORD>::max (), ACE_INVALID_HANDLE)
#else
 , threadID_ (-1, ACE_INVALID_HANDLE)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::Stream_HeadModuleTaskBase_T"));

  inherited2::threadCount_ = STREAM_MODULE_DEFAULT_HEAD_THREADS;

  // set group ID for worker thread(s)
  inherited2::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::~Stream_HeadModuleTaskBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::~Stream_HeadModuleTaskBase_T"));

  int result = -1;

  if (timerID_ != -1)
  {
    const void* act_p = NULL;
    result =
      COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                &act_p);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                  timerID_));
    else
      ACE_DEBUG ((LM_WARNING, // <-- should happen in STREAM_END_SESSION
                  ACE_TEXT ("cancelled timer in Stream_HeadModuleTaskBase_T dtor (id was: %d)\n"),
                  timerID_));
  } // end IF

  if (sessionData_)
    sessionData_->decrease ();

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

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::put (ACE_Message_Block* messageBlock_in,
                                                          ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::put"));

  int result = -1;

  // if active, simply drop the message into the queue
  if (inherited2::thr_count_)
  {
    result = inherited2::putq (messageBlock_in, timeout_in);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited2::name ()));
    return result;
  } // end IF

  // otherwise, process manually
  bool stop_processing = false;
  inherited2::handleMessage (messageBlock_in,
                             stop_processing);

  //return (stop_processing ? -1 : 0);
  return 0;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::open (void* arg_in)
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
  sessionData_ = reinterpret_cast<SessionDataContainerType*> (arg_in);
  ACE_ASSERT (sessionData_);
  sessionData_->increase ();

  // step1: (re-)activate() the message queue
  // *NOTE*: the first time around, the queue will have been open()ed
  //         from within the default ctor; this sets it into an ACTIVATED state
  //         The second time around (i.e. the stream has been stopped/started,
  //         the queue will have been deactivate()d in the process, and getq()
  //         (see svc()) would fail (ESHUTDOWN)
  //         --> (re-)activate() the queue here !
  // step1: (re-)activate() message queue
  result = inherited2::queue_.activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
    return -1;
  } // end IF

  // standard usecase: being implicitly invoked through ACE_Stream::push()
  // --> don't do anything, unless auto-starting
  if (autoStart_)
  {
    if (inherited2::module ())
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("auto-starting \"%s\"...\n"),
                  inherited2::name ()));
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

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::close (u_long arg_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::close"));

  // *NOTE*: this method may be invoked
  //         - by an external thread closing down the active object
  //           --> should NEVER happen as a module !
  //         - by the worker thread which calls this after returning from svc()
  //           --> in this case, this should be a NOP...
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

      ACE_NOTREACHED (return -1;)
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

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::module_closed (void)
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

  if (sessionData_)
  {
    sessionData_->decrease ();
    sessionData_ = NULL;
  } // end IF

  return 0;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::svc"));

  // sanity check(s)
  ACE_ASSERT (sessionData_);

  bool                   finished        = false;
  int                    result          = -1;
  ACE_Message_Block*     message_block_p = NULL;
  bool                   stop_processing = false;
  const SessionDataType& session_data_r  = sessionData_->get ();

  // step1: process data
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));

  while (inherited2::getq (message_block_p,
                           NULL) != -1)
  {
    // sanity check(s)
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        // clean up
        message_block_p->release ();
        message_block_p = NULL;

        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage
        
        // signal the controller ?
        if (!finished)
        {
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::finished ();
          finished = true;

          // *NOTE*: (if passive,) STREAM_SESSION_END has been processed
          //         --> done
          if (inherited2::thr_count_ == 0)
            goto done; // finished processing

          continue;
        } // end IF

done:
        result = 0;

        goto continue_; // STREAM_SESSION_END has been processed
      }
      default:
      {
        inherited2::handleMessage (message_block_p,
                                   stop_processing);
        break;
      }
    } // end SWITCH
    // finished ?
    if (stop_processing)
    {
      // *IMPORTANT NOTE*: message_block_p has already been released() !

      // signal the controller
      inherited::finished ();
      finished = true;

      continue;
    } // end IF

    // session aborted ?
    if (session_data_r.aborted)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("session aborted...\n")));

      // signal the controller
      inherited::finished ();
      finished = true;

      continue;
    } // end IF

    // clean up
    message_block_p = NULL;
  } // end WHILE
  result = -1;

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("worker thread (ID: %t) failed to ACE_Task::getq(): \"%m\", aborting\n")));

continue_:
  return result;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::waitForIdleState"));

  // delegate this to the queue
  try
  {
    inherited2::queue_.waitForIdleState ();
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_IMessageQueue::waitForIdleState, continuing\n")));
  }
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (configuration_);

  SessionDataContainerType& session_data_container_r =
    const_cast<SessionDataContainerType&> (message_inout->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // *TODO*: remove type inference
      ACE_ASSERT (configuration_->streamConfiguration);

      session_data_container_r.increase ();
      sessionData_ =
        &const_cast<SessionDataContainerType&> (session_data_container_r);

      // schedule regular statistic collection ?
      // *NOTE*: the runtime-statistic module is responsible for regular
      //         reporting, the head module merely collects information
      if (configuration_->streamConfiguration->statisticReportingInterval !=
          ACE_Time_Value::zero)
      {
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL, 0);
        ACE_ASSERT (timerID_ == -1);
        ACE_Event_Handler* handler_p = &statisticCollectionHandler_;
        timerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                      NULL,                       // argument
                                                                      COMMON_TIME_NOW + interval, // first wakeup time
                                                                      interval);                  // interval
        if (timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", return\n")));
          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    timerID_,
//                    &interval));
      } // end IF

      break;
    }
    case STREAM_SESSION_END:
    {
      if (timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                    &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      timerID_));
        timerID_ = -1;
      } // end IF

      if (!sessionData_)
        goto continue_;

      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (session_data_container_r.get ());
      SessionDataType& session_data_2 =
        const_cast<SessionDataType&> (sessionData_->get ());
      // *NOTE*: most probable reason: stream has been link()ed after the
      //         session had started, and session data is now that of upstream
      // *TODO*: data could be merged to improve consistency
      if (&session_data_r != &session_data_2)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("session data is inconsistent, continuing\n")));
        session_data_container_r.set (session_data_2);
      } // end IF

      sessionData_->decrease ();
      sessionData_ = NULL;

continue_:
      inherited2::shutdown ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
const ConfigurationType&
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  bool result = false;
  int result_2 = -1;

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.stateMachineLock);

  if (initialized_)
  {
    if (timerID_ != -1)
    {
      const void* act_p = NULL;
      result_2 =
        COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
                                                                  &act_p);
      if (result_2 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                    timerID_));
    } // end IF
    timerID_ = -1;

    initialized_ = false;
  } // end IF

  // *TODO*: remove type inference
  active_ = configuration_in.active;
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  runSvcRoutineOnStart_ = (!configuration_in.active &&
                            configuration_in.passive);
  // sanity check(s)
  ACE_ASSERT (!(active_ && runSvcRoutineOnStart_));

  // *TODO*: remove type inference
  result = inherited::initialize (*configuration_->stateMachineLock);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_StateMachine_Base_T::initialize(), aborting\n")));
    return false;
  } // end IF
  inherited::change (STREAM_STATE_INITIALIZED);

  initialized_ = true;

  return result;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::collect"));

  ACE_UNUSED_ARG (data_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::control (Stream_ControlType control_in,
                                                              bool /* forwardUpStream_in */)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::control"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);
  ACE_ASSERT (sessionData_);

  switch (control_in)
  {
    case STREAM_CONTROL_STEP:
    {
      if (!putSessionMessage (STREAM_SESSION_STEP,
                              *sessionData_,
                              configuration_->streamConfiguration->messageAllocator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putSessionMessage(STREAM_SESSION_STEP), returning\n"),
                    inherited2::name ()));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown control (was: %d), returning\n"),
                  control_in));
      return;
    }
  } // end IF
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::start"));

  if (sessionData_)
  {
    // *TODO*: remove type inference
    SessionDataType& session_data_r =
        const_cast<SessionDataType&> (sessionData_->get ());
    session_data_r.startOfSession = COMMON_TIME_NOW;
  } // end IF

  // --> start a worker thread, if active
  inherited::change (STREAM_STATE_RUNNING);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::stop (bool waitForCompletion_in,
                                                           bool lockedAccess_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::stop"));

  ACE_UNUSED_ARG (lockedAccess_in);

  // (try to) change state
  inherited::change (STREAM_STATE_STOPPED);

  if (waitForCompletion_in)
    waitForCompletion ();
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::isRunning () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::isRunning"));

  Stream_StateMachine_ControlState status = inherited::current ();

  return ((status == STREAM_STATE_PAUSED) || (status == STREAM_STATE_RUNNING));
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::flush (bool flushInbound_in,
                                                            bool flushUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::flush"));

  ACE_UNUSED_ARG (flushInbound_in);
  ACE_UNUSED_ARG (flushUpStream_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::pause ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::pause"));

  // (try to) change state
  inherited::change (STREAM_STATE_PAUSED);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
ProtocolMessageType*
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      // *TODO*: remove type inference
      message_p =
          static_cast<ProtocolMessageType*> (configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      ProtocolMessageType (requestedSize_in));
  if (!message_p)
  {
    if (configuration_->streamConfiguration->messageAllocator)
    {
      if (configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                    requestedSize_in));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate ProtocolMessageType(%u): \"%m\", aborting\n"),
                  requestedSize_in));
  } // end IF

  return message_p;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::rewind ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::rewind"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::upStream (Stream_Base_t* upStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::upStream"));

  ACE_UNUSED_ARG (upStream_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Base_t*
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::upStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::upStream"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_StateMachine_ControlState
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::status () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::status"));

  Stream_StateMachine_ControlState result = inherited::current ();

  return result;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::waitForCompletion (bool waitForThreads_in,
                                                                        bool waitForUpStream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::waitForCompletion"));

  ACE_UNUSED_ARG (waitForUpStream_in);

  // *IMPORTANT NOTE*: when a connection is close()d, a race condition may
  //                   arise here between any of the following actors:
  // - the application (main) thread waiting in Stream_Base_T::waitForCompletion
  // - a (network) event dispatching thread (connection hanndler calling
  //   handle_close() --> waitForCompletion() of the (network) data processing
  //   (sub-)stream)
  // - a stream head module thread pushing the SESSION_END message (i.e.
  //   processing in the 'Net Source/Target' module)
  // - a 'Net IO' module thread processing the SESSION_END message

  int result = -1;

  // step1: wait for final state
  inherited::wait (STREAM_STATE_FINISHED,
                   NULL); // <-- block

  // step2: wait for worker(s) to join ?
  if (waitForThreads_in)
  {
    ACE_thread_t thread_id = ACE_Thread::self ();

    // *NOTE*: pthread_join() returns EDEADLK when the calling thread IS the
    //         the thread to join
    //         --> prevent this by comparing thread ids
    if (ACE_OS::thr_equal (thread_id,
                           threadID_.id ()))
      goto continue_;

    // *IMPORTANT NOTE*: (on Win32) only one thread may inherited2::wait(),
    //                   because ::CloseHandle() was being called twice on the
    //                   same handle, throwing exceptions
    // *TODO*: This is a bug in ACE being worked around here
    //         --> clarify the issue and submit a patch
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited2::lock_);

    if (inherited2::thr_count_)
    {
      // *NOTE*: the task has a dedicated worker thread

//      ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (inherited2::lock_);
//      {
//        ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_MUTEX> > aGuard_2 (reverse_lock);

        result = inherited2::wait ();
//      } // end lock scope
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task_Base::wait(): \"%m\", continuing\n")));
    } // end IF
    else if (runSvcRoutineOnStart_)
    {
      // *NOTE*: the stream head module is using the calling thread

      thread_id = threadID_.id ();
      ACE_THR_FUNC_RETURN status;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_hthread_t handle = threadID_.handle ();
      if (handle != ACE_INVALID_HANDLE)
      {
        result = ACE_Thread::join (handle, &status);
        // *NOTE*: successful join()s close the thread handle
        //         (see OS_NS_Thread.inl:2971)
        if (result == 0) threadID_.handle (ACE_INVALID_HANDLE);
        threadID_.id (std::numeric_limits<DWORD>::max ());
      } // end IF
      else
        result = 0;
#else
      if (static_cast<int> (thread_id) != -1)
      {
        result = ACE_Thread::join (thread_id, NULL, &status);
        threadID_.id (-1);
      } // end IF
      else
        result = 0;
#endif
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread::join(%lu): \"%m\", continuing\n"),
                    thread_id));
    } // end IF
  } // end IF

continue_:

  return;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_t*
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::find (const std::string& name_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}
template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
std::string
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::name () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::name"));

  std::string result = ACE_TEXT_ALWAYS_CHAR (inherited2::name ());
  return result;
}
template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
const StreamStateType&
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::state"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (StreamStateType ());
  ACE_NOTREACHED (return StreamStateType ());
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::initialize (const StreamStateType& streamState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::initialize"));

  streamState_ = &const_cast<StreamStateType&> (streamState_in);
//  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

  inherited::change (STREAM_STATE_INITIALIZED);

  return true;
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_->streamConfiguration);
  ACE_ASSERT (sessionData_);

  // step1: update session state
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

  //  // step2: create session data object container
  //  SessionDataContainerType* session_data_p = NULL;
  //  ACE_NEW_NORETURN (session_data_p,
  //                    SessionDataContainerType (inherited::sessionData_,
  //                                              false));
  //  if (!session_data_p)
  //  {
  //    ACE_DEBUG ((LM_CRITICAL,
  //                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
  //    return false;
  //  } // end IF

  // step3: send the statistic data downstream
  //  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return putSessionMessage (STREAM_SESSION_STATISTIC,
                            *sessionData_,
                            configuration_->streamConfiguration->messageAllocator);
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::onChange (Stream_StateType_t newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::onChange"));

  // sanity check(s)
  ACE_ASSERT (inherited::stateLock_);
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_->streamConfiguration);

  int result = -1;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (*inherited::stateLock_);

  switch (newState_in)
  {
    case STREAM_STATE_INITIALIZED:
    {
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited2::lock_);

      sessionEndSent_ = false;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_hthread_t handle = threadID_.handle ();
      if (handle != ACE_INVALID_HANDLE)
        if (!::CloseHandle (handle))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CloseHandle(0x%@): \"%s\", continuing\n"),
                      handle,
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
      threadID_.handle (ACE_INVALID_HANDLE);
      threadID_.id (std::numeric_limits<DWORD>::max ());
#else
      threadID_.handle (ACE_INVALID_HANDLE);
      threadID_.id (-1);
#endif

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("\"%s\" head module (re-)initialized...\n"),
      //            inherited2::name ()));

      break;
    }
    case STREAM_STATE_RUNNING:
    {
      // *NOTE*: implement tape-recorder logic:
      //         transition PAUSED --> PAUSED is mapped to PAUSED --> RUNNING
      //         --> check for this condition before doing anything else...
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*inherited::stateLock_);

      if (inherited::state_ == STREAM_STATE_PAUSED)
      {
        size_t number_of_threads = inherited2::thr_count_;

        // resume worker ?
        if (number_of_threads)
        {
          result = inherited2::resume ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task::resume(): \"%m\", continuing\n")));
        } // end IF
        else
        {
          // task object not active --> resume the borrowed thread
          ACE_hthread_t handle = threadID_.handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
          ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif
          result = ACE_Thread::resume (handle);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Thread::resume(): \"%m\", continuing\n")));
        } // end ELSE

        break;
      } // end IF

      // send initial session message downstream ?
      if (generateSessionMessages_)
      {
        ACE_ASSERT (sessionData_);
        if (!putSessionMessage (STREAM_SESSION_BEGIN,                                   // type
                                *sessionData_,                                          // session data
                                configuration_->streamConfiguration->messageAllocator)) // allocator
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_BEGIN) failed, continuing\n")));
          break;
        } // end IF
      } // end IF

      if (active_)
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
                                  THR_INHERIT_SCHED),         // flags
                                inherited2::threadCount_,    // number of threads
                                0,                           // force spawning
                                ACE_DEFAULT_THREAD_PRIORITY, // priority
                                inherited2::grp_id (),       // group id (see above)
                                NULL,                        // corresp. task --> use 'this'
                                thread_handles,              // thread handle(s)
                                NULL,                        // thread stack(s)
                                NULL,                        // thread stack size(s)
                                thread_ids,                  // thread id(s)
                                thread_names);               // thread name(s)
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

        // *NOTE*: this may not work if the thread count is > 1 (see
        //         waitForCompletion() above)
        {
          ACE_Guard<ACE_SYNCH_MUTEX> aGuard_2 (inherited2::lock_);

          threadID_.id (thread_ids[0]);
          threadID_.handle (thread_handles[0]);
        } // end lock scope
      } // end IF
      else if (runSvcRoutineOnStart_)
      {
        // *NOTE*: if the implementation is 'passive', the whole operation
        //         pertaining to newState_in is processed 'in-line' by the
        //         calling thread and would complete before the state
        //         actually has been set to 'running'
        //         --> in this case set the state early
        // *TODO*: this may not be the best way to implement that case
        inherited::state_ = STREAM_STATE_RUNNING;

        {
          ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited2::lock_);

          threadID_.id (ACE_Thread::self ());
          ACE_hthread_t handle = ACE_INVALID_HANDLE;
          ACE_Thread::self (handle);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          HANDLE process_handle = ::GetCurrentProcess ();
          if (!::DuplicateHandle (process_handle,
                                  handle,
                                  process_handle,
                                  &handle,
                                  0,
                                  FALSE,
                                  DUPLICATE_CLOSE_SOURCE | DUPLICATE_SAME_ACCESS))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to DuplicateHandle(0x%@): \"%s\", continuing\n"),
                        handle,
                        ACE_TEXT (Common_Tools::error2String (GetLastError ()).c_str ())));
#endif
          threadID_.handle (handle);
        } // end lock scope

        {
          ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_MUTEX> > aGuard_2 (reverse_lock);
          result = svc ();
        } // end lock scope
        //if (result == -1) // *NOTE*: most probable reason: session aborted
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to ACE_Task_Base::svc(): \"%m\", continuing\n")));

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
      else
      {
        // *IMPORTANT NOTE*: this means that there is no worker thread
        //                   driving this module; neither in-line, nor
        //                   dedicated

        // *NOTE*: check if any of the modules failed to initialize
        //         --> just signal the controller

        // sanity check(s)
        ACE_ASSERT (sessionData_);
        // *TODO*: remove type inferences
        SessionDataType& session_data_r =
            const_cast<SessionDataType&> (sessionData_->get ());
        if (session_data_r.aborted)
          this->finished ();
      } // end IF

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      size_t number_of_threads = inherited2::thr_count_;

      // suspend the worker(s) ?
      if (number_of_threads)
      {
        result = inherited2::suspend ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::suspend(): \"%m\", continuing\n")));
      } // end IF
      else
      {
        // task object not active --> suspend the borrowed thread
        ACE_hthread_t handle = threadID_.handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
        ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif
        result = ACE_Thread::suspend (handle);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Thread::suspend(): \"%m\", continuing\n")));
      } // end ELSE

      break;
    }
    case STREAM_STATE_STOPPED:
    {
      // resume worker ?
      size_t number_of_threads = inherited2::thr_count_;

      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*inherited::stateLock_);

        if (inherited::state_ == STREAM_STATE_PAUSED)
        {
          // *TODO*: remove type inference
          if (number_of_threads)
          {
            result = inherited2::resume ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_Task::resume(): \"%m\", continuing\n")));
          } // end IF
          else
          {
            // task object not active --> resume the borrowed thread
            ACE_hthread_t handle = threadID_.handle ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            ACE_ASSERT (handle != ACE_INVALID_HANDLE);
#else
            ACE_ASSERT (static_cast<int> (handle) != ACE_INVALID_HANDLE);
#endif
            result = ACE_Thread::resume (handle);
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_Thread::resume(): \"%m\", continuing\n")));
          } // end ELSE
        } // end IF

        // *NOTE*: in 'passive' mode the finished() method waits for the stream
        //         --> set the (intermediate) state early
        inherited::state_ = STREAM_STATE_STOPPED;
      } // end lock scope

      ACE_thread_t thread_id = ACE_Thread::self ();

      // *TODO*: remove type inference
      if (number_of_threads ||
          (runSvcRoutineOnStart_  && !ACE_OS::thr_equal (thread_id,
                                                         threadID_.id ())))
      {
        // *TODO*: use ACE_Stream::control() instead ?
        inherited2::shutdown ();
      } // end IF
      else
      {
        //if (runSvcRoutineOnStart_)
        //{
        //  ACE_ASSERT (threadID_ != -1);
        //  result = ACE_Thread::kill (threadID_, SIGKILL);
        //  if (result == -1)
        //    ACE_DEBUG ((LM_ERROR,
        //                ACE_TEXT ("ACE_Thread::kill(%d, \"%S\") failed: \"%m\", continuing\n"),
        //                threadID_, SIGKILL));
        //} // end IF

        // signal the controller
        this->finished ();
      } // end ELSE

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*inherited::stateLock_);

        // *NOTE*: modules processing the final session message (see below) may
        //         (indirectly) invoke waitForCompletion() on the stream,
        //         which would deadlock if the implementation is 'passive'
        //         --> set the state early
        // *TODO*: this may not be the best way to handle that case (i.e. it
        //         could introduce other race conditions...)
        inherited::state_ = STREAM_STATE_FINISHED;
      } // end lock scope

      // send final session message downstream ?
      // *IMPORTANT NOTE*: the transition STOPPED --> FINISHED is automatic (see
      //                   above [*NOTE*: shutdown() MUST trigger this
      //                   transition]). As the stream may be stop()ed several
      //                   times (e.g. (safety/sanity) precaution during
      //                   shutdown), this transition COULD occur several times
      //                   --> ensure that (at most) one (!) session end message
      //                       is generated per session
      bool send_end_message = false;
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited2::lock_);

        if (!sessionEndSent_)
        {
          sessionEndSent_ = true;
          send_end_message = true;
        } // end IF
      } // end lock scope
      if (generateSessionMessages_ &&
          send_end_message)
      {
        ACE_ASSERT (sessionData_);
        if (!putSessionMessage (STREAM_SESSION_END,                                     // session message type
                                *sessionData_,                                          // session data
                                configuration_->streamConfiguration->messageAllocator)) // allocator
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("putSessionMessage(SESSION_END) failed, continuing\n")));
      } // end IF

//       ACE_DEBUG ((LM_DEBUG,
//                   ACE_TEXT ("stream processing complete\n")));

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid state transition: \"%s\" --> \"%s\", continuing\n"),
                  ACE_TEXT (inherited::state2String (inherited::state_).c_str ()),
                  ACE_TEXT (inherited::state2String (newState_in).c_str ())));
      break;
    }
  } // end SWITCH
}

template <typename LockType,
          typename TaskSynchType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_HeadModuleTaskBase_T<LockType,
                            TaskSynchType,
                            TimePolicyType,
                            SessionMessageType,
                            ProtocolMessageType,
                            ConfigurationType,
                            StreamStateType,
                            SessionDataType,
                            SessionDataContainerType,
                            StatisticContainerType>::putSessionMessage (Stream_SessionMessageType messageType_in,
                                                                        SessionDataContainerType& sessionData_in,
                                                                        Stream_IAllocator* allocator_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::putSessionMessage"));

  // sanity check(s)
  ACE_ASSERT (streamState_);

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

//      // clean up
//      sessionData_inout->decrease ();
//      sessionData_inout = NULL;

      return false;
    }

    // keep retrying ?
    if (!session_message_p &&
        !allocator_in->block ())
      goto allocate;
  } // end IF
  else
  {
    sessionData_in.increase ();
    SessionDataContainerType* session_data_container_p = &sessionData_in;
    // *TODO*: remove type inference
    ACE_NEW_NORETURN (session_message_p,
                      SessionMessageType (messageType_in,
                                          session_data_container_p,
                                          streamState_->userData));
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
    sessionData_in.decrease ();

    return false;
  } // end IF
  if (allocator_in)
  {
    sessionData_in.increase ();
    SessionDataContainerType* session_data_container_p = &sessionData_in;
    // *TODO*: remove type inference
    session_message_p->initialize (messageType_in,
                                   session_data_container_p,
                                   streamState_->userData);
  } // end IF

  // pass message downstream...
  result = const_cast<OWN_TYPE_T*> (this)->put (session_message_p,
                                                NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put(): \"%m\", aborting\n")));

    // clean up
    session_message_p->release ();

    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("enqueued session message...\n")));

  return true;
}
