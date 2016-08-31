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

#ifndef TEST_U_AUDIOEFFECT_COMMON_H
#define TEST_U_AUDIOEFFECT_COMMON_H

#include <list>
#include <map>
#include <string>

#include "strmif.h"
#include "mfapi.h"
#include "mfidl.h"

#include "gtk/gtk.h"

#include "ace/config-lite.h"

#include "common_isubscribe.h"
#include "common_tools.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "test_u_common.h"
#include "test_u_defines.h"

// forward declarations
class Stream_IAllocator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_U_AudioEffect_DirectShow_Message;
class Test_U_AudioEffect_DirectShow_SessionMessage;
class Test_U_AudioEffect_DirectShow_Stream;
class Test_U_AudioEffect_MediaFoundation_Message;
class Test_U_AudioEffect_MediaFoundation_SessionMessage;
class Test_U_AudioEffect_MediaFoundation_Stream;
#else
class Test_U_AudioEffect_Message;
class Test_U_AudioEffect_SessionMessage;
class Test_U_AudioEffect_Stream;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_MessageData
{
  inline Test_U_AudioEffect_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMediaSample* sample;
  double      sampleTime;
};
struct Test_U_AudioEffect_MediaFoundation_MessageData
{
  inline Test_U_AudioEffect_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMFSample* sample;
  LONGLONG   sampleTime;
};
#else
struct Test_U_AudioEffect_MessageData
{
  inline Test_U_AudioEffect_MessageData ()
   : device (-1)
   , index (0)
   , method (MODULE_DEV_CAM_V4L_DEFAULT_IO_METHOD)
   , release (false)
  {};

  int         device; // (capture) device file descriptor
  __u32       index;  // 'index' field of v4l2_buffer
  v4l2_memory method;
  bool        release;
};
#endif

struct Test_U_AudioEffect_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  inline Test_U_AudioEffect_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , area ()
   , audioOutput (0)
   , device ()
   , gdkWindow (NULL)
   , pixelBuffer (NULL)
   , targetFileName ()
  {};

  GdkRectangle area;
  int          audioOutput;
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : alsa device file (e.g. "/dev/alsa" (Linux))
  std::string  device;
  GdkWindow*   gdkWindow;
  GdkPixbuf*   pixelBuffer;
  std::string  targetFileName;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  inline Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , builder (NULL)
   , format (NULL)
  {};

  IGraphBuilder*       builder;
  struct _AMMediaType* format;
};
struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  inline Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , format (NULL)
   , sampleGrabberNodeId (0)
   , session (NULL)
  {
    HRESULT result = MFCreateMediaType (&format);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  };

  IMFMediaType*    format;
  TOPOID           sampleGrabberNodeId;
  IMFMediaSession* session;
};
#endif

typedef Test_U_SessionData Test_U_AudioEffect_SessionData;
typedef Stream_SessionData_T<Test_U_AudioEffect_SessionData> Test_U_AudioEffect_SessionData_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_SessionData
 : Test_U_AudioEffect_SessionData
{
  inline Test_U_AudioEffect_DirectShow_SessionData ()
   : Test_U_AudioEffect_SessionData ()
   , builder (NULL)
   , format (NULL)
  {};

  IGraphBuilder*       builder;
  struct _AMMediaType* format;
};
typedef Stream_SessionData_T<Test_U_AudioEffect_DirectShow_SessionData> Test_U_AudioEffect_DirectShow_SessionData_t;
struct Test_U_AudioEffect_MediaFoundation_SessionData
 : Test_U_AudioEffect_SessionData
{
  inline Test_U_AudioEffect_MediaFoundation_SessionData ()
   : Test_U_AudioEffect_SessionData ()
   , format (NULL)
   , rendererNodeId (0)
   , session (NULL)
  {};

  IMFMediaType*    format;
  TOPOID           rendererNodeId;
  IMFMediaSession* session;
};
typedef Stream_SessionData_T<Test_U_AudioEffect_MediaFoundation_SessionData> Test_U_AudioEffect_MediaFoundation_SessionData_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_StreamConfiguration
 : Stream_Configuration
{
  inline Test_U_AudioEffect_DirectShow_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  inline Test_U_AudioEffect_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
#else
struct Test_U_AudioEffect_StreamConfiguration
 : Stream_Configuration
{
  inline Test_U_AudioEffect_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Test_U_AudioEffect_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
#endif

struct Test_U_AudioEffect_SignalHandlerConfiguration
{
  inline Test_U_AudioEffect_SignalHandlerConfiguration ()
   : actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistics collecting interval (second(s)) [0: off]
};

struct Test_U_AudioEffect_Configuration
 : Test_U_Configuration
{
  inline Test_U_AudioEffect_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
  {};

  Test_U_AudioEffect_SignalHandlerConfiguration signalHandlerConfiguration;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_Configuration
 : Test_U_AudioEffect_Configuration
{
  inline Test_U_AudioEffect_DirectShow_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
  {};

  Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_U_AudioEffect_DirectShow_StreamConfiguration        streamConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_Configuration
 : Test_U_AudioEffect_Configuration
{
  inline Test_U_AudioEffect_MediaFoundation_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
  {};

  Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration        streamConfiguration;
};
#endif

typedef Stream_INotify_T<Stream_SessionMessageType> Test_U_AudioEffect_IStreamNotify_t;
typedef Stream_IStreamControl_T<Stream_ControlType,
                                Stream_SessionMessageType,
                                Stream_StateMachine_ControlState,
                                Stream_State> Test_U_AudioEffect_IStreamControl_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Test_U_AudioEffect_DirectShow_Message,
                                Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                          Test_U_AudioEffect_DirectShow_Message,
                                          Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_U_AudioEffect_DirectShow_SessionData,
                                    Stream_SessionMessageType,
                                    Test_U_AudioEffect_DirectShow_Message,
                                    Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_DirectShow_ISessionNotify_t*> Test_U_AudioEffect_DirectShow_Subscribers_t;
typedef Test_U_AudioEffect_DirectShow_Subscribers_t::iterator Test_U_AudioEffect_DirectShow_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_U_AudioEffect_DirectShow_ISessionNotify_t> Test_U_AudioEffect_DirectShow_ISubscribe_t;

typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Test_U_AudioEffect_MediaFoundation_Message,
                                Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                          Test_U_AudioEffect_MediaFoundation_Message,
                                          Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_U_AudioEffect_MediaFoundation_SessionData,
                                    Stream_SessionMessageType,
                                    Test_U_AudioEffect_MediaFoundation_Message,
                                    Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*> Test_U_AudioEffect_MediaFoundation_Subscribers_t;
typedef Test_U_AudioEffect_MediaFoundation_Subscribers_t::iterator Test_U_AudioEffect_MediaFoundation_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t> Test_U_AudioEffect_MediaFoundation_ISubscribe_t;
#else
typedef Stream_ControlMessage_T<Stream_ControlMessageType,
                                Stream_AllocatorConfiguration,
                                Test_U_AudioEffect_Message,
                                Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_ControlMessage_t,
                                          Test_U_AudioEffect_Message,
                                          Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Test_U_AudioEffect_SessionData,
                                    Stream_SessionMessageType,
                                    Test_U_AudioEffect_Message,
                                    Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_ISessionNotify_t*> Test_U_AudioEffect_Subscribers_t;
typedef Test_U_AudioEffect_Subscribers_t::iterator Test_U_AudioEffect_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_U_AudioEffect_ISessionNotify_t> Test_U_AudioEffect_ISubscribe_t;
#endif

//////////////////////////////////////////

typedef std::map<guint, ACE_Thread_ID> Test_U_AudioEffect_PendingActions_t;
typedef Test_U_AudioEffect_PendingActions_t::iterator Test_U_AudioEffect_PendingActionsIterator_t;
typedef std::set<guint> Test_U_AudioEffect_CompletedActions_t;
typedef Test_U_AudioEffect_CompletedActions_t::iterator Test_U_AudioEffect_CompletedActionsIterator_t;
struct Test_U_AudioEffect_GTK_ProgressData
{
  inline Test_U_AudioEffect_GTK_ProgressData ()
   : completedActions ()
//   , cursorType (GDK_LAST_CURSOR)
   , GTKState (NULL)
   , pendingActions ()
   , processed (0)
   , statistic ()
  {};

  Test_U_AudioEffect_CompletedActions_t completedActions;
//  GdkCursorType                      cursorType;
  Common_UI_GTKState   *                GTKState;
  Test_U_AudioEffect_PendingActions_t   pendingActions;
  size_t                                processed; // bytes
  Test_U_RuntimeStatistic_t             statistic;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Test_U_AudioEffect_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , isFirst (true)
   , pixelBuffer (NULL)
   , progressData ()
   , progressEventSourceID (0)
   , subscribersLock ()
   , useMediaFoundation (TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
  {};

  bool                                isFirst; // first activation ?
  GdkPixbuf*                          pixelBuffer;
  Test_U_AudioEffect_GTK_ProgressData progressData;
  guint                               progressEventSourceID;
  ACE_SYNCH_RECURSIVE_MUTEX           subscribersLock;
  bool                                useMediaFoundation;
};
struct Test_U_AudioEffect_DirectShow_GTK_CBData
 : Test_U_AudioEffect_GTK_CBData
{
  inline Test_U_AudioEffect_DirectShow_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {};

  Test_U_AudioEffect_DirectShow_Configuration* configuration;
  Test_U_AudioEffect_DirectShow_Stream*        stream;
  IAMStreamConfig*                             streamConfiguration;
  Test_U_AudioEffect_DirectShow_Subscribers_t  subscribers;
};
struct Test_U_AudioEffect_MediaFoundation_GTK_CBData
 : Test_U_AudioEffect_GTK_CBData
{
  inline Test_U_AudioEffect_MediaFoundation_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBData ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
  {};

  Test_U_AudioEffect_MediaFoundation_Configuration* configuration;
  Test_U_AudioEffect_MediaFoundation_Stream*        stream;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t  subscribers;
};
#else
struct Test_U_AudioEffect_GTK_CBData
 : Test_U_GTK_CBData
{
  inline Test_U_AudioEffect_GTK_CBData ()
   : Test_U_GTK_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , pixelBuffer (NULL)
   , progressData ()
   , progressEventSourceID (0)
   , stream (NULL)
   , subscribers ()
   , subscribersLock ()
  {};

  Test_U_AudioEffect_Configuration*   configuration;
  bool                                isFirst; // first activation ?
  GdkPixbuf*                          pixelBuffer;
  Test_U_AudioEffect_GTK_ProgressData progressData;
  guint                               progressEventSourceID;
  Test_U_AudioEffect_Stream*          stream;
  Test_U_AudioEffect_Subscribers_t    subscribers;
  ACE_SYNCH_RECURSIVE_MUTEX           subscribersLock;
};
#endif

struct Test_U_AudioEffect_ThreadData
{
  inline Test_U_AudioEffect_ThreadData ()
   : eventSourceID (0)
   , sessionID (0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
#endif
  {};

  guint  eventSourceID;
  size_t sessionID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool   useMediaFoundation;
#endif
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_ThreadData
 : Test_U_AudioEffect_ThreadData
{
  inline Test_U_AudioEffect_DirectShow_ThreadData ()
   : Test_U_AudioEffect_ThreadData ()
   , CBData (NULL)
  {};

  Test_U_AudioEffect_DirectShow_GTK_CBData* CBData;
};
struct Test_U_AudioEffect_MediaFoundation_ThreadData
 : Test_U_AudioEffect_ThreadData
{
  inline Test_U_AudioEffect_MediaFoundation_ThreadData ()
   : Test_U_AudioEffect_ThreadData ()
   , CBData (NULL)
  {};

  Test_U_AudioEffect_MediaFoundation_GTK_CBData* CBData;
};
#endif

#endif
