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

#ifndef TEST_I_EXTRACT_STREAM_COMMON_H
#define TEST_I_EXTRACT_STREAM_COMMON_H

#include <list>
#include <map>
#include <string>

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#undef DrawText
#include "wx/apptrait.h"
#include "wx/window.h"
#endif // WXWIDGETS_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_inotify.h"
#include "common_isubscribe.h"
#include "common_tools.h"

#include "common_ui_common.h"
#include "common_ui_windowtype_converter.h"
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_messageallocatorheap_base.h"
#else
#include "stream_messageallocatorheap_base.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_session_data.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

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

//#include "extract_stream_wxwidgets_ui.h"
#endif // WXWIDGETS_SUPPORT

#include "test_i_session_message.h"

// forward declarations
class Stream_IAllocator;
template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Test_I_EventHandler_T;
#if defined (WXWIDGETS_SUPPORT)
//template <typename WidgetBaseClassType,
//          typename InterfaceType,
//          typename StreamType>
//class Test_I_WxWidgetsDialog_T;
#endif // WXWIDGETS_SUPPORT

enum Test_I_ExtractStream_ProgramMode
{
  TEST_I_EXTRACTSTREAM_PROGRAMMODE_PRINT_VERSION = 0,
  TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY,
  TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_VIDEO_ONLY,
  TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AV,
  ////////////////////////////////////////
  TEST_I_EXTRACTSTREAM_PROGRAMMODE_MAX,
  TEST_I_EXTRACTSTREAM_PROGRAMMODE_INVALID
};

struct Test_I_ExtractStream_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                      struct Stream_MediaFramework_FFMPEG_MediaType,
                                      struct Test_I_ExtractStream_StreamState,
                                      struct Stream_Statistic,
                                      struct Stream_UserData> Test_I_ExtractStream_SessionData;
typedef Stream_SessionData_T<Test_I_ExtractStream_SessionData> Test_I_ExtractStream_SessionData_t;

template <typename DataType>
class Test_I_Message_T;
//template <typename DataMessageType,
//          typename SessionDataType>
//class Test_I_ExtractStream_SessionMessage_T;
typedef Test_I_Message_T<struct Test_I_MessageData> Test_I_Message_t;
typedef Test_I_SessionMessage_T<Test_I_Message_t,
                                Test_I_ExtractStream_SessionData_t> Test_I_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_I_ExtractStream_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message_t,
                                    Test_I_SessionMessage_t> Test_I_ISessionNotify_t;
typedef std::list<Test_I_ISessionNotify_t*> Test_I_Subscribers_t;
typedef Test_I_Subscribers_t::iterator Test_I_SubscribersIterator_t;

struct Test_I_ExtractStream_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_ExtractStream_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
   , deviceType (AV_HWDEVICE_TYPE_NONE)
#endif // FFMPEG_SUPPORT
   , display ()
   , effect (ACE_TEXT_ALWAYS_CHAR ("tempo")) // preserve pitch
   , effectOptions ()
   , manageSoX (true)
#if defined (FFMPEG_SUPPORT)
   , outputFormat ()
   , streamIndex (-1)
#endif // FFMPEG_SUPPORT
   , subscriber (NULL)
   , targetFileName ()
   , window ()
  {}

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
  enum AVHWDeviceType                                     deviceType;
#endif // FFMPEG_SUPPORT
  struct Common_UI_DisplayDevice                          display;
  std::string effect;
  std::vector<std::string>                                effectOptions;
  bool                                                    manageSoX;
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_MediaType           outputFormat;
  int                                                     streamIndex;
#endif // FFMPEG_SUPPORT
  Test_I_ISessionNotify_t*                                subscriber;
  std::string                                             targetFileName;
  struct Common_UI_Window                                 window;
};

//extern const char stream_name_string_[];
struct Test_I_ExtractStream_StreamConfiguration;
struct Test_I_ExtractStream_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_ExtractStream_StreamConfiguration,
                               struct Test_I_ExtractStream_ModuleHandlerConfiguration> Test_I_ExtractStream_StreamConfiguration_t;

struct Test_I_ExtractStream_StreamState
 : Stream_State
{
  Test_I_ExtractStream_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_ExtractStream_SessionData* sessionData;
};

struct Test_I_ExtractStream_StreamConfiguration
 : Stream_Configuration
{
  Test_I_ExtractStream_StreamConfiguration ()
   : Stream_Configuration ()
   , mode (TEST_I_EXTRACTSTREAM_PROGRAMMODE_EXTRACT_AUDIO_ONLY)
   , slowDown (-1)
  {
    printFinalReport = true;
  }

  enum Test_I_ExtractStream_ProgramMode mode;
  int                                   slowDown;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_I_ExtractStream_StreamState> Test_I_ExtractStream_IStreamControl_t;

struct Test_I_ExtractStream_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_ExtractStream_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , streamConfiguration ()
  {
    allocatorConfiguration.defaultBufferSize = 1048576; // 1 mB
  }

  // **************************** stream data **********************************
  Test_I_ExtractStream_StreamConfiguration_t streamConfiguration;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message_t,
                                          Test_I_SessionMessage_t> Test_I_MessageAllocator_t;

#if defined (WXWIDGETS_SUPPORT)
struct Test_I_ExtractStream_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Test_I_ExtractStream_UI_CBData> Test_I_ExtractStream_WxWidgetsIApplication_t;
#endif // WXWIDGETS_SUPPORT
struct Test_I_GTK_State;

typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

typedef Test_I_EventHandler_T<Test_I_ISessionNotify_t,
                              Test_I_Message_t,
#if defined (GTK_USE)
                              struct Test_I_GTK_State,
#elif defined (WXWIDGETS_USE)
                              struct Common_UI_wxWidgets_State,
                              Common_UI_wxWidgets_IApplicationBase_t,
#endif
                              Test_I_SessionMessage_t> Test_I_EventHandler_t;

//////////////////////////////////////////

enum Test_I_ExtractStream_UI_EventType
{
  STREAM_AV_UI_EVENT_INVALID = COMMON_UI_EVENT_OTHER_USER_BASE,
  // -------------------------------------
  STREAM_AV_UI_EVENT_DATA_AUDIO,
  STREAM_AV_UI_EVENT_DATA_VIDEO,
  // -------------------------------------
  STREAM_AV_UI_EVENT_MAX
};
typedef ACE_Unbounded_Stack<enum Test_I_ExtractStream_UI_EventType> Test_I_ExtractStream_UI_Events_t;
typedef ACE_Unbounded_Stack<enum Test_I_ExtractStream_UI_EventType>::ITERATOR Test_I_ExtractStream_UI_EventsIterator_t;

struct Test_I_GTK_State
 : Common_UI_GTK_State_t
{
  Test_I_GTK_State ()
   : Common_UI_GTK_State_t ()
   , eventStack (NULL)
  {}

  Test_I_ExtractStream_UI_Events_t eventStack;
};

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                Common_UI_GTK_Configuration_t,
                                struct Test_I_GTK_State,
                                gpointer> Test_I_GTK_Manager_t;
typedef ACE_Singleton<Test_I_GTK_Manager_t,
                      ACE_MT_SYNCH::MUTEX> TEST_I_GTK_MANAGER_SINGLETON;

struct Test_I_ExtractStream_ProgressData
 : Test_I_UI_ProgressData
{
  Test_I_ExtractStream_ProgressData ()
   : Test_I_UI_ProgressData ()
   , audioFrameSize (0)
   , state (NULL)
   , lastStatistic ()
   , timeStamp (ACE_Time_Value::zero)
  {}

  unsigned int             audioFrameSize;
  struct Test_I_GTK_State* state;
  struct Stream_Statistic  lastStatistic;
  ACE_Time_Value           timeStamp;
};

class Test_I_Stream;
struct Test_I_ExtractStream_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_ExtractStream_UI_CBData ()
   : Test_I_UI_CBData ()
   , configuration (NULL)
   , progressData ()
   , stream (NULL)
   , UIState (NULL)
  {
    progressData.state = UIState;
  }

  struct Test_I_ExtractStream_Configuration* configuration;
  struct Test_I_ExtractStream_ProgressData   progressData;
  Test_I_Stream*                             stream;
  struct Test_I_GTK_State*                   UIState;
};


struct Test_I_ExtractStream_UI_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_ExtractStream_UI_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_ExtractStream_UI_CBData* CBData;
};

#if defined (WXWIDGETS_SUPPORT)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Test_I_WxWidgetsXRCDefinition_t;
//typedef Test_I_WxWidgetsDialog_T<wxDialog_main,
//                                 Test_I_WxWidgetsIApplication_t,
//                                 Test_I_Stream> Test_I_WxWidgetsDialog_t;
//typedef Comon_UI_WxWidgets_Application_T<Test_I_WxWidgetsXRCDefinition_t,
//                                         struct Common_UI_wxWidgets_State,
//                                         struct Test_I_ExtractStream_UI_CBData,
//                                         Test_I_WxWidgetsDialog_t,
//                                         wxGUIAppTraits> Test_I_WxWidgetsApplication_t;
#endif // WXWIDGETS_SUPPORT

#endif
