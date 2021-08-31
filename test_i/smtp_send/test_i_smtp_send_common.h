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

#ifndef TEST_I_SMTP_SEND_COMMON_H
#define TEST_I_SMTP_SEND_COMMON_H

#include <list>

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#elif defined (WXWIDGETS_USE)
#include "wx/apptrait.h"
#include "wx/window.h"
#endif
#endif // GUI_SUPPORT

#include "common_isubscribe.h"
#include "common_tools.h"

#if defined (GUI_SUPPORT)
#include "common_ui_common.h"
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_common.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#elif defined (WXWIDGETS_USE)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_common.h"
#include "common_ui_wxwidgets_xrc_definition.h"
#endif
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "net_connection_configuration.h"

#include "smtp_common.h"
#include "smtp_configuration.h"
#include "smtp_message.h"
#include "smtp_sessionmessage.h"
#include "smtp_stream_common.h"

#include "test_i_common.h"
#include "test_i_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#elif defined (QT_USE)
#include "test_i_qt_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_i_wxwidgets_common.h"
#endif
#endif // GUI_SUPPORT
#include "test_i_smtp_send_stream_common.h"

// forward declarations
class Stream_IAllocator;
template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
class Stream_SMTPSend_EventHandler_T;
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
template <typename WidgetBaseClassType,
          typename InterfaceType,
          typename StreamType>
class Stream_SMTPSend_WxWidgetsDialog_T;
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

enum Stream_SMTPSend_ProgramMode
{
  STREAM_SMTPSEND_PROGRAMMODE_PRINT_VERSION = 0,
  STREAM_SMTPSEND_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  STREAM_SMTPSEND_PROGRAMMODE_MAX,
  STREAM_SMTPSEND_PROGRAMMODE_INVALID
};

struct Stream_SMTPSend_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Stream_SMTPSend_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

typedef std::list<SMTP_ISessionNotify_t*> Stream_SMTPSend_Subscribers_t;
typedef Stream_SMTPSend_Subscribers_t::iterator Stream_SMTPSend_SubscribersIterator_t;

struct Stream_SMTPSend_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
 , SMTP_ModuleHandlerConfiguration
{
  Stream_SMTPSend_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , SMTP_ModuleHandlerConfiguration ()
   , connectionConfigurations (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

  Net_ConnectionConfigurations_t* connectionConfigurations;
  SMTP_ISessionNotify_t*          subscriber;
  Stream_SMTPSend_Subscribers_t*  subscribers;
};

struct Stream_SMTPSend_Configuration
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
  Stream_SMTPSend_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , allocatorConfiguration ()
   , protocolConfiguration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , timerConfiguration ()
   , address (static_cast<u_short> (0),
              ACE_TEXT_ALWAYS_CHAR (ACE_LOCALHOST),
              AF_INET)
   , username ()
   , password ()
   , from ()
   , to ()
   , message ()
  {}

  struct SMTP_AllocatorConfiguration                allocatorConfiguration;
  // **************************** protocol data ********************************
  struct SMTP_ProtocolConfiguration                 protocolConfiguration;
  // **************************** signal data **********************************
  struct Stream_SMTPSend_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_SMTPSend_StreamConfiguration_t             streamConfiguration;
  // **************************** timer data ***********************************
  struct Common_TimerConfiguration                  timerConfiguration;

  ACE_INET_Addr                                     address;
  std::string                                       username;
  std::string                                       password;
  std::string                                       from;
  std::string                                       to;
  std::string                                       message;
};

typedef SMTP_Message_T<enum Stream_MessageType> SMTP_Message_t;
typedef SMTP_SessionMessage_T<SMTP_Stream_SessionData_t,
                              struct Stream_UserData> SMTP_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct SMTP_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          SMTP_Message_t,
                                          SMTP_SessionMessage_t> Stream_SMTPSend_MessageAllocator_t;

#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
struct Stream_SMTPSend_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_SMTPSend_UI_CBData> Stream_SMTPSend_WxWidgetsIApplication_t;
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

typedef Stream_SMTPSend_EventHandler_T<SMTP_ISessionNotify_t,
                                       SMTP_Message_t,
#if defined (GUI_SUPPORT)
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
#endif // GUI_SUPPORT
                                       SMTP_SessionMessage_t> Stream_SMTPSend_EventHandler_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Stream_SMTPSend_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (QT_USE)
 : Test_I_Qt_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#else
 : Test_I_UI_ProgressData
#endif
{
  Stream_SMTPSend_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#elif defined (QT_USE)
    : Test_I_Qt_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
#else
   : Test_I_UI_ProgressData ()
#endif
   , statistic ()
  {}

  SMTP_Statistic_t statistic;
};

struct Stream_SMTPSend_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (QT_USE)
 : Test_I_Qt_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#else
 : Test_I_UI_CBData
#endif
{
  Stream_SMTPSend_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#elif defined (QT_USE)
   : Test_I_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
#else
   : Test_I_UI_CBData ()
#endif
   , configuration (NULL)
   , progressData ()
   , stream (NULL)
   , subscribers ()
  {
    progressData.state = UIState;
  }

  struct Stream_SMTPSend_Configuration* configuration;
  struct Stream_SMTPSend_ProgressData   progressData;
  Test_I_SMTPSend_Stream_t*             stream;
  Stream_SMTPSend_Subscribers_t         subscribers;
};

struct Stream_SMTPSend_UI_ThreadData
#if defined (GTK_USE)
 : Test_I_GTK_ThreadData
#elif defined (QT_USE)
 : Test_I_Qt_ThreadData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ThreadData
#else
 : Test_I_UI_ThreadData
#endif
{
  Stream_SMTPSend_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_I_GTK_ThreadData ()
#elif defined (QT_USE)
   : Test_I_Qt_ThreadData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ThreadData ()
#else
   : Test_I_UI_ThreadData ()
#endif
   , CBData (NULL)
  {}

  struct Stream_SMTPSend_UI_CBData* CBData;
};

#if defined (GTK_USE)
#elif defined (WXWIDGETS_USE)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Stream_SMTPSend_WxWidgetsXRCDefinition_t;
typedef Stream_SMTPSend_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_SMTPSend_WxWidgetsIApplication_t,
                                         Stream_SMTPSend_Stream> Stream_SMTPSend_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_SMTPSend_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_SMTPSend_UI_CBData,
                                         Stream_SMTPSend_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_SMTPSend_WxWidgetsApplication_t;
#endif
#endif // GUI_SUPPORT

#endif
