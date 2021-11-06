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
#include "common_file_common.h"

#include "common_parser_common.h"

#include "stream_common.h"
#include "stream_configuration.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_istreamcontrol.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_lib_common.h"
#include "stream_lib_defines.h"

#include "test_i_common.h"
#include "test_i_defines.h"

// forward declarations
class Stream_IStreamControlBase;

struct Test_I_ModuleHandlerConfiguration
 : virtual Stream_ModuleHandlerConfiguration
{
  Test_I_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , printProgressDot (false)
   , fileIdentifier ()
  {}

  bool                   printProgressDot;
  Common_File_Identifier fileIdentifier; // source-/target-
};

struct Test_I_StreamConfiguration
 : Stream_Configuration
{
  Test_I_StreamConfiguration ()
   : Stream_Configuration ()
   , fileIdentifier ()
  {}

  Common_File_Identifier fileIdentifier; // target-
};

struct Test_I_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_SignalHandlerConfiguration ()
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

//struct Test_I_StreamConfiguration
// : Stream_Configuration
//{
//  Test_I_StreamConfiguration ()
//   : Stream_Configuration ()
//  {}
//};

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
   : /*allocatorConfiguration ()
   ,*/ dispatchConfiguration ()
   , signalHandlerConfiguration ()
   , parserConfiguration ()
   //, streamConfiguration ()
   , userData ()
  {}

//  struct Test_I_AllocatorConfiguration     allocatorConfiguration;
  struct Common_EventDispatchConfiguration   dispatchConfiguration;
  // **************************** signal data **********************************
  struct Test_I_SignalHandlerConfiguration   signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Common_FlexBisonParserConfiguration parserConfiguration;
  //struct Stream_Configuration              streamConfiguration;

  ////////////////////////////////////////

  struct Stream_UserData                     userData;
};

#endif
