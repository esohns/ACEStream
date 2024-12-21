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

#ifndef TEST_I_CAMERA_MSA_COMMON_H
#define TEST_I_CAMERA_MSA_COMMON_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "BaseTyps.h"
#include "OAIdl.h"
#include "control.h"
#include "CGuid.h"
#include "Guiddef.h"
#include "d3d9.h"
#undef GetObject
#include "evr.h"
#include "mfapi.h"
#undef GetObject
#include "mfidl.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

//#include "X11/X.h"
typedef unsigned long Window;
//#undef CursorShape
#include "wayland-client.h"

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include <string>

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_tools.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_common.h"
#endif // GTK_SUPPORT

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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_common.h"
#include "stream_lib_directshow_tools.h"
#else
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_i_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;
#endif // ACE_WIN32 || ACE_WIN64
class Stream_IAllocator;
template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Test_I_EventHandler_T;

struct Test_I_StatisticData
 : Stream_Statistic
{
  Test_I_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_I_StatisticData operator+= (const struct Test_I_StatisticData& rhs_in)
  {
    Stream_Statistic::operator+= (rhs_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    capturedFrames += rhs_in.capturedFrames;
#endif // ACE_WIN32 || ACE_WIN64

    return *this;
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  unsigned int capturedFrames;
#endif // ACE_WIN32 || ACE_WIN64
};
typedef Common_StatisticHandler_T<struct Test_I_StatisticData> Test_I_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamState;
class Test_I_CameraMSA_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct _AMMediaType,
                                        struct Test_I_CameraMSA_DirectShow_StreamState,
                                        struct Test_I_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_I_CameraMSA_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct _AMMediaType,
                                   struct Test_I_CameraMSA_DirectShow_StreamState,
                                   struct Test_I_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , resetToken (0)
  {}

  Test_I_CameraMSA_DirectShow_SessionData& operator= (const Test_I_CameraMSA_DirectShow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Test_I_CameraMSA_DirectShow_StreamState,
                                  struct Test_I_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);

    return *this;
  }
  Test_I_CameraMSA_DirectShow_SessionData& operator+= (const Test_I_CameraMSA_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Test_I_CameraMSA_DirectShow_StreamState,
                                  struct Test_I_StatisticData,
                                  struct Stream_UserData>::operator+= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);

    return *this;
  }

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex* direct3DDevice;
#else
  IDirect3DDevice9*   direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  UINT                direct3DManagerResetToken;
  UINT                resetToken;
};
typedef Stream_SessionData_T<Test_I_CameraMSA_DirectShow_SessionData> Test_I_CameraMSA_DirectShow_SessionData_t;

struct Test_I_MediaFoundation_StreamState;
class Test_I_CameraMSA_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        IMFMediaType*,
                                        struct Test_I_CameraMSA_MediaFoundation_StreamState,
                                        struct Test_I_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_I_CameraMSA_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   IMFMediaType*,
                                   struct Test_I_CameraMSA_MediaFoundation_StreamState,
                                   struct Test_I_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
  {}

  Test_I_CameraMSA_MediaFoundation_SessionData& operator= (const Test_I_CameraMSA_MediaFoundation_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Test_I_CameraMSA_MediaFoundation_StreamState,
                                  struct Test_I_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    rendererNodeId = (rendererNodeId ? rendererNodeId : rhs_in.rendererNodeId);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);
    session = (session ? session : rhs_in.session);

    return *this;
  }
  Test_I_CameraMSA_MediaFoundation_SessionData& operator+= (const Test_I_CameraMSA_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Test_I_CameraMSA_MediaFoundation_StreamState,
                                  struct Test_I_StatisticData,
                                  struct Stream_UserData>::operator+= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    rendererNodeId = (rendererNodeId ? rendererNodeId : rhs_in.rendererNodeId);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);
    session = (session ? session : rhs_in.session);

    return *this;
  }

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*                 direct3DDevice;
#else
  IDirect3DDevice9*                   direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  UINT                                direct3DManagerResetToken;
  TOPOID                              rendererNodeId;
  UINT                                resetToken;
  IMFMediaSession*                    session;
};
typedef Stream_SessionData_T<Test_I_CameraMSA_MediaFoundation_SessionData> Test_I_CameraMSA_MediaFoundation_SessionData_t;
#else
struct Test_I_CameraMSA_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                      struct Stream_MediaFramework_V4L_MediaType,
                                      struct Test_I_CameraMSA_StreamState,
                                      struct Test_I_StatisticData,
                                      struct Stream_UserData> Test_I_CameraMSA_V4L_SessionData;
typedef Stream_SessionData_T<Test_I_CameraMSA_V4L_SessionData> Test_I_CameraMSA_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

template <typename DataType,
          typename SessionDataType>
class Test_I_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Test_I_SessionMessage_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_Message_T<struct Test_I_DirectShow_MessageData,
                         Test_I_CameraMSA_DirectShow_SessionData_t> Test_I_DirectShow_Message_t;
typedef Test_I_SessionMessage_T<Test_I_DirectShow_Message_t,
                                Test_I_CameraMSA_DirectShow_SessionData_t> Test_I_DirectShow_SessionMessage_t;
typedef Test_I_Message_T<struct Test_I_MediaFoundation_MessageData,
                         Test_I_CameraMSA_MediaFoundation_SessionData_t> Test_I_MediaFoundation_Message_t;
typedef Test_I_SessionMessage_T<Test_I_MediaFoundation_Message_t,
                                Test_I_CameraMSA_MediaFoundation_SessionData_t> Test_I_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_I_CameraMSA_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_DirectShow_Message_t,
                                    Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_ISessionNotify_t;
typedef std::list<Test_I_DirectShow_ISessionNotify_t*> Test_I_DirectShow_Subscribers_t;
typedef Test_I_DirectShow_Subscribers_t::iterator Test_I_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Test_I_CameraMSA_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_MediaFoundation_Message_t,
                                    Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_I_MediaFoundation_ISessionNotify_t*> Test_I_MediaFoundation_Subscribers_t;
typedef Test_I_MediaFoundation_Subscribers_t::iterator Test_I_MediaFoundation_SubscribersIterator_t;
#else
typedef Test_I_Message_T<struct Test_I_V4L_MessageData,
                         Test_I_CameraMSA_V4L_SessionData_t> Test_I_Message_t;
typedef Test_I_SessionMessage_T<Test_I_Message_t,
                                Test_I_CameraMSA_V4L_SessionData_t> Test_I_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_I_CameraMSA_V4L_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message_t,
                                    Test_I_SessionMessage_t> Test_I_ISessionNotify_t;
typedef std::list<Test_I_ISessionNotify_t*> Test_I_Subscribers_t;
typedef Test_I_Subscribers_t::iterator Test_I_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64
struct Test_I_CameraMSA_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_CameraMSA_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , deviceIdentifier ()
   , display ()
   , fullScreen (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , window (NULL)
#else
   , window (0)
#endif // ACE_WIN32 || ACE_WIN64
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  struct Stream_Device_Identifier                         deviceIdentifier; // source module
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Common_UI_DisplayDevice                          display; // display module
#else
  struct Common_UI_Display                                display; // display module
#endif // ACE_WIN32 || ACE_WIN64
  bool                                                    fullScreen;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                                    window;
#else
  Window                                                  window;
#endif // ACE_WIN32 || ACE_WIN64
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamConfiguration;
struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_DirectShow_StreamConfiguration,
                               struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration> Test_I_DirectShow_StreamConfiguration_t;
struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration
 : Test_I_CameraMSA_ModuleHandlerConfiguration
{
  Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_CameraMSA_ModuleHandlerConfiguration ()
   , area ()
   , builder (NULL)
   , direct3DConfiguration (NULL)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   //, sourceFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
   , windowController2 (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration operator= (const struct Test_I_CameraMSA_DirectShow_ModuleHandlerConfiguration& rhs_in)
  {
    area = rhs_in.area;
    if (builder)
    {
      builder->Release (); builder = NULL;
    } // end IF
    if (rhs_in.builder)
    {
      rhs_in.builder->AddRef ();
      builder = rhs_in.builder;
    } // end IF
    direct3DConfiguration = rhs_in.direct3DConfiguration;
    filterConfiguration = rhs_in.filterConfiguration;
    filterCLSID = rhs_in.filterCLSID;
//    if (outputFormat)
//      Stream_MediaFramework_DirectShow_Tools::delete_ (outputFormat);
    push = rhs_in.push;
    subscriber = rhs_in.subscriber;
    subscribers = rhs_in.subscribers;
    if (windowController)
    {
      windowController->Release (); windowController = NULL;
    } // end IF
    if (rhs_in.windowController)
    {
      rhs_in.windowController->AddRef ();
      windowController = rhs_in.windowController;
    } // end IF
    if (windowController2)
    {
      windowController2->Release (); windowController2 = NULL;
    } // end IF
    if (rhs_in.windowController2)
    {
      rhs_in.windowController2->AddRef ();
      windowController2 = rhs_in.windowController2;
    } // end IF

    return *this;
  }

  struct tagRECT                                        area;
  IGraphBuilder*                                        builder;
  struct Stream_MediaFramework_Direct3D_Configuration*  direct3DConfiguration;
  struct Test_I_DirectShow_FilterConfiguration*         filterConfiguration;
  CLSID                                                 filterCLSID;
  struct _AMMediaType                                   outputFormat;
  bool                                                  push;
  //struct _AMMediaType                                   sourceFormat;
  Test_I_DirectShow_ISessionNotify_t*                   subscriber;
  Test_I_DirectShow_Subscribers_t*                      subscribers;
  IVideoWindow*                                         windowController;
  IMFVideoDisplayControl*                               windowController2; // EVR
};

struct Test_I_MediaFoundation_StreamConfiguration;
struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_MediaFoundation_StreamConfiguration,
                               struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration> Test_I_MediaFoundation_StreamConfiguration_t;
struct Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_CameraMSA_ModuleHandlerConfiguration
{
  Test_I_CameraMSA_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_CameraMSA_ModuleHandlerConfiguration ()
   , area ()
   , direct3DConfiguration (NULL)
   , manageMediaSession (false)
   , mediaFoundationConfiguration (NULL)
   , outputFormat (NULL)
   , session (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
  }

  struct tagRECT                                              area;
  struct Stream_MediaFramework_Direct3D_Configuration*        direct3DConfiguration;
  bool                                                        manageMediaSession;
  struct Stream_MediaFramework_MediaFoundation_Configuration* mediaFoundationConfiguration;
  IMFMediaType*                                               outputFormat;
  IMFMediaSession*                                            session;
  Test_I_MediaFoundation_ISessionNotify_t*       subscriber;
  Test_I_MediaFoundation_Subscribers_t*          subscribers;
  IMFVideoDisplayControl*                                     windowController;
};
#else
struct Test_I_V4L_StreamConfiguration;
struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_V4L_StreamConfiguration,
                               struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration> Test_I_StreamConfiguration_t;
struct Test_I_CameraMSA_V4L_ModuleHandlerConfiguration
 : Test_I_CameraMSA_ModuleHandlerConfiguration
{
  Test_I_CameraMSA_V4L_ModuleHandlerConfiguration ()
   : Test_I_CameraMSA_ModuleHandlerConfiguration ()
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , outputFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
   , surface (NULL)
   , waylandDisplay (NULL)
  {
    // *PORTABILITY*: v4l2: device path (e.g. "[/dev/]video0")
    deviceIdentifier.identifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);

    ACE_OS::memset (&outputFormat, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
  }

  __u32                                      buffers; // v4l device buffers
  enum v4l2_memory                           method; // v4l camera source
  struct Stream_MediaFramework_V4L_MediaType outputFormat;
  Test_I_ISessionNotify_t*                   subscriber;
  Test_I_Subscribers_t*                      subscribers;
  struct wl_shell_surface*                   surface;
  struct wl_display*                         waylandDisplay;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_CameraMSA_DirectShow_StreamState
 : Stream_State
{
  Test_I_CameraMSA_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_CameraMSA_DirectShow_SessionData* sessionData;
};

struct Test_I_CameraMSA_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_CameraMSA_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_CameraMSA_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_I_CameraMSA_StreamState
 : Stream_State
{
  Test_I_CameraMSA_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_CameraMSA_V4L_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_I_CameraMSA_StreamConfiguration
 : Stream_Configuration
{
  Test_I_CameraMSA_StreamConfiguration ()
   : Stream_Configuration ()
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_INVALID)
  {
    printFinalReport = true;
  }
  enum Stream_Visualization_VideoRenderer renderer;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamConfiguration
 : Test_I_CameraMSA_StreamConfiguration
{
  Test_I_DirectShow_StreamConfiguration ()
   : Test_I_CameraMSA_StreamConfiguration ()
   , format ()
  {}

  struct _AMMediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_I_CameraMSA_DirectShow_StreamState> Test_I_DirectShow_IStreamControl_t;

struct Test_I_MediaFoundation_StreamConfiguration
 : Test_I_CameraMSA_StreamConfiguration
{
  Test_I_MediaFoundation_StreamConfiguration ()
   : Test_I_CameraMSA_StreamConfiguration ()
   , format (NULL)
  {}

  IMFMediaType* format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_I_CameraMSA_MediaFoundation_StreamState> Test_I_MediaFoundation_IStreamControl_t;
#else
struct Test_I_V4L_StreamConfiguration
 : Test_I_CameraMSA_StreamConfiguration
{
  Test_I_V4L_StreamConfiguration ()
   : Test_I_CameraMSA_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_V4L_MediaType format; // session data-
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_I_CameraMSA_StreamState> Test_I_IStreamControl_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  Test_I_DirectShow_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_I_DirectShow_StreamConfiguration_t             streamConfiguration;
};

struct Test_I_MediaFoundation_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  Test_I_MediaFoundation_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_I_MediaFoundation_StreamConfiguration_t        streamConfiguration;
};
#else
struct Test_I_V4L_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#endif // GTK_USE
#else
 : Test_I_Configuration
#endif // GUI_SUPPORT
{
  Test_I_V4L_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_I_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_DirectShow_Message_t,
                                          Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_MediaFoundation_Message_t,
                                          Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message_t,
                                          Test_I_SessionMessage_t> Test_I_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Test_I_DirectShow_ISessionNotify_t> Test_I_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Test_I_MediaFoundation_ISessionNotify_t> Test_I_MediaFoundation_ISubscribe_t;

typedef Test_I_EventHandler_T<Test_I_DirectShow_ISessionNotify_t,
                                           Test_I_DirectShow_Message_t,
                                           Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_EventHandler_t;
typedef Test_I_EventHandler_T<Test_I_MediaFoundation_ISessionNotify_t,
                                           Test_I_MediaFoundation_Message_t,
                                           Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Test_I_ISessionNotify_t> Test_I_ISubscribe_t;

typedef Test_I_EventHandler_T<Test_I_ISessionNotify_t,
                              Test_I_Message_t,
                              Test_I_SessionMessage_t> Test_I_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
class MSAFluidSolver2D;
struct Test_I_UI_GTK_CBData
 : Common_UI_GTK_CBData
{
  Test_I_UI_GTK_CBData ()
   : solver (NULL)
  {}

  MSAFluidSolver2D* solver;
};
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#endif // TEST_I_CAMERA_MSA_COMMON_H
