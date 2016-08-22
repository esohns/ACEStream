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

#include "gtk/gtk.h"

#include "stream_control_message.h"

#include "test_i_connection_manager_common.h"
#include "test_i_defines.h"
#include "test_i_filestream_common.h"
#include "test_i_message.h"

struct Test_I_Source_Configuration;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_UserData
 : Stream_UserData
{
  inline Test_I_Source_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  // *TODO*: currently required by the connection handler (see:
  //         netsocketconnectionbase.inl:437)
  //         --> add to the socket handler configuration ASAP
  Test_I_Source_Configuration*       configuration;
  Test_I_Source_StreamConfiguration* streamConfiguration;
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

  std::string             fileName;
  unsigned int            size;
  std::string             targetFileName;
  Test_I_Source_UserData* userData;
};
typedef Stream_SessionData_T<Test_I_Source_SessionData> Test_I_Source_SessionData_t;

struct Test_I_Source_StreamState
 : Test_I_StreamState
{
  inline Test_I_Source_StreamState ()
   : Test_I_StreamState ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_Source_SessionData* currentSessionData;
  Test_I_Source_UserData*    userData;
};

struct Test_I_Source_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , connectionManager (NULL)
   , userData (NULL)
  {};

  Test_I_Source_InetConnectionManager_t* connectionManager; // TCP IO module

  Test_I_Source_UserData*                userData;
};

struct Test_I_Source_Configuration;
struct Test_I_Source_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          Test_I_Source_Configuration,
                          Test_I_Source_ConnectionState,
                          Test_I_RuntimeStatistic_t> Test_I_Source_IConnection_t;
struct Test_I_Source_StreamConfiguration;
struct Test_I_Source_ModuleHandlerConfiguration;
class Test_I_Source_SessionMessage;
typedef Test_I_Message_T<Test_I_Source_SessionMessage> Test_I_Source_Message_t;
typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Test_I_Source_Message_t,
                                Test_I_Source_SessionMessage> Test_I_Source_ControlMessage_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      int,
                      Stream_SessionMessageType,
                      Stream_StateMachine_ControlState,
                      Test_I_Source_StreamState,
                      Test_I_Source_StreamConfiguration,
                      Test_I_RuntimeStatistic_t,
                      Stream_ModuleConfiguration,
                      Test_I_Source_ModuleHandlerConfiguration,
                      Test_I_Source_SessionData,   // session data
                      Test_I_Source_SessionData_t, // session data container (reference counted)
                      Test_I_Source_ControlMessage_t,
                      Test_I_Source_Message_t,
                      Test_I_Source_SessionMessage> Test_I_StreamBase_t;
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
  {};

  Test_I_Source_IConnection_t*              connection; // TCP target module
  Test_I_Source_InetConnectionManager_t*    connectionManager; // TCP target module
  guint                                     contextID;
  std::string                               fileName; // file reader module
  Test_I_Source_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_StreamBase_t*                      stream;
};

struct Test_I_Source_StreamConfiguration
 : Test_I_StreamConfiguration
{
  inline Test_I_Source_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_I_Source_ModuleHandlerConfiguration* moduleHandlerConfiguration;
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

struct Test_I_Source_Configuration
 : Test_I_Configuration
{
  inline Test_I_Source_Configuration ()
   : Test_I_Configuration ()
   , signalHandlerConfiguration ()
   , socketHandlerConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
   , userData ()
  {};

  // **************************** signal data **********************************
  Test_I_Source_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Source_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Test_I_Source_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_Source_StreamConfiguration        streamConfiguration;
  // *************************** protocol data *********************************

  Net_TransportLayerType                   protocol;
  Test_I_Source_UserData                   userData;
};

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_I_Source_ControlMessage_t,
                                          Test_I_Source_Message_t,
                                          Test_I_Source_SessionMessage> Test_I_Source_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_I_Source_SessionData,
                                    Stream_SessionMessageType,
                                    Test_I_Source_Message_t,
                                    Test_I_Source_SessionMessage> Test_I_Source_ISessionNotify_t;
typedef std::list<Test_I_Source_ISessionNotify_t*> Test_I_Source_Subscribers_t;
typedef Test_I_Source_Subscribers_t::iterator Test_I_Source_SubscribersIterator_t;

typedef Common_ISubscribe_T<Test_I_Source_ISessionNotify_t> Test_I_Source_ISubscribe_t;

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
   , UDPStream(NULL)
  {};

  Test_I_Source_Configuration*   configuration;
  size_t                         loop;
  Test_I_Source_GTK_ProgressData progressData;
  Test_I_StreamBase_t*           stream;
  Test_I_Source_Subscribers_t    subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX      subscribersLock;
  Test_I_StreamBase_t*           UDPStream;
};

struct Test_I_Source_ThreadData
 : Test_I_ThreadData
{
  inline Test_I_Source_ThreadData ()
   : Test_I_ThreadData ()
   , CBData (NULL)
  {};

  Test_I_Source_GTK_CBData* CBData;
};

#endif
