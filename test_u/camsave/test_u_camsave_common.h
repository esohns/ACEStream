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

#ifndef TEST_U_STREAM_CAMSAVE_COMMON_H
#define TEST_U_STREAM_CAMSAVE_COMMON_H

#include <list>
#include <map>
#include <string>

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#include <coguid.h>
#include <combaseapi.h>
#include <control.h>
#include <d3d9.h>
#include <evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <strmif.h>
#else
#include "linux/videodev2.h"

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif
#endif // ACE_WIN32 || ACE_WIN64

#include "gtk/gtk.h"

#include "common_isubscribe.h"
#include "common_tools.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

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
#include "stream_lib_directshow_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "test_u_common.h"
#include "test_u_gtk_common.h"

#include "test_u_camsave_eventhandler.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct ISampleGrabber;
#endif // ACE_WIN32 || ACE_WIN64
class Stream_IAllocator;

struct Stream_CamSave_UserData
 : Stream_UserData
{
  Stream_CamSave_UserData ()
   : Stream_UserData ()
  {}
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_MessageData
{
  Stream_CamSave_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMediaSample* sample;
  double        sampleTime;
};
typedef Stream_DataBase_T<struct Stream_CamSave_DirectShow_MessageData> Stream_CamSave_DirectShow_MessageData_t;

struct Stream_CamSave_MediaFoundation_MessageData
{
  Stream_CamSave_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMFSample* sample;
  LONGLONG   sampleTime;
};
typedef Stream_DataBase_T<struct Stream_CamSave_MediaFoundation_MessageData> Stream_CamSave_MediaFoundation_MessageData_t;
#else
struct Stream_CamSave_MessageData
{
  Stream_CamSave_MessageData ()
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {}

  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
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

struct Stream_CamSave_SessionData
 : Test_U_SessionData
{
  Stream_CamSave_SessionData ()
   : Test_U_SessionData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , inputFormat (NULL)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
#else
   , format (AV_PIX_FMT_RGB24) // output-
   , frameRate ()
   , inputFormat ()
   , sourceFormat ()
#endif // ACE_WIN32 || ACE_WIN64
   , statistic ()
   , userData (NULL)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    inputFormat =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!inputFormat)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));
    else
      ACE_OS::memset (inputFormat, 0, sizeof (struct _AMMediaType));
#else
    ACE_OS::memset (&sourceFormat, 0, sizeof (GdkRectangle));
#endif // ACE_WIN32 || ACE_WIN64
  }

  struct Stream_CamSave_SessionData operator+= (const struct Stream_CamSave_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_U_SessionData::operator+= (rhs_in);

    // *NOTE*: the idea is to 'merge' the data
    statistic += rhs_in.statistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    direct3DManagerResetToken =
      (direct3DManagerResetToken ? direct3DManagerResetToken
                                 : rhs_in.direct3DManagerResetToken);
    //finputFormat = (inputFormat ? inputFormat : rhs_in.inputFormat);
    //rendererNodeId = (rendererNodeId ? rendererNodeId : rhs_in.rendererNodeId);
    resetToken = (resetToken ? resetToken : rhs_in.resetToken);
    //session = (session ? session : rhs_in.session);
#else
    //format =
    //frameRate =
#endif // ACE_WIN32 || ACE_WIN64

    return *this;
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IDirect3DDevice9Ex*                 direct3DDevice;
  UINT                                direct3DManagerResetToken;
  struct _AMMediaType*                inputFormat; // input-
  TOPOID                              rendererNodeId;
  UINT                                resetToken;
  IMFMediaSession*                    session;
#else
  enum AVPixelFormat                  format; // input-
  struct v4l2_fract                   frameRate; // time-per-frame
  struct v4l2_format                  inputFormat;
  GdkRectangle                        sourceFormat; // gtk cairo/pixbuf module
#endif // ACE_WIN32 || ACE_WIN64
  struct Stream_CamSave_StatisticData statistic;

  struct Stream_CamSave_UserData*     userData;
};
typedef Stream_SessionData_T<struct Stream_CamSave_SessionData> Stream_CamSave_SessionData_t;

struct Stream_CamSave_SignalHandlerConfiguration
 : Stream_SignalHandlerConfiguration
{
  Stream_CamSave_SignalHandlerConfiguration ()
   : Stream_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

template <typename DataType>
class Stream_CamSave_Message_T;
template <typename DataMessageType>
class Stream_CamSave_SessionMessage_T;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_CamSave_Message_T<struct Stream_CamSave_DirectShow_MessageData> Stream_CamSave_DirectShow_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_DirectShow_Message_t> Stream_CamSave_DirectShow_SessionMessage_t;
typedef Stream_CamSave_Message_T<struct Stream_CamSave_MediaFoundation_MessageData> Stream_CamSave_MediaFoundation_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_MediaFoundation_Message_t> Stream_CamSave_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Stream_CamSave_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_DirectShow_Message_t,
                                    Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_ISessionNotify_t;
typedef std::list<Stream_CamSave_DirectShow_ISessionNotify_t*> Stream_CamSave_DirectShow_Subscribers_t;
typedef Stream_CamSave_DirectShow_Subscribers_t::iterator Stream_CamSave_DirectShow_SubscribersIterator_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Stream_CamSave_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_MediaFoundation_Message_t,
                                    Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_ISessionNotify_t;
typedef std::list<Stream_CamSave_MediaFoundation_ISessionNotify_t*> Stream_CamSave_MediaFoundation_Subscribers_t;
typedef Stream_CamSave_MediaFoundation_Subscribers_t::iterator Stream_CamSave_MediaFoundation_SubscribersIterator_t;
#else
typedef Stream_CamSave_Message_T<struct Stream_CamSave_MessageData> Stream_CamSave_Message_t;
typedef Stream_CamSave_SessionMessage_T<Stream_CamSave_Message_t> Stream_CamSave_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Stream_CamSave_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_Message_t,
                                    Stream_CamSave_SessionMessage_t> Stream_CamSave_ISessionNotify_t;
typedef std::list<Stream_CamSave_ISessionNotify_t*> Stream_CamSave_Subscribers_t;
typedef Stream_CamSave_Subscribers_t::iterator Stream_CamSave_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64
struct Stream_CamSave_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Stream_CamSave_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , fullScreen (false)
   , targetFileName ()
  {
    hasHeader = true;
  }

  std::string deviceIdentifier;
  bool        fullScreen;
  std::string targetFileName;
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_CamSave_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration> Stream_CamSave_DirectShow_StreamConfiguration_t;
struct Stream_CamSave_DirectShow_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_DirectShow_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
   , area ()
   , builder (NULL)
   , direct3DDevice (NULL)
   , filterCLSID (GUID_NULL)
   , filterConfiguration (NULL)
   , inputFormat (NULL)
   , push (MODULE_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , sourceFormat (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , window (NULL)
   , windowController (NULL)
   , windowController2 (NULL)
  {
    //mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
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
    if (direct3DDevice)
    {
      direct3DDevice->Release (); direct3DDevice = NULL;
    } // end IF
    if (rhs_in.direct3DDevice)
    {
      rhs_in.direct3DDevice->AddRef ();
      direct3DDevice = rhs_in.direct3DDevice;
    } // end IF
    filterCLSID = rhs_in.filterCLSID;
    filterConfiguration = rhs_in.filterConfiguration;
    if (inputFormat)
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (inputFormat);
    if (rhs_in.inputFormat)
      if (!Stream_MediaFramework_DirectShow_Tools::copyMediaType (*rhs_in.inputFormat,
                                                                  inputFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copyMediaType(), returning\n")));
        return *this;
      } // end IF
    push = rhs_in.push;
    if (sourceFormat)
      Stream_MediaFramework_DirectShow_Tools::deleteMediaType (sourceFormat);
    if (rhs_in.sourceFormat)
      if (!Stream_MediaFramework_DirectShow_Tools::copyMediaType (*rhs_in.sourceFormat,
                                                                  sourceFormat))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copyMediaType(), returning\n")));
        return *this;
      } // end IF
    subscriber = rhs_in.subscriber;
    subscribers = rhs_in.subscribers;
    window = rhs_in.window;
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
  IDirect3DDevice9Ex*                                   direct3DDevice;
  struct _GUID                                          filterCLSID;
  struct Stream_CamSave_DirectShow_FilterConfiguration* filterConfiguration;
  struct _AMMediaType*                                  inputFormat;
  bool                                                  push;
  struct _AMMediaType*                                  sourceFormat;
  Stream_CamSave_DirectShow_ISessionNotify_t*           subscriber;
  Stream_CamSave_DirectShow_Subscribers_t*              subscribers;
  HWND                                                  window;
  IVideoWindow*                                         windowController;
  IMFVideoDisplayControl*                               windowController2; // EVR
};

struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_CamSave_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration> Stream_CamSave_MediaFoundation_StreamConfiguration_t;
struct Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_MediaFoundation_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
   , area ()
   , direct3DDevice (NULL)
   , inputFormat (NULL)
   , rendererNodeId (0)
   , sampleGrabberNodeId (0)
   , session (NULL)
   , sourceFormat (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , window (NULL)
   , windowController (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
  }

  struct tagRECT                                   area;
  IDirect3DDevice9Ex*                              direct3DDevice;
  IMFMediaType*                                    inputFormat;
  TOPOID                                           rendererNodeId;
  TOPOID                                           sampleGrabberNodeId;
  IMFMediaSession*                                 session;
  IMFMediaType*                                    sourceFormat;
  Stream_CamSave_MediaFoundation_ISessionNotify_t* subscriber;
  Stream_CamSave_MediaFoundation_Subscribers_t*    subscribers;
  HWND                                             window;
  IMFVideoDisplayControl*                          windowController;
};
#else
struct Stream_CamSave_V4L_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_CamSave_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_CamSave_V4L_ModuleHandlerConfiguration> Stream_CamSave_V4L_StreamConfiguration_t;
struct Stream_CamSave_V4L_ModuleHandlerConfiguration
 : Stream_CamSave_ModuleHandlerConfiguration
{
  Stream_CamSave_V4L_ModuleHandlerConfiguration ()
   : Stream_CamSave_ModuleHandlerConfiguration ()
   , area ()
   , buffers (MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , codecFormat (AV_PIX_FMT_NONE)
   , codecId (AV_CODEC_ID_NONE)
   , fileDescriptor (-1)
   , format (AV_PIX_FMT_NONE)
   , frameRate ()
   , inputFormat ()
   , outputFormat (AV_PIX_FMT_RGB24)
   , pixelBuffer (NULL)
   , pixelBufferLock (NULL)
   , sourceFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
   , v4l2Method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , v4l2Window (NULL)
   , window (NULL)
  {
    // *PORTABILITY*: v4l2: device path (e.g. "[/dev/]video0")
    deviceIdentifier = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_DEFAULT_VIDEO_DEVICE);

    ACE_OS::memset (&frameRate, 0, sizeof (struct v4l2_fract));
    ACE_OS::memset (&inputFormat, 0, sizeof (struct v4l2_format));
    inputFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    ACE_OS::memset (&sourceFormat, 0, sizeof (GdkRectangle));
  }

  GdkRectangle                     area;
  __u32                            buffers; // v4l device buffers
  enum AVPixelFormat               codecFormat; // preferred output-
  enum AVCodecID                   codecId;
  int                              fileDescriptor;
  struct v4l2_fract                frameRate; // time-per-frame (s)
  struct v4l2_format               inputFormat;
  enum AVPixelFormat               outputFormat;
  GdkPixbuf*                       pixelBuffer;
  ACE_SYNCH_MUTEX*                 pixelBufferLock;
  GdkRectangle                     sourceFormat; // gtk cairo/pixbuf module
  Stream_CamSave_ISessionNotify_t* subscriber;
  Stream_CamSave_Subscribers_t*    subscribers;
  enum v4l2_memory                 v4l2Method; // v4l camera source
  struct v4l2_window*              v4l2Window;
  GdkWindow*                       window;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Stream_CamSave_StreamState
 : Stream_State
{
  Stream_CamSave_StreamState ()
   : Stream_State ()
   , userData (NULL)
  {}

  struct Stream_CamSave_UserData* userData;
};

struct Stream_CamSave_StreamConfiguration
 : Stream_Configuration
{
  Stream_CamSave_StreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {
    printFinalReport = true;
  }

  struct Stream_CamSave_UserData* userData;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_CamSave_StreamState> Stream_CamSave_IStreamControl_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_CamSave_DirectShow_Configuration
 : Test_U_Configuration
{
  Stream_CamSave_DirectShow_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_CamSave_DirectShow_StreamConfiguration_t  streamConfiguration;

  struct Stream_CamSave_UserData                   userData;
};

struct Stream_CamSave_MediaFoundation_Configuration
 : Test_U_Configuration
{
  Stream_CamSave_MediaFoundation_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration     signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_CamSave_MediaFoundation_StreamConfiguration_t streamConfiguration;

  struct Stream_CamSave_UserData                       userData;
};
#else
struct Stream_CamSave_Configuration
 : Test_U_Configuration
{
  Stream_CamSave_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {}

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_CamSave_StreamConfiguration_t             streamConfiguration;

  struct Stream_CamSave_UserData                   userData;
};
#endif // ACE_WIN32 || ACE_WIN64

//template <typename AllocatorConfigurationType,
//          typename CommandType,
//          typename ControlMessageType,
//          typename SessionMessageType>
//class Stream_MessageBase_T;
//typedef Stream_MessageBase_T<struct Stream_AllocatorConfiguration,
//                             int,
//                             Test_U_ControlMessage_t,
//                             Test_U_SessionMessage_t> Test_U_Message_t;

//typedef Stream_SessionData_T<struct Stream_SessionData> Test_U_SessionData_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Test_U_ControlMessage_t,
//                                         Stream_CamSave_Message,
//                                         Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_DirectShow_Message_t,
                                          Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_MediaFoundation_Message_t,
                                          Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_Message_t,
                                          Stream_CamSave_SessionMessage_t> Stream_CamSave_MessageAllocator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_ISubscribe_T<Stream_CamSave_DirectShow_ISessionNotify_t> Stream_CamSave_DirectShow_ISubscribe_t;
typedef Common_ISubscribe_T<Stream_CamSave_MediaFoundation_ISessionNotify_t> Stream_CamSave_MediaFoundation_ISubscribe_t;

typedef Stream_CamSave_EventHandler_T<Stream_CamSave_DirectShow_ISessionNotify_t,
                                      Stream_CamSave_DirectShow_Message_t,
                                      Stream_CamSave_DirectShow_SessionMessage_t> Stream_CamSave_DirectShow_EventHandler_t;
typedef Stream_CamSave_EventHandler_T<Stream_CamSave_MediaFoundation_ISessionNotify_t,
                                      Stream_CamSave_MediaFoundation_Message_t,
                                      Stream_CamSave_MediaFoundation_SessionMessage_t> Stream_CamSave_MediaFoundation_EventHandler_t;
#else
typedef Common_ISubscribe_T<Stream_CamSave_ISessionNotify_t> Stream_CamSave_ISubscribe_t;

typedef Stream_CamSave_EventHandler_T<Stream_CamSave_ISessionNotify_t,
                                      Stream_CamSave_Message_t,
                                      Stream_CamSave_SessionMessage_t> Stream_CamSave_EventHandler_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

struct Stream_CamSave_GTK_ProgressData
 : Test_U_GTK_ProgressData
{
  Stream_CamSave_GTK_ProgressData ()
   : Test_U_GTK_ProgressData ()
   , statistic ()
  {}

  struct Stream_CamSave_StatisticData statistic;
};

struct Stream_CamSave_GTK_CBData
 : Test_U_GTK_CBData
{
  Stream_CamSave_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , isFirst (true)
   , progressData ()
  {}

  bool                                   isFirst; // first activation ?
  struct Stream_CamSave_GTK_ProgressData progressData;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_CamSave_DirectShow_Stream;
struct Stream_CamSave_DirectShow_GTK_CBData
 : Stream_CamSave_GTK_CBData
{
  Stream_CamSave_DirectShow_GTK_CBData ()
   : Stream_CamSave_GTK_CBData ()
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
struct Stream_CamSave_MediaFoundation_GTK_CBData
 : Stream_CamSave_GTK_CBData
{
  Stream_CamSave_MediaFoundation_GTK_CBData ()
   : Stream_CamSave_GTK_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
  {}

  struct Stream_CamSave_MediaFoundation_Configuration* configuration;
  Stream_CamSave_MediaFoundation_Stream*               stream;
  Stream_CamSave_MediaFoundation_Subscribers_t         subscribers;
};
#else
class Stream_CamSave_Stream;
struct Stream_CamSave_V4L_GTK_CBData
 : Stream_CamSave_GTK_CBData
{
  Stream_CamSave_V4L_GTK_CBData ()
   : Stream_CamSave_GTK_CBData ()
   , configuration (NULL)
   , fileDescriptor (-1)
   , pixelBuffer (NULL)
   , pixelBufferLock (NULL)
   , stream (NULL)
   , subscribers ()
  {
    pixelBufferLock = &lock;
  }

  struct Stream_CamSave_Configuration* configuration;
  int                                  fileDescriptor; // (capture) device file descriptor
  GdkPixbuf*                           pixelBuffer;
  ACE_SYNCH_MUTEX*                     pixelBufferLock;
  Stream_CamSave_Stream*               stream;
  Stream_CamSave_Subscribers_t         subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_UI_GtkBuilderDefinition_T<struct Stream_CamSave_DirectShow_GTK_CBData> Stream_CamSave_DirectShow_GtkBuilderDefinition_t;
typedef Common_UI_GtkBuilderDefinition_T<struct Stream_CamSave_MediaFoundation_GTK_CBData> Stream_CamSave_MediaFoundation_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct Stream_CamSave_DirectShow_GTK_CBData> Stream_CamSave_DirectShow_GTK_Manager_t;
typedef ACE_Singleton<Stream_CamSave_DirectShow_GTK_Manager_t,
                      typename ACE_MT_SYNCH::MUTEX> CAMSAVE_DIRECTSHOW_GTK_MANAGER_SINGLETON;

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct Stream_CamSave_MediaFoundation_GTK_CBData> Stream_CamSave_MediaFoundation_GTK_Manager_t;
typedef ACE_Singleton<Stream_CamSave_MediaFoundation_GTK_Manager_t,
                      typename ACE_MT_SYNCH::MUTEX> CAMSAVE_MEDIAFOUNDATION_GTK_MANAGER_SINGLETON;
#else
typedef Common_UI_GtkBuilderDefinition_T<struct Stream_CamSave_V4L_GTK_CBData> Stream_CamSave_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<ACE_MT_SYNCH,
                                struct Stream_CamSave_V4L_GTK_CBData> Stream_CamSave_GTK_Manager_t;
typedef ACE_Singleton<Stream_CamSave_GTK_Manager_t,
                      typename ACE_MT_SYNCH::MUTEX> CAMSAVE_GTK_MANAGER_SINGLETON;
#endif // ACE_WIN32 || ACE_WIN64

#endif
