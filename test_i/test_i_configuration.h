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

#ifndef TEST_I_CONFIGURATION_H
#define TEST_I_CONFIGURATION_H

#include <string>

#include <ace/Time_Value.h>

#include "stream_common.h"

#include "net_configuration.h"
#include "net_defines.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"

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
//                      Test_I_Stream_SessionData,
//                      Test_I_Stream_SessionData_t,
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
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , socketConfiguration (NULL)
   , socketHandlerConfiguration (NULL)
   , targetFileName ()
  {
    traceParsing = NET_PROTOCOL_DEFAULT_YACC_TRACE; // parser module
    traceScanning = NET_PROTOCOL_DEFAULT_LEX_TRACE; // parser module
  };

  struct Test_I_Configuration*              configuration;
  bool                                      inbound; // statistic/IO module
  bool                                      printProgressDot; // file writer module
  bool                                      pushStatisticMessages; // statistic module
  struct Net_SocketConfiguration*           socketConfiguration;
  struct Test_I_SocketHandlerConfiguration* socketHandlerConfiguration;
  std::string                               targetFileName; // file writer module
};

//struct Test_I_SocketHandlerConfiguration
// : Net_SocketHandlerConfiguration
//{
//  inline Test_I_SocketHandlerConfiguration ()
//   : Net_SocketHandlerConfiguration ()
//   ///////////////////////////////////////
//   , userData (NULL)
//  {};

//  struct Test_I_UserData* userData;
//};

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

  struct Test_I_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaSession;
struct Test_I_MediaFoundationConfiguration
{
  inline Test_I_MediaFoundationConfiguration ()
   : controller (NULL)
   , mediaSession (NULL)
  {};

  Common_ITaskControl_t* controller;
  IMFMediaSession*       mediaSession;
};
#endif

struct Test_I_Configuration
{
  inline Test_I_Configuration ()
   : allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , socketConfiguration ()
   , socketHandlerConfiguration ()
   , connectionConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
   , useReactor (NET_EVENT_USE_REACTOR)
  {};

  // ***************************** allocator ***********************************
  struct Stream_AllocatorConfiguration     allocatorConfiguration;
  // **************************** signal data **********************************
  struct Test_I_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  struct Net_SocketConfiguration           socketConfiguration;
  struct Test_I_SocketHandlerConfiguration socketHandlerConfiguration;
  struct Test_I_ConnectionConfiguration    connectionConfiguration;
  // **************************** stream data **********************************
  struct Stream_ModuleConfiguration        moduleConfiguration;
  struct Test_I_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_StreamConfiguration        streamConfiguration;

  struct Test_I_UserData                   userData;
  bool                                     useReactor;
};

#endif
