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

#include <ace/config-lite.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <strmif.h>
#include <mfapi.h>
#include <mfidl.h>
#include <dsound.h>
#else
#include <alsa/asoundlib.h>
#endif

#if defined (GTKGL_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <gl/GL.h>
#else
#include <GL/gl.h>
#endif
#endif

#include <gtk/gtk.h>
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#include <gtkgl/gdkgl.h>
#endif
#else
#if defined (GTKGLAREA_SUPPORT)
#include <gtkgl/gdkgl.h>
#else
#include <gtk/gtkgl.h> // gtkglext
#endif
#endif
#endif

#include "common_isubscribe.h"
#include "common_tools.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_common.h"
#else
#include "stream_dev_common.h"
#endif
#include "stream_vis_defines.h"
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"

#include "test_u_common.h"
#include "test_u_defines.h"
#include "test_u_gtk_common.h"

#include "test_u_audioeffect_defines.h"

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
  double        sampleTime;
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
   : deviceHandle (NULL)
   , release (false)
  {};

  struct _snd_pcm* deviceHandle; // (capture) device handle
  bool             release;
};
#endif

typedef Common_IDispatch_T<enum Stream_Module_StatisticAnalysis_Event> Test_U_AudioEffect_IDispatch_t;
struct Test_U_AudioEffect_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  inline Test_U_AudioEffect_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , area2D ()
   , area3D ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , asynchPlayback (false)
#endif
   , audioOutput (0)
   , device ()
   , dispatch (NULL)
   , fps (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , captureDeviceHandle (NULL)
   , effect ()
   , effectOptions ()
   , format (NULL)
   , manageSoX (false)
   , playbackDeviceHandle (NULL)
#endif
   , GdkWindow2D (NULL)
   , mute (false)
#if GTK_CHECK_VERSION (3,0,0)
   , cairoSurfaceLock (NULL)
   , cairoSurface2D (NULL)
#else
   , pixelBufferLock (NULL)
   , pixelBuffer2D (NULL)
#endif
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
   , OpenGLContext (NULL)
#else
   , OpenGLContext (NULL)
   , GdkWindow3D (NULL)
#endif
#else
   , OpenGLContext (NULL)
   , GdkWindow3D (NULL)
#endif
   , OpenGLTextureID (0)
#endif
   , spectrumAnalyzer2DMode (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzer3DMode (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_3DMODE)
   , spectrumAnalyzerResolution (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
   , sinus (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS)
   , sinusFrequency (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS_FREQUENCY)
   , targetFileName ()
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    device = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME);
#endif
  };

  GdkRectangle                    area2D;
  GdkRectangle                    area3D;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: current capturing is asynchronous (SIGIO), so asynchronous playback
  //         is not possible (playback eventually hogs all threads and starves)
  bool                            asynchPlayback;
#endif
  int                             audioOutput;
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : (ALSA/OSS/...) device file (e.g. "/dev/snd/pcmC0D0c", "/dev/dsp" (Linux))
  std::string                     device;
  Test_U_AudioEffect_IDispatch_t* dispatch;
  unsigned int                    fps;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct _snd_pcm*                        captureDeviceHandle;
  std::string                             effect;
  std::vector<std::string>                effectOptions;
  struct Stream_Module_Device_ALSAConfiguration* format;
  bool                                    manageSoX;
  struct _snd_pcm*                        playbackDeviceHandle;
#endif
  GdkWindow*                      GdkWindow2D;
  bool                            mute;
#if GTK_CHECK_VERSION (3,0,0)
  ACE_SYNCH_MUTEX*                cairoSurfaceLock;
  cairo_surface_t*                cairoSurface2D;
#else
  ACE_SYNCH_MUTEX* pixelBufferLock;
  GdkPixbuf*       pixelBuffer2D;
#endif
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GdkGLContext*                   OpenGLContext;
#else
  GglaContext*                    OpenGLContext;
  GdkWindow*                      GdkWindow3D;
#endif
#else
  GdkGLContext*                   OpenGLContext;
#if defined (GTKGLAREA_SUPPORT)
  GdkWindow*                      GdkWindow3D;
#else
  GdkGLDrawable*                  GdkWindow3D;
#endif
#endif
  GLuint                          OpenGLTextureID;
#endif
  enum Stream_Module_Visualization_SpectrumAnalyzer2DMode spectrumAnalyzer2DMode;
  enum Stream_Module_Visualization_SpectrumAnalyzer3DMode spectrumAnalyzer3DMode;
  unsigned int                                            spectrumAnalyzerResolution;
  bool                            sinus;
  double                          sinusFrequency;
  std::string                     targetFileName;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  inline Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , builder (NULL)
   , effect (GUID_NULL)
   , effectOptions ()
   , format (NULL)
  {};

  IGraphBuilder*       builder;
  CLSID                effect;
  union Stream_Decoder_DirectShow_AudioEffectOptions effectOptions;
  struct _AMMediaType* format;
};
struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  inline Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , effect (GUID_NULL)
   , effectOptions ()
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

  CLSID            effect;
  std::string      effectOptions;
  IMFMediaType*    format;
  TOPOID           sampleGrabberNodeId;
  IMFMediaSession* session;
};
#endif

struct Test_U_AudioEffect_RuntimeStatistic
 : Test_U_RuntimeStatistic_t
{
  inline Test_U_AudioEffect_RuntimeStatistic ()
   : Test_U_RuntimeStatistic_t ()
   , amplitudeAverage (0.0)
   , amplitudeVariance (0.0)
   , streakAverage (0.0)
   , streakCount (0)
   , streakVariance (0.0)
   , volumeAverage (0.0)
   , volumeVariance (0.0)
  {};

  double       amplitudeAverage;
  double       amplitudeVariance;
  double       streakAverage;
  unsigned int streakCount;
  double       streakVariance;
  double       volumeAverage;
  double       volumeVariance;
};
struct Test_U_AudioEffect_SessionData
 : Test_U_SessionData
{
  inline Test_U_AudioEffect_SessionData ()
   : Test_U_SessionData ()
   , currentStatistic ()
 #if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//   , deviceHandle (NULL)
   , format ()
#endif
  {};

  struct Test_U_AudioEffect_RuntimeStatistic    currentStatistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//  struct _snd_pcm*           deviceHandle;
  struct Stream_Module_Device_ALSAConfiguration format;
#endif
};
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

  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  inline Test_U_AudioEffect_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};
#else
struct Test_U_AudioEffect_StreamConfiguration
 : Stream_Configuration
{
  inline Test_U_AudioEffect_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  struct Test_U_AudioEffect_ModuleHandlerConfiguration* moduleHandlerConfiguration;
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , ALSAConfiguration ()
#endif
   , moduleHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , streamConfiguration ()
#endif
   , signalHandlerConfiguration ()
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_Module_Device_ALSAConfiguration        ALSAConfiguration;
#endif
  struct Test_U_AudioEffect_ModuleHandlerConfiguration moduleHandlerConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Test_U_AudioEffect_StreamConfiguration        streamConfiguration;
#endif
  struct Test_U_AudioEffect_SignalHandlerConfiguration signalHandlerConfiguration;
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

  struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_U_AudioEffect_DirectShow_StreamConfiguration        streamConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_Configuration
 : Test_U_AudioEffect_Configuration
{
  inline Test_U_AudioEffect_MediaFoundation_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
  {};

  struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration moduleHandlerConfiguration;
  struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration        streamConfiguration;
};
#endif

typedef Stream_INotify_T<Stream_SessionMessageType> Test_U_AudioEffect_IStreamNotify_t;
typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_State> Test_U_AudioEffect_IStreamControl_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_U_AudioEffect_DirectShow_Message,
                                Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<struct Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_DirectShow_ControlMessage_t,
                                          Test_U_AudioEffect_DirectShow_Message,
                                          Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_AudioEffect_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_DirectShow_Message,
                                    Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_DirectShow_ISessionNotify_t*> Test_U_AudioEffect_DirectShow_Subscribers_t;
typedef Test_U_AudioEffect_DirectShow_Subscribers_t::iterator Test_U_AudioEffect_DirectShow_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_U_AudioEffect_DirectShow_ISessionNotify_t> Test_U_AudioEffect_DirectShow_ISubscribe_t;

typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_U_AudioEffect_MediaFoundation_Message,
                                Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<struct Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_MediaFoundation_ControlMessage_t,
                                          Test_U_AudioEffect_MediaFoundation_Message,
                                          Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_AudioEffect_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_MediaFoundation_Message,
                                    Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*> Test_U_AudioEffect_MediaFoundation_Subscribers_t;
typedef Test_U_AudioEffect_MediaFoundation_Subscribers_t::iterator Test_U_AudioEffect_MediaFoundation_SubscribersIterator_t;
typedef Common_ISubscribe_T<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t> Test_U_AudioEffect_MediaFoundation_ISubscribe_t;
#else
typedef Stream_ControlMessage_T<enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration,
                                Test_U_AudioEffect_Message,
                                Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_ControlMessage_t,
                                          Test_U_AudioEffect_Message,
                                          Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_AudioEffect_SessionData,
                                    enum Stream_SessionMessageType,
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
   , statistic ()
  {};

  Test_U_AudioEffect_CompletedActions_t      completedActions;
//  GdkCursorType                      cursorType;
  struct Common_UI_GTKState*                 GTKState;
  Test_U_AudioEffect_PendingActions_t        pendingActions;
  struct Test_U_AudioEffect_RuntimeStatistic statistic;
};

#if GTK_CHECK_VERSION (3,10,0)
typedef Common_ISetP_T<cairo_surface_t> Test_U_Common_ISet_t;
#else
typedef Common_ISetP_T<GdkPixbuf> Test_U_Common_ISet_t;
#endif
struct Test_U_AudioEffect_GTK_CBDataBase
 : Test_U_GTK_CBData
{
  inline Test_U_AudioEffect_GTK_CBDataBase ()
   : Test_U_GTK_CBData ()
   , area2D ()
   , area3D ()
#if GTK_CHECK_VERSION (3,10,0)
   , cairoSurfaceLock ()
   , cairoSurface2D (NULL)
#else
   , pixelBufferLock ()
   , pixelBuffer2D (NULL)
#endif
   , isFirst (true)
   , progressData ()
   , progressEventSourceID (0)
   , resizeNotification (NULL)
   , subscribersLock ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
#endif
  {};

  GdkRectangle                               area2D;
  GdkRectangle                               area3D;
#if GTK_CHECK_VERSION (3,10,0)
  ACE_SYNCH_MUTEX                            cairoSurfaceLock;
  cairo_surface_t*                           cairoSurface2D;
#else
  ACE_SYNCH_MUTEX                            pixelBufferLock;
  GdkPixbuf*                                 pixelBuffer2D;
#endif
  bool                                       isFirst; // first activation ?
  struct Test_U_AudioEffect_GTK_ProgressData progressData;
  guint                                      progressEventSourceID;
  Test_U_Common_ISet_t*                      resizeNotification;
  ACE_SYNCH_RECURSIVE_MUTEX                  subscribersLock;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                                       useMediaFoundation;
#endif
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_GTK_CBData
 : Test_U_AudioEffect_GTK_CBDataBase
{
  inline Test_U_AudioEffect_DirectShow_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBDataBase ()
   , configuration (NULL)
   , stream (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {};

  struct Test_U_AudioEffect_DirectShow_Configuration* configuration;
  Test_U_AudioEffect_DirectShow_Stream*               stream;
  IAMStreamConfig*                                    streamConfiguration;
  Test_U_AudioEffect_DirectShow_Subscribers_t         subscribers;
};
struct Test_U_AudioEffect_MediaFoundation_GTK_CBData
 : Test_U_AudioEffect_GTK_CBDataWin32Base
{
  inline Test_U_AudioEffect_MediaFoundation_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBDataWin32Base ()
   , configuration (NULL)
   , stream (NULL)
   , subscribers ()
  {};

  struct Test_U_AudioEffect_MediaFoundation_Configuration* configuration;
  Test_U_AudioEffect_MediaFoundation_Stream*               stream;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t         subscribers;
};
#else
struct Test_U_AudioEffect_GTK_CBData
 : Test_U_AudioEffect_GTK_CBDataBase
{
  inline Test_U_AudioEffect_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBDataBase ()
   , configuration (NULL)
   , device (NULL)
   , stream (NULL)
   , subscribers ()
  {};

  struct Test_U_AudioEffect_Configuration* configuration;
  struct _snd_pcm*                         device; // (capture) device handle
  Test_U_AudioEffect_Stream*               stream;
  Test_U_AudioEffect_Subscribers_t         subscribers;
};
#endif

struct Test_U_AudioEffect_ThreadData
{
  inline Test_U_AudioEffect_ThreadData ()
   : eventSourceID (0)
   , sessionID (0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (TEST_U_STREAM_WIN32_FRAMEWORK_DEFAULT_USE_MEDIAFOUNDATION)
#else
   , CBData (NULL)
#endif
  {};

  guint                                 eventSourceID;
  size_t                                sessionID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                                  useMediaFoundation;
#else
  struct Test_U_AudioEffect_GTK_CBData* CBData;
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

  struct Test_U_AudioEffect_DirectShow_GTK_CBData* CBData;
};
struct Test_U_AudioEffect_MediaFoundation_ThreadData
 : Test_U_AudioEffect_ThreadData
{
  inline Test_U_AudioEffect_MediaFoundation_ThreadData ()
   : Test_U_AudioEffect_ThreadData ()
   , CBData (NULL)
  {
    useMediaFoundation = true;
  };

  struct Test_U_AudioEffect_MediaFoundation_GTK_CBData* CBData;
};
#endif

#endif
