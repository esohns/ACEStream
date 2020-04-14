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

#include <list>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "common_file_common.h"
#include "common_isubscribe.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_control_message.h"
#include "stream_base.h"
#include "stream_isessionnotify.h"

#include "net_configuration.h"

#include "test_i_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_i_connection_manager_common.h"
#include "test_i_filestream_defines.h"
#include "test_i_filestream_network.h"
#include "test_i_message.h"

struct Test_I_Source_SessionData
 : Test_I_SessionData
{
  Test_I_Source_SessionData ()
   : Test_I_SessionData ()
   , fileName ()
   , size (0)
   , targetFileName ()
  {}

  struct Test_I_Source_SessionData& operator+= (const struct Test_I_Source_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_I_SessionData::operator+= (rhs_in);

    fileName = (fileName.empty () ? rhs_in.fileName : fileName);
    size = ((size == 0) ? rhs_in.size : size);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);
    userData = (userData ? userData : rhs_in.userData);

    return *this;
  }

  std::string  fileName;
  unsigned int size;
  std::string  targetFileName;
};
typedef Stream_SessionData_T<struct Test_I_Source_SessionData> Test_I_Source_SessionData_t;

struct Test_I_Source_StreamState
 : Test_I_StreamState
{
  Test_I_Source_StreamState ()
   : Test_I_StreamState ()
   , sessionData (NULL)
  {}

  struct Test_I_Source_SessionData* sessionData;
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_I_Source_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Source_Message_t,
                                    Test_I_Source_SessionMessage> Test_I_Source_ISessionNotify_t;
typedef std::list<Test_I_Source_ISessionNotify_t*> Test_I_Source_Subscribers_t;
typedef Test_I_Source_Subscribers_t::iterator Test_I_Source_SubscribersIterator_t;
struct Test_I_Source_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_Source_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , fileIdentifier ()
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  Test_I_Source_ITCPConnection_t*       connection; // TCP target module
  Net_ConnectionConfigurations_t*       connectionConfigurations;
  Test_I_Source_TCPConnectionManager_t* connectionManager; // TCP target module
  Common_File_Identifier                fileIdentifier; // file reader module
  Test_I_Source_StreamConfiguration_t*  streamConfiguration; // net source module
  Test_I_Source_ISessionNotify_t*       subscriber;
  Test_I_Source_Subscribers_t*          subscribers;
};

struct Test_I_Source_StreamConfiguration
 : Stream_Configuration
{
  Test_I_Source_StreamConfiguration ()
   : Stream_Configuration ()
  {}
};

typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_Source_StreamState,
                      struct Test_I_Source_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_Source_ModuleHandlerConfiguration,
                      struct Test_I_Source_SessionData,
                      Test_I_Source_SessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_Source_Message_t,
                      Test_I_Source_SessionMessage> Test_I_StreamBase_t;
struct Test_I_Source_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Test_I_Source_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , statisticReportingInterval (0)
   , stream (NULL)
  {}

  unsigned int         statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
  Test_I_StreamBase_t* stream;
};

struct Test_I_Source_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  Test_I_Source_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , connectionConfigurations ()
   , streamConfiguration ()
   , protocol (TEST_I_DEFAULT_TRANSPORT_LAYER)
  {}

  // **************************** signal data **********************************
  struct Test_I_Source_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** socket data **********************************
  Net_ConnectionConfigurations_t                  connectionConfigurations;
  // **************************** stream data **********************************
  Test_I_Source_StreamConfiguration_t             streamConfiguration;
  // *************************** protocol data *********************************
  enum Net_TransportLayerType                     protocol;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Source_Message_t,
                                          Test_I_Source_SessionMessage> Test_I_Source_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_I_Source_ISessionNotify_t> Test_I_Source_ISubscribe_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_I_Source_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#else
 : Test_I_UI_ProgressData
#endif // GTK_USE
{
  Test_I_Source_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#else
   : Test_I_UI_ProgressData ()
#endif // GTK_USE
   , size (0)
   , transferred (0)
  {}

  size_t size; // bytes
  size_t transferred;
};

struct Test_I_Source_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#else
 : Test_I_UI_CBData
#endif // GTK_USE
{
  Test_I_Source_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#else
   : Test_I_UI_CBData ()
#endif // GTK_USE
   , configuration (NULL)
   , loop(0)
   , progressData ()
   , stream (NULL)
   , subscribers ()
   , UDPStream(NULL)
  {}

  struct Test_I_Source_Configuration* configuration;
  size_t                              loop;
  struct Test_I_Source_ProgressData   progressData;
  Test_I_StreamBase_t*                stream;
  Test_I_Source_Subscribers_t         subscribers;
  Test_I_StreamBase_t*                UDPStream;
};

struct Test_I_Source_UI_ThreadData
#if defined (GTK_USE)
 : Test_I_GTK_ThreadData
#else
 : Test_I_UI_ThreadData
#endif // GTK_USE
{
  Test_I_Source_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_I_GTK_ThreadData ()
#else
   : Test_I_UI_ThreadData ()
#endif // GTK_USE
   , CBData (NULL)
  {}

  struct Test_I_Source_UI_CBData* CBData;
};
#endif // GUI_SUPPORT

#endif
