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

#ifndef HTTP_GET_NETWORK_H
#define HTTP_GET_NETWORK_H

#include <map>
#include <string>

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/SSL/SSL_SOCK_Stream.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_module_io_stream.h"
#include "stream_session_data.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#include "net_client_ssl_connector.h"

// forward declarations
struct HTTPGet_AllocatorConfiguration;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct HTTPGet_AllocatorConfiguration> HTTPGet_ControlMessage_t;
class HTTPGet_Message;
class HTTPGet_SessionMessage;
struct HTTPGet_ConnectionConfiguration;
struct HTTPGet_ConnectionState;
struct HTTPGet_ModuleHandlerConfiguration;
struct HTTPGet_SessionData;
typedef Stream_SessionData_T<struct HTTPGet_SessionData> HTTPGet_SessionData_t;
struct HTTPGet_StreamState;
//struct HTTPGet_UserData;

struct HTTPGet_Configuration;
struct HTTPGet_ConnectionState
 : Net_ConnectionState
{
  inline HTTPGet_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   //, currentStatistic ()
   , userData (NULL)
  {};

  struct HTTPGet_Configuration* configuration;

  //ARDrone_RuntimeStatistic_t    currentStatistic;

  struct Stream_UserData*       userData;
};

struct HTTPGet_ConnectionConfiguration;
struct HTTPGet_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline HTTPGet_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct HTTPGet_ConnectionConfiguration* connectionConfiguration;

  struct Stream_UserData*                 userData;
};

struct HTTPGet_StreamConfiguration;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct HTTPGet_ConnectionConfiguration,
                                 struct HTTPGet_ConnectionState,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> HTTPGet_IConnectionManager_t;
struct HTTPGet_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline HTTPGet_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   , connectionManager (NULL)
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  HTTPGet_IConnectionManager_t*             connectionManager;
  struct HTTPGet_SocketHandlerConfiguration socketHandlerConfiguration;
  struct HTTPGet_StreamConfiguration*       streamConfiguration;

  struct Stream_UserData*                   userData;
};
typedef std::map<std::string,
                 struct HTTPGet_ConnectionConfiguration> HTTPGet_ConnectionConfigurations_t;
typedef HTTPGet_ConnectionConfigurations_t::iterator HTTPGet_ConnectionConfigurationIterator_t;

typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct HTTPGet_ConnectionConfiguration,
                                 struct HTTPGet_ConnectionState,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> HTTPGet_ConnectionManager_t;

//////////////////////////////////////////

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct HTTPGet_StreamState,
                                      struct HTTPGet_StreamConfiguration,
                                      struct Stream_Statistic,
                                      struct Stream_ModuleConfiguration,
                                      struct HTTPGet_ModuleHandlerConfiguration,
                                      struct HTTPGet_SessionData,
                                      HTTPGet_SessionData_t,
                                      HTTPGet_ControlMessage_t,
                                      HTTPGet_Message,
                                      HTTPGet_SessionMessage,
                                      ACE_INET_Addr,
                                      HTTPGet_ConnectionManager_t,
                                      struct Stream_UserData> HTTPGet_NetStream_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct HTTPGet_ConnectionConfiguration,
                          struct HTTPGet_ConnectionState,
                          struct Stream_Statistic> HTTPGet_IConnection_t;

//////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct HTTPGet_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct HTTPGet_ConnectionConfiguration,
                                  struct HTTPGet_ConnectionState,
                                  struct Stream_Statistic,
                                  HTTPGet_NetStream_t,
                                  struct Stream_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct HTTPGet_ModuleHandlerConfiguration> HTTPGet_TCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct HTTPGet_SocketHandlerConfiguration,
                                                         ACE_SSL_SOCK_Stream>,
                                  ACE_INET_Addr,
                                  struct HTTPGet_ConnectionConfiguration,
                                  struct HTTPGet_ConnectionState,
                                  struct Stream_Statistic,
                                  HTTPGet_NetStream_t,
                                  struct Stream_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct HTTPGet_ModuleHandlerConfiguration> HTTPGet_SSLTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct HTTPGet_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct HTTPGet_ConnectionConfiguration,
                                        struct HTTPGet_ConnectionState,
                                        struct Stream_Statistic,
                                        HTTPGet_NetStream_t,
                                        struct Stream_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct HTTPGet_ModuleHandlerConfiguration> HTTPGet_AsynchTCPHandler_t;

typedef Net_TCPConnectionBase_T<HTTPGet_TCPHandler_t,
                                struct HTTPGet_ConnectionConfiguration,
                                struct HTTPGet_ConnectionState,
                                struct Stream_Statistic,
                                struct HTTPGet_SocketHandlerConfiguration,
                                struct Net_SocketConfiguration,
                                HTTPGet_NetStream_t,
                                struct Stream_UserData> HTTPGet_TCPConnection_t;
typedef Net_TCPConnectionBase_T<HTTPGet_SSLTCPHandler_t,
                                struct HTTPGet_ConnectionConfiguration,
                                struct HTTPGet_ConnectionState,
                                struct Stream_Statistic,
                                struct HTTPGet_SocketHandlerConfiguration,
                                struct Net_SocketConfiguration,
                                HTTPGet_NetStream_t,
                                struct Stream_UserData> HTTPGet_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<HTTPGet_AsynchTCPHandler_t,
                                      struct HTTPGet_ConnectionConfiguration,
                                      struct HTTPGet_ConnectionState,
                                      struct Stream_Statistic,
                                      struct HTTPGet_SocketHandlerConfiguration,
                                      struct Net_SocketConfiguration,
                                      HTTPGet_NetStream_t,
                                      struct Stream_UserData> HTTPGet_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct HTTPGet_ConnectionConfiguration> HTTPGet_Stream_IInetConnector_t;

/////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<HTTPGet_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct HTTPGet_ConnectionConfiguration,
                                     struct HTTPGet_ConnectionState,
                                     struct Stream_Statistic,
                                     struct HTTPGet_SocketHandlerConfiguration,
                                     HTTPGet_NetStream_t,
                                     struct Stream_UserData> HTTPGet_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<HTTPGet_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct HTTPGet_ConnectionConfiguration,
                               struct HTTPGet_ConnectionState,
                               struct Stream_Statistic,
                               struct HTTPGet_SocketHandlerConfiguration,
                               HTTPGet_NetStream_t,
                               struct Stream_UserData> HTTPGet_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<HTTPGet_SSLTCPConnection_t,
                                   Net_SOCK_Connector,
                                   ACE_INET_Addr,
                                   struct HTTPGet_ConnectionConfiguration,
                                   struct HTTPGet_ConnectionState,
                                   struct Stream_Statistic,
                                   struct HTTPGet_SocketHandlerConfiguration,
                                   HTTPGet_NetStream_t,
                                   struct Stream_UserData> HTTPGet_SSLTCPConnector_t;

#endif
