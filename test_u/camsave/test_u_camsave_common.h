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

#include <ace/config-lite.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <d3d9.h>
#include <evr.h>
#include <mfapi.h>
#include <mfobjects.h>
#include <mfreadwrite.h>
#include <strmif.h>
#else
#include <linux/videodev2.h>

#include <gtk/gtk.h>
#endif

#include "common_isubscribe.h"
#include "common_tools.h"

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

struct Stream_CamSave_MessageData
{
  inline Stream_CamSave_MessageData ()
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
  int             device; // (capture) device file descriptor
  __u32           index;  // 'index' field of v4l2_buffer
  v4l2_memory     method;
  bool            release;
#endif
};

struct Stream_CamSave_StatisticData
 : Stream_Statistic
{
  inline Stream_CamSave_StatisticData ()
   : Stream_Statistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , capturedFrames (0)
#endif
  {};

  inline Stream_CamSave_StatisticData operator+= (const Stream_CamSave_StatisticData& rhs_in)
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

struct Stream_CamSave_SessionData
 : Test_U_SessionData
{
  inline Stream_CamSave_SessionData ()
   : Test_U_SessionData ()
   , currentStatistic ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DDevice (NULL)
   , format (NULL)
   , rendererNodeId (0)
   , resetToken (0)
   , session (NULL)
#else
   , format (NULL)
   , frameRate (NULL)
#endif
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

  inline Stream_CamSave_SessionData operator+= (const Stream_CamSave_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Test_U_SessionData::operator+= (rhs_in);

    // *NOTE*: the idea is to 'merge' the data
    currentStatistic += rhs_in.currentStatistic;
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

  Stream_CamSave_StatisticData currentStatistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IDirect3DDevice9Ex*          direct3DDevice;
  struct _AMMediaType*         format;
  TOPOID                       rendererNodeId;
  UINT                         resetToken; // direct 3D manager 'id'
  IMFMediaSession*             session;
#else
  struct v4l2_format*          format;
  struct v4l2_fract*           frameRate; // time-per-frame
#endif
};
typedef Stream_SessionData_T<Stream_CamSave_SessionData> Stream_CamSave_SessionData_t;

struct Stream_CamSave_SignalHandlerConfiguration
{
  inline Stream_CamSave_SignalHandlerConfiguration ()
   : actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // (statistic) reporting interval (second(s)) [0: off]
};

struct Stream_CamSave_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  inline Stream_CamSave_ModuleHandlerConfiguration ()
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
   , format ()
   , frameRate ()
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
#endif
   , device ()
   , lock (NULL)
   , pixelBuffer (NULL)
   , targetFileName ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , v4l2Window (NULL)
#endif
   , gdkWindow (NULL)
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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#else
    ACE_OS::memset (&format, 0, sizeof (format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ACE_OS::memset (&frameRate, 0, sizeof (frameRate));
#endif
  };

  GdkRectangle               area;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //IGraphBuilder*           builder;
  //struct _AMMediaType*     format;
  IMFMediaType*              format;
  TOPOID                     rendererNodeId;
  TOPOID                     sampleGrabberNodeId;
  IMFMediaSession*           session;
  //IVideoWindow*        windowController;
  IMFVideoDisplayControl*    windowController;
#else
  __u32                      buffers; // v4l device buffers
  int                        fileDescriptor;
  struct v4l2_format         format;
  struct v4l2_fract          frameRate; // time-per-frame (s)
  v4l2_memory                method; // v4l camera source
#endif
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : v4l2 device file (e.g. "/dev/video0" (Linux))
  std::string                device;
  ACE_SYNCH_MUTEX*           lock;
  GdkPixbuf*                 pixelBuffer;
  std::string                targetFileName;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct v4l2_window*        v4l2Window;
#endif
  GdkWindow*                 gdkWindow;
};

struct Stream_CamSave_StreamConfiguration
 : Stream_Configuration
{
  inline Stream_CamSave_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Stream_CamSave_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Stream_CamSave_Configuration
{
  inline Stream_CamSave_Configuration ()
   : allocatorConfiguration ()
   , signalHandlerConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , streamUserData ()
  {};

  // ***************************** allocator ***********************************
  Stream_AllocatorConfiguration             allocatorConfiguration;
  // **************************** signal data **********************************
  Stream_CamSave_SignalHandlerConfiguration signalHandlerConfiguration;
  // **************************** stream data **********************************
  Stream_ModuleConfiguration                moduleConfiguration;
  Stream_CamSave_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Stream_CamSave_StreamConfiguration        streamConfiguration;

  Stream_UserData                           streamUserData;
};

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Stream_CamSave_Message,
                                Stream_CamSave_SessionMessage> Test_U_ControlMessage_t;

//template <typename AllocatorConfigurationType,
//          typename CommandType,
//          typename ControlMessageType,
//          typename SessionMessageType>
//class Stream_MessageBase_T;
//typedef Stream_MessageBase_T<Stream_AllocatorConfiguration,
//                             int,
//                             Test_U_ControlMessage_t,
//                             Test_U_SessionMessage_t> Test_U_Message_t;

//typedef Stream_SessionData_T<Stream_SessionData> Test_U_SessionData_t;
//template <typename AllocatorConfigurationType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename UserDataType,
//          typename ControlMessageType,
//          typename DataMessageType>
//class Stream_SessionMessageBase_T;
//typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
//                                    Stream_SessionMessageType,
//                                    Test_U_SessionData_t,
//                                    Stream_UserData,
//                                    Test_U_ControlMessage_t,
//                                    Test_U_Message_t> Test_U_SessionMessage_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<Stream_AllocatorConfiguration,
//                                         Test_U_ControlMessage_t,
//                                         Stream_CamSave_Message,
//                                         Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_Message,
                                          Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Stream_CamSave_Message,
                                          Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
#endif

typedef Stream_INotify_T<Stream_SessionMessageType> Stream_CamSave_IStreamNotify_t;
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Stream_CamSave_SessionData,
                                    Stream_SessionMessageType,
                                    Stream_CamSave_Message,
                                    Stream_CamSave_SessionMessage> Stream_CamSave_ISessionNotify_t;
typedef std::list<Stream_CamSave_ISessionNotify_t*> Stream_CamSave_Subscribers_t;
typedef Stream_CamSave_Subscribers_t::iterator Stream_CamSave_SubscribersIterator_t;

typedef Common_ISubscribe_T<Stream_CamSave_ISessionNotify_t> Stream_CamSave_ISubscribe_t;

typedef std::map<guint, ACE_Thread_ID> Stream_CamSave_PendingActions_t;
typedef Stream_CamSave_PendingActions_t::iterator Stream_CamSave_PendingActionsIterator_t;
typedef std::set<guint> Stream_CamSave_CompletedActions_t;
typedef Stream_CamSave_CompletedActions_t::iterator Stream_CamSave_CompletedActionsIterator_t;
struct Stream_CamSave_GTK_ProgressData
{
  inline Stream_CamSave_GTK_ProgressData ()
   : completedActions ()
//   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
   , statistic ()
  {};

  Stream_CamSave_CompletedActions_t completedActions;
//  GdkCursorType                      cursorType;
  Common_UI_GTKState*               GTKState;
  Stream_CamSave_PendingActions_t   pendingActions;

  Stream_CamSave_StatisticData      statistic;
};

struct Stream_CamSave_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Stream_CamSave_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , pixelBuffer (NULL)
   , progressData ()
   , progressEventSourceID (0)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   //, streamConfiguration (NULL)
#else
   , device (-1)
#endif
  {};

  Stream_CamSave_Configuration*   configuration;
  bool                            isFirst; // first activation ?
  GdkPixbuf*                      pixelBuffer;
  Stream_CamSave_GTK_ProgressData progressData;
  guint                           progressEventSourceID;
  Stream_CamSave_Stream*          stream;
  Stream_CamSave_Subscribers_t    subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX       subscribersLock;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //IAMStreamConfig*                streamConfiguration;
#else
  int                             device; // (capture) device file descriptor
#endif
};

struct Stream_CamSave_ThreadData
{
  inline Stream_CamSave_ThreadData ()
   : CBData (NULL)
   , eventSourceID (0)
   , sessionID (0)
  {};

  Stream_CamSave_GTK_CBData* CBData;
  guint                      eventSourceID;
  size_t                     sessionID;
};

#endif
