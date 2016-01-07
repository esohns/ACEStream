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

#include "ace/INET_Addr.h"
#include "ace/os_include/sys/os_socket.h"
#include "ace/Time_Value.h"

#include "stream_dev_defines.h"

#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_common.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"

struct Test_I_Target_Configuration;
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_UserData
 : Stream_UserData
{
  inline Test_I_Target_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Target_Configuration*       configuration;
  Test_I_Target_StreamConfiguration* streamConfiguration;
};

struct Test_I_Target_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Target_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_Target_UserData* userData;
};

struct Test_I_Target_Stream_ModuleHandlerConfiguration
 : Test_I_Stream_ModuleHandlerConfiguration
{
  inline Test_I_Target_Stream_ModuleHandlerConfiguration ()
   : Test_I_Stream_ModuleHandlerConfiguration ()
   , area ()
   , connection (NULL)
   , connectionManager (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , sourceFilter (TEST_I_STREAM_MODULE_DIRECTSHOW_SOURCE_FILTER_NAME)
   , windowController (NULL)
#endif
   , printProgressDot (false)
   , targetFileName ()
   , window (NULL)
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct tagRECT                         area;
  LPCWSTR                                sourceFilter;
  IVideoWindow*                          windowController;
#else
  GdkRectangle                           area;
#endif
  Test_I_Target_IConnection_t*           connection; // TCP target/IO module
  Test_I_Target_InetConnectionManager_t* connectionManager; // TCP IO module
  bool                                   printProgressDot;
  std::string                            targetFileName; // file writer module
  HWND                                   window; // *TODO*
};

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
 : Stream_SignalHandlerConfiguration
{
  inline Test_I_Target_SignalHandlerConfiguration ()
   : Stream_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerID (-1)
  {};

  Test_I_Target_IListener_t*          listener;
  Test_I_StatisticReportingHandler_t* statisticReportingHandler;
  long                                statisticReportingTimerID;
};

struct Test_I_Target_Stream_SessionData
 : Test_I_Stream_SessionData
{
  inline Test_I_Target_Stream_SessionData ()
   : Test_I_Stream_SessionData ()
   , connectionState (NULL)
   , targetFileName ()
   , userData (NULL)
  {};
  inline Test_I_Target_Stream_SessionData& operator+= (Test_I_Target_Stream_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data...
    Test_I_Stream_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_Target_ConnectionState* connectionState;
  std::string                    targetFileName;
  Test_I_Target_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_Target_Stream_SessionData> Test_I_Target_Stream_SessionData_t;

struct Test_I_Target_StreamConfiguration
 : Stream_Configuration
{
  inline Test_I_Target_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_I_Target_Stream_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Test_I_Target_StreamState
  : Stream_State
{
  inline Test_I_Target_StreamState ()
    : Stream_State ()
    , currentSessionData (NULL)
    , userData (NULL)
  {};

  Test_I_Target_Stream_SessionData* currentSessionData;
  Test_I_Target_UserData*           userData;
};

struct Test_I_Target_Configuration
 : Test_I_Configuration
{
  inline Test_I_Target_Configuration ()
   : Test_I_Configuration ()
   , socketHandlerConfiguration ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
   , listenerConfiguration ()
   , signalHandlerConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** socket data **********************************
  Test_I_Target_SocketHandlerConfiguration        socketHandlerConfiguration;
  // **************************** listener data ********************************
  ACE_HANDLE                                      handle;
  //Test_I_Target_IListener_t*               listener;
  Test_I_Target_ListenerConfiguration             listenerConfiguration;
  // **************************** signal data **********************************
  Test_I_Target_SignalHandlerConfiguration        signalHandlerConfiguration;
  // **************************** stream data **********************************
  Test_I_Target_Stream_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_Target_StreamConfiguration               streamConfiguration;

  Test_I_Target_UserData                          userData;
};

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,

                                          Test_I_Stream_Message,
                                          Test_I_Target_Stream_SessionMessage> Test_I_Target_MessageAllocator_t;

typedef Common_INotify_T<Test_I_Target_Stream_SessionData,
                         Test_I_Stream_Message,
                         Test_I_Target_Stream_SessionMessage> Test_I_Target_IStreamNotify_t;
typedef std::list<Test_I_Target_IStreamNotify_t*> Test_I_Target_Subscribers_t;
typedef Test_I_Target_Subscribers_t::iterator Test_I_Target_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_I_Target_IStreamNotify_t> Test_I_Target_ISubscribe_t;

struct Test_I_Target_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_Target_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , progressEventSourceID (0)
   , subscribers ()
  {};

  Test_I_Target_Configuration* configuration;
  guint                        progressEventSourceID;
  Test_I_Target_Subscribers_t  subscribers;
};

#endif
