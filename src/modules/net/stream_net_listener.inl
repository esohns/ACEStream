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

#include "ace/INET_Addr.h"
#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_net_common.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_connection_configuration.h"

#include "net_client_common_tools.h"
#include "net_client_defines.h"

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Net_Listener_Reader_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::Stream_Module_Net_Listener_Reader_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Listener_Reader_T::Stream_Module_Net_Listener_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Net_Listener_Reader_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Listener_Reader_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_END:
      break;
    case STREAM_CONTROL_MESSAGE_DISCONNECT:
    {
      // *NOTE*: there is no need to do anything, the control is handled by the
      //         upstream Stream_Module_Net_IOReader_T task
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: unknown/invalid control message type (was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_in.type ()));
      return;
    }
  } // end SWITCH
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              StatisticContainerType,
                              TimerManagerType,
                              ListenerType,
                              UserDataType>::Stream_Module_Net_ListenerH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , handles_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::Stream_Module_Net_ListenerH_T"));

  inherited::threadCount_ = 0; // make sure 'this' is never start()ed
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              StatisticContainerType,
                              TimerManagerType,
                              ListenerType,
                              UserDataType>::~Stream_Module_Net_ListenerH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::~Stream_Module_Net_ListenerH_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
bool
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              StatisticContainerType,
                              TimerManagerType,
                              ListenerType,
                              UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
  {
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::initialize"));

  if (inherited::isInitialized_)
  {
    handles_.clear ();
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
void
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              StatisticContainerType,
                              TimerManagerType,
                              ListenerType,
                              UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      goto end;

      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    {
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inferences
      ListenerType* listener_p =
        typename ListenerType::SINGLETON_T::instance ();
      ACE_ASSERT (listener_p);
      bool stop_listening_b = false;

      // schedule regular statistic collection ?
      if (inherited::configuration_->statisticReportingInterval != ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                   : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        inherited::timerId_ =
            itimer_manager_p->schedule_timer (&(inherited::statisticHandler_), // event handler handle
                                              NULL,                            // asynchronous completion token
                                              COMMON_TIME_NOW + interval,      // first wakeup time
                                              interval);                       // interval
        if (inherited::timerId_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    inherited::mod_->name (),
//                    inherited::timerId_,
//                    &interval));
      } // end IF

      // --> start listening

      // step1: initialize
      // sanity check(s)
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->listenerConfiguration);
      typename ListenerType::CONFIGURATION_T* listener_configuration_p =
        static_cast<typename ListenerType::CONFIGURATION_T*> (inherited::configuration_->listenerConfiguration);
      if (!listener_p->initialize (*listener_configuration_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ListenerType::initialize(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // step2: register for accepted connections / disconnects
      listener_p->subscribe (this);

      // step3: listen
      if (!listener_p->start (NULL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ListenerType::start(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      stop_listening_b = true;

      goto continue_;

error:
      if (stop_listening_b)
        listener_p->stop (true, false);

      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

      // stop listening
      ListenerType* listener_p =
        typename ListenerType::SINGLETON_T::instance ();
      ACE_ASSERT (listener_p);

      // deregister for accepted connections / disconnects
      listener_p->unsubscribe (this);

      if (likely (listener_p->isRunning ()))
        listener_p->stop (true, false);

      if (inherited::timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                   : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        int result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                     &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
      } // end IF

      if (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
      { Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,  // wait for completion ?
                       false); // high priority ?
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
void
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              StatisticContainerType,
                              TimerManagerType,
                              ListenerType,
                              UserDataType>::connect (ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::connect"));

  // step1: notify downstream
  typename SessionMessageType::DATA_T* session_data_container_p = NULL;
  if (likely (inherited::sessionData_))
    session_data_container_p = inherited::sessionData_->clone ();
  else
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no session data (yet|any more); making a temporary one, continuing\n"),
                inherited::name ()));

    typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      typename SessionMessageType::DATA_T::DATA_T ());
    ACE_ASSERT (session_data_p);
    ACE_NEW_NORETURN (session_data_container_p,
                      typename SessionMessageType::DATA_T (session_data_p));
    ACE_ASSERT (session_data_container_p);
  } // end ELSE
  ACE_ASSERT (session_data_container_p);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());
  // *TODO*: remove type inferences
  session_data_r.sessionId = reinterpret_cast<Stream_SessionId_t> (handle_in);
  ACE_ASSERT (inherited::streamState_);
  // *NOTE*: "fire-and-forget" the second argument
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_CONNECT,
                                               session_data_container_p,
                                               inherited::streamState_->userData,
                                               false))) // expedited ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::name (),
                STREAM_SESSION_MESSAGE_CONNECT));

  // step2: store handle
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    handles_.push_back (handle_in);
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
void
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              StatisticContainerType,
                              TimerManagerType,
                              ListenerType,
                              UserDataType>::disconnect (ACE_HANDLE handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::disconnect"));

  // step1: notify downstream
  typename SessionMessageType::DATA_T* session_data_container_p = NULL;
  if (likely (inherited::sessionData_))
    session_data_container_p = inherited::sessionData_->clone ();
  else
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no session data (yet|any more); making a temporary one, continuing\n"),
                inherited::name ()));

    typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
    ACE_NEW_NORETURN (session_data_p,
                      typename SessionMessageType::DATA_T::DATA_T ());
    ACE_ASSERT (session_data_p);
    ACE_NEW_NORETURN (session_data_container_p,
                      typename SessionMessageType::DATA_T (session_data_p));
    ACE_ASSERT (session_data_container_p);
  } // end ELSE
  ACE_ASSERT (session_data_container_p);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());
  session_data_r.sessionId = reinterpret_cast<Stream_SessionId_t> (handle_in); // *TODO*: remove type inferences
  ACE_ASSERT (inherited::streamState_);
  // *NOTE*: "fire-and-forget" the second argument
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_DISCONNECT,
                                               session_data_container_p,
                                               inherited::streamState_->userData,
                                               false))) // expedited ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::name (),
                STREAM_SESSION_MESSAGE_DISCONNECT));

  // step2: store handle
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    HANDLES_ITERATOR_T iterator =
      std::find (handles_.begin (), handles_.end (), handle_in);
    ACE_ASSERT (iterator != handles_.end ());
    handles_.erase (iterator);
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ListenerType,
          typename UserDataType>
bool
Stream_Module_Net_ListenerH_T<ACE_SYNCH_USE,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            ConfigurationType,
                            StreamControlType,
                            StreamNotificationType,
                            StreamStateType,
                            StatisticContainerType,
                            TimerManagerType,
                            ListenerType,
                            UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_ListenerH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistic information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}
