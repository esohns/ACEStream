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

#ifndef TEST_I_CAMERA_ML_COMMON_H
#define TEST_I_CAMERA_ML_COMMON_H

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

#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_tools.h"

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
class Stream_CameraML_EventHandler_T;

enum Stream_CameraML_ProgramMode
{
  STREAM_CAMERA_ML_PROGRAMMODE_PRINT_VERSION = 0,
  STREAM_CAMERA_ML_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  STREAM_CAMERA_ML_PROGRAMMODE_MAX,
  STREAM_CAMERA_ML_PROGRAMMODE_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_MessageData
{
  Stream_CameraML_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Stream_CameraML_DirectShow_MessageData> Stream_CameraML_DirectShow_MessageData_t;

struct Stream_CameraML_MediaFoundation_MessageData
{
  Stream_CameraML_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Stream_CameraML_MediaFoundation_MessageData> Stream_CameraML_MediaFoundation_MessageData_t;
#else
struct Stream_CameraML_MessageData
{
  Stream_CameraML_MessageData ()
   : fileDescriptor (-1)
   , index (0)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int              fileDescriptor; // (capture) device file descriptor
  __u32            index;  // 'index' field of v4l2_buffer
  enum v4l2_memory method;
  bool             release;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CameraML_StatisticData
 : Stream_Statistic
{
  Stream_CameraML_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Stream_CameraML_StatisticData operator+= (const struct Stream_CameraML_StatisticData& rhs_in)
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
typedef Common_StatisticHandler_T<struct Stream_CameraML_StatisticData> Stream_CameraML_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_StreamState;
class Stream_CameraML_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct _AMMediaType,
                                        struct Stream_CameraML_DirectShow_StreamState,
                                        struct Stream_CameraML_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_CameraML_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct _AMMediaType,
                                   struct Stream_CameraML_DirectShow_StreamState,
                                   struct Stream_CameraML_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , resetToken (0)
  {}

  Stream_CameraML_DirectShow_SessionData& operator= (const Stream_CameraML_DirectShow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Stream_CameraML_DirectShow_StreamState,
                                  struct Stream_CameraML_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);

    return *this;
  }
  Stream_CameraML_DirectShow_SessionData& operator+= (const Stream_CameraML_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Stream_CameraML_DirectShow_StreamState,
                                  struct Stream_CameraML_StatisticData,
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
typedef Stream_SessionData_T<Stream_CameraML_DirectShow_SessionData> Stream_CameraML_DirectShow_SessionData_t;

struct Stream_CameraML_MediaFoundation_StreamState;
class Stream_CameraML_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        IMFMediaType*,
                                        struct Stream_CameraML_MediaFoundation_StreamState,
                                        struct Stream_CameraML_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_CameraML_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   IMFMediaType*,
                                   struct Stream_CameraML_MediaFoundation_StreamState,
                                   struct Stream_CameraML_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
  {}

  Stream_CameraML_MediaFoundation_SessionData& operator= (const Stream_CameraML_MediaFoundation_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Stream_CameraML_MediaFoundation_StreamState,
                                  struct Stream_CameraML_StatisticData,
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
  Stream_CameraML_MediaFoundation_SessionData& operator+= (const Stream_CameraML_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Stream_CameraML_MediaFoundation_StreamState,
                                  struct Stream_CameraML_StatisticData,
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
typedef Stream_SessionData_T<Stream_CameraML_MediaFoundation_SessionData> Stream_CameraML_MediaFoundation_SessionData_t;
#else
struct Stream_CameraML_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                      struct Stream_MediaFramework_V4L_MediaType,
                                      struct Stream_CameraML_StreamState,
                                      struct Stream_CameraML_StatisticData,
                                      struct Stream_UserData> Stream_CameraML_V4L_SessionData;
typedef Stream_SessionData_T<Stream_CameraML_V4L_SessionData> Stream_CameraML_V4L_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

template <typename DataType,
          typename SessionDataType>
class Stream_CameraML_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_CameraML_SessionMessage_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_CameraML_Message_T<struct Stream_CameraML_DirectShow_MessageData,
                                      Stream_CameraML_DirectShow_SessionData_t> Stream_CameraML_DirectShow_Message_t;
typedef Stream_CameraML_SessionMessage_T<Stream_CameraML_DirectShow_Message_t,
                                             Stream_CameraML_DirectShow_SessionData_t> Stream_CameraML_DirectShow_SessionMessage_t;
typedef Stream_CameraML_Message_T<struct Stream_CameraML_MediaFoundation_MessageData,
                                      Stream_CameraML_MediaFoundation_SessionData_t> Stream_CameraML_MediaFoundation_Message_t;
typedef Stream_CameraML_SessionMessage_T<Stream_CameraML_MediaFoundation_Message_t,
                                             Stream_CameraML_MediaFoundation_SessionData_t> Stream_CameraML_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_CameraML_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CameraML_DirectShow_Message_t,
                                    Stream_CameraML_DirectShow_SessionMessage_t> Stream_CameraML_DirectShow_ISessionNotify_t;
typedef std::list<Stream_CameraML_DirectShow_ISessionNotify_t*> Stream_CameraML_DirectShow_Subscribers_t;
typedef Stream_CameraML_DirectShow_Subscribers_t::iterator Stream_CameraML_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Stream_CameraML_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CameraML_MediaFoundation_Message_t,
                                    Stream_CameraML_MediaFoundation_SessionMessage_t> Stream_CameraML_MediaFoundation_ISessionNotify_t;
typedef std::list<Stream_CameraML_MediaFoundation_ISessionNotify_t*> Stream_CameraML_MediaFoundation_Subscribers_t;
typedef Stream_CameraML_MediaFoundation_Subscribers_t::iterator Stream_CameraML_MediaFoundation_SubscribersIterator_t;
#else
typedef Stream_CameraML_Message_T<struct Stream_CameraML_MessageData,
                                  Stream_CameraML_V4L_SessionData_t> Stream_CameraML_Message_t;
typedef Stream_CameraML_SessionMessage_T<Stream_CameraML_Message_t,
                                         Stream_CameraML_V4L_SessionData_t> Stream_CameraML_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_CameraML_V4L_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CameraML_Message_t,
                                    Stream_CameraML_SessionMessage_t> Stream_CameraML_ISessionNotify_t;
typedef std::list<Stream_CameraML_ISessionNotify_t*> Stream_CameraML_Subscribers_t;
typedef Stream_CameraML_Subscribers_t::iterator Stream_CameraML_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64
struct Stream_CameraML_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Stream_CameraML_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , display ()
   , fullScreen (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , shaderFile ()
   , window (NULL)
#else
   , window (0)
#endif // ACE_WIN32 || ACE_WIN64
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
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
  std::string                     shaderFile;
  HWND                            window;
#else
  Window                          window;
#endif // ACE_WIN32 || ACE_WIN64
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_StreamConfiguration;
struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CameraML_DirectShow_StreamConfiguration,
                               struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration> Stream_CameraML_DirectShow_StreamConfiguration_t;
struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration
 : Stream_CameraML_ModuleHandlerConfiguration
{
  Stream_CameraML_DirectShow_ModuleHandlerConfiguration ()
   : Stream_CameraML_ModuleHandlerConfiguration ()
   , area ()
   , builder (NULL)
   , direct3DConfiguration (NULL)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   //, sourceFormat ()
   , subscriber (NULL)
   //, subscribers (NULL)
   , windowController (NULL)
   , windowController2 (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration operator= (const struct Stream_CameraML_DirectShow_ModuleHandlerConfiguration& rhs_in)
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
    //subscribers = rhs_in.subscribers;
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

  struct tagRECT                                         area;
  IGraphBuilder*                                         builder;
  struct Stream_MediaFramework_Direct3D_Configuration*   direct3DConfiguration;
  struct Stream_CameraML_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                  filterCLSID;
  struct _AMMediaType                                    outputFormat;
  bool                                                   push;
  //struct _AMMediaType                                   sourceFormat;
  Stream_CameraML_DirectShow_ISessionNotify_t*           subscriber;
  //Stream_CameraML_DirectShow_Subscribers_t*              subscribers;
  IVideoWindow*                                          windowController;
  IMFVideoDisplayControl*                                windowController2; // EVR
};

struct Stream_CameraML_MediaFoundation_StreamConfiguration;
struct Stream_CameraML_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CameraML_MediaFoundation_StreamConfiguration,
                               struct Stream_CameraML_MediaFoundation_ModuleHandlerConfiguration> Stream_CameraML_MediaFoundation_StreamConfiguration_t;
struct Stream_CameraML_MediaFoundation_ModuleHandlerConfiguration
 : Stream_CameraML_ModuleHandlerConfiguration
{
  Stream_CameraML_MediaFoundation_ModuleHandlerConfiguration ()
   : Stream_CameraML_ModuleHandlerConfiguration ()
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
  Stream_CameraML_MediaFoundation_ISessionNotify_t*       subscriber;
  Stream_CameraML_MediaFoundation_Subscribers_t*          subscribers;
  IMFVideoDisplayControl*                                     windowController;
};
#else
struct Stream_CameraML_V4L_StreamConfiguration;
struct Stream_CameraML_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_CameraML_V4L_StreamConfiguration,
                               struct Stream_CameraML_V4L_ModuleHandlerConfiguration> Stream_CameraML_StreamConfiguration_t;
struct Stream_CameraML_V4L_ModuleHandlerConfiguration
 : Stream_CameraML_ModuleHandlerConfiguration
{
  Stream_CameraML_V4L_ModuleHandlerConfiguration ()
   : Stream_CameraML_ModuleHandlerConfiguration ()
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
#if defined (FFMPEG_SUPPORT)
   , codecFormat (AV_PIX_FMT_NONE)
   , codecId (AV_CODEC_ID_NONE)
#endif // FFMPEG_SUPPORT
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , labelFile ()
   , modelFile ()
   , outputFormat ()
   , subscriber (NULL)
//   , subscribers (NULL)
   , surface (NULL)
   , waylandDisplay (NULL)
  {
    // *PORTABILITY*: v4l2: device path (e.g. "[/dev/]video0")
    deviceIdentifier.identifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);

    ACE_OS::memset (&outputFormat, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
  }

  __u32                                      buffers; // v4l device buffers
#if defined (FFMPEG_SUPPORT)
  enum AVPixelFormat                         codecFormat; // preferred output-
  enum AVCodecID                             codecId;
#endif // FFMPEG_SUPPORT
  enum v4l2_memory                           method; // v4l camera source
  std::string                                labelFile; // tensorflow
  std::string                                modelFile; // tensorflow
  struct Stream_MediaFramework_V4L_MediaType outputFormat;
  Stream_CameraML_ISessionNotify_t*          subscriber;
//  Stream_CameraML_Subscribers_t*             subscribers;
  struct wl_shell_surface*                   surface;
  struct wl_display*                         waylandDisplay;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_StreamState
 : Stream_State
{
  Stream_CameraML_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  Stream_CameraML_DirectShow_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

struct Stream_CameraML_MediaFoundation_StreamState
 : Stream_State
{
  Stream_CameraML_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  Stream_CameraML_MediaFoundation_SessionData* sessionData;

  struct Stream_UserData*             userData;
};
#else
struct Stream_CameraML_StreamState
 : Stream_State
{
  Stream_CameraML_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Stream_CameraML_V4L_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CameraML_StreamConfiguration
 : Stream_Configuration
{
  Stream_CameraML_StreamConfiguration ()
   : Stream_Configuration ()
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_INVALID)
  {
    printFinalReport = true;
  }
  enum Stream_Visualization_VideoRenderer renderer;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_StreamConfiguration
 : Stream_CameraML_StreamConfiguration
{
  Stream_CameraML_DirectShow_StreamConfiguration ()
   : Stream_CameraML_StreamConfiguration ()
   , format ()
  {}

  struct _AMMediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CameraML_DirectShow_StreamState> Stream_CameraML_DirectShow_IStreamControl_t;

struct Stream_CameraML_MediaFoundation_StreamConfiguration
 : Stream_CameraML_StreamConfiguration
{
  Stream_CameraML_MediaFoundation_StreamConfiguration ()
   : Stream_CameraML_StreamConfiguration ()
   , format (NULL)
  {}

  IMFMediaType* format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CameraML_MediaFoundation_StreamState> Stream_CameraML_MediaFoundation_IStreamControl_t;
#else
struct Stream_CameraML_V4L_StreamConfiguration
 : Stream_CameraML_StreamConfiguration
{
  Stream_CameraML_V4L_StreamConfiguration ()
   : Stream_CameraML_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_V4L_MediaType format; // session data-
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CameraML_StreamState> Stream_CameraML_IStreamControl_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CameraML_DirectShow_Configuration
 : Test_I_Configuration
{
  Stream_CameraML_DirectShow_Configuration ()
   : Test_I_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration  direct3DConfiguration;
  Stream_CameraML_DirectShow_StreamConfiguration_t streamConfiguration;
};

struct Stream_CameraML_MediaFoundation_Configuration
 : Test_I_Configuration
{
  Stream_CameraML_MediaFoundation_Configuration ()
   : Test_I_Configuration ()
   , direct3DConfiguration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  struct Stream_MediaFramework_Direct3D_Configuration       direct3DConfiguration;
  Stream_CameraML_MediaFoundation_StreamConfiguration_t streamConfiguration;
};
#else
struct Stream_CameraML_Configuration
 : Test_I_Configuration
{
  Stream_CameraML_Configuration ()
   : Test_I_Configuration ()
   , streamConfiguration ()
  {}

  // **************************** stream data **********************************
  Stream_CameraML_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Stream_CameraML_Message,
//                                         Stream_CameraML_SessionMessage> Stream_CameraML_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraML_DirectShow_Message_t,
                                          Stream_CameraML_DirectShow_SessionMessage_t> Stream_CameraML_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraML_MediaFoundation_Message_t,
                                          Stream_CameraML_MediaFoundation_SessionMessage_t> Stream_CameraML_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_CameraML_Message_t,
                                          Stream_CameraML_SessionMessage_t> Stream_CameraML_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Stream_CameraML_DirectShow_ISessionNotify_t> Stream_CameraML_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_CameraML_MediaFoundation_ISessionNotify_t> Stream_CameraML_MediaFoundation_ISubscribe_t;

typedef Stream_CameraML_EventHandler_T<Stream_CameraML_DirectShow_ISessionNotify_t,
                                           Stream_CameraML_DirectShow_Message_t,
                                           Stream_CameraML_DirectShow_SessionMessage_t> Stream_CameraML_DirectShow_EventHandler_t;
typedef Stream_CameraML_EventHandler_T<Stream_CameraML_MediaFoundation_ISessionNotify_t,
                                           Stream_CameraML_MediaFoundation_Message_t,
                                           Stream_CameraML_MediaFoundation_SessionMessage_t> Stream_CameraML_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Stream_CameraML_ISessionNotify_t> Stream_CameraML_ISubscribe_t;

typedef Stream_CameraML_EventHandler_T<Stream_CameraML_ISessionNotify_t,
                                           Stream_CameraML_Message_t,
                                           Stream_CameraML_SessionMessage_t> Stream_CameraML_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
