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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Synch_Traits.h"

#include "common.h"
#include "common_istatistic.h"

#include "common_parser_common.h"

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

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#else
 : Test_U_Configuration
#endif // GUI_SUPPORT
{
  HTTPGet_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#else
   : Test_U_Configuration ()
#endif // GUI_SUPPORT
   , allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , parserConfiguration ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
  {}

  struct Common_Parser_FlexAllocatorConfiguration allocatorConfiguration;
  // **************************** signal data **********************************
  struct HTTPGet_SignalHandlerConfiguration       signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                  connectionConfigurations;
  // **************************** stream data **********************************
  struct HTTP_ParserConfiguration                 parserConfiguration;
  HTTPGet_StreamConfiguration_t                   streamConfiguration;
  HTTPGet_StreamConfiguration_t                   streamConfiguration_2; // net-
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
#endif // GTK_SUPPORT

#endif
