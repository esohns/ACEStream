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

#include "net_common.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           AddressType,
                           ConnectionManagerType,
                           ConnectorType>::Stream_Module_Net_Source_T ()
 : inherited ()
 , sessionData_ (NULL)
 , connector_ (NULL,
               ACE_Time_Value::zero)
 , connection_ (NULL)
 , isLinked_ (false)
 , isPassive_ (false)
 , lock_ ()
 , sessionEndInProgress_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::Stream_Module_Net_Source_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           AddressType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::~Stream_Module_Net_Source_T"));

  int result = -1;

  if (sessionData_)
    sessionData_->decrease ();

  if (connection_)
  {
    if (isLinked_)
    {
      typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
        dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      if (!socket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                    connection_));
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
    connection_->decrease ();
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Source_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           AddressType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    if (connection_)
    {
      if (isLinked_)
      {
        typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        if (!socket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", aborting\n"),
                      connection_));
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
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: unlinked i/o streams\n"),
                    inherited::mod_->name ()));
      } // end IF
      isLinked_ = false;

      ACE_TCHAR buffer[BUFSIZ];
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
      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    sessionEndInProgress_ = false;

    inherited::isInitialized_ = false;
  } // end IF

  // *TODO*: remove type inferences
  isPassive_ = configuration_in.passive;
  if (isPassive_)
  {
    if (configuration_in.connection)
    {
      configuration_in.connection->increase ();
      connection_ = configuration_in.connection;
    } // end IF
  } // end IF

  return inherited::initialize (configuration_in);
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Source_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           AddressType,
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::mod_);

  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      sessionData_ =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->get ());
      sessionData_->increase ();
      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (sessionData_->get ());

      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        (inherited::configuration_->connectionManager ? inherited::configuration_->connectionManager
                                                      : NULL);
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p = NULL;
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;

      if (isPassive_)
      {
        // sanity check(s)
        if (connection_)
          goto continue_; // --> using configured connection

        // --> using session connection

        // sanity check(s)
        ACE_ASSERT (iconnection_manager_p);

        // *TODO*: remove type inference
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        handle = reinterpret_cast<ACE_HANDLE> (session_data_r.sessionID);
#else
        handle = static_cast<ACE_HANDLE> (session_data_r.sessionID);
#endif
        ACE_ASSERT (handle != ACE_INVALID_HANDLE);
        connection_ = iconnection_manager_p->get (handle);
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: 0x%@), returning\n"),
                      inherited::mod_->name (),
                      handle));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: %d), returning\n"),
                      inherited::mod_->name (),
                      handle));
#endif
          return;
        } // end IF

        goto continue_;
      } // end IF

      // --> open new connection

      ACE_ASSERT (inherited::configuration_->socketConfiguration);
      ACE_ASSERT (inherited::configuration_->socketHandlerConfiguration);

      // debug information
      result =
        inherited::configuration_->socketConfiguration->address.addr_to_string (buffer,
                                                                                sizeof (buffer),
                                                                                1);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n"),
                    inherited::mod_->name ()));

      //// step1: initialize connection manager
      //    typename ConnectionManagerType::CONFIGURATION_T original_configuration;
      //    typename SessionMessageType::USER_DATA_T* user_data_p = NULL;
      //    configuration_in.connectionManager->get (original_configuration,
      //                                             user_data_p);
      //    typename ConnectionManagerType::CONFIGURATION_T configuration =
      //        original_configuration;
      //    configuration.streamConfiguration.cloneModule = false;
      //    configuration.streamConfiguration.deleteModule = false;
      //    configuration.streamConfiguration.module = NULL;
      //    iconnection_manager_p->set (configuration,
      //                                user_data_p);

      // step2: initialize connector
      ACE_ASSERT (inherited::configuration_->streamConfiguration);
      bool clone_module, delete_module;
      clone_module =
        inherited::configuration_->streamConfiguration->cloneModule;
      delete_module =
        inherited::configuration_->streamConfiguration->deleteModule;
      module_p =
        inherited::configuration_->streamConfiguration->module;
      inherited::configuration_->streamConfiguration->cloneModule = false;
      inherited::configuration_->streamConfiguration->deleteModule = false;
      inherited::configuration_->streamConfiguration->module = NULL;
      if (!iconnector_p->initialize (*inherited::configuration_->socketHandlerConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ConnectorType::INTERFACE_T::initialize(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto reset;
      } // end IF

      // step3: connect
      ACE_ASSERT (!inherited::configuration_->connection);
      handle =
        iconnector_p->connect (inherited::configuration_->socketConfiguration->address);
      if (iconnector_p->useReactor ())
        connection_ =
          inherited::configuration_->connectionManager->get (handle);
      else
      {
        // step1: wait for the connection to register with the manager
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
          connection_ =
            iconnection_manager_p->get (inherited::configuration_->socketConfiguration->address);
          if (connection_)
          {
            // step2: wait for the connection to finish initializing
            Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
            do
            {
              status = connection_->status ();
              if (status == NET_CONNECTION_STATUS_OK)
                break;
            } while (COMMON_TIME_NOW < deadline);

            if (status == NET_CONNECTION_STATUS_OK)
            {
              // step3: wait for the connection stream to finish initializing
              typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p =
                dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
              if (!isocket_connection_p)
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T>(0x%@), returning\n"),
                            inherited::mod_->name (),
                            connection_));
                goto reset;
              } // end IF
              isocket_connection_p->wait (STREAM_STATE_RUNNING,
                                          NULL); // <-- block
              break;
            } // end IF
          } // end IF
        } while (COMMON_TIME_NOW < deadline);
      } // end IF
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to \"%s\", returning\n"),
                    inherited::mod_->name (),
                    buffer));

        // clean up
        iconnector_p->abort ();

        goto reset;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: connected to %s...\n"),
                  inherited::mod_->name (),
                  buffer));

reset:
      inherited::configuration_->streamConfiguration->cloneModule =
        clone_module;
      inherited::configuration_->streamConfiguration->deleteModule =
        delete_module;
      inherited::configuration_->streamConfiguration->module = module_p;
      if (!connection_)
        goto error;

      // link processing streams
      isocket_connection_p =
        dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      if (!isocket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_ISocketConnection_T> (0x%@): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF
      stream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
      ACE_ASSERT (inherited::configuration_->stream);
      result = inherited::configuration_->stream->link (*stream_p);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Stream::link(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: linked i/o streams\n"),
                  inherited::mod_->name ()));
      isLinked_ = true;
      //stream_p->dump_state ();

      // update session data in the session message
      // *WARNING*: this works as long as the STREAM_SESSION_LINK message has
      //            already been received at this point (see below; OK if
      //            upstream is completely synchronous)
      ACE_ASSERT (sessionData_);
      sessionData_->increase ();
      session_data_container_p = sessionData_;
      message_inout->initialize (STREAM_SESSION_MESSAGE_BEGIN,
                                 session_data_container_p,
                                 &const_cast<typename SessionMessageType::USER_DATA_T&> (message_inout->data ()));

      goto continue_;

error:
      if (connection_)
      {
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s...\n"),
                    inherited::mod_->name (),
                    buffer));
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      // sanity check(s)
      ACE_ASSERT (connection_);

      // re-set session ID, retain state information
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_r.lock);
      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
      session_data_r.connectionState =
        &const_cast<typename ConnectionManagerType::STATE_T&> (connection_->state ());
      session_data_r.sessionID = connection_->id ();

      break;
    }
    // *NOTE*: the stream has been link()ed (see above), the message contains
    //         the (merged) upstream session data --> retain a reference
    case STREAM_SESSION_MESSAGE_LINK:
    {
      // *IMPORTANT NOTE*: in case the session has been aborted asynchronously,
      //                   the 'session end' message may already have been
      //                   processed at this point (concurrent scenario).
      // *TODO*: - implement a state machine for network sessions to help avoid
      //           race conditions
      //         - flush the processing stream thoroughly on end-of-session
      // sanity check(s)
      if (!sessionData_)
        break;

      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        sessionData_->get ();

      typename SessionMessageType::DATA_T* session_data_container_p =
        &const_cast<typename SessionMessageType::DATA_T&> (message_inout->get ());
      session_data_container_p->increase ();

      // *IMPORTANT NOTE*: although reuse of the upstream session data is
      //                   warranted, it may not be safe, as the connection
      //                   might go away, destroying the session data lock
      //                   --> use 'this' streams' session data lock instead
      typename SessionMessageType::DATA_T::DATA_T& session_data_2 =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->get ());
      session_data_2.lock = session_data_r.lock;

      sessionData_->decrease ();
      sessionData_ = session_data_container_p;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      const typename SessionMessageType::DATA_T& session_data_container_r =
        const_cast<typename SessionMessageType::DATA_T&> (message_inout->get ());
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        session_data_container_r.get ();

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      // *NOTE*: when the connection is (asynchronously) closed by the peer, the
      //         connection handler will finished() the connection processing
      //         stream. If it is linked to another processing stream at this
      //         point (i.e. 'this'), the 'session end' message is propagated.
      //         Note that when the (linked) processing stream itself is
      //         finished() (see below), it propagates a second session end
      //         message. Handle this situation here; there is nothing more to
      //         do in this case
      {
        ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

        if (sessionEndInProgress_)
        {
          passMessageDownstream_out = false;
          message_inout->release ();

          break; // done
        } // end IF

        sessionEndInProgress_ = true;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->ilock);
      ACE_Reverse_Lock<ACE_SYNCH_MUTEX> reverse_lock (inherited::configuration_->ilock->getLock ());
      if (connection_)
      {
        // wait for data (!) processing to complete
        ACE_ASSERT (inherited::configuration_->stream);
        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        if (!isocket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T> (0x%@): \"%m\", returning\n"),
                      connection_));
          goto error_2;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());

        // *NOTE*: if:
        //         - the connection was closed abruptly (peer reset)
        //         - the session has been aborted (initialization/processing
        //           failed, user abort, tea is ready, ...)
        //         there may be undispatched data in the connection stream
        //         --> flush() so waitForCompletion() (see below) does not block
        // *TODO*: this does not work reliably (race condition).
        //         Also:
        //         - verify role of the connection; do not flush server-side
        //           inbound data
        Net_Connection_Status status = connection_->status ();
        if ((status != NET_CONNECTION_STATUS_OK) ||
            session_data_r.aborted)
          stream_p->flush (true,   // flush inbound data ?
                           false,  // flush session messages ?
                           false); // flush upstream (if any) ?

        // *NOTE*: finalize the processing stream state so
        //         waitForCompletion() (see below) does not block. Note that the
        //         stream may already have finished() at this point (in fact,
        //         this may be the finished() callstack)
        // *TODO*: which scenarios are these ? (initialization failed, ...)
        // *NOTE*: this (probably) also wakes up the (main) thread
        // *TODO*: iff the connection has been (asynchronously) closed by the
        //         peer, there is no need to signal completion here
        inherited::configuration_->stream->finished (false); // finish upstream ?

        if (session_data_r.aborted)
          goto continue_2;

        {
          ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock);
          //inherited::configuration_->ilock->unlock ();
          inherited::configuration_->stream->waitForCompletion (false, // wait for worker(s) ?
                                                                true); // wait for upstream ?
          //inherited::configuration_->ilock->lock ();
        } // end lock scope
      } // end IF

continue_2:
error_2:
      if (connection_)
      {
        if (isLinked_)
        {
          // sanity check(s)
          ACE_ASSERT (inherited::configuration_->stream);

          result = inherited::configuration_->stream->unlink ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Stream::unlink(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          else
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: unlinked i/o streams\n"),
                        inherited::mod_->name ()));
          //inherited::configuration_->stream->dump_state ();
        } // end IF
        isLinked_ = false;

        if (!isPassive_)
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
                        ACE_TEXT ("%s: failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n"),
                        inherited::mod_->name ()));

          // *NOTE*: closing the connection entails interaction with the event
          //         dispatcher (see: handle_close() --> remove_handler()),
          //         which may in turn be trying to deliver data to the
          //         processing stream. (Depending on the dispatcher
          //         implementation,) this can lead to a deadlock situation
          //         --> relinquish the processing stream lock temporarily
          {
            ACE_GUARD (ACE_Reverse_Lock<ACE_SYNCH_MUTEX>, aGuard, reverse_lock);

            connection_->close ();
          } // end lock scope
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed connection to %s...\n"),
                      inherited::mod_->name (),
                      buffer));
        } // end IF

        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      inherited::shutdown ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

template <typename LockType,
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
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_SourceH_T<LockType,
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
                           ConnectionManagerType,
                           ConnectorType>::Stream_Module_Net_SourceH_T (LockType* lock_in,
                                                                       bool isPassive_in)
 : inherited (lock_in, // lock handle
              false,   // auto-start ?
              true)    // generate sesssion messages ?
 , connector_ (NULL,
               ACE_Time_Value::zero)
 , connection_ (NULL)
 , isLinked_ (false)
 , isPassive_ (isPassive_in)
 , lock_ ()
 , sessionEndInProgress_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::Stream_Module_Net_SourceH_T"));

}

template <typename LockType,
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
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_SourceH_T<LockType,
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
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_SourceH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::~Stream_Module_Net_SourceH_T"));

  int result = -1;

  if (connection_)
  {
    if (isLinked_)
    {
      typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
        dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      if (!socket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", returning\n"),
                    connection_));
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
    connection_->decrease ();
  } // end IF
}

template <typename LockType,
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
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_SourceH_T<LockType,
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
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::initialize"));

  bool result = false;
  int result_2 = -1;

  if (inherited::initialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    if (connection_)
    {
      if (isLinked_)
      {
        typename ConnectorType::ISOCKET_CONNECTION_T* socket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        if (!socket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", aborting\n"),
                      connection_));
          return false;
        } // end IF
        typename ConnectorType::STREAM_T& stream_r =
          const_cast<typename ConnectorType::STREAM_T&> (socket_connection_p->stream ());
        result_2 = stream_r.unlink ();
        if (result_2 == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Stream::unlink(): \"%m\", aborting\n")));
          return false;
        } // end IF
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: unlinked i/o streams\n"),
                    inherited::mod_->name ()));
      } // end IF
      isLinked_ = false;

      ACE_TCHAR buffer[BUFSIZ];
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      ACE_INET_Addr local_address, peer_address;
      connection_->info (handle,
                         local_address, peer_address);
      result_2 = peer_address.addr_to_string (buffer,
                                              sizeof (buffer),
                                              1);
      if (result_2 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));

      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed connection to \"%s\"...\n"),
                  buffer));
      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    sessionEndInProgress_ = false;
  
    inherited::initialized_ = false;
  } // end IF

  result = inherited::initialize (configuration_in);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF
  // *NOTE*: data is fed into the stream from outside, as it arrives
  //         --> do not run svc() on start()
  inherited::runSvcOnStart_ = false;

  isPassive_ = inherited::configuration_->passive;
  if (isPassive_)
  {
    if (inherited::configuration_->connection)
    {
      inherited::configuration_->connection->increase ();
      connection_ = inherited::configuration_->connection;
    } // end IF
  } // end IF

  return result;
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
//Stream_Module_Net_SourceH_T<SessionMessageType,
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
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::handleDataMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

template <typename LockType,
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
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_SourceH_T<LockType,
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
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::initialized_);
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  ACE_TCHAR buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // schedule regular statistic collection ?
      if (inherited::configuration_->streamConfiguration->statisticReportingInterval !=
          ACE_Time_Value::zero)
      {
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
        ACE_ASSERT (inherited::timerID_ == -1);
        ACE_Event_Handler* handler_p =
            &(inherited::statisticCollectionHandler_);
        inherited::timerID_ =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                  // event handler
                                                                        NULL,                       // argument
                                                                        COMMON_TIME_NOW + interval, // first wakeup time
                                                                        interval);                  // interval
        if (inherited::timerID_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Timer_Manager::schedule_timer(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          return;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    inherited::mod_->name (),
//                    inherited::timerID_,
//                    &interval));
      } // end IF

      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        (inherited::configuration_->connectionManager ? inherited::configuration_->connectionManager
                                                      : NULL);
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p = NULL;
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      SessionDataContainerType* session_data_container_p = NULL;

      if (isPassive_)
      {
        // sanity check(s)
        if (connection_)
          goto continue_; // --> using configured connection

        // --> using session connection

        // sanity check(s)
        ACE_ASSERT (iconnection_manager_p);

        // *TODO*: remove type inference
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        handle = reinterpret_cast<ACE_HANDLE> (session_data_r.sessionID);
#else
        handle = static_cast<ACE_HANDLE> (session_data_r.sessionID);
#endif
        ACE_ASSERT (handle != ACE_INVALID_HANDLE);
        connection_ = iconnection_manager_p->get (handle);
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: 0x%@), returning\n"),
                      inherited::mod_->name (),
                      handle));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: %d), returning\n"),
                      inherited::mod_->name (),
                      handle));
#endif
          return;
        } // end IF

        goto continue_;
      } // end IF

      // --> open new connection

      ACE_ASSERT (inherited::configuration_->socketConfiguration);
      ACE_ASSERT (inherited::configuration_->socketHandlerConfiguration);

      // debug information
      result =
        inherited::configuration_->socketConfiguration->address.addr_to_string (buffer,
                                                                                sizeof (buffer),
                                                                                1);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n"),
                    inherited::mod_->name ()));

      //// step1: initialize connection manager
      //    typename ConnectionManagerType::CONFIGURATION_T original_configuration;
      //    typename SessionMessageType::USER_DATA_T* user_data_p = NULL;
      //    configuration_in.connectionManager->get (original_configuration,
      //                                             user_data_p);
      //    typename ConnectionManagerType::CONFIGURATION_T configuration =
      //        original_configuration;
      //    configuration.streamConfiguration.cloneModule = false;
      //    configuration.streamConfiguration.deleteModule = false;
      //    configuration.streamConfiguration.module = NULL;
      //    iconnection_manager_p->set (configuration,
      //                                user_data_p);

      // step2: initialize connector
      ACE_ASSERT (inherited::configuration_->streamConfiguration);
      bool clone_module, delete_module;
      clone_module =
          inherited::configuration_->streamConfiguration->cloneModule;
      delete_module =
          inherited::configuration_->streamConfiguration->deleteModule;
      module_p =
          inherited::configuration_->streamConfiguration->module;
      inherited::configuration_->streamConfiguration->cloneModule = false;
      inherited::configuration_->streamConfiguration->deleteModule = false;
      inherited::configuration_->streamConfiguration->module = NULL;
      if (!iconnector_p->initialize (*inherited::configuration_->socketHandlerConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ConnectorType::INTERFACE_T::initialize(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto reset;
      } // end IF

      // step3: connect
      ACE_ASSERT (!inherited::configuration_->connection);
      handle =
          iconnector_p->connect (inherited::configuration_->socketConfiguration->address);
      if (iconnector_p->useReactor ())
        connection_ =
            inherited::configuration_->connectionManager->get (handle);
      else
      {
        // step1: wait for the connection to register with the manager
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
          connection_ =
            iconnection_manager_p->get (inherited::configuration_->socketConfiguration->address);
          if (connection_)
          {
            // step2: wait for the connection to finish initializing
            Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
            do
            {
              status = connection_->status ();
              if (status == NET_CONNECTION_STATUS_OK)
                break;
            } while (COMMON_TIME_NOW < deadline);

            if (status == NET_CONNECTION_STATUS_OK)
            {
              // step3: wait for the connection stream to finish initializing
              typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p =
                dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
              if (!isocket_connection_p)
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T>(0x%@), returning\n"),
                            inherited::mod_->name (),
                            connection_));
                goto reset;
              } // end IF
              isocket_connection_p->wait (STREAM_STATE_RUNNING,
                                          NULL); // <-- block
              break;
            } // end IF
          } // end IF
        } while (COMMON_TIME_NOW < deadline);
      } // end IF
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to \"%s\", returning\n"),
                    inherited::mod_->name (),
                    buffer));

        // clean up
        iconnector_p->abort ();

        goto reset;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: connected to %s...\n"),
                  inherited::mod_->name (),
                  buffer));

reset:
      inherited::configuration_->streamConfiguration->cloneModule =
          clone_module;
      inherited::configuration_->streamConfiguration->deleteModule =
          delete_module;
      inherited::configuration_->streamConfiguration->module = module_p;
      if (!connection_)
        goto error;

      // link processing streams
      isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
      if (!isocket_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_ISocketConnection_T> (0x%@): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF
      stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
      ACE_ASSERT (inherited::configuration_->stream);
      result = inherited::configuration_->stream->link (*stream_p);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Stream::link(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: linked i/o streams\n"),
                  inherited::mod_->name ()));
      isLinked_ = true;
      //stream_p->dump_state ();

      // update session data in the session message
      // *WARNING*: this works only if the STREAM_SESSION_LINK message has been
      //            received by now (see below; OK if upstream is completely
      //            synchronous)
      session_data_container_p = inherited::sessionData_;
      message_inout->initialize (STREAM_SESSION_MESSAGE_BEGIN,
                                 session_data_container_p,
                                 &const_cast<typename SessionMessageType::USER_DATA_T&> (message_inout->data ()));

      goto continue_;

error:
      if (connection_)
      {
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s...\n"),
                    inherited::mod_->name (),
                    buffer));
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      session_data_r.aborted = true;

      break;

continue_:
      // sanity check(s)
      ACE_ASSERT (connection_);

      // set session ID
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_r.lock);
      ACE_Guard<ACE_SYNCH_MUTEX> aGuard (*session_data_r.lock);
      session_data_r.sessionID = connection_->id ();

      break;
    }
    // *NOTE*: the stream has been link()ed (see above), the message contains
    //         the (merged) upstream session data --> retain a reference
    case STREAM_SESSION_MESSAGE_LINK:
    {
      SessionDataContainerType& session_data_container_r =
        const_cast<SessionDataContainerType&> (message_inout->get ());
      session_data_container_r.increase ();
      // *IMPORTANT NOTE*: although reuse of the upstream session data is
      //                   warranted, it may not be safe, as the connection
      //                   might go away, destroying the session data lock
      //                   --> use 'this' streams' session data lock instead
      SessionDataType& session_data_2 =
        const_cast<SessionDataType&> (session_data_container_r.get ());
      session_data_2.lock = session_data_r.lock;
      inherited::sessionData_->decrease ();
      inherited::sessionData_ = &session_data_container_r;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: when the connection is (asynchronously) closed by the peer, the
      //         connection handler will finished() the connection processing
      //         stream. If it is linked to another processing stream at this
      //         point, the 'session end' message is propagated. Note that when
      //         the (linked) processing stream itself is finished() (see
      //         below), it sends a second session end message. Handle this
      //         situation here; there is nothing more to do in this case
      {
        ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

        if (sessionEndInProgress_)
        {
          passMessageDownstream_out = false;
          message_inout->release ();
          
          break; // done
        } // end IF

        sessionEndInProgress_ = true;
      } // end lock scope

      if (inherited::timerID_ != -1)
      {
        const void* act_p = NULL;
        result =
            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
                                                                      &act_p);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to cancel timer (ID: %d): \"%m\", returning\n"),
                      inherited::mod_->name (),
                      inherited::timerID_));
          goto error_2;
        } // end IF
      } // end IF

      if (connection_)
      {
        // wait for data (!) processing to complete
        ACE_ASSERT (inherited::configuration_->stream);
        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISOCKET_CONNECTION_T* isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_);
        if (!isocket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISOCKET_CONNECTION_T> (0x%@): \"%m\", returning\n"),
                      connection_));
          goto error_2;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());

        // *NOTE*: if the connection was closed abruptly, or the session was
        //         aborted, there may well be undispatched data in the
        //         connection stream. Flush it so waitForCompletion() (see
        //         below) does not block
        // *TODO*: this does not work reliably (race condition)
        Net_Connection_Status status = connection_->status ();
        if ((status != NET_CONNECTION_STATUS_OK) ||
            session_data_r.aborted)
          stream_p->flush (true);

        // *NOTE*: finalize the processing stream state so
        //         waitForCompletion() (see below) does not block
        // *NOTE*: this (probably) also wakes up the (main) thread
        // *TODO*: iff the connection has been (asynchronously) closed by the
        //         peer, there is no need to signal completion here
        inherited::configuration_->stream->finished (false); // finish upstream ?
        inherited::configuration_->stream->waitForCompletion (false, // wait for worker(s) ?
                                                              true); // wait for upstream ?
      } // end IF

error_2:
      if (connection_)
      {
        if (isLinked_)
        {
          // sanity check(s)
          ACE_ASSERT (inherited::configuration_->stream);

          result = inherited::configuration_->stream->unlink ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Stream::unlink(): \"%m\", continuing\n"),
                        inherited::mod_->name ()));
          else
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: unlinked i/o streams\n"),
                        inherited::mod_->name ()));
          //inherited::configuration_->stream->dump_state ();
        } // end IF
        isLinked_ = false;

        if (!isPassive_)
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
                        ACE_TEXT ("%s: failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n"),
                        inherited::mod_->name ()));

          connection_->close ();
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed connection to %s...\n"),
                      inherited::mod_->name (),
                      buffer));
        } // end IF

        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      inherited::sessionData_->decrease ();
      inherited::sessionData_ = NULL;

      inherited::shutdown ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
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
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_SourceH_T<LockType,
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
                           ConnectionManagerType,
                           ConnectorType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ConnectionManagerType,
//          typename ConnectorType>
//void
//Stream_Module_Net_SourceH_T<LockType,
//                           SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ConnectionManagerType,
//                           ConnectorType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

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
//Stream_Module_Net_SourceH_T<SessionMessageType,
//                          ProtocolMessageType,
//                          ConfigurationType,
//                          StreamStateType,
//                          SessionDataType,
//                          SessionDataContainerType,
//                          StatisticContainerType,
//                          ConnectionManagerType,
//                          ConnectorType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::svc"));

//  int result = -1;
//  typename ConnectionManagerType::CONNECTION_T* connection_p = NULL;
//  // *TODO*: remove type inferences
//  ConnectionManagerType* connection_manager_p =
//      inherited::configuration_->connectionManager;
//  ACE_ASSERT (connection_manager_p);
//  ACE_TCHAR buffer[BUFSIZ];
//  ACE_OS::memset (buffer, 0, sizeof (buffer));

//  // step1: process connection data
//  result = inherited::svc ();

//  // step2: close connection
//  if (isOpen_)
//  {
//    result =
//      inherited::configuration_->peerAddress.addr_to_string (buffer,
//                                                            sizeof (buffer));
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_INET_Addr::addr_to_string: \"%m\", continuing\n")));
//    connection_p =
//        connection_manager_p->get (inherited::configuration_->peerAddress);
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

//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ConnectionManagerType,
//          typename ConnectorType>
//ProtocolMessageType*
//Stream_Module_Net_SourceH_T<LockType,
//                           SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ConnectionManagerType,
//                           ConnectorType>::allocateMessage (unsigned int requestedSize_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::allocateMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
//  // initialize return value(s)
//  ProtocolMessageType* message_out = NULL;
//
//  if (inherited::configuration_->streamConfiguration->messageAllocator)
//  {
//    try
//    {
//      // *TODO*: remove type inference
//      message_out =
//          static_cast<ProtocolMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    }
//    catch (...)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
//                  requestedSize_in));
//      message_out = NULL;
//    }
//  } // end IF
//  else
//  {
//    ACE_NEW_NORETURN (message_out,
//                      ProtocolMessageType (requestedSize_in));
//  } // end ELSE
//  if (!message_out)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to Stream_IAllocator::malloc(%u), aborting\n"),
//                requestedSize_in));
//  } // end IF
//
//  return message_out;
//}
//
//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType,
//          typename ConnectionManagerType,
//          typename ConnectorType>
//bool
//Stream_Module_Net_SourceH_T<LockType,
//                           SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType,
//                           ConnectionManagerType,
//                           ConnectorType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::putStatisticMessage"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);
//  // *TODO*: remove type inferences
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
//  // step1: update session state
//  SessionDataType& session_data_r =
//        const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  // *TODO*: remove type inferences
//  session_data_r.currentStatistic = statisticData_in;
//
//  // *TODO*: attach stream state information to the session data
//
////  // step2: create session data object container
////  SessionDataContainerType* session_data_p = NULL;
////  ACE_NEW_NORETURN (session_data_p,
////                    SessionDataContainerType (inherited::sessionData_,
////                                              false));
////  if (!session_data_p)
////  {
////    ACE_DEBUG ((LM_CRITICAL,
////                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
////    return false;
////  } // end IF
//
//  // step3: send the statistic data downstream
////  // *NOTE*: fire-and-forget session_data_p here
//  // *TODO*: remove type inference
//  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
//                                       *inherited::sessionData_,
//                                       inherited::configuration_->streamConfiguration->messageAllocator);
//}
