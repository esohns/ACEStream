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

#include "stream_dec_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "net_configuration.h"

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
    paddingBytes = STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE;
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
  {};

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
 : Common_SignalHandlerConfiguration
{
  inline Test_I_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingInterval (ACE_Time_Value::zero)
   , statisticReportingTimerID (-1)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (COMMON_DEFAULT_WIN32_MEDIA_FRAMEWORK == COMMON_WIN32_FRAMEWORK_MEDIAFOUNDATION)
#endif
  {};

  ACE_Time_Value statisticReportingInterval; // statistic reporting interval (second(s)) [0: off]
  long           statisticReportingTimerID;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool           useMediaFoundation;
#endif
};

struct Test_I_StreamConfiguration
 : Stream_Configuration
{
  inline Test_I_StreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {};

  struct Test_I_UserData* userData;
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
   , parserConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , useReactor (NET_EVENT_USE_REACTOR)
   , userData ()
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
  struct Common_ParserConfiguration        parserConfiguration;
  struct Stream_ModuleConfiguration        moduleConfiguration;
  struct Test_I_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_I_StreamConfiguration        streamConfiguration;

  bool                                     useReactor;

  struct Test_I_UserData                   userData;
};

#endif
