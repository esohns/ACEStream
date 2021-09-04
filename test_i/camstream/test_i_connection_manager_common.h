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

#ifndef TEST_I_CONNECTION_MANAGER_COMMON_H
#define TEST_I_CONNECTION_MANAGER_COMMON_H

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"

#include "net_connection_manager.h"
#include "net_iconnectionmanager.h"

#include "test_i_common.h"

#include "test_i_camstream_network.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_DirectShow_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_DirectShow_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_DirectShow_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_DirectShow_UDPConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_DirectShow_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_DirectShow_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_DirectShow_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_DirectShow_UDPConnectionManager_t;

typedef ACE_Singleton<Test_I_Source_DirectShow_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Source_DirectShow_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_DIRECTSHOW_UDP_CONNECTIONMANAGER_SINGLETON;

typedef ACE_Singleton<Test_I_Target_DirectShow_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_DirectShow_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_DIRECTSHOW_UDP_CONNECTIONMANAGER_SINGLETON;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_MediaFoundation_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_MediaFoundation_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_MediaFoundation_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_MediaFoundation_UDPConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_MediaFoundation_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_MediaFoundation_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_MediaFoundation_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_MediaFoundation_UDPConnectionManager_t;

typedef ACE_Singleton<Test_I_Source_MediaFoundation_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_MEDIAFOUNDATION_TCP_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Source_MediaFoundation_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_MEDIAFOUNDATION_UDP_CONNECTIONMANAGER_SINGLETON;

typedef ACE_Singleton<Test_I_Target_MediaFoundation_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_MEDIAFOUNDATION_TCP_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_MediaFoundation_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_MEDIAFOUNDATION_UDP_CONNECTIONMANAGER_SINGLETON;
#else
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_V4L_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_V4L_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Source_V4L_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Source_V4L_UDPConnectionManager_t;

typedef ACE_Singleton<Test_I_Source_V4L_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_V4L_TCP_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Source_V4L_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_V4L_UDP_CONNECTIONMANAGER_SINGLETON;

typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_TCPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_TCPConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_MT_SYNCH,
                                 ACE_INET_Addr,
                                 Test_I_Target_UDPConnectionConfiguration_t,
                                 struct Net_StreamConnectionState,
                                 Net_StreamStatistic_t,
                                 struct Net_UserData> Test_I_Target_UDPConnectionManager_t;

typedef ACE_Singleton<Test_I_Target_TCPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_TCP_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_UDPConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_UDP_CONNECTIONMANAGER_SINGLETON;
#endif

#endif
