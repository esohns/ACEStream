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

#include "common_ilock.h"
#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_net_common.h"

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename ConnectionManagerType>
Stream_Module_Net_Source_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  ConnectionManagerType>::Stream_Module_Net_Source_Reader_T ()
 : inherited (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Reader_T::Stream_Module_Net_Source_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename ConnectionManagerType>
void
Stream_Module_Net_Source_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  ConnectionManagerType>::handleControlMessage (ACE_Message_Block& messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Reader_T::handleControlMessage"));

  switch (messageBlock_in.msg_type ())
  {
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
                  messageBlock_in.msg_type ()));
      return;
    }
  } // end SWITCH
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  ConnectorType>::Stream_Module_Net_Source_Writer_T (ISTREAM_T* stream_in)
#else
                                  ConnectorType>::Stream_Module_Net_Source_Writer_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , address_ ()
 , connector_ (true)
 , connection_ (NULL)
 , isOpen_ (false)
 , isPassive_ (false)
 , unlink_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::Stream_Module_Net_Source_Writer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::~Stream_Module_Net_Source_Writer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::~Stream_Module_Net_Source_Writer_T"));

  if (connection_)
  {
    if (unlink_)
    {
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T*>(%@): \"%m\", returning\n"),
                    connection_));
        return;
      } // end IF
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p->_unlink ();
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: unlinked i/o stream(s) in dtor --> check implementation !\n"),
                  inherited::mod_->name ()));
    } // end IF

    if (!isPassive_ &&
        isOpen_)
    {
      connection_->close ();
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: closed connection to \"%s\" in dtor --> check implementation !\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
    } // end IF

    connection_->decrease ();
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::initialize (const ConfigurationType& configuration_in,
                                                              Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (unlink_)
    { ACE_ASSERT (connection_);
      typename ConnectorType::ISTREAM_CONNECTION_T* stream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!stream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T> (%@): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    connection_));
        return false;
      } // end IF
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (stream_connection_p->stream ());
      istream_p->_unlink ();

      unlink_ = false;
    } // end IF

    if (!isPassive_ &&
        isOpen_)
    { ACE_ASSERT (connection_);
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      typename ConnectorType::ADDRESS_T local_address, peer_address;
      connection_->info (handle,
                         local_address, peer_address);

      Net_ConnectionId_t id = connection_->id ();
      connection_->close ();
      isOpen_ = false;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address).c_str ()),
                  id));
    } // end IF
    isOpen_ = false;

    if (connection_)
    {
      connection_->decrease ();
      connection_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
  if (configuration_in.connection &&
      configuration_in.passive)
  {
    configuration_in.connection->increase ();
    connection_ = configuration_in.connection;
    isPassive_ = true;
  } // end IF
  else
    isPassive_ = false;

  // sanity check(s)
  ACE_ASSERT (configuration_in.connectionConfigurations);
  ACE_ASSERT (!configuration_in.connectionConfigurations->empty ());

  Net_ConnectionConfigurationsIterator_t iterator =
    inherited::configuration_->connectionConfigurations->find (inherited::mod_->name ());
  if (iterator == inherited::configuration_->connectionConfigurations->end ())
    iterator =
      inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: applying connection configuration\n"),
                inherited::mod_->name ()));
#endif // _DEBUG
  ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());
  switch (connector_.transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      Net_TCPSocketConfiguration_t* socket_configuration_p =
          dynamic_cast<Net_TCPSocketConfiguration_t*> ((*iterator).second);
      ACE_ASSERT (socket_configuration_p);
      address_ = socket_configuration_p->address;
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      Net_UDPSocketConfiguration_t* socket_configuration_p =
          dynamic_cast<Net_UDPSocketConfiguration_t*> ((*iterator).second);
      ACE_ASSERT (socket_configuration_p);
      address_ = socket_configuration_p->listenAddress;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown transport layer type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  connector_.transportLayer ()));
      break;
    }
  } // end SWITCH

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectionManagerType,
                                  ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session (id was: %u) aborted, closed connection to %s\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
      } // end IF
      isOpen_ = false;

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!isOpen_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

      // *TODO*: remove type inferences
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        ConnectionManagerType::SINGLETON_T::instance ();
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
//      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      bool clone_module, delete_module;
      bool notify_connect = false;
      Net_ConnectionConfigurationsIterator_t iterator;
      ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
      typename ConnectorType::ADDRESS_T local_SAP, peer_SAP;
      bool is_error = false;
      int result = -1;
      typename inherited::ISTREAM_T* istream_p = NULL;
      typename ConnectorType::CONFIGURATION_T* configuration_p = NULL;

      if (isPassive_)
      {
        // sanity check(s)
        if (connection_)
          goto continue_; // --> using configured connection

        // --> using session connection

        // sanity check(s)
        ACE_ASSERT (iconnection_manager_p);
        // *TODO*: remove type inferences
        ACE_ASSERT (session_data_r.lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          ACE_ASSERT (!session_data_r.connectionStates.empty ());
          ACE_ASSERT ((*session_data_r.connectionStates.begin ()).first != ACE_INVALID_HANDLE);
          connection_ =
            iconnection_manager_p->get ((*session_data_r.connectionStates.begin ()).first);
        } // end lock scope
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: 0x%@), aborting\n"),
                      inherited::mod_->name (),
                      (*session_data_r.connectionStates.begin ()).first));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: %d), aborting\n"),
                      inherited::mod_->name (),
                      (*session_data_r.connectionStates.begin ()).first));
#endif
          goto error;
        } // end IF

        goto link;
      } // end IF

      // --> open new connection

      // step2: initialize connector
      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->connectionConfigurations);
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      iterator =
        inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
      if (iterator == inherited::configuration_->connectionConfigurations->end ())
        iterator =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: applying connection configuration\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());
      configuration_p =
          dynamic_cast<typename ConnectorType::CONFIGURATION_T*> ((*iterator).second);
      ACE_ASSERT (configuration_p);

      clone_module =
        inherited::configuration_->streamConfiguration->configuration_.cloneModule;
      module_p =
        inherited::configuration_->streamConfiguration->configuration_.module;
      inherited::configuration_->streamConfiguration->configuration_.cloneModule =
          false;
      inherited::configuration_->streamConfiguration->configuration_.module =
          NULL;

      if (!iconnector_p->initialize (*configuration_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ConnectorType::INTERFACE_T::initialize(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        is_error = true;
        goto reset;
      } // end IF

      // step3: connect
      ACE_ASSERT (!connection_);
      handle_h = iconnector_p->connect (address_);
      if (handle_h == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      if (iconnector_p->useReactor ())
        connection_ = iconnection_manager_p->get (handle_h);
      else
      {
        // step1: wait for the connection to register with the manager
        // *TODO*: avoid these tight loops
        ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
        ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
        // *TODO*: this may not be accurate/applicable for/to all protocols
        bool is_peer_address =
          (iconnector_p->transportLayer () == NET_TRANSPORTLAYER_TCP);
        enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
        do
        {
          // *TODO*: avoid these tight loops
          connection_ = iconnection_manager_p->get (address_,
                                                    is_peer_address);
          if (!connection_)
            continue;
          istream_connection_p =
              dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
          if (!istream_connection_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), aborting\n"),
                        inherited::mod_->name (),
                        connection_));
            is_error = true;
            goto reset;
          } // end IF

          // step2: wait for the connection to finish initializing
          do
          {
            status = connection_->status ();
            if (status == NET_CONNECTION_STATUS_OK)
              break;
          } while (COMMON_TIME_NOW < deadline);
          if (status != NET_CONNECTION_STATUS_OK)
            break;

          // step3: wait for the connection stream to finish initializing
          istream_connection_p->wait (STREAM_STATE_RUNNING,
                                      NULL); // <-- block
          break;
        } while (COMMON_TIME_NOW < deadline);
      } // end ELSE
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      isOpen_ = true;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connected to %s (id: %u)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
      //            connection_->id ()));
      notify_connect = true;

reset:
      inherited::configuration_->streamConfiguration->configuration_.cloneModule =
          clone_module;
      inherited::configuration_->streamConfiguration->configuration_.module =
          module_p;

      if (is_error)
        goto error;

      // *NOTE*: forward the session begin message early; if at all possible, it
      //         should always be the first session message seen by downstream
      result = inherited::put_next (message_inout, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // clean up
      message_inout = NULL;
      passMessageDownstream_out = false;

link:
      // sanity check(s)
      ACE_ASSERT (connection_);

      istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF
      stream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p = const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
      ACE_ASSERT (istream_p);
      if (!istream_p->link (stream_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IStream_T::link(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      unlink_ = true;

      //// update session data in the current session message
      //// *WARNING*: this works iff (!) the STREAM_SESSION_LINK message has
      ////            already been processed at this point (i.e. as long as
      ////            upstream is completely synchronous)
      //session_data_r =
      //  const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      //inherited::sessionData_->increase ();
      //session_data_container_p = inherited::sessionData_;
      //message_inout->initialize (session_data_r.sessionId,
      //                           STREAM_SESSION_MESSAGE_BEGIN,
      //                           session_data_container_p,
      //                           &const_cast<typename SessionMessageType::USER_DATA_T&> (message_inout->data ()));

      goto continue_;

error:
      if (unlink_)
      {
        typename ConnectorType::ISTREAM_CONNECTION_T* stream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!stream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      connection_));
          goto error_2;
        } // end IF
        typename inherited::ISTREAM_T* istream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (stream_connection_p->stream ());
        istream_p->_unlink ();

        unlink_ = false;
      } // end IF

error_2:
      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                    id));
      } // end IF
      isOpen_ = false;

      if (connection_)
      {
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      //// sanity check(s)
      //ACE_ASSERT (connection_);

      //// update session data
      //connection_->info (handle_h,
      //                   local_SAP, peer_SAP);
      //// *TODO*: remove type inferences
      //ACE_ASSERT (session_data_r.lock);
      //{ ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
      //  session_data_r.connectionStates.insert (std::make_pair (handle_h,
      //                                                          &const_cast<typename ConnectionManagerType::STATE_T&> (connection_->state ())));
      //} // end lock scope

      if (notify_connect)
        inherited::notify (STREAM_SESSION_MESSAGE_CONNECT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      typename inherited::ISTREAM_T* istream_p = NULL;

      // *NOTE*: control reaches this point because either:
      //         - the connection has been closed and the processing stream is
      //           finished()-ing (see: ACE_Svc_Handler::handle_close()
      //           derivates)
      //           --> wait for upstream data processing to complete before
      //               unlinking
      //         - the session is being aborted by the user/some module
      //           --> do NOT wait for upstream data processing to complete
      //               before unlinking, i.e. flush the connection stream
      // *TODO*: never flush any server-side inbound data

      if (connection_)
      {
        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", returning\n"),
                      inherited::mod_->name (),
                      connection_));
          goto flush;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());

        // wait for upstream data processing to complete before unlinking ?
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          if (session_data_r.aborted)
            goto flush;
        } // end lock scope

        // *NOTE*: finalize the (connection) stream state so waitForCompletion()
        //         does not block forever
        // *NOTE*: stop()ping the connection stream will also unlink it
        //         --> send the disconnect notification early
        inherited::notify (STREAM_SESSION_MESSAGE_DISCONNECT);
        // *TODO*: this shouldn't be necessary (--> only wait for data to flush)
        stream_p->stop (false, // wait for completion ?
                        false, // recurse upstream ?
                        true); // locked access ?
        connection_->waitForCompletion (false); // --> data only

        goto continue_2;

flush:
        stream_p->flush (true,   // flush inbound data ?
                         false,  // flush session messages ?
                         false); // flush upstream (if any) ?
      } // end IF

continue_2:
      if (unlink_)
      {
        istream_p =
            const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
        ACE_ASSERT (istream_p);
        istream_p->_unlink ();

        unlink_ = false;
      } // end IF

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                    id));
      } // end IF
      isOpen_ = false;

      if (connection_)
      {
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      if (isOpen_)
      {
        // *TODO*: the stream can control several connections, so this may be
        //         wrong...
        //isOpen_ = false;

        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("%s: disconnected\n"),
        //            inherited::mod_->name ()));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    {
      typename inherited::ISTREAM_T* istream_p =
        const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: linked i/o stream(s)\n"),
      //            inherited::mod_->name ()));

      // sanity check(s)
      ACE_ASSERT (istream_p);

#if defined (_DEBUG)
      istream_p->dump_state ();
#endif

      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      typename inherited::ISTREAM_T* istream_p =
        const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
      //            inherited::mod_->name ()));

      // sanity check(s)
      if (!inherited::linked_)
      { // *TODO*: clean this up
        break;
      } // end IF
      ACE_ASSERT (istream_p);

      // *NOTE*: most probable reason: the stream has been stop()ped
      // *TODO*: the stream can have several substreams that are (un-)linked
      //         dynamically, so this may be wrong...
      unlink_ = false;

#if defined (_DEBUG)
      istream_p->dump_state ();
#endif

      break;
    }
    default:
      break;
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
          typename TimerManagerType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename UserDataType>
Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            ConnectionManagerType,
                            ConnectorType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            UserDataType>::Stream_Module_Net_SourceH_T (ISTREAM_T* stream_in,
#else
                            UserDataType>::Stream_Module_Net_SourceH_T (typename inherited::ISTREAM_T* stream_in,
#endif
                                                                        bool generateSessionMessages_in,
                                                                        bool isPassive_in)
 : inherited (stream_in,
              false,
              STREAM_HEADMODULECONCURRENCY_CONCURRENT,
              generateSessionMessages_in)
 , address_ ()
 , connector_ (true)
 , connection_ (NULL)
 , isOpen_ (false)
 , isPassive_ (isPassive_in)
 , unlink_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::Stream_Module_Net_SourceH_T"));

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
          typename TimerManagerType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename UserDataType>
Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::~Stream_Module_Net_SourceH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::~Stream_Module_Net_SourceH_T"));

  if (connection_)
  {
    if (unlink_)
    {
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T> (%@): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    connection_));
        return;
      } // end IF
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p->_unlink ();
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: unlinked i/o stream(s) in dtor --> check implementation !\n"),
                  inherited::mod_->name ()));
    } // end IF

    if (isOpen_ &&
        !isPassive_)
    {
      connection_->close ();
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: closed connection to \"%s\" in dtor --> check implementation !\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
    } // end IF

    connection_->decrease ();
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
          typename TimerManagerType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename UserDataType>
bool
Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (unlink_)
    { ACE_ASSERT (connection_);
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    connection_));
        goto close;
      } // end IF
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p->_unlink ();

      unlink_ = false;
    } // end IF

close:
    if (isOpen_ &&
        !isPassive_)
    { ACE_ASSERT (connection_);
      Net_ConnectionId_t id = connection_->id ();
      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                  id));
    } // end IF
    isOpen_ = false;

    if (connection_)
    {
      connection_->decrease ();
      connection_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
//  if (configuration_in.connection &&
//      configuration_in.passive)
//  {
//    connection_ = configuration_in.connection;
//    connection_->increase ();

//    isPassive_ = true;
//  } // end IF
//  else
    isPassive_ = false;

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.connectionConfigurations);
  Net_ConnectionConfigurationsIterator_t iterator =
    configuration_in.connectionConfigurations->find (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
  if (iterator == configuration_in.connectionConfigurations->end ())
    iterator =
      configuration_in.connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: applying connection configuration\n"),
                inherited::mod_->name ()));
#endif // _DEBUG
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations->end ());
  switch (connector_.transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      Net_TCPSocketConfiguration_t* socket_configuration_p =
          dynamic_cast<Net_TCPSocketConfiguration_t*> ((*iterator).second);
      ACE_ASSERT (socket_configuration_p);
      address_ = socket_configuration_p->address;
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      Net_UDPSocketConfiguration_t* socket_configuration_p =
          dynamic_cast<Net_UDPSocketConfiguration_t*> ((*iterator).second);
      ACE_ASSERT (socket_configuration_p);
      address_ = socket_configuration_p->listenAddress;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown transport layer type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  connector_.transportLayer ()));
      break;
    }
  } // end SWITCH

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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename UserDataType>
void
Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session (id was: %u) aborted, closed connection (id was: %u)\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId, id));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!isOpen_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        ConnectionManagerType::SINGLETON_T::instance ();
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename inherited::TASK_BASE_T::ISTREAM_T* istream_p = NULL;
//      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      bool notify_connect = false;
//      bool clone_module, delete_module;
      Net_ConnectionConfigurationsIterator_t iterator_2;
      // *NOTE*: (currently,) this could be a TCP (--> test peer address),
      //         or a UDP (--> test local address) connection
      bool is_peer_address =
          (iconnector_p->transportLayer () == NET_TRANSPORTLAYER_TCP);
      bool is_error = false;
      ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
      typename ConnectorType::ADDRESS_T local_SAP, peer_SAP;
      typename ConnectorType::CONFIGURATION_T* configuration_p = NULL;

      // schedule regular statistic collection ?
      if (inherited::configuration_->statisticReportingInterval !=
          ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                   : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL,
                                 0);
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

      if (isPassive_)
      {
        // sanity check(s)
        if (connection_)
          goto link; // --> using configured connection

        // --> using session connection

        // sanity check(s)
        ACE_ASSERT (iconnection_manager_p);
        // *TODO*: remove type inferences
        ACE_ASSERT (session_data_r.lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          ACE_ASSERT (!session_data_r.connectionStates.empty ());
          connection_ =
            iconnection_manager_p->get ((*session_data_r.connectionStates.begin ()).first);
        } // end lock scope
        if (!connection_)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: 0x%@), aborting\n"),
                      inherited::mod_->name (),
                      (*session_data_r.connectionStates.begin ()).first));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (handle was: %d), aborting\n"),
                      inherited::mod_->name (),
                      (*session_data_r.connectionStates.begin ()).first));
#endif
          goto error;
        } // end IF

        goto link;
      } // end IF

      // --> open new connection

      // step2: initialize connector
      // sanity check(s)
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->connectionConfigurations);

      iterator_2 =
        inherited::configuration_->connectionConfigurations->find (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
      if (iterator_2 == inherited::configuration_->connectionConfigurations->end ())
        iterator_2 =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: applying connection configuration\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
      ACE_ASSERT (iterator_2 != inherited::configuration_->connectionConfigurations->end ());
      configuration_p =
          dynamic_cast<typename ConnectorType::CONFIGURATION_T*> ((*iterator_2).second);
      ACE_ASSERT (configuration_p);

//      // *NOTE*: the stream configuration may contain a module handle that is
//      //         meant to be the final module of this processing stream. As
//      //         the connection stream will be appended to this pipeline, the
//      //         connection should not enqueue that same module again
//      //         --> temporarily 'hide' the module handle, if any
//      // *TODO*: remove this ASAP
//      clone_module =
//          inherited::configuration_->streamConfiguration->configuration_.cloneModule;
//      delete_module =
//          inherited::configuration_->streamConfiguration->configuration_.deleteModule;
//      module_p =
//          inherited::configuration_->streamConfiguration->configuration_.module;
//      inherited::configuration_->streamConfiguration->configuration_.cloneModule =
//          false;
//      inherited::configuration_->streamConfiguration->configuration_.deleteModule =
//          false;
//      inherited::configuration_->streamConfiguration->configuration_.module =
//          NULL;

      if (!iconnector_p->initialize (*configuration_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize connector: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        is_error = true;
        goto reset;
      } // end IF

      // step3: connect
      ACE_ASSERT (!connection_);

      try {
        handle_h = iconnector_p->connect (address_);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Net_IConnector::connect(%s), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
        is_error = true;
        goto reset;
      }
      if (handle_h == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      if (iconnector_p->useReactor ())
        connection_ = iconnection_manager_p->get (handle_h);
      else
      {
        enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
        ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
        ACE_Time_Value deadline;

        // step0: wait for the connection attempt to complete
        typename ConnectorType::IASYNCH_CONNECTOR_T* iasynch_connector_p =
          dynamic_cast<typename ConnectorType::IASYNCH_CONNECTOR_T*> (&connector_);
        ACE_ASSERT (iasynch_connector_p);
        result = iasynch_connector_p->wait (handle_h,
                                            timeout);
        if (result)
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Net_IConnector::connect(%s): \"%s\" aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                      ACE::sock_error (result)));
#else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Net_IConnector::connect(%s): \"%s\" aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                      ACE_TEXT (ACE_OS::strerror (result))));
#endif
          is_error = true;
          goto reset;
        } // end IF
        // *TODO*: deduct the interval from the deadline

        // step1: wait for the connection to register with the manager
        // *TODO*: this may not be accurate/applicable for/to all protocols
        deadline = COMMON_TIME_NOW + timeout;
        do
        {
          // *TODO*: avoid this tight loop
          connection_ = iconnection_manager_p->get (address_,
                                                    is_peer_address);
          if (connection_)
            break;
        } while (COMMON_TIME_NOW < deadline);
        if (!connection_)
        { ACE_ASSERT (COMMON_TIME_NOW >= deadline);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to connect to %s (timeout: %#T), aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                      &timeout));
          is_error = true;
          goto reset;
        } // end IF

        // step2: wait for the connection to finish initializing
        do
        {
          status = connection_->status ();
          if (status == NET_CONNECTION_STATUS_OK)
            break;
        } while (COMMON_TIME_NOW < deadline);
        if (status != NET_CONNECTION_STATUS_OK)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
          is_error = true;
          goto reset;
        } // end IF

        // step3: wait for the connection stream to finish initializing
        istream_connection_p =
            dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), aborting\n"),
                      inherited::mod_->name (),
                      connection_));
          is_error = true;
          goto reset;
        } // end IF
        istream_connection_p->wait (STREAM_STATE_RUNNING,
                                    NULL); // <-- block
      } // end ELSE
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      isOpen_ = true;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connected to %s (id: %u)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
      //            connection_->id ()));
      notify_connect = true;

reset:
//      inherited::configuration_->streamConfiguration->configuration_.cloneModule =
//          clone_module;
//      inherited::configuration_->streamConfiguration->configuration_.deleteModule =
//          delete_module;
//      inherited::configuration_->streamConfiguration->configuration_.module =
//          module_p;

      if (is_error)
        goto error;

      // *NOTE*: forward the session begin message early; if at all possible, it
      //         should always be the first session message seen by downstream
      // *IMPORTANT NOTE*: how this policy has serious repercussions
      result = inherited::put_next (message_inout, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // clean up
      message_inout = NULL;
      passMessageDownstream_out = false;

link:
      // sanity check(s)
      ACE_ASSERT (connection_);

      istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF

      // link processing streams
      stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p =
          const_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (inherited::getP ());
      ACE_ASSERT (istream_p);
      if (unlikely (!istream_p->link (stream_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Stream_IStream_T::link(0x%@): \"%m\", aborting\n"),
                    ACE_TEXT (istream_p->name ().c_str ()),
                    inherited::mod_->name (),
                    stream_p));
        goto error;
      } // end IF
      unlink_ = true;

      //// update session data in the current session message
      //// *WARNING*: this works iff (!) the STREAM_SESSION_LINK message has been
      ////            received by now (i.e. iff upstream is entirely synchronous)
      //session_data_r =
      //  const_cast<SessionDataType&> (inherited::sessionData_->get ());
      //inherited::sessionData_->increase ();
      //session_data_container_p = inherited::sessionData_;
      //message_inout->initialize (session_data_r.sessionId,
      //                           STREAM_SESSION_MESSAGE_BEGIN,
      //                           session_data_container_p,
      //                           &const_cast<typename SessionMessageType::USER_DATA_T&> (message_inout->data ()));

      goto continue_;

error:
      if (unlink_)
      { ACE_ASSERT (connection_);
        istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(%@): \"%m\", returning\n"),
                      inherited::mod_->name (),
                      connection_));
          return;
        } // end IF
        typename inherited::ISTREAM_T* istream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        istream_p->_unlink ();
      
        unlink_ = false;
      } // end IF

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
                    id));
      } // end IF
      isOpen_ = false;

      if (connection_)
      {
        connection_->decrease (); connection_ = NULL;
      } // end IF

      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      //// sanity check(s)
      //ACE_ASSERT (connection_);

      //// update session data
      //connection_->info (handle_h,
      //                   local_SAP, peer_SAP);
      //// *TODO*: remove type inferences
      //ACE_ASSERT (session_data_r.lock);
      //{ ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
      //  session_data_r.connectionStates.insert (std::make_pair (handle_h,
      //                                                          &const_cast<typename ConnectionManagerType::STATE_T&> (connection_->state ())));
      //} // end lock scope

      if (notify_connect)
        inherited::notify (STREAM_SESSION_MESSAGE_CONNECT);

      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      if (isOpen_)
      { ACE_ASSERT (connection_);
        // *TODO*: the stream can control several connections, so this may be
        //         wrong...
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: disconnected (id was: %u)\n"),
                    inherited::mod_->name (),
                    connection_->id ()));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: control reaches this point because either:
      //         - the connection has been closed and the processing stream is
      //           finished()-ing (see: ACE_Svc_Handler::handle_close()
      //           derivates)
      //           --> wait for upstream data processing to complete before
      //               unlinking
      //         - the session is being aborted by the user/some module
      //           --> do NOT wait for upstream data processing to complete
      //               before unlinking, i.e. flush the connection stream

      // *NOTE*: when the connection is (asynchronously) closed by the peer, the
      //         connection handler will finished() the connection processing
      //         stream. If it is linked to another processing stream (e.g.
      //         'this') at this stage, the 'session end' message is propagated.
      //         Note that when the (linked) processing stream (e.g. 'this')
      //         itself is finished() (see below), it sends its own session end
      //         message. Handle both scenarios (and race conditions) here, i.e.
      //         never process consecutive 'session end' messages
      { ACE_GUARD (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

      if (inherited::isRunning ())
      {
        { ACE_GUARD (typename inherited::ITASKCONTROL_T::MUTEX_T, aGuard, inherited::lock_);
          inherited::sessionEndSent_ = true;
        } // end lock scope
      } // end IF

      if (inherited::timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
          (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                   : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
          goto continue_2;
        } // end IF
      } // end IF

      if (connection_)
      {
        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      connection_));
          goto continue_2;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());

        // wait for upstream data processing to complete before unlinking ?
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          if (session_data_r.aborted)
            goto flush;
        } // end lock scope

        // *NOTE*: finalize the (connection) stream state so waitForCompletion()
        //         does not block forever
        // *NOTE*: stop()ping the connection stream will also unlink it
        //         --> send the disconnect notification early
        inherited::notify (STREAM_SESSION_MESSAGE_DISCONNECT);
        // *TODO*: this shouldn't be necessary (--> only wait for data to flush)
        stream_p->stop (false, // wait for completion ?
                        false, // recurse upstream ?
                        true); // locked access ?
        connection_->waitForCompletion (false); // --> data only

        goto continue_2;

flush:
        stream_p->flush (true,   // flush inbound data ?
                         false,  // flush session messages ?
                         false); // flush upstream (if any) ?
      } // end IF

continue_2:
      if (unlink_)
      {
        typename inherited::TASK_BASE_T::ISTREAM_T* istream_p =
            const_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (inherited::getP ());
        ACE_ASSERT (istream_p);
        istream_p->_unlink ();
        unlink_ = false;
      } // end IF

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection (id was: %u)\n"),
                    inherited::mod_->name (),
                    id));
      } // end IF

      if (connection_)
      {
        connection_->decrease (); connection_ = NULL;
      } // end IF

      if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
        inherited::TASK_BASE_T::stop (false,  // wait for completion ?
                                      false); // N/A

      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    {
#if defined (_DEBUG)
      const typename inherited::TASK_BASE_T::ISTREAM_T* const istream_p =
          inherited::getP ();
      ACE_ASSERT (istream_p);
      istream_p->dump_state ();
#endif
      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      // sanity check(s)
      if (!inherited::linked_)
        break;

      // *NOTE*: most probable reasons:
      //         - upstream has been stop()ped because the session has ended
      //           due to user interaction (see below). On STOP the state
      //           machine currently invokes finished(), which unlink()s any
      //           upstream automatically
      //         - the stream has been stop()ped because the connection has
      //           close()d, finished()-ing the upstream connection stream,
      //           which in turn unlink()s any downstream during shutdown
      //         in the latter case, do not unlink
      // *TODO*: the stream could have several substreams that are (un-)linked
      //         dynamically and independently; to avoid race conditions a
      //         context reference test is required here
      unlink_ = false;

//#if defined (_DEBUG)
//      ACE_ASSERT (inherited::stream_);
//      inherited::stream_->dump_state ();
//#endif
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
          typename TimerManagerType,
          typename ConnectionManagerType,
          typename ConnectorType,
          typename UserDataType>
bool
Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
                            TimerManagerType,
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::collect"));

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
                ACE_TEXT ("%s: failed to putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}
