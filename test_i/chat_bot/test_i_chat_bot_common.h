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

#ifndef TEST_I_ChatBot_COMMON_H
#define TEST_I_ChatBot_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "Audioclient.h"
#include "devicetopology.h"
#include "endpointvolume.h"
#endif // ACE_WIN32 || ACE_WIN64

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
#include "stream_message_base.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"
#include "stream_session_manager.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_common.h"
#include "stream_misc_input_manager.h"
#include "stream_misc_input_stream.h"

#include "stream_vis_gtk_cairo_spectrum_analyzer.h"

#include "test_i_common.h"
#include "test_i_configuration.h"

#include "test_i_stream_common.h"

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Test_I_EventHandler_T;
template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Test_I_InputHandler_T;
#if defined (WXWIDGETS_SUPPORT)
template <typename WidgetBaseClassType,
          typename InterfaceType,
          typename StreamType>
class Test_I_WxWidgetsDialog_T;
#endif // WXWIDGETS_SUPPORT

struct Test_I_ChatBot_Configuration
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
{
  Test_I_ChatBot_Configuration ()
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
   , inputConfiguration ()
   , inputManagerConfiguration ()
  {
    dispatchConfiguration.dispatch = COMMON_EVENT_DISPATCH_REACTOR;
    dispatchConfiguration.numberOfProactorThreads = 0;
    dispatchConfiguration.numberOfReactorThreads = 0;
    inputConfiguration.allocatorConfiguration = &allocatorConfiguration;
    inputConfiguration.eventDispatchConfiguration =
      &dispatchConfiguration;
    inputManagerConfiguration.eventDispatchConfiguration =
      &dispatchConfiguration;
    inputManagerConfiguration.handlerConfiguration = &inputConfiguration;
    inputManagerConfiguration.manageEventDispatch = true;
  }

  struct Common_Input_Configuration    inputConfiguration;
  Stream_Input_Manager_Configuration_t inputManagerConfiguration;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_Configuration
 : Test_I_ChatBot_Configuration
{
  Test_I_DirectShow_Configuration ()
   : Test_I_ChatBot_Configuration ()
   , allocatorProperties ()
   , filterConfiguration ()
   , pinConfiguration ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
  {
    ACE_OS::memset (&allocatorProperties, 0, sizeof (struct _AllocatorProperties));
    //allocatorProperties_.cBuffers = -1; // <-- use default
    allocatorProperties.cBuffers =
      STREAM_LIB_DIRECTSHOW_AUDIO_DEFAULT_SOURCE_BUFFERS;
    allocatorProperties.cbBuffer = -1; // <-- use default
    // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
    //         if this is -1/0 (why ?)
    //allocatorProperties_.cbAlign = -1;  // <-- use default
    allocatorProperties.cbAlign = 1;
    // *TODO*: IMemAllocator::SetProperties returns E_INVALIDARG (0x80070057)
    //         if this is -1/0 (why ?)
    //allocatorProperties.cbPrefix = -1; // <-- use default
    allocatorProperties.cbPrefix = 0;

    filterConfiguration.allocatorProperties = &allocatorProperties;
    filterConfiguration.pinConfiguration = &pinConfiguration;
    pinConfiguration.allocatorProperties = &allocatorProperties;
  }

  // **************************** framework data *******************************
  struct _AllocatorProperties                                    allocatorProperties; // IMediaSample-
  struct Stream_MediaFramework_DirectShow_FilterConfiguration    filterConfiguration;
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration pinConfiguration;
  // **************************** stream data **********************************
  Test_I_DirectShow_StreamConfiguration_t                        streamConfiguration;
  Stream_Input_Configuration_t                                   streamConfiguration_2; // input-
};

struct Test_I_MediaFoundation_Configuration
 : Test_I_ChatBot_Configuration
{
  Test_I_MediaFoundation_Configuration ()
   : Test_I_ChatBot_Configuration ()
   , mediaFoundationConfiguration ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
  {}

  // **************************** framework data *******************************
  struct Stream_MediaFramework_MediaFoundation_Configuration mediaFoundationConfiguration;
  // **************************** stream data **********************************
  Test_I_MediaFoundation_StreamConfiguration_t               streamConfiguration;
  Stream_Input_Configuration_t                               streamConfiguration_2; // input-
};
#else
struct Test_I_ALSA_Configuration
 : Test_I_ChatBot_Configuration
{
  Test_I_ALSA_Configuration ()
   : Test_I_ChatBot_Configuration ()
   , streamConfiguration ()
   , streamConfiguration_2 ()
  {}

  // **************************** stream data **********************************
  Test_I_ALSA_StreamConfiguration_t streamConfiguration;
  Stream_Input_Configuration_t      streamConfiguration_2; // input-
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (WXWIDGETS_SUPPORT)
struct Test_I_ChatBot_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Test_I_ChatBot_UI_CBData> Test_I_WxWidgetsIApplication_t;
#endif // WXWIDGETS_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_EventHandler_T<Test_I_DirectShow_ISessionNotify_t,
                              Test_I_DirectShow_Message,
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
                              Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_EventHandler_t;
typedef Test_I_EventHandler_T<Test_I_MediaFoundation_ISessionNotify_t,
                              Test_I_MediaFoundation_Message,
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
                              Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_EventHandler_t;
#else
typedef Test_I_EventHandler_T<Test_I_ALSA_ISessionNotify_t,
                              Test_I_Message,
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
                              Test_I_ALSA_SessionMessage_t> Test_I_ALSA_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64
typedef Test_I_InputHandler_T<Stream_IInputSessionNotify_t,
                              Stream_MessageBase_T<Stream_DataBase_T<Stream_CommandType_t>,
                                                   enum Stream_MessageType,
                                                   Stream_CommandType_t>,
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
                              Stream_SessionMessageBase_T<enum Stream_SessionMessageType,
                                                          Stream_SessionData_T<struct Stream_SessionData>,
                                                          struct Stream_UserData> > Test_I_InputHandler_t;

typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Stream_SessionData,
                                 struct Stream_Statistic,
                                 struct Stream_UserData> Test_I_SessionManager_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Test_I_ChatBot_DirectShow_SessionData,
                                 struct Test_I_Statistic,
                                 struct Stream_UserData> Test_I_DirectShow_SessionManager_2;
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Test_I_ChatBot_MediaFoundation_SessionData,
                                 struct Test_I_Statistic,
                                 struct Stream_UserData> Test_I_MediaFoundation_SessionManager_2;
#else
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Test_I_ChatBot_ALSA_SessionData,
                                 struct Test_I_Statistic,
                                 struct Stream_UserData> Test_I_ALSA_SessionManager_2;
#endif // ACE_WIN32 || ACE_WIN64

typedef Stream_Miscellaneous_Input_Stream_T<ACE_MT_SYNCH,
                                            Common_TimePolicy_t,
                                            enum Stream_ControlType,
                                            enum Stream_SessionMessageType,
                                            enum Stream_StateMachine_ControlState,
                                            struct Stream_State,
                                            struct Stream_Configuration,
                                            struct Stream_Statistic,
                                            Common_Timer_Manager_t,
                                            struct Stream_Input_ModuleHandlerConfiguration,
                                            Test_I_SessionManager_t,
                                            Stream_ControlMessage_t,
                                            Stream_MessageBase_T<Stream_DataBase_T<Stream_CommandType_t>,
                                                                 enum Stream_MessageType,
                                                                 Stream_CommandType_t>,
                                            Stream_SessionMessageBase_T<enum Stream_SessionMessageType,
                                                                        Stream_SessionData_T<struct Stream_SessionData>,
                                                                        struct Stream_UserData>,
                                            struct Stream_UserData> Test_I_InputStream_t;
typedef Stream_Input_Manager_T<ACE_MT_SYNCH,
                               Stream_Input_Manager_Configuration_t,
                               Common_InputHandler_t,
                               Test_I_InputStream_t> Test_I_InputManager_t;

//////////////////////////////////////////

enum Test_I_ChatBot_InputCommand
{
  TEST_I_INPUT_COMMAND_SHUTDOWN = 0,
  TEST_I_INPUT_COMMAND_GAIN_DECREASE,
  TEST_I_INPUT_COMMAND_GAIN_INCREASE,
  ////////////////////////////////////////
  TEST_I_INPUT_COMMAND_MAX,
  TEST_I_INPUT_COMMAND_INVALID = -1,
};

//////////////////////////////////////////

struct Test_I_ChatBot_UI_ProgressData
 : Test_I_UI_ProgressData
{
  Test_I_ChatBot_UI_ProgressData ()
   : Test_I_UI_ProgressData ()
   , words ()
  {}

  Stream_Decoder_STT_Result_t words;
};

#if defined (GTK_SUPPORT)
#if GTK_CHECK_VERSION (4,0,0)
typedef Common_ISetP_T<GdkSurface> Test_I_Common_ISet_t;
#else
typedef Common_ISetP_T<GdkWindow> Test_I_Common_ISet_t;
#endif // GTK_CHECK_VERSION (4,0,0)
#endif // GTK_SUPPORT
struct Test_I_ChatBot_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_ChatBot_UI_CBData ()
   : Test_I_UI_CBData ()
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions ()
   , objectRotation (1)
#endif // GTKGL_SUPPORT
#endif // GTK_SUPPORT
#if defined (GTK_SUPPORT)
   , resizeNotification (NULL)
   , spectrumAnalyzer (NULL)
   , spectrumAnalyzerCBData ()
#endif // GTK_SUPPORT
   , stream (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , boostControl (NULL)
   , captureVolumeControl (NULL)
  //, renderVolumeControl (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , progressData ()
  {
    ACE_OS::memset (&spectrumAnalyzerCBData, 0, sizeof (struct acestream_visualization_gtk_cairo_cbdata));
  }

#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_GTKGL_Instructions_t       OpenGLInstructions;
  int                                             objectRotation;
#endif // GTKGL_SUPPORT
#endif // GTK_SUPPORT
#if defined (GTK_SUPPORT)
  Test_I_Common_ISet_t*                           resizeNotification;
  Common_IDispatch*                               spectrumAnalyzer;
  struct acestream_visualization_gtk_cairo_cbdata spectrumAnalyzerCBData;
#endif // GTK_SUPPORT
  Stream_IStreamControlBase*                      stream;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IAudioVolumeLevel*                              boostControl;
  IAudioEndpointVolume*                           captureVolumeControl;
  //ISimpleAudioVolume*                       renderVolumeControl;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_I_ChatBot_UI_ProgressData           progressData;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_UI_CBData
 : Test_I_ChatBot_UI_CBData
{
  Test_I_DirectShow_UI_CBData ()
   : Test_I_ChatBot_UI_CBData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {}

  struct Test_I_DirectShow_Configuration* configuration;
  IAMStreamConfig*                        streamConfiguration;
  Test_I_DirectShow_Subscribers_t         subscribers;
};

struct Test_I_MediaFoundation_UI_CBData
 : Test_I_ChatBot_UI_CBData
{
  Test_I_MediaFoundation_UI_CBData ()
   : Test_I_ChatBot_UI_CBData ()
   , configuration (NULL)
   , subscribers ()
  {}

  struct Test_I_MediaFoundation_Configuration* configuration;
  Test_I_MediaFoundation_Subscribers_t         subscribers;
};

struct Test_I_DirectShow_UI_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_DirectShow_UI_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_DirectShow_UI_CBData* CBData;
};

struct Test_I_MediaFoundation_UI_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_MediaFoundation_UI_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_MediaFoundation_UI_CBData* CBData;
};
#else
struct Test_I_ALSA_UI_CBData
 : Test_I_ChatBot_UI_CBData
{
  Test_I_ALSA_UI_CBData ()
   : Test_I_ChatBot_UI_CBData ()
   , configuration (NULL)
   , handle (NULL)
   , subscribers ()
  {}

  struct Test_I_ALSA_Configuration* configuration;
  struct _snd_pcm*                  handle; // (capture) device handle
  Test_I_ALSA_Subscribers_t         subscribers;
};

struct Test_I_ALSA_UI_ThreadData
 : Test_I_UI_ThreadData
{
  Test_I_ALSA_UI_ThreadData ()
   : Test_I_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_I_ALSA_UI_CBData* CBData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (WXWIDGETS_SUPPORT)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Test_I_WxWidgetsXRCDefinition_t;
//typedef Test_I_WxWidgetsDialog_T<wxDialog_main,
//                                 Test_I_WxWidgetsIApplication_t,
//                                 Test_I_Stream> Test_I_WxWidgetsDialog_t;
//typedef Comon_UI_WxWidgets_Application_T<Test_I_WxWidgetsXRCDefinition_t,
//                                         struct Common_UI_wxWidgets_State,
//                                         struct Test_I_ChatBot_UI_CBData,
//                                         Test_I_WxWidgetsDialog_t,
//                                         wxGUIAppTraits> Test_I_WxWidgetsApplication_t;
#endif // WXWIDGETS_SUPPORT

#endif
