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

#ifndef TEST_I_TARGET_LISTENER_COMMON_H
#define TEST_I_TARGET_LISTENER_COMMON_H

#include <ace/Global_Macros.h>
#include <ace/INET_Addr.h>
#include <ace/Singleton.h>
#include <ace/SOCK_Connector.h>
#include <ace/Synch_Traits.h>

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_sock_acceptor.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_stream_udpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"
#include "net_udpconnection_base.h"

#include "net_client_connector.h"
#include "net_client_asynchconnector.h"

#include "net_server_asynchlistener.h"
#include "net_server_listener.h"

#include "test_i_filestream_network.h"
#include "test_i_target_common.h"
#include "test_i_target_stream.h"

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<struct Test_I_Target_SocketHandlerConfiguration,
                                                         ACE_SOCK_STREAM>,
                                  ACE_INET_Addr,
                                  struct Test_I_Target_ConnectionConfiguration,
                                  struct Test_I_Target_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Target_Stream,
                                  struct Test_I_Target_UserData,
                                  struct Stream_ModuleConfiguration,
                                  struct Test_I_ModuleHandlerConfiguration> Test_I_Target_TCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<struct Test_I_Target_SocketHandlerConfiguration>,
                                        ACE_INET_Addr,
                                        struct Test_I_Target_ConnectionConfiguration,
                                        struct Test_I_Target_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Target_Stream,
                                        struct Test_I_Target_UserData,
                                        struct Stream_ModuleConfiguration,
                                        struct Test_I_ModuleHandlerConfiguration> Test_I_Target_AsynchTCPHandler_t;

typedef Net_TCPConnectionBase_T<Test_I_Target_TCPHandler_t,
                                struct Test_I_Target_ConnectionConfiguration,
                                struct Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Target_SocketHandlerConfiguration,
                                struct Test_I_Target_ListenerConfiguration,
                                Test_I_Target_Stream,
                                struct Test_I_Target_UserData> Test_I_Target_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_Target_AsynchTCPHandler_t,
                                      struct Test_I_Target_ConnectionConfiguration,
                                      struct Test_I_Target_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Target_SocketHandlerConfiguration,
                                      struct Test_I_Target_ListenerConfiguration,
                                      Test_I_Target_Stream,
                                      struct Test_I_Target_UserData> Test_I_Target_AsynchTCPConnection_t;

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
                                  struct Test_I_ModuleHandlerConfiguration> Test_I_InboundUDPHandler_t;
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
                                        struct Test_I_ModuleHandlerConfiguration> Test_I_InboundAsynchUDPHandler_t;

typedef Net_UDPConnectionBase_T<Test_I_InboundUDPHandler_t,
                                struct Test_I_Target_ConnectionConfiguration,
                                struct Test_I_Target_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                struct Test_I_Target_SocketHandlerConfiguration,
                                Test_I_Target_Stream,
                                struct Test_I_Target_UserData> Test_I_InboundUDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Test_I_InboundAsynchUDPHandler_t,
                                      struct Test_I_Target_ConnectionConfiguration,
                                      struct Test_I_Target_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      struct Test_I_Target_SocketHandlerConfiguration,
                                      Test_I_Target_Stream,
                                      struct Test_I_Target_UserData> Test_I_InboundAsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_Server_AsynchListener_T<Test_I_Target_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    struct Test_I_Target_ListenerConfiguration,
                                    struct Test_I_Target_ConnectionState,
                                    struct Test_I_Target_SocketHandlerConfiguration,
                                    Test_I_Target_Stream,
                                    struct Test_I_Target_UserData> Test_I_Target_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_Target_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              struct Test_I_Target_ListenerConfiguration,
                              struct Test_I_Target_ConnectionState,
                              struct Test_I_Target_SocketHandlerConfiguration,
                              Test_I_Target_Stream,
                              struct Test_I_Target_UserData> Test_I_Target_Listener_t;

typedef Net_Client_AsynchConnector_T<Test_I_InboundAsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     struct Test_I_Target_ConnectionConfiguration,
                                     struct Test_I_Target_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     struct Test_I_Target_SocketHandlerConfiguration,
                                     Test_I_Target_Stream,
                                     struct Test_I_Target_UserData> Test_I_InboundUDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_InboundUDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               struct Test_I_Target_ConnectionConfiguration,
                               struct Test_I_Target_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               struct Test_I_Target_SocketHandlerConfiguration,
                               Test_I_Target_Stream,
                               struct Test_I_Target_UserData> Test_I_InboundUDPConnector_t;

typedef ACE_Singleton<Test_I_Target_AsynchListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_LISTENER_SINGLETON;

#endif
