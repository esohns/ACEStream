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

#ifndef TEST_U_CAMERA_FILTER_COMMON_H
#define TEST_U_CAMERA_FILTER_COMMON_H

#include <list>
#include <map>
#include <string>

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

// #include "X11/X.h"
#undef CursorShape
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

//#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_tools.h"

#if defined (CURSES_SUPPORT)
#include "common_ui_curses_common.h"
#endif // CURSES_SUPPORT

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

#include "test_u_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;
#else
typedef unsigned long XID;
typedef XID Window;
#endif // ACE_WIN32 || ACE_WIN64
class Stream_IAllocator;
template <typename NotificationType,
          typename DataMessageType,
          typename SessionMessageType>
class Test_U_EventHandler_T;
template <typename DataType,
          typename SessionDataType>
class Test_U_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Test_U_SessionMessage_T;

struct Test_U_StatisticData
 : Stream_Statistic
{
  Test_U_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Test_U_StatisticData operator+= (const struct Test_U_StatisticData& rhs_in)
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
typedef Common_StatisticHandler_T<struct Test_U_StatisticData> Test_U_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_StreamState;
class Test_U_CameraFilter_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct _AMMediaType,
                                        struct Test_U_DirectShow_StreamState,
                                        struct Test_U_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_U_CameraFilter_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct _AMMediaType,
                                   struct Test_U_DirectShow_StreamState,
                                   struct Test_U_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , resetToken (0)
  {}

  Test_U_CameraFilter_DirectShow_SessionData& operator= (const Test_U_CameraFilter_DirectShow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct _AMMediaType,
                                  struct Test_U_DirectShow_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);

    return *this;
  }
  Test_U_CameraFilter_DirectShow_SessionData& operator+= (const Test_U_CameraFilter_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct _AMMediaType,
                                  struct Test_U_DirectShow_StreamState,
                                  struct Test_U_StatisticData,
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
typedef Stream_SessionData_T<Test_U_CameraFilter_DirectShow_SessionData> Test_U_CameraFilter_DirectShow_SessionData_t;

struct Test_U_MediaFoundation_StreamState;
class Test_U_CameraFilter_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        IMFMediaType*,
                                        struct Test_U_MediaFoundation_StreamState,
                                        struct Test_U_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_U_CameraFilter_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   IMFMediaType*,
                                   struct Test_U_MediaFoundation_StreamState,
                                   struct Test_U_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
  {}

  Test_U_CameraFilter_MediaFoundation_SessionData& operator= (const Test_U_CameraFilter_MediaFoundation_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  IMFMediaType*,
                                  struct Test_U_MediaFoundation_StreamState,
                                  struct Test_U_StatisticData,
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
  Test_U_CameraFilter_MediaFoundation_SessionData& operator+= (const Test_U_CameraFilter_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  IMFMediaType*,
                                  struct Test_U_MediaFoundation_StreamState,
                                  struct Test_U_StatisticData,
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

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*                 direct3DDevice;
#else
  IDirect3DDevice9*                   direct3DDevice;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  UINT                                direct3DManagerResetToken;
  TOPOID                              rendererNodeId;
  UINT                                resetToken;
  IMFMediaSession*                    session;
};
typedef Stream_SessionData_T<Test_U_CameraFilter_MediaFoundation_SessionData> Test_U_CameraFilter_MediaFoundation_SessionData_t;
#else
struct Test_U_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                      struct Stream_MediaFramework_V4L_MediaType,
                                      struct Test_U_StreamState,
                                      struct Test_U_StatisticData,
                                      struct Stream_UserData> Test_U_CameraFilter_V4L_SessionData;
typedef Stream_SessionData_T<Test_U_CameraFilter_V4L_SessionData> Test_U_CameraFilter_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_U_Message_T<struct Test_U_DirectShow_MessageData,
                         Test_U_CameraFilter_DirectShow_SessionData_t> Test_U_DirectShow_Message_t;
typedef Test_U_SessionMessage_T<Test_U_DirectShow_Message_t,
                                Test_U_CameraFilter_DirectShow_SessionData_t> Test_U_DirectShow_SessionMessage_t;
typedef Test_U_Message_T<struct Test_U_MediaFoundation_MessageData,
                         Test_U_CameraFilter_MediaFoundation_SessionData_t> Test_U_MediaFoundation_Message_t;
typedef Test_U_SessionMessage_T<Test_U_MediaFoundation_Message_t,
                                Test_U_CameraFilter_MediaFoundation_SessionData_t> Test_U_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_U_CameraFilter_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_DirectShow_Message_t,
                                    Test_U_DirectShow_SessionMessage_t> Test_U_DirectShow_ISessionNotify_t;
typedef std::list<Test_U_DirectShow_ISessionNotify_t*> Test_U_DirectShow_Subscribers_t;
typedef Test_U_DirectShow_Subscribers_t::iterator Test_U_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Test_U_CameraFilter_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_MediaFoundation_Message_t,
                                    Test_U_MediaFoundation_SessionMessage_t> Test_U_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_U_MediaFoundation_ISessionNotify_t*> Test_U_MediaFoundation_Subscribers_t;
typedef Test_U_MediaFoundation_Subscribers_t::iterator Test_U_MediaFoundation_SubscribersIterator_t;
#else
typedef Test_U_Message_T<struct Test_U_V4L2_MessageData,
                         Test_U_CameraFilter_V4L_SessionData_t> Test_U_Message_t;
typedef Test_U_SessionMessage_T<Test_U_Message_t,
                                Test_U_CameraFilter_V4L_SessionData_t> Test_U_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_U_CameraFilter_V4L_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message_t,
                                    Test_U_SessionMessage_t> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_CameraFilter_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_CameraFilter_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , display ()
   , fullScreen (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , window (NULL)
#else
   , window (0)
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
#endif // ACE_WIN32 || ACE_WIN64
  }

  struct Stream_Device_Identifier deviceIdentifier; // source module
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Common_UI_DisplayDevice  display; // display module
#else
  struct Common_UI_Display        display; // display module
#endif // ACE_WIN32 || ACE_WIN64
  bool                            fullScreen;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                            window;
#else
  Window                          window;
#endif // ACE_WIN32 || ACE_WIN64
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_CameraFilter_DirectShow_StreamConfiguration;
struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_CameraFilter_DirectShow_StreamConfiguration,
                               struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration> Test_U_DirectShow_StreamConfiguration_t;
struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration
 : Test_U_CameraFilter_ModuleHandlerConfiguration
{
  Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_CameraFilter_ModuleHandlerConfiguration ()
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

  struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration operator= (const struct Test_U_CameraFilter_DirectShow_ModuleHandlerConfiguration& rhs_in)
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
  struct Test_U_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                 filterCLSID;
  struct _AMMediaType                                   outputFormat;
  bool                                                  push;
  //struct _AMMediaType                                   sourceFormat;
  Test_U_DirectShow_ISessionNotify_t*           subscriber;
  Test_U_DirectShow_Subscribers_t*              subscribers;
  IVideoWindow*                                         windowController;
  IMFVideoDisplayControl*                               windowController2; // EVR
};

struct Test_U_CameraFilter_MediaFoundation_StreamConfiguration;
struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_CameraFilter_MediaFoundation_StreamConfiguration,
                               struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration> Test_U_MediaFoundation_StreamConfiguration_t;
struct Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_CameraFilter_ModuleHandlerConfiguration
{
  Test_U_CameraFilter_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_CameraFilter_ModuleHandlerConfiguration ()
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
  Test_U_MediaFoundation_ISessionNotify_t*       subscriber;
  Test_U_MediaFoundation_Subscribers_t*          subscribers;
  IMFVideoDisplayControl*                                     windowController;
};
#else
struct Test_U_CameraFilter_V4L_StreamConfiguration;
struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_CameraFilter_V4L_StreamConfiguration,
                               struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
struct Test_U_CameraFilter_V4L_ModuleHandlerConfiguration
 : Test_U_CameraFilter_ModuleHandlerConfiguration
{
  Test_U_CameraFilter_V4L_ModuleHandlerConfiguration ()
   : Test_U_CameraFilter_ModuleHandlerConfiguration ()
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
// #if defined (FFMPEG_SUPPORT)
//    , codecFormat (AV_PIX_FMT_NONE)
//    , codecId (AV_CODEC_ID_NONE)
// #endif // FFMPEG_SUPPORT
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
// #if defined (FFMPEG_SUPPORT)
//   enum AVPixelFormat                         codecFormat; // preferred output-
//   enum AVCodecID                             codecId;
// #endif // FFMPEG_SUPPORT
  enum v4l2_memory                           method; // v4l camera source
  struct Stream_MediaFramework_V4L_MediaType outputFormat;
  Test_U_ISessionNotify_t*                   subscriber;
  Test_U_Subscribers_t*                      subscribers;
  struct wl_shell_surface*                   surface;
  struct wl_display*                         waylandDisplay;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_StreamState
 : Stream_State
{
  Test_U_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_CameraFilter_DirectShow_SessionData* sessionData;
};

struct Test_U_MediaFoundation_StreamState
 : Stream_State
{
  Test_U_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_CameraFilter_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_U_StreamState
 : Stream_State
{
  Test_U_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_CameraFilter_V4L_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

enum Test_U_CameraFilter_Mode
{
  TEST_U_MODE_SOBEL = 0,
  TEST_U_MODE_PERLIN_NOISE,
  TEST_U_MODE_MARCHING_SQUARES,
  TEST_U_MODE_WEIGHTED_VORONOI_STIPPLE,
  TEST_U_MODE_GLUT,
  TEST_U_MODE_GLUT_2,
  TEST_U_MODE_GLUT_3
};

struct Test_U_CameraFilter_StreamConfiguration
 : Stream_Configuration
{
  Test_U_CameraFilter_StreamConfiguration ()
   : Stream_Configuration ()
   , mode (TEST_U_MODE_SOBEL)
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_INVALID)
  {
    printFinalReport = true;
  }

  enum Test_U_CameraFilter_Mode           mode;
  enum Stream_Visualization_VideoRenderer renderer;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_CameraFilter_DirectShow_StreamConfiguration
 : Test_U_CameraFilter_StreamConfiguration
{
  Test_U_CameraFilter_DirectShow_StreamConfiguration ()
   : Test_U_CameraFilter_StreamConfiguration ()
   , format ()
  {}

  struct _AMMediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_DirectShow_StreamState> Test_U_DirectShow_IStreamControl_t;

struct Test_U_CameraFilter_MediaFoundation_StreamConfiguration
 : Test_U_CameraFilter_StreamConfiguration
{
  Test_U_CameraFilter_MediaFoundation_StreamConfiguration ()
   : Test_U_CameraFilter_StreamConfiguration ()
   , format (NULL)
  {}

  IMFMediaType* format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_MediaFoundation_StreamState> Test_U_MediaFoundation_IStreamControl_t;
#else
struct Test_U_CameraFilter_V4L_StreamConfiguration
 : Test_U_CameraFilter_StreamConfiguration
{
  Test_U_CameraFilter_V4L_StreamConfiguration ()
   : Test_U_CameraFilter_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_V4L_MediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_StreamState> Test_U_IStreamControl_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_DirectShow_Configuration
 : Test_U_Configuration
{
  Test_U_DirectShow_Configuration ()
   : Test_U_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_U_DirectShow_StreamConfiguration_t             streamConfiguration;
};

struct Test_U_MediaFoundation_Configuration
 : Test_U_Configuration
{
  Test_U_MediaFoundation_Configuration ()
   : Test_U_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Test_U_MediaFoundation_StreamConfiguration_t        streamConfiguration;
};
#else
struct Test_U_V4L_Configuration
 : Test_U_Configuration
{
  Test_U_V4L_Configuration ()
   : Test_U_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Test_U_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Test_U_Message,
//                                         Test_U_SessionMessage> Test_U_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_DirectShow_Message_t,
                                          Test_U_DirectShow_SessionMessage_t> Test_U_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_MediaFoundation_Message_t,
                                          Test_U_MediaFoundation_SessionMessage_t> Test_U_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message_t,
                                          Test_U_SessionMessage_t> Test_U_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Test_U_DirectShow_ISessionNotify_t> Test_U_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Test_U_MediaFoundation_ISessionNotify_t> Test_U_MediaFoundation_ISubscribe_t;

typedef Test_U_EventHandler_T<Test_U_DirectShow_ISessionNotify_t,
                              Test_U_DirectShow_Message_t,
                              Test_U_DirectShow_SessionMessage_t> Test_U_DirectShow_EventHandler_t;
typedef Test_U_EventHandler_T<Test_U_MediaFoundation_ISessionNotify_t,
                              Test_U_MediaFoundation_Message_t,
                              Test_U_MediaFoundation_SessionMessage_t> Test_U_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Test_U_ISessionNotify_t> Test_U_ISubscribe_t;

typedef Test_U_EventHandler_T<Test_U_ISessionNotify_t,
                              Test_U_Message_t,
                              Test_U_SessionMessage_t> Test_U_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
