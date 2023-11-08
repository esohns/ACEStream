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

#ifndef TEST_U_CONFIGURATION_H
#define TEST_U_CONFIGURATION_H

#include <string>

#include "ace/config-lite.h"
#include "ace/Time_Value.h"

#include "common_file_common.h"

#include "common_event_common.h"

#include "common_parser_common.h"

#include "common_timer_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_istreamcontrol.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_dev_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_mediafoundation_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_defines.h"

struct Test_U_ModuleHandlerConfiguration
 : virtual Stream_ModuleHandlerConfiguration
{
  Test_U_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , fileIdentifier ()
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//   , manageCOM (false)
//#endif // ACE_WIN32 || ACE_WIN64
   , printProgressDot (false)
   , pushStatisticMessages (true)
  {}

  struct Common_File_Identifier fileIdentifier; // source-/target-
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  bool                          manageCOM;
//#endif // ACE_WIN32 || ACE_WIN64
  bool                          printProgressDot;
  bool                          pushStatisticMessages; // statistic module
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , builder (NULL)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , sampleIsDataMessage (false)
  {}

  IGraphBuilder*                                               builder;
  struct Stream_MediaFramework_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                        filterCLSID;
  // *IMPORTANT NOTE*: 'asynchronous' filters implement IAsyncReader (downstream
  //                   filters 'pull' media samples), 'synchronous' filters
  //                   implement IMemInputPin and 'push' media samples to
  //                   downstream filters
  bool                                                         push;
  bool                                                         sampleIsDataMessage;
};

struct Test_U_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , manageMediaSession (false)
   , mediaFoundationConfiguration (NULL)
   , session (NULL)
  {}

  bool                                                        manageMediaSession;
  struct Stream_MediaFramework_MediaFoundation_Configuration* mediaFoundationConfiguration;
  IMFMediaSession*                                            session;
};
#else
struct Test_U_ALSA_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_ALSA_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , ALSAConfiguration (NULL)
   , manageSoX (false)
  {
    deviceIdentifier.identifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME);
  }

  struct Stream_MediaFramework_ALSA_Configuration* ALSAConfiguration;
  struct Stream_Device_Identifier                  deviceIdentifier;
  bool                                             manageSoX;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_StreamConfiguration
 : Stream_Configuration
{
  Test_U_StreamConfiguration ()
   : Stream_Configuration ()
   , fileIdentifier ()
  {}

  struct Common_File_Identifier fileIdentifier; // target-
};

struct Test_U_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_U_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , mediaFramework (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
#endif // ACE_WIN32 || ACE_WIN64
   , statisticReportingInterval (ACE_Time_Value::zero)
   , statisticReportingTimerId (-1)
   , stream (NULL)
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  enum Stream_MediaFramework_Type mediaFramework;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_Time_Value                  statisticReportingInterval; // statistic reporting interval (second(s)) [0: off]
  long                            statisticReportingTimerId;
  Stream_IStreamControlBase*      stream;
};

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//struct Test_U_MediaFoundationConfiguration
// : Stream_MediaFramework_MediaFoundation_Configuration
//{
//  Test_U_MediaFoundationConfiguration ()
//   : Stream_MediaFramework_MediaFoundation_Configuration ()
//  {}
//};
//#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_Configuration
{
  Test_U_Configuration ()
   : dispatchConfiguration ()
   , parserConfiguration ()
   , timerConfiguration ()
   , signalHandlerConfiguration ()
   , allocatorConfiguration ()
   , userData ()
  {}

  // **************************** event data **********************************
  struct Common_EventDispatchConfiguration   dispatchConfiguration;
  // **************************** parser data **********************************
  struct Common_FlexBisonParserConfiguration parserConfiguration;
  // **************************** timer data **********************************
  struct Common_TimerConfiguration           timerConfiguration;
  // **************************** signal data **********************************
  struct Test_U_SignalHandlerConfiguration   signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Stream_AllocatorConfiguration       allocatorConfiguration;

  ////////////////////////////////////////

  struct Stream_UserData                     userData;
};

#endif
