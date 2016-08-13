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

#ifndef TEST_I_COMMON_H
#define TEST_I_COMMON_H

#include <algorithm>
#include <deque>
#include <limits>
#include <map>
#include <string>

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_inotify.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_inotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"
#include "stream_statemachine_control.h"

#include "net_configuration.h"
#include "net_defines.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;
struct Test_I_ConnectionState;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

typedef Stream_Statistic Test_I_RuntimeStatistic_t;

typedef Common_IStatistic_T<Test_I_RuntimeStatistic_t> Test_I_StatisticReportingHandler_t;

struct Test_I_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_I_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = NET_PROTOCOL_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

struct Test_I_Configuration;
struct Test_I_StreamConfiguration;
struct Test_I_UserData
 : Stream_UserData
{
  inline Test_I_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
  {};

  Test_I_Configuration*       configuration;
  Test_I_StreamConfiguration* streamConfiguration;
};

struct Test_I_SessionData
 : Stream_SessionData
{
  inline Test_I_SessionData ()
   : Stream_SessionData ()
   , connectionState (NULL)
   , userData (NULL)
  {};
  inline Test_I_SessionData& operator+= (const Test_I_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionData::operator+= (rhs_in);

    connectionState = (connectionState ? connectionState : rhs_in.connectionState);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  Test_I_ConnectionState* connectionState;
  Test_I_UserData*        userData;
};
typedef Stream_SessionData_T<Test_I_SessionData> Test_I_SessionData_t;

struct Test_I_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ///////////////////////////////////////
   , userData (NULL)
  {};

  Test_I_UserData* userData;
};

// forward declarations
struct Test_I_Configuration;
//typedef Stream_Base_T<ACE_MT_SYNCH,
//                      ACE_MT_SYNCH,
//                      Common_TimePolicy_t,
//                      int,
//                      Stream_SessionMessageType,
//                      Stream_StateMachine_ControlState,
//                      Test_I_Stream_State,
//                      Test_I_Stream_Configuration,
//                      Test_I_RuntimeStatistic_t,
//                      Stream_ModuleConfiguration,
//                      Test_I_Stream_ModuleHandlerConfiguration,
//                      Test_I_Stream_SessionData,   // session data
//                      Test_I_Stream_SessionData_t, // session data container (reference counted)
//                      ACE_Message_Block,
//                      Test_I_Stream_Message,
//                      Test_I_Stream_SessionMessage> Test_I_StreamBase_t;
struct Test_I_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Test_I_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , configuration (NULL)
   , inbound (false)
   , printFinalReport (true)
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , targetFileName ()
  {
    traceParsing = NET_PROTOCOL_DEFAULT_YACC_TRACE; // HTTP parser module
    traceScanning = NET_PROTOCOL_DEFAULT_LEX_TRACE; // HTTP parser module
  };

  Test_I_Configuration*              configuration;
  bool                               inbound; // IO module
  bool                               printFinalReport;
  bool                               printProgressDot; // file writer module
  bool                               pushStatisticMessages;
  Net_SocketConfiguration*           socketConfiguration;
  Test_I_SocketHandlerConfiguration* socketHandlerConfiguration;
  std::string                        targetFileName; // file writer module
};

struct Test_I_SignalHandlerConfiguration
{
  inline Test_I_SignalHandlerConfiguration ()
   : //messageAllocator (NULL)
   /*,*/ statisticReportingInterval (ACE_Time_Value::zero)
   , useReactor (true)
  {};

  //Stream_IAllocator* messageAllocator;
  ACE_Time_Value     statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  bool               useReactor;
};

struct Test_I_StreamConfiguration
 : Stream_Configuration
{
  inline Test_I_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_I_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Test_I_StreamState
 : Stream_State
{
  inline Test_I_StreamState ()
   : Stream_State ()
   , currentSessionData (NULL)
   , userData (NULL)
  {};

  Test_I_SessionData* currentSessionData;
  Test_I_UserData*    userData;
};

struct Test_I_Configuration
{
  inline Test_I_Configuration ()
    : allocatorConfiguration ()
    , signalHandlerConfiguration ()
    , socketConfiguration ()
    , socketHandlerConfiguration ()
    , moduleConfiguration ()
    , streamConfiguration ()
    , userData ()
    , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // ***************************** allocator ***********************************
  Stream_AllocatorConfiguration     allocatorConfiguration;
  // **************************** signal data **********************************
  Test_I_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_SocketConfiguration           socketConfiguration;
  Test_I_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_ModuleConfiguration        moduleConfiguration;
  Test_I_StreamConfiguration        streamConfiguration;

  Test_I_UserData                   userData;
  bool                              useReactor;
};

typedef Stream_INotify_T<Stream_SessionMessageType> Test_I_IStreamNotify_t;

#endif
