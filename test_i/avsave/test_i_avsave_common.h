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

#ifndef TEST_I_AVSAVE_COMMON_H
#define TEST_I_AVSAVE_COMMON_H

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
#include "evr.h"
#include "mfapi.h"
#include "mfidl.h"
#include "strmif.h"
#else
#include "linux/videodev2.h"

#include "X11/Xlib.h"

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
#if defined (GTK_USE)
#include "gtk/gtk.h"
#elif defined (WXWIDGETS_USE)
#include "wx/apptrait.h"
#include "wx/window.h"
#endif
#endif // GUI_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

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
#include "stream_lib_alsa_defines.h"

#include "stream_lib_v4l_defines.h"
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_i_common.h"
#include "test_i_configuration.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_i_gtk_common.h"
#elif defined (QT_USE)
#include "test_i_qt_common.h"
#elif defined (WXWIDGETS_USE)
#include "test_i_wxwidgets_common.h"

#include "avsave_wxwidgets_ui.h"
#endif
#endif // GUI_SUPPORT

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;
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
class Stream_AVSave_EventHandler_T;
#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
template <typename WidgetBaseClassType,
          typename InterfaceType,
          typename StreamType>
class Stream_AVSave_WxWidgetsDialog_T;
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT

enum Stream_AVSave_ProgramMode
{
  STREAM_AVSAVE_PROGRAMMODE_PRINT_VERSION = 0,
  STREAM_AVSAVE_PROGRAMMODE_TEST_METHODS,
  STREAM_AVSAVE_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  STREAM_AVSAVE_PROGRAMMODE_MAX,
  STREAM_AVSAVE_PROGRAMMODE_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_MessageData
{
  Stream_AVSave_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
   , task (NULL)
   , index (-1)
  {}

  // video
  IMediaSample*                sample;
  double                       sampleTime;
  // audio
  Common_ISet_T<unsigned int>* task;
  unsigned int                 index;
};
typedef Stream_DataBase_T<struct Stream_AVSave_DirectShow_MessageData> Stream_AvSave_DirectShow_MessageData_t;

struct Stream_AVSave_MediaFoundation_MessageData
{
  Stream_AVSave_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
   , task (NULL)
   , index (-1)
  {}

  // video
  IMFSample* sample;
  LONGLONG   sampleTime;
  // audio
  Common_ISet_T<unsigned int>* task;
  unsigned int                 index;
};
typedef Stream_DataBase_T<struct Stream_AVSave_MediaFoundation_MessageData> Stream_AvSave_MediaFoundation_MessageData_t;
#else
struct Stream_AVSave_MessageData
{
  Stream_AVSave_MessageData ()
   : device (-1)
   , index (0)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_AVSave_StatisticData
 : Stream_Statistic
{
  Stream_AVSave_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Stream_AVSave_StatisticData operator+= (const struct Stream_AVSave_StatisticData& rhs_in)
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
typedef Common_StatisticHandler_T<struct Stream_AVSave_StatisticData> Test_I_AVSave_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_StreamState;
typedef Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                      struct _AMMediaType,
                                      struct Stream_AVSave_DirectShow_StreamState,
                                      struct Stream_AVSave_StatisticData,
                                      struct Stream_UserData> Stream_AVSave_DirectShow_SessionData;
typedef Stream_SessionData_T<Stream_AVSave_DirectShow_SessionData> Stream_AVSave_DirectShow_SessionData_t;

struct Stream_AVSave_MediaFoundation_StreamState;
class Stream_AVSave_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        IMFMediaType*,
                                        struct Stream_AVSave_MediaFoundation_StreamState,
                                        struct Stream_AVSave_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_AVSave_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   IMFMediaType*,
                                   struct Stream_AVSave_MediaFoundation_StreamState,
                                   struct Stream_AVSave_StatisticData,
                                   struct Stream_UserData> ()
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
  {}

  Stream_AVSave_MediaFoundation_SessionData& operator= (const Stream_AVSave_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Stream_AVSave_MediaFoundation_StreamState,
                                  struct Stream_AVSave_StatisticData,
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
  Stream_AVSave_MediaFoundation_SessionData& operator+= (const Stream_AVSave_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Stream_AVSave_MediaFoundation_StreamState,
                                  struct Stream_AVSave_StatisticData,
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

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_MediaFoundation_SessionData (const Stream_AVSave_MediaFoundation_SessionData&))
};
typedef Stream_SessionData_T<Stream_AVSave_MediaFoundation_SessionData> Stream_AVSave_MediaFoundation_SessionData_t;
#else
struct Stream_AVSave_V4L_StreamState;
class Stream_AVSave_V4L_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct Stream_MediaFramework_V4L_MediaType,
                                        struct Stream_AVSave_V4L_StreamState,
                                        struct Stream_AVSave_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_AVSave_V4L_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct Stream_MediaFramework_V4L_MediaType,
                                   struct Stream_AVSave_V4L_StreamState,
                                   struct Stream_AVSave_StatisticData,
                                   struct Stream_UserData> ()
  {}

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_V4L_SessionData& operator= (const Stream_AVSave_V4L_SessionData&))
};
typedef Stream_SessionData_T<Stream_AVSave_V4L_SessionData> Stream_AVSave_V4L_SessionData_t;

class Stream_AVSave_ALSA_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct Stream_MediaFramework_ALSA_MediaType,
                                        struct Stream_AVSave_ALSA_StreamState,
                                        struct Stream_AVSave_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Stream_AVSave_ALSA_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType,
                                   struct Stream_AVSave_ALSA_StreamState,
                                   struct Stream_AVSave_StatisticData,
                                   struct Stream_UserData> ()
  {}

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AVSave_ALSA_SessionData& operator= (const Stream_AVSave_ALSA_SessionData&))
};
typedef Stream_SessionData_T<Stream_AVSave_ALSA_SessionData> Stream_AVSave_ALSA_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_AVSave_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Stream_AVSave_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

template <typename DataType>
class Stream_AVSave_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_AVSave_SessionMessage_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_AVSave_Message_T<struct Stream_AVSave_DirectShow_MessageData> Stream_AVSave_DirectShow_Message_t;
typedef Stream_AVSave_SessionMessage_T<Stream_AVSave_DirectShow_Message_t,
                                       Stream_AVSave_DirectShow_SessionData_t> Stream_AVSave_DirectShow_SessionMessage_t;
typedef Stream_AVSave_Message_T<struct Stream_AVSave_MediaFoundation_MessageData> Stream_AVSave_MediaFoundation_Message_t;
typedef Stream_AVSave_SessionMessage_T<Stream_AVSave_MediaFoundation_Message_t,
                                       Stream_AVSave_MediaFoundation_SessionData_t> Stream_AVSave_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_AVSave_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_AVSave_DirectShow_Message_t,
                                    Stream_AVSave_DirectShow_SessionMessage_t> Stream_AVSave_DirectShow_ISessionNotify_t;
typedef std::list<Stream_AVSave_DirectShow_ISessionNotify_t*> Stream_AVSave_DirectShow_Subscribers_t;
typedef Stream_AVSave_DirectShow_Subscribers_t::iterator Stream_AVSave_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Stream_AVSave_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_AVSave_MediaFoundation_Message_t,
                                    Stream_AVSave_MediaFoundation_SessionMessage_t> Stream_AVSave_MediaFoundation_ISessionNotify_t;
typedef std::list<Stream_AVSave_MediaFoundation_ISessionNotify_t*> Stream_AVSave_MediaFoundation_Subscribers_t;
typedef Stream_AVSave_MediaFoundation_Subscribers_t::iterator Stream_AVSave_MediaFoundation_SubscribersIterator_t;
#else
typedef Stream_AVSave_Message_T<struct Stream_AVSave_MessageData> Stream_AVSave_Message_t;
typedef Stream_AVSave_SessionMessage_T<Stream_AVSave_Message_t,
                                       Stream_AVSave_ALSA_SessionData_t> Stream_AVSave_ALSA_SessionMessage_t;
typedef Stream_AVSave_SessionMessage_T<Stream_AVSave_Message_t,
                                       Stream_AVSave_V4L_SessionData_t> Stream_AVSave_V4L_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_AVSave_V4L_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_AVSave_Message_t,
                                    Stream_AVSave_V4L_SessionMessage_t> Stream_AVSave_V4L_ISessionNotify_t;
typedef std::list<Stream_AVSave_V4L_ISessionNotify_t*> Stream_AVSave_V4L_Subscribers_t;
typedef Stream_AVSave_V4L_Subscribers_t::iterator Stream_AVSave_V4L_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Stream_AVSave_ALSA_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_AVSave_Message_t,
                                    Stream_AVSave_ALSA_SessionMessage_t> Stream_AVSave_ALSA_ISessionNotify_t;
typedef std::list<Stream_AVSave_ALSA_ISessionNotify_t*> Stream_AVSave_ALSA_Subscribers_t;
typedef Stream_AVSave_ALSA_Subscribers_t::iterator Stream_AVSave_ALSA_SubscribersIterator_t;

#endif // ACE_WIN32 || ACE_WIN64
struct Stream_AVSave_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Stream_AVSave_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , display ()
   , fullScreen (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , window (NULL)
#else
   , window (None)
   , X11Display (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , targetFileName ()
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    concurrency = STREAM_HEADMODULECONCURRENCY_CONCURRENT;
#else
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
#endif // ACE_WIN32 || ACE_WIN64
  }

  struct Stream_Device_Identifier deviceIdentifier; // source module
  struct Common_UI_DisplayDevice  display; // display module
  bool                            fullScreen;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                            window;
#else
  Window                          window;
  struct _XDisplay*               X11Display;
#endif // ACE_WIN32 || ACE_WIN64
  std::string                     targetFileName;
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_StreamConfiguration;
struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AVSave_DirectShow_StreamConfiguration,
                               struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration> Stream_AVSave_DirectShow_StreamConfiguration_t;
struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration
 : Stream_AVSave_ModuleHandlerConfiguration
{
  Stream_AVSave_DirectShow_ModuleHandlerConfiguration ()
   : Stream_AVSave_ModuleHandlerConfiguration ()
   , area ()
   , audioInput (0)
   , builder (NULL)
   , direct3DConfiguration (NULL)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outboundStreamName ()
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

  struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration operator= (const struct Stream_AVSave_DirectShow_ModuleHandlerConfiguration& rhs_in)
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
  UINT_PTR                                              audioInput;
  IGraphBuilder*                                        builder;
  struct Stream_MediaFramework_Direct3D_Configuration*  direct3DConfiguration;
  struct Stream_AVSave_DirectShow_FilterConfiguration* filterConfiguration;
  CLSID                                                 filterCLSID;
  std::string                                           outboundStreamName; // message handler
  struct _AMMediaType                                   outputFormat;
  bool                                                  push;
  //struct _AMMediaType                                   sourceFormat;
  Stream_AVSave_DirectShow_ISessionNotify_t*           subscriber;
  Stream_AVSave_DirectShow_Subscribers_t*              subscribers;
  IVideoWindow*                                         windowController;
  IMFVideoDisplayControl*                               windowController2; // EVR
};

struct Stream_AVSave_MediaFoundation_StreamConfiguration;
struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AVSave_MediaFoundation_StreamConfiguration,
                               struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration> Stream_AVSave_MediaFoundation_StreamConfiguration_t;
struct Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration
 : Stream_AVSave_ModuleHandlerConfiguration
{
  Stream_AVSave_MediaFoundation_ModuleHandlerConfiguration ()
   : Stream_AVSave_ModuleHandlerConfiguration ()
   , area ()
   , direct3DConfiguration (NULL)
   , outboundStreamName ()
   , outputFormat (NULL)
   , rendererNodeId (0)
   , sampleGrabberNodeId (0)
   , session (NULL)
   //, sourceFormat (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , windowController (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
  }

  struct tagRECT                                       area;
  struct Stream_MediaFramework_Direct3D_Configuration* direct3DConfiguration;
  std::string                                          outboundStreamName; // message handler
  IMFMediaType*                                        outputFormat;
  TOPOID                                               rendererNodeId;
  TOPOID                                               sampleGrabberNodeId;
  IMFMediaSession*                                     session;
  //IMFMediaType*                                        sourceFormat;
  Stream_AVSave_MediaFoundation_ISessionNotify_t*     subscriber;
  Stream_AVSave_MediaFoundation_Subscribers_t*        subscribers;
  IMFVideoDisplayControl*                              windowController;
};
#else
struct Stream_AVSave_V4L_StreamConfiguration;
struct Stream_AVSave_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AVSave_V4L_StreamConfiguration,
                               struct Stream_AVSave_V4L_ModuleHandlerConfiguration> Stream_AVSave_V4L_StreamConfiguration_t;
struct Stream_AVSave_V4L_ModuleHandlerConfiguration
 : Stream_AVSave_ModuleHandlerConfiguration
{
  Stream_AVSave_V4L_ModuleHandlerConfiguration ()
   : Stream_AVSave_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
   , area ()
#endif // GUI_SUPPORT
   , buffers (STREAM_LIB_V4L_DEFAULT_DEVICE_BUFFERS)
   , codecFormat (AV_PIX_FMT_NONE)
   , codecId (AV_CODEC_ID_NONE)
   , method (STREAM_LIB_V4L_DEFAULT_IO_METHOD)
   , outboundStreamName ()
   , outputFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
#if defined (GUI_SUPPORT)
    ACE_OS::memset (&area, 0, sizeof (struct v4l2_rect));
#endif // GUI_SUPPORT

    // *PORTABILITY*: v4l2: device path (e.g. "[/dev/]video0")
    deviceIdentifier.identifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_DEFAULT_VIDEO_DEVICE);

    ACE_OS::memset (&outputFormat, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
  }

#if defined (GUI_SUPPORT)
  struct v4l2_rect                           area;
#endif // GUI_SUPPORT
  __u32                                      buffers; // v4l device buffers
  enum AVPixelFormat                         codecFormat; // preferred output-
  enum AVCodecID                             codecId;
  enum v4l2_memory                           method; // v4l camera source
  std::string                                outboundStreamName; // message handler
  struct Stream_MediaFramework_V4L_MediaType outputFormat;
  Stream_AVSave_V4L_ISessionNotify_t*        subscriber;
  Stream_AVSave_V4L_Subscribers_t*           subscribers;
};

struct Stream_AVSave_ALSA_StreamConfiguration;
struct Stream_AVSave_ALSA_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AVSave_ALSA_StreamConfiguration,
                               struct Stream_AVSave_ALSA_ModuleHandlerConfiguration> Stream_AVSave_ALSA_StreamConfiguration_t;
struct Stream_AVSave_ALSA_ModuleHandlerConfiguration
 : Stream_AVSave_ModuleHandlerConfiguration
{
  Stream_AVSave_ALSA_ModuleHandlerConfiguration ()
   : Stream_AVSave_ModuleHandlerConfiguration ()
   , captureDeviceHandle (NULL)
   , outputFormat ()
   , sinus (false)
   , sinusFrequency (440.0)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    deviceIdentifier.identifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_MIC_ALSA_DEFAULT_DEVICE_NAME);

    ACE_OS::memset (&outputFormat, 0, sizeof (struct Stream_MediaFramework_ALSA_MediaType));
  }

  struct _snd_pcm*                            captureDeviceHandle;
  struct Stream_MediaFramework_ALSA_MediaType outputFormat;
  bool                                        sinus;
  double                                      sinusFrequency;
  Stream_AVSave_V4L_ISessionNotify_t*         subscriber;
  Stream_AVSave_V4L_Subscribers_t*            subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_StreamState
 : Stream_State
{
  Stream_AVSave_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  Stream_AVSave_DirectShow_SessionData* sessionData;

  struct Stream_UserData*        userData;
};

struct Stream_AVSave_MediaFoundation_StreamState
 : Stream_State
{
  Stream_AVSave_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
   , userData (NULL)
  {}

  Stream_AVSave_MediaFoundation_SessionData* sessionData;

  struct Stream_UserData*             userData;
};
#else
struct Stream_AVSave_V4L_StreamState
 : Stream_State
{
  Stream_AVSave_V4L_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Stream_AVSave_V4L_SessionData* sessionData;
};

struct Stream_AVSave_ALSA_StreamState
 : Stream_State
{
  Stream_AVSave_ALSA_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Stream_AVSave_ALSA_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_AVSave_StreamConfiguration
 : Stream_Configuration
{
  Stream_AVSave_StreamConfiguration ()
   : Stream_Configuration ()
   , module_2 (NULL)
  {
    printFinalReport = true;
  }

  Stream_Module_t* module_2;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_StreamConfiguration
 : Stream_AVSave_StreamConfiguration
{
  Stream_AVSave_DirectShow_StreamConfiguration ()
   : Stream_AVSave_StreamConfiguration ()
   , format ()
  {}

  struct _AMMediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_AVSave_DirectShow_StreamState> Stream_AVSave_DirectShow_IStreamControl_t;

struct Stream_AVSave_MediaFoundation_StreamConfiguration
 : Stream_AVSave_StreamConfiguration
{
  Stream_AVSave_MediaFoundation_StreamConfiguration ()
   : Stream_AVSave_StreamConfiguration ()
   , format (NULL)
  {}

  IMFMediaType* format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_AVSave_MediaFoundation_StreamState> Stream_AVSave_MediaFoundation_IStreamControl_t;
#else
struct Stream_AVSave_V4L_StreamConfiguration
 : Stream_AVSave_StreamConfiguration
{
  Stream_AVSave_V4L_StreamConfiguration ()
   : Stream_AVSave_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_V4L_MediaType format;
};

struct Stream_AVSave_ALSA_StreamConfiguration
 : Stream_AVSave_StreamConfiguration
{
  Stream_AVSave_ALSA_StreamConfiguration ()
   : Stream_AVSave_StreamConfiguration ()
   , format ()
  {}

  struct Stream_MediaFramework_ALSA_MediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_AVSave_V4L_StreamState> Stream_AVSave_V4L_IStreamControl_t;
typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_AVSave_ALSA_StreamState> Stream_AVSave_ALSA_IStreamControl_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#endif // GUI_SUPPORT
{
  Stream_AVSave_DirectShow_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , audioStreamConfiguration ()
   , videoStreamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_AVSave_SignalHandlerConfiguration    signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_AVSave_DirectShow_StreamConfiguration_t      audioStreamConfiguration;
  //struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
  Stream_AVSave_DirectShow_StreamConfiguration_t      videoStreamConfiguration;

  struct Stream_UserData                              userData;
};

struct Stream_AVSave_MediaFoundation_Configuration
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_I_GTK_Configuration
#else
 : Test_I_Configuration
#endif // GTK_USE
#endif // GUI_SUPPORT
{
  Stream_AVSave_MediaFoundation_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , audioStreamConfiguration ()
   , videoStreamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_AVSave_SignalHandlerConfiguration     signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_AVSave_MediaFoundation_StreamConfiguration_t audioStreamConfiguration;
  //struct Stream_MediaFramework_Direct3D_Configuration  direct3DConfiguration;
  Stream_AVSave_MediaFoundation_StreamConfiguration_t videoStreamConfiguration;

  struct Stream_UserData                       userData;
};
#else
struct Stream_AVSave_Configuration
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
  Stream_AVSave_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_I_GTK_Configuration ()
#else
   : Test_I_Configuration ()
#endif // GTK_USE
#else
   : Test_I_Configuration ()
#endif // GUI_SUPPORT
   , signalHandlerConfiguration ()
   , audioStreamConfiguration ()
   , videoStreamConfiguration ()
  {}

  // **************************** signal data **********************************
  struct Stream_AVSave_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_AVSave_ALSA_StreamConfiguration_t        audioStreamConfiguration;
  Stream_AVSave_V4L_StreamConfiguration_t         videoStreamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Stream_ControlMessage_t,
//                                         Stream_AVSave_Message,
//                                         Stream_AVSave_SessionMessage> Stream_AVSave_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_AVSave_DirectShow_Message_t,
                                          Stream_AVSave_DirectShow_SessionMessage_t> Stream_AVSave_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_AVSave_MediaFoundation_Message_t,
                                          Stream_AVSave_MediaFoundation_SessionMessage_t> Stream_AVSave_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_AVSave_Message_t,
                                          Stream_AVSave_ALSA_SessionMessage_t> Stream_AVSave_ALSA_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_AVSave_Message_t,
                                          Stream_AVSave_V4L_SessionMessage_t> Stream_AVSave_V4L_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_AVSave_DirectShow_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_AVSave_DirectShow_UI_CBData> Stream_AVSave_DirectShow_WxWidgetsIApplication_t;
struct Stream_AVSave_MediaFoundation_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_AVSave_MediaFoundation_UI_CBData> Stream_AVSave_MediaFoundation_WxWidgetsIApplication_t;
#else
struct Stream_AVSave_V4L_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_AVSave_V4L_UI_CBData> Stream_AVSave_V4L_WxWidgetsIApplication_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif // WXWIDGETS_USE
#endif // GUI_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Stream_AVSave_DirectShow_ISessionNotify_t> Stream_AVSave_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_AVSave_MediaFoundation_ISessionNotify_t> Stream_AVSave_MediaFoundation_ISubscribe_t;

typedef Stream_AVSave_EventHandler_T<Stream_AVSave_DirectShow_ISessionNotify_t,
                                      Stream_AVSave_DirectShow_Message_t,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                      Common_UI_GTK_State_t,
#elif defined (WXWIDGETS_USE)
                                      struct Common_UI_wxWidgets_State,
                                      Common_UI_wxWidgets_IApplicationBase_t,
#endif
#endif // GUI_SUPPORT
                                      Stream_AVSave_DirectShow_SessionMessage_t> Stream_AVSave_DirectShow_EventHandler_t;
typedef Stream_AVSave_EventHandler_T<Stream_AVSave_MediaFoundation_ISessionNotify_t,
                                      Stream_AVSave_MediaFoundation_Message_t,
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
                                      Common_UI_GTK_State_t,
#elif defined (WXWIDGETS_USE)
                                      struct Common_UI_wxWidgets_State,
                                      Common_UI_wxWidgets_IApplicationBase_t,
#endif
#endif // GUI_SUPPORT
                                      Stream_AVSave_MediaFoundation_SessionMessage_t> Stream_AVSave_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Stream_AVSave_ALSA_ISessionNotify_t> Stream_AVSave_ALSA_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_AVSave_V4L_ISessionNotify_t> Stream_AVSave_V4L_ISubscribe_t;

typedef Stream_AVSave_EventHandler_T<Stream_AVSave_V4L_ISessionNotify_t,
                                     Stream_AVSave_Message_t,
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
                                      Stream_AVSave_V4L_SessionMessage_t> Stream_AVSave_V4L_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Stream_AVSave_ProgressData
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
  Stream_AVSave_ProgressData ()
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

  struct Stream_AVSave_StatisticData statistic;
};

struct Stream_AVSave_UI_CBData
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
  Stream_AVSave_UI_CBData ()
#if defined (GTK_USE)
   : Test_I_GTK_CBData ()
#elif defined (QT_USE)
   : Test_I_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_I_wxWidgets_CBData ()
#else
   : Test_I_UI_CBData ()
#endif
   , isFirst (true)
   , progressData ()
  {
    progressData.state = UIState;
  }

  bool                               isFirst; // first activation ?
  struct Stream_AVSave_ProgressData progressData;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_AVSave_WaveIn_Stream;
class Stream_AVSave_DirectShow_Stream;
struct Stream_AVSave_DirectShow_UI_CBData
 : Stream_AVSave_UI_CBData
{
  Stream_AVSave_DirectShow_UI_CBData ()
   : Stream_AVSave_UI_CBData ()
   , configuration (NULL)
   , audioStream (NULL)
   , videoStream (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {}

  struct Stream_AVSave_DirectShow_Configuration* configuration;
  Stream_AVSave_WaveIn_Stream*                   audioStream;
  Stream_AVSave_DirectShow_Stream*               videoStream;
  IAMStreamConfig*                               streamConfiguration;
  Stream_AVSave_DirectShow_Subscribers_t         subscribers;
};

class Stream_AVSave_MediaFoundation_Stream;
struct Stream_AVSave_MediaFoundation_UI_CBData
 : Stream_AVSave_UI_CBData
{
  Stream_AVSave_MediaFoundation_UI_CBData ()
   : Stream_AVSave_UI_CBData ()
   , configuration (NULL)
   , audioStream (NULL)
   , videoStream (NULL)
   , subscribers ()
  {}

  struct Stream_AVSave_MediaFoundation_Configuration* configuration;
  Stream_AVSave_MediaFoundation_Stream*               audioStream;
  Stream_AVSave_MediaFoundation_Stream*               videoStream;
  Stream_AVSave_MediaFoundation_Subscribers_t         subscribers;
};
#else
class Stream_AVSave_ALSA_Stream;
class Stream_AVSave_V4L_Stream;
struct Stream_AVSave_V4L_UI_CBData
 : Stream_AVSave_UI_CBData
{
  Stream_AVSave_V4L_UI_CBData ()
   : Stream_AVSave_UI_CBData ()
   , configuration (NULL)
#if defined (GTK_USE)
//   , pixelBuffer (NULL)
//   , pixelBufferLock (NULL)
#endif // GTK_USE
   , audioStream (NULL)
   , videoStream (NULL)
   , subscribers ()
  {
#if defined (GTK_USE)
//    pixelBufferLock = &UIState->lock;
#endif // GTK_USE
  }

  struct Stream_AVSave_Configuration* configuration;
#if defined (GTK_USE)
//#if GTK_CHECK_VERSION(3,0,0)
//  cairo_surface_t*                     pixelBuffer;
//#elif GTK_CHECK_VERSION(2,0,0)
//  GdkPixbuf*                           pixelBuffer;
//#endif // GTK_CHECK_VERSION
//  ACE_SYNCH_MUTEX*                     pixelBufferLock;
#endif // GTK_USE
  Stream_AVSave_ALSA_Stream*          audioStream;
  Stream_AVSave_V4L_Stream*           videoStream;
//  Stream_AVSave_ALSA_Subscribers_t    subscribers;
  Stream_AVSave_V4L_Subscribers_t     subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_AVSave_UI_ThreadData
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
  Stream_AVSave_UI_ThreadData ()
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

  struct Stream_AVSave_UI_CBData* CBData;
};

#if defined (GTK_USE)
#elif defined (WXWIDGETS_USE)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Stream_AVSave_WxWidgetsXRCDefinition_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_AVSave_WxWidgetsDialog_T<Stream_AVSave_DirectShow_WxWidgetsIApplication_t,
                                         Stream_AVSave_DirectShow_Stream> Stream_AVSave_DirectShow_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_AVSave_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_AVSave_DirectShow_UI_CBData,
                                         Stream_AVSave_DirectShow_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_AVSave_DirectShow_WxWidgetsApplication_t;
typedef Stream_AVSave_WxWidgetsDialog_T<Stream_AVSave_MediaFoundation_WxWidgetsIApplication_t,
                                         Stream_AVSave_MediaFoundation_Stream> Stream_AVSave_MediaFoundation_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_AVSave_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_AVSave_MediaFoundation_UI_CBData,
                                         Stream_AVSave_MediaFoundation_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_AVSave_MediaFoundation_WxWidgetsApplication_t;
#else
typedef Stream_AVSave_WxWidgetsDialog_T<wxDialog_main,
                                         Stream_AVSave_V4L_WxWidgetsIApplication_t,
                                         Stream_AVSave_V4L_Stream> Stream_AVSave_V4L_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_AVSave_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_AVSave_V4L_UI_CBData,
                                         Stream_AVSave_V4L_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_AVSave_V4L_WxWidgetsApplication_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif
#endif // GUI_SUPPORT

#endif
