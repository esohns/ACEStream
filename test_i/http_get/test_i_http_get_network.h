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

#ifndef TEST_I_HTTP_GET_NETWORK_H
#define TEST_I_HTTP_GET_NETWORK_H

#include "ace/INET_Addr.h"
#include "ace/Synch_Traits.h"
#include "ace/SSL/SSL_SOCK_Stream.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_module_io_stream.h"
#include "stream_session_data.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_configuration.h"
#include "net_common.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#include "net_client_ssl_connector.h"

#include "test_i_connection_common.h"

// forward declarations
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_I_AllocatorConfiguration> Test_I_ControlMessage_t;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;
typedef Stream_Statistic Test_I_Statistic_t;
struct Test_I_Stream_SessionData;
typedef Stream_SessionData_T<struct Test_I_Stream_SessionData> Test_I_Stream_SessionData_t;
struct Test_I_HTTPGet_ConnectionConfiguration;
struct Test_I_HTTPGet_ConnectionState;
struct Test_I_HTTPGet_UserData;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 struct Test_I_HTTPGet_ConnectionConfiguration,
                                 struct Test_I_HTTPGet_ConnectionState,
                                 Test_I_Statistic_t,
                                 struct Test_I_HTTPGet_UserData> Test_I_Stream_InetConnectionManager_t;

//////////////////////////////////////////

struct Test_I_HTTPGet_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  Test_I_HTTPGet_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , socketConfiguration_2 ()
   , connectionConfiguration (NULL)
   , userData (NULL)
  {
    socketConfiguration = &socketConfiguration_2;
  };

  struct Net_TCPSocketConfiguration              socketConfiguration_2;
  struct Test_I_HTTPGet_ConnectionConfiguration* connectionConfiguration;

  struct Test_I_HTTPGet_UserData*                userData;
};

//extern const char stream_name_string_[];
struct Test_I_StreamConfiguration;
struct Test_I_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_AllocatorConfiguration,
                               struct Test_I_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_I_ModuleHandlerConfiguration> Test_I_StreamConfiguration_t;
struct Test_I_HTTPGet_ConnectionConfiguration
 : Net_ConnectionConfiguration
{
  Test_I_HTTPGet_ConnectionConfiguration ()
   : Net_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration ()
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_HTTPGet_SocketHandlerConfiguration socketHandlerConfiguration;
  Test_I_StreamConfiguration_t*                    streamConfiguration;

  struct Test_I_HTTPGet_UserData*                  userData;
};
typedef std::map<std::string,
                 struct Test_I_HTTPGet_ConnectionConfiguration> Test_I_HTTPGet_ConnectionConfigurations_t;
typedef Test_I_HTTPGet_ConnectionConfigurations_t::iterator Test_I_HTTPGet_ConnectionConfigurationIterator_t;

struct Test_I_HTTPGet_ConnectionState
 : Net_ConnectionState
{
  Test_I_HTTPGet_ConnectionState ()
   : Net_ConnectionState ()
   , configuration (NULL)
   , currentStatistic ()
   , userData (NULL)
  {};

  struct Test_I_HTTPGet_ConnectionConfiguration* configuration;
  Test_I_Statistic_t                      currentStatistic;

  struct Test_I_HTTPGet_UserData*                userData;
};

//////////////////////////////////////////

extern const char stream_name_string_[];

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_HTTPGet_StreamState,
                                      struct Test_I_StreamConfiguration,
                                      Test_I_Statistic_t,
                                      Common_Timer_Manager_t,
                                      struct Test_I_AllocatorConfiguration,
                                      struct Stream_ModuleConfiguration,
                                      struct Test_I_ModuleHandlerConfiguration,
                                      struct Test_I_Stream_SessionData,
                                      Test_I_Stream_SessionData_t,
                                      Test_I_ControlMessage_t,
                                      Test_I_Stream_Message,
                                      Test_I_Stream_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Stream_InetConnectionManager_t,
                                      struct Test_I_HTTPGet_UserData> Test_I_NetStream_t;
//typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
//                                      Common_TimePolicy_t,
//                                      stream_name_string_,
//                                      enum Stream_ControlType,
//                                      enum Stream_SessionMessageType,
//                                      enum Stream_StateMachine_ControlState,
//                                      struct Test_I_HTTPGet_StreamState,
//                                      struct Test_I_StreamConfiguration,
//                                      Test_I_Statistic_t,
//                                      Test_I_StatisticHandlerProactor_t,
//                                      struct Test_I_AllocatorConfiguration,
//                                      struct Stream_ModuleConfiguration,
//                                      struct Test_I_ModuleHandlerConfiguration,
//                                      struct Test_I_Stream_SessionData,
//                                      Test_I_Stream_SessionData_t,
//                                      Test_I_ControlMessage_t,
//                                      Test_I_Stream_Message,
//                                      Test_I_Stream_SessionMessage,
//                                      ACE_INET_Addr,
//                                      Test_I_Stream_InetConnectionManager_t,
//                                      struct Test_I_HTTPGet_UserData> Test_I_AsynchNetStream_t;

//////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_HTTPGet_ConnectionConfiguration,
                          struct Test_I_HTTPGet_ConnectionState,
                          Test_I_Statistic_t> Test_I_IConnection_t;

//////////////////////////////////////////

// outbound
typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SOCK_STREAM,
                               struct Test_I_HTTPGet_SocketHandlerConfiguration> Test_I_TCPSocketHandler_t;
typedef Net_TCPSocketHandler_T<ACE_MT_SYNCH,
                               ACE_SSL_SOCK_Stream,
                               struct Test_I_HTTPGet_SocketHandlerConfiguration> Test_I_SSLTCPSocketHandler_t;
typedef Net_AsynchTCPSocketHandler_T<struct Test_I_HTTPGet_SocketHandlerConfiguration> Test_I_AsynchTCPSocketHandler_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Test_I_TCPSocketHandler_t,
                                struct Test_I_HTTPGet_ConnectionConfiguration,
                                struct Test_I_HTTPGet_ConnectionState,
                                Test_I_Statistic_t,
                                struct Test_I_HTTPGet_SocketHandlerConfiguration,
                                struct Test_I_ListenerConfiguration,
                                Test_I_NetStream_t,
                                Common_Timer_Manager_t,
                                struct Test_I_HTTPGet_UserData> Test_I_TCPConnection_t;
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Test_I_SSLTCPSocketHandler_t,
                                struct Test_I_HTTPGet_ConnectionConfiguration,
                                struct Test_I_HTTPGet_ConnectionState,
                                Test_I_Statistic_t,
                                struct Test_I_HTTPGet_SocketHandlerConfiguration,
                                struct Test_I_ListenerConfiguration,
                                Test_I_NetStream_t,
                                Common_Timer_Manager_t,
                                struct Test_I_HTTPGet_UserData> Test_I_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_AsynchTCPSocketHandler_t,
                                      struct Test_I_HTTPGet_ConnectionConfiguration,
                                      struct Test_I_HTTPGet_ConnectionState,
                                      Test_I_Statistic_t,
                                      struct Test_I_HTTPGet_SocketHandlerConfiguration,
                                      struct Test_I_ListenerConfiguration,
                                      Test_I_NetStream_t,
                                      Common_Timer_Manager_t,
                                      struct Test_I_HTTPGet_UserData> Test_I_AsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_HTTPGet_ConnectionConfiguration> Test_I_Stream_IInetConnector_t;

/////////////////////////////////////////

// outbound
typedef Net_Client_AsynchConnector_T<Test_I_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_HTTPGet_ConnectionConfiguration,
                                     struct Test_I_HTTPGet_ConnectionState,
                                     Test_I_Statistic_t,
                                     struct Net_TCPSocketConfiguration,
                                     struct Test_I_HTTPGet_SocketHandlerConfiguration,
                                     Test_I_NetStream_t,
                                     struct Test_I_HTTPGet_UserData> Test_I_Stream_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_I_HTTPGet_ConnectionConfiguration,
                               struct Test_I_HTTPGet_ConnectionState,
                               Test_I_Statistic_t,
                               struct Net_TCPSocketConfiguration,
                               struct Test_I_HTTPGet_SocketHandlerConfiguration,
                               Test_I_NetStream_t,
                               struct Test_I_HTTPGet_UserData> Test_I_Stream_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<Test_I_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   struct Test_I_HTTPGet_ConnectionConfiguration,
                                   struct Test_I_HTTPGet_ConnectionState,
                                   Test_I_Statistic_t,
                                   struct Test_I_HTTPGet_SocketHandlerConfiguration,
                                   Test_I_NetStream_t,
                                   struct Test_I_HTTPGet_UserData> Test_I_Stream_SSLTCPConnector_t;

#endif
