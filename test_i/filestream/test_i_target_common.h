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

#include <list>
#include <string>

#include <ace/INET_Addr.h>
#include <ace/os_include/sys/os_socket.h>
#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>
#include <ace/Time_Value.h>

#include <gtk/gtk.h>

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "common_isubscribe.h"

#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_configuration.h"
#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_configuration.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
#include "test_i_filestream_common.h"
#include "test_i_filestream_network.h"
#include "test_i_message.h"

// forward declarations
class Stream_IAllocator;

struct Test_I_Target_ConnectionConfiguration;
struct Test_I_Target_UserData
 : Test_I_UserData
{
  inline Test_I_Target_UserData ()
   : Test_I_UserData ()
   , connectionConfiguration (NULL)
  {};

  // *TODO*: currently required by the connection handler (see:
  //         netsocketconnectionbase.inl:437)
  //         --> add to the socket handler configuration ASAP
  struct Test_I_Target_ConnectionConfiguration* connectionConfiguration;
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

  inline struct Test_I_Target_SessionData& operator+= (const struct Test_I_Target_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    size = ((size == 0) ? rhs_in.size : size);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  unsigned int                   size;
  std::string                    targetFileName;
  struct Test_I_Target_UserData* userData;
};
typedef Stream_SessionData_T<struct Test_I_Target_SessionData> Test_I_Target_SessionData_t;

struct Test_I_Target_StreamState
 : Test_I_StreamState
{
  inline Test_I_Target_StreamState ()
   : Test_I_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Target_SessionData* currentSessionData;
  struct Test_I_Target_UserData*    userData;
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

  ACE_INET_Addr                                    address;
  int                                              addressFamily;
  Test_I_Target_IInetConnectionManager_t*          connectionManager;
  Stream_IAllocator*                               messageAllocator;
  struct Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
  ACE_Time_Value                                   statisticReportingInterval; // [ACE_Time_Value::zero: off]
  bool                                             useLoopBackDevice;
};

typedef Net_IListener_T<struct Test_I_Target_ListenerConfiguration,
                        struct Test_I_Target_SocketHandlerConfiguration> Test_I_Target_IListener_t;

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

class Test_I_Target_SessionMessage;
typedef Test_I_Message_T<enum Stream_MessageType,
                         Test_I_Target_SessionMessage> Test_I_Target_Message_t;
typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_I_Target_ControlMessage_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Target_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Target_Message_t,
                                    Test_I_Target_SessionMessage> Test_I_Target_ISessionNotify_t;
typedef std::list<Test_I_Target_ISessionNotify_t*> Test_I_Target_Subscribers_t;
typedef Test_I_Target_Subscribers_t::iterator Test_I_SubscribersIterator_t;
struct Test_I_Target_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  inline Test_I_Target_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , contextID (0)
   , socketHandlerConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {};

  guint                                            contextID;
  struct Test_I_Target_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_Target_ISessionNotify_t*                  subscriber;
  Test_I_Target_Subscribers_t*                     subscribers;
};

struct Test_I_Target_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Target_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , userData (NULL)
  {};

  struct Test_I_Target_UserData* userData;
};

struct Test_I_Target_ConnectionConfiguration;
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
   , connectionConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
   , userData ()
  {};

  ACE_HANDLE                                      handle;
  //Test_I_Target_IListener_t*               listener;
  struct Test_I_Target_ListenerConfiguration      listenerConfiguration;
  struct Test_I_Target_SignalHandlerConfiguration signalHandlerConfiguration;
  struct Test_I_Target_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_Target_ConnectionConfiguration    connectionConfiguration;
  struct Test_I_Target_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_Target_StreamConfiguration        streamConfiguration;

  enum Net_TransportLayerType                     protocol;
  struct Test_I_Target_UserData                   userData;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_I_Target_ControlMessage_t,
                                          Test_I_Target_Message_t,
                                          Test_I_Target_SessionMessage> Test_I_Target_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_I_Target_ISessionNotify_t> Test_I_Target_ISubscribe_t;

//////////////////////////////////////////

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
  {};

  struct Test_I_Target_Configuration* configuration;
  Test_I_Target_Subscribers_t         subscribers;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_Target_GTK_CBData> Test_I_Target_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_I_Target_GTK_CBData> Test_I_Target_GTK_Manager_t;
typedef ACE_Singleton<Test_I_Target_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_I_TARGET_GTK_MANAGER_SINGLETON;

#endif
