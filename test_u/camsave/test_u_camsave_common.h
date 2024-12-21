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

#ifndef TEST_U_CAMSAVE_COMMON_H
#define TEST_U_CAMSAVE_COMMON_H

#include <list>
#include <map>
#include <string>

#include "ace/config-lite.h"
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

#if defined (LIBCAMERA_SUPPORT)
#include "libcamera/libcamera.h"
#endif // LIBCAMERA_SUPPORT

#include "X11/X.h"

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

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#undef DrawText
#undef SIZEOF_SIZE_T
#include "wx/apptrait.h"
#include "wx/window.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_inotify.h"
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
#include "stream_lib_ffmpeg_common.h"
#include "stream_lib_v4l_common.h"
#if defined (LIBCAMERA_SUPPORT)
#include "stream_lib_libcamera_common.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_u_common.h"
#include "test_u_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_u_qt_common.h"
#endif // QT_SUPPORT
#if defined (WXWIDGETS_SUPPORT)
#include "test_u_wxwidgets_common.h"

#include "camsave_wxwidgets_ui.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;
#else
struct _XDisplay;
template <typename DataType,
          typename SessionDataType> // derives off Stream_SessionData_T
class Stream_CamSave_LibCamera_Message_T;
#endif // ACE_WIN32 || ACE_WIN64
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
class Stream_CamSave_EventHandler_T;
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_SUPPORT)
template <typename WidgetBaseClassType,
          typename InterfaceType,
          typename StreamType>
class Stream_CamSave_WxWidgetsDialog_T;
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

enum Stream_Camsave_ProgramMode
{
  STREAM_CAMSAVE_PROGRAMMODE_PRINT_VERSION = 0,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  STREAM_CAMSAVE_PROGRAMMODE_TEST_METHODS,
#endif // ACE_WIN32 || ACE_WIN64
  STREAM_CAMSAVE_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  STREAM_CAMSAVE_PROGRAMMODE_MAX,
  STREAM_CAMSAVE_PROGRAMMODE_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_MessageData
{
  Stream_CamSave_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Stream_CamSave_DirectShow_MessageData> Stream_CamSave_DirectShow_MessageData_t;

struct Stream_CamSave_MediaFoundation_MessageData
{
  Stream_CamSave_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Stream_CamSave_MediaFoundation_MessageData> Stream_CamSave_MediaFoundation_MessageData_t;
#else
struct Stream_CamSave_V4L_MessageData
{
  Stream_CamSave_V4L_MessageData ()
   : fileDescriptor (-1)
   , index (0)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         fileDescriptor; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};

#if defined (LIBCAMERA_SUPPORT)
namespace libcamera {
class FrameBuffer;
class Camera;
class Request;
class Stream;
} /* namespace libcamera */
struct Stream_CamSave_LibCamera_MessageData
{
  Stream_CamSave_LibCamera_MessageData ()
   : buffer (NULL)
   , camera (NULL)
   , release (false)
   , request (NULL)
   , stream (NULL)
  {}

  libcamera::FrameBuffer* buffer;
  libcamera::Camera*      camera;
  bool                    release;
  libcamera::Request*     request;
  libcamera::Stream*      stream;
};
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CamSave_StatisticData
 : Stream_Statistic
{
  Stream_CamSave_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Stream_CamSave_StatisticData operator+= (const struct Stream_CamSave_StatisticData& rhs_in)
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
typedef Common_StatisticHandler_T<struct Stream_CamSave_StatisticData> Test_U_CamSave_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_StreamState;
class Stream_CamSave_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                        struct _AMMediaType,
                                        struct Stream_CamSave_DirectShow_StreamState,
                                        struct Stream_CamSave_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_CamSave_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                   struct _AMMediaType,
                                   struct Stream_CamSave_DirectShow_StreamState,
                                   struct Stream_CamSave_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , resetToken (0)
  {}

  Stream_CamSave_DirectShow_SessionData& operator= (const Stream_CamSave_DirectShow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  struct _AMMediaType,
                                  struct Stream_CamSave_DirectShow_StreamState,
                                  struct Stream_CamSave_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);

    return *this;
  }
  Stream_CamSave_DirectShow_SessionData& operator+= (const Stream_CamSave_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  struct _AMMediaType,
                                  struct Stream_CamSave_DirectShow_StreamState,
                                  struct Stream_CamSave_StatisticData,
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
typedef Stream_SessionData_T<Stream_CamSave_DirectShow_SessionData> Stream_CamSave_DirectShow_SessionData_t;

struct Stream_CamSave_MediaFoundation_StreamState;
class Stream_CamSave_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                        IMFMediaType*,
                                        struct Stream_CamSave_MediaFoundation_StreamState,
                                        struct Stream_CamSave_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_CamSave_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                   IMFMediaType*,
                                   struct Stream_CamSave_MediaFoundation_StreamState,
                                   struct Stream_CamSave_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
  {}

  Stream_CamSave_MediaFoundation_SessionData& operator= (const Stream_CamSave_MediaFoundation_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  IMFMediaType*,
                                  struct Stream_CamSave_MediaFoundation_StreamState,
                                  struct Stream_CamSave_StatisticData,
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
  Stream_CamSave_MediaFoundation_SessionData& operator+= (const Stream_CamSave_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                  IMFMediaType*,
                                  struct Stream_CamSave_MediaFoundation_StreamState,
                                  struct Stream_CamSave_StatisticData,
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
typedef Stream_SessionData_T<Stream_CamSave_MediaFoundation_SessionData> Stream_CamSave_MediaFoundation_SessionData_t;
#else
struct Stream_CamSave_V4L_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                      struct Stream_MediaFramework_V4L_MediaType,
                                      struct Stream_CamSave_V4L_StreamState,
                                      struct Stream_CamSave_StatisticData,
                                      struct Stream_UserData> Stream_CamSave_V4L_SessionData;
typedef Stream_SessionData_T<Stream_CamSave_V4L_SessionData> Stream_CamSave_V4L_SessionData_t;

#if defined (LIBCAMERA_SUPPORT)
struct Stream_CamSave_LibCamera_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Stream_SessionData,
                                      struct Stream_MediaFramework_LibCamera_MediaType,
                                      struct Stream_CamSave_LibCamera_StreamState,
                                      struct Stream_CamSave_StatisticData,
                                      struct Stream_UserData> Stream_CamSave_LibCamera_SessionData;
typedef Stream_SessionData_T<Stream_CamSave_LibCamera_SessionData> Stream_CamSave_LibCamera_SessionData_t;
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CamSave_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Stream_CamSave_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

template <typename DataType,
          typename SessionDataType>
class Stream_CamSave_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_CamSave_SessionMessage_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_CamSave_Message_T<struct Stream_CamSave_DirectShow_MessageData,
                                 Stream_CamSave_DirectShow_SessionData_t> Stream_CamSave_DirectShow_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_DirectShow_Message_t,
                                        Stream_CamSave_DirectShow_SessionData_t> Stream_CamSave_DirectShow_SessionMessage_t;
typedef Stream_CamSave_Message_T<struct Stream_CamSave_MediaFoundation_MessageData,
                                 Stream_CamSave_MediaFoundation_SessionData_t> Stream_CamSave_MediaFoundation_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_MediaFoundation_Message_t,
                                        Stream_CamSave_MediaFoundation_SessionData_t> Stream_CamSave_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_CamSave_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_DirectShow_Message_t,
                                    Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_ISessionNotify_t;
typedef std::list<Stream_CamSave_DirectShow_ISessionNotify_t*> Stream_CamSave_DirectShow_Subscribers_t;
typedef Stream_CamSave_DirectShow_Subscribers_t::iterator Stream_CamSave_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Stream_CamSave_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_MediaFoundation_Message_t,
                                    Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_ISessionNotify_t;
typedef std::list<Stream_CamSave_MediaFoundation_ISessionNotify_t*> Stream_CamSave_MediaFoundation_Subscribers_t;
typedef Stream_CamSave_MediaFoundation_Subscribers_t::iterator Stream_CamSave_MediaFoundation_SubscribersIterator_t;
#else
typedef Stream_CamSave_Message_T<struct Stream_CamSave_V4L_MessageData,
                                 Stream_CamSave_V4L_SessionData_t> Stream_CamSave_V4L_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_V4L_Message_t,
                                        Stream_CamSave_V4L_SessionData_t> Stream_CamSave_V4L_SessionMessage_t;
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_CamSave_LibCamera_Message_T<struct Stream_CamSave_LibCamera_MessageData,
                                           Stream_CamSave_LibCamera_SessionData_t> Stream_CamSave_LibCamera_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_LibCamera_Message_t,
                                        Stream_CamSave_LibCamera_SessionData_t> Stream_CamSave_LibCamera_SessionMessage_t;
#endif // LIBCAMERA_SUPPORT

typedef Stream_ISessionDataNotify_T<Stream_CamSave_V4L_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_V4L_Message_t,
                                    Stream_CamSave_V4L_SessionMessage_t> Stream_CamSave_V4L_ISessionNotify_t;
typedef std::list<Stream_CamSave_V4L_ISessionNotify_t*> Stream_CamSave_V4L_Subscribers_t;
typedef Stream_CamSave_V4L_Subscribers_t::iterator Stream_CamSave_V4L_SubscribersIterator_t;
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_ISessionDataNotify_T<Stream_CamSave_LibCamera_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_LibCamera_Message_t,
                                    Stream_CamSave_LibCamera_SessionMessage_t> Stream_CamSave_LibCamera_ISessionNotify_t;
typedef std::list<Stream_CamSave_LibCamera_ISessionNotify_t*> Stream_CamSave_LibCamera_Subscribers_t;
typedef Stream_CamSave_LibCamera_Subscribers_t::iterator Stream_CamSave_LibCamera_SubscribersIterator_t;
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
struct Stream_CamSave_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Stream_CamSave_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , deviceIdentifier ()
   , display ()
   , fullScreen (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , shaderFile ()
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
   , window (NULL)
#endif // GUI_SUPPORT
   , targetFileName ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  struct Stream_Device_Identifier deviceIdentifier; // source/renderer module
  struct Common_UI_DisplayDevice  display;
  bool                            fullScreen;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  std::string                     shaderFile;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT) && defined (GTK_USE)
  GdkWindow*                      window;
#else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                            window;
#else
  Window                          window;
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTK_SUPPORT && GTK_USE
#endif // GUI_SUPPORT
  std::string                     targetFileName;
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_StreamConfiguration;
struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CamSave_DirectShow_StreamConfiguration,
                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration> Stream_CamSave_DirectShow_StreamConfiguration_t;
struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_DirectShow_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
   , area ()
   , builder (NULL)
   , direct3DConfiguration (NULL)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , preview (STREAM_DEV_CAM_VIDEOFORWINDOW_DEFAULT_PREVIEW_MODE)
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
   , windowController2 (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration operator= (const struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration& rhs_in)
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
  struct Stream_CamSave_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                 filterCLSID;
  struct _AMMediaType                                   outputFormat;
  bool                                                  preview; // VfW
  bool                                                  push;
  Stream_CamSave_DirectShow_ISessionNotify_t*           subscriber;
  Stream_CamSave_DirectShow_Subscribers_t*              subscribers;
  IVideoWindow*                                         windowController;
  IMFVideoDisplayControl*                               windowController2; // EVR
};

struct Stream_CamSave_MediaFoundation_StreamConfiguration;
struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CamSave_MediaFoundation_StreamConfiguration,
                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration> Stream_CamSave_MediaFoundation_StreamConfiguration_t;
struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
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
  Stream_CamSave_MediaFoundation_ISessionNotify_t*            subscriber;
  Stream_CamSave_MediaFoundation_Subscribers_t*               subscribers;
  IMFVideoDisplayControl*                                     windowController;
};
#else
struct Stream_CamSave_V4L_StreamConfiguration;
struct Stream_CamSave_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CamSave_V4L_StreamConfiguration,
                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration> Stream_CamSave_V4L_StreamConfiguration_t;
struct Stream_CamSave_V4L_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_V4L_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
   , area ()
#endif // GUI_SUPPORT
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , outputFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
#if defined (GUI_SUPPORT)
    ACE_OS::memset (&area, 0, sizeof (struct v4l2_rect));
#endif // GUI_SUPPORT

    // *PORTABILITY*: v4l2: device path (e.g. "[/dev/]video0")
    deviceIdentifier.identifier =
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEVICE_DIRECTORY);
    deviceIdentifier.identifier += ACE_DIRECTORY_SEPARATOR_CHAR_A;
    deviceIdentifier.identifier +=
      ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);

    ACE_OS::memset (&outputFormat, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
  }

#if defined (GUI_SUPPORT)
  struct v4l2_rect                           area;
#endif // GUI_SUPPORT
  __u32                                      buffers; // v4l device buffers
  enum v4l2_memory                           method; // v4l camera source
  struct Stream_MediaFramework_V4L_MediaType outputFormat;
  Stream_CamSave_V4L_ISessionNotify_t*       subscriber;
  Stream_CamSave_V4L_Subscribers_t*          subscribers;
};
#if defined (LIBCAMERA_SUPPORT)
struct Stream_CamSave_LibCamera_StreamConfiguration;
struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CamSave_LibCamera_StreamConfiguration,
                               struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration> Stream_CamSave_LibCamera_StreamConfiguration_t;
struct Stream_CamSave_LibCamera_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_LibCamera_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecFormat (AV_PIX_FMT_NONE)
   , codecId (AV_CODEC_ID_NONE)
#endif // FFMPEG_SUPPORT
   , outputFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
    ACE_OS::memset (&outputFormat, 0, sizeof (struct Stream_MediaFramework_LibCamera_MediaType));
  }

#if defined (FFMPEG_SUPPORT)
  enum AVPixelFormat                               codecFormat; // preferred output-
  enum AVCodecID                                   codecId;
#endif // FFMPEG_SUPPORT
  struct Stream_MediaFramework_LibCamera_MediaType outputFormat;
  Stream_CamSave_LibCamera_ISessionNotify_t*       subscriber;
  Stream_CamSave_LibCamera_Subscribers_t*          subscribers;
};
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_StreamState
 : Stream_State
{
  Stream_CamSave_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  Stream_CamSave_DirectShow_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

struct Stream_CamSave_MediaFoundation_StreamState
 : Stream_State
{
  Stream_CamSave_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  Stream_CamSave_MediaFoundation_SessionData* sessionData;

  struct Stream_UserData*             userData;
};
#else
struct Stream_CamSave_V4L_StreamState
 : Stream_State
{
  Stream_CamSave_V4L_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Stream_CamSave_V4L_SessionData* sessionData;
};
#if defined (LIBCAMERA_SUPPORT)
struct Stream_CamSave_LibCamera_StreamState
 : Stream_State
{
  Stream_CamSave_LibCamera_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Stream_CamSave_LibCamera_SessionData* sessionData;
};
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CamSave_StreamConfiguration
 : Stream_Configuration
{
  Stream_CamSave_StreamConfiguration ()
   : Stream_Configuration ()
   , capturer (STREAM_DEVICE_CAPTURER_INVALID)
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_INVALID)
  {}

  enum Stream_Device_Capturer             capturer;
  enum Stream_Visualization_VideoRenderer renderer;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_StreamConfiguration
 : Stream_CamSave_StreamConfiguration
{
  Stream_CamSave_DirectShow_StreamConfiguration ()
   : Stream_CamSave_StreamConfiguration ()
   , format ()
  {}

  struct _AMMediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CamSave_DirectShow_StreamState> Stream_CamSave_DirectShow_IStreamControl_t;

struct Stream_CamSave_MediaFoundation_StreamConfiguration
 : Stream_CamSave_StreamConfiguration
{
  Stream_CamSave_MediaFoundation_StreamConfiguration ()
   : Stream_CamSave_StreamConfiguration ()
   , format (NULL)
  {}

  IMFMediaType* format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CamSave_MediaFoundation_StreamState> Stream_CamSave_MediaFoundation_IStreamControl_t;
#else
#if defined (LIBCAMERA_SUPPORT)
struct Stream_CamSave_LibCamera_StreamConfiguration
 : Stream_CamSave_StreamConfiguration
{
  Stream_CamSave_LibCamera_StreamConfiguration ()
   : Stream_CamSave_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_LibCamera_MediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CamSave_LibCamera_StreamState> Stream_CamSave_LibCamera_IStreamControl_t;
#endif // LIBCAMERA_SUPPORT
struct Stream_CamSave_V4L_StreamConfiguration
 : Stream_CamSave_StreamConfiguration
{
  Stream_CamSave_V4L_StreamConfiguration ()
   : Stream_CamSave_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_V4L_MediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CamSave_V4L_StreamState> Stream_CamSave_V4L_IStreamControl_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#endif // GUI_SUPPORT
{
  Stream_CamSave_DirectShow_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration    signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Stream_CamSave_DirectShow_StreamConfiguration_t     streamConfiguration;

  struct Stream_UserData                      userData;
};

struct Stream_CamSave_MediaFoundation_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#endif // GUI_SUPPORT
{
  Stream_CamSave_MediaFoundation_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration     signalHandlerConfiguration;
  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration  direct3DConfiguration;
  Stream_CamSave_MediaFoundation_StreamConfiguration_t streamConfiguration;

  struct Stream_UserData                       userData;
};
#else
struct Stream_CamSave_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#else
 : Test_U_Configuration
#endif // GUI_SUPPORT
{
  Stream_CamSave_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#else
   : Test_U_Configuration ()
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , v4l_streamConfiguration ()
#if defined (LIBCAMERA_SUPPORT)
   , libCamera_streamConfiguration ()
#endif // LIBCAMERA_SUPPORT
  {}

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_CamSave_V4L_StreamConfiguration_t         v4l_streamConfiguration;
#if defined (LIBCAMERA_SUPPORT)
  Stream_CamSave_LibCamera_StreamConfiguration_t   libCamera_streamConfiguration;
#endif // LIBCAMERA_SUPPORT
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Stream_CamSave_Message,
//                                         Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CamSave_DirectShow_Message_t,
                                          Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CamSave_MediaFoundation_Message_t,
                                          Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CamSave_V4L_Message_t,
                                          Stream_CamSave_V4L_SessionMessage_t> Stream_CamSave_V4L_MessageAllocator_t;
#if defined (LIBCAMERA_SUPPORT)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CamSave_LibCamera_Message_t,
                                          Stream_CamSave_LibCamera_SessionMessage_t> Stream_CamSave_LibCamera_MessageAllocator_t;
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_CamSave_DirectShow_UI_CBData> Stream_CamSave_DirectShow_WxWidgetsIApplication_t;
struct Stream_CamSave_MediaFoundation_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_CamSave_MediaFoundation_UI_CBData> Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t;

class Stream_CamSave_DirectShow_Stream;
typedef Stream_CamSave_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                         Stream_CamSave_DirectShow_Stream> Stream_CamSave_DirectShow_WxWidgetsDialog_t;
class Stream_CamSave_MediaFoundation_Stream;
typedef Stream_CamSave_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                         Stream_CamSave_MediaFoundation_Stream> Stream_CamSave_MediaFoundation_WxWidgetsDialog_t;
#else
struct Stream_CamSave_V4L_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_CamSave_V4L_UI_CBData> Stream_CamSave_V4L_WxWidgetsIApplication_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Stream_CamSave_DirectShow_ISessionNotify_t> Stream_CamSave_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_CamSave_MediaFoundation_ISessionNotify_t> Stream_CamSave_MediaFoundation_ISubscribe_t;

typedef Stream_CamSave_EventHandler_T<Stream_CamSave_DirectShow_ISessionNotify_t,
                                      Stream_CamSave_DirectShow_Message_t,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                      Common_UI_GTK_State_t,
#elif defined (WXWIDGETS_USE)
                                      struct Common_UI_wxWidgets_State,
                                      Common_UI_wxWidgets_IApplicationBase_t,
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
                                      Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_EventHandler_t;
typedef Stream_CamSave_EventHandler_T<Stream_CamSave_MediaFoundation_ISessionNotify_t,
                                      Stream_CamSave_MediaFoundation_Message_t,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                      Common_UI_GTK_State_t,
#elif defined (WXWIDGETS_USE)
                                      struct Common_UI_wxWidgets_State,
                                      Common_UI_wxWidgets_IApplicationBase_t,
#endif // GTK_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
                                      Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Stream_CamSave_V4L_ISessionNotify_t> Stream_CamSave_V4L_ISubscribe_t;
typedef Stream_CamSave_EventHandler_T<Stream_CamSave_V4L_ISessionNotify_t,
                                      Stream_CamSave_V4L_Message_t,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                      Common_UI_GTK_State_t,
#elif defined (QT_USE)
                                      struct Common_UI_Qt_State,
#elif defined (WXWIDGETS_USE)
                                      struct Common_UI_wxWidgets_State,
                                      Common_UI_wxWidgets_IApplicationBase_t,
#else
                                      struct Common_UI_State,
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
                                      Stream_CamSave_V4L_SessionMessage_t> Stream_CamSave_V4L_EventHandler_t;

#if defined (LIBCAMERA_SUPPORT)
typedef Common_ISubscribe_T<Stream_CamSave_LibCamera_ISessionNotify_t> Stream_CamSave_LibCamera_ISubscribe_t;
typedef Stream_CamSave_EventHandler_T<Stream_CamSave_LibCamera_ISessionNotify_t,
                                      Stream_CamSave_LibCamera_Message_t,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                      Common_UI_GTK_State_t,
#elif defined (QT_USE)
                                      struct Common_UI_Qt_State,
#elif defined (WXWIDGETS_USE)
                                      struct Common_UI_wxWidgets_State,
                                      Common_UI_wxWidgets_IApplicationBase_t,
#else
                                      struct Common_UI_State,
#endif // GTK_USE || QT_USE || WXWIDGETS_USE
#endif // GUI_SUPPORT
                                      Stream_CamSave_LibCamera_SessionMessage_t> Stream_CamSave_LibCamera_EventHandler_t;
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Stream_CamSave_ProgressData
 : Test_U_UI_ProgressData
{
  Stream_CamSave_ProgressData ()
   : Test_U_UI_ProgressData ()
   , statistic ()
   , lastStatistic ()
   , timestamp (ACE_Time_Value::zero)
  {}

  struct Stream_CamSave_StatisticData statistic;
  struct Stream_CamSave_StatisticData lastStatistic;
  ACE_Time_Value                      timestamp;
};

struct Stream_CamSave_UI_CBData
 : Test_U_UI_CBData
{
  Stream_CamSave_UI_CBData ()
   : Test_U_UI_CBData ()
   , dispatch (NULL)
   , eventSourceId (0)
   , isFirst (true)
   , progressData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , useLibCamera (false)
#endif // ACE_WIN32 || defined ACE_WIN64
  {
    progressData.state = UIState;
  }

  Common_IDispatch*                  dispatch;
  guint                              eventSourceId; // display update-
  bool                               isFirst; // first activation ?
  struct Stream_CamSave_ProgressData progressData;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool                               useLibCamera;
#endif // ACE_WIN32 || defined ACE_WIN64
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_CamSave_DirectShow_Stream;
struct Stream_CamSave_DirectShow_UI_CBData
 : Stream_CamSave_UI_CBData
{
  Stream_CamSave_DirectShow_UI_CBData ()
   : Stream_CamSave_UI_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {}

  struct Stream_CamSave_DirectShow_Configuration* configuration;
  Stream_CamSave_DirectShow_Stream*               stream;
  IAMStreamConfig*                                streamConfiguration;
  Stream_CamSave_DirectShow_Subscribers_t         subscribers;
};

class Stream_CamSave_MediaFoundation_Stream;
struct Stream_CamSave_MediaFoundation_UI_CBData
 : Stream_CamSave_UI_CBData
{
  Stream_CamSave_MediaFoundation_UI_CBData ()
   : Stream_CamSave_UI_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
  {}

  struct Stream_CamSave_MediaFoundation_Configuration* configuration;
  Stream_CamSave_MediaFoundation_Stream*               stream;
  Stream_CamSave_MediaFoundation_Subscribers_t         subscribers;
};
#else
class Stream_CamSave_V4L_Stream;
struct Stream_CamSave_V4L_UI_CBData
 : Stream_CamSave_UI_CBData
{
  Stream_CamSave_V4L_UI_CBData ()
   : Stream_CamSave_UI_CBData ()
   , configuration (NULL)
#if defined (GTK_USE)
//   , pixelBuffer (NULL)
//   , pixelBufferLock (NULL)
#endif // GTK_USE
   , stream (NULL)
   , subscribers ()
  {
#if defined (GTK_USE)
//    pixelBufferLock = &UIState->lock;
#endif // GTK_USE
  }

  struct Stream_CamSave_Configuration* configuration;
#if defined (GTK_USE)
//#if GTK_CHECK_VERSION(3,0,0)
//  cairo_surface_t*                     pixelBuffer;
//#elif GTK_CHECK_VERSION(2,0,0)
//  GdkPixbuf*                           pixelBuffer;
//#endif // GTK_CHECK_VERSION
//  ACE_SYNCH_MUTEX*                     pixelBufferLock;
#endif // GTK_USE
  Stream_CamSave_V4L_Stream*           stream;
  Stream_CamSave_V4L_Subscribers_t     subscribers;
};
#if defined (LIBCAMERA_SUPPORT)
class Stream_CamSave_LibCamera_Stream;
struct Stream_CamSave_LibCamera_UI_CBData
 : Stream_CamSave_UI_CBData
{
  Stream_CamSave_LibCamera_UI_CBData ()
   : Stream_CamSave_UI_CBData ()
   , configuration (NULL)
#if defined (GTK_USE)
//   , pixelBuffer (NULL)
//   , pixelBufferLock (NULL)
#endif // GTK_USE
   , stream (NULL)
   , subscribers ()
  {
#if defined (GTK_USE)
//    pixelBufferLock = &UIState->lock;
#endif // GTK_USE
  }

  struct Stream_CamSave_Configuration*   configuration;
#if defined (GTK_USE)
//#if GTK_CHECK_VERSION(3,0,0)
//  cairo_surface_t*                     pixelBuffer;
//#elif GTK_CHECK_VERSION(2,0,0)
//  GdkPixbuf*                           pixelBuffer;
//#endif // GTK_CHECK_VERSION
//  ACE_SYNCH_MUTEX*                     pixelBufferLock;
#endif // GTK_USE
  Stream_CamSave_LibCamera_Stream*       stream;
  Stream_CamSave_LibCamera_Subscribers_t subscribers;
};
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CamSave_UI_ThreadData
 : Test_U_UI_ThreadData
{
  Stream_CamSave_UI_ThreadData ()
   : Test_U_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Stream_CamSave_UI_CBData* CBData;
};

#if defined (WXWIDGETS_SUPPORT)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Stream_CamSave_WxWidgetsXRCDefinition_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_CamSave_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_CamSave_DirectShow_WxWidgetsIApplication_t,
                                         Stream_CamSave_DirectShow_Stream> Stream_CamSave_DirectShow_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_CamSave_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_CamSave_DirectShow_UI_CBData,
                                         Stream_CamSave_DirectShow_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_CamSave_DirectShow_WxWidgetsApplication_t;
typedef Stream_CamSave_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_CamSave_MediaFoundation_WxWidgetsIApplication_t,
                                         Stream_CamSave_MediaFoundation_Stream> Stream_CamSave_MediaFoundation_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_CamSave_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_CamSave_MediaFoundation_UI_CBData,
                                         Stream_CamSave_MediaFoundation_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_CamSave_MediaFoundation_WxWidgetsApplication_t;
#else
typedef Stream_CamSave_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_CamSave_V4L_WxWidgetsIApplication_t,
                                         Stream_CamSave_V4L_Stream> Stream_CamSave_V4L_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_CamSave_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_CamSave_V4L_UI_CBData,
                                         Stream_CamSave_V4L_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_CamSave_V4L_WxWidgetsApplication_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#endif
