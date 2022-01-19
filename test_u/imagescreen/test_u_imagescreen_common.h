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

#ifndef TEST_U_IMAGESCREEN_COMMON_H
#define TEST_U_IMAGESCREEN_COMMON_H

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "X11/Xlib.h"
#undef CursorShape
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_SUPPORT)
#undef DrawText
#include "wx/wx.h"
#include "wx/apptrait.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#include "common_isubscribe.h"

#if defined (GUI_SUPPORT)
#if defined (WXWIDGETS_SUPPORT)
#include "common_ui_wxwidgets_application.h"
#include "common_ui_wxwidgets_xrc_definition.h"
#endif // WXWIDGETS_SUPPORT
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directdraw_common.h"
#else
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_common.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_u_common.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT
#if defined (QT_SUPPORT)
#include "test_u_qt_common.h"
#endif // QT_SUPPORT
#endif // GUI_SUPPORT

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Stream_ImageScreen_EventHandler_T;

typedef Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                      struct _AMMediaType,
#else
#if defined (FFMPEG_SUPPORT)
                                      struct Stream_MediaFramework_FFMPEG_VideoMediaType,
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                                      struct Stream_State,
                                      struct Stream_Statistic,
                                      struct Stream_UserData> Stream_ImageScreen_SessionData;
typedef Stream_SessionData_T<Stream_ImageScreen_SessionData> Stream_ImageScreen_SessionData_t;

template <typename SessionDataType>
class Stream_ImageScreen_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_ImageScreen_SessionMessage_T;
typedef Stream_ImageScreen_Message_T<Stream_ImageScreen_SessionData_t> Stream_ImageScreen_Message_t;
typedef Stream_ImageScreen_SessionMessage_T<Stream_ImageScreen_Message_t,
                                            Stream_ImageScreen_SessionData_t> Stream_ImageScreen_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_ImageScreen_SessionData,
                                    enum Stream_SessionMessageType,
                                    Stream_ImageScreen_Message_t,
                                    Stream_ImageScreen_SessionMessage_t> Stream_ImageScreen_ISessionNotify_t;
typedef std::list<Stream_ImageScreen_ISessionNotify_t*> Stream_ImageScreen_Subscribers_t;
typedef Stream_ImageScreen_Subscribers_t::iterator Stream_ImageScreen_SubscribersIterator_t;
struct Stream_ImageScreen_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Stream_ImageScreen_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecId (AV_CODEC_ID_NONE)
#endif // FFMPEG_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
   , codecFormat (AV_PIX_FMT_NONE)
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
   , delayConfiguration (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
   , display ()
   , fileIdentifier ()
   , fullScreen (false)
   , individualFormat (true)
   , outputFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GTK_USE)
//   , window (NULL)
#endif // GTK_USE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , window (NULL)
#else
   , window (0)
#endif // ACE_WIN32 || ACE_WIN64
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
  }

#if defined (FFMPEG_SUPPORT)
  enum AVCodecID                                codecId;
#endif // FFMPEG_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
  enum AVPixelFormat                            codecFormat; // preferred output-
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  struct Stream_Miscellaneous_DelayConfiguration* delayConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration* direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Common_UI_DisplayDevice                display; // display module
#else
  struct Common_UI_Display                      display; // display module
#endif // ACE_WIN32 || ACE_WIN64
  Common_File_Identifier                        fileIdentifier; // source module
  // *NOTE*: treat each image separately (different sizes)
  bool                                          fullScreen;
  bool                                          individualFormat;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                           outputFormat;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType outputFormat;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  Stream_ImageScreen_ISessionNotify_t*          subscriber;
  Stream_ImageScreen_Subscribers_t*             subscribers;
#if defined (GTK_USE)
//  GdkWindow*                                    window;
#endif // GTK_USE
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                          window;
#else
  Window                                        window;
#endif // ACE_WIN32 || ACE_WIN64
};
//extern const char stream_name_string_[];
struct Stream_ImageScreen_StreamConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_ImageScreen_StreamConfiguration,
                               struct Stream_ImageScreen_ModuleHandlerConfiguration> Stream_ImageScreen_StreamConfiguration_t;

struct Stream_ImageScreen_StreamState
 : Stream_State
{
  Stream_ImageScreen_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Stream_ImageScreen_SessionData* sessionData;
};

struct Stream_ImageScreen_StreamConfiguration
 : Stream_Configuration
{
  Stream_ImageScreen_StreamConfiguration ()
   : Stream_Configuration ()
//   , format ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    , renderer (STREAM_VISUALIZATION_VIDEORENDERER_DIRECTDRAW_3D)
#else
    , renderer (STREAM_VISUALIZATION_VIDEORENDERER_X11)
#endif // ACE_WIN32 || ACE_WIN64
  {
    printFinalReport = true;
  }

//  struct Stream_MediaFramework_FFMPEG_VideoMediaType format;
  enum Stream_Visualization_VideoRenderer       renderer;
};

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_ImageScreen_StreamState> Stream_ImageScreen_IStreamControl_t;

struct Stream_ImageScreen_Configuration
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
{
  Stream_ImageScreen_Configuration ()
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
   , delayConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , direct3DConfiguration ()
#endif // ACE_WIN32 || ACE_WIN64
   , timerConfiguration ()
   , streamConfiguration ()
  {}

  struct Stream_Miscellaneous_DelayConfiguration      delayConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_Direct3D_Configuration direct3DConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  struct Common_TimerConfiguration                    timerConfiguration;
  Stream_ImageScreen_StreamConfiguration_t            streamConfiguration;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_ImageScreen_Message_t,
                                          Stream_ImageScreen_SessionMessage_t> Stream_ImageScreen_MessageAllocator_t;

typedef Common_ISubscribe_T<Stream_ImageScreen_ISessionNotify_t> Stream_ImageScreen_ISubscribe_t;

typedef Stream_ImageScreen_EventHandler_T<Stream_ImageScreen_ISessionNotify_t,
                                          Stream_ImageScreen_Message_t,
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
#endif // GTK_USE || WXWIDGETS_USE || QT_USE
#endif // GUI_SUPPORT
                                          Stream_ImageScreen_SessionMessage_t> Stream_ImageScreen_EventHandler_t;

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Stream_ImageScreen_ProgressData
 : Test_U_UI_ProgressData
{
  Stream_ImageScreen_ProgressData ()
   : Test_U_UI_ProgressData ()
   , current (0)
   , total (0)
  {}

  unsigned int current;
  unsigned int total;
};

class Stream_ImageScreen_Stream;
struct Stream_ImageScreen_UI_CBData
 : Test_U_UI_CBData
{
  Stream_ImageScreen_UI_CBData ()
   : Test_U_UI_CBData ()
   , configuration (NULL)
   , isFirst (true)
   , progressData ()
   , stream (NULL)
   , subscribers ()
  {
    progressData.state = UIState;
  }

  struct Stream_ImageScreen_Configuration* configuration;
  bool                                     isFirst; // first activation ?
  struct Stream_ImageScreen_ProgressData   progressData;
  Stream_ImageScreen_Stream*               stream;
  Stream_ImageScreen_Subscribers_t         subscribers;
};

struct Stream_ImageScreen_UI_ThreadData
 : Test_U_UI_ThreadData
{
  Stream_ImageScreen_UI_ThreadData ()
   : Test_U_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Stream_ImageScreen_UI_CBData* CBData;
};
#endif // GUI_SUPPORT

#endif
