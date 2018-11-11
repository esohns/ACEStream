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

#ifndef HTTP_GET_COMMON_H
#define HTTP_GET_COMMON_H

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "common.h"
#include "common_istatistic.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dec_defines.h"

#include "http_get_network.h"
#include "http_get_stream_common.h"

#include "test_u_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

// forward declarations
class HTTPGet_Message;
class HTTPGet_SessionMessage;

typedef Common_IStatistic_T<struct Stream_Statistic> HTTPGet_StatisticReportingHandler_t;

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Common_FlexParserAllocatorConfiguration> HTTPGet_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_FlexParserAllocatorConfiguration,
                                          HTTPGet_ControlMessage_t,
                                          HTTPGet_Message,
                                          HTTPGet_SessionMessage> HTTPGet_MessageAllocator_t;

struct HTTPGet_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  HTTPGet_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingInterval (0)
  {}

  unsigned int statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct HTTPGet_Configuration
{
  HTTPGet_Configuration ()
   : allocatorConfiguration ()
   , dispatchConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  struct Common_FlexParserAllocatorConfiguration allocatorConfiguration;
  struct Common_EventDispatchConfiguration       dispatchConfiguration;
  // **************************** signal data **********************************
  struct HTTPGet_SignalHandlerConfiguration      signalHandlerConfiguration;
  // **************************** socket data **********************************
  HTTPGet_ConnectionConfigurations_t             connectionConfigurations;
  // **************************** stream data **********************************
  struct Common_ParserConfiguration              parserConfiguration;
  HTTPGet_StreamConfiguration_t                  streamConfiguration;

  struct Stream_UserData                         userData;
};

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct HTTPGet_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#else
 : Test_U_UI_ProgressData
#endif // GTK_USE
{
  HTTPGet_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#else
   : Test_U_UI_ProgressData ()
#endif // GTK_USE
   , statistic ()
  {}

  struct Stream_Statistic statistic;
};

//static constexpr const char stream_name_string_[] =
//    ACE_TEXT_ALWAYS_CHAR ("HTTPGetStream");
//typedef Stream_Base_T<ACE_MT_SYNCH,
//                      Common_TimePolicy_t,
//                      stream_name_string_,
//                      enum Stream_ControlType,
//                      enum Stream_SessionMessageType,
//                      enum Stream_StateMachine_ControlState,
//                      struct HTTPGet_StreamState,
//                      struct HTTPGet_StreamConfiguration,
//                      struct Stream_Statistic,
//                      struct Stream_AllocatorConfiguration,
//                      struct Stream_ModuleConfiguration,
//                      struct HTTPGet_ModuleHandlerConfiguration,
//                      struct HTTPGet_SessionData,
//                      HTTPGet_SessionData_t,
//                      HTTPGet_ControlMessage_t,
//                      HTTPGet_Message,
//                      HTTPGet_SessionMessage> HTTPGet_StreamBase_t;
struct HTTPGet_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#else
 : Test_U_UI_CBData
#endif // GTK_USE
{
  HTTPGet_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#else
   : Test_U_UI_CBData ()
#endif // GTK_USE
   , configuration (NULL)
   , dispatchState ()
   , messageAllocator (NULL)
   , progressData ()
   , stream (NULL)
  {}

  struct HTTPGet_Configuration*    configuration;
  struct Common_EventDispatchState dispatchState;
  // *NOTE*: on the host ("server"), use the device bias registers instead !
  // *TODO*: implement a client->server protocol to do this
  //struct ARDrone_SensorBias clientSensorBias; // client side ONLY (!)
  HTTPGet_MessageAllocator_t*      messageAllocator;
  struct HTTPGet_ProgressData      progressData;
  Stream_IStream_t*                stream;
};

struct HTTPGet_UI_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#else
 : Test_U_UI_ThreadData
#endif // GTK_USE
{
  HTTPGet_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
#else
   : Test_U_UI_ThreadData ()
#endif // GTK_USE
   , CBData (NULL)
  {}

  struct HTTPGet_UI_CBData* CBData;
};

#if defined (GTK_USE)
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct HTTPGet_UI_CBData> HTTPGet_GtkBuilderDefinition_t;
#endif // GTK_USE
#endif // GTK_SUPPORT

#endif
