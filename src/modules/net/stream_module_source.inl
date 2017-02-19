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

#include <ace/INET_Addr.h>
#include <ace/Log_Msg.h>

#include "common_ilock.h"
#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "net_common.h"
#include "net_common_tools.h"
#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

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
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Reader_T::Stream_Module_Net_Source_Reader_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename ConnectionManagerType>
Stream_Module_Net_Source_Reader_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  ConnectionManagerType>::~Stream_Module_Net_Source_Reader_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Reader_T::~Stream_Module_Net_Source_Reader_T"));

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

      //Stream_Task_t* task_p = inherited::sibling ();
      //if (!task_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_Task::sibling(): \"%m\", returning\n"),
      //              inherited::mod_->name ()));
      //  return;
      //} // end IF
      //SIBLING_T* sibling_p = dynamic_cast<SIBLING_T*> (task_p);
      //if (!sibling_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to dynamic_cast<Stream_Module_Net_Source_Reader_T::SIBLING_T*>(0x%@): \"%m\", returning\n"),
      //              inherited::mod_->name (),
      //              task_p));
      //  return;
      //} // end IF
      //typename ConnectionManagerType::CONNECTION_T* connection_p =
      //  sibling_p->get ();
      //if (!connection_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to retrieve connection handle, returning\n"),
      //              inherited::mod_->name ()));
      //  return;
      //} // end IF

      //connection_p->close ();
      //connection_p->release ();

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
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  AddressType,
                                  ConnectionManagerType,
                                  ConnectorType>::Stream_Module_Net_Source_Writer_T ()
 : inherited ()
 , address_ ()
 , connector_ (NULL,
               ACE_Time_Value::zero)
 , connection_ (NULL)
 , isLinked_ (false)
 , isOpen_ (false)
 , isPassive_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::Stream_Module_Net_Source_Writer_T"));

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
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  AddressType,
                                  ConnectionManagerType,
                                  ConnectorType>::~Stream_Module_Net_Source_Writer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::~Stream_Module_Net_Source_Writer_T"));

  int result = -1;

  if (connection_)
  {
    if (isLinked_)
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
      Stream_IStream* stream_p =
        &const_cast<typename ConnectorType::ISTREAM_CONNECTION_T::STREAM_T&> (istream_connection_p->stream ());
      stream_p->_unlink ();
    } // end IF

    if (!isPassive_ &&
        isOpen_)
    {
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      AddressType local_address, peer_address;
      connection_->info (handle,
                         local_address, peer_address);
      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed connection to %s...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));
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
          typename AddressType,
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  AddressType,
                                  ConnectionManagerType,
                                  ConnectorType>::initialize (const ConfigurationType& configuration_in,
                                                              Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    address_.reset ();

    if (connection_)
    {
      if (isLinked_)
      {
        typename ConnectorType::ISTREAM_CONNECTION_T* stream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!stream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T> (%@): \"%m\", aborting\n"),
                      connection_));
          return false;
        } // end IF
        Stream_IStream* stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (stream_connection_p->stream ());
        stream_p->_unlink ();
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                    inherited::mod_->name ()));
      } // end IF
      isLinked_ = false;

      if (!isPassive_ &&
          isOpen_)
      {
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        AddressType local_address, peer_address;
        connection_->info (handle,
                           local_address, peer_address);

        connection_->close ();
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));
      } // end IF

      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    stream_ = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.socketConfiguration);

  // *TODO*: remove type inferences
  address_ = configuration_in.socketConfiguration->address;
  isPassive_ = configuration_in.passive;
  if (isPassive_)
  {
    if (configuration_in.connection)
    {
      configuration_in.connection->increase ();
      connection_ = configuration_in.connection;
    } // end IF
  } // end IF
  stream_ = configuration_in.stream;

  return inherited::initialize (configuration_in,
                                allocator_in);
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
void
Stream_Module_Net_Source_Writer_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Source_Writer_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (inherited::sessionData_);

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      if (!isPassive_ &&
          connection_ &&
          isOpen_)
      {
        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        AddressType local_address, peer_address;
        connection_->info (handle,
                           local_address, peer_address);
        connection_->close ();
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->connectionManager);

      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        (inherited::configuration_->connectionManager ? inherited::configuration_->connectionManager
                                                      : NULL);
      typename ConnectorType::STREAM_T* stream_p = NULL;
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      typename SessionMessageType::DATA_T* session_data_container_p = NULL;
      ACE_HANDLE handle = ACE_INVALID_HANDLE;

      if (isPassive_)
      {
        // sanity check(s)
        if (connection_)
          goto continue_; // --> using configured connection

        // --> using session connection

        // sanity check(s)
        ACE_ASSERT (iconnection_manager_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_ASSERT (reinterpret_cast<ACE_HANDLE> (session_data_r.sessionID) != ACE_INVALID_HANDLE);
#else
        ACE_ASSERT (static_cast<ACE_HANDLE> (session_data_r.sessionID) != ACE_INVALID_HANDLE);
#endif
        connection_ =
          iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (session_data_r.sessionID));
        if (!connection_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (id was: %u), returning\n"),
                      inherited::mod_->name (),
                      session_data_r.sessionID));
          return;
        } // end IF

        goto continue_;
      } // end IF

      // --> open new connection

      ACE_ASSERT (inherited::configuration_->socketHandlerConfiguration);

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
      ACE_ASSERT (!connection_);
      handle = iconnector_p->connect (address_);
      if (iconnector_p->useReactor ())
      {
        if (handle != ACE_INVALID_HANDLE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          connection_ =
            iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
          connection_ =
            iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif
      } // end IF
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
          connection_ = iconnection_manager_p->get (address_);
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
              typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
                dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
              if (!istream_connection_p)
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T>(0x%@), returning\n"),
                            inherited::mod_->name (),
                            connection_));
                goto reset;
              } // end IF
              istream_connection_p->wait (STREAM_STATE_RUNNING,
                                          NULL); // <-- block
              break;
            } // end IF
          } // end IF
        } while (COMMON_TIME_NOW < deadline);
      } // end ELSE
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddress2String (address_).c_str ())));

        // clean up
        iconnector_p->abort ();

        goto reset;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: connected to %s...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddress2String (address_).c_str ())));

reset:
      inherited::configuration_->streamConfiguration->cloneModule =
        clone_module;
      inherited::configuration_->streamConfiguration->deleteModule =
        delete_module;
      inherited::configuration_->streamConfiguration->module = module_p;
      if (!connection_)
        goto error;

      // link processing streams
      istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T*>(0x%@): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF
      stream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      ACE_ASSERT (stream_);
      if (!stream_->link (stream_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IStream::link(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: linked i/o stream(s)\n"),
                  inherited::mod_->name ()));
      isLinked_ = true;
      //stream_p->dump_state ();

      // update session data in the session message
      // *WARNING*: this works if the STREAM_SESSION_LINK message has already
      //            been processed at this point (i.e. OK only as long as
      //            upstream is completely synchronous)
      inherited::sessionData_->increase ();
      session_data_container_p = inherited::sessionData_;
      message_inout->initialize (STREAM_SESSION_MESSAGE_BEGIN,
                                 session_data_container_p,
                                 &const_cast<typename SessionMessageType::USER_DATA_T&> (message_inout->data ()));
      session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->get ());

      goto continue_;

error:
      if (connection_)
      {
        if (isOpen_)
        {
          connection_->close ();
          isOpen_ = false;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed connection to %s...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddress2String (inherited::configuration_->socketConfiguration->address).c_str ())));
        } // end IF

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
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.connectionState =
          &const_cast<typename ConnectionManagerType::STATE_T&> (connection_->state ());
        session_data_r.sessionID = connection_->id ();
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - the connection has been closed and the processing
      //                     stream has/is finished()-ing (see: handle_close())
      //                   - the session is being aborted by the user
      //                   - the session is being aborted by some module
      // *NOTE*: in any case, the connection has been closed at this point

      //// *NOTE*: The connection handler will finished() the connection
      ////         processing stream when it returns (see handle_close()). If it
      ////         is link()ed to another processing stream at this stage (e.g.
      ////         'this'), the 'session end' message is propagated.
      ////         Note that when the (linked) processing stream (e.g. 'this')
      ////         itself is finished() (see below), it propagates a second
      ////         'session end' message. Handle this situation here; there is
      ////         nothing more to do in this case
      //{ ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

      //  if (sessionEndInProgress_)
      //  {
      //    passMessageDownstream_out = false;
      //    message_inout->release ();

      //    break; // done
      //  } // end IF

      //  sessionEndInProgress_ = true;
      //} // end lock scope

      if (connection_)
      {
        // wait for upstream data processing to complete before unlinking ?

        // *NOTE*: if the connection has been closed (see above), this is the
        //         final session message, so there is no need to wait
        //         --> proceed with unlink()
        //         Otherwise, the session has been aborted (see above), and
        //         there may be unprocessed data in the connection stream
        //         --> flush() any remaining data first ?
        // *TODO*: try not to flush server-side inbound data
        if (!session_data_r.aborted)
          goto error_2;

        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T> (0x%@): \"%m\", returning\n"),
                      connection_));
          goto error_2;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        stream_p->flush (true,   // flush inbound data ?
                         false,  // flush session messages ?
                         false); // flush upstream (if any) ?
      } // end IF

error_2:
      if (connection_)
      {
        if (isLinked_)
        {
          // sanity check(s)
          ACE_ASSERT (stream_);

          stream_->_unlink ();
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                      inherited::mod_->name ()));
          //inherited::configuration_->stream->dump_state ();
        } // end IF
        isLinked_ = false;

        if (!isPassive_ &&
            isOpen_)
        {
          ACE_HANDLE handle = ACE_INVALID_HANDLE;
          AddressType local_address, peer_address;
          connection_->info (handle,
                             local_address, peer_address);
          connection_->close ();
          isOpen_ = false;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed connection to %s...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));
        } // end IF

        connection_->decrease ();
        connection_ = NULL;
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
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
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::Stream_Module_Net_SourceH_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                        bool generateSessionMessages_in,
                                                                        bool isPassive_in)
 : inherited (lock_in,
              false,
              STREAM_HEADMODULECONCURRENCY_CONCURRENT,
              generateSessionMessages_in)
 , address_ ()
 , connector_ (NULL,
               ACE_Time_Value::zero)
 , connection_ (NULL)
 , isLinked_ (false)
 , isOpen_ (false)
 , isPassive_ (isPassive_in)
 //, lock_ ()
 //, sessionEndInProgress_ (false)
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
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::~Stream_Module_Net_SourceH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::~Stream_Module_Net_SourceH_T"));

  int result = -1;

  if (connection_)
  {
    if (isLinked_)
    {
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
        dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T> (%@): \"%m\", returning\n"),
                    connection_));
        return;
      } // end IF
      Stream_IStream* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p->_unlink ();
      ACE_ASSERT (inherited::mod_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                  inherited::mod_->name ()));
    } // end IF

    ACE_HANDLE handle = ACE_INVALID_HANDLE;
    ACE_INET_Addr local_address, peer_address;
    connection_->info (handle,
                       local_address, peer_address);

    connection_->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("closed connection to \"%s\" in dtor --> check implementation !\n"),
                ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));

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
                            ConnectionManagerType,
                            ConnectorType,
                            UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_SourceH_T::initialize"));

  bool result = false;
  int result_2 = -1;

  if (inherited::isInitialized_)
  {
    address_.reset ();

    if (connection_)
    {
      if (isLinked_)
      {
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<Net_ISocketConnection_T> (%@): \"%m\", aborting\n"),
                      connection_));
          return false;
        } // end IF
        Stream_IStream* stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        stream_p->_unlink ();
        ACE_ASSERT (inherited::mod_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                    inherited::mod_->name ()));
      } // end IF
      isLinked_ = false;

      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      ACE_INET_Addr local_address, peer_address;
      connection_->info (handle,
                         local_address, peer_address);

      connection_->close ();
      isOpen_ = false;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed connection to %s...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));

      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    stream_ = configuration_in.stream;
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.socketConfiguration);

  // *TODO*: remove type inferences
  address_ = configuration_in.socketConfiguration->address;

  isPassive_ = inherited::configuration_->passive;
  if (isPassive_)
  {
    if (inherited::configuration_->connection)
    {
      inherited::configuration_->connection->increase ();
      connection_ = inherited::configuration_->connection;
    } // end IF
  } // end IF
  stream_ = inherited::configuration_->stream;

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
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::mod_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      if (!isPassive_ &&
          connection_ &&
          isOpen_)
      {
        // sanity check(s)
        ACE_ASSERT (inherited::sessionData_);

        SessionDataType* session_data_p =
          &const_cast<SessionDataType&> (inherited::sessionData_->get ());

        ACE_HANDLE handle = ACE_INVALID_HANDLE;
        ACE_INET_Addr local_address, peer_address;
        connection_->info (handle,
                           local_address, peer_address);
        connection_->close ();
        isOpen_ = false;
        // *TODO*: remove type inference
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session (id was: %u) aborted, closed connection to %s...\n"),
                    inherited::mod_->name (),
                    session_data_p->sessionID,
                    ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType* session_data_p =
        &const_cast<SessionDataType&> (inherited::sessionData_->get ());

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
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
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
        ACE_ASSERT (reinterpret_cast<ACE_HANDLE> (session_data_p->sessionID) != ACE_INVALID_HANDLE);
#else
        ACE_ASSERT (static_cast<ACE_HANDLE> (session_data_p->sessionID) != ACE_INVALID_HANDLE);
#endif
        connection_ =
          iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (session_data_p->sessionID));
        if (!connection_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to retrieve connection (id was: %u), returning\n"),
                      inherited::mod_->name (),
                      session_data_p->sessionID));
          return;
        } // end IF

        goto continue_;
      } // end IF

      // --> open new connection

      ACE_ASSERT (inherited::configuration_->socketConfiguration);
      ACE_ASSERT (inherited::configuration_->socketHandlerConfiguration);

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
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: connecting to %s...\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Net_Common_Tools::IPAddress2String (address_).c_str ())));

      ACE_ASSERT (!connection_);
      handle = iconnector_p->connect (address_);
      if (iconnector_p->useReactor ())
      {
        if (handle != ACE_INVALID_HANDLE)
          connection_ =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
            iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif
      } // end IF
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
        // *TODO*: this may not be accurate/applicable for all protocols
        bool is_peer_address =
          (iconnector_p->transportLayer () == NET_TRANSPORTLAYER_TCP);
        do
        { // *NOTE*: this could be a TCP (peer address), or a UDP (local
          //         address) connection --> try both
          connection_ = iconnection_manager_p->get (address_,
                                                    is_peer_address);
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
              typename ConnectorType::ISTREAM_CONNECTION_T* isocket_connection_p =
                dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
              if (!isocket_connection_p)
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T>(0x%@), returning\n"),
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
                    ACE_TEXT ("%s: failed to connect to %s, returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddress2String (address_).c_str ())));

        // clean up
        iconnector_p->abort ();

        goto reset;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: connected to %s...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddress2String (address_).c_str ())));

reset:
      inherited::configuration_->streamConfiguration->cloneModule =
          clone_module;
      inherited::configuration_->streamConfiguration->deleteModule =
          delete_module;
      inherited::configuration_->streamConfiguration->module = module_p;
      if (!connection_)
        goto error;

      // link processing streams
      istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T*>(0x%@): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    connection_));
        goto error;
      } // end IF
      stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      ACE_ASSERT (stream_);
      if (!stream_->link (stream_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Stream::link(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: linked i/o stream(s)\n"),
                  inherited::mod_->name ()));
      isLinked_ = true;
      //stream_p->dump_state ();

      // update session data in the session message
      // *WARNING*: this works only if the STREAM_SESSION_LINK message has been
      //            received by now (see below; OK if upstream is completely
      //            synchronous)
      inherited::sessionData_->increase ();
      session_data_container_p = inherited::sessionData_;
      message_inout->initialize (STREAM_SESSION_MESSAGE_BEGIN,
                                 session_data_container_p,
                                 &const_cast<typename SessionMessageType::USER_DATA_T&> (message_inout->data ()));
      session_data_p =
        &const_cast<SessionDataType&> (inherited::sessionData_->get ());

      goto continue_;

error:
      if (connection_)
      {
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddress2String (address_).c_str ())));
        isOpen_ = false;

        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      // sanity check(s)
      ACE_ASSERT (connection_);

      // set session ID
      // *TODO*: remove type inferences
      ACE_ASSERT (session_data_p->lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);

        session_data_p->sessionID = connection_->id ();
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_DISCONNECT:
    {
      isOpen_ = false;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: disconnected...\n"),
                  inherited::mod_->name ()));
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - the connection has been closed and the processing
      //                     stream has/is finished()-ing (see: handle_close())
      //                   [- the session is being aborted by the user
      //                   - the session is being aborted by some module]

      // *NOTE*: when the connection is (asynchronously) closed by the peer, the
      //         connection handler will finished() the connection processing
      //         stream. If it is linked to another processing stream (e.g.
      //         'this') at this stage, the 'session end' message is propagated.
      //         Note that when the (linked) processing stream (e.g. 'this')
      //         itself is finished() (see below), it sends its own session end
      //         message. Handle both scenarios (and race conditions) here
      //         --> only process the first 'session end' message
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

        if (inherited::sessionEndProcessed_) break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (inherited::isRunning ())
      {
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);

          //// sanity check(s)
          //ACE_ASSERT (!inherited::sessionEndSent_);

          inherited::sessionEndSent_ = true;
        } // end lock scope
      } // end IF
      //if (inherited::sessionEndProcessed_)
      //{
      //  // clean up
      //  message_inout->release ();
      //  message_inout = NULL;
      //  passMessageDownstream_out = false;
      //  break;
      //} // end IF

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
        // sanity check(s)
        ACE_ASSERT (inherited::sessionData_);

        SessionDataType* session_data_p =
          &const_cast<SessionDataType&> (inherited::sessionData_->get ());

        // wait for upstream data processing to complete before unlinking ?

        // *NOTE*: if the connection has been closed (see above), this is the
        //         final session message, so there is no need to wait
        //         --> proceed with unlink()
        //         Otherwise, the session has been aborted (see above), and
        //         there may be unprocessed data in the connection stream
        //         --> flush() any remaining data first ?
        // *TODO*: try not to flush server-side inbound data
        // *TODO*: remove type inferences
        ACE_ASSERT (session_data_p->lock);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_p->lock);

          if (!session_data_p->aborted)
            goto error_2;
        } // end lock scope

        typename ConnectorType::STREAM_T* stream_p = NULL;
        typename ConnectorType::ISTREAM_CONNECTION_T* isocket_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!isocket_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T> (0x%@): \"%m\", returning\n"),
                      connection_));
          goto error_2;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (isocket_connection_p->stream ());
        stream_p->flush (true);
      } // end IF

error_2:
      if (connection_)
      {
        if (isLinked_)
        {
          // sanity check(s)
          ACE_ASSERT (stream_);

          stream_->_unlink ();
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                      inherited::mod_->name ()));
          //inherited::configuration_->stream->dump_state ();
        } // end IF
        isLinked_ = false;

        if (!isPassive_ &&
            isOpen_)
        {
          ACE_HANDLE handle = ACE_INVALID_HANDLE;
          ACE_INET_Addr local_address, peer_address;
          connection_->info (handle,
                             local_address, peer_address);
          connection_->close ();
          isOpen_ = false;
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed connection to %s...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Net_Common_Tools::IPAddress2String (peer_address).c_str ())));
        } // end IF

        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      if (inherited::concurrency_ != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
        inherited::TASK_BASE_T::stop (false,  // wait for completion ?
                                      false); // N/A

      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: upstream has been linked...\n"),
                  inherited::mod_->name ()));
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

//template <ACE_SYNCH_DECL,
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
//Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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

//template <ACE_SYNCH_DECL,
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
//Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
//    try {
//      // *TODO*: remove type inference
//      message_out =
//          static_cast<ProtocolMessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
//    } catch (...) {
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
//template <ACE_SYNCH_DECL,
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
//Stream_Module_Net_SourceH_T<ACE_SYNCH_USE,
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
