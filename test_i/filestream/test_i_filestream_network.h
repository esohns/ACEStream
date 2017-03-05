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

#ifndef TEST_I_FILESTREAM_NETWORK_H
#define TEST_I_FILESTREAM_NETWORK_H

#include <ace/INET_Addr.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <ace/Netlink_Addr.h>
#endif
#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_session_data.h"

#include "stream_module_io_stream.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_configuration.h"
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

#include "test_i_connection_common.h"

#include "test_i_message.h"
//#include "test_i_source_common.h"

// forward declarations
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Target_ConnectionConfiguration;
//struct Test_I_Source_ConnectionState;
struct Test_I_Target_ConnectionState;
typedef Stream_Statistic Test_I_RuntimeStatistic_t;
//struct Test_I_Source_Stream_Configuration;
struct Test_I_StreamConfiguration;
//struct Test_I_Source_ModuleHandlerConfiguration;
struct Test_I_ModuleHandlerConfiguration;
//struct Test_I_Source_SocketHandlerConfiguration;
struct Test_I_Target_SocketHandlerConfiguration;
//struct Test_I_Source_SessionData;
//typedef Stream_SessionData_T<Test_I_Source_SessionData> Test_I_Source_SessionData_t;
struct Test_I_StreamState;
struct Test_I_Source_SessionData;
struct Test_I_Source_UserData;
struct Test_I_Target_UserData;
struct Test_I_UserData;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Test_I_Source_ConnectionConfiguration,
                                 struct Test_I_Source_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Source_UserData> Test_I_Source_IInetConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Source_ConnectionConfiguration,
                                 struct Test_I_Source_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Source_UserData> Test_I_Source_InetConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 struct Test_I_Target_ConnectionConfiguration,
                                 struct Test_I_Target_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Target_UserData> Test_I_Target_IInetConnectionManager_t;
struct Test_I_Source_SocketHandlerConfiguration;
struct Test_I_SocketHandlerConfiguration;

//////////////////////////////////////////

typedef Stream_SessionData_T<struct Test_I_Source_SessionData> Test_I_Source_SessionData_t;
class Test_I_Source_SessionMessage;
typedef Test_I_Message_T<enum Stream_MessageType,
                         Test_I_Source_SessionMessage> Test_I_Source_Message_t;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_I_Source_ControlMessage_t;

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_Source_StreamState,
                                      struct Test_I_Source_StreamConfiguration,
                                      Test_I_RuntimeStatistic_t,
                                      struct Stream_ModuleConfiguration,
                                      struct Test_I_Source_ModuleHandlerConfiguration,
                                      struct Test_I_Source_SessionData,
                                      Test_I_Source_SessionData_t,
                                      Test_I_Source_ControlMessage_t,
                                      Test_I_Source_Message_t,
                                      Test_I_Source_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Source_InetConnectionManager_t,
                                      struct Test_I_Source_UserData> Test_I_Source_NetStream_t;
//typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
//                                      ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      enum Stream_ControlType,
//                                      enum Stream_SessionMessageType,
//                                      enum Stream_StateMachine_ControlState,
//                                      struct Test_I_Target_StreamState,
//                                      struct Test_I_StreamConfiguration,
//                                      Test_I_RuntimeStatistic_t,
//                                      struct Stream_ModuleConfiguration,
//                                      struct Test_I_Target_ModuleHandlerConfiguration,
//                                      struct Test_I_Target_SessionData,
//                                      Test_I_Target_SessionData_t,
//                                      Test_I_Target_ControlMessage_t,
//                                      Test_I_Target_Message_t,
//                                      Test_I_Target_SessionMessage,
//                                      ACE_INET_Addr,
//                                      Test_I_Target_InetConnectionManager_t> Test_I_Target_NetStream_t;

//////////////////////////////////////////

struct Test_I_Source_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_ConnectionConfiguration* connectionConfiguration;

  struct Test_I_Source_UserData*                userData;
};

struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_UserData;
struct Test_I_Source_ConnectionConfiguration
 : Test_I_ConnectionConfiguration
{
  inline Test_I_Source_ConnectionConfiguration ()
   : Test_I_ConnectionConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  Test_I_Source_IInetConnectionManager_t*          connectionManager; // TCP IO module
  struct Test_I_Source_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Test_I_Source_StreamConfiguration*        streamConfiguration;

  struct Test_I_Source_UserData*                   userData;
};

struct Test_I_Source_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Source_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_ConnectionConfiguration* configuration;

  struct Test_I_Source_UserData*                userData;
};

struct Test_I_Target_UserData;
struct Test_I_Target_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Target_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , connectionConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_ConnectionConfiguration* connectionConfiguration;

  struct Test_I_Target_UserData*                userData;
};

struct Test_I_Target_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  inline Test_I_Target_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  Test_I_Target_IInetConnectionManager_t*          connectionManager; // TCP IO module
  struct Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Test_I_Target_StreamConfiguration*        streamConfiguration;

  struct Test_I_TargetUserData*                    userData;
};

struct Test_I_Target_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Target_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_ConnectionConfiguration* configuration;

  struct Test_I_Target_UserData*                userData;
};

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_Source_ConnectionConfiguration,
                          struct Test_I_Source_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_IConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_Target_ConnectionConfiguration,
                          struct Test_I_Target_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Target_IConnection_t;

//////////////////////////////////////////

// outbound
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_I_Source_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct Test_I_Source_ConnectionConfiguration,
                                  struct Test_I_Source_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_NetStream_t,
                                  struct Test_I_Source_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Source_ModuleHandlerConfiguration> Test_I_Source_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Test_I_Source_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct Test_I_Source_ConnectionConfiguration,
                                        struct Test_I_Source_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_NetStream_t,
                                        struct Test_I_Source_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Source_ModuleHandlerConfiguration> Test_I_Source_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         struct Test_I_Source_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_I_Source_ConnectionConfiguration,
                                  struct Test_I_Source_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  struct Test_I_Source_SocketHandlerConfiguration,
                                  Test_I_Source_NetStream_t,
                                  struct Test_I_Source_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Source_ModuleHandlerConfiguration> Test_I_Source_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_I_Source_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        struct Test_I_Source_ConnectionConfiguration,
                                        struct Test_I_Source_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        struct Test_I_Source_SocketHandlerConfiguration,
                                        Test_I_Source_NetStream_t,
                                        struct Test_I_Source_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Source_ModuleHandlerConfiguration> Test_I_Source_AsynchUDPHandler_t;

typedef Net_TCPConnectionBase_T<Test_I_Source_TCPHandler_t,
                                struct Test_I_Source_ConnectionConfiguration,
                                struct Test_I_Source_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Source_SocketHandlerConfiguration,
                                Test_I_Source_NetStream_t,
                                struct Test_I_Source_UserData> Test_I_Source_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Source_AsynchTCPHandler_t,
                                      struct Test_I_Source_ConnectionConfiguration,
                                      struct Test_I_Source_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Source_SocketHandlerConfiguration,
                                      Test_I_Source_NetStream_t,
                                      struct Test_I_Source_UserData> Test_I_Source_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Source_UDPHandler_t,
                                struct Test_I_Source_ConnectionConfiguration,
                                struct Test_I_Source_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Source_SocketHandlerConfiguration,
                                Test_I_Source_NetStream_t,
                                struct Test_I_Source_UserData> Test_I_Source_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Source_AsynchUDPHandler_t,
                                      struct Test_I_Source_ConnectionConfiguration,
                                      struct Test_I_Source_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Source_SocketHandlerConfiguration,
                                      Test_I_Source_NetStream_t,
                                      struct Test_I_Source_UserData> Test_I_Source_AsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_Source_SocketHandlerConfiguration> Test_I_Source_IInetConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IInetConnector_t;

/////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<Test_I_Source_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_Source_ConnectionConfiguration,
                                     struct Test_I_Source_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     struct Test_I_Source_SocketHandlerConfiguration,
                                     Test_I_Source_NetStream_t,
                                     struct Test_I_Source_UserData> Test_I_Source_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_I_Source_ConnectionConfiguration,
                               struct Test_I_Source_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               struct Test_I_Source_SocketHandlerConfiguration,
                               Test_I_Source_NetStream_t,
                               struct Test_I_Source_UserData> Test_I_Source_TCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Source_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_Source_ConnectionConfiguration,
                                     struct Test_I_Source_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     struct Test_I_Source_SocketHandlerConfiguration,
                                     Test_I_Source_NetStream_t,
                                     struct Test_I_Source_UserData> Test_I_Source_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct Test_I_Source_ConnectionConfiguration,
                               struct Test_I_Source_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               struct Test_I_Source_SocketHandlerConfiguration,
                               Test_I_Source_NetStream_t,
                               struct Test_I_Source_UserData> Test_I_Source_UDPConnector_t;

#endif
