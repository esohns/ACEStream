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

#include "net_common.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::Stream_Module_Net_Target_T (bool isPassive_in)
 : inherited ()
 , configuration_ (NULL)
 , connection_ (NULL)
 , connector_ (NULL,
               ACE_Time_Value::zero)
 , sessionData_ (NULL)
 , isInitialized_ (false)
 , isLinked_ (false)
 , isPassive_ (false)
 , lock_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::Stream_Module_Net_Target_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::~Stream_Module_Net_Target_T"));

  int result = -1;
  typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p = NULL;

  connector_.abort ();

  if (isLinked_)
  {
    // sanity check(s)
    ACE_ASSERT (connection_);

    isocket_connection_p =
      dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
    if (!isocket_connection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", continuing\n"),
                  connection_));
      goto close;
    } // end IF
    typename ConnectorType::STREAM_T& stream_r =
      const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
    result = stream_r.unlink ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::unlink(): \"%m\", continuing\n")));
  } // end IF

close:
  if (!isPassive_ && connection_)
  {
    ACE_TCHAR buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_INET_Addr local_address, peer_address;
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
                buffer));
  } // end IF

  if (connection_)
    connection_->decrease ();

  if (sessionData_)
    sessionData_->decrease ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleDataMessage (MessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::handleSessionMessage"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_->streamConfiguration);
      ACE_ASSERT (!connection_);
      ACE_ASSERT (!sessionData_);

      sessionData_ =
          &const_cast<SessionDataContainerType&> (message_inout->get ());
      sessionData_->increase ();
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (sessionData_->get ());

      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        (configuration_->connectionManager ? configuration_->connectionManager
                                           : NULL);
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;

      if (configuration_->connection)
      {
        connection_ = configuration_->connection;
        connection_->increase ();
        isPassive_ = true;

        goto link;
      } // end IF

      // *TODO*: remove type inference
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      handle = reinterpret_cast<ACE_HANDLE> (session_data_r.sessionID);
#else
      handle = static_cast<ACE_HANDLE> (session_data_r.sessionID);
#endif
      if (handle != ACE_INVALID_HANDLE)
      {
        // sanity check(s)
        ACE_ASSERT (iconnection_manager_p);

        connection_ = iconnection_manager_p->get (handle);
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection (handle was: 0x%@), aborting\n"),
                      handle));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to retrieve connection (handle was: %d), aborting\n"),
                      handle));
#endif
          goto error;
        } // end IF

        goto link;
      } // end IF

      // step1: initialize connection manager
      // *TODO*: remove type inferences
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

      // sanity check(s)
      ACE_ASSERT (configuration_->socketHandlerConfiguration);
      ACE_ASSERT (configuration_->streamConfiguration);

      // step2: initialize connector
      // *NOTE*: the stream configuration may contain a module handle that is
      //         meant to be the final module of this processing stream. As
      //         the connection stream will be prepended to this pipeline, the
      //         connection should not enqueue that same module again
      //         --> temporarily 'hide' the module handle, if any
      // *TODO*: remove this ASAP
      bool clone_module, delete_module;
      clone_module = configuration_->streamConfiguration->cloneModule;
      delete_module = configuration_->streamConfiguration->deleteModule;
      module_p = configuration_->streamConfiguration->module;
      configuration_->streamConfiguration->cloneModule = false;
      configuration_->streamConfiguration->deleteModule = false;
      configuration_->streamConfiguration->module = NULL;

      if (!iconnector_p->initialize (*configuration_->socketHandlerConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
        goto reset;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (configuration_->socketConfiguration);

      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      result =
        configuration_->socketConfiguration->address.addr_to_string (buffer,
                                                                     sizeof (buffer),
                                                                     1);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

      // step3: connect
      // *TODO*: support single-thread operation (e.g. by scheduling a signal
      //         and manually running the dispatch loop for a limited period)
      handle =
         iconnector_p->connect (configuration_->socketConfiguration->address);
      if (iconnector_p->useReactor ())
        connection_ = iconnection_manager_p->get (handle);
      else
      {
        // step3a: wait for the connection to register with the manager
        ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
        ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
        do
        {
          connection_ =
            iconnection_manager_p->get (configuration_->socketConfiguration->address);
          // *TODO*: avoid tight loop here
          if (connection_)
            break;
        } while (COMMON_TIME_NOW < deadline);
        if (!connection_)
        {
          ACE_ASSERT (COMMON_TIME_NOW >= deadline);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to connect to \"%s\" (timeout: %#T), aborting\n"),
                      buffer,
                      &timeout));

          // clean up
          iconnector_p->abort ();

          goto reset;
        } // end IF

        // step3b: wait for the connection to finish initializing
        do
        {
          status = connection_->status ();
          // *TODO*: avoid tight loop here
          if (status != NET_CONNECTION_STATUS_INVALID)
            break;
        } while (COMMON_TIME_NOW < deadline);
      } // end IF
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to connect to \"%s\", aborting\n"),
                    buffer));

        // clean up
        iconnector_p->abort ();

        goto reset;
      } // end IF
      status = connection_->status ();
      if (status != NET_CONNECTION_STATUS_OK)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize connection to \"%s\" (status was: %d), aborting\n"),
                    buffer,
                    status));

        // clean up
        connection_->close ();
        connection_->decrease ();
        connection_ = NULL;

        goto reset;
      } // end IF
      // step3a/c: wait for the connection stream to finish initializing
      isocket_connection_p =
        dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      if (!isocket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T>(0x%@), aborting\n"),
                    connection_));

        // clean up
        connection_->close ();
        connection_->decrease ();
        connection_ = NULL;

        goto reset;
      } // end IF
      isocket_connection_p->wait (STREAM_STATE_RUNNING,
                                  NULL); // <-- block
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("connected to \"%s\"...\n"),
                  buffer));

reset:
      configuration_->streamConfiguration->cloneModule = clone_module;
      configuration_->streamConfiguration->deleteModule = delete_module;
      configuration_->streamConfiguration->module = module_p;

      if (!connection_)
        goto error;

link:
      if (!isocket_connection_p)
      {
        // *NOTE*: --> passive mode
        isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        if (!isocket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T>(0x%@): \"%m\", aborting\n"),
                      configuration_->connection));
          goto error;
        } // end IF

        // *NOTE*: To avoid a subtle race condition (passive mode), before
        //         linking, wait for the connection stream to initialize (this
        //         ensures that all modules have been pushed)
        // *TODO*: waiting for STREAM_STATE_INITIALIZED should suffice here
        if (!isocket_connection_p->wait (STREAM_STATE_RUNNING,
                                         NULL))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Net_ISocketConnection_T::wait(STREAM_STATE_RUNNING), aborting\n")));
          goto error;
        } // end IF
      } // end IF

      stream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
      ACE_ASSERT (configuration_->stream);
      result = stream_p->link (*configuration_->stream);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("\"%s\": failed to Stream_Base_T::link(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (stream_p->name ().c_str ()),
                    ACE_TEXT (configuration_->stream->name ().c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("linked i/o streams\n")));
      isLinked_ = true;
      configuration_->stream->dump_state ();

      goto done;

error:
      if (!isPassive_ && connection_)
      {
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed connection to \"%s\"...\n"),
                    buffer));
      } // end IF
      if (connection_)
      {
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      {
        ACE_ASSERT (session_data_r.lock);
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*session_data_r.lock);
        session_data_r.aborted = true;
      } // end lock scope

      break;

done:
      // set session ID
      ACE_ASSERT (connection_);
      // *TODO*: remove type inference
      ACE_ASSERT (session_data_r.lock);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard_2 (*session_data_r.lock);
      session_data_r.sessionID = connection_->id ();

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;

      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

      if (!isPassive_ && connection_)
      {
        // wait for data (!) processing to complete
//        ACE_ASSERT (configuration_->stream);
        isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        if (!isocket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T> (0x%@): \"%m\", continuing\n"),
                      connection_));
          goto unlink;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());

        //// *NOTE*: if the connection was closed abruptly, there may well be
        ////         undispatched data in the connection stream. Flush it so
        ////         waitForCompletion() (see below) does not block
        //Net_Connection_Status status = configuration_->connection->status ();
        //if (status != NET_CONNECTION_STATUS_OK)
        //  stream_p->flush (true);
        //configuration_->stream->waitForCompletion (false, // wait for worker(s) ?
        //                                          true); // wait for upstream ?

        // *NOTE*: finalize the (connection) stream state so waitForCompletion()
        //         does not block forever
        // *TODO*: this shouldn't be necessary (--> only wait for data to flush)
        stream_p->finished (false); // finish upstream ?
        connection_->waitForCompletion (false); // data only
      } // end IF

unlink:
      if (isLinked_)
      {
        // sanity check(s)
        ACE_ASSERT (connection_);
  
        // *TODO*: if active (!) finished() (see above) already forwarded a
        //         session end message (on the linked stream)
        // *TODO*: this prevents the GTK close_all button from working
        //         properly in the integration test, as the connection has not
        //         yet been released when session end is notified to the
        //         application
        result = inherited::put_next (message_inout, NULL);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

        // clean up
        message_inout = NULL;
        passMessageDownstream_out = false;

        if (!isocket_connection_p)
        {
          isocket_connection_p =
              dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
          if (!isocket_connection_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T> (0x%@): \"%m\", continuing\n"),
                        connection_));
            goto close;
          } // end IF
        } // end IF
        if (!stream_p)
          stream_p =
            &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
        result = stream_p->unlink ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Base_T::unlink(): \"%m\", continuing\n")));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("unlinked i/o streams\n")));
      } // end IF
      isLinked_ = false;

close:
      if (!isPassive_ && connection_)
      {
        ACE_TCHAR buffer[BUFSIZ];
        ACE_OS::memset (buffer, 0, sizeof (buffer));
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        ACE_INET_Addr local_address, peer_address;
        connection_->info (handle,
                           local_address, peer_address);
        result = peer_address.addr_to_string (buffer,
                                              sizeof (buffer));
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

        // *NOTE*: potential race condition here
        Net_Connection_Status status = connection_->status ();
        if (status == NET_CONNECTION_STATUS_OK)
        {
          connection_->close ();
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("closed connection to \"%s\"...\n"),
                      buffer));
        } // end IF
      } // end IF

      if (connection_)
      {
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  int result = -1;
  ACE_TCHAR buffer[BUFSIZ];
  typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p = NULL;

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_->socketConfiguration);
  if (configuration_->socketConfiguration->address.is_any ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid peer address (was: any), aborting\n")));
    return false;
  } // end IF
  //ACE_OS::memset (buffer, 0, sizeof (buffer));
  //result =
  //  configuration_->socketConfiguration->peerAddress.addr_to_string (buffer,
  //                                                                   sizeof (buffer));
  //if (result == -1)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    if (isLinked_ && connection_)
    {
      isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      if (!isocket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", continuing\n"),
                    connection_));
        goto close;
      } // end IF
      typename ConnectorType::STREAM_T& stream_r =
        const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
      result = stream_r.unlink ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::unlink(): \"%m\", aborting\n")));
        return false;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("unlinked i/o streams\n")));
    } // end IF
    isLinked_ = false;

close:
    if (!isPassive_ && connection_)
    {
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      ACE_INET_Addr local_address, peer_address;
      connection_->info (handle,
                         local_address, peer_address);
      result = peer_address.addr_to_string (buffer,
                                            sizeof (buffer),
                                            1);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed connection to \"%s\"...\n"),
                  buffer));
    } // end IF

    if (connection_)
    {
      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  isInitialized_ = true;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
const ConfigurationType&
Stream_Module_Net_Target_T<SessionMessageType,
                           MessageType,
                           ConfigurationType,
                           SessionDataType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
