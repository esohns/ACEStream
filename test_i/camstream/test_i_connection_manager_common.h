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

//#include "test_i_source_common.h"
//#include "test_i_target_common.h"

// forward declarations
typedef Stream_Statistic Test_I_RuntimeStatistic_t;
struct Test_I_Source_Configuration;
struct Test_I_Source_ConnectionState;
struct Test_I_Source_UserData;
struct Test_I_Target_Configuration;
struct Test_I_Target_ConnectionState;
struct Test_I_Target_UserData;

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Source_Configuration,
                                 Test_I_Source_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_Source_UserData> Test_I_Source_IInetConnectionManager_t;
typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Test_I_Target_Configuration,
                                 Test_I_Target_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_Target_UserData> Test_I_Target_IInetConnectionManager_t;

typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Source_Configuration,
                                 Test_I_Source_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_Source_UserData> Test_I_Source_InetConnectionManager_t;
typedef Net_Connection_Manager_T<ACE_INET_Addr,
                                 Test_I_Target_Configuration,
                                 Test_I_Target_ConnectionState,
                                 Test_I_RuntimeStatistic_t,
                                 ////////
                                 Test_I_Target_UserData> Test_I_Target_InetConnectionManager_t;

typedef ACE_Singleton<Test_I_Source_InetConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_SOURCE_CONNECTIONMANAGER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_InetConnectionManager_t,
                      ACE_SYNCH_MUTEX> TEST_I_TARGET_CONNECTIONMANAGER_SINGLETON;

#endif
