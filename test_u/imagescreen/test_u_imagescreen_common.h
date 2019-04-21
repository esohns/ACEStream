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

//#include <wx/apptrait.h>

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_session_data.h"

#include "stream_lib_ffmpeg_common.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

#include "test_u_common.h"
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#elif defined (QT_USE)
#include "test_u_qt_common.h"
#endif // GTK_USE

// forward declarations
template <typename NotificationType,
          typename DataMessageType,
          typename UIStateType,
#if defined (WXWIDGETS_USE)
          typename InterfaceType, // implements Common_UI_wxWidgets_IApplicationBase_T
#endif // WXWIDGETS_USE
          typename SessionMessageType>
class Stream_ImageScreen_EventHandler_T;

class Stream_ImageScreen_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                        struct Stream_MediaFramework_FFMPEG_MediaType,
                                        struct Stream_State,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Stream_ImageScreen_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
                                   struct Stream_MediaFramework_FFMPEG_MediaType,
                                   struct Stream_State,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

//  Stream_ImageScreen_SessionData& operator+= (const Stream_ImageScreen_SessionData& rhs_in)
//  {
//    // *NOTE*: the idea is to 'merge' the data
//    Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
//                                  struct Stream_MediaFramework_FFMPEG_MediaType,
//                                  struct Stream_State,
//                                  struct Stream_Statistic,
//                                  struct Stream_UserData>::operator+= (rhs_in);

//    return *this;
//  }

 private:
//  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_SessionData (const Stream_ImageScreen_SessionData&))
  ACE_UNIMPLEMENTED_FUNC (Stream_ImageScreen_SessionData& operator= (const Stream_ImageScreen_SessionData&))
};
typedef Stream_SessionData_T<Stream_ImageScreen_SessionData> Stream_ImageScreen_SessionData_t;

template <typename SessionDataType>
class Stream_ImageScreen_Message_T;
template <typename DataMessageType,
          typename SessionDataType>
class Stream_ImageScreen_SessionMessage_T;
typedef Stream_ImageScreen_Message_T<Stream_ImageScreen_SessionData_t> Stream_ImageScreen_Message_t;
typedef Stream_ImageScreen_SessionMessage_T<Stream_ImageScreen_Message_t,
                                            Stream_ImageScreen_SessionData_t> Stream_ImageScreen_SessionMessage_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    Stream_ImageScreen_SessionData,
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
   , codecFormat (AV_PIX_FMT_NONE)
   , codecId (AV_CODEC_ID_NONE)
   , delay (5, 0)
   , display ()
   , fileIdentifier ()
   , fullScreen (false)
   , individualFormat (true)
   , outputFormat ()
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GTK_USE)
   , window (NULL)
#else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , window (NULL)
#else
   , window (None)
   , X11Display (NULL)
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTK_USE
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_ACTIVE;
    hasHeader = true;
  }

  enum AVPixelFormat                            codecFormat; // preferred output-
  enum AVCodecID                                codecId;
  ACE_Time_Value                                delay;
  struct Common_UI_DisplayDevice                display; // display module
  Common_File_Identifier                        fileIdentifier; // source module
  // *NOTE*: treat each image separately (different sizes)
  bool                                          individualFormat;
  bool                                          fullScreen;
  struct Stream_MediaFramework_FFMPEG_MediaType outputFormat;
  Stream_ImageScreen_ISessionNotify_t*          subscriber;
  Stream_ImageScreen_Subscribers_t*             subscribers;
#if defined (GTK_USE)
  GdkWindow*                                    window;
#else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HWND                                          window;
#else
  Window                                        window;
  Display*                                      X11Display;
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTK_USE
};
//extern const char stream_name_string_[];
struct Stream_ImageScreen_StreamConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Stream_ImageScreen_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
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
   , format ()
   , renderer (STREAM_VISUALIZATION_VIDEORENDERER_X11)
  {
    printFinalReport = true;
  }

  struct Stream_MediaFramework_FFMPEG_MediaType format;
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
   , timerConfiguration ()
   , streamConfiguration ()
  {}

  struct Common_TimerConfiguration         timerConfiguration;
  Stream_ImageScreen_StreamConfiguration_t streamConfiguration;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Stream_ImageScreen_Message_t,
                                          Stream_ImageScreen_SessionMessage_t> Stream_ImageScreen_MessageAllocator_t;

#if defined (WXWIDGETS_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_ImageScreen_DirectShow_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_ImageScreen_DirectShow_UI_CBData> Stream_ImageScreen_DirectShow_WxWidgetsIApplication_t;
struct Stream_ImageScreen_MediaFoundation_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_ImageScreen_MediaFoundation_UI_CBData> Stream_ImageScreen_MediaFoundation_WxWidgetsIApplication_t;
#else
struct Stream_ImageScreen_UI_CBData;
typedef Common_UI_wxWidgets_IApplication_T<struct Common_UI_wxWidgets_State,
                                           struct Stream_ImageScreen_UI_CBData> Stream_ImageScreen_WxWidgetsIApplication_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif // WXWIDGETS_USE
typedef Common_ISubscribe_T<Stream_ImageScreen_ISessionNotify_t> Stream_ImageScreen_ISubscribe_t;

typedef Stream_ImageScreen_EventHandler_T<Stream_ImageScreen_ISessionNotify_t,
                                          Stream_ImageScreen_Message_t,
#if defined (GTK_USE)
                                          Common_UI_GTK_State_t,
#elif defined (WXWIDGETS_USE)
                                          struct Common_UI_wxWidgets_State,
                                          Common_UI_wxWidgets_IApplicationBase_t,
#elif defined (QT_USE)
                                          struct Common_UI_Qt_State,
#endif
                                          Stream_ImageScreen_SessionMessage_t> Stream_ImageScreen_EventHandler_t;

//////////////////////////////////////////

struct Stream_ImageScreen_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_ProgressData
#elif defined (QT_USE)
 : Test_U_Qt_ProgressData
#endif
{
  Stream_ImageScreen_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_ProgressData ()
#elif defined (QT_USE)
   : Test_U_Qt_ProgressData ()
#endif
   , current (0)
   , total (0)
  {}

  unsigned int current;
  unsigned int total;
};

class Stream_ImageScreen_Stream;
struct Stream_ImageScreen_UI_CBData
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#elif defined (QT_USE)
 : Test_U_Qt_CBData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_CBData
#endif
{
  Stream_ImageScreen_UI_CBData ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#elif defined (QT_USE)
   : Test_U_Qt_CBData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_CBData ()
#endif
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
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#elif defined (QT_USE)
 : Test_U_Qt_ThreadData
#elif defined (WXWIDGETS_USE)
 : Test_U_wxWidgets_ThreadData
#endif
{
  Stream_ImageScreen_UI_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
#elif defined (QT_USE)
   : Test_U_Qt_ThreadData ()
#elif defined (WXWIDGETS_USE)
   : Test_U_wxWidgets_ThreadData ()
#endif
   , CBData (NULL)
  {}

  struct Stream_ImageScreen_UI_CBData* CBData;
};

#if defined (GTK_USE)
#elif defined (WXWIDGETS_USE)
extern const char toplevel_widget_classname_string_[];
typedef Common_UI_WxWidgetsXRCDefinition_T<struct Common_UI_wxWidgets_State,
                                           toplevel_widget_classname_string_> Stream_ImageScreen_WxWidgetsXRCDefinition_t;
typedef Stream_ImageScreen_WxWidgetsDialog_T<wxDialog_main,
                                             Stream_ImageScreen_WxWidgetsIApplication_t,
                                             Stream_ImageScreen_Stream> Stream_ImageScreen_WxWidgetsDialog_t;
typedef Comon_UI_WxWidgets_Application_T<Stream_ImageScreen_WxWidgetsXRCDefinition_t,
                                         struct Common_UI_wxWidgets_State,
                                         struct Stream_ImageScreen_UI_CBData,
                                         Stream_ImageScreen_WxWidgetsDialog_t,
                                         wxGUIAppTraits> Stream_ImageScreen_WxWidgetsApplication_t;
#endif

#endif
