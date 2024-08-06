/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns                                      *
 *   erik.sohns@web.de                                                     *
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

#ifndef TEST_I_POP_RECEIVE_NETWORK_H
#define TEST_I_POP_RECEIVE_NETWORK_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_timer_manager_common.h"

#include "stream_net_io_stream.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#if defined (SSL_SUPPORT)
#include "net_client_ssl_connector.h"
#endif // SSL_SUPPORT

#include "pop_common.h"
#include "pop_configuration.h"
#include "pop_stream_common.h"
#include "pop_network.h"

#include "test_i_pop_receive_stream_common.h"

const char stream_net_name_string_[] = ACE_TEXT_ALWAYS_CHAR (STREAM_NET_DEFAULT_NAME_STRING);

/////////////////////////////////////////

typedef Net_StreamConnectionConfiguration_T<Stream_POPReceive_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_POPReceive_ConnectionConfiguration_t;

/////////////////////////////////////////

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_POPReceive_ConnectionConfiguration_t,
                                 struct POP_ConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_POPReceive_IConnection_Manager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_POPReceive_ConnectionConfiguration_t,
                                 struct POP_ConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_POPReceive_Connection_Manager_t;

/////////////////////////////////////////

typedef ACE_Singleton<Test_I_POPReceive_Connection_Manager_t,
                      ACE_SYNCH_MUTEX> TEST_I_POPReceive_CONNECTIONMANAGER_SINGLETON;

/////////////////////////////////////////

typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_net_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct POP_StreamState,
                                      struct POP_StreamConfiguration,
                                      POP_Statistic_t,
                                      Common_Timer_Manager_t,
                                      struct Stream_POPReceive_ModuleHandlerConfiguration,
                                      struct POP_Stream_SessionData,
                                      POP_Stream_SessionData_t,
                                      Stream_ControlMessage_t,
                                      POP_Message_t,
                                      POP_SessionMessage_t,
                                      ACE_INET_Addr,
                                      Test_I_POPReceive_Connection_Manager_t,
                                      struct Stream_UserData> Test_I_POPReceive_ConnectionStream_t;

typedef Net_IStreamConnection_T<ACE_INET_Addr,
                                Test_I_POPReceive_ConnectionConfiguration_t,
                                struct POP_ConnectionState,
                                Net_StreamStatistic_t,
                                Net_TCPSocketConfiguration_t,
                                Test_I_POPReceive_ConnectionStream_t,
                                enum Stream_StateMachine_ControlState> Test_I_POPReceive_IStreamConnection_t;

typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_POPReceive_ConnectionConfiguration_t,
                                struct POP_ConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_POPReceive_ConnectionStream_t,
                                struct Net_UserData> Test_I_POPReceive_Connection_t;
#if defined (SSL_SUPPORT)
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_SSLSocketHandler_t,
                                Test_I_POPReceive_ConnectionConfiguration_t,
                                struct POP_ConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_POPReceive_ConnectionStream_t,
                                struct Net_UserData> Test_I_POPReceive_SSLConnection_t;
#endif // SSL_SUPPORT

typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_POPReceive_ConnectionConfiguration_t,
                                      struct POP_ConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_POPReceive_ConnectionStream_t,
                                      struct Net_UserData> Test_I_POPReceive_AsynchConnection_t;

/////////////////////////////////////////

typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_POPReceive_Connection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_POPReceive_ConnectionConfiguration_t,
                               struct POP_ConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_POPReceive_ConnectionStream_t,
                               struct Net_UserData> Test_I_POPReceive_Connector_t;
#if defined (SSL_SUPPORT)
typedef Net_Client_SSL_Connector_T<Test_I_POPReceive_SSLConnection_t,
                                   ACE_SSL_SOCK_Connector,
                                   Test_I_POPReceive_ConnectionConfiguration_t,
                                   struct POP_ConnectionState,
                                   Net_StreamStatistic_t,
                                   Test_I_POPReceive_ConnectionStream_t,
                                   struct Net_UserData> Test_I_POPReceive_SSLConnector_t;
#endif // SSL_SUPPORT

typedef Net_Client_AsynchConnector_T<Test_I_POPReceive_AsynchConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_POPReceive_ConnectionConfiguration_t,
                                     struct POP_ConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_POPReceive_ConnectionStream_t,
                                     struct Net_UserData> Test_I_POPReceive_AsynchConnector_t;

/////////////////////////////////////////

#endif
