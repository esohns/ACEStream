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

#include <map>
#include <string>

#include "ace/INET_Addr.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Netlink_Addr.h"
#endif
#include "ace/SOCK_Connector.h"
#include "ace/Synch_Traits.h"

#include "common_timer_manager_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_control_message.h"
#include "stream_session_data.h"

#include "stream_net_io_stream.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_asynch_udpsockethandler.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_sock_connector.h"
#include "net_socket_common.h"
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

// forward declarations
//struct Test_I_Source_Stream_Configuration;
//struct Test_I_StreamConfiguration;
//struct Test_I_Source_ModuleHandlerConfiguration;
//struct Test_I_ModuleHandlerConfiguration;
//struct Test_I_Source_SessionData;
//typedef Stream_SessionData_T<Test_I_Source_SessionData> Test_I_Source_SessionData_t;
struct Test_I_Source_SessionData;

//////////////////////////////////////////

typedef Stream_SessionData_T<struct Test_I_Source_SessionData> Test_I_Source_SessionData_t;
class Test_I_Source_SessionMessage;
typedef Test_I_Message_T<enum Stream_MessageType,
                         Test_I_Source_SessionMessage> Test_I_Source_Message_t;
//typedef Stream_ControlMessage_T<enum Stream_ControlType,
//                                enum Stream_ControlMessageType,
//                                struct Common_AllocatorConfiguration> Stream_ControlMessage_t;

//////////////////////////////////////////

//extern const char stream_name_string_[];
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_Source_StreamConfiguration,
                               struct Test_I_Source_ModuleHandlerConfiguration> Test_I_Source_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_Source_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_Source_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_Source_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_Source_UDPConnectionConfiguration_t;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Source_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_ITCPConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Source_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_IUDPConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_UDPConnectionManager_t;

//////////////////////////////////////////

struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_Target_StreamConfiguration,
                               struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_StreamConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_Target_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_TCP> Test_I_Target_TCPConnectionConfiguration_t;
typedef Net_StreamConnectionConfiguration_T<Test_I_Target_StreamConfiguration_t,
                                            NET_TRANSPORTLAYER_UDP> Test_I_Target_UDPConnectionConfiguration_t;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Target_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_ITCPConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Target_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_IUDPConnectionManager_t;

//////////////////////////////////////////

//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_Source_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_Source_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_Source_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_Source_IUDPConnection_t;
//
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_Target_TCPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_Target_ITCPConnection_t;
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          //Test_I_Target_UDPConnectionConfiguration_t,
//                          struct Net_StreamConnectionState,
//                          Net_StreamStatistic_t> Test_I_Target_IUDPConnection_t;

//////////////////////////////////////////

//static constexpr const char network_io_stream_name_string_[] =
//    ACE_TEXT_ALWAYS_CHAR ("NetworkIOStream");
extern const char stream_name_string_[];
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_Source_StreamState,
                                      struct Test_I_Source_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_Source_ModuleHandlerConfiguration,
                                      struct Test_I_Source_SessionData,
                                      Test_I_Source_SessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_I_Source_Message_t,
                                      Test_I_Source_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Source_TCPConnectionManager_t,
                                      struct Stream_UserData> Test_I_Source_Net_TCPStream_t;
typedef Stream_Module_Net_IO_Stream_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      stream_name_string_,
                                      enum Stream_ControlType,
                                      enum Stream_SessionMessageType,
                                      enum Stream_StateMachine_ControlState,
                                      struct Test_I_Source_StreamState,
                                      struct Test_I_Source_StreamConfiguration,
                                      struct Stream_Statistic,
                                      Common_Timer_Manager_t,
                                      struct Test_I_Source_ModuleHandlerConfiguration,
                                      struct Test_I_Source_SessionData,
                                      Test_I_Source_SessionData_t,
                                      Stream_ControlMessage_t,
                                      Test_I_Source_Message_t,
                                      Test_I_Source_SessionMessage,
                                      ACE_INET_Addr,
                                      Test_I_Source_UDPConnectionManager_t,
                                      struct Stream_UserData> Test_I_Source_Net_UDPStream_t;

//////////////////////////////////////////

// outbound
typedef Net_TCPConnectionBase_T<ACE_MT_SYNCH,
                                Net_TCPSocketHandler_t,
                                Test_I_Source_TCPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_Source_Net_TCPStream_t,
                                struct Net_UserData> Test_I_Source_TCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Net_AsynchTCPSocketHandler_t,
                                      Test_I_Source_TCPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_Source_Net_TCPStream_t,
                                      struct Net_UserData> Test_I_Source_AsynchTCPConnection_t;

typedef Net_UDPConnectionBase_T<ACE_MT_SYNCH,
                                Net_UDPSocketHandler_t,
                                Test_I_Source_UDPConnectionConfiguration_t,
                                struct Net_StreamConnectionState,
                                Net_StreamStatistic_t,
                                Test_I_Source_Net_UDPStream_t,
                                struct Net_UserData> Test_I_Source_UDPConnection_t;
typedef Net_AsynchUDPConnectionBase_T<Net_AsynchUDPSocketHandler_t,
                                      Test_I_Source_UDPConnectionConfiguration_t,
                                      struct Net_StreamConnectionState,
                                      Net_StreamStatistic_t,
                                      Test_I_Source_Net_UDPStream_t,
                                      struct Net_UserData> Test_I_Source_AsynchUDPConnection_t;

//////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Source_TCPConnectionConfiguration_t> Test_I_Source_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Source_UDPConnectionConfiguration_t> Test_I_Source_IUDPConnector_t;

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Target_TCPConnectionConfiguration_t> Test_I_Target_ITCPConnector_t;
typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Target_UDPConnectionConfiguration_t> Test_I_Target_IUDPConnector_t;

/////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<Test_I_Source_AsynchTCPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_TCPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_TCPSocketConfiguration_t,
                                     Test_I_Source_Net_TCPStream_t,
                                     struct Net_UserData> Test_I_Source_TCPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_Source_TCPConnection_t,
                               Net_SOCK_Connector,
                               ACE_INET_Addr,
                               Test_I_Source_TCPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_TCPSocketConfiguration_t,
                               Test_I_Source_Net_TCPStream_t,
                               struct Net_UserData> Test_I_Source_TCPConnector_t;

typedef Net_Client_AsynchConnector_T<Test_I_Source_AsynchUDPConnection_t,
                                     ACE_INET_Addr,
                                     Test_I_Source_UDPConnectionConfiguration_t,
                                     struct Net_StreamConnectionState,
                                     Net_StreamStatistic_t,
                                     Net_UDPSocketConfiguration_t,
                                     Test_I_Source_Net_UDPStream_t,
                                     struct Net_UserData> Test_I_Source_UDPAsynchConnector_t;
typedef Net_Client_Connector_T<ACE_MT_SYNCH,
                               Test_I_Source_UDPConnection_t,
                               ACE_SOCK_CONNECTOR,
                               ACE_INET_Addr,
                               Test_I_Source_UDPConnectionConfiguration_t,
                               struct Net_StreamConnectionState,
                               Net_StreamStatistic_t,
                               Net_UDPSocketConfiguration_t,
                               Test_I_Source_Net_UDPStream_t,
                               struct Net_UserData> Test_I_Source_UDPConnector_t;

#endif
