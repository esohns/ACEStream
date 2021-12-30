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

#ifndef TEST_I_SPEECHCOMMAND_COMMON_H
#define TEST_I_SPEECHCOMMAND_COMMON_H

#include <list>

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "wx/apptrait.h"
#include "wx/window.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#include "common_isubscribe.h"
#include "common_tools.h"

#if defined (GUI_SUPPORT)
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
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "test_i_common.h"
#include "test_i_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_i_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_i_qt_common.h"
#endif // QT_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_i_wxwidgets_common.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT
#include "test_i_session_message.h"
#include "test_i_stream_common.h"

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
#if defined (GUI_SUPPORT)
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
          typename SessionMessageType>
class Test_I_EventHandler_T;
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_SUPPORT)
template <typename WidgetBaseClassType,
          typename InterfaceType,
          typename StreamType>
class Test_I_WxWidgetsDialog_T;
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

class Test_I_Message;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_SessionMessage_T<Test_I_DirectShow_SessionData_t,
                                struct Stream_UserData> Test_I_DirectShow_SessionMessage_t;
typedef Test_I_SessionMessage_T<Test_I_MediaFoundation_SessionData_t,
                                struct Stream_UserData> Test_I_MediaFoundation_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Test_I_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_ISessionNotify_t;
typedef Stream_ISessionDataNotify_T<Test_I_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_ISessionNotify_t;

typedef std::list<Test_I_DirectShow_ISessionNotify_t*> Test_I_DirectShow_Subscribers_t;
typedef Test_I_DirectShow_Subscribers_t::iterator Test_I_DirectShow_SubscribersIterator_t;
typedef std::list<Test_I_MediaFoundation_ISessionNotify_t*> Test_I_MediaFoundation_Subscribers_t;
typedef Test_I_MediaFoundation_Subscribers_t::iterator Test_I_MediaFoundation_SubscribersIterator_t;
#else
typedef Test_I_SessionMessage_T<Test_I_ALSA_SessionData_t,
                                struct Stream_UserData> Test_I_ALSA_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_ALSA_SessionMessage_t> Test_I_ALSA_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Test_I_ALSA_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_ALSA_SessionMessage_t> Test_I_ALSA_ISessionNotify_t;

typedef std::list<Test_I_ALSA_ISessionNotify_t*> Test_I_ALSA_Subscribers_t;
typedef Test_I_ALSA_Subscribers_t::iterator Test_I_ALSA_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  Test_I_DirectShow_ISessionNotify_t* subscriber;
  Test_I_DirectShow_Subscribers_t*    subscribers;
};

struct Test_I_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  Test_I_MediaFoundation_ISessionNotify_t* subscriber;
  Test_I_MediaFoundation_Subscribers_t*    subscribers;
};
#else
struct Test_I_ALSA_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_ALSA_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  Test_I_ALSA_ISessionNotify_t* subscriber;
  Test_I_ALSA_Subscribers_t*    subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_SpeechCommand_Configuration
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
  Test_I_SpeechCommand_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
  {}
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_Configuration
 : Test_I_SpeechCommand_Configuration
{
  Test_I_DirectShow_Configuration ()
   : Test_I_SpeechCommand_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_I_DirectShow_StreamConfiguration_t streamConfiguration;
};
struct Test_I_MediaFoundation_Configuration
 : Test_I_SpeechCommand_Configuration
{
  Test_I_MediaFoundation_Configuration ()
   : Test_I_SpeechCommand_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_I_MediaFoundation_StreamConfiguration_t streamConfiguration;
};
#else
struct Test_I_ALSA_Configuration
 : Test_I_SpeechCommand_Configuration
{
  Test_I_ALSA_Configuration ()
   : Test_I_SpeechCommand_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_I_ALSA_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_SUPPORT)
struct Test_I_SpeechCommand_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Test_I_SpeechCommand_UI_CBData> Test_I_WxWidgetsIApplication_t;
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_EventHandler_T<Test_I_DirectShow_ISessionNotify_t,
                              Test_I_Message,
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
#endif // GTK_USE || WXWIDGETS_USE || QT_USE
#endif // GUI_SUPPORT
                              Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_EventHandler_t;
typedef Test_I_EventHandler_T<Test_I_MediaFoundation_ISessionNotify_t,
                              Test_I_Message,
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
#endif // GTK_USE || WXWIDGETS_USE || QT_USE
#endif // GUI_SUPPORT
                              Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_EventHandler_t;
#else
typedef Test_I_EventHandler_T<Test_I_ALSA_ISessionNotify_t,
                              Test_I_Message,
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
#endif // GTK_USE || WXWIDGETS_USE || QT_USE
#endif // GUI_SUPPORT
                              Test_I_ALSA_SessionMessage_t> Test_I_ALSA_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_I_ProgressData
#if defined (GTK_USE)
 : Test_I_GTK_ProgressData
#elif defined (QT_USE)
 : Test_I_Qt_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ProgressData
#else
 : Test_I_UI_ProgressData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_ProgressData ()
#if defined (GTK_USE)
   : Test_I_GTK_ProgressData ()
#elif defined (QT_USE)
    : Test_I_Qt_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ProgressData ()
#else
   : Test_I_UI_ProgressData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
  {}
};

class Test_I_Stream;
struct Test_I_SpeechCommand_UI_CBData
#if defined (GTK_USE)
 : Test_I_GTK_CBData
#elif defined (QT_USE)
 : Test_I_Qt_CBData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_CBData
#else
 : Test_I_UI_CBData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_SpeechCommand_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#elif defined (QT_USE)
   : Test_I_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
#else
   : Test_I_UI_CBData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , configuration (NULL)
   , progressData ()
   , stream (NULL)
  {
    progressData.state = UIState;
  }

  struct Test_I_Configuration* configuration;
  struct Test_I_ProgressData   progressData;
  Test_I_Stream*               stream;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_DirectShow_UI_CBData ()
   : Test_I_UI_CBData ()
   , subscribers ()
  {}

  Test_I_DirectShow_Subscribers_t subscribers;
};
struct Test_I_MediaFoundation_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_MediaFoundation_UI_CBData ()
   : Test_I_UI_CBData ()
   , subscribers ()
  {}

  Test_I_MediaFoundation_Subscribers_t subscribers;
};
#else
struct Test_I_ALSA_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_ALSA_UI_CBData ()
   : Test_I_UI_CBData ()
   , subscribers ()
  {}

  Test_I_ALSA_Subscribers_t subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_SpeechCommand_UI_ThreadData
#if defined (GTK_USE)
 : Test_I_GTK_ThreadData
#elif defined (QT_USE)
 : Test_I_Qt_ThreadData
#elif defined (WXWIDGETS_USE)
 : Test_I_wxWidgets_ThreadData
#else
 : Test_I_UI_ThreadData
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
{
  Test_I_SpeechCommand_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_I_GTK_ThreadData ()
#elif defined (QT_USE)
   : Test_I_Qt_ThreadData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_ThreadData ()
#else
   : Test_I_UI_ThreadData ()
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
   , CBData (NULL)
  {}

  struct Test_I_SpeechCommand_UI_CBData* CBData;
};

#if defined (WXWIDGETS_USE)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Test_I_WxWidgetsXRCDefinition_t;
typedef Test_I_WxWidgetsDialog_T<wxDialog_main,
                                 Test_I_WxWidgetsIApplication_t,
                                 Test_I_Stream> Test_I_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Test_I_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Test_I_SpeechCommand_UI_CBData,
                                         Test_I_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Test_I_WxWidgetsApplication_t;
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

#endif
