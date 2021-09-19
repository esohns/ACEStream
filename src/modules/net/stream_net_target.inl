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

#include "stream_istreamcontrol.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_configuration.h"
#include "net_iconnector.h"

#include "net_client_common_tools.h"
#include "net_client_defines.h"

#include "stream_net_common.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           ConnectorType>::Stream_Module_Net_Target_T (ISTREAM_T* stream_in,
#else
                           ConnectorType>::Stream_Module_Net_Target_T (typename inherited::ISTREAM_T* stream_in,
#endif
                                                                       bool isPassive_in)
 : inherited (stream_in)
 , connection_ (NULL)
 , connector_ (true)
 , sessionEndProcessed_ (false)
 /////////////////////////////////////////
 , address_ ()
 , isOpen_ (false)
 , isPassive_ (isPassive_in)
 //, lock_ ()
 , unlink_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::Stream_Module_Net_Target_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::~Stream_Module_Net_Target_T"));

  if (!connector_.useReactor ())
  {
    typename ConnectorType::IASYNCH_CONNECTOR_T* iasynch_connector_p =
      dynamic_cast<typename ConnectorType::IASYNCH_CONNECTOR_T*> (&connector_);
    ACE_ASSERT (iasynch_connector_p);
    iasynch_connector_p->abort ();
  } // end IF

  if (unlikely (unlink_))
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
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: unlinked i/o stream(s) in dtor --> check implementation !\n"),
                inherited::mod_->name ()));
  } // end IF

close:
  if (unlikely (isOpen_ &&
                !isPassive_))
  { ACE_ASSERT (connection_);
    connection_->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: closed connection to %s in dtor --> check implementation !\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_, false, false).c_str ())));
  } // end IF

  if (unlikely (connection_))
    connection_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Target_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    if (unlink_)
    { ACE_ASSERT (connection_);
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      ACE_ASSERT (istream_connection_p);
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
    sessionEndProcessed_ = false;
  } // end IF

  // *TODO*: remove type inferences
  if (unlikely (configuration_in.connection &&
                configuration_in.passive))
  {
//    connection_ = configuration_in.connection;
    connection_->increase ();

    isPassive_ = true;
  } // end IF
  else
    isPassive_ = false;

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.connectionConfigurations);
  Net_ConnectionConfigurationsIterator_t iterator =
    configuration_in.connectionConfigurations->find (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
  if (likely (iterator == configuration_in.connectionConfigurations->end ()))
    iterator =
      configuration_in.connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: applying connection configuration\n"),
                inherited::mod_->name ()));
#endif // _DEBUG
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations->end ());
  typename ConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T* configuration_p =
    static_cast<typename ConnectorType::ISTREAM_CONNECTION_T::CONFIGURATION_T*> ((*iterator).second);
  ACE_ASSERT (configuration_p);
  switch (connector_.transportLayer ())
  {
    case NET_TRANSPORTLAYER_TCP:
    {
      Net_TCPSocketConfiguration_t* socket_configuration_p =
        reinterpret_cast<Net_TCPSocketConfiguration_t*> (&configuration_p->socketConfiguration);
      address_ = socket_configuration_p->address;
      break;
    }
    case NET_TRANSPORTLAYER_UDP:
    {
      Net_UDPSocketConfiguration_t* socket_configuration_p =
        reinterpret_cast<Net_UDPSocketConfiguration_t*> (&configuration_p->socketConfiguration);
      address_ = socket_configuration_p->peerAddress;
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
          typename SessionDataContainerType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_Net_Target_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           ConnectionManagerType,
                           ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::handleSessionMessage"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionDataContainerType::DATA_T& session_data_r =
          const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());

      if (unlikely (isOpen_ &&
                    !isPassive_))
      { ACE_ASSERT (connection_);
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session %u aborted, closed connection to %s\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId,
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_, false, false).c_str ())));
      } // end IF
      isOpen_ = false;

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!isOpen_);

      typename SessionDataContainerType::DATA_T& session_data_r =
          const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        ConnectionManagerType::SINGLETON_T::instance ();
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename ConnectorType::CONFIGURATION_T* configuration_p;
      typename ConnectorType::USERDATA_T user_data_s;
      bool notify_connect = false;
      Net_ConnectionConfigurationsIterator_t iterator_2;
      typename inherited::TASK_BASE_T::STREAM_T* stream_2 = NULL;
      ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
      typename ConnectorType::ADDRESS_T local_SAP, peer_SAP;

      if (unlikely (isPassive_))
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
#endif // ACE_WIN32 || ACE_WIN64
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
      if (likely (iterator_2 == inherited::configuration_->connectionConfigurations->end ()))
        iterator_2 =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: applying connection configuration\n"),
                    inherited::mod_->name ()));
      ACE_ASSERT (iterator_2 != inherited::configuration_->connectionConfigurations->end ());
      configuration_p =
          static_cast<typename ConnectorType::CONFIGURATION_T*> ((*iterator_2).second);
      ACE_ASSERT (configuration_p);

      switch (connector_.transportLayer ())
      {
        case NET_TRANSPORTLAYER_TCP:
        {
          Net_TCPSocketConfiguration_t* socket_configuration_p =
              (Net_TCPSocketConfiguration_t*)&configuration_p->socketConfiguration;
          ACE_ASSERT (socket_configuration_p);
          peer_SAP = socket_configuration_p->address;
          break;
        }
        case NET_TRANSPORTLAYER_UDP:
        {
          Net_UDPSocketConfiguration_t* socket_configuration_p =
            (Net_UDPSocketConfiguration_t*)&configuration_p->socketConfiguration;
          ACE_ASSERT (socket_configuration_p);
          peer_SAP = socket_configuration_p->peerAddress;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown transport layer type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      connector_.transportLayer ()));
          goto error;
        }
      } // end SWITCH

      // step2: connect
      handle_h =
        Net_Client_Common_Tools::connect<ConnectorType> (connector_,
                                                         *configuration_p,
                                                         user_data_s,
                                                         peer_SAP,
                                                         true, // wait ?
                                                         true);
      if (handle_h == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ())));
        goto error;
      } // end IF
      connection_ = iconnection_manager_p->get (handle_h);
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ())));
        goto error;
      } // end IF
      isOpen_ = true;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connected to %s (id: %u)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ()),
      //            connection_->id ()));
      notify_connect = true;

      // *NOTE*: forward the session begin message early; if at all possible, it
      //         should always be the first session message seen by downstream
      result = inherited::put_next (message_inout, NULL);
      if (unlikely (result == -1))
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
      if (unlikely (!istream_connection_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF
      stream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      stream_2 =
          dynamic_cast<typename inherited::TASK_BASE_T::STREAM_T*> (const_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (inherited::getP ()));
      ACE_ASSERT (stream_2);
      result = stream_p->link (stream_2);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: %s failed to ACE_Stream::link(%s), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (stream_p->name ().c_str ()),
                    ACE_TEXT (inherited::getP ()->name ().c_str ())));
        goto error;
      } // end IF
      unlink_ = true;

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
        typename ConnectorType::STREAM_T& stream_r =
          const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        stream_r._unlink ();

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
      // sanity check(s)
      ACE_ASSERT (connection_);

      // update session data
      connection_->info (handle_h,
                         local_SAP, peer_SAP);
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.connectionStates.insert (std::make_pair (handle_h,
                                                                &const_cast<typename ConnectionManagerType::STATE_T&> (connection_->state ())));
      } // end lock scope

      if (likely (notify_connect))
        inherited::notify (STREAM_SESSION_MESSAGE_CONNECT);

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
    case STREAM_SESSION_MESSAGE_END:
    {
      if (unlikely (sessionEndProcessed_))
        break; // done
      sessionEndProcessed_ = true;

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionDataContainerType::DATA_T& session_data_r =
          const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());

      // *NOTE*: control reaches this point because either:
      //         - the connection has been closed and the processing stream is
      //           finished()-ing (see: ACE_Svc_Handler::handle_close()
      //           derivates)
      //           --> wait for upstream data processing to complete before
      //               unlinking
      //         - the session is being aborted by the user/some module
      //           --> do NOT wait for upstream data processing to complete
      //               before unlinking, i.e. flush the connection stream

      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;

      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

      if (likely (connection_))
      {
        // wait for data (!) processing to complete
        istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (unlikely (!istream_connection_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T*>(0x%@): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      connection_));
          goto continue_2;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());

        // wait for upstream data processing to complete before unlinking ?
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          if (unlikely (session_data_r.aborted))
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
        stream_p->flush (true,   // flush outbound data ?
                         false,  // flush session messages ?
                         false); // flush upstream (if any) ?
      } // end IF

continue_2:
      if (likely (unlink_))
      {
        // *TODO*: if active (!) finished() (see above) already forwarded a
        //         session end message (on the linked stream)
        // *TODO*: this prevents the GTK close_all button from working
        //         properly in the integration test, as the connection has not
        //         yet been released when session end is notified to the
        //         application
        result = inherited::put_next (message_inout, NULL);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));

        // clean up
        message_inout = NULL;
        passMessageDownstream_out = false;

        //ACE_ASSERT (connection_);
        //ACE_ASSERT (istream_connection_p);
        ACE_ASSERT (stream_p);
        stream_p->_unlink ();
        unlink_ = false;
      } // end IF

      if (unlikely (isOpen_ &&
                    !isPassive_))
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

      if (likely (connection_))
      {
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    {
//#if defined (_DEBUG)
//      ACE_ASSERT (inherited::stream_);
//      inherited::stream_->dump_state ();
//#endif
      break;
    }
    case STREAM_SESSION_MESSAGE_UNLINK:
    {
      // sanity check(s)
      if (unlikely (!inherited::linked_))
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
