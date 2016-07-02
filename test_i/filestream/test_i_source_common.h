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

#include "test_i_common.h"

struct Test_I_Source_SocketHandlerConfiguration
 : Net_SocketHandlerConfiguration
{
  inline Test_I_Source_SocketHandlerConfiguration ()
   : Net_SocketHandlerConfiguration ()
   ////////////////////////////////////
   , connectionManager (NULL)
   , userData (NULL)
  {};

  Test_I_Source_InetConnectionManager_t* connectionManager; // TCP IO module

  Test_I_Source_UserData*                userData;
};

struct Test_I_Source_Stream_Configuration;
struct Test_I_Source_ModuleHandlerConfiguration;
typedef Stream_Base_T<ACE_SYNCH_MUTEX,
                      ////////////////////
                      ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      ////////////////////
                      Stream_StateMachine_ControlState,
                      Test_I_Stream_State,
                      ////////////////////
                      Test_I_Source_Stream_Configuration,
                      ////////////////////
                      Test_I_RuntimeStatistic_t,
                      ////////////////////
                      Stream_ModuleConfiguration,
                      Test_I_Source_ModuleHandlerConfiguration,
                      ////////////////////
                      Test_I_Stream_SessionData,   // session data
                      Test_I_Stream_SessionData_t, // session data container (reference counted)
                      Test_I_Stream_SessionMessage,
                      Test_I_Stream_Message> Test_I_StreamBase_t;
struct Test_I_Source_ModuleHandlerConfiguration
 : Test_I_Stream_ModuleHandlerConfiguration
{
  inline Test_I_Source_ModuleHandlerConfiguration ()
   : Test_I_Stream_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionManager (NULL)
   , socketHandlerConfiguration (NULL)
   , stream (NULL)
  {};

  Test_I_Source_IConnection_t*              connection; // TCP target module
  Test_I_Source_InetConnectionManager_t*    connectionManager; // TCP target module
  Test_I_Source_SocketHandlerConfiguration* socketHandlerConfiguration;
  Test_I_StreamBase_t*                      stream;
};

struct Test_I_Source_Stream_Configuration
 : Test_I_Stream_Configuration
{
  inline Test_I_Source_Stream_Configuration ()
   : Test_I_Stream_Configuration ()
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

struct Test_I_Source_Configuration;
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
  Test_I_Source_Configuration*        configuration;
  Test_I_Source_Stream_Configuration* streamConfiguration;
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
  {};

  // **************************** signal data **********************************
  Test_I_Source_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Test_I_Source_SocketHandlerConfiguration socketHandlerConfiguration;
  // **************************** stream data **********************************
  Test_I_Source_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_I_Source_Stream_Configuration       streamConfiguration;
  // *************************** protocol data *********************************

  Test_I_Source_UserData                   userData;
};

struct Test_I_Source_GTK_CBData
 : Stream_GTK_CBData
{
  inline Test_I_Source_GTK_CBData ()
   : Stream_GTK_CBData ()
   , configuration (NULL)
   , loop(0)
   , stream (NULL)
   , UDPStream(NULL)
  {};

  Test_I_Source_Configuration* configuration;
  size_t                       loop;
  Test_I_StreamBase_t*         stream;
  Test_I_StreamBase_t*         UDPStream;
};

struct Test_I_Source_ThreadData
{
  inline Test_I_Source_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
  {};

  Test_I_Source_GTK_CBData* CBData;
  guint                     eventSourceID;
};

#endif
