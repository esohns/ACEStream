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

#if defined (SSL_SUPPORT)
#include "ace/SSL/SSL_SOCK_Stream.h"
#endif // SSL_SUPPORT

#include "common_configuration.h"
#include "common_time_common.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

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
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "test_u_common.h"

// forward declarations
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Common_FlexParserAllocatorConfiguration> HTTPGet_ControlMessage_t;
class HTTPGet_Message;
class HTTPGet_SessionMessage;
struct HTTPGet_SessionData;
typedef Stream_SessionData_T<struct HTTPGet_SessionData> HTTPGet_SessionData_t;
struct HTTPGet_StreamState;
//struct HTTPGet_UserData;
//extern const char stream_name_string_[];
struct HTTPGet_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Common_FlexParserAllocatorConfiguration,
                               struct Stream_Configuration,
                               struct Stream_ModuleConfiguration,
                               struct HTTPGet_ModuleHandlerConfiguration> HTTPGet_StreamConfiguration_t;
struct HTTPGet_Configuration;

struct HTTPGet_ConnectionConfiguration;
typedef Net_ConnectionConfiguration_T<struct Common_FlexParserAllocatorConfiguration,
                                      HTTPGet_StreamConfiguration_t,
                                      NET_TRANSPORTLAYER_TCP> HTTPGet_ConnectionConfiguration_t;
typedef Net_IConnectionManager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 HTTPGet_ConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> HTTPGet_IConnectionManager_t;
//
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 HTTPGet_ConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> HTTPGet_ConnectionManager_t;

//////////////////////////////////////////

extern const char stream_name_string_[];

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct HTTPGet_StreamState,
                                      struct Stream_Configuration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Common_FlexParserAllocatorConfiguration,
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
                          HTTPGet_ConnectionConfiguration_t,
                          struct Net_StreamConnectionState,
                          Net_StreamStatistic_t> HTTPGet_IConnection_t;

//////////////////////////////////////////

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                HTTPGet_ConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                HTTPGet_NetStream_t,
                                struct Net_UserData> HTTPGet_TCPConnection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                HTTPGet_ConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                HTTPGet_NetStream_t,
                                struct Net_UserData> HTTPGet_SSLTCPConnection_t;
#endif // SSL_SUPPORT
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      HTTPGet_ConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      HTTPGet_NetStream_t,
                                      struct Net_UserData> HTTPGet_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         HTTPGet_ConnectionConfiguration_t> HTTPGet_Stream_IInetConnector_t;

/////////////////////////////////////////

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               HTTPGet_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               HTTPGet_ConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Net_TCPSocketConfiguration_t,
                               HTTPGet_NetStream_t,
                               struct Net_UserData> HTTPGet_TCPConnector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<HTTPGet_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   HTTPGet_ConnectionConfiguration_t,
                                   struct Net_StreamConnectionState,
                                   Net_StreamStatistic_t,
                                   HTTPGet_NetStream_t,
                                   struct Net_UserData> HTTPGet_SSLTCPConnector_t;
#endif // SSL_SUPPORT
typedef Net_Client_AsynchConnector_T<HTTPGet_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     HTTPGet_ConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Net_TCPSocketConfiguration_t,
                                     HTTPGet_NetStream_t,
                                     struct Net_UserData> HTTPGet_TCPAsynchConnector_t;

#endif
