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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "qedit.h"
#else
#include "linux/videodev2.h"

#include "gtk/gtk.h"
#endif

#include "common_inotify.h"
#include "common_isubscribe.h"

#include "stream_common.h"
//#include "stream_data_base.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "stream_directshow_allocator_base.h"
#include "stream_messageallocatorheap_base.h"
#else
#include "stream_messageallocatorheap_base.h"
#endif
#include "stream_session_data.h"

#include "stream_dev_common.h"
#include "stream_dev_defines.h"

#include "test_u_common.h"

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
   , sampleTime (0.0)
#else
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
#endif
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IMediaSample*  sample;
  double         sampleTime;
#else
  int            device; // (capture) device file descriptor
  __u32          index;  // 'index' field of v4l2_buffer
  v4l2_memory    method;
  bool           release;
#endif
};
//typedef Stream_DataBase_T<Stream_CamSave_MessageData> Stream_CamSave_MessageData_t;

struct Stream_CamSave_SessionData
 : Stream_SessionData
{
  inline Stream_CamSave_SessionData ()
   : Stream_SessionData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , sampleGrabber (NULL)
#else
   , format ()
#endif
//   , size (0)
   , targetFileName ()
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ISampleGrabber*     sampleGrabber;
#else
  struct v4l2_format  format;
#endif
  //  unsigned int size;
  std::string  targetFileName;
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
  unsigned int       statisticReportingInterval; // statistics collecting interval (second(s)) [0: off]
};

struct Stream_CamSave_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Stream_CamSave_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , active (false)
   , area ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , builder (NULL)
   , windowController (NULL)
#else
   , bufferMap ()
   , buffers (MODULE_DEV_CAM_V4L_DEFAULT_DEVICE_BUFFERS)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
#endif
   , contextID (0)
   , device ()
   , fileDescriptor (-1)
   , printProgressDot (true)
   , targetFileName ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , v4l2Window (NULL)
#endif
   , window (NULL)
  {};

  bool                active;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct tagRECT      area;
  IGraphBuilder*      builder;
  IVideoWindow*       windowController;
#else
  GdkRectangle        area;
  INDEX2BUFFER_MAP_T  bufferMap;
  __u32               buffers; // v4l device buffers
  v4l2_memory         method; // v4l camera source
#endif
  guint               contextID;
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : v4l2 device file (e.g. "/dev/video0" (Linux))
  std::string         device;
  int                 fileDescriptor;
  bool                printProgressDot;
  std::string         targetFileName;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                window;
#else
  struct v4l2_window* v4l2Window;
  GdkWindow*          window;
#endif
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
   : signalHandlerConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , streamUserData ()
  {};

  Stream_CamSave_SignalHandlerConfiguration signalHandlerConfiguration;

  Stream_ModuleConfiguration                moduleConfiguration;
  Stream_CamSave_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Stream_CamSave_StreamConfiguration        streamConfiguration;

  Stream_UserData                           streamUserData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//typedef Stream_DirectShowAllocatorBase_T<Stream_AllocatorConfiguration,
//
//                                         Stream_CamSave_Message,
//                                         Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,

                                          Stream_CamSave_Message,
                                          Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,

                                          Stream_CamSave_Message,
                                          Stream_CamSave_SessionMessage> Stream_CamSave_MessageAllocator_t;
#endif

typedef Common_INotify_T<Stream_CamSave_SessionData,
                         Stream_CamSave_Message,
                         Stream_CamSave_SessionMessage> Stream_CamSave_IStreamNotify_t;
typedef std::list<Stream_CamSave_IStreamNotify_t*> Stream_CamSave_Subscribers_t;
typedef Stream_CamSave_Subscribers_t::iterator Stream_CamSave_SubscribersIterator_t;

typedef Common_ISubscribe_T<Stream_CamSave_IStreamNotify_t> Stream_CamSave_ISubscribe_t;

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

  Stream_Statistic                  statistic;
};

struct Stream_CamSave_GTK_CBData
 : Stream_Test_U_GTK_CBData
{
  inline Stream_CamSave_GTK_CBData ()
   : Stream_Test_U_GTK_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , progressData ()
   , progressEventSourceID (0)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , streamConfiguration (NULL)
#else
   , device (-1)
#endif
  {};

  Stream_CamSave_Configuration*   configuration;
  bool                            isFirst; // first activation ?
  Stream_CamSave_GTK_ProgressData progressData;
  guint                           progressEventSourceID;
  Stream_CamSave_Stream*          stream;
  Stream_CamSave_Subscribers_t    subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX       subscribersLock;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IAMStreamConfig*                streamConfiguration;
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
