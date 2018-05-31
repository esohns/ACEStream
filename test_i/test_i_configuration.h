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

#include "ace/config-lite.h"
#include "ace/Time_Value.h"

#include "common.h"
#include "common_configuration.h"

#include "stream_common.h"
#include "stream_configuration.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_istreamcontrol.h"
#endif

#include "stream_dec_defines.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"

#include "net_configuration.h"

#include "test_i_common.h"
#include "test_i_connection_common.h"
#include "test_i_defines.h"

struct Test_I_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  Test_I_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    defaultBufferSize = TEST_I_DEFAULT_BUFFER_SIZE;
  }
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
  Test_I_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , configuration (NULL)
   //, connectionConfigurations (NULL)
   , inbound (false)
   , printProgressDot (false)
   , pushStatisticMessages (true)
   , targetFileName ()
  {}

  struct Test_I_Configuration*       configuration;
  //Test_I_ConnectionConfigurations_t* connectionConfigurations;
  bool                               inbound; // statistic/IO module
  bool                               printProgressDot; // file writer module
  bool                               pushStatisticMessages; // statistic module
  std::string                        targetFileName; // file writer module
};
//typedef std::map<std::string,
//                 struct Test_I_ModuleHandlerConfiguration> Test_I_ModuleHandlerConfigurations_t;
//typedef Test_I_ModuleHandlerConfigurations_t::iterator Test_I_ModuleHandlerConfigurationsIterator_t;

struct Test_I_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , statisticReportingInterval (ACE_Time_Value::zero)
   , statisticReportingTimerId (-1)
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Time_Value                  statisticReportingInterval; // statistic reporting interval (second(s)) [0: off]
  long                            statisticReportingTimerId;
};

struct Test_I_StreamConfiguration
 : Stream_Configuration
{
  Test_I_StreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {}

  struct Test_I_UserData* userData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IMFMediaSession;
struct Test_I_MediaFoundationConfiguration
{
  Test_I_MediaFoundationConfiguration ()
   : controller (NULL)
   , mediaSession (NULL)
  {}

  Stream_IStreamControlBase* controller;
  IMFMediaSession*           mediaSession;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_Configuration
{
  Test_I_Configuration ()
   : allocatorConfiguration ()
   , dispatchConfiguration ()
   , signalHandlerConfiguration ()
   //, connectionConfigurations ()
   , parserConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  struct Test_I_AllocatorConfiguration     allocatorConfiguration;
  struct Common_EventDispatchConfiguration dispatchConfiguration;
  // **************************** signal data **********************************
  struct Test_I_SignalHandlerConfiguration signalHandlerConfiguration;
  // ************************** connection data ********************************
  //Test_I_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** stream data **********************************
  struct Common_ParserConfiguration        parserConfiguration;
  struct Test_I_StreamConfiguration        streamConfiguration;

  struct Test_I_UserData                   userData;
};

#endif
