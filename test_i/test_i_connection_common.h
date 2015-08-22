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
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif

#include "stream_common.h"

#include "net_asynch_tcpsockethandler.h"
#include "net_common.h"
#include "net_stream_asynch_tcpsocket_base.h"
#include "net_stream_tcpsocket_base.h"
#include "net_tcpconnection_base.h"
#include "net_tcpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"

#include "test_i_common.h"

// forward declarations
struct Test_I_Stream_SocketHandlerConfiguration;
//struct Test_I_Configuration;
//typedef Stream_Statistic Test_I_RuntimeStatistic_t;
class Test_I_Source_Stream;
struct Test_I_Stream_UserData;
class Test_I_Target_Stream;

struct Test_I_ConnectionState
{
  inline Test_I_ConnectionState ()
   : configuration ()
   , status (NET_CONNECTION_STATUS_INVALID)
   , currentStatistic ()
   , userData (NULL)
  {};

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Test_I_Configuration      configuration;

  Net_Connection_Status     status;

  Test_I_RuntimeStatistic_t currentStatistic;

  Test_I_Stream_UserData*   userData;
};

/////////////////////////////////////////

typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Configuration,
                          Test_I_ConnectionState,
                          Test_I_RuntimeStatistic_t,
                          Test_I_Source_Stream> Test_I_ISourceConnection_t;

/////////////////////////////////////////

typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Stream_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Configuration,
                                  Test_I_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Source_Stream,
                                  ///////
                                  Test_I_Stream_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Stream_ModuleHandlerConfiguration> Test_I_SourceTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Stream_SocketHandlerConfiguration>,

                                        ACE_INET_Addr,
                                        Test_I_Configuration,
                                        Test_I_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Source_Stream,

                                        Test_I_Stream_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Stream_ModuleHandlerConfiguration> Test_I_SourceAsynchTCPHandler_t;
typedef Net_StreamTCPSocketBase_T<Net_TCPSocketHandler_T<Test_I_Stream_SocketHandlerConfiguration>,
                                  ///////
                                  ACE_INET_Addr,
                                  Test_I_Configuration,
                                  Test_I_ConnectionState,
                                  Test_I_RuntimeStatistic_t,
                                  Test_I_Target_Stream,
                                  ///////
                                  Test_I_Stream_UserData,
                                  ///////
                                  Stream_ModuleConfiguration,
                                  Test_I_Stream_ModuleHandlerConfiguration> Test_I_TargetTCPHandler_t;
typedef Net_StreamAsynchTCPSocketBase_T<Net_AsynchTCPSocketHandler_T<Test_I_Stream_SocketHandlerConfiguration>,

                                        ACE_INET_Addr,
                                        Test_I_Configuration,
                                        Test_I_ConnectionState,
                                        Test_I_RuntimeStatistic_t,
                                        Test_I_Target_Stream,

                                        Test_I_Stream_UserData,

                                        Stream_ModuleConfiguration,
                                        Test_I_Stream_ModuleHandlerConfiguration> Test_I_TargetAsynchTCPHandler_t;

typedef Net_TCPConnectionBase_T<Test_I_SourceTCPHandler_t,
                                /////////
                                Test_I_Configuration,
                                Test_I_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Source_Stream,
                                /////////
                                Test_I_Stream_SocketHandlerConfiguration,
                                /////////
                                Test_I_Stream_UserData> Test_I_SourceTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_SourceAsynchTCPHandler_t,
                                      ///
                                      Test_I_Configuration,
                                      Test_I_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Source_Stream,
                                      /////////
                                      Test_I_Stream_SocketHandlerConfiguration,
                                      /////////
                                      Test_I_Stream_UserData> IRC_Client_SourceAsynchTCPConnection_t;
typedef Net_TCPConnectionBase_T<Test_I_TargetTCPHandler_t,
                                /////////
                                Test_I_Configuration,
                                Test_I_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Target_Stream,
                                /////////
                                Test_I_Stream_SocketHandlerConfiguration,
                                /////////
                                Test_I_Stream_UserData> Test_I_TargetTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_TargetAsynchTCPHandler_t,
                                      ///
                                      Test_I_Configuration,
                                      Test_I_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Target_Stream,
                                      /////////
                                      Test_I_Stream_SocketHandlerConfiguration,
                                      /////////
                                      Test_I_Stream_UserData> IRC_Client_TargetAsynchTCPConnection_t;

typedef Net_TCPConnectionBase_T<Test_I_TargetTCPHandler_t,
                                /////////
                                Test_I_Configuration,
                                Test_I_ConnectionState,
                                Test_I_RuntimeStatistic_t,
                                Test_I_Target_Stream,
                                /////////
                                Test_I_Stream_SocketHandlerConfiguration,
                                /////////
                                Test_I_Stream_UserData> Test_I_TargetTCPConnection_t;
typedef Net_AsynchTCPConnectionBase_T<Test_I_TargetAsynchTCPHandler_t,
                                      ///
                                      Test_I_Configuration,
                                      Test_I_ConnectionState,
                                      Test_I_RuntimeStatistic_t,
                                      Test_I_Target_Stream,
                                      /////////
                                      Test_I_Stream_SocketHandlerConfiguration,
                                      /////////
                                      Test_I_Stream_UserData> IRC_Client_TargetAsynchTCPConnection_t;

/////////////////////////////////////////

typedef Net_IConnector_T<ACE_INET_Addr,
                         Test_I_Stream_SocketHandlerConfiguration> Test_I_Stream_IInetConnector_t;

/////////////////////////////////////////

typedef Net_Client_AsynchConnector_T<IRC_Client_SourceAsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Configuration,
                                     Test_I_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Source_Stream,
                                     ////
                                     Test_I_Stream_SocketHandlerConfiguration,
                                     ////
                                     Test_I_Stream_UserData> Test_I_Stream_SourceTCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_SourceTCPConnection_t,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Configuration,
                               Test_I_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Source_Stream,
                               //////////
                               Test_I_Stream_SocketHandlerConfiguration,
                               //////////
                               Test_I_Stream_UserData> Test_I_Stream_SourceTCPConnector_t;
typedef Net_Client_AsynchConnector_T<IRC_Client_TargetAsynchTCPConnection_t,
                                     ////
                                     ACE_INET_Addr,
                                     Test_I_Configuration,
                                     Test_I_ConnectionState,
                                     Test_I_RuntimeStatistic_t,
                                     Test_I_Target_Stream,
                                     ////
                                     Test_I_Stream_SocketHandlerConfiguration,
                                     ////
                                     Test_I_Stream_UserData> Test_I_Stream_TargetTCPAsynchConnector_t;
typedef Net_Client_Connector_T<Test_I_TargetTCPConnection_t,
                               //////////
                               ACE_INET_Addr,
                               Test_I_Configuration,
                               Test_I_ConnectionState,
                               Test_I_RuntimeStatistic_t,
                               Test_I_Target_Stream,
                               //////////
                               Test_I_Stream_SocketHandlerConfiguration,
                               //////////
                               Test_I_Stream_UserData> Test_I_Stream_TargetTCPConnector_t;

#endif
