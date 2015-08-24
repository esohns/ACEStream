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

#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "net_server_asynchlistener.h"
#include "net_server_listener.h"

#include "test_i_connection_common.h"

#include "test_i_target_common.h"
#include "test_i_target_stream.h"

typedef Net_Server_AsynchListener_T<Test_I_AsynchTCPConnection_t,
                                    /////
                                    ACE_INET_Addr,
                                    Test_I_Target_ListenerConfiguration,
                                    Test_I_ConnectionState,
                                    Test_I_Target_Stream,
                                    /////
                                    Test_I_Stream_SocketHandlerConfiguration,
                                    /////
                                    Test_I_Stream_UserData> Test_I_Target_AsynchListener_t;
typedef Net_Server_Listener_T<Test_I_TCPConnection_t,
                              ///////////
                              ACE_INET_Addr,
                              Test_I_Target_ListenerConfiguration,
                              Test_I_ConnectionState,
                              Test_I_Target_Stream,
                              ///////////
                              Test_I_Stream_SocketHandlerConfiguration,
                              ///////////
                              Test_I_Stream_UserData> Test_I_Target_Listener_t;

typedef ACE_Singleton<Test_I_Target_AsynchListener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_ASYNCHLISTENER_SINGLETON;
typedef ACE_Singleton<Test_I_Target_Listener_t,
                      ACE_SYNCH_RECURSIVE_MUTEX> TEST_I_TARGET_LISTENER_SINGLETON;

#endif
