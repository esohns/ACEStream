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

#ifndef TEST_I_CONNECTION_COMMON_H
#define TEST_I_CONNECTION_COMMON_H

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "stream_module_io_stream.h"
#include "stream_session_data.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_udpconnection_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

// forward declarations
struct Test_I_Source_Configuration;
struct Test_I_Source_UserData;
class Test_I_Stream_Message;
typedef Stream_Statistic Test_I_RuntimeStatistic_t;
struct Test_I_Source_ConnectionState;
struct Test_I_Source_SocketHandlerConfiguration;
struct Test_I_Source_Stream_SessionData;
typedef Stream_SessionData_T<Test_I_Source_Stream_SessionData> Test_I_Source_Stream_SessionData_t;
class Test_I_Source_Stream_SessionMessage;
struct Test_I_Source_Stream_ModuleHandlerConfiguration;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_StreamState;
struct Test_I_Target_Configuration;
struct Test_I_Target_ConnectionState;
struct Test_I_Target_SocketHandlerConfiguration;
struct Test_I_Target_Stream_ModuleHandlerConfiguration;
struct Test_I_Target_Stream_SessionData;
typedef Stream_SessionData_T<Test_I_Target_Stream_SessionData> Test_I_Target_Stream_SessionData_t;
class Test_I_Target_Stream_SessionMessage;
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_StreamState;
struct Test_I_Target_UserData;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Source_Configuration,
                                 Test_I_Source_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_Source_UserData> Test_I_Source_InetConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Target_Configuration,
                                 Test_I_Target_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_Target_UserData> Test_I_Target_InetConnectionManager_t;

/////////////////////////////////////////

typedef Stream_Module_Net_IO_Stream_T<ACE_SYNCH_MUTEX,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ///
                                      Stream_StateMachine_ControlState,
                                      Test_I_Source_StreamState,
                                      ///
                                      Test_I_Source_StreamConfiguration,
                                      ///
                                      Test_I_RuntimeStatistic_t,
                                      ///
                                      Stream_ModuleConfiguration,
                                      Test_I_Source_Stream_ModuleHandlerConfiguration,
                                      ///
                                      Test_I_Source_Stream_SessionData,   // session data
                                      Test_I_Source_Stream_SessionData_t, // session data container (reference counted)
                                      Test_I_Source_Stream_SessionMessage,
                                      Test_I_Stream_Message,
                                      ///
                                      ACE_INET_Addr,
                                      Test_I_Source_InetConnectionManager_t> Test_I_Source_NetStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_SYNCH_MUTEX,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ///
                                      Stream_StateMachine_ControlState,
                                      Test_I_Target_StreamState,
                                      ///
                                      Test_I_Target_StreamConfiguration,
                                      ///
                                      Test_I_RuntimeStatistic_t,
                                      ///
                                      Stream_ModuleConfiguration,
                                      Test_I_Target_Stream_ModuleHandlerConfiguration,
                                      ///
                                      Test_I_Target_Stream_SessionData,   // session data
                                      Test_I_Target_Stream_SessionData_t, // session data container (reference counted)
                                      Test_I_Target_Stream_SessionMessage,
                                      Test_I_Stream_Message,
                                      ///
                                      ACE_INET_Addr,
                                      Test_I_Target_InetConnectionManager_t> Test_I_Target_NetStream_t;

/////////////////////////////////////////

struct Test_I_Source_ConnectionState
{
  inline Test_I_Source_ConnectionState ()
   : configuration (NULL)
   , status (NET_CONNECTION_STATUS_INVALID)
   , currentStatistic ()
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Test_I_Source_Configuration* configuration;

  Net_Connection_Status        status;

  Test_I_RuntimeStatistic_t    currentStatistic;

  Test_I_Source_UserData*      userData;
};
struct Test_I_Target_ConnectionState
{
  inline Test_I_Target_ConnectionState ()
   : configuration (NULL)
   , status (NET_CONNECTION_STATUS_INVALID)
   , currentStatistic ()
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Test_I_Target_Configuration* configuration;

  Net_Connection_Status        status;

  Test_I_RuntimeStatistic_t    currentStatistic;

  Test_I_Target_UserData*      userData;
};

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Source_Configuration,
                          Test_I_Source_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_IConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Target_Configuration,
                          Test_I_Target_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Target_IConnection_t;

/////////////////////////////////////////

// outbound
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Source_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Source_Configuration,
                                  Test_I_Source_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_NetStream_t,
                                  ///////
                                  Test_I_Source_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_Stream_ModuleHandlerConfiguration> Test_I_Source_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Source_SocketHandlerConfiguration>,

                                        ACE_INET_Addr,
                                        Test_I_Source_Configuration,
                                        Test_I_Source_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_NetStream_t,

                                        Test_I_Source_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Source_Stream_ModuleHandlerConfiguration> Test_I_Source_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         Test_I_Source_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Source_Configuration,
                                  Test_I_Source_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_NetStream_t,
                                  ///////
                                  Test_I_Source_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_Stream_ModuleHandlerConfiguration,
                                  ///////
                                  Test_I_Source_SocketHandlerConfiguration> Test_I_Source_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_I_Source_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,

                                        ACE_INET_Addr,
                                        Test_I_Source_Configuration,
                                        Test_I_Source_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_NetStream_t,

                                        Test_I_Source_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Source_Stream_ModuleHandlerConfiguration,

                                        Test_I_Source_SocketHandlerConfiguration> Test_I_Source_AsynchUDPHandler_t;

typedef Net_TCPConnectionBase_T<Test_I_Source_TCPHandler_t,
                                /////////
                                Test_I_Source_Configuration,
                                Test_I_Source_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_NetStream_t,
                                /////////
                                Test_I_Source_SocketHandlerConfiguration,
                                /////////
                                Test_I_Source_UserData> Test_I_Source_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Source_AsynchTCPHandler_t,
                                      ///
                                      Test_I_Source_Configuration,
                                      Test_I_Source_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_NetStream_t,
                                      ///
                                      Test_I_Source_SocketHandlerConfiguration,
                                      ///
                                      Test_I_Source_UserData> Test_I_Source_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Source_UDPHandler_t,
                                /////////
                                Test_I_Source_Configuration,
                                Test_I_Source_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_NetStream_t,
                                /////////
                                Test_I_Source_SocketHandlerConfiguration,
                                /////////
                                Test_I_Source_UserData> Test_I_Source_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Source_AsynchUDPHandler_t,
                                      ///
                                      Test_I_Source_Configuration,
                                      Test_I_Source_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_NetStream_t,
                                      ///
                                      Test_I_Source_SocketHandlerConfiguration,
                                      ///
                                      Test_I_Source_UserData> Test_I_Source_AsynchUDPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Source_SocketHandlerConfiguration> Test_I_Source_IInetConnector_t;

/////////////////////////////////////////

// outbound
typedef Net_Client_AsynchConnector_T<Test_I_Source_AsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Source_Configuration,
                                     Test_I_Source_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_NetStream_t,
                                     ////
                                     Test_I_Source_SocketHandlerConfiguration,
                                     ////
                                     Test_I_Source_UserData> Test_I_Source_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_TCPConnection_t,
                               Net_SOCK_Connector,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Source_Configuration,
                               Test_I_Source_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_NetStream_t,
                               //////////
                               Test_I_Source_SocketHandlerConfiguration,
                               //////////
                               Test_I_Source_UserData> Test_I_Source_TCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Source_AsynchUDPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Source_Configuration,
                                     Test_I_Source_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_NetStream_t,
                                     ////
                                     Test_I_Source_SocketHandlerConfiguration,
                                     ////
                                     Test_I_Source_UserData> Test_I_Source_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Source_Configuration,
                               Test_I_Source_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_NetStream_t,
                               //////////
                               Test_I_Source_SocketHandlerConfiguration,
                               //////////
                               Test_I_Source_UserData> Test_I_Source_UDPConnector_t;

/////////////////////////////////////////

// inbound
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Target_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Target_Configuration,
                                  Test_I_Target_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Target_NetStream_t,
                                  ///////
                                  Test_I_Target_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Target_Stream_ModuleHandlerConfiguration> Test_I_Target_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Target_SocketHandlerConfiguration>,

                                        ACE_INET_Addr,
                                        Test_I_Target_Configuration,
                                        Test_I_Target_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Target_NetStream_t,

                                        Test_I_Target_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Target_Stream_ModuleHandlerConfiguration> Test_I_Target_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         Test_I_Target_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Target_Configuration,
                                  Test_I_Target_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Target_NetStream_t,
                                  ///////
                                  Test_I_Target_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Target_Stream_ModuleHandlerConfiguration,
                                  ///////
                                  Test_I_Target_SocketHandlerConfiguration> Test_I_Target_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_I_Target_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,

                                        ACE_INET_Addr,
                                        Test_I_Target_Configuration,
                                        Test_I_Target_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Target_NetStream_t,

                                        Test_I_Target_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Target_Stream_ModuleHandlerConfiguration,

                                        Test_I_Target_SocketHandlerConfiguration> Test_I_Target_AsynchUDPHandler_t;

typedef Net_TCPConnectionBase_T<Test_I_Target_TCPHandler_t,
                                /////////
                                Test_I_Target_Configuration,
                                Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Target_NetStream_t,
                                /////////
                                Test_I_Target_SocketHandlerConfiguration,
                                /////////
                                Test_I_Target_UserData> Test_I_Target_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Target_AsynchTCPHandler_t,
                                      ///
                                      Test_I_Target_Configuration,
                                      Test_I_Target_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Target_NetStream_t,
                                      ///
                                      Test_I_Target_SocketHandlerConfiguration,
                                      ///
                                      Test_I_Target_UserData> Test_I_Target_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Target_UDPHandler_t,
                                /////////
                                Test_I_Target_Configuration,
                                Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Target_NetStream_t,
                                /////////
                                Test_I_Target_SocketHandlerConfiguration,
                                /////////
                                Test_I_Target_UserData> Test_I_Target_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Target_AsynchUDPHandler_t,
                                      ///
                                      Test_I_Target_Configuration,
                                      Test_I_Target_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Target_NetStream_t,
                                      ///
                                      Test_I_Target_SocketHandlerConfiguration,
                                      ///
                                      Test_I_Target_UserData> Test_I_Target_AsynchUDPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IInetConnector_t;

/////////////////////////////////////////

// inbound
typedef Net_Client_AsynchConnector_T<Test_I_Target_AsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Target_Configuration,
                                     Test_I_Target_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Target_NetStream_t,
                                     ////
                                     Test_I_Target_SocketHandlerConfiguration,
                                     ////
                                     Test_I_Target_UserData> Test_I_Target_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Target_TCPConnection_t,
                               Net_SOCK_Connector,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Target_Configuration,
                               Test_I_Target_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Target_NetStream_t,
                               //////////
                               Test_I_Target_SocketHandlerConfiguration,
                               //////////
                               Test_I_Target_UserData> Test_I_Target_TCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Target_AsynchUDPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Target_Configuration,
                                     Test_I_Target_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Target_NetStream_t,
                                     ////
                                     Test_I_Target_SocketHandlerConfiguration,
                                     ////
                                     Test_I_Target_UserData> Test_I_Target_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Target_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Target_Configuration,
                               Test_I_Target_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Target_NetStream_t,
                               //////////
                               Test_I_Target_SocketHandlerConfiguration,
                               //////////
                               Test_I_Target_UserData> Test_I_Target_UDPConnector_t;

#endif