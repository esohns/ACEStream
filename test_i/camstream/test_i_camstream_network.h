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

#ifndef TEST_I_CAMSTREAM_NETWORK_H
#define TEST_I_CAMSTREAM_NETWORK_H

#include <ace/config-lite.h>
#include <ace/INET_Addr.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <ace/Netlink_Addr.h>
#endif
#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>
#include <ace/SSL/SSL_SOCK_Stream.h>

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
#include "net_client_ssl_connector.h"

#include "test_i_connection_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_DirectShow_ConnectionState;
struct Test_I_Source_DirectShow_ModuleHandlerConfiguration;
struct Test_I_Source_DirectShow_SessionData;
struct Test_I_Source_DirectShow_SocketHandlerConfiguration;
struct Test_I_Source_DirectShow_StreamConfiguration;
class Test_I_Source_DirectShow_Stream_Message;
class Test_I_Source_DirectShow_Stream_SessionMessage;
struct Test_I_Source_DirectShow_StreamState;
struct Test_I_Source_DirectShow_UserData;

struct Test_I_Target_ConnectionConfiguration;
struct Test_I_Target_DirectShow_ConnectionState;
struct Test_I_Target_DirectShow_ModuleHandlerConfiguration;
struct Test_I_Target_DirectShow_SessionData;
struct Test_I_Target_DirectShow_SocketHandlerConfiguration;
class Test_I_Target_DirectShow_Stream;
class Test_I_Target_DirectShow_Stream_Message;
class Test_I_Target_DirectShow_Stream_SessionMessage;
struct Test_I_Target_DirectShow_StreamConfiguration;
struct Test_I_Target_DirectShow_StreamState;
struct Test_I_Target_DirectShow_UserData;

typedef Stream_SessionData_T<struct Test_I_Source_DirectShow_SessionData> Test_I_Source_DirectShow_SessionData_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Source_ConnectionConfiguration,
                                 struct Test_I_Source_DirectShow_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_InetConnectionManager_t;

typedef Stream_SessionData_T<Test_I_Target_DirectShow_SessionData> Test_I_Target_DirectShow_SessionData_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Target_ConnectionConfiguration,
                                 struct Test_I_Target_DirectShow_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_InetConnectionManager_t;

struct Test_I_Source_MediaFoundation_Configuration;
struct Test_I_Source_MediaFoundation_ConnectionState;
struct Test_I_Source_MediaFoundation_ModuleHandlerConfiguration;
struct Test_I_Source_MediaFoundation_SessionData;
struct Test_I_Source_MediaFoundation_SocketHandlerConfiguration;
class Test_I_Source_MediaFoundation_Stream_Message;
class Test_I_Source_MediaFoundation_Stream_SessionMessage;
struct Test_I_Source_MediaFoundation_StreamConfiguration;
struct Test_I_Source_MediaFoundation_StreamState;
struct Test_I_Source_MediaFoundation_UserData;

struct Test_I_Target_MediaFoundation_Configuration;
struct Test_I_Target_MediaFoundation_ConnectionState;
struct Test_I_Target_MediaFoundation_ModuleHandlerConfiguration;
struct Test_I_Target_MediaFoundation_SessionData;
struct Test_I_Target_MediaFoundation_SocketHandlerConfiguration;
class Test_I_Target_MediaFoundation_Stream;
class Test_I_Target_MediaFoundation_Stream_Message;
class Test_I_Target_MediaFoundation_Stream_SessionMessage;
struct Test_I_Target_MediaFoundation_StreamConfiguration;
struct Test_I_Target_MediaFoundation_StreamState;
struct Test_I_Target_MediaFoundation_UserData;

typedef Stream_SessionData_T<struct Test_I_Source_MediaFoundation_SessionData> Test_I_Source_MediaFoundation_SessionData_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Source_ConnectionConfiguration,
                                 struct Test_I_Source_MediaFoundation_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_InetConnectionManager_t;

typedef Stream_SessionData_T<struct Test_I_Target_MediaFoundation_SessionData> Test_I_Target_MediaFoundation_SessionData_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Target_ConnectionConfiguration,
                                 struct Test_I_Target_MediaFoundation_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_InetConnectionManager_t;

#else
struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_V4L2_ConnectionState;
struct Test_I_Source_V4L2_ModuleHandlerConfiguration;
struct Test_I_Source_V4L2_SessionData;
struct Test_I_Source_V4L2_SocketHandlerConfiguration;
class Test_I_Source_V4L2_Stream_Message;
class Test_I_Source_V4L2_Stream_SessionMessage;
struct Test_I_Source_V4L2_StreamConfiguration;
struct Test_I_Source_V4L2_StreamState;
struct Test_I_Source_V4L2_UserData;

typedef Stream_SessionData_T<struct Test_I_Source_V4L2_SessionData> Test_I_Source_V4L2_SessionData_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Source_ConnectionConfiguration,
                                 struct Test_I_Source_V4L2_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_InetConnectionManager_t;
#endif

struct Test_I_Target_ConnectionConfiguration;
struct Test_I_Target_ConnectionState;
struct Test_I_Target_ModuleHandlerConfiguration;
struct Test_I_Target_SessionData;
struct Test_I_Target_SocketHandlerConfiguration;
class Test_I_Target_Stream;
class Test_I_Target_Stream_Message;
class Test_I_Target_Stream_SessionMessage;
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_StreamState;
struct Test_I_Target_UserData;

typedef Stream_SessionData_T<struct Test_I_Target_SessionData> Test_I_Target_SessionData_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 struct Test_I_Target_ConnectionConfiguration,
                                 struct Test_I_Target_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 struct Test_I_Target_UserData> Test_I_Target_InetConnectionManager_t;

typedef Stream_Statistic Test_I_RuntimeStatistic_t;

struct Test_I_Source_V4L2_SocketHandlerConfiguration;
struct Test_I_Source_V4L2_StreamConfiguration;
struct Test_I_Source_UserData;
struct Test_I_Source_ConnectionConfiguration
 : Test_I_ConnectionConfiguration
{
  inline Test_I_Source_ConnectionConfiguration ()
   : Test_I_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_V4L2_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Test_I_Source_V4L2_StreamConfiguration*        streamConfiguration;

  struct Test_I_Source_UserData*                        userData;
};

struct Test_I_Target_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Target_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  struct Test_I_Target_UserData* userData;
};

struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_ConnectionConfiguration
 : Test_I_ConnectionConfiguration
{
  inline Test_I_Target_ConnectionConfiguration ()
   : Test_I_ConnectionConfiguration ()
   ///////////////////////////////////////
   , socketHandlerConfiguration (NULL)
   , streamConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
  struct Test_I_Target_StreamConfiguration*        streamConfiguration;

  struct Test_I_Target_UserData*                   userData;
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Stream_ControlType,
                                      Stream_SessionMessageType,
                                      Stream_StateMachine_ControlState,
                                      Test_I_Source_DirectShow_StreamState,
                                      Test_I_Source_DirectShow_StreamConfiguration,
                                      Test_I_RuntimeStatistic_t,
                                      Stream_ModuleConfiguration,
                                      Test_I_Source_DirectShow_ModuleHandlerConfiguration,
                                      Test_I_Source_DirectShow_SessionData,
                                      Test_I_Source_DirectShow_SessionData_t,
                                      ACE_Message_Block,
                                      Test_I_Source_DirectShow_Stream_Message,
                                      Test_I_Source_DirectShow_Stream_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Source_DirectShow_InetConnectionManager_t> Test_I_Source_DirectShow_NetStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      Stream_ControlType,
                                      Stream_SessionMessageType,
                                      Stream_StateMachine_ControlState,
                                      Test_I_Source_MediaFoundation_StreamState,
                                      Test_I_Source_MediaFoundation_StreamConfiguration,
                                      Test_I_RuntimeStatistic_t,
                                      Stream_ModuleConfiguration,
                                      Test_I_Source_MediaFoundation_ModuleHandlerConfiguration,
                                      Test_I_Source_MediaFoundation_SessionData,
                                      Test_I_Source_MediaFoundation_SessionData_t,
                                      ACE_Message_Block,
                                      Test_I_Source_MediaFoundation_Stream_Message,
                                      Test_I_Source_MediaFoundation_Stream_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Source_MediaFoundation_InetConnectionManager_t> Test_I_Source_MediaFoundation_NetStream_t;
#else
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_Source_V4L2_StreamState,
                                      struct Test_I_Source_V4L2_StreamConfiguration,
                                      Test_I_RuntimeStatistic_t,
                                      struct Stream_ModuleConfiguration,
                                      struct Test_I_Source_V4L2_ModuleHandlerConfiguration,
                                      struct Test_I_Source_V4L2_SessionData,
                                      Test_I_Source_V4L2_SessionData_t,
                                      ACE_Message_Block,
                                      Test_I_Source_V4L2_Stream_Message,
                                      Test_I_Source_V4L2_Stream_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Source_V4L2_InetConnectionManager_t> Test_I_Source_V4L2_NetStream_t;
#endif

//////////////////////////////////////////

//struct Test_I_ConnectionState
// : Net_ConnectionState
//{
//  inline Test_I_ConnectionState ()
//   : Net_ConnectionState ()
//   , configuration (NULL)
//   , currentStatistic ()
//   , userData (NULL)
//  {};

//  struct Test_I_ConnectionConfiguration* configuration;

//  Test_I_RuntimeStatistic_t              currentStatistic;

//  struct Test_I_UserData*                userData;
//};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Source_DirectShow_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Source_DirectShow_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  Test_I_Source_ConnectionConfiguration* configuration;
  Test_I_Source_DirectShow_UserData*     userData;
};
struct Test_I_Source_MediaFoundation_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Source_MediaFoundation_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  Test_I_Source_ConnectionConfiguration*  configuration;
  Test_I_Source_MediaFoundation_UserData* userData;
};

struct Test_I_Target_DirectShow_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Target_DirectShow_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Test_I_Target_ConnectionConfiguration* configuration;
  Test_I_Target_DirectShow_UserData*     userData;
};
struct Test_I_Target_MediaFoundation_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Target_MediaFoundation_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Test_I_Target_ConnectionConfiguration*  configuration;
  Test_I_Target_MediaFoundation_UserData* userData;
};
#else
struct Test_I_Source_V4L2_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Source_V4L2_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_ConnectionConfiguration* configuration;
  struct Test_I_Source_V4L2_UserData*           userData;
};
#endif
struct Test_I_Target_ConnectionState
 : Test_I_ConnectionState
{
  inline Test_I_Target_ConnectionState ()
   : Test_I_ConnectionState ()
   , configuration (NULL)
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  struct Test_I_Target_ConnectionConfiguration* configuration;
  struct Test_I_Target_UserData*                userData;
};

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Source_ConnectionConfiguration,
                          Test_I_Source_DirectShow_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_DirectShow_IConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Target_ConnectionConfiguration,
                          Test_I_Target_DirectShow_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Target_DirectShow_IConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Source_ConnectionConfiguration,
                          Test_I_Source_MediaFoundation_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_MediaFoundation_IConnection_t;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Target_ConnectionConfiguration,
                          Test_I_Target_MediaFoundation_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Target_MediaFoundation_IConnection_t;
#else
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_Source_ConnectionConfiguration,
                          struct Test_I_Source_V4L2_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_V4L2_IConnection_t;
#endif
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_Target_ConnectionConfiguration,
                          struct Test_I_Target_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Target_IConnection_t;

//////////////////////////////////////////

// outbound
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  Test_I_Source_ConnectionConfiguration,
                                  Test_I_Source_DirectShow_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_DirectShow_NetStream_t,
                                  Test_I_Source_DirectShow_UserData,
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_DirectShow_ModuleHandlerConfiguration> Test_I_Source_DirectShow_TCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                                         ACE_SSL_SOCK_Stream>,
                                  ACE_INET_Addr,
                                  Test_I_Source_ConnectionConfiguration,
                                  Test_I_Source_DirectShow_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_DirectShow_NetStream_t,
                                  Test_I_Source_DirectShow_UserData,
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_DirectShow_ModuleHandlerConfiguration> Test_I_Source_DirectShow_SSLTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Source_DirectShow_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        Test_I_Source_ConnectionConfiguration,
                                        Test_I_Source_DirectShow_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_DirectShow_NetStream_t,
                                        Test_I_Source_DirectShow_UserData,
                                        Stream_ModuleConfiguration,
                                        Test_I_Source_DirectShow_ModuleHandlerConfiguration> Test_I_Source_DirectShow_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         Test_I_Source_DirectShow_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  Test_I_Source_ConnectionConfiguration,
                                  Test_I_Source_DirectShow_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                  Test_I_Source_DirectShow_NetStream_t,
                                  Test_I_Source_DirectShow_UserData,
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_DirectShow_ModuleHandlerConfiguration> Test_I_Source_DirectShow_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_I_Source_DirectShow_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        Test_I_Source_ConnectionConfiguration,
                                        Test_I_Source_DirectShow_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                        Test_I_Source_DirectShow_NetStream_t,
                                        Test_I_Source_DirectShow_UserData,
                                        Stream_ModuleConfiguration,
                                        Test_I_Source_DirectShow_ModuleHandlerConfiguration> Test_I_Source_DirectShow_AsynchUDPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  Test_I_Source_ConnectionConfiguration,
                                  Test_I_Source_MediaFoundation_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_MediaFoundation_NetStream_t,
                                  Test_I_Source_MediaFoundation_UserData,
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_MediaFoundation_ModuleHandlerConfiguration> Test_I_Source_MediaFoundation_TCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                                         ACE_SSL_SOCK_Stream>,
                                  ACE_INET_Addr,
                                  Test_I_Source_ConnectionConfiguration,
                                  Test_I_Source_MediaFoundation_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_MediaFoundation_NetStream_t,
                                  Test_I_Source_MediaFoundation_UserData,
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_MediaFoundation_ModuleHandlerConfiguration> Test_I_Source_MediaFoundation_SSLTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Source_MediaFoundation_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        Test_I_Source_ConnectionConfiguration,
                                        Test_I_Source_MediaFoundation_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_MediaFoundation_NetStream_t,
                                        Test_I_Source_MediaFoundation_UserData,
                                        Stream_ModuleConfiguration,
                                        Test_I_Source_MediaFoundation_ModuleHandlerConfiguration> Test_I_Source_MediaFoundation_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         Test_I_Source_MediaFoundation_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  Test_I_Source_ConnectionConfiguration,
                                  Test_I_Source_MediaFoundation_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                  Test_I_Source_MediaFoundation_NetStream_t,
                                  Test_I_Source_MediaFoundation_UserData,
                                  Stream_ModuleConfiguration,
                                  Test_I_Source_MediaFoundation_ModuleHandlerConfiguration> Test_I_Source_MediaFoundation_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_I_Source_MediaFoundation_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        Test_I_Source_ConnectionConfiguration,
                                        Test_I_Source_MediaFoundation_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                        Test_I_Source_MediaFoundation_NetStream_t,
                                        Test_I_Source_MediaFoundation_UserData,
                                        Stream_ModuleConfiguration,
                                        Test_I_Source_MediaFoundation_ModuleHandlerConfiguration> Test_I_Source_MediaFoundation_AsynchUDPHandler_t;
#else
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct Test_I_Source_ConnectionConfiguration,
                                  struct Test_I_Source_V4L2_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_V4L2_NetStream_t,
                                  struct Test_I_Source_V4L2_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Source_V4L2_ModuleHandlerConfiguration> Test_I_Source_V4L2_TCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                                         ACE_SSL_SOCK_Stream>,
                                  ACE_INET_Addr,
                                  struct Test_I_Source_ConnectionConfiguration,
                                  struct Test_I_Source_V4L2_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_V4L2_NetStream_t,
                                  struct Test_I_Source_V4L2_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Source_V4L2_ModuleHandlerConfiguration> Test_I_Source_V4L2_SSLTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Test_I_Source_V4L2_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct Test_I_Source_ConnectionConfiguration,
                                        struct Test_I_Source_V4L2_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_V4L2_NetStream_t,
                                        struct Test_I_Source_V4L2_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Source_V4L2_ModuleHandlerConfiguration> Test_I_Source_V4L2_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         struct Test_I_Source_V4L2_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_I_Source_ConnectionConfiguration,
                                  struct Test_I_Source_V4L2_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                  Test_I_Source_V4L2_NetStream_t,
                                  struct Test_I_Source_V4L2_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Source_V4L2_ModuleHandlerConfiguration> Test_I_Source_V4L2_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_I_Source_V4L2_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        struct Test_I_Source_ConnectionConfiguration,
                                        struct Test_I_Source_V4L2_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                        Test_I_Source_V4L2_NetStream_t,
                                        struct Test_I_Source_V4L2_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Source_V4L2_ModuleHandlerConfiguration> Test_I_Source_V4L2_AsynchUDPHandler_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_TCPConnectionBase_T<Test_I_Source_DirectShow_TCPHandler_t,
                                Test_I_Source_ConnectionConfiguration,
                                Test_I_Source_DirectShow_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                Test_I_Source_DirectShow_NetStream_t,
                                Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_TCPConnection_t;
typedef Net_TCPConnectionBase_T<Test_I_Source_DirectShow_SSLTCPHandler_t,
                                Test_I_Source_ConnectionConfiguration,
                                Test_I_Source_DirectShow_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                Test_I_Source_DirectShow_NetStream_t,
                                Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Source_DirectShow_AsynchTCPHandler_t,
                                      Test_I_Source_ConnectionConfiguration,
                                      Test_I_Source_DirectShow_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                      Test_I_Source_DirectShow_NetStream_t,
                                      Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Source_DirectShow_UDPHandler_t,
                                Test_I_Source_ConnectionConfiguration,
                                Test_I_Source_DirectShow_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                Test_I_Source_DirectShow_NetStream_t,
                                Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Source_DirectShow_AsynchUDPHandler_t,
                                      Test_I_Source_ConnectionConfiguration,
                                      Test_I_Source_DirectShow_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                      Test_I_Source_DirectShow_NetStream_t,
                                      Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_AsynchUDPConnection_t;
typedef Net_TCPConnectionBase_T<Test_I_Source_MediaFoundation_TCPHandler_t,
                                Test_I_Source_ConnectionConfiguration,
                                Test_I_Source_MediaFoundation_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                Test_I_Source_MediaFoundation_NetStream_t,
                                Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_TCPConnection_t;
typedef Net_TCPConnectionBase_T<Test_I_Source_MediaFoundation_SSLTCPHandler_t,
                                Test_I_Source_ConnectionConfiguration,
                                Test_I_Source_MediaFoundation_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                Test_I_Source_MediaFoundation_NetStream_t,
                                Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Source_MediaFoundation_AsynchTCPHandler_t,
                                      Test_I_Source_ConnectionConfiguration,
                                      Test_I_Source_MediaFoundation_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                      Test_I_Source_MediaFoundation_NetStream_t,
                                      Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Source_MediaFoundation_UDPHandler_t,
                                Test_I_Source_ConnectionConfiguration,
                                Test_I_Source_MediaFoundation_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                Test_I_Source_MediaFoundation_NetStream_t,
                                Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Source_MediaFoundation_AsynchUDPHandler_t,
                                      Test_I_Source_ConnectionConfiguration,
                                      Test_I_Source_MediaFoundation_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                      Test_I_Source_MediaFoundation_NetStream_t,
                                      Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_AsynchUDPConnection_t;
#else
typedef Net_TCPConnectionBase_T<Test_I_Source_V4L2_TCPHandler_t,
                                struct Test_I_Source_ConnectionConfiguration,
                                struct Test_I_Source_V4L2_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                Test_I_Source_V4L2_NetStream_t,
                                struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_TCPConnection_t;
typedef Net_TCPConnectionBase_T<Test_I_Source_V4L2_SSLTCPHandler_t,
                                struct Test_I_Source_ConnectionConfiguration,
                                struct Test_I_Source_V4L2_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                Test_I_Source_V4L2_NetStream_t,
                                struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Source_V4L2_AsynchTCPHandler_t,
                                      struct Test_I_Source_ConnectionConfiguration,
                                      struct Test_I_Source_V4L2_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                      Test_I_Source_V4L2_NetStream_t,
                                      struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Source_V4L2_UDPHandler_t,
                                struct Test_I_Source_ConnectionConfiguration,
                                struct Test_I_Source_V4L2_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                Test_I_Source_V4L2_NetStream_t,
                                struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Source_V4L2_AsynchUDPHandler_t,
                                      struct Test_I_Source_ConnectionConfiguration,
                                      struct Test_I_Source_V4L2_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                      Test_I_Source_V4L2_NetStream_t,
                                      struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_AsynchUDPConnection_t;
#endif

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Source_DirectShow_SocketHandlerConfiguration> Test_I_Source_DirectShow_IInetConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Source_MediaFoundation_SocketHandlerConfiguration> Test_I_Source_MediaFoundation_IInetConnector_t;
#else
typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_Source_V4L2_SocketHandlerConfiguration> Test_I_Source_V4L2_IInetConnector_t;
#endif

//////////////////////////////////////////

// outbound
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Client_AsynchConnector_T<Test_I_Source_DirectShow_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_ConnectionConfiguration,
                                     Test_I_Source_DirectShow_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                     Test_I_Source_DirectShow_NetStream_t,
                                     Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_DirectShow_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_Source_ConnectionConfiguration,
                               Test_I_Source_DirectShow_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_DirectShow_SocketHandlerConfiguration,
                               Test_I_Source_DirectShow_NetStream_t,
                               Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<Test_I_Source_DirectShow_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   Test_I_Source_ConnectionConfiguration,
                                   Test_I_Source_DirectShow_ConnectionState,
                                   Test_I_RuntimeStatistic_t,
                                   Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                   Test_I_Source_DirectShow_NetStream_t,
                                   Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_SSLTCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Source_DirectShow_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_ConnectionConfiguration,
                                     Test_I_Source_DirectShow_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_DirectShow_SocketHandlerConfiguration,
                                     Test_I_Source_DirectShow_NetStream_t,
                                     Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_DirectShow_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_I_Source_ConnectionConfiguration,
                               Test_I_Source_DirectShow_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_DirectShow_SocketHandlerConfiguration,
                               Test_I_Source_DirectShow_NetStream_t,
                               Test_I_Source_DirectShow_UserData> Test_I_Source_DirectShow_UDPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Source_MediaFoundation_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_ConnectionConfiguration,
                                     Test_I_Source_MediaFoundation_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                     Test_I_Source_MediaFoundation_NetStream_t,
                                     Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_MediaFoundation_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_Source_ConnectionConfiguration,
                               Test_I_Source_MediaFoundation_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                               Test_I_Source_MediaFoundation_NetStream_t,
                               Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<Test_I_Source_MediaFoundation_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   Test_I_Source_ConnectionConfiguration,
                                   Test_I_Source_MediaFoundation_ConnectionState,
                                   Test_I_RuntimeStatistic_t,
                                   Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                   Test_I_Source_MediaFoundation_NetStream_t,
                                   Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_SSLTCPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Source_MediaFoundation_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_ConnectionConfiguration,
                                     Test_I_Source_MediaFoundation_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                                     Test_I_Source_MediaFoundation_NetStream_t,
                                     Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_MediaFoundation_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_I_Source_ConnectionConfiguration,
                               Test_I_Source_MediaFoundation_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_MediaFoundation_SocketHandlerConfiguration,
                               Test_I_Source_MediaFoundation_NetStream_t,
                               Test_I_Source_MediaFoundation_UserData> Test_I_Source_MediaFoundation_UDPConnector_t;
#else
typedef Net_Client_AsynchConnector_T<Test_I_Source_V4L2_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_Source_ConnectionConfiguration,
                                     struct Test_I_Source_V4L2_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                     Test_I_Source_V4L2_NetStream_t,
                                     struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_V4L2_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               struct Test_I_Source_ConnectionConfiguration,
                               struct Test_I_Source_V4L2_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                               Test_I_Source_V4L2_NetStream_t,
                               struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_TCPConnector_t;
typedef Net_Client_SSL_Connector_T<Test_I_Source_V4L2_SSLTCPConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   ACE_INET_Addr,
                                   struct Test_I_Source_ConnectionConfiguration,
                                   struct Test_I_Source_V4L2_ConnectionState,
                                   Test_I_RuntimeStatistic_t,
                                   struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                   Test_I_Source_V4L2_NetStream_t,
                                   struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_SSLTCPConnector_t;

typedef Net_Client_AsynchConnector_T<Test_I_Source_V4L2_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_Source_ConnectionConfiguration,
                                     struct Test_I_Source_V4L2_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                                     Test_I_Source_V4L2_NetStream_t,
                                     struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Source_V4L2_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct Test_I_Source_ConnectionConfiguration,
                               struct Test_I_Source_V4L2_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               struct Test_I_Source_V4L2_SocketHandlerConfiguration,
                               Test_I_Source_V4L2_NetStream_t,
                               struct Test_I_Source_V4L2_UserData> Test_I_Source_V4L2_UDPConnector_t;
#endif

//////////////////////////////////////////

// inbound
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                                         ACE_SOCK_STREAM>,
//                                  ACE_INET_Addr,
//                                  Test_I_Target_ConnectionConfiguration,
//                                  Test_I_Target_DirectShow_ConnectionState,
//                                  Test_I_RuntimeStatistic_t,
//                                  Test_I_Target_DirectShow_Stream,
//                                  Test_I_Target_DirectShow_UserData,
//                                  Stream_ModuleConfiguration,
//                                  Test_I_Target_DirectShow_ModuleHandlerConfiguration> Test_I_Target_DirectShow_TCPHandler_t;
//typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                                         ACE_SSL_SOCK_Stream>,
//                                  ACE_INET_Addr,
//                                  Test_I_Target_ConnectionConfiguration,
//                                  Test_I_Target_DirectShow_ConnectionState,
//                                  Test_I_RuntimeStatistic_t,
//                                  Test_I_Target_DirectShow_Stream,
//                                  Test_I_Target_DirectShow_UserData,
//                                  Stream_ModuleConfiguration,
//                                  Test_I_Target_DirectShow_ModuleHandlerConfiguration> Test_I_Target_DirectShow_SSLTCPHandler_t;
//typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Target_DirectShow_SocketHandlerConfiguration>,
//                                        ACE_INET_Addr,
//                                        Test_I_Target_ConnectionConfiguration,
//                                        Test_I_Target_DirectShow_ConnectionState,
//                                        Test_I_RuntimeStatistic_t,
//                                        Test_I_Target_DirectShow_Stream,
//                                        Test_I_Target_DirectShow_UserData,
//                                        Stream_ModuleConfiguration,
//                                        Test_I_Target_DirectShow_ModuleHandlerConfiguration> Test_I_Target_DirectShow_AsynchTCPHandler_t;
//typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
//                                                         Test_I_Target_DirectShow_SocketHandlerConfiguration>,
//                                  ACE_INET_Addr,
//                                  Test_I_Target_ConnectionConfiguration,
//                                  Test_I_Target_DirectShow_ConnectionState,
//                                  Test_I_RuntimeStatistic_t,
//                                  Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                  Test_I_Target_DirectShow_Stream,
//                                  Test_I_Target_DirectShow_UserData,
//                                  Stream_ModuleConfiguration,
//                                  Test_I_Target_DirectShow_ModuleHandlerConfiguration> Test_I_Target_DirectShow_UDPHandler_t;
//typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_I_Target_DirectShow_SocketHandlerConfiguration>,
//                                        Net_SOCK_Dgram,
//                                        ACE_INET_Addr,
//                                        Test_I_Target_ConnectionConfiguration,
//                                        Test_I_Target_DirectShow_ConnectionState,
//                                        Test_I_RuntimeStatistic_t,
//                                        Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                        Test_I_Target_DirectShow_Stream,
//                                        Test_I_Target_DirectShow_UserData,
//                                        Stream_ModuleConfiguration,
//                                        Test_I_Target_DirectShow_ModuleHandlerConfiguration> Test_I_Target_DirectShow_AsynchUDPHandler_t;
//typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                                         ACE_SOCK_STREAM>,
//                                  ACE_INET_Addr,
//                                  Test_I_Target_ConnectionConfiguration,
//                                  Test_I_Target_MediaFoundation_ConnectionState,
//                                  Test_I_RuntimeStatistic_t,
//                                  Test_I_Target_MediaFoundation_Stream,
//                                  Test_I_Target_MediaFoundation_UserData,
//                                  Stream_ModuleConfiguration,
//                                  Test_I_Target_MediaFoundation_ModuleHandlerConfiguration> Test_I_Target_MediaFoundation_TCPHandler_t;
//typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                                         ACE_SSL_SOCK_Stream>,
//                                  ACE_INET_Addr,
//                                  Test_I_Target_ConnectionConfiguration,
//                                  Test_I_Target_MediaFoundation_ConnectionState,
//                                  Test_I_RuntimeStatistic_t,
//                                  Test_I_Target_MediaFoundation_Stream,
//                                  Test_I_Target_MediaFoundation_UserData,
//                                  Stream_ModuleConfiguration,
//                                  Test_I_Target_MediaFoundation_ModuleHandlerConfiguration> Test_I_Target_MediaFoundation_SSLTCPHandler_t;
//typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Target_MediaFoundation_SocketHandlerConfiguration>,
//                                        ACE_INET_Addr,
//                                        Test_I_Target_ConnectionConfiguration,
//                                        Test_I_Target_MediaFoundation_ConnectionState,
//                                        Test_I_RuntimeStatistic_t,
//                                        Test_I_Target_MediaFoundation_Stream,
//                                        Test_I_Target_MediaFoundation_UserData,
//                                        Stream_ModuleConfiguration,
//                                        Test_I_Target_MediaFoundation_ModuleHandlerConfiguration> Test_I_Target_MediaFoundation_AsynchTCPHandler_t;
//typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
//                                                         Test_I_Target_MediaFoundation_SocketHandlerConfiguration>,
//                                  ACE_INET_Addr,
//                                  Test_I_Target_ConnectionConfiguration,
//                                  Test_I_Target_MediaFoundation_ConnectionState,
//                                  Test_I_RuntimeStatistic_t,
//                                  Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                  Test_I_Target_MediaFoundation_Stream,
//                                  Test_I_Target_MediaFoundation_UserData,
//                                  Stream_ModuleConfiguration,
//                                  Test_I_Target_MediaFoundation_ModuleHandlerConfiguration> Test_I_Target_MediaFoundation_UDPHandler_t;
//typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<Test_I_Target_MediaFoundation_SocketHandlerConfiguration>,
//                                        Net_SOCK_Dgram,
//                                        ACE_INET_Addr,
//                                        Test_I_Target_ConnectionConfiguration,
//                                        Test_I_Target_MediaFoundation_ConnectionState,
//                                        Test_I_RuntimeStatistic_t,
//                                        Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                        Test_I_Target_MediaFoundation_Stream,
//                                        Test_I_Target_MediaFoundation_UserData,
//                                        Stream_ModuleConfiguration,
//                                        Test_I_Target_MediaFoundation_ModuleHandlerConfiguration> Test_I_Target_MediaFoundation_AsynchUDPHandler_t;
#endif
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_I_Target_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct Test_I_Target_ConnectionConfiguration,
                                  struct Test_I_Target_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Target_Stream,
                                  struct Test_I_Target_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_TCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_I_Target_SocketHandlerConfiguration,
                                                         ACE_SSL_SOCK_Stream>,
                                  ACE_INET_Addr,
                                  struct Test_I_Target_ConnectionConfiguration,
                                  struct Test_I_Target_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Target_Stream,
                                  struct Test_I_Target_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_SSLTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Test_I_Target_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct Test_I_Target_ConnectionConfiguration,
                                        struct Test_I_Target_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Target_Stream,
                                        struct Test_I_Target_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_AsynchTCPHandler_t;
typedef Net_StreamUDPSocketBase_T<Net_UDPSocketHandler_T<Net_SOCK_Dgram,
                                                         struct Test_I_Target_SocketHandlerConfiguration>,
                                  ACE_INET_Addr,
                                  struct Test_I_Target_ConnectionConfiguration,
                                  struct Test_I_Target_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  struct Test_I_Target_SocketHandlerConfiguration,
                                  Test_I_Target_Stream,
                                  struct Test_I_Target_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_AsynchUDPSocketHandler_T<struct Test_I_Target_SocketHandlerConfiguration>,
                                        Net_SOCK_Dgram,
                                        ACE_INET_Addr,
                                        struct Test_I_Target_ConnectionConfiguration,
                                        struct Test_I_Target_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        struct Test_I_Target_SocketHandlerConfiguration,
                                        Test_I_Target_Stream,
                                        struct Test_I_Target_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_AsynchUDPHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Net_TCPConnectionBase_T<Test_I_Target_DirectShow_TCPHandler_t,
//                                Test_I_Target_ConnectionConfiguration,
//                                Test_I_Target_DirectShow_ConnectionState,
//                                Test_I_RuntimeStatistic_t,
//                                Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                Test_I_Target_DirectShow_Stream,
//                                Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_TCPConnection_t;
//typedef Net_TCPConnectionBase_T<Test_I_Target_DirectShow_SSLTCPHandler_t,
//                                Test_I_Target_ConnectionConfiguration,
//                                Test_I_Target_DirectShow_ConnectionState,
//                                Test_I_RuntimeStatistic_t,
//                                Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                Test_I_Target_DirectShow_Stream,
//                                Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_SSLTCPConnection_t;
//typedef Net_AsynchTCPConnectionBase_T<Test_I_Target_DirectShow_AsynchTCPHandler_t,
//                                      Test_I_Target_ConnectionConfiguration,
//                                      Test_I_Target_DirectShow_ConnectionState,
//                                      Test_I_RuntimeStatistic_t,
//                                      Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                      Test_I_Target_DirectShow_Stream,
//                                      Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_AsynchTCPConnection_t;
//typedef Net_UDPConnectionBase_T<Test_I_Target_DirectShow_UDPHandler_t,
//                                Test_I_Target_ConnectionConfiguration,
//                                Test_I_Target_DirectShow_ConnectionState,
//                                Test_I_RuntimeStatistic_t,
//                                Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                Test_I_Target_DirectShow_Stream,
//                                Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_UDPConnection_t;
//typedef Net_AsynchUDPConnectionBase_T<Test_I_Target_DirectShow_AsynchUDPHandler_t,
//                                      Test_I_Target_ConnectionConfiguration,
//                                      Test_I_Target_DirectShow_ConnectionState,
//                                      Test_I_RuntimeStatistic_t,
//                                      Test_I_Target_DirectShow_SocketHandlerConfiguration,
//                                      Test_I_Target_DirectShow_Stream,
//                                      Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_AsynchUDPConnection_t;
//typedef Net_TCPConnectionBase_T<Test_I_Target_MediaFoundation_TCPHandler_t,
//                                Test_I_Target_ConnectionConfiguration,
//                                Test_I_Target_MediaFoundation_ConnectionState,
//                                Test_I_RuntimeStatistic_t,
//                                Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                Test_I_Target_MediaFoundation_Stream,
//                                Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_TCPConnection_t;
//typedef Net_TCPConnectionBase_T<Test_I_Target_MediaFoundation_SSLTCPHandler_t,
//                                Test_I_Target_ConnectionConfiguration,
//                                Test_I_Target_MediaFoundation_ConnectionState,
//                                Test_I_RuntimeStatistic_t,
//                                Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                Test_I_Target_MediaFoundation_Stream,
//                                Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_SSLTCPConnection_t;
//typedef Net_AsynchTCPConnectionBase_T<Test_I_Target_MediaFoundation_AsynchTCPHandler_t,
//                                      Test_I_Target_ConnectionConfiguration,
//                                      Test_I_Target_MediaFoundation_ConnectionState,
//                                      Test_I_RuntimeStatistic_t,
//                                      Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                      Test_I_Target_MediaFoundation_Stream,
//                                      Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_AsynchTCPConnection_t;
//typedef Net_UDPConnectionBase_T<Test_I_Target_MediaFoundation_UDPHandler_t,
//                                Test_I_Target_ConnectionConfiguration,
//                                Test_I_Target_MediaFoundation_ConnectionState,
//                                Test_I_RuntimeStatistic_t,
//                                Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                Test_I_Target_MediaFoundation_Stream,
//                                Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_UDPConnection_t;
//typedef Net_AsynchUDPConnectionBase_T<Test_I_Target_MediaFoundation_AsynchUDPHandler_t,
//                                      Test_I_Target_ConnectionConfiguration,
//                                      Test_I_Target_MediaFoundation_ConnectionState,
//                                      Test_I_RuntimeStatistic_t,
//                                      Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
//                                      Test_I_Target_MediaFoundation_Stream,
//                                      Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_AsynchUDPConnection_t;
#endif
typedef Net_TCPConnectionBase_T<Test_I_Target_TCPHandler_t,
                                struct Test_I_Target_ConnectionConfiguration,
                                struct Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Target_SocketHandlerConfiguration,
                                Test_I_Target_Stream,
                                struct Test_I_Target_UserData> Test_I_Target_TCPConnection_t;
typedef Net_TCPConnectionBase_T<Test_I_Target_SSLTCPHandler_t,
                                struct Test_I_Target_ConnectionConfiguration,
                                struct Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Target_SocketHandlerConfiguration,
                                Test_I_Target_Stream,
                                struct Test_I_Target_UserData> Test_I_Target_SSLTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Target_AsynchTCPHandler_t,
                                      struct Test_I_Target_ConnectionConfiguration,
                                      struct Test_I_Target_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Target_SocketHandlerConfiguration,
                                      Test_I_Target_Stream,
                                      struct Test_I_Target_UserData> Test_I_Target_AsynchTCPConnection_t;
typedef Net_UDPConnectionBase_T<Test_I_Target_UDPHandler_t,
                                struct Test_I_Target_ConnectionConfiguration,
                                struct Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Target_SocketHandlerConfiguration,
                                Test_I_Target_Stream,
                                struct Test_I_Target_UserData> Test_I_Target_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_Target_AsynchUDPHandler_t,
                                      struct Test_I_Target_ConnectionConfiguration,
                                      struct Test_I_Target_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Target_SocketHandlerConfiguration,
                                      Test_I_Target_Stream,
                                      struct Test_I_Target_UserData> Test_I_Target_AsynchUDPConnection_t;

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Target_DirectShow_SocketHandlerConfiguration> Test_I_Target_DirectShow_IInetConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Target_MediaFoundation_SocketHandlerConfiguration> Test_I_Target_MediaFoundation_IInetConnector_t;
#endif
typedef Net_IConnector_T<ACE_INET_Addr,
                         struct Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IInetConnector_t;

#endif
