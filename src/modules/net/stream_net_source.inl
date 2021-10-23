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
#include "net_common_tools.h"
#include "net_configuration.h"

#include "net_client_common_tools.h"
#include "net_client_defines.h"

#include "stream_net_common.h"

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_Module_Net_Source_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  SessionMessageType>::Stream_Module_Net_Source_Reader_T (ISTREAM_T* stream_in)
#else
                                  SessionMessageType>::Stream_Module_Net_Source_Reader_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Reader_T::Stream_Module_Net_Source_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_Module_Net_Source_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Reader_T::handleControlMessage"));

  switch (message_in.msg_type ())
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
                  message_in.msg_type ()));
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
          typename ConnectorType>
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  ConnectorType>::Stream_Module_Net_Source_Writer_T (ISTREAM_T* stream_in)
#else
                                  ConnectorType>::Stream_Module_Net_Source_Writer_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , connector_ (true)
 , connection_ (NULL)
 , handles_ ()
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
          typename ConnectorType>
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConnectorType>::~Stream_Module_Net_Source_Writer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::~Stream_Module_Net_Source_Writer_T"));

  if (connection_)
  {
    if (unlink_)
    {
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      ACE_ASSERT (istream_connection_p);
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
                  ACE_TEXT ("%s: closed connection in dtor --> check implementation !\n"),
                  inherited::mod_->name ()));
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
          typename ConnectorType>
bool
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
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
      ACE_ASSERT (stream_connection_p);
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (stream_connection_p->stream ());
      istream_p->_unlink ();
      unlink_ = false;
    } // end IF

    handles_.clear ();

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
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_address, false, false).c_str ()),
                  id));
    } // end IF
    isOpen_ = false;

    if (connection_)
    {
      connection_->decrease (); connection_ = NULL;
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

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConnectorType>
void
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
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
                    ACE_TEXT ("%s: session (id was: %u) aborted, closed connection\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId));
      } // end IF
      isOpen_ = false;

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
    case STREAM_SESSION_MESSAGE_LINK:
    {
#if defined (_DEBUG)
      const typename inherited::TASK_BASE_T::ISTREAM_T* const istream_p =
        inherited::getP ();
      ACE_ASSERT (istream_p);
      istream_p->dump_state ();
#endif // _DEBUG
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
      typename ConnectorType::CONNECTION_MANAGER_T::INTERFACE_T* iconnection_manager_p =
        ConnectorType::CONNECTION_MANAGER_T::SINGLETON_T::instance ();
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;
      int result = -1;
      bool notify_connect = false;
      Net_ConnectionConfigurationsIterator_t iterator;
      ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
      typename ConnectorType::ADDRESS_T local_SAP, peer_SAP;
      bool is_error = false;
      typename inherited::ISTREAM_T* istream_p = NULL;
      typename ConnectorType::CONFIGURATION_T* configuration_p = NULL;
      typename ConnectorType::USERDATA_T user_data_s;
      bool is_peer_address_b = true;

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

      // step1: initialize
      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->connectionConfigurations);
      ACE_ASSERT (!connection_);

      iterator =
        inherited::configuration_->connectionConfigurations->find (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
      if (iterator == inherited::configuration_->connectionConfigurations->end ())
        iterator =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: applying dedicated connection configuration\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());
      configuration_p =
          static_cast<typename ConnectorType::CONFIGURATION_T*> ((*iterator).second);
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
          peer_SAP = socket_configuration_p->listenAddress;
          is_peer_address_b = false;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown transport layer type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      connector_.transportLayer ()));
          is_error = true;
          goto reset;
        }
      } // end SWITCH

      // step2: connect
      handle_h =
        Net_Client_Common_Tools::connect<ConnectorType> (connector_,
                                                         *configuration_p,
                                                         user_data_s,
                                                         peer_SAP,
                                                         true, // wait ?
                                                         is_peer_address_b);
      if (handle_h == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      connection_ = iconnection_manager_p->get (handle_h);
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      isOpen_ = true;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connected to %s (id: %u)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ()),
      //            connection_->id ()));
      notify_connect = true;

      // update session data
      connection_->info (handle_h,
                         local_SAP, peer_SAP);
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.connectionStates.insert (std::make_pair (handle_h,
                                                                &const_cast<typename ConnectorType::CONNECTION_MANAGER_T::STATE_T&> (connection_->state ())));
      } // end lock scope
      handles_.push_back (handle_h);

reset:
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
      ACE_ASSERT (istream_connection_p);
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
#if defined (_DEBUG)
      stream_p->dump_state ();
#endif // _DEBUG

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
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        ACE_ASSERT (istream_connection_p);
        typename inherited::ISTREAM_T* istream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        istream_p->_unlink ();
        unlink_ = false;
      } // end IF

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        Stream_ConnectionStatesIterator_t iterator = session_data_r.connectionStates.find (handle_h);
        ACE_ASSERT (iterator != session_data_r.connectionStates.end ());
        session_data_r.connectionStates.erase (iterator);
      } // end lock scope
      handles_.pop_back ();

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ()),
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
      // *WARNING*: never flush any server-side inbound data

      if (connection_)
      {
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        ACE_ASSERT (istream_connection_p);
        typename ConnectorType::STREAM_T* stream_p =
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
        istream_p->dump_state ();
        unlink_ = false;
      } // end IF

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        Stream_ConnectionStatesIterator_t iterator = session_data_r.connectionStates.find (handles_.back ());
        ACE_ASSERT (iterator != session_data_r.connectionStates.end ());
        session_data_r.connectionStates.erase (iterator);
      } // end lock scope
      handles_.pop_back ();

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection (id was: %u)\n"),
                    inherited::mod_->name (),
                    id));
      } // end IF
      isOpen_ = false;

      if (connection_)
      {
        connection_->decrease (); connection_ = NULL;
      } // end IF

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
          typename StatisticContainerType,
          typename TimerManagerType,
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
                            StatisticContainerType,
                            TimerManagerType,
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
 , connector_ (true)
 , connection_ (NULL)
 , handles_ ()
 , isOpen_ (false)
 , isPassive_ (isPassive_in)
 , unlink_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::Stream_Module_Net_SourceH_T"));

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
                            StatisticContainerType,
                            TimerManagerType,
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
      ACE_ASSERT (istream_connection_p);
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
                  ACE_TEXT ("%s: closed connection in dtor --> check implementation !\n"),
                  inherited::mod_->name ()));
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
          typename StatisticContainerType,
          typename TimerManagerType,
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
                            StatisticContainerType,
                            TimerManagerType,
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
      ACE_ASSERT (istream_connection_p);
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p->_unlink ();
      unlink_ = false;
    } // end IF

    handles_.clear ();

    if (isOpen_ &&
        !isPassive_)
    { ACE_ASSERT (connection_);
      Net_ConnectionId_t id = connection_->id ();
      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed connection (id was: %u)\n"),
                  inherited::mod_->name (),
                  id));
    } // end IF
    isOpen_ = false;

    if (connection_)
    {
      connection_->decrease (); connection_ = NULL;
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
                            StatisticContainerType,
                            TimerManagerType,
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
      isOpen_ = false;

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
    case STREAM_SESSION_MESSAGE_LINK:
    {
#if defined (_DEBUG)
      const typename inherited::TASK_BASE_T::ISTREAM_T* const istream_p =
          inherited::getP ();
      ACE_ASSERT (istream_p);
      istream_p->dump_state ();
#endif // _DEBUG
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
      typename ConnectorType::CONNECTION_MANAGER_T::INTERFACE_T* iconnection_manager_p =
        ConnectorType::CONNECTION_MANAGER_T::SINGLETON_T::instance ();
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;
      int result = -1;
      bool notify_connect = false;
      Net_ConnectionConfigurationsIterator_t iterator;
      ACE_HANDLE handle_h = ACE_INVALID_HANDLE;
      typename ConnectorType::ADDRESS_T local_SAP, peer_SAP;
      bool is_error = false;
      typename inherited::ISTREAM_T* istream_p = NULL;
      typename ConnectorType::CONFIGURATION_T* configuration_p = NULL;
      typename ConnectorType::USERDATA_T user_data_s;
      bool is_peer_address_b = true;

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
#endif // ACE_WIN32 || ACE_WIN64
          goto error;
        } // end IF

        goto link;
      } // end IF

      // --> open new connection

      // step1: initialize
      // sanity check(s)
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->connectionConfigurations);
      ACE_ASSERT (!connection_);

      iterator =
        inherited::configuration_->connectionConfigurations->find (Stream_Tools::sanitizeUniqueName (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ())));
      if (iterator == inherited::configuration_->connectionConfigurations->end ())
        iterator =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: applying connection configuration\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
      ACE_ASSERT (iterator != inherited::configuration_->connectionConfigurations->end ());
      configuration_p =
          static_cast<typename ConnectorType::CONFIGURATION_T*> ((*iterator).second);
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
          peer_SAP = socket_configuration_p->listenAddress;
          is_peer_address_b = false;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown transport layer type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      connector_.transportLayer ()));
          is_error = true;
          goto reset;
        }
      } // end SWITCH

      // step2: connect
      handle_h =
        Net_Client_Common_Tools::connect<ConnectorType> (connector_,
                                                         *configuration_p,
                                                         user_data_s,
                                                         peer_SAP,
                                                         true, // wait ?
                                                         is_peer_address_b);
      if (handle_h == ACE_INVALID_HANDLE)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      connection_ = iconnection_manager_p->get (handle_h);
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ())));
        is_error = true;
        goto reset;
      } // end IF
      isOpen_ = true;
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connected to %s (id: %u)\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ()),
      //            connection_->id ()));
      notify_connect = true;

      // update session data
      connection_->info (handle_h,
                         local_SAP, peer_SAP);
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.connectionStates.insert (std::make_pair (handle_h,
                                                                &const_cast<typename ConnectorType::CONNECTION_MANAGER_T::STATE_T&> (connection_->state ())));
      } // end lock scope
      handles_.push_back (handle_h);

reset:
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
      ACE_ASSERT (istream_connection_p);
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
#if defined (_DEBUG)
      stream_p->dump_state ();
#endif // _DEBUG

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
        ACE_ASSERT (istream_connection_p);
        typename inherited::ISTREAM_T* istream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        istream_p->_unlink ();
        unlink_ = false;
      } // end IF

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        Stream_ConnectionStatesIterator_t iterator = session_data_r.connectionStates.find (handle_h);
        ACE_ASSERT (iterator != session_data_r.connectionStates.end ());
        session_data_r.connectionStates.erase (iterator);
      } // end lock scope
      handles_.pop_back ();

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s (id was: %u)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (peer_SAP).c_str ()),
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
      if (notify_connect)
        inherited::notify (STREAM_SESSION_MESSAGE_CONNECT);

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
      // *WARNING*: never flush any server-side inbound data

      // *NOTE*: when the connection is (asynchronously) closed by the peer, the
      //         connection handler will finished() the connection processing
      //         stream. If it is linked to another processing stream (e.g.
      //         'this') at this stage, the 'session end' message is propagated.
      //         Note that when the (linked) processing stream (e.g. 'this')
      //         itself is finished() (see below), it sends its own session end
      //         message. Handle both scenarios (and race conditions) here, i.e.
      //         never process consecutive 'session end' messages
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      typename inherited::ISTREAM_T* istream_p = NULL;

      if (inherited::isRunning ())
      {
        { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
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
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
      } // end IF

      if (connection_)
      {
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        ACE_ASSERT (istream_connection_p);
        typename ConnectorType::STREAM_T* stream_p =
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
            const_cast<typename inherited::TASK_BASE_T::ISTREAM_T*> (inherited::getP ());
        ACE_ASSERT (istream_p);
        istream_p->_unlink ();
        unlink_ = false;
      } // end IF

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        Stream_ConnectionStatesIterator_t iterator = session_data_r.connectionStates.find (handles_.back ());
        ACE_ASSERT (iterator != session_data_r.connectionStates.end ());
        session_data_r.connectionStates.erase (iterator);
      } // end lock scope
      handles_.pop_back ();

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        Net_ConnectionId_t id = connection_->id ();
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection (id was: %u)\n"),
                    inherited::mod_->name (),
                    id));
      } // end IF
      isOpen_ = false;

      if (connection_)
      {
        connection_->decrease (); connection_ = NULL;
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
                            StatisticContainerType,
                            TimerManagerType,
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
