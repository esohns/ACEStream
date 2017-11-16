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
#include <d3d9.h>
#include <evr.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <strmif.h>
#else
#include "linux/videodev2.h"

#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixfmt.h"
}
#endif

#include "gtk/gtk.h"
#endif

#include "common_isubscribe.h"
#include "common_tools.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_manager.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_messageallocatorheap_base.h"
#else
#include "stream_messageallocatorheap_base.h"
#endif
#include "stream_session_data.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "test_u_common.h"
#include "test_u_gtk_common.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IAMStreamConfig;
struct IGraphBuilder;
struct IMediaSample;
struct ISampleGrabber;
struct IVideoWindow;
#endif
class Stream_IAllocator;
class Stream_CamSave_Message;
class Stream_CamSave_SessionMessage;
class Stream_CamSave_Stream;

struct Stream_CamSave_UserData
 : Stream_UserData
{
  Stream_CamSave_UserData ()
   : Stream_UserData ()
  {};
};

struct Stream_CamSave_MessageData
{
  Stream_CamSave_MessageData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   : sample (NULL)
   , sampleTime (0)
#else
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
#endif
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IMFSample* sample;
  LONGLONG   sampleTime;
#else
  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
#endif
};

struct Stream_CamSave_StatisticData
 : Stream_Statistic
{
  Stream_CamSave_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif
  {};

  struct Stream_CamSave_StatisticData operator+= (const Stream_CamSave_StatisticData& rhs_in)
  {
    Stream_Statistic::operator+= (rhs_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    capturedFrames += rhs_in.capturedFrames;
#endif

    return *this;
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  unsigned int capturedFrames;
#endif
};
typedef Stream_StatisticHandler_T<struct Stream_CamSave_StatisticData> Test_U_CamSave_StatisticHandler_t;

struct Stream_CamSave_SessionData
 : Test_U_SessionData
{
  Stream_CamSave_SessionData ()
   : Test_U_SessionData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DDevice (NULL)
   , direct3DManagerResetToken (0)
   , format (NULL)
   , rendererNodeId (0)
   , session (NULL)
#else
   , format (AV_PIX_FMT_RGB24) // output-
   , height (0)
   , v4l2Format ()
   , v4l2FrameRate ()
   , width (0)
#endif
   , statistic ()
   , userData (NULL)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    format =
      static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    if (!format)
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, continuing\n")));
    else
      ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
#endif
  };

  struct Stream_CamSave_SessionData operator+= (const Stream_CamSave_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_U_SessionData::operator+= (rhs_in);

    // *NOTE*: the idea is to 'merge' the data
    statistic += rhs_in.statistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    direct3DDevice = (direct3DDevice ? direct3DDevice : rhs_in.direct3DDevice);
    //format = (format ? format : rhs_in.format);
    //rendererNodeId = (rendererNodeId ? rendererNodeId : rhs_in.rendererNodeId);
    //resetToken = (resetToken ? resetToken : rhs_in.resetToken);
    //session = (session ? session : rhs_in.session);
#else
    //format =
    //frameRate =
#endif

    return *this;
  };

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IDirect3DDevice9Ex*                 direct3DDevice;
  UINT                                direct3DManagerResetToken;
  struct _AMMediaType*                format;
  TOPOID                              rendererNodeId;
  IMFMediaSession*                    session;
#else
  enum AVPixelFormat                  format;
  unsigned int                        height;
  struct v4l2_format                  v4l2Format;
  struct v4l2_fract                   v4l2FrameRate; // time-per-frame
  unsigned int                        width;
#endif
  struct Stream_CamSave_StatisticData statistic;

  struct Stream_CamSave_UserData*     userData;
};
typedef Stream_SessionData_T<struct Stream_CamSave_SessionData> Stream_CamSave_SessionData_t;

struct Stream_CamSave_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Stream_CamSave_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Stream_CamSave_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_CamSave_Message,
                                    Stream_CamSave_SessionMessage> Stream_CamSave_ISessionNotify_t;
typedef std::list<Stream_CamSave_ISessionNotify_t*> Stream_CamSave_Subscribers_t;
typedef Stream_CamSave_Subscribers_t::iterator Stream_CamSave_SubscribersIterator_t;
//extern const char stream_name_string_[];
struct Stream_CamSave_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_CamSave_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Stream_CamSave_ModuleHandlerConfiguration> Stream_CamSave_StreamConfiguration_t;
struct Stream_CamSave_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Stream_CamSave_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , area ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   //, builder (NULL)
   , format (NULL)
   , rendererNodeId (0)
   , sampleGrabberNodeId (0)
   , session (NULL)
   , windowController (NULL)
#else
   , buffers (MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , fileDescriptor (-1)
   , format (AV_PIX_FMT_RGB24)
#endif
   , device ()
   , lock (NULL)
   , pixelBuffer (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , targetFileName ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , v4l2Format ()
   , v4l2FrameRate ()
   , v4l2Method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , v4l2Window (NULL)
#endif
   , window (NULL)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //format =
    //  static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
    //if (!format)
    //{
    //  ACE_DEBUG ((LM_CRITICAL,
    //              ACE_TEXT ("failed to allocate memory, continuing\n")));
    //} // end IF
    //else
    //  ACE_OS::memset (format, 0, sizeof (struct _AMMediaType));
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    useMediaFoundation = true;
#else
    ACE_OS::memset (&v4l2Format, 0, sizeof (struct v4l2_format));
    v4l2Format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ACE_OS::memset (&v4l2FrameRate, 0, sizeof (struct v4l2_fract));
#endif
  };

  GdkRectangle                     area;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //IGraphBuilder*           builder;
  //struct _AMMediaType*     format;
  IMFMediaType*                    format;
  TOPOID                           rendererNodeId;
  TOPOID                           sampleGrabberNodeId;
  IMFMediaSession*                 session;
  //IVideoWindow*        windowController;
  IMFVideoDisplayControl*          windowController;
#else
  __u32                            buffers; // v4l device buffers
  int                              fileDescriptor;
  enum AVPixelFormat               format;
#endif
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : v4l2 device file (e.g. "/dev/video0" (Linux))
  std::string                      device;
  ACE_SYNCH_MUTEX*                 lock;
  GdkPixbuf*                       pixelBuffer;
  Stream_CamSave_ISessionNotify_t* subscriber;
  Stream_CamSave_Subscribers_t*    subscribers;
  std::string                      targetFileName;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct v4l2_format               v4l2Format;
  struct v4l2_fract                v4l2FrameRate; // time-per-frame (s)
  enum v4l2_memory                 v4l2Method; // v4l camera source
  struct v4l2_window*              v4l2Window;
#endif
  GdkWindow*                       window;
};
//typedef std::map<std::string,
//                 struct Stream_CamSave_ModuleHandlerConfiguration> Stream_CamSave_ModuleHandlerConfigurations_t;
//typedef Stream_CamSave_ModuleHandlerConfigurations_t::const_iterator Stream_CamSave_ModuleHandlerConfigurationsConstIterator_t;
//typedef Stream_CamSave_ModuleHandlerConfigurations_t::iterator Stream_CamSave_ModuleHandlerConfigurationsIterator_t;

struct Stream_CamSave_StreamState
 : Stream_State
{
  Stream_CamSave_StreamState ()
   : Stream_State ()
   , userData (NULL)
  {};

  struct Stream_CamSave_UserData* userData;
};

struct Stream_CamSave_StreamConfiguration
 : Stream_Configuration
{
  Stream_CamSave_StreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {};

  struct Stream_CamSave_UserData* userData;
};

struct Stream_CamSave_Configuration
{
  Stream_CamSave_Configuration ()
   : signalHandlerConfiguration ()
   , streamConfiguration ()
   , userData ()
  {};

  // **************************** signal data **********************************
  struct Stream_CamSave_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_CamSave_StreamConfiguration_t             streamConfiguration;

  struct Stream_CamSave_UserData                   userData;
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_U_ControlMessage_t;

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
//template <typename AllocatorConfigurationType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename UserDataType,
//          typename ControlMessageType,
//          typename DataMessageType>
//class Stream_SessionMessageBase_T;
//typedef Stream_SessionMessageBase_T<struct Stream_AllocatorConfiguration,
//                                    enum Stream_SessionMessageType,
//                                    Test_U_SessionData_t,
//                                    struct Stream_UserData,
//                                    Test_U_ControlMessage_t,
//                                    Test_U_Message_t> Test_U_SessionMessage_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<struct Stream_AllocatorConfiguration,
//                                         Test_U_ControlMessage_t,
//                                         Stream_CamSave_Message,
//                                         Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_Message,
                                          Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_Message,
                                          Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
#endif

typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_CamSave_IStreamNotify_t;

typedef Common_ISubscribe_T<Stream_CamSave_ISessionNotify_t> Stream_CamSave_ISubscribe_t;

//////////////////////////////////////////

typedef std::map<guint, ACE_Thread_ID> Stream_CamSave_PendingActions_t;
typedef Stream_CamSave_PendingActions_t::iterator Stream_CamSave_PendingActionsIterator_t;
typedef std::set<guint> Stream_CamSave_CompletedActions_t;
typedef Stream_CamSave_CompletedActions_t::iterator Stream_CamSave_CompletedActionsIterator_t;
struct Stream_CamSave_GTK_ProgressData
{
  Stream_CamSave_GTK_ProgressData ()
   : completedActions ()
//   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
   , statistic ()
  {};

  Stream_CamSave_CompletedActions_t   completedActions;
//  GdkCursorType                      cursorType;
  struct Common_UI_GTKState*          GTKState;
  Stream_CamSave_PendingActions_t     pendingActions;

  struct Stream_CamSave_StatisticData statistic;
};

struct Stream_CamSave_GTK_CBData
 : Test_U_GTK_CBData
{
  Stream_CamSave_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , pixelBuffer (NULL)
   , progressData ()
   , progressEventSourceId (0)
   , stream (NULL)
   , subscribers ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   //, streamConfiguration (NULL)
#else
   , device (-1)
#endif
  {};

  struct Stream_CamSave_Configuration*   configuration;
  bool                                   isFirst; // first activation ?
  GdkPixbuf*                             pixelBuffer;
  struct Stream_CamSave_GTK_ProgressData progressData;
  guint                                  progressEventSourceId;
  Stream_CamSave_Stream*                 stream;
  Stream_CamSave_Subscribers_t           subscribers;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //IAMStreamConfig*                streamConfiguration;
#else
  int                                    device; // (capture) device file descriptor
#endif
};

struct Stream_CamSave_ThreadData
{
  Stream_CamSave_ThreadData ()
   : CBData (NULL)
   , eventSourceId (0)
   , sessionId (0)
  {};

  struct Stream_CamSave_GTK_CBData* CBData;
  guint                             eventSourceId;
  size_t                            sessionId;
};

typedef Common_UI_GtkBuilderDefinition_T<struct Stream_CamSave_GTK_CBData> Stream_CamSave_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Stream_CamSave_GTK_CBData> Stream_CamSave_GTK_Manager_t;
typedef ACE_Singleton<Stream_CamSave_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> CAMSAVE_UI_GTK_MANAGER_SINGLETON;

#endif
