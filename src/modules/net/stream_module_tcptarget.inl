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
#include "ace/SOCK_Stream.h"

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
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_TCPTarget_T<SessionMessageType,
                          MessageType,
                          ConfigurationType,
                          ModuleHandlerConfigurationType,
                          SessionDataType,
                          ConnectionManagerType,
                          ConnectorType>::Stream_Module_TCPTarget_T ()
 : inherited ()
 , configuration_ ()
 , connection_ (NULL)
 , isInitialized_ (false)
 , isLinked_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPTarget_T::Stream_Module_TCPTarget_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ConnectionManagerType,
          typename ConnectorType>
Stream_Module_TCPTarget_T<SessionMessageType,
                          MessageType,
                          ConfigurationType,
                          ModuleHandlerConfigurationType,
                          SessionDataType,
                          ConnectionManagerType,
                          ConnectorType>::~Stream_Module_TCPTarget_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPTarget_T::~Stream_Module_TCPTarget_T"));

  int result = -1;
  ACE_TCHAR buffer[BUFSIZ];

  if (isLinked_)
  {
    ACE_ASSERT (connection_);
    typename ConnectorType::STREAM_T& stream_r =
      const_cast<typename ConnectorType::STREAM_T&> (connection_->stream ());
    Stream_Module_t* module_p = NULL;
    result = stream_r.top (module_p);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", continuing\n")));
      goto close;
    } // end IF
    ACE_ASSERT (module_p);
    stream_r.head ()->link (module_p);
  } // end IF

close:
  if (connection_)
  {
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
                ACE_TEXT ("closed connection to \"%s\" in dtor...\n"),
                ACE_TEXT (buffer)));
    connection_->decrease ();
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_TCPTarget_T<SessionMessageType,
                          MessageType,
                          ConfigurationType,
                          ModuleHandlerConfigurationType,
                          SessionDataType,
                          ConnectionManagerType,
                          ConnectorType>::handleDataMessage (MessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPTarget_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (connection_);
  ACE_ASSERT (message_inout);
  if (!connection_->send (*message_inout))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Net_IConnection_T::send(%d): \"%m\", continuing\n"),
                message_inout->total_length ()));
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ConnectionManagerType,
          typename ConnectorType>
void
Stream_Module_TCPTarget_T<SessionMessageType,
                          MessageType,
                          ConfigurationType,
                          ModuleHandlerConfigurationType,
                          SessionDataType,
                          ConnectionManagerType,
                          ConnectorType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPTarget_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_END:
    {
      ACE_TCHAR buffer[BUFSIZ];

      if (isLinked_)
      {
        ACE_ASSERT (connection_);
        typename ConnectorType::STREAM_T& stream_r =
          const_cast<typename ConnectorType::STREAM_T&> (connection_->stream ());
        Stream_Module_t* module_p = NULL;
        result = stream_r.top (module_p);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", continuing\n")));
          goto close;
        } // end IF
        ACE_ASSERT (module_p);
        stream_r.head ()->link (module_p);
        isLinked_ = false;
      } // end IF

close:
      if (connection_)
      {
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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("closed connection to \"%s\"...\n"),
                    ACE_TEXT (buffer)));
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
          typename ConnectionManagerType,
          typename ConnectorType>
bool
Stream_Module_TCPTarget_T<SessionMessageType,
                          MessageType,
                          ConfigurationType,
                          ModuleHandlerConfigurationType,
                          SessionDataType,
                          ConnectionManagerType,
                          ConnectorType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPTarget_T::initialize"));

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

    if (isLinked_)
    {
      ACE_ASSERT (connection_);
      typename ConnectorType::STREAM_T& stream_r =
        const_cast<typename ConnectorType::STREAM_T&> (connection_->stream ());
      Stream_Module_t* module_p = NULL;
      result = stream_r.top (module_p);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", aborting\n")));
        return false;
      } // end IF
      ACE_ASSERT (module_p);
      stream_r.head ()->link (module_p);
      isLinked_ = false;
    } // end IF

    if (connection_)
    {
      connection_->close ();
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed connection to \"%s\"...\n"),
                  ACE_TEXT (buffer)));
      connection_->decrease ();
      connection_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  // step1: initialize connector
  // *TODO*: remove type inferences
  ConnectionManagerType* connection_manager_p =
      configuration_.connectionManager;
  ACE_ASSERT (connection_manager_p);
  typename ConnectionManagerType::INTERFACE_T* iconnection_manager_p =
    connection_manager_p;
  ACE_ASSERT (configuration_.streamConfiguration);
  ConnectorType connector (iconnection_manager_p,
                           configuration_.streamConfiguration->statisticReportingInterval);
//  Stream_IInetConnector_t* iconnector_p = &connector;
  ACE_HANDLE handle = ACE_INVALID_HANDLE;
  Stream_Module_t* module_p = NULL;
  typename SessionMessageType::USER_DATA_T* user_data_p = NULL;
  ACE_ASSERT (configuration_.configuration);
  if (!connector.initialize (configuration_.configuration->socketHandlerConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize connector: \"%m\", aborting\n")));
    return false;
  } // end IF

  // step2: initialize connection manager
  // *TODO*: remove type inferences
  ConfigurationType configuration;
  connection_manager_p->get (configuration,
                             user_data_p);
  connection_manager_p->set (*configuration_.configuration,
                             user_data_p);

  // step3: connect
  ACE_ASSERT (!connection_);
  handle =
    connector.connect (configuration_.configuration->socketConfiguration.peerAddress);
  const typename ConnectorType::STREAM_T* stream_p = NULL;
  Stream_Module_t* module_2 = NULL;
  if (connector.useReactor ())
    connection_ =
      dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_manager_p->get (handle));
  else
  {
    ACE_Time_Value one_second (1, 0);
    result = ACE_OS::sleep (one_second);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &one_second));
    connection_ =
      dynamic_cast<typename ConnectorType::ISOCKET_CONNECTION_T*> (connection_manager_p->get (configuration_.configuration->socketConfiguration.peerAddress));
  } // end IF
  if (!connection_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to connect to \"%s\", aborting\n"),
                ACE_TEXT (buffer)));
    goto failed;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("connected to \"%s\"...\n"),
              ACE_TEXT (buffer)));

  // step4: forward any inbound data upstream
  ACE_ASSERT (!isLinked_);
  stream_p = &connection_->stream ();
  module_p = inherited::module ();
  ACE_ASSERT (module_p);
  result =
    const_cast<typename ConnectorType::STREAM_T*> (stream_p)->top (module_2);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", aborting\n")));
    goto failed;
  } // end IF
  ACE_ASSERT (module_2);
  module_2->reader ()->next (module_p->reader ());
  isLinked_ = true;

  isInitialized_ = true;

  return true;

failed:
  if (isLinked_)
  {
    ACE_ASSERT (connection_);
    typename ConnectorType::STREAM_T& stream_r =
      const_cast<typename ConnectorType::STREAM_T&> (connection_->stream ());
    Stream_Module_t* module_p = NULL;
    result = stream_r.top (module_p);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Stream::top(): \"%m\", aborting\n")));
      return false;
    } // end IF
    ACE_ASSERT (module_p);
    stream_r.head ()->link (module_p);
    isLinked_ = false;
  } // end IF

  if (connection_)
  {
    connection_->close ();
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed connection to \"%s\"...\n"),
                ACE_TEXT (buffer)));
    connection_->decrease ();
    connection_ = NULL;
  } // end IF

  return false;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ConnectionManagerType,
          typename ConnectorType>
const ModuleHandlerConfigurationType&
Stream_Module_TCPTarget_T<SessionMessageType,
                          MessageType,
                          ConfigurationType,
                          ModuleHandlerConfigurationType,
                          SessionDataType,
                          ConnectionManagerType,
                          ConnectorType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_TCPTarget_T::get"));

  return configuration_;
}
