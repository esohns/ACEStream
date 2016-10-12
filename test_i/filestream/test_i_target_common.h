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

#ifndef TEST_I_TARGET_COMMON_H
#define TEST_I_TARGET_COMMON_H

#include <gtk/gtk.h>

#include <ace/INET_Addr.h>
#include <ace/os_include/sys/os_socket.h>
#include <ace/Time_Value.h>

#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
#include "test_i_filestream_common.h"
#include "test_i_message.h"

struct Test_I_Target_UserData
 : Test_I_UserData
{
  inline Test_I_Target_UserData ()
   : Test_I_UserData ()
   , configuration (NULL)
  {};

  // *TODO*: currently required by the connection handler (see:
  //         netsocketconnectionbase.inl:437)
  //         --> add to the socket handler configuration ASAP
  Test_I_Target_Configuration* configuration;
};

struct Test_I_Target_SessionData
 : Test_I_SessionData
{
  inline Test_I_Target_SessionData ()
   : Test_I_SessionData ()
   , size (0)
   , targetFileName ()
   , userData (NULL)
  {};

  inline Test_I_Target_SessionData& operator+= (const Test_I_Target_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    size = ((size == 0) ? rhs_in.size : size);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  unsigned int            size;
  std::string             targetFileName;
  Test_I_Target_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_Target_SessionData> Test_I_Target_SessionData_t;

struct Test_I_Target_StreamState
 : Test_I_StreamState
{
  inline Test_I_Target_StreamState ()
   : Test_I_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_Target_SessionData* currentSessionData;
  Test_I_Target_UserData*    userData;
};

struct Test_I_Target_SocketHandlerConfiguration;
struct Test_I_Target_ListenerConfiguration
{
  inline Test_I_Target_ListenerConfiguration ()
   : address (TEST_I_DEFAULT_PORT, static_cast<ACE_UINT32> (INADDR_ANY))
   , addressFamily (ACE_ADDRESS_FAMILY_INET)
   , connectionManager (NULL)
   , messageAllocator (NULL)
   , socketHandlerConfiguration (NULL)
   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
   , useLoopBackDevice (false)
  {};

  ACE_INET_Addr                             address;
  int                                       addressFamily;
  Test_I_Target_IInetConnectionManager_t*   connectionManager;
  Stream_IAllocator*                        messageAllocator;
  Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                            statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                                      useLoopBackDevice;
};

typedef Net_IListener_T<Test_I_Target_ListenerConfiguration,
                        Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IListener_t;

struct Test_I_Target_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Target_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Test_I_Target_IListener_t*          listener;
  Test_I_StatisticReportingHandler_t* statisticReportingHandler;
  long                                statisticReportingTimerID;
};

struct Test_I_Target_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Target_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_Target_UserData* userData;
};

struct Test_I_Target_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  inline Test_I_Target_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , contextID (0)
   , socketHandlerConfiguration (NULL)
  {};

  guint                                     contextID;
  Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
};

struct Test_I_Target_Configuration
 : Test_I_Configuration
{
  inline Test_I_Target_Configuration ()
   : Test_I_Configuration ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   , socketHandlerConfiguration ()
   , moduleHandlerConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
   , userData ()
  {};

  ACE_HANDLE                               handle;
  //Test_I_Target_IListener_t*               listener;
  Test_I_Target_ListenerConfiguration      listenerConfiguration;
  Test_I_Target_SignalHandlerConfiguration signalHandlerConfiguration;
  Test_I_Target_SocketHandlerConfiguration socketHandlerConfiguration;
  Test_I_Target_ModuleHandlerConfiguration moduleHandlerConfiguration;

  Net_TransportLayerType                   protocol;
  Test_I_Target_UserData                   userData;
};

class Test_I_Target_SessionMessage;
typedef Test_I_Message_T<Test_I_Target_SessionMessage> Test_I_Target_Message_t;
typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Test_I_Target_Message_t,
                                Test_I_Target_SessionMessage> Test_I_Target_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_I_Target_ControlMessage_t,
                                          Test_I_Target_Message_t,
                                          Test_I_Target_SessionMessage> Test_I_Target_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Target_SessionData,
                                    Stream_SessionMessageType,
                                    Test_I_Target_Message_t,
                                    Test_I_Target_SessionMessage> Test_I_Target_ISessionNotify_t;
typedef std::list<Test_I_Target_ISessionNotify_t*> Test_I_Target_Subscribers_t;
typedef Test_I_Target_Subscribers_t::iterator Test_I_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_I_Target_ISessionNotify_t> Test_I_Target_ISubscribe_t;

//struct Test_I_Target_GTK_ProgressData
// : Test_I_GTK_ProgressData
//{
//  inline Test_I_Target_GTK_ProgressData ()
//   : Test_I_GTK_ProgressData ()
//   , transferred (0)
//  {};
//
//  size_t transferred; // bytes
//};

struct Test_I_Target_GTK_CBData
 : Test_I_FileStream_GTK_CBData
{
  inline Test_I_Target_GTK_CBData ()
   : Test_I_FileStream_GTK_CBData ()
   , configuration (NULL)
   , subscribers ()
   , subscribersLock ()
  {};

  Test_I_Target_Configuration* configuration;
  Test_I_Target_Subscribers_t  subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX    subscribersLock;
};

#endif
