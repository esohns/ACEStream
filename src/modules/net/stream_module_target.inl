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
#include "net_iconnector.h"

#include "net_client_defines.h"

#include "stream_module_net_common.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           ConnectionConfigurationIteratorType,
                           ConnectionManagerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           ConnectorType>::Stream_Module_Net_Target_T (ISTREAM_T* stream_in,
#else
                           ConnectorType>::Stream_Module_Net_Target_T (typename inherited::ISTREAM_T* stream_in,
#endif
                                                                       bool isPassive_in)
 : inherited (stream_in)
 , connection_ (NULL)
 , connector_ (NULL,
               ACE_Time_Value::zero)
 /////////////////////////////////////////
 , address_ ()
 , isOpen_ (false)
 , isPassive_ (isPassive_in)
 , lock_ ()
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
          typename ConnectionConfigurationIteratorType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_Net_Target_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           ConnectionConfigurationIteratorType,
                           ConnectionManagerType,
                           ConnectorType>::~Stream_Module_Net_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::~Stream_Module_Net_Target_T"));

  connector_.abort ();

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
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: unlinked i/o stream(s) in dtor --> check implementation !\n"),
                inherited::mod_->name ()));
  } // end IF

close:
  if (isOpen_ &&
      !isPassive_)
  { ACE_ASSERT (connection_);
    connection_->close ();
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: closed connection to %s in dtor --> check implementation !\n"),
                inherited::mod_->name (),
                ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
  } // end IF

  if (connection_)
    connection_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename ConnectionConfigurationIteratorType,
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
                           ConnectionConfigurationIteratorType,
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
      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        // sanity check(s)
        ACE_ASSERT (inherited::sessionData_);
        typename SessionDataContainerType::DATA_T* session_data_p =
          &const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->get ());

        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session %u aborted, closed connection to %s\n"),
                    inherited::mod_->name (),
                    session_data_p->sessionID,
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

      typename SessionDataContainerType::DATA_T& session_data_r =
          const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->get ());

      // *TODO*: remove type inferences
      typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
        (inherited::configuration_->connectionManager ? inherited::configuration_->connectionManager
                                                      : NULL);
//      ConnectionManagerType* connection_manager_p =
//          dynamic_cast<ConnectionManagerType*> (iconnection_manager_p);
      typename ConnectorType::ICONNECTOR_T* iconnector_p = &connector_;
      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename inherited::ISTREAM_T* istream_p = NULL;
      typename ConnectorType::STREAM_T::MODULE_T* module_p = NULL;
      enum Net_Connection_Status status = NET_CONNECTION_STATUS_INVALID;
      ACE_HANDLE handle = ACE_INVALID_HANDLE;
      bool clone_module, delete_module;
      STREAM_T* stream_2 = NULL;
      bool notify_connect = false;
      bool is_inbound = true;
      CONFIGURATION_ITERATOR_T iterator;
      ConfigurationType* module_configuration_p = NULL;
      TASK_T* task_p = NULL;
      ConnectionConfigurationIteratorType iterator_2;

      if (isPassive_)
      {
        // sanity check(s)
        if (connection_)
          goto link; // --> using configured connection

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
                      ACE_TEXT ("%s: failed to retrieve connection (id was: %u), aborting\n"),
                      inherited::mod_->name (),
                      session_data_r.sessionID));
          goto error;
        } // end IF

        goto link;
      } // end IF

      // --> open new connection
//
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
      // *TODO*: remove type inferences
      ACE_ASSERT (inherited::configuration_->connectionConfigurations);
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      iterator_2 =
        inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
      if (iterator_2 == inherited::configuration_->connectionConfigurations->end ())
        iterator_2 =
          inherited::configuration_->connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
      //else
      //  ACE_DEBUG ((LM_DEBUG,
      //              ACE_TEXT ("%s: applying connection configuration\n"),
      //              inherited::mod_->name ()));
      ACE_ASSERT (iterator_2 != inherited::configuration_->connectionConfigurations->end ());

      // step2: initialize connector
      // *NOTE*: the stream configuration may contain a module handle that is
      //         meant to be the final module of this processing stream. As
      //         the connection stream will be appended to this pipeline, the
      //         connection should not enqueue that same module again
      //         --> temporarily 'hide' the module handle, if any
      // *TODO*: remove this ASAP
      clone_module =
          inherited::configuration_->streamConfiguration->configuration_.cloneModule;
      delete_module =
          inherited::configuration_->streamConfiguration->configuration_.deleteModule;
      module_p =
          inherited::configuration_->streamConfiguration->configuration_.module;
      inherited::configuration_->streamConfiguration->configuration_.cloneModule =
          false;
      inherited::configuration_->streamConfiguration->configuration_.deleteModule =
          false;
      inherited::configuration_->streamConfiguration->configuration_.module =
          NULL;

      if (!iconnector_p->initialize ((*iterator_2).second))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize connector: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto reset;
      } // end IF

      iterator =
          inherited::configuration_->streamConfiguration->find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (iterator != inherited::configuration_->streamConfiguration->end ());
      module_configuration_p =
          const_cast<ConfigurationType*> (static_cast<const ConfigurationType*> (&(*iterator).second));
      ACE_ASSERT (module_configuration_p);
      is_inbound = module_configuration_p->inbound;
      module_configuration_p->inbound = false;

      // step3: connect
      ACE_ASSERT (!connection_);
      // *TODO*: support single-thread operation (e.g. by scheduling a signal
      //         and manually running the dispatch loop for a limited period)
      handle = iconnector_p->connect (address_);
      if (iconnector_p->useReactor ())
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        connection_ =
            iconnection_manager_p->get (reinterpret_cast<Net_ConnectionId_t> (handle));
#else
        connection_ =
            iconnection_manager_p->get (static_cast<Net_ConnectionId_t> (handle));
#endif
      else
      {
        // step3a: wait for the connection to register with the manager
        ACE_Time_Value timeout (NET_CLIENT_DEFAULT_ASYNCH_CONNECT_TIMEOUT, 0);
        ACE_Time_Value deadline = COMMON_TIME_NOW + timeout;
        // *TODO*: avoid these tight loops
        do
        {
          connection_ = iconnection_manager_p->get (address_);
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

          // clean up
          iconnector_p->abort ();

          goto reset;
        } // end IF
        istream_connection_p =
            dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to dynamic_cast<Net_IStreamConnection_T>(0x%@), aborting\n"),
                      inherited::mod_->name (),
                      connection_));
          goto reset;
        } // end IF

        // step3b: wait for the connection to finish initializing
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
          goto reset;
        } // end IF

        // step3c: wait for the connection stream to finish initializing
        istream_connection_p->wait (STREAM_STATE_RUNNING,
                                    NULL); // <-- block
      } // end ELSE
      if (!connection_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to connect to %s, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
        goto reset;
      } // end IF
      isOpen_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: connected to %s\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
      notify_connect = true;

reset:
      inherited::configuration_->streamConfiguration->configuration_.cloneModule =
          clone_module;
      inherited::configuration_->streamConfiguration->configuration_.deleteModule =
          delete_module;
      inherited::configuration_->streamConfiguration->configuration_.module =
          module_p;

      if (module_configuration_p)
        module_configuration_p->inbound = is_inbound;

      if (!connection_)
        goto error;

link:
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
      istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      ACE_ASSERT (inherited::stream_);
      stream_2 = dynamic_cast<Stream_Base_t*> (inherited::stream_);
      ACE_ASSERT (stream_2);
      result = istream_p->link (stream_2);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to Stream_Base_T::link(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (istream_p->name ().c_str ()),
                    ACE_TEXT (inherited::stream_->name ().c_str ())));
        goto error;
      } // end IF
      unlink_ = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: linked i/o stream(s)\n"),
                  inherited::mod_->name ()));
#if defined (_DEBUG)
      ACE_ASSERT (inherited::stream_);
      inherited::stream_->dump_state ();
#endif

      // set up reactor/proactor notification
      task_p = inherited::mod_->reader ();
      ACE_ASSERT (task_p);
      //while (task_p->module () != module_p)
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
                    ACE_TEXT ("%s: no head module reader task found, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (task_p->msg_queue_);
      task_p->msg_queue_->notification_strategy (connection_->notification ());

      goto done;

error:
      if (unlink_)
      {
        unlink_ = false;

        ACE_ASSERT (connection_);
        typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p =
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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                    inherited::mod_->name ()));
      } // end IF

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
      } // end IF
      if (connection_)
      {
        connection_->decrease ();
        connection_ = NULL;
      } // end IF

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

done:
      // set session ID
      ACE_ASSERT (connection_);
      // *TODO*: remove type inference
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, *session_data_r.lock);
        session_data_r.sessionID = connection_->id ();
      } // end lock scope
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: reset session id to %u\n"),
                  inherited::mod_->name (),
                  session_data_r.sessionID));

      if (notify_connect)
        inherited::notify (STREAM_SESSION_MESSAGE_CONNECT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *IMPORTANT NOTE*: control reaches this point because either:
      //                   - the connection has been closed and the processing
      //                     stream has/is finished()-ing (see: handle_close())
      //                   [- the session is being aborted by the user
      //                   - the session is being aborted by some module]

      typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;
      typename ConnectorType::STREAM_T* stream_p = NULL;

      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);

      if (connection_)
      {
        // wait for data (!) processing to complete
//        ACE_ASSERT (configuration_->stream);
        istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
        if (!istream_connection_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T*>(0x%@): \"%m\", continuing\n"),
                      connection_));
          goto unlink;
        } // end IF
        stream_p =
          &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());

        //// *NOTE*: if the connection was closed abruptly, there may well be
        ////         undispatched data in the connection stream. Flush it so
        ////         waitForCompletion() (see below) does not block
        //Net_Connection_Status status = configuration_->connection->status ();
        //if (status != NET_CONNECTION_STATUS_OK)
        //  stream_p->flush (true);
        //configuration_->stream->wait (false, // wait for worker(s) ?
        //                              true); // wait for upstream ?

        // *NOTE*: finalize the (connection) stream state so waitForCompletion()
        //         does not block forever
        // *TODO*: this shouldn't be necessary (--> only wait for data to flush)
        stream_p->stop (false, // wait for completion ?
                        true); // locked access ?
        connection_->waitForCompletion (false); // data only
      } // end IF

unlink:
      if (unlink_)
      {
        unlink_ = false;

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

        if (!istream_connection_p)
        {
          istream_connection_p =
              dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
          if (!istream_connection_p)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to dynamic_cast<ConnectorType::ISTREAM_CONNECTION_T*>(0x%@): \"%m\", continuing\n"),
                        connection_));
            goto release;
          } // end IF
        } // end IF
        if (!stream_p)
          stream_p =
            &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
        stream_p->_unlink ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                    inherited::mod_->name ()));
      } // end IF

      if (isOpen_ &&
          !isPassive_)
      { ACE_ASSERT (connection_);
        connection_->close ();
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: closed connection to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
      } // end IF
      isOpen_ = false;

release:
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
        isOpen_ = false;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: disconnected\n"),
                    inherited::mod_->name ()));
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_CONNECT:
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename ConnectionConfigurationIteratorType,
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
                           ConnectionConfigurationIteratorType,
                           ConnectionManagerType,
                           ConnectorType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Net_Target_T::initialize"));

  typename ConnectorType::ISTREAM_CONNECTION_T* istream_connection_p = NULL;

  if (inherited::isInitialized_)
  {
    if (unlink_)
    {
      unlink_ = false;

      ACE_ASSERT (connection_);
      istream_connection_p =
          dynamic_cast<typename ConnectorType::ISTREAM_CONNECTION_T*> (connection_);
      if (!istream_connection_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to dynamic_cast<Net_IStreamConnection_T>(%@): \"%m\", continuing\n"),
                    connection_));
        goto close;
      } // end IF
      typename inherited::ISTREAM_T* istream_p =
        &const_cast<typename ConnectorType::STREAM_T&> (istream_connection_p->stream ());
      istream_p->_unlink ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: unlinked i/o stream(s)\n"),
                  inherited::mod_->name ()));
    } // end IF

close:
    if (isOpen_ &&
        !isPassive_)
    { ACE_ASSERT (connection_);
      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: closed connection to %s\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Net_Common_Tools::IPAddressToString (address_).c_str ())));
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
    connection_ = configuration_in.connection;
    connection_->increase ();

    isPassive_ = true;
  } // end IF
  else
    isPassive_ = false;

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.connectionConfigurations);
  ConnectionConfigurationIteratorType iterator =
    configuration_in.connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()));
  if (iterator == configuration_in.connectionConfigurations->end ())
    iterator =
      configuration_in.connectionConfigurations->find (ACE_TEXT_ALWAYS_CHAR (""));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: applying connection configuration\n"),
                inherited::mod_->name ()));
  ACE_ASSERT (iterator != configuration_in.connectionConfigurations->end ());
  address_ = (*iterator).second.socketHandlerConfiguration.socketConfiguration.address;

  return inherited::initialize (configuration_in,
                                allocator_in);
}
