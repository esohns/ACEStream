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

#ifndef TEST_I_POP_RECEIVE_COMMON_H
#define TEST_I_POP_RECEIVE_COMMON_H

#include <list>

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#if defined (WXWIDGETS_SUPPORT)
#include "wx/apptrait.h"
#include "wx/window.h"
#endif // WXWIDGETS_SUPPORT

#include "common_isubscribe.h"
#include "common_tools.h"

#include "common_ui_common.h"
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_common.h"
#include "common_ui_wxwidgets_xrc_definition.h"
#endif // WXWIDGETS_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "net_connection_configuration.h"

#include "pop_common.h"
#include "pop_configuration.h"
#include "pop_message.h"
#include "pop_sessionmessage.h"
#include "pop_stream_common.h"

#include "test_i_common.h"
#include "test_i_configuration.h"
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_i_qt_common.h"
#endif // QT_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT

#include "test_i_pop_receive_stream_common.h"

// forward declarations
class Stream_IAllocator;
template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Stream_POPReceive_EventHandler_T;
#if defined (WXWIDGETS_USE)
template <typename WidgetBaseClassType,
          typename InterfaceType,
          typename StreamType>
class Stream_POPReceive_WxWidgetsDialog_T;
#endif // WXWIDGETS_USE

enum Stream_POPReceive_ProgramMode
{
  STREAM_POPRECEIVE_PROGRAMMODE_PRINT_VERSION = 0,
  STREAM_POPRECEIVE_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  STREAM_POPRECEIVE_PROGRAMMODE_MAX,
  STREAM_POPRECEIVE_PROGRAMMODE_INVALID
};

struct Stream_POPReceive_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Stream_POPReceive_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

typedef std::list<POP_ISessionNotify_t*> Stream_POPReceive_Subscribers_t;
typedef Stream_POPReceive_Subscribers_t::iterator Stream_POPReceive_SubscribersIterator_t;

struct Stream_POPReceive_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
 , POP_ModuleHandlerConfiguration
{
  Stream_POPReceive_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , POP_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

  POP_IConnection_t*             connection;
  Net_ConnectionConfigurations_t* connectionConfigurations;
  POP_ISessionNotify_t*          subscriber;
  Stream_POPReceive_Subscribers_t*  subscribers;
};

struct Stream_POPReceive_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Stream_POPReceive_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , allocatorConfiguration ()
   , protocolConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , timerConfiguration ()
   , hostname ()
   , address (static_cast<u_short> (0),
              ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
              AF_INET)
   , username ()
   , password ()
  {}

  struct POP_AllocatorConfiguration                   allocatorConfiguration;
  // **************************** protocol data ********************************
  struct POP_ProtocolConfiguration                    protocolConfiguration;
  // **************************** signal data **********************************
  struct Stream_POPReceive_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_POPReceive_StreamConfiguration_t             streamConfiguration;
  // **************************** timer data ***********************************
  struct Common_TimerConfiguration                    timerConfiguration;

  std::string                                         hostname;
  ACE_INET_Addr                                       address; // server-
  std::string                                         username;
  std::string                                         password;
};

typedef POP_Message_T<enum Stream_MessageType> POP_Message_t;
typedef POP_SessionMessage_T<POP_Stream_SessionData_t,
                              struct Stream_UserData> POP_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct POP_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          POP_Message_t,
                                          POP_SessionMessage_t> Stream_POPReceive_MessageAllocator_t;

#if defined (WXWIDGETS_SUPPORT)
struct Stream_POPReceive_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_POPReceive_UI_CBData> Stream_POPReceive_WxWidgetsIApplication_t;
#endif // WXWIDGETS_SUPPORT

typedef Stream_POPReceive_EventHandler_T<POP_ISessionNotify_t,
                                         POP_Message_t,
#if defined (GTK_USE)
                                         Common_UI_GTK_State_t,
#elif defined (WXWIDGETS_USE)
                                         struct Common_UI_wxWidgets_State,
                                         Common_UI_wxWidgets_IApplicationBase_t,
#elif defined (QT_USE)
                                         struct Common_UI_Qt_State,
#else
                                         struct Common_UI_State,
#endif
                                         POP_SessionMessage_t> Stream_POPReceive_EventHandler_t;

//////////////////////////////////////////

struct Stream_POPReceive_ProgressData
 : Test_I_UI_ProgressData
{
  Stream_POPReceive_ProgressData ()
   : Test_I_UI_ProgressData ()
   , statistic ()
  {}

  POP_Statistic_t statistic;
};

class Test_I_POPReceive_Stream;
struct Stream_POPReceive_UI_CBData
 : Test_I_UI_CBData
{
  Stream_POPReceive_UI_CBData ()
   : Test_I_UI_CBData ()
   , configuration (NULL)
   , messageData ()
   , progressData ()
   , stream (NULL)
   , subscribers ()
  {
    progressData.state = UIState;
  }

  struct Stream_POPReceive_Configuration* configuration;
  std::string                             messageData;
  struct Stream_POPReceive_ProgressData   progressData;
  Test_I_POPReceive_Stream*               stream;
  Stream_POPReceive_Subscribers_t         subscribers;
};

struct Stream_POPReceive_UI_ThreadData
 : Test_I_UI_ThreadData
{
  Stream_POPReceive_UI_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Stream_POPReceive_UI_CBData* CBData;
};

#if defined (WXWIDGETS_SUPPORT)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Stream_POPReceive_WxWidgetsXRCDefinition_t;
//typedef Stream_POPReceive_WxWidgetsDialog_T<wxDialog_main,
//                                            Stream_POPReceive_WxWidgetsIApplication_t,
//                                            Stream_POPReceive_Stream> Stream_POPReceive_WxWidgetsDialog_t;
//typedef Comon_UI_WxWidgets_Application_T<Stream_POPReceive_WxWidgetsXRCDefinition_t,
//                                         struct Common_UI_wxWidgets_State,
//                                         struct Stream_POPReceive_UI_CBData,
//                                         Stream_POPReceive_WxWidgetsDialog_t,
//                                         wxGUIAppTraits> Stream_POPReceive_WxWidgetsApplication_t;
#endif // WXWIDGETS_SUPPORT

#endif
