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
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::Stream_Module_Net_Source_T (bool isPassive_in)
 : inherited (false, // active object ?
              false, // auto-start ?
              false) // run svc() routine on start ? (passive only)
 //, connection_ (NULL)
 , isInitialized_ (false)
 , isPassive_ (isPassive_in)
 , statisticCollectionHandler_ (ACTION_COLLECT,
                                this,
                                false)
 , timerID_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::Stream_Module_Net_Source_T"));

}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::~Stream_Module_Net_Source_T"));

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

  if (!isPassive_ && inherited::configuration_.connection)
  {
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_INET_Addr local_address, peer_address;
    inherited::configuration_.connection->info (handle,
                                                local_address, peer_address);
    result = peer_address.addr_to_string (buffer,
                                          sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

    inherited::configuration_.connection->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("closed connection to \"%s\" in dtor --> check implementation !\n"),
                ACE_TEXT (buffer)));
    inherited::configuration_.connection->decrease ();
    inherited::configuration_.connection = NULL;
  } // end IF
}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::initialize"));

  int result = -1;
  ACE_TCHAR buffer[BUFSIZ];

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.socketConfiguration);
  ACE_ASSERT (configuration_in.streamConfiguration);

  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result =
    configuration_in.socketConfiguration->peerAddress.addr_to_string (buffer,
                                                                      sizeof (buffer),
                                                                      1);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

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

    if (!isPassive_ && inherited::configuration_.connection)
    {
      inherited::configuration_.connection->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed connection to \"%s\"...\n"),
                  ACE_TEXT (buffer)));
      inherited::configuration_.connection->decrease ();
      inherited::configuration_.connection = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  isPassive_ = configuration_in.passive;
  ACE_ASSERT (configuration_in.connectionManager);
  if (!isPassive_)
  {
    // step1: initialize connection manager
    // *TODO*: remove type inferences
    typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
      configuration_in.connectionManager;
    //typename ConnectionManagerType::CONFIGURATION_T configuration;
    //typename SessionMessageType::USER_DATA_T* user_data_p = NULL;
    //configuration_in.connectionManager->get (configuration,
    //                                         user_data_p);
    //configuration.streamConfiguration.cloneModule = false;
    //configuration.streamConfiguration.deleteModule = false;
    //configuration.streamConfiguration.module = inherited::module ();
    //ACE_ASSERT (configuration.streamConfiguration.module);
    //configuration_in.connectionManager->set (configuration,
    //                                         user_data_p);

    // step2: initialize connector
    // *TODO*: remove type inferences
    ConnectorType connector (iconnection_manager_p,
                             configuration_in.streamConfiguration->statisticReportingInterval);
    //  Stream_IInetConnector_t* iconnector_p = &connector;
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    // *TODO*: reset this later...
    //ACE_ASSERT (inherited::configuration_.configuration->socketHandlerConfiguration.userData);
    //configuration_in.configuration->socketHandlerConfiguration.userData->configuration =
    //  &configuration;
    if (!connector.initialize (*configuration_in.socketHandlerConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
      return false;
    } // end IF

    // step3: connect
    ACE_ASSERT (!inherited::configuration_.connection);
    handle =
      connector.connect (configuration_in.socketConfiguration->peerAddress);
    if (connector.useReactor ())
      inherited::configuration_.connection =
        configuration_in.connectionManager->get (handle);
    else
    {
      ACE_Time_Value one_second (1, 0);
      result = ACE_OS::sleep (one_second);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &one_second));
      inherited::configuration_.connection =
        configuration_in.connectionManager->get (configuration_in.socketConfiguration->peerAddress);
    } // end IF
    if (!inherited::configuration_.connection)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to connect to \"%s\", aborting\n"),
                  ACE_TEXT (buffer)));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("connected to \"%s\"...\n"),
                ACE_TEXT (buffer)));
  } // end IF

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    goto close;
  } // end IF

  return true;

close:
  if (inherited::configuration_.connection)
  {
    inherited::configuration_.connection->close ();
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed connection to \"%s\"...\n"),
                ACE_TEXT (buffer)));
    inherited::configuration_.connection->decrease ();
    inherited::configuration_.connection = NULL;
  } // end IF

  return false;
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
//void
//Stream_Module_Net_Source_T<SessionMessageType,
//                          ProtocolMessageType,
//                          ConfigurationType,
//                          StreamStateType,
//                          SessionDataType,
//                          SessionDataContainerType,
//                          StatisticContainerType,
//                          ConnectionManagerType,
//                          ConnectorType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                             bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::handleDataMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

template <typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::handleSessionMessage"));

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

      if (isPassive_)
      {
        // sanity check(s)
        ACE_ASSERT (!inherited::configuration_.connection);
        ACE_ASSERT (inherited::configuration_.connectionManager);

        const SessionDataContainerType& session_data_container_r =
            message_inout->get ();
        const SessionDataType* session_data_p =
            session_data_container_r.getData ();
        // *TODO*: remove type inference
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_ASSERT (session_data_p->sessionID != reinterpret_cast<unsigned int> (ACE_INVALID_HANDLE));
        inherited::configuration_.connection =
          inherited::configuration_.connectionManager->get (reinterpret_cast<ACE_HANDLE> (session_data_p->sessionID));
#else
        ACE_ASSERT (session_data_p->sessionID != ACE_INVALID_HANDLE);
        inherited::configuration_.connection =
          inherited::configuration_.connectionManager->get (static_cast<ACE_HANDLE> (session_data_p->sessionID));
#endif
        if (!inherited::configuration_.connection)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection (handle was: %d), returning\n"),
                      session_data_p->sessionID));
          return;
        } // end IF
      } // end IF

//      // start profile timer...
//      profile_.start ();

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

      if (!isPassive_ && inherited::configuration_.connection)
      {
        // *IMPORTANT NOTE*: 'this' is the top module of the connection's stream
        //                   (see below). Close()ing the connection would lead
        //                   to an endless loop --> remove it from the stream
        //                   first
        typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (inherited::configuration_.connection);
        if (!socket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                      inherited::configuration_.connection));
          return;
        } // end IF
        typename ConnectorType::STREAM_T& stream_r =
          const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        Stream_Module_t* module_p = inherited::module ();
        ACE_ASSERT (module_p);
        module_p = stream_r.find (module_p->name ());
        if (module_p)
        {
          if (!stream_r.remove (module_p))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_Base_T::remove(\"%s\"): \"%m\", continuing\n"),
                        module_p->name ()));
        } // end IF

        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        ACE_INET_Addr local_address, peer_address;
        inherited::configuration_.connection->info (handle,
                                                    local_address, peer_address);
        result = peer_address.addr_to_string (buffer,
                                              sizeof (buffer));
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

        inherited::configuration_.connection->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed connection to \"%s\"...\n"),
                    ACE_TEXT (buffer)));
      } // end IF
      if (inherited::configuration_.connection)
      {
        inherited::configuration_.connection->decrease ();
        inherited::configuration_.connection = NULL;
      } // end IF

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
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::collect"));

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
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
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
//Stream_Module_Net_Source_T<SessionMessageType,
//                          ProtocolMessageType,
//                          ConfigurationType,
//                          StreamStateType,
//                          SessionDataType,
//                          SessionDataContainerType,
//                          StatisticContainerType,
//                          ConnectionManagerType,
//                          ConnectorType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::svc"));

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
          typename ConnectionManagerType,
          typename ConnectorType>
ProtocolMessageType*
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::allocateMessage"));

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
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Source_T<SessionMessageType,
                           ProtocolMessageType,
                           ConfigurationType,
                           StreamStateType,
                           SessionDataType,
                           SessionDataContainerType,
                           StatisticContainerType,
                           ConnectionManagerType,
                           ConnectorType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::putStatisticMessage"));

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
