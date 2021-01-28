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

//#include <list>
//#include <string>

//#include "ace/os_include/sys/os_socket.h"
//#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "common_isubscribe.h"

#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_session_data.h"

#include "net_common.h"
#include "net_configuration.h"
#include "net_defines.h"
#include "net_ilistener.h"

#include "test_i_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_i_connection_manager_common.h"
#include "test_i_filestream_defines.h"
#include "test_i_filestream_network.h"
#include "test_i_message.h"

// forward declarations
class Stream_IAllocator;

struct Test_I_Target_SessionData
 : Test_I_SessionData
{
  Test_I_Target_SessionData ()
   : Test_I_SessionData ()
   , connection (NULL)
   , size (0)
   , targetFileName ()
  {}

  struct Test_I_Target_SessionData& operator+= (const struct Test_I_Target_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    connection = ((connection == NULL) ? rhs_in.connection : connection);
    size = ((size == 0) ? rhs_in.size : size);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  Net_IINETConnection_t* connection;
  unsigned int           size;
  std::string            targetFileName;
};
typedef Stream_SessionData_T<struct Test_I_Target_SessionData> Test_I_Target_SessionData_t;

struct Test_I_Target_StreamState
 : Test_I_StreamState
{
  Test_I_Target_StreamState ()
   : Test_I_StreamState ()
   , sessionData (NULL)
  {}

  struct Test_I_Target_SessionData* sessionData;
};

//struct Test_I_Target_ListenerConfiguration
// : Net_ListenerConfiguration_T<Test_I_Target_TCPConnectionConfiguration_t,
//                               NET_TRANSPORTLAYER_TCP>
//{
//  Test_I_Target_ListenerConfiguration ()
//   : Net_ListenerConfiguration_T ()
//   , connectionConfiguration (NULL)
//   , connectionManager (NULL)
//   , statisticReportingInterval (NET_STREAM_DEFAULT_STATISTIC_REPORTING_INTERVAL, 0)
//  {}

//  Test_I_Target_TCPConnectionConfiguration_t* connectionConfiguration;
//  Test_I_Target_ITCPConnectionManager_t*      connectionManager;
//  ACE_Time_Value                              statisticReportingInterval; // [ACE_Time_Value::zero: off]
//};

typedef Net_IListener_T<Test_I_Target_TCPConnectionConfiguration_t> Test_I_Target_IListener_t;

struct Test_I_Target_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_Target_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , listener (NULL)
   , statisticReportingHandler (NULL)
   , statisticReportingTimerId (-1)
  {}

  Test_I_Target_IListener_t*     listener;
  Net_IStreamStatisticHandler_t* statisticReportingHandler;
  long                           statisticReportingTimerId;
};

//extern const char stream_name_string_[];
struct Test_I_Target_StreamConfiguration;
struct Test_I_Target_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_Target_StreamConfiguration,
                               struct Test_I_Target_ModuleHandlerConfiguration> Test_I_Target_StreamConfiguration_t;
class Test_I_Target_SessionMessage;
typedef Test_I_Message_T<enum Stream_MessageType,
                         Test_I_Target_SessionMessage> Test_I_Target_Message_t;
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
  Test_I_Target_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , connectionConfigurations (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  Net_ConnectionConfigurations_t*      connectionConfigurations;
  Test_I_Target_StreamConfiguration_t* streamConfiguration;
  Test_I_Target_ISessionNotify_t*      subscriber;
  Test_I_Target_Subscribers_t*         subscribers;
};

struct Test_I_Target_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Target_StreamConfiguration ()
   : Stream_Configuration ()
  {}
};

struct Test_I_Target_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  Test_I_Target_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , connectionConfigurations ()
   , handle (ACE_INVALID_HANDLE)
   //, listener (NULL)
//   , listenerConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
   , signalHandlerConfiguration ()
   , streamConfiguration ()
  {}

  Net_ConnectionConfigurations_t                  connectionConfigurations;
  ACE_HANDLE                                      handle;
  //Test_I_Target_IListener_t*               listener;
//  struct Test_I_Target_ListenerConfiguration      listenerConfiguration;
  enum Net_TransportLayerType                     protocol;
  struct Test_I_Target_SignalHandlerConfiguration signalHandlerConfiguration;
  Test_I_Target_StreamConfiguration_t             streamConfiguration;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Target_Message_t,
                                          Test_I_Target_SessionMessage> Test_I_Target_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_I_Target_ISessionNotify_t> Test_I_Target_ISubscribe_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
//struct Test_I_Target_GTK_ProgressData
// : Test_I_GTK_ProgressData
//{
//  Test_I_Target_GTK_ProgressData ()
//   : Test_I_GTK_ProgressData ()
//   , transferred (0)
//  {}
//
//  size_t transferred; // bytes
//};

struct Test_I_Target_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#else
 : Test_I_UI_CBData
#endif // GTK_USE
{
  Test_I_Target_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#else
   : Test_I_UI_CBData ()
#endif // GTK_USE
   , configuration (NULL)
   , subscribers ()
  {}

  struct Test_I_Target_Configuration* configuration;
  Test_I_Target_Subscribers_t         subscribers;
};
#endif // GUI_SUPPORT

#endif
