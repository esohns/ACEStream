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

#include "gtk/gtk.h"

#include "common.h"
#include "common_istatistic.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"

#include "stream_dec_defines.h"

#include "http_get_network.h"
#include "http_get_stream_common.h"

// forward declarations
class HTTPGet_Message;
class HTTPGet_SessionMessage;

typedef Common_IStatistic_T<struct Stream_Statistic> HTTPGet_StatisticReportingHandler_t;

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct HTTPGet_AllocatorConfiguration> HTTPGet_ControlMessage_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct HTTPGet_AllocatorConfiguration,
                                          HTTPGet_ControlMessage_t,
                                          HTTPGet_Message,
                                          HTTPGet_SessionMessage> HTTPGet_MessageAllocator_t;

//struct HTTPGet_ConnectionConfiguration;
//struct HTTPGet_StreamConfiguration;
//struct HTTPGet_UserData
// : Stream_UserData
//{
//  inline HTTPGet_UserData ()
//   : Stream_UserData ()
//   //, connectionConfiguration (NULL)
//   //, streamConfiguration (NULL)
//  {};
//
//  //struct HTTPGet_ConnectionConfiguration* connectionConfiguration;
//  //struct HTTPGet_StreamConfiguration*     streamConfiguration;
//};

struct HTTPGet_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  inline HTTPGet_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   //messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  //Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct HTTPGet_Configuration
{
  inline HTTPGet_Configuration ()
   : signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , streamConfiguration ()
   //, useReactor (NET_EVENT_USE_REACTOR)
   , userData ()
  {};

  // **************************** signal data **********************************
  struct HTTPGet_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  HTTPGet_ConnectionConfigurations_t        connectionConfigurations;
  // **************************** stream data **********************************
  struct Common_ParserConfiguration         parserConfiguration;
  HTTPGet_StreamConfiguration_t             streamConfiguration;

  struct Stream_UserData                    userData;
};

//////////////////////////////////////////

struct HTTPGet_GtkProgressData
 : Common_UI_GTK_ProgressData
{
  inline HTTPGet_GtkProgressData ()
   : Common_UI_GTK_ProgressData ()
   , statistic ()
  {};

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
struct HTTPGet_GtkCBData
 : Common_UI_GTKState
{
 inline HTTPGet_GtkCBData ()
  : Common_UI_GTKState ()
  , configuration (NULL)
  , messageAllocator (NULL)
  , stream (NULL)
  , progressData (NULL)
 {};

 struct HTTPGet_Configuration*        configuration;
 // *NOTE*: on the host ("server"), use the device bias registers instead !
 // *TODO*: implement a client->server protocol to do this
 //struct ARDrone_SensorBias clientSensorBias; // client side ONLY (!)
 HTTPGet_MessageAllocator_t*          messageAllocator;
 Stream_IStream_t*                    stream;
 struct HTTPGet_GtkProgressData*      progressData;
};

struct HTTPGet_ThreadData
{
  inline HTTPGet_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  struct HTTPGet_GtkCBData* CBData;
  guint                     eventSourceID;
};

typedef Common_UI_GtkBuilderDefinition_T<struct HTTPGet_GtkCBData> HTTPGet_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct HTTPGet_GtkCBData> HTTPGet_GTK_Manager_t;
typedef ACE_Singleton<HTTPGet_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> HTTPGET_UI_GTK_MANAGER_SINGLETON;

#endif
