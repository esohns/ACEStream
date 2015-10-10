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

#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::Stream_Module_Net_IOWriter_T ()
 : inherited (false, // active object ?
              false, // auto-start ?
              false) // run svc() routine on start ? (passive only)
 , connection_ (NULL)
 , isInitialized_ (false)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::Stream_Module_Net_IOWriter_T"));

}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::~Stream_Module_Net_IOWriter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::~Stream_Module_Net_IOWriter_T"));

  int result = -1;
  ACE_TCHAR buffer[BUFSIZ];

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
      ACE_DEBUG ((LM_WARNING, // this should happen in END_SESSION
                  ACE_TEXT ("cancelled timer (ID: %d)\n"),
                  timerID_));
  } // end IF

  if (connection_)
  {
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    AddressType local_address, peer_address;
    connection_->info (handle,
                       local_address, peer_address);
    result = peer_address.addr_to_string (buffer,
                                          sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

    connection_->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("closed connection to \"%s\" in dtor --> check implementation !\n"),
                ACE_TEXT (buffer)));
    connection_->decrease ();
    connection_ = NULL;
  } // end IF
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
bool
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::initialize"));

  int result = -1;

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    // clean up
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
    } // end IF
    timerID_ = -1;

    if (connection_)
    {
      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::handleDataMessage"));

  if (inherited::configuration_.inbound)
  {
    ACE_UNUSED_ARG (message_inout);
    ACE_UNUSED_ARG (passMessageDownstream_out);
  } // end IF
  else
  {
    // *NOTE*: outbound data: enqueue message on siblings' queue (will be
    //         forwarded to the (sub-)streams' head and notify()d to the
    //         reactor/proactor from there)
    int result = -1;

    // sanity check(s)
    ACE_ASSERT (message_inout);
    ACE_Message_Block* message_block_p = message_inout->duplicate ();
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MessageType::duplicate(): \"%m\", returning\n")));
      return;
    } // end IF
    result = inherited::reply (message_block_p, NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN) // 108,10058: connection/stream has/is shutting down
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::reply(): \"%m\", returning\n")));

      // clean up
      message_block_p->release ();

      return;
    } // end IF
  } // end IF
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_.streamConfiguration);
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      if (inherited::configuration_.streamConfiguration->statisticReportingInterval)
      {
        // schedule regular statistics collection...
        ACE_Time_Value interval (STREAM_STATISTIC_COLLECTION, 0);
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
                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", returning\n")));
          return;
        } // end IF
        //        ACE_DEBUG ((LM_DEBUG,
        //                    ACE_TEXT ("scheduled statistics collecting timer (ID: %d) for interval %#T...\n"),
        //                    timerID_,
        //                    &interval));
      } // end IF

      // *WARNING*: ward consecutive STREAM_SESSION_BEGIN messages here
      //            This happens when using the Stream_Module_Net_Target_T in
      //            active mode. When the connection stream is start()ed (1x)
      //            during connection establishment, a link between the module
      //            stream and the connection stream is created. After the
      //            module has initialized the connection (see
      //            stream_module_target.inl:266), the session message is
      //            forwarded downstream, onto the connections' stream (2x).
      // *NOTE*: the connection handle has already been retrieved when the
      //         second message arrives (see stream_module_io_stream.inl:232)
      if (!connection_)
      {
        // sanity check(s)
        ACE_ASSERT (inherited::configuration_.connectionManager);

        const SessionDataContainerType& session_data_container_r =
          message_inout->get ();
        const SessionDataType* session_data_p =
          session_data_container_r.getData ();
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        handle = reinterpret_cast<ACE_HANDLE> (session_data_p->sessionID);
#else
        handle = static_cast<ACE_HANDLE> (session_data_p->sessionID);
#endif
        connection_ =
          inherited::configuration_.connectionManager->get (handle);
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection (handle was: 0x%@), returning\n"),
                      handle));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection (handle was: %d), returning\n"),
                      handle));
#endif
          return;
        } // end IF

        // set up reactor/proactor notification
        // *TODO*: find a way to retrieve the stream handle here
        //typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
        //  dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        //if (!socket_connection_p)
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
        //              connection_));
        //  return;
        //} // end IF
        //typename ConnectorType::STREAM_T& stream_r =
        //  const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        //Stream_Module_t* module_p = stream_r.head ();
        Stream_Module_t* module_p = inherited::module ();
        ACE_ASSERT (module_p);
        Stream_Task_t* task_p = module_p->reader ();
        ACE_ASSERT (task_p);
        while (ACE_OS::strcmp (module_p->name (),
                               ACE_TEXT ("ACE_Stream_Head")) != 0)
        {
          task_p = task_p->next ();
          if (!task_p) break;
          module_p = task_p->module ();
        } // end WHILE
        //if (!module_p)
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("no head module found, returning\n")));
        //  return;
        //} // end IF
        //Stream_Task_t* task_p = module_p->reader ();
        if (!task_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("no head module reader task found, returning\n")));
          return;
        } // end IF
        Stream_Queue_t* queue_p = task_p->msg_queue ();
        if (!queue_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("no head module reader task queue found, returning\n")));
          return;
        } // end IF
        queue_p->notification_strategy (connection_->notification ());

        //      // start profile timer...
        //      profile_.start ();
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

      if (connection_)
      {
        // *NOTE*: pulse the stream head queue so it does not fill up (the
        //         generator module blocks in some scenarios)
        Stream_Module_t* module_p = inherited::module ();
        ACE_ASSERT (module_p);
        Stream_Task_t* task_p = module_p->reader ();
        ACE_ASSERT (task_p);
        while (ACE_OS::strcmp (module_p->name (),
                               ACE_TEXT ("ACE_Stream_Head")) != 0)
        {
          task_p = task_p->next ();
          if (!task_p) break;
          module_p = task_p->module ();
        } // end WHILE
        //if (!module_p)
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("no head module found, returning\n")));
        //  return;
        //} // end IF
        //Stream_Task_t* task_p = module_p->reader ();
        if (!task_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("no head module reader task found, returning\n")));
          return;
        } // end IF
        Stream_Queue_t* queue_p = task_p->msg_queue ();
        if (!queue_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("no head module reader task queue found, returning\n")));
          return;
        } // end IF
        result = queue_p->pulse ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue::pulse(): \"%m\", continuing\n")));

        // wait for data processing to complete
        //typename ConnectorType::STREAM_T* stream_p = NULL;
        //typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
        //  dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_.connection);
        //if (!socket_connection_p)
        //{
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
        //              configuration_.connection));
        //  if (!isPassive_)
        //    goto unlink_close;
        //  else
        //    goto release;
        //} // end IF
        //stream_p =
        //  &const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        //stream_p->finished ();
        connection_->waitForCompletion (false);

        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      // reset reactor/proactor notification
      Stream_Module_t* module_p = inherited::module ();
      ACE_ASSERT (module_p);
      Stream_Task_t* task_p = module_p->reader ();
      ACE_ASSERT (task_p);
      while (ACE_OS::strcmp (module_p->name (),
                             ACE_TEXT ("ACE_Stream_Head")) != 0)
      {
        task_p = task_p->next ();
        if (!task_p) break;
        module_p = task_p->module ();
      } // end WHILE
      //if (!module_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("no head module found, returning\n")));
      //  return;
      //} // end IF
      //Stream_Task_t* task_p = module_p->reader ();
      if (!task_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("no head module reader task found, returning\n")));
        return;
      } // end IF
      Stream_Queue_t* queue_p = task_p->msg_queue ();
      if (!queue_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("no head module reader task queue found, returning\n")));
        return;
      } // end IF
      queue_p->notification_strategy (NULL);

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
bool
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: initialize container
  //  data_out.dataMessages = 0;
  //  data_out.droppedMessages = 0;
  //  data_out.bytes = 0.0;
  data_out.timestamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::upStream (Stream_Base_t* streamBase_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::upStream"));

  ACE_UNUSED_ARG (streamBase_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Base_t*
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::upStream () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::upStream"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ConnectionManagerType,
//          typename ConnectorType>
//int
//Stream_Module_Net_IOWriter_T<SessionMessageType,
//                         ProtocolMessageType,
//                         ConfigurationType,
//                         StreamStateType,
//                         SessionDataType,
//                         SessionDataContainerType,
//                         StatisticContainerType,
//                         ConnectionManagerType,
//                         ConnectorType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::svc"));

//  int result = -1;
//  typename ConnectionManagerType::CONNECTION_T* connection_p = NULL;
//  // *TODO*: remove type inferences
//  ConnectionManagerType* connection_manager_p =
//      inherited::configuration_.connectionManager;
//  ACE_ASSERT (connection_manager_p);
//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));

//  // step1: process connection data
//  result = inherited::svc ();

//  // step2: close connection
//  if (isOpen_)
//  {
//    result =
//      inherited::configuration_.peerAddress.addr_to_string (buffer,
//                                                            sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));
//    connection_p =
//        connection_manager_p->get (inherited::configuration_.peerAddress);
//    if (!connection_p)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to retrieve connection to \"%s\", continuing\n"),
//                  ACE_TEXT (buffer)));
//    else
//    {
//      connection_p->close ();
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("closed connection to \"%s\"...\n"),
//                  ACE_TEXT (buffer)));
//      connection_p->decrease ();
//    } // end ELSE
//    isOpen_ = false;
//  } // end IF

//  // *NOTE*: unlinking happens during STREAM_SESSION_END processing (see above)

//  return result;
//}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
ProtocolMessageType*
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

  // initialize return value(s)
  ProtocolMessageType* message_out = NULL;

  if (inherited::configuration_.streamConfiguration->messageAllocator)
  {
    try
    {
      // *TODO*: remove type inference
      message_out =
        static_cast<ProtocolMessageType*> (inherited::configuration_.streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
        ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
        requestedSize_in));
      message_out = NULL;
    }
  } // end IF
  else
  {
    ACE_NEW_NORETURN (message_out,
                      ProtocolMessageType (requestedSize_in));
  } // end ELSE
  if (!message_out)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
                requestedSize_in));
  } // end IF

  return message_out;
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType>
bool
Stream_Module_Net_IOWriter_T<SessionMessageType,
                             ProtocolMessageType,
                             ConfigurationType,
                             StreamStateType,
                             SessionDataType,
                             SessionDataContainerType,
                             StatisticContainerType,
                             AddressType,
                             ConnectionManagerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOWriter_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  // *TODO*: remove type inferences
  ACE_ASSERT (inherited::configuration_.streamConfiguration);

  // step1: update session state
  // *TODO*: remove type inferences
  inherited::sessionData_->currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

  // step2: create session data object container
  SessionDataContainerType* session_data_p = NULL;
  ACE_NEW_NORETURN (session_data_p,
                    SessionDataContainerType (inherited::sessionData_,
                    false));
  if (!session_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
    return false;
  } // end IF

  // step3: send the statistic data downstream
  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       session_data_p,
                                       inherited::configuration_.streamConfiguration->messageAllocator);
}

/////////////////////////////////////////

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Module_Net_IOReader_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             ModuleHandlerConfigurationType,
                             SessionDataType,
                             SessionDataContainerType,
                             AddressType,
                             ConnectionManagerType>::Stream_Module_Net_IOReader_T ()
 : inherited ()
 , configuration_ ()
 , connection_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::Stream_Module_Net_IOReader_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename AddressType,
          typename ConnectionManagerType>
Stream_Module_Net_IOReader_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             ModuleHandlerConfigurationType,
                             SessionDataType,
                             SessionDataContainerType,
                             AddressType,
                             ConnectionManagerType>::~Stream_Module_Net_IOReader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::~Stream_Module_Net_IOReader_T"));

//  int result = -1;
//  ACE_TCHAR buffer[BUFSIZ];

  if (connection_)
  {
//    ACE_OS::memset (buffer, 0, sizeof (buffer));
//    ACE_HANDLE handle = ACE_INVALID_HANDLE;
//    AddressType local_address, peer_address;
//    connection_->info (handle,
//                       local_address, peer_address);
//    result = peer_address.addr_to_string (buffer,
//                                          sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

//    connection_->close ();
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("closed connection to \"%s\" in dtor --> check implementation !\n"),
//                ACE_TEXT (buffer)));
    connection_->decrease ();
    connection_ = NULL;
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IOReader_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             ModuleHandlerConfigurationType,
                             SessionDataType,
                             SessionDataContainerType,
                             AddressType,
                             ConnectionManagerType>::handleDataMessage (MessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (isInitialized_);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename AddressType,
          typename ConnectionManagerType>
void
Stream_Module_Net_IOReader_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             ModuleHandlerConfigurationType,
                             SessionDataType,
                             SessionDataContainerType,
                             AddressType,
                             ConnectionManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!connection_);
      ACE_ASSERT (configuration_.connectionManager);

      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      const SessionDataType* session_data_p =
        session_data_container_r.getData ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (session_data_p->sessionID != reinterpret_cast<size_t> (ACE_INVALID_HANDLE));
#else
      ACE_ASSERT (session_data_p->sessionID != static_cast<size_t> (ACE_INVALID_HANDLE));
#endif
      connection_ =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          configuration_.connectionManager->get (reinterpret_cast<ACE_HANDLE> (session_data_p->sessionID));
#else
          configuration_.connectionManager->get (static_cast<ACE_HANDLE> (session_data_p->sessionID));
#endif
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve connection (handle was: %u), returning\n"),
                    session_data_p->sessionID));
        return;
      } // end IF

      break;
    }
    case STREAM_SESSION_END:
    {
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

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename AddressType,
          typename ConnectionManagerType>
bool
Stream_Module_Net_IOReader_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             ModuleHandlerConfigurationType,
                             SessionDataType,
                             SessionDataContainerType,
                             AddressType,
                             ConnectionManagerType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::initialize"));

  configuration_ = configuration_in;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename AddressType,
          typename ConnectionManagerType>
const ModuleHandlerConfigurationType&
Stream_Module_Net_IOReader_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             ModuleHandlerConfigurationType,
                             SessionDataType,
                             SessionDataContainerType,
                             AddressType,
                             ConnectionManagerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_IOReader_T::get"));

  return configuration_;
}
