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

#ifndef STREAM_MODULE_NET_COMMON_H
#define STREAM_MODULE_NET_COMMON_H

#include "ace/INET_Addr.h"
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
#include "ace/Netlink_Addr.h"
#endif
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

//#include "net_connection_manager.h"
//#include "net_iconnectionmanager.h"

#include "net_defines.h"
//#include "net_configuration.h"
//#include "net_connection_common.h"
#include "net_iconnector.h"

// forward declarations
class Stream_IAllocator;
//struct Net_Configuration;
struct Net_SocketConfiguration;
//struct Net_StreamUserData;
//struct Test_I_ConnectionState;

//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//typedef Net_IConnection_T<ACE_Netlink_Addr,
//                          Net_Configuration,
//                          Stream_Statistic,
//                          Net_Stream> Net_INetlinkConnection_t;
//#endif
//typedef Net_IConnection_T<ACE_INET_Addr,
//                          Net_Configuration,
//                          Stream_Statistic,
//                          Net_Stream> Net_IConnection_t;

//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//typedef Net_IConnectionManager_T<ACE_Netlink_Addr,
//                                 Test_I_Configuration,
//                                 Test_I_Stream_ConnectionState,
//                                 Net_RuntimeStatistic_t,
//                                 Test_I_Stream,
//                                 ////////
//                                 Test_I_Stream_UserData> Test_I_Stream_INetlinkConnectionManager_t;
//#endif
//typedef Net_IConnectionManager_T<ACE_INET_Addr,
//                                 Test_I_Configuration,
//                                 Test_I_Stream_ConnectionState,
//                                 Net_RuntimeStatistic_t,
//                                 Test_I_Stream,
//                                 ////////
//                                 Test_I_Stream_UserData> Test_I_Stream_IInetConnectionManager_t;

//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//typedef Net_Connection_Manager_T<ACE_Netlink_Addr,
//                                 Test_I_Configuration,
//                                 Test_I_Stream_ConnectionState,
//                                 Net_RuntimeStatistic_t,
//                                 Test_I_Stream,
//                                 ////////
//                                 Test_I_Stream_UserData> Test_I_Stream_NetlinkConnectionManager_t;
//#endif
//typedef Net_Connection_Manager_T<ACE_INET_Addr,
//                                 Test_I_Configuration,
//                                 Test_I_Stream_ConnectionState,
//                                 Net_RuntimeStatistic_t,
//                                 Test_I_Stream,
//                                 ////////
//                                 Test_I_Stream_UserData> Test_I_Stream_InetConnectionManager_t;

//#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
//typedef ACE_Singleton<Test_I_Stream_NetlinkConnectionManager_t,
//                      ACE_SYNCH_MUTEX> TEST_I_STREAM_NETLINKCONNECTIONMANAGER_SINGLETON;
//#endif
//typedef ACE_Singleton<Test_I_Stream_InetConnectionManager_t,
//                      ACE_SYNCH_MUTEX> TEST_I_STREAM_CONNECTIONMANAGER_SINGLETON;

///////////////////////////////////////////

struct Stream_SocketHandlerConfiguration
{
  inline Stream_SocketHandlerConfiguration ()
   : bufferSize (NET_STREAM_MESSAGE_DATA_BUFFER_SIZE)
   , messageAllocator (NULL)
   , socketConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
     ////////////////////////////////////
//   , userData (NULL)
  {};

  unsigned int             bufferSize; // pdu size (if fixed)
  Stream_IAllocator*       messageAllocator;
  Net_SocketConfiguration* socketConfiguration;
  ACE_Time_Value           statisticReportingInterval; // [ACE_Time_Value::zero: off]

//  Net_StreamUserData*      userData;
};

typedef Net_IConnector_T<ACE_INET_Addr,
                         Stream_SocketHandlerConfiguration> Stream_IInetConnector_t;
//typedef Net_IConnector_T<ACE_INET_Addr,
//                         Stream_SocketHandlerConfiguration> Net_IConnector_t;

//typedef Net_Client_AsynchConnector_T<Net_AsynchTCPConnection,
//                                     ////
//                                     ACE_INET_Addr,
//                                     Test_I_Configuration,
//                                     Test_I_Stream_ConnectionState,
//                                     Net_RuntimeStatistic_t,
//                                     Net_Stream,
//                                     ////
//                                     Stream_SocketHandlerConfiguration,
//                                     ////
//                                     Test_I_Stream_UserData> Test_I_Stream_AsynchConnector_t;
//typedef Net_Client_Connector_T<Net_TCPConnection,
//                               //////////
//                               ACE_INET_Addr,
//                               Test_I_Configuration,
//                               Test_I_Stream_ConnectionState,
//                               Net_RuntimeStatistic_t,
//                               Net_Stream,
//                               //////////
//                               Stream_SocketHandlerConfiguration,
//                               //////////
//                               Test_I_Stream_UserData> Test_I_Stream_Connector_t;

#endif
