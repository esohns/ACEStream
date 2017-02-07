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

#ifndef TEST_I_SOURCE_COMMON_H
#define TEST_I_SOURCE_COMMON_H

#include <list>

#include <ace/Singleton.h>
#include <ace/Synch_Traits.h>

#include <gtk/gtk.h>

#include "common_isubscribe.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_control_message.h"
#include "stream_base.h"
#include "stream_isessionnotify.h"

#include "net_configuration.h"

#include "test_i_configuration.h"
#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
#include "test_i_filestream_common.h"
#include "test_i_filestream_network.h"
#include "test_i_message.h"

struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_UserData
 : Stream_UserData
{
  inline Test_I_Source_UserData ()
   : Stream_UserData ()
   , connectionConfiguration (NULL)
   , streamConfiguration (NULL)
  {};

  // *TODO*: currently required by the connection handler (see:
  //         netsocketconnectionbase.inl:437)
  //         --> add to the socket handler configuration ASAP
  struct Test_I_Source_ConnectionConfiguration* connectionConfiguration;
  struct Test_I_Source_StreamConfiguration*     streamConfiguration;
};

struct Test_I_Source_SessionData
 : Test_I_SessionData
{
  inline Test_I_Source_SessionData ()
   : Test_I_SessionData ()
   , fileName ()
   , size (0)
   , targetFileName ()
   , userData (NULL)
  {};

  inline Test_I_Source_SessionData& operator+= (const Test_I_Source_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    fileName = (fileName.empty () ? rhs_in.fileName : fileName);
    size = ((size == 0) ? rhs_in.size : size);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  std::string                    fileName;
  unsigned int                   size;
  std::string                    targetFileName;
  struct Test_I_Source_UserData* userData;
};
typedef Stream_SessionData_T<struct Test_I_Source_SessionData> Test_I_Source_SessionData_t;

struct Test_I_Source_StreamState
 : Test_I_StreamState
{
  inline Test_I_Source_StreamState ()
   : Test_I_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_SessionData* currentSessionData;
  struct Test_I_Source_UserData*    userData;
};

struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Test_I_Source_ConnectionConfiguration,
                          struct Test_I_Source_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_IConnection_t;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_ModuleHandlerConfiguration;
//class Test_I_Source_SessionMessage;
//typedef Test_I_Message_T<Test_I_Source_SessionMessage> Test_I_Source_Message_t;
//typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
//                                struct Stream_AllocatorConfiguration,
//                                Test_I_Source_Message_t,
//                                Test_I_Source_SessionMessage> Test_I_Source_ControlMessage_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_StreamState,
                      struct Test_I_Source_StreamConfiguration,
                      Test_I_RuntimeStatistic_t,
                      struct Stream_ModuleConfiguration,
                      struct Test_I_Source_ModuleHandlerConfiguration,
                      struct Test_I_Source_SessionData,
                      Test_I_Source_SessionData_t,
                      Test_I_Source_ControlMessage_t,
                      Test_I_Source_Message_t,
                      Test_I_Source_SessionMessage> Test_I_StreamBase_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_Message_t,
                                    Test_I_Source_SessionMessage> Test_I_Source_ISessionNotify_t;
typedef std::list<Test_I_Source_ISessionNotify_t*> Test_I_Source_Subscribers_t;
typedef Test_I_Source_Subscribers_t::iterator Test_I_Source_SubscribersIterator_t;
struct Test_I_Source_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  inline Test_I_Source_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionManager (NULL)
   , contextID (0)
   , fileName ()
   , socketHandlerConfiguration (NULL)
   , stream (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {};

  Test_I_Source_IConnection_t*                     connection; // TCP target module
  Test_I_Source_InetConnectionManager_t*           connectionManager; // TCP target module
  guint                                            contextID;
  std::string                                      fileName; // file reader module
  struct Test_I_Source_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_StreamBase_t*                             stream;
  Test_I_Source_ISessionNotify_t*                  subscriber;
  Test_I_Source_Subscribers_t*                     subscribers;
};

struct Test_I_Source_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Source_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
   , userData (NULL)
  {};

  struct Test_I_Source_ModuleHandlerConfiguration* moduleHandlerConfiguration;

  struct Test_I_Source_UserData*                   userData;
};

struct Test_I_Source_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline Test_I_Source_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingInterval (0)
   , stream (NULL)
  {};

  unsigned int         statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_StreamBase_t* stream;
};

struct Test_I_Source_ConnectionConfiguration;
struct Test_I_Source_Configuration
 : Test_I_Configuration
{
  inline Test_I_Source_Configuration ()
   : Test_I_Configuration ()
   , signalHandlerConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
   , userData ()
  {};

  // **************************** signal data **********************************
  struct Test_I_Source_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Test_I_Source_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_Source_ConnectionConfiguration    connectionConfiguration;
  // **************************** stream data **********************************
  struct Test_I_Source_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_Source_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************

  enum Net_TransportLayerType                     protocol;
  struct Test_I_Source_UserData                   userData;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_I_Source_ControlMessage_t,
                                          Test_I_Source_Message_t,
                                          Test_I_Source_SessionMessage> Test_I_Source_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_I_Source_ISessionNotify_t> Test_I_Source_ISubscribe_t;

//////////////////////////////////////////

struct Test_I_Source_GTK_ProgressData
 : Test_I_FileStream_GTK_ProgressData
{
  inline Test_I_Source_GTK_ProgressData ()
   : Test_I_FileStream_GTK_ProgressData ()
   , size (0)
  {};

  size_t size; // bytes
};

struct Test_I_Source_GTK_CBData
 : Test_I_GTK_CBData
{
  inline Test_I_Source_GTK_CBData ()
   : Test_I_GTK_CBData ()
   , configuration (NULL)
   , loop(0)
   , progressData ()
   , stream (NULL)
   , subscribers ()
   , UDPStream(NULL)
  {};

  struct Test_I_Source_Configuration*   configuration;
  size_t                                loop;
  struct Test_I_Source_GTK_ProgressData progressData;
  Test_I_StreamBase_t*                  stream;
  Test_I_Source_Subscribers_t           subscribers;
  Test_I_StreamBase_t*                  UDPStream;
};

struct Test_I_Source_ThreadData
 : Test_I_ThreadData
{
  inline Test_I_Source_ThreadData ()
   : Test_I_ThreadData ()
   , CBData (NULL)
  {};

  struct Test_I_Source_GTK_CBData* CBData;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Test_I_Source_GTK_CBData> Test_I_Source_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_I_Source_GTK_CBData> Test_I_Source_GTK_Manager_t;
typedef ACE_Singleton<Test_I_Source_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> TEST_I_SOURCE_GTK_MANAGER_SINGLETON;

#endif
