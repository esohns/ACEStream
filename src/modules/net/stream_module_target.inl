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

#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::Stream_Module_Net_Target_T (bool isPassive_in)
 : inherited ()
 , configuration_ ()
 , iconnector_ (NULL)
 , isInitialized_ (false)
 , isLinked_ (false)
 , isPassive_ (isPassive_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::Stream_Module_Net_Target_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::~Stream_Module_Net_Target_T"));

  int result = -1;

  if (!isPassive_ && configuration_.connection)
  {
    if (isLinked_)
    {
      typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
        dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_.connection);
      if (!socket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                    configuration_.connection));
        return;
      } // end IF
      typename ConnectorType::STREAM_T& stream_r =
        const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
      result = stream_r.unlink ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::unlink(): \"%m\", continuing\n")));
    } // end IF

    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_INET_Addr local_address, peer_address;
    configuration_.connection->info (handle,
                                                local_address, peer_address);
    result = peer_address.addr_to_string (buffer,
                                          sizeof (buffer));
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

    configuration_.connection->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("closed connection to \"%s\" in dtor --> check implementation !\n"),
                buffer));
  } // end IF
  if (configuration_.connection)
    configuration_.connection->decrease ();

  if (iconnector_)
    delete iconnector_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleDataMessage (MessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // check wheter the connection is alive
  if (configuration_.connection)
  {
    Net_Connection_Status status = configuration_.connection->status ();
    if (status != NET_CONNECTION_STATUS_OK)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("connection %d closed (status was: %d), aborting session %d...\n"),
                  configuration_.connection->id (),
                  status,
                  sessionData_->sessionID));

      ACE_ASSERT (sessionData_);
      ACE_ASSERT (sessionData_->lock);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*sessionData_->lock);

      sessionData_->aborted = true;
    } // end IF
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::handleSessionMessage"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      sessionData_ =
        const_cast<SessionDataType*> (session_data_container_r.getData ());
      ACE_ASSERT (sessionData_);

      if (isPassive_)
      {
        // sanity check(s)
        ACE_ASSERT (!configuration_.connection);
        ACE_ASSERT (configuration_.connectionManager);

        // *TODO*: remove type inference
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        handle = reinterpret_cast<ACE_HANDLE> (sessionData_->sessionID);
#else
        handle = static_cast<ACE_HANDLE> (session_data_p->sessionID);
#endif
        configuration_.connection =
          configuration_.connectionManager->get (handle);
        if (!configuration_.connection)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection (handle was: %d), returning\n"),
                      sessionData_->sessionID));

          // clean up
          ACE_ASSERT (sessionData_->lock);
          ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*sessionData_->lock);
          sessionData_->aborted = true;

          return;
        } // end IF
      } // end IF
      else
      {
        ACE_ASSERT (configuration_.socketConfiguration);
        ACE_ASSERT (configuration_.connectionManager);
        ACE_ASSERT (configuration_.socketHandlerConfiguration);

        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        result =
          configuration_.socketConfiguration->peerAddress.addr_to_string (buffer,
                                                                          sizeof (buffer),
                                                                          1);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

        // step1: initialize connection manager
        // *TODO*: remove type inferences
        typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
          configuration_.connectionManager;
        //    typename ConnectionManagerType::CONFIGURATION_T original_configuration;
        //    typename SessionMessageType::USER_DATA_T* user_data_p = NULL;
        //    configuration_in.connectionManager->get (original_configuration,
        //                                             user_data_p);
        //    typename ConnectionManagerType::CONFIGURATION_T configuration =
        //        original_configuration;
        //    configuration.streamConfiguration.cloneModule = false;
        //    configuration.streamConfiguration.deleteModule = false;
        //    configuration.streamConfiguration.module = NULL;
        //    configuration_in.connectionManager->set (configuration,
        //                                             user_data_p);

        // step2: initialize connector
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        if (!iconnector_)
        {
          // *TODO*: remove type inferences
          ACE_NEW_NORETURN (iconnector_,
                            ConnectorType (iconnection_manager_p,
                                           configuration_.streamConfiguration->statisticReportingInterval));
          if (!iconnector_)
          {
            ACE_DEBUG ((LM_CRITICAL,
                        ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
            return;
          } // end IF
        } // end IF
        ACE_ASSERT (iconnector_);
        ACE_ASSERT (configuration_.streamConfiguration);
        bool clone_module, delete_module;
        clone_module =
          configuration_.streamConfiguration->cloneModule;
        delete_module =
          configuration_.streamConfiguration->deleteModule;
        typename ConnectorType::STREAM_T::MODULE_T* module_p =
          configuration_.streamConfiguration->module;
        configuration_.streamConfiguration->cloneModule = false;
        configuration_.streamConfiguration->deleteModule = false;
        configuration_.streamConfiguration->module = NULL;
        if (!iconnector_->initialize (*configuration_.socketHandlerConfiguration))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize connector: \"%m\", returning\n")));
          goto reset;
        } // end IF

        // step3: connect
        ACE_ASSERT (!configuration_.connection);
        // *TODO*: support one-thread operation by scheduling a signal and manually
        //         running the dispatch loop for a limited time...
        handle =
          iconnector_->connect (configuration_.socketConfiguration->peerAddress);
        if (iconnector_->useReactor ())
          configuration_.connection =
            configuration_.connectionManager->get (handle);
        else
        {
          // *TODO*: avoid tight loop here
          ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
          //result = ACE_OS::sleep (timeout);
          //if (result == -1)
          //  ACE_DEBUG ((LM_ERROR,
          //              ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
          //              &timeout));
          ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
          do
          {
            configuration_.connection =
              configuration_.connectionManager->get (configuration_.socketConfiguration->peerAddress);
            if (configuration_.connection)
              break;
          } while (COMMON_TIME_NOW < deadline);
        } // end IF
        if (!configuration_.connection)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to connect to \"%s\", returning\n"),
                      buffer));

          // clean up
          iconnector_->abort ();
          ACE_ASSERT (sessionData_->lock);
          ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*sessionData_->lock);
          sessionData_->aborted = true;

          goto reset;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("connected to \"%s\"...\n"),
                    buffer));

reset:
        configuration_.streamConfiguration->cloneModule =
          clone_module;
        configuration_.streamConfiguration->deleteModule =
          delete_module;
        configuration_.streamConfiguration->module = module_p;
        if (!configuration_.connection)
          break;

        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_.connection);
        if (!socket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                      configuration_.connection));
          goto close;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        ACE_ASSERT (configuration_.stream);
        result = stream_p->link (*configuration_.stream);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("\"%s\": failed to Stream_Base_T::link(\"%s\"): \"%m\", returning\n"),
                      ACE_TEXT (stream_p->name ().c_str ()),
                      ACE_TEXT (configuration_.stream->name ().c_str ())));
          goto close;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("linked i/o streams...\n")));
        isLinked_ = true;

        goto done;

close:
        ACE_ASSERT (configuration_.connection);
        configuration_.connection->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed connection to \"%s\"...\n"),
                    buffer));
        configuration_.connection->decrease ();
        configuration_.connection = NULL;

        break;
      } // end ELSE

done:
      // set session ID
      // *TODO*: remove type inference
      ACE_ASSERT (sessionData_->lock);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*sessionData_->lock);
      sessionData_->sessionID = configuration_.connection->id ();

      break;
    }
    case STREAM_SESSION_END:
    {
      if (configuration_.connection)
      {
        // wait for data (!) processing to complete
        ACE_ASSERT (configuration_.stream);
        configuration_.stream->waitForCompletion (false, // wait for worker(s) ?
                                                  true); // wait for upstream ?

        // *NOTE*: finalize the connection stream state so waitForCompletion()
        //         does not block
        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_.connection);
        if (!socket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                      configuration_.connection));
          if (!isPassive_)
            goto unlink_close;
          else
            goto release;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        stream_p->finished ();

        // *NOTE*: if the connection was closed abruptly, there may well be
        //         undispatched data in the connection stream. Flush it so
        //         waitForCompletion() does not block
        Net_Connection_Status status = configuration_.connection->status ();
        if (status != NET_CONNECTION_STATUS_OK)
          configuration_.stream->flush (true);
        configuration_.connection->waitForCompletion (false); // data only
      } // end IF

      if (!isPassive_ && configuration_.connection)
      {
unlink_close:
        if (isLinked_)
        {
          // *TODO*: finished () (see above) already forwarded a session end
          //         message (on the linked stream)...
          result = inherited::put_next (message_inout, NULL);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

          // clean up
          //message_inout->release ();
          message_inout = NULL;
          passMessageDownstream_out = false;

          typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
            dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_.connection);
          if (!socket_connection_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                        configuration_.connection));
            return;
          } // end IF
          typename ConnectorType::STREAM_T& stream_r =
            const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
          result = stream_r.unlink ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_Base_T::unlink(): \"%m\", continuing\n")));
          else
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("unlinked i/o streams...\n")));
        } // end IF
        isLinked_ = false;

        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        ACE_INET_Addr local_address, peer_address;
        configuration_.connection->info (handle,
                                         local_address, peer_address);
        result = peer_address.addr_to_string (buffer,
                                              sizeof (buffer));
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

        configuration_.connection->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed connection to \"%s\"...\n"),
                    ACE_TEXT (buffer)));
      } // end IF
release:
      if (configuration_.connection)
      {
        configuration_.connection->decrease ();
        configuration_.connection = NULL;
      } // end IF

      sessionData_ = NULL;

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
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::initialize"));

  configuration_ = configuration_in;

  int result = -1;
  ACE_TCHAR buffer[BUFSIZ];

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_.configuration);
  if (configuration_.configuration->socketConfiguration.peerAddress.is_any ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid peer address (was: any), aborting\n")));
    return false;
  } // end IF
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  result =
    configuration_.configuration->socketConfiguration.peerAddress.addr_to_string (buffer,
                                                                                  sizeof (buffer));
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    if (!isPassive_ && configuration_.connection)
    {
      if (isLinked_)
      {
        typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (configuration_.connection);
        if (!socket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", aborting\n"),
                      configuration_.connection));
          return false;
        } // end IF
        typename ConnectorType::STREAM_T& stream_r =
          const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        result = stream_r.unlink ();
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Stream::unlink(): \"%m\", aborting\n")));
          return false;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("unlinked i/o streams...\n")));
      } // end IF
      isLinked_ = false;

      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      ACE_INET_Addr local_address, peer_address;
      configuration_.connection->info (handle,
                                                  local_address, peer_address);
      result = peer_address.addr_to_string (buffer,
                                            sizeof (buffer),
                                            1);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

      configuration_.connection->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed connection to \"%s\"...\n"),
                  buffer));
      configuration_.connection->decrease ();
      configuration_.connection = NULL;
    } // end IF

    if (iconnector_)
    {
      delete iconnector_;
      iconnector_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  isInitialized_ = true;
  isPassive_ = configuration_.passive;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
const ModuleHandlerConfigurationType&
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::get"));

  return configuration_;
}
