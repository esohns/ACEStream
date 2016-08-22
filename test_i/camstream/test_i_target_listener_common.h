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

#include "ace/config-lite.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_server_asynchlistener.h"
#include "net_server_listener.h"
#include "net_server_ssl_listener.h"
#include "net_sock_acceptor.h"

#include "test_i_connection_common.h"

#include "test_i_target_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_Target_DirectShow_ListenerConfiguration;
struct Test_I_Target_MediaFoundation_ListenerConfiguration;
class Test_I_Target_DirectShow_Stream;
class Test_I_Target_MediaFoundation_Stream;
#else
struct Test_I_Target_V4L2_ListenerConfiguration;
class Test_I_Target_V4L2_Stream;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Server_AsynchListener_T<Test_I_Target_DirectShow_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    Test_I_Target_DirectShow_ListenerConfiguration,
                                    Test_I_Target_DirectShow_ConnectionState,
                                    Test_I_Target_DirectShow_Stream,
                                    Test_I_Target_DirectShow_SocketHandlerConfiguration,
                                    Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_Target_DirectShow_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_I_Target_DirectShow_ListenerConfiguration,
                              Test_I_Target_DirectShow_ConnectionState,
                              Test_I_Target_DirectShow_Stream,
                              Test_I_Target_DirectShow_SocketHandlerConfiguration,
                              Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_Listener_t;
typedef Net_Server_SSL_Listener_T<Test_I_Target_DirectShow_SSLTCPConnection_t,
                                  ACE_SSL_SOCK_Connector,
                                  ACE_INET_Addr,
                                  Test_I_Target_DirectShow_Configuration,
                                  Test_I_Target_DirectShow_ConnectionState,
                                  Test_I_Target_DirectShow_Stream,
                                  Test_I_Target_DirectShow_SocketHandlerConfiguration,
                                  Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_SSLListener_t;
typedef Net_Server_AsynchListener_T<Test_I_Target_MediaFoundation_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    Test_I_Target_MediaFoundation_ListenerConfiguration,
                                    Test_I_Target_MediaFoundation_ConnectionState,
                                    Test_I_Target_MediaFoundation_Stream,
                                    Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
                                    Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_Target_MediaFoundation_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_I_Target_MediaFoundation_ListenerConfiguration,
                              Test_I_Target_MediaFoundation_ConnectionState,
                              Test_I_Target_MediaFoundation_Stream,
                              Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
                              Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_Listener_t;
typedef Net_Server_SSL_Listener_T<Test_I_Target_MediaFoundation_SSLTCPConnection_t,
                                  ACE_SSL_SOCK_Connector,
                                  ACE_INET_Addr,
                                  Test_I_Target_MediaFoundation_Configuration,
                                  Test_I_Target_MediaFoundation_ConnectionState,
                                  Test_I_Target_MediaFoundation_Stream,
                                  Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
                                  Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_SSLListener_t;
#else
typedef Net_Server_AsynchListener_T<Test_I_Target_V4L2_AsynchTCPConnection_t,
                                    ACE_INET_Addr,
                                    Test_I_Target_V4L2_ListenerConfiguration,
                                    Test_I_Target_V4L2_ConnectionState,
                                    Test_I_Target_V4L2_Stream,
                                    Test_I_Target_SocketHandlerConfiguration,
                                    Test_I_Target_UserData> Test_I_Target_V4L2_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_Target_V4L2_TCPConnection_t,
                              Net_SOCK_Acceptor,
                              ACE_INET_Addr,
                              Test_I_Target_V4L2_ListenerConfiguration,
                              Test_I_Target_V4L2_ConnectionState,
                              Test_I_Target_V4L2_Stream,
                              Test_I_Target_SocketHandlerConfiguration,
                              Test_I_Target_UserData> Test_I_Target_V4L2_Listener_t;
typedef Net_Server_SSL_Listener_T<Test_I_Target_V4L2_SSLTCPConnection_t,
                                  ACE_SSL_SOCK_Connector,
                                  ACE_INET_Addr,
                                  Test_I_Target_V4L2_Configuration,
                                  Test_I_Target_V4L2_ConnectionState,
                                  Test_I_Target_V4L2_Stream,
                                  Test_I_Target_SocketHandlerConfiguration,
                                  Test_I_Target_UserData> Test_I_Target_V4L2_SSLListener_t;
#endif

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Client_AsynchConnector_T<Test_I_Target_DirectShow_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Target_DirectShow_Configuration,
                                     Test_I_Target_DirectShow_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Target_DirectShow_Stream,
                                     Test_I_Target_DirectShow_SocketHandlerConfiguration,
                                     Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Target_DirectShow_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_I_Target_DirectShow_Configuration,
                               Test_I_Target_DirectShow_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Target_DirectShow_Stream,
                               Test_I_Target_DirectShow_SocketHandlerConfiguration,
                               Test_I_Target_DirectShow_UserData> Test_I_Target_DirectShow_UDPConnector_t;
typedef Net_Client_AsynchConnector_T<Test_I_Target_MediaFoundation_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Target_MediaFoundation_Configuration,
                                     Test_I_Target_MediaFoundation_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Target_MediaFoundation_Stream,
                                     Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
                                     Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Target_MediaFoundation_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_I_Target_MediaFoundation_Configuration,
                               Test_I_Target_MediaFoundation_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Target_MediaFoundation_Stream,
                               Test_I_Target_MediaFoundation_SocketHandlerConfiguration,
                               Test_I_Target_MediaFoundation_UserData> Test_I_Target_MediaFoundation_UDPConnector_t;
#else
typedef Net_Client_AsynchConnector_T<Test_I_Target_V4L2_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Target_V4L2_Configuration,
                                     Test_I_Target_V4L2_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Target_V4L2_Stream,
                                     Test_I_Target_SocketHandlerConfiguration,
                                     Test_I_Target_UserData> Test_I_Target_V4L2_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_Target_V4L2_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_I_Target_V4L2_Configuration,
                               Test_I_Target_V4L2_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Target_V4L2_Stream,
                               Test_I_Target_SocketHandlerConfiguration,
                               Test_I_Target_UserData> Test_I_Target_V4L2_UDPConnector_t;
#endif

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef ACE_Singleton<Test_I_Target_DirectShow_AsynchListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_DIRECTSHOW_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_DirectShow_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_DIRECTSHOW_LISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_DirectShow_SSLListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_DIRECTSHOW_SSL_LISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_MediaFoundation_AsynchListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_MEDIAFOUNDATION_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_MediaFoundation_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_MEDIAFOUNDATION_LISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_MediaFoundation_SSLListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_MEDIAFOUNDATION_SSL_LISTENER_SINGLETON;
#else
typedef ACE_Singleton<Test_I_Target_V4L2_AsynchListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_V4L2_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_LISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_V4L2_SSLListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_SSL_LISTENER_SINGLETON;
#endif

#endif
