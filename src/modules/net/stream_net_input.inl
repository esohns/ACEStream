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

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_net_common.h"

#include "net_common_tools.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_InputReader_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::Stream_Module_Net_InputReader_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputReader_T::Stream_Module_Net_InputReader_T"));

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
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_InputReader_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputReader_T::handleControlMessage"));

  ConnectionManagerType* connection_manager_p =
    ConnectionManagerType::SINGLETON_T::instance ();
  typename ConnectionManagerType::ICONNECTION_T* connection_p = NULL;
  WRITER_T* sibling_task_p = static_cast<WRITER_T*> (inherited::sibling ());
  if (unlikely (!sibling_task_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Module_Net_InputWriter_T>(0x%@), returning\n"),
                inherited::mod_->name (),
                inherited::sibling ()));
    return;
  } // end IF
  SESSION_DATA_T* session_data_p = NULL;

  // sanity check(s)
  ACE_ASSERT (connection_manager_p);
  // *TODO*: remove type inferences
  if (unlikely (!sibling_task_p->sessionData_))
    goto continue_; // something went wrong: session aborted ?

  session_data_p =
    &const_cast<SESSION_DATA_T&> (sibling_task_p->sessionData_->getR ());
  //  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);
    ACE_ASSERT (!session_data_p->connectionStates.empty ());
    ACE_ASSERT (session_data_p->connectionStates.size () == 1);
    ACE_ASSERT ((*session_data_p->connectionStates.begin ()).first != ACE_INVALID_HANDLE);
    connection_p =
      connection_manager_p->get ((*session_data_p->connectionStates.begin ()).first);
  //} // end lock scope
  if (unlikely (!connection_p))
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve connection (id was: 0x%@), returning\n"),
                inherited::mod_->name (),
                (*session_data_p->connectionStates.begin ()).first));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve connection (id was: %d), returning\n"),
                inherited::mod_->name (),
                (*session_data_p->connectionStates.begin ()).first));
#endif // ACE_WIN32 || ACE_WIN64
    return;
  } // end IF

continue_:
  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_DISCONNECT:
    { ACE_ASSERT (connection_p);

      // *WARNING*: regular disconnections must enforce that all enqueued
      //            outbound data has been dispatched by the kernel. This
      //            implementation works as long as there are no asynchronous
      //            upstream module reader tasks
      Stream_IMessageQueue* i_message_queue_p =
        dynamic_cast<Stream_IMessageQueue*> (connection_p);
      if (unlikely (!i_message_queue_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Stream_IMessageQueue>(0x%@), returning\n"),
                    inherited::mod_->name (),
                    connection_p));
        return;
      } // end IF
      try {
        i_message_queue_p->waitForIdleState ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Net_IStreamConnection_T::waitForIdleState() (id was: %u), continuing\n"),
                    inherited::mod_->name (),
                    connection_p->id ()));
        return;
      }

      // *WARNING*: the control flow falls through here
      ACE_FALLTHROUGH;
    }
    case STREAM_CONTROL_MESSAGE_ABORT:
    { ACE_ASSERT (connection_p);
      try {
        connection_p->abort ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Net_IConnection_T::abort() (id was: %u), continuing\n"),
                    inherited::mod_->name (),
                    connection_p->id ()));
        return;
      }
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: aborted connection (id was: %u)\n"),
                  inherited::mod_->name (),
                  connection_p->id ()));

      break;
    }
    default:
      break;
  } // end SWITCH

  connection_p->decrease (); connection_p = NULL;
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
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_InputWriter_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::Stream_Module_Net_InputWriter_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , manageSessionData_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputWriter_T::Stream_Module_Net_InputWriter_T"));

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
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_InputWriter_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputWriter_T::initialize"));

  bool result = false;

  if (unlikely (inherited::isInitialized_))
  {
    manageSessionData_ = true;
  } // end IF

  // *TODO*: remove type inferences
  if (configuration_in.concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: trying to correct (head-module-) concurrency setting\n"),
                inherited::mod_->name ()));
    const_cast<ConfigurationType&> (configuration_in).concurrency =
      STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n"),
                inherited::mod_->name ()));

  return result;
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
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_InputWriter_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputWriter_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // *TODO*: remove this test(, it slows things down unnecessarily)
  ACE_ASSERT (inherited::sessionData_);
  const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    inherited::sessionData_->getR ();
  message_inout->set (session_data_r.sessionId); // session id
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
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_InputWriter_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputWriter_T::handleSessionMessage"));

  int result = -1;
  Stream_Task_t* task_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - the session is being aborted by the user
      //                   - the session is being aborted by some module

      // *NOTE*: deactivate the stream head queue so it does not accept new
      //         data ?
      if (inherited::configuration_->outboundNotificationHandle)
      {
        task_p = inherited::mod_->reader ();
        ACE_ASSERT (task_p);
        while ((ACE_OS::strcmp (task_p->module ()->name (),
                                ACE_TEXT ("ACE_Stream_Head"))) &&
               (ACE_OS::strcmp (task_p->module ()->name (),
                                ACE_TEXT (STREAM_MODULE_HEAD_NAME))))
        {
          task_p = task_p->next ();
          if (!task_p)
            break;
        } // end WHILE
        if (unlikely (!task_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: no head module reader task found, returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        ACE_ASSERT (task_p->msg_queue_);
        result = task_p->msg_queue_->deactivate ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *NOTE*: (depending on stream configuration options) if this is an
      //         outbound connection, there may be two 'session begin' messages:
      //         - one when the connection stream is started (i.e. when the
      //           connection has been established)
      //         - one when the connection stream is appended to the processing
      //           stream

      if (inherited::configuration_->statisticReportingInterval != ACE_Time_Value::zero)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p = NULL;

        // schedule regular statistic collection ?
        if (inherited::timerId_ != -1)
          goto continue_2;

        itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_), // event handler handle
                                            NULL,                            // asynchronous completion token
                                            COMMON_TIME_NOW + interval,      // first wakeup time
                                            interval);                       // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
      } // end IF

continue_2:
      // *WARNING*: ward consecutive STREAM_SESSION_BEGIN messages here.
      //            This happens when using the Stream_Module_Net_Target_T in
      //            active mode. When the connection stream is start()ed (1x)
      //            during connection establishment, a link between the module
      //            stream and the connection stream is established. After the
      //            module has initialized the connection (see
      //            stream_module_target.inl:266), the session message is
      //            forwarded downstream, onto the connections' stream (2x).
      // *NOTE*: the connection handle has already been retrieved when the
      //         second message arrives (see stream_module_io_stream.inl:232)
      goto continue_3;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_3:
      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_UNLINK:
      break;
    case STREAM_SESSION_MESSAGE_CONNECT:
    { ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ConnectionManagerType* connection_manager_p =
        ConnectionManagerType::SINGLETON_T::instance ();
      typename ConnectionManagerType::ICONNECTION_T* connection_p = NULL;
      ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
      ACE_ASSERT (connection_manager_p);
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        ACE_ASSERT (!session_data_r.connectionStates.empty ());
        ACE_ASSERT (session_data_r.connectionStates.size () == 1);
        handle_h = (*session_data_r.connectionStates.begin ()).first;
      } // end lock scope
      ACE_ASSERT (handle_h != ACE_INVALID_HANDLE);
      connection_p = connection_manager_p->get (handle_h);
      if (unlikely (!connection_p))
      {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to retrieve connection (id was: 0x%@), returning\n"),
                    inherited::mod_->name (),
                    handle_h));
#else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to retrieve connection (id was: %d), returning\n"),
                    inherited::mod_->name (),
                    handle_h));
#endif // ACE_WIN32 || ACE_WIN64
        return;
      } // end IF
      // *NOTE*: if this connection was initiated by a stream (e.g. the net
      //         source module), the session data already contains the
      //         connection handle
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        if (likely (!session_data_r.connection && manageSessionData_))
          session_data_r.connection = connection_p;
        else
        { ACE_ASSERT (session_data_r.connection == connection_p);
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: session data already contains connection handle (was: %@, is: %@), continuing\n"),
                      inherited::mod_->name (),
                      session_data_r.connection,
                      connection_p));
          connection_p->decrease ();
          manageSessionData_ = false;
        } // end ELSE
      } // end lock scope
      connection_p = NULL;
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      if (likely (manageSessionData_))
      { ACE_ASSERT (inherited::sessionData_);
        typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          if (session_data_r.connection)
          {
            session_data_r.connection->decrease (); session_data_r.connection = NULL;
          } // end IF
        } // end lock scope
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - processing is complete ('outbound' scenario)
      //                   - the connection has been closed and the processing
      //                     stream has/is finished()-ing (see: handle_close())
      //                   [- the session is being aborted by the user
      //                   - the session is being aborted by some module]

      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (unlikely (inherited::sessionEndProcessed_))
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (likely (inherited::timerId_ != -1))
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer (id was: %d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
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
          typename SessionManagerType,
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_InputWriter_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                AddressType,
                                ConnectionManagerType,
                                UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_InputWriter_T::collect"));

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
  if (unlikely (!inherited::putStatisticMessage (data_out))) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}
