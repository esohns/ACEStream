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

#include "net_common_tools.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::Stream_Module_Net_IOReader_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::Stream_Module_Net_IOReader_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::~Stream_Module_Net_IOReader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::~Stream_Module_Net_IOReader_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::handleControlMessage"));

  WRITER_T* sibling_task_p =
    dynamic_cast<WRITER_T*> (inherited::sibling ());
  if (!sibling_task_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<Stream_Module_Net_IOWriter_T>(0x%@), returning\n"),
                inherited::mod_->name (),
                inherited::sibling ()));
    return;
  } // end IF

  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_DISCONNECT:
    {
      if (sibling_task_p->connection_)
      {
        // *IMPORTANT NOTE*: regular disconnections must enforce that all
        //                   enqueued outbound data has been dispatched by the
        //                   kernel. This implementation works as long as there
        //                   are no asynchronous upstream module reader tasks
        Stream_IMessageQueue* i_message_queue_p =
          dynamic_cast<Stream_IMessageQueue*> (sibling_task_p->connection_);
        if (!i_message_queue_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Stream_IMessageQueue*>(0x%@), returning\n"),
                      inherited::mod_->name (),
                      sibling_task_p->connection_));
          return;
        } // end IF
        try {
          i_message_queue_p->waitForIdleState ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Net_IStreamConnection_T::waitForIdleState() (id was: %u), continuing\n"),
                      inherited::mod_->name (),
                      sibling_task_p->connection_->id ()));
          return;
        }
      } // end IF
    } // *WARNING*: the control flow falls through here
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      if (sibling_task_p->connection_)
      {
        try {
          sibling_task_p->connection_->close ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Net_IConnection_T::close() (id was: %u), continuing\n"),
                      inherited::mod_->name (),
                      sibling_task_p->connection_->id ()));
          return;
        }
      } // end IF

      break;
    }
    default:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: invalid/unknown control message type (was: %d), continuing\n"),
      //            inherited::mod_->name (),
      //            messageBlock_in.msg_type ()));
      break;
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::Stream_Module_Net_IOWriter_T (ISTREAM_T* stream_in,
                                                                          bool generateSessionMessages_in)
 : inherited (stream_in,                               // stream handle
              false,                                   // auto-start ? (active mode only)
              STREAM_HEADMODULECONCURRENCY_CONCURRENT, // concurrency mode
              generateSessionMessages_in)              // generate session messages ?
 , connection_ (NULL)
 , lock_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::Stream_Module_Net_IOWriter_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::~Stream_Module_Net_IOWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::~Stream_Module_Net_IOWriter_T"));

//  int result = -1;

  //if (timerID_ != -1)
  //{
  //  const void* act_p = NULL;
  //  result =
  //    COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (timerID_,
  //                                                              &act_p);
  //  if (result == -1)
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
  //                timerID_));
  //  else
  //    ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
  //                ACE_TEXT ("cancelled timer (ID: %d)\n"),
  //                timerID_));
  //} // end IF

  if (connection_)
  {
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    AddressType local_address, peer_address;
    connection_->info (handle,
                       local_address, peer_address);

    connection_->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: closed connection to \"%s\" in dtor --> check implementation !\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ())));
    connection_->decrease ();
    connection_ = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                        Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::initialize"));

  bool result = false;

  if (inherited::isInitialized_)
  {
    if (connection_)
    {
      connection_->decrease ();
      connection_ = NULL;
    } // end IF
    inbound_ = true;
  } // end IF

  // *TODO*: remove type inference
  inbound_ = configuration_in.inbound;

  enum Stream_HeadModuleConcurrency concurrency = configuration_in.concurrency;
  const_cast<ConfigurationType&> (configuration_in).concurrency =
    STREAM_HEADMODULECONCURRENCY_CONCURRENT;
  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
  const_cast<ConfigurationType&> (configuration_in).concurrency =
    concurrency;

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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // *TODO*: remove type inferences
  if (inbound_)
  {
    ACE_UNUSED_ARG (message_inout);
    ACE_UNUSED_ARG (passMessageDownstream_out);
  } // end IF
  else
  {
    // *NOTE*: this module dispatches outbound data: enqueue message on
    //         siblings' queue; it is forwarded to the (sub-)streams' head and
    //         notify()d to the reactor/proactor from there
    int result = -1;

    // sanity check(s)
    ACE_ASSERT (message_inout);
    ACE_Message_Block* message_block_p = message_inout->duplicate ();
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    result = inherited::reply (message_block_p, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN) // 108,10058: connection/stream has/is shut/ting
                              //            down
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", returning\n"),
                    inherited::mod_->name ()));

      // clean up
      message_block_p->release ();

      return;
    } // end IF
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
void
Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::handleSessionMessage"));

  int result = -1;
  Stream_Task_t* task_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - the session is being aborted by the user
      //                   - the session is being aborted by some module

      if (connection_)
      {
        // *NOTE*: deactivate the stream head queue so it does not accept new
        //         data
        task_p = inherited::mod_->reader ();
        ACE_ASSERT (task_p);
        while ((ACE_OS::strcmp (task_p->module ()->name (),
                                ACE_TEXT ("ACE_Stream_Head"))       != 0) &&
               (ACE_OS::strcmp (task_p->module ()->name (),
                                ACE_TEXT (STREAM_MODULE_HEAD_NAME)) != 0))
        {
          task_p = task_p->next ();
          if (!task_p) break;
        } // end WHILE
        if (!task_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: no head module reader task found, returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
        ACE_ASSERT (task_p->msg_queue_);
        result = task_p->msg_queue_->deactivate ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *NOTE*: (depending on configuration options) if this is an outbound
      //         connection, there will be two 'session begin' messages:
      //         - one when the stream is started by the connection
      //         - one when the connection stream is appended to the processing
      //           stream

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const SessionDataType& session_data_r =
        inherited::sessionData_->get ();

      if (inherited::configuration_->statisticReportingInterval !=
          ACE_Time_Value::zero)
      {
        // schedule regular statistic collection ?
        if (inherited::timerID_ != -1)
          goto continue_;

        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
        ACE_Event_Handler* handler_p = &(inherited::statisticCollectionHandler_);
        inherited::timerID_ =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                      NULL,                       // argument
                                                                      COMMON_TIME_NOW + interval, // first wakeup time
                                                                      interval);                  // interval
        if (inherited::timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end IF

continue_:
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
      if (!connection_)
      {
        ConnectionManagerType* connection_manager_p =
          ConnectionManagerType::SINGLETON_T::instance ();
        ACE_ASSERT (connection_manager_p);

        connection_ =
          connection_manager_p->get (static_cast<Net_ConnectionId_t> (session_data_r.sessionID));
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (id was: 0x%@), aborting\n"),
                      inherited::mod_->name (),
                      session_data_r.sessionID));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (id was: %d), aborting\n"),
                      inherited::mod_->name (),
                      session_data_r.sessionID));
#endif
          goto error;
        } // end IF
      } // end IF

      // set up reactor/proactor notification
      //typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
      //  dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      //if (!socket_connection_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T>(%@): \"%m\", returning\n"),
      //              connection_));
      //  return;
      //} // end IF
      //typename ConnectorType::STREAM_T& stream_r =
      //  const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
      //Stream_Module_t* module_p = stream_r.head ();
      task_p = inherited::mod_->reader ();
      ACE_ASSERT (task_p);
      //while (task_p->module () != module_p)
      while ((ACE_OS::strcmp (task_p->module ()->name (),
                              ACE_TEXT ("ACE_Stream_Head"))       != 0) &&
             (ACE_OS::strcmp (task_p->module ()->name (),
                              ACE_TEXT (STREAM_MODULE_HEAD_NAME)) != 0))
      {
        task_p = task_p->next ();
        if (!task_p)
          break;
      } // end WHILE
      if (!task_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: no head module reader task found, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (task_p->msg_queue_);
      task_p->msg_queue_->notification_strategy (connection_->notification ());

      goto continue_2;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_2:
      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("%s: disconnected\n"),
//                  inherited::mod_->name ()));
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - processing is complete ('outbound' scenario)
      //                   - the connection has been closed and the processing
      //                     stream has/is finished()-ing (see: handle_close())
      //                   [- the session is being aborted by the user
      //                   - the session is being aborted by some module]

      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const SessionDataType& session_data_r =
        inherited::sessionData_->get ();

      if (inherited::timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
          COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
                                                                    &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to cancel timer (ID: %d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerID_));
        inherited::timerID_ = -1;
      } // end IF

      // sanity check(s)
      //ACE_ASSERT (inherited::configuration_->ilock);
      //int nesting_level = -1;
      //ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (inherited::configuration_->ilock->getLock ());
      if (connection_)
      {
        // wait for data processing to complete ?
        if (session_data_r.aborted)
          goto continue_3;

        // *NOTE*: release the stream lock while waiting, to prevent deadlocks
        //nesting_level = inherited::configuration_->ilock->unlock (true);
        //{ ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock);

          // *NOTE*: there is no need to wait for network stream processing
          //         thread(s) here; the source/target module, the connection
          //         manager (if any) and/or the application itself will do
          //         that
          connection_->waitForCompletion (false); // wait for threads ?
        //} // end lock scope
        //if (nesting_level >= 0)
        //  COMMON_ILOCK_ACQUIRE_N (inherited::configuration_->ilock, nesting_level + 1);
      } // end IF

continue_3:
      // reset reactor/proactor notification; stop dispatching outbound data
      //typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
      //  dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      //if (!socket_connection_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T>(%@): \"%m\", returning\n"),
      //              connection_));
      //  return;
      //} // end IF
      //typename ConnectorType::STREAM_T& stream_r =
      //  const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
      //Stream_Module_t* module_p = stream_r.head ();
      //ACE_ASSERT (inherited::mod_);
      task_p = inherited::mod_->reader ();
      ACE_ASSERT (task_p);
      while ((ACE_OS::strcmp (task_p->module ()->name (),
                              ACE_TEXT ("ACE_Stream_Head"))       != 0) &&
             (ACE_OS::strcmp (task_p->module ()->name (),
                              ACE_TEXT (STREAM_MODULE_HEAD_NAME)) != 0))
      {
        task_p = task_p->next ();
        if (!task_p)
          break;
      } // end WHILE
      if (!task_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: no head module reader task found, returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
      ACE_ASSERT (task_p->msg_queue_);
      task_p->msg_queue_->notification_strategy (NULL);

      if (connection_)
      {
        connection_->decrease ();
        connection_ = NULL;
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
bool
Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             ConfigurationType,
                             StreamControlType,
                             StreamNotificationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType,
                             UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  //  data_out.dataMessages = 0;
  //  data_out.droppedMessages = 0;
  //  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistic information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}
