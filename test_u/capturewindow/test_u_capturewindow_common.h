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

#ifndef TEST_U_CAMERASCREEN_COMMON_H
#define TEST_U_CAMERASCREEN_COMMON_H

#include <list>
#include <map>
#include <string>

#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_tools.h"

#include "common_ui_windowtype_converter.h"

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
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_u_common.h"

// forward declarations
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

enum Test_U_CaptureWindow_ProgramMode
{
  TEST_U_PROGRAMMODE_PRINT_VERSION = 0,
  TEST_U_PROGRAMMODE_NORMAL,
  ////////////////////////////////////////
  TEST_U_PROGRAMMODE_MAX,
  TEST_U_PROGRAMMODE_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_CaptureWindow_DirectShow_MessageData
 : Test_U_DirectShow_MessageData
{
  Test_U_CaptureWindow_DirectShow_MessageData ()
   : Test_U_DirectShow_MessageData () 
  {}
};
typedef Stream_DataBase_T<struct Test_U_CaptureWindow_DirectShow_MessageData> Test_U_CaptureWindow_DirectShow_MessageData_t;

struct Test_U_CaptureWindow_MediaFoundation_MessageData
 : Test_U_MediaFoundation_MessageData
{
  Test_U_CaptureWindow_MediaFoundation_MessageData ()
   : Test_U_MediaFoundation_MessageData ()
  {}
};
typedef Stream_DataBase_T<struct Test_U_CaptureWindow_MediaFoundation_MessageData> Test_U_CaptureWindow_MediaFoundation_MessageData_t;
#else
struct Test_U_CaptureWindow_MessageData
{
  Test_U_CaptureWindow_MessageData ()
  {}
};
typedef Stream_DataBase_T<struct Test_U_CaptureWindow_MessageData> Test_U_CaptureWindow_MessageData_t;
#endif // ACE_WIN32 || ACE_WIN64

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
class Test_U_CaptureWindow_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct _AMMediaType,
                                        struct Test_U_DirectShow_StreamState,
                                        struct Test_U_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_U_CaptureWindow_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct _AMMediaType,
                                   struct Test_U_DirectShow_StreamState,
                                   struct Test_U_StatisticData,
                                   struct Stream_UserData> ()
   , stream (NULL)
  {}

  Test_U_CaptureWindow_DirectShow_SessionData& operator= (const Test_U_CaptureWindow_DirectShow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct _AMMediaType,
                                  struct Test_U_DirectShow_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    stream = rhs_in.stream;

    return *this;
  }
  Test_U_CaptureWindow_DirectShow_SessionData& operator+= (const Test_U_CaptureWindow_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct _AMMediaType,
                                  struct Test_U_DirectShow_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator+= (rhs_in);

    stream = (stream ? stream : rhs_in.stream);

    return *this;
  }

  Stream_Base_t* stream;
};
typedef Stream_SessionData_T<Test_U_CaptureWindow_DirectShow_SessionData> Test_U_CaptureWindow_DirectShow_SessionData_t;

struct Test_U_MediaFoundation_StreamState;
class Test_U_CaptureWindow_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        IMFMediaType*,
                                        struct Test_U_MediaFoundation_StreamState,
                                        struct Test_U_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_U_CaptureWindow_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   IMFMediaType*,
                                   struct Test_U_MediaFoundation_StreamState,
                                   struct Test_U_StatisticData,
                                   struct Stream_UserData> ()
   , rendererNodeId (0)
   , session (NULL)
  {}

  Test_U_CaptureWindow_MediaFoundation_SessionData& operator= (const Test_U_CaptureWindow_MediaFoundation_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  IMFMediaType*,
                                  struct Test_U_MediaFoundation_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    rendererNodeId = (rendererNodeId ? rendererNodeId : rhs_in.rendererNodeId);
    session = (session ? session : rhs_in.session);

    return *this;
  }
  Test_U_CaptureWindow_MediaFoundation_SessionData& operator+= (const Test_U_CaptureWindow_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  IMFMediaType*,
                                  struct Test_U_MediaFoundation_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator+= (rhs_in);

    rendererNodeId = (rendererNodeId ? rendererNodeId : rhs_in.rendererNodeId);
    session = (session ? session : rhs_in.session);

    return *this;
  }

  TOPOID                              rendererNodeId;
  IMFMediaSession*                    session;
};
typedef Stream_SessionData_T<Test_U_CaptureWindow_MediaFoundation_SessionData> Test_U_CaptureWindow_MediaFoundation_SessionData_t;
#else
struct Test_U_StreamState;
class Test_U_CaptureWindow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                        struct Test_U_StreamState,
                                        struct Test_U_StatisticData,
                                        struct Stream_UserData>
{
 public:
  Test_U_CaptureWindow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                   struct Test_U_StreamState,
                                   struct Test_U_StatisticData,
                                   struct Stream_UserData> ()
   , stream (NULL)
  {}

  Test_U_CaptureWindow_SessionData& operator= (const Test_U_CaptureWindow_SessionData& rhs_in)
  {
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                  struct Test_U_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator= (rhs_in);

    stream = rhs_in.stream;

    return *this;
  }
  Test_U_CaptureWindow_SessionData& operator+= (const Test_U_CaptureWindow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                  struct Stream_MediaFramework_FFMPEG_VideoMediaType,
                                  struct Test_U_StreamState,
                                  struct Test_U_StatisticData,
                                  struct Stream_UserData>::operator+= (rhs_in);

    stream = (stream ? stream : rhs_in.stream);

    return *this;
  }

  Stream_Base_t* stream;
};
typedef Stream_SessionData_T<Test_U_CaptureWindow_SessionData> Test_U_CaptureWindow_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_U_Message_T<struct Test_U_CaptureWindow_DirectShow_MessageData,
                         Test_U_CaptureWindow_DirectShow_SessionData_t> Test_U_DirectShow_Message_t;
typedef Test_U_SessionMessage_T<Test_U_DirectShow_Message_t,
                                Test_U_CaptureWindow_DirectShow_SessionData_t> Test_U_DirectShow_SessionMessage_t;
typedef Test_U_Message_T<struct Test_U_CaptureWindow_MediaFoundation_MessageData,
                         Test_U_CaptureWindow_MediaFoundation_SessionData_t> Test_U_MediaFoundation_Message_t;
typedef Test_U_SessionMessage_T<Test_U_MediaFoundation_Message_t,
                                Test_U_CaptureWindow_MediaFoundation_SessionData_t> Test_U_MediaFoundation_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_U_CaptureWindow_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_DirectShow_Message_t,
                                    Test_U_DirectShow_SessionMessage_t> Test_U_DirectShow_ISessionNotify_t;
typedef std::list<Test_U_DirectShow_ISessionNotify_t*> Test_U_DirectShow_Subscribers_t;
typedef Test_U_DirectShow_Subscribers_t::iterator Test_U_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Test_U_CaptureWindow_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_MediaFoundation_Message_t,
                                    Test_U_MediaFoundation_SessionMessage_t> Test_U_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_U_MediaFoundation_ISessionNotify_t*> Test_U_MediaFoundation_Subscribers_t;
typedef Test_U_MediaFoundation_Subscribers_t::iterator Test_U_MediaFoundation_SubscribersIterator_t;
#else
typedef Test_U_Message_T<struct Test_U_CaptureWindow_MessageData,
                         Test_U_CaptureWindow_SessionData_t> Test_U_Message_t;
typedef Test_U_SessionMessage_T<Test_U_Message_t,
                                Test_U_CaptureWindow_SessionData_t> Test_U_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Test_U_CaptureWindow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message_t,
                                    Test_U_SessionMessage_t> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64
struct Test_U_CaptureWindow_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_CaptureWindow_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , window ()
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  union Common_UI_Window                                  window;
};
//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_CaptureWindow_DirectShow_StreamConfiguration;
struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_CaptureWindow_DirectShow_StreamConfiguration,
                               struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration> Test_U_DirectShow_StreamConfiguration_t;
struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration
 : Test_U_CaptureWindow_ModuleHandlerConfiguration
{
  Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_CaptureWindow_ModuleHandlerConfiguration ()
   , builder (NULL)
   , direct3DConfiguration (NULL)
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   //, sourceFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration operator= (const struct Test_U_CaptureWindow_DirectShow_ModuleHandlerConfiguration& rhs_in)
  {
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

    return *this;
  }

  IGraphBuilder*                                        builder;
  struct Stream_MediaFramework_Direct3D_Configuration*  direct3DConfiguration;
  struct Test_U_DirectShow_FilterConfiguration*         filterConfiguration;
  CLSID                                                 filterCLSID;
  struct _AMMediaType                                   outputFormat;
  bool                                                  push;
  //struct _AMMediaType                                   sourceFormat;
  Test_U_DirectShow_ISessionNotify_t*                   subscriber;
  Test_U_DirectShow_Subscribers_t*                      subscribers;
};

struct Test_U_CaptureWindow_MediaFoundation_StreamConfiguration;
struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_CaptureWindow_MediaFoundation_StreamConfiguration,
                               struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration> Test_U_MediaFoundation_StreamConfiguration_t;
struct Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_CaptureWindow_ModuleHandlerConfiguration
{
  Test_U_CaptureWindow_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_CaptureWindow_ModuleHandlerConfiguration ()
   , direct3DConfiguration (NULL)
   , manageMediaSession (false)
   , mediaFoundationConfiguration (NULL)
   , outputFormat (NULL)
   , session (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
  }

  struct Stream_MediaFramework_Direct3D_Configuration*        direct3DConfiguration;
  bool                                                        manageMediaSession;
  struct Stream_MediaFramework_MediaFoundation_Configuration* mediaFoundationConfiguration;
  IMFMediaType*                                               outputFormat;
  IMFMediaSession*                                            session;
  Test_U_MediaFoundation_ISessionNotify_t*                    subscriber;
  Test_U_MediaFoundation_Subscribers_t*                       subscribers;
};
#else
struct Test_U_CaptureWindow_StreamConfiguration;
struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_CaptureWindow_StreamConfiguration,
                               struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
struct Test_U_CaptureWindow_2_ModuleHandlerConfiguration
 : Test_U_CaptureWindow_ModuleHandlerConfiguration
{
  Test_U_CaptureWindow_2_ModuleHandlerConfiguration ()
   : Test_U_CaptureWindow_ModuleHandlerConfiguration ()
   , display ()
   , fullScreen (false)
#if defined (FFMPEG_SUPPORT)
   , outputFormat ()
#endif // FFMPEG_SUPPORT
   , subscriber (NULL)
   , subscribers (NULL)
   , surface (NULL)
   , waylandDisplay (NULL)
  {}

  struct Common_UI_Display                           display;
  bool                                               fullScreen;
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType outputFormat;
#endif // FFMPEG_SUPPORT
  Test_U_ISessionNotify_t*                           subscriber;
  Test_U_Subscribers_t*                              subscribers;
  struct wl_shell_surface*                           surface;
  struct wl_display*                                 waylandDisplay;
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

  Test_U_CaptureWindow_DirectShow_SessionData* sessionData;
};

struct Test_U_MediaFoundation_StreamState
 : Stream_State
{
  Test_U_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_CaptureWindow_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_U_StreamState
 : Stream_State
{
  Test_U_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_CaptureWindow_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_CaptureWindow_StreamConfiguration
 : Stream_Configuration
{
  Test_U_CaptureWindow_StreamConfiguration ()
   : Stream_Configuration ()
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_INVALID)
  {
    printFinalReport = true;
  }
  enum Stream_Visualization_VideoRenderer renderer;
};
struct Test_U_CaptureWindow_DirectShow_StreamConfiguration
 : Test_U_CaptureWindow_StreamConfiguration
{
  Test_U_CaptureWindow_DirectShow_StreamConfiguration ()
   : Test_U_CaptureWindow_StreamConfiguration ()
   , format ()
  {}

  struct _AMMediaType format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_DirectShow_StreamState> Test_U_DirectShow_IStreamControl_t;

struct Test_U_CaptureWindow_MediaFoundation_StreamConfiguration
 : Test_U_CaptureWindow_StreamConfiguration
{
  Test_U_CaptureWindow_MediaFoundation_StreamConfiguration ()
   : Test_U_CaptureWindow_StreamConfiguration ()
   , format (NULL)
  {}

  IMFMediaType* format;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_MediaFoundation_StreamState> Test_U_MediaFoundation_IStreamControl_t;
#else
struct Test_U_CaptureWindow_StreamConfiguration
 : Test_U_StreamConfiguration
{
  Test_U_CaptureWindow_StreamConfiguration ()
   : Test_U_StreamConfiguration ()
   , format ()
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_INVALID)
  {
    printFinalReport = true;
  }

  struct Stream_MediaFramework_FFMPEG_VideoMediaType format; // session data-
  enum Stream_Visualization_VideoRenderer            renderer;
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
struct Test_U_CaptureWindow_Configuration
 : Test_U_Configuration
{
  Test_U_CaptureWindow_Configuration ()
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
