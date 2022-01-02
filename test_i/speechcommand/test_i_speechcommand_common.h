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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "Audioclient.h"
#include "devicetopology.h"
#include "endpointvolume.h"
#endif // ACE_WIN32 || ACE_WIN64

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_i_common.h"
#include "test_i_configuration.h"

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
   , allocatorProperties ()
   , filterConfiguration ()
   , pinConfiguration ()
   , streamConfiguration ()
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
};

struct Test_I_MediaFoundation_Configuration
 : Test_I_SpeechCommand_Configuration
{
  Test_I_MediaFoundation_Configuration ()
   : Test_I_SpeechCommand_Configuration ()
   , mediaFoundationConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** framework data *******************************
  struct Stream_MediaFramework_MediaFoundation_Configuration mediaFoundationConfiguration;
  // **************************** stream data **********************************
  Test_I_MediaFoundation_StreamConfiguration_t               streamConfiguration;
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
                              Test_I_DirectShow_Message,
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
                              Test_I_MediaFoundation_Message,
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
struct Test_I_SpeechCommand_UI_CBData
 : Test_I_UI_CBData
{
  Test_I_SpeechCommand_UI_CBData ()
   : Test_I_UI_CBData ()
#if defined (GTK_USE)
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions ()
#endif // GTKGL_SUPPORT
#endif // GTK_USE
   , stream (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , boostControl (NULL)
   , captureVolumeControl (NULL)
   , renderVolumeControl (NULL)
#endif // ACE_WIN32 || ACE_WIN64
  {}

#if defined (GTK_USE)
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_GTKGL_Instructions_t OpenGLInstructions;
#endif // GTKGL_SUPPORT
#endif // GTK_USE
  Stream_IStreamControlBase*                stream;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IAudioVolumeLevel*                        boostControl;
  IAudioEndpointVolume*                     captureVolumeControl;
  ISimpleAudioVolume*                       renderVolumeControl;
#endif // ACE_WIN32 || ACE_WIN64
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_UI_CBData
 : Test_I_SpeechCommand_UI_CBData
{
  Test_I_DirectShow_UI_CBData ()
   : Test_I_SpeechCommand_UI_CBData ()
   , configuration (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {}

  struct Test_I_DirectShow_Configuration* configuration;
  IAMStreamConfig*                        streamConfiguration;
  Test_I_DirectShow_Subscribers_t         subscribers;
};

struct Test_I_MediaFoundation_UI_CBData
 : Test_I_SpeechCommand_UI_CBData
{
  Test_I_MediaFoundation_UI_CBData ()
   : Test_I_SpeechCommand_UI_CBData ()
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
 : Test_I_SpeechCommand_UI_CBData
{
  Test_I_ALSA_UI_CBData ()
   : Test_I_SpeechCommand_UI_CBData ()
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
