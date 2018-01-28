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

#include <deque>
#include <list>
#include <map>
#include <string>

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <strmif.h>
#include <mfapi.h>
#include <mfidl.h>
#include <dsound.h>
#else
#include "alsa/asoundlib.h"
#endif

#if defined (GTKGL_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <gl/GL.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "gtk/gtk.h"
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#include "gtkgl/gdkgl.h"
#endif
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#else
#include "gtk/gtkgl.h" // gtkglext
#endif
#endif
#endif

#include "common_isubscribe.h"
#include "common_statistic_handler.h"
#include "common_tools.h"

#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_gl_common.h"
#include "common_ui_gtk_manager.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_common.h"
#endif

#include "stream_dev_common.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"

#include "stream_vis_common.h"
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
  Test_U_AudioEffect_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMediaSample* sample;
  double        sampleTime;
};
struct Test_U_AudioEffect_MediaFoundation_MessageData
{
  Test_U_AudioEffect_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {};

  IMFSample* sample;
  LONGLONG   sampleTime;
};
#else
struct Test_U_AudioEffect_MessageData
{
  Test_U_AudioEffect_MessageData ()
   : deviceHandle (NULL)
   , release (false)
  {};

  struct _snd_pcm* deviceHandle; // (capture) device handle
  bool             release;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_AudioEffect_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_DirectShow_Message,
                                    Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_DirectShow_ISessionNotify_t*> Test_U_AudioEffect_DirectShow_Subscribers_t;
typedef Test_U_AudioEffect_DirectShow_Subscribers_t::iterator Test_U_AudioEffect_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_AudioEffect_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_MediaFoundation_Message,
                                    Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*> Test_U_AudioEffect_MediaFoundation_Subscribers_t;
typedef Test_U_AudioEffect_MediaFoundation_Subscribers_t::iterator Test_U_AudioEffect_MediaFoundation_SubscribersIterator_t;
#else
typedef Stream_ISessionDataNotify_T<Stream_SessionId_t,
                                    struct Test_U_AudioEffect_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_Message,
                                    Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_ISessionNotify_t*> Test_U_AudioEffect_Subscribers_t;
typedef Test_U_AudioEffect_Subscribers_t::iterator Test_U_AudioEffect_SubscribersIterator_t;
#endif
typedef Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType> Test_U_AudioEffect_IDispatch_t;
struct Test_U_AudioEffect_StreamConfiguration;
struct Test_U_AudioEffect_ModuleHandlerConfiguration;
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Test_U_AudioEffect_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_AudioEffect_ModuleHandlerConfiguration> Test_U_AudioEffect_StreamConfiguration_t;
struct Test_U_AudioEffect_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , area2D ()
   , area3D ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , asynchPlayback (false)
#endif
   , audioOutput (0)
   , deviceName ()
   , dispatch (NULL)
   , fps (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , captureDeviceHandle (NULL)
   , effect ()
   , effectOptions ()
   , format (NULL)
   , frameRate ()
   , manageSoX (false)
   , playbackDeviceHandle (NULL)
#endif
   , GdkWindow2D (NULL)
   , mute (false)
   , surfaceLock (NULL)
#if GTK_CHECK_VERSION (3,0,0)
   , cairoSurface2D (NULL)
#else
   , pixelBuffer2D (NULL)
#endif /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions (NULL)
   , OpenGLInstructionsLock (NULL)
   , OpenGLTextureId (0)
   , OpenGLWindow (NULL)
#if GTK_CHECK_VERSION (3,0,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#else
   , OpenGLContext (NULL)
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */
   , spectrumAnalyzer2DMode (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzer3DMode (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_3DMODE)
   , spectrumAnalyzerResolution (MODULE_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
   , sinus (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS)
   , sinusFrequency (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS_FREQUENCY)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , streamConfiguration (NULL)
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , subscriber (NULL)
   , subscribers (NULL)
#endif
   , targetFileName ()
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    deviceName = ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME);
#endif
  };

  GdkRectangle                                            area2D;
  GdkRectangle                                            area3D;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // *NOTE*: current capturing is asynchronous (SIGIO), so asynchronous playback
  //         is not possible (playback eventually hogs all threads and starves)
  bool                                                    asynchPlayback;
#endif
  int                                                     audioOutput;
  // *PORTABILITY*: Win32: "FriendlyName" property
  //                UNIX : (ALSA/OSS/...) device file path (e.g. "/dev/snd/pcmC0D0c", "/dev/dsp" (Linux))
  std::string                                             deviceName;
  Test_U_AudioEffect_IDispatch_t*                         dispatch;
  unsigned int                                            fps;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct _snd_pcm*                                        captureDeviceHandle;
  std::string                                             effect;
  std::vector<std::string>                                effectOptions;
  struct Stream_Module_Device_ALSAConfiguration*          format;
  struct AVRational                                       frameRate; // *TODO*: remove ASAP !
  bool                                                    manageSoX;
  struct _snd_pcm*                                        playbackDeviceHandle;
#endif
  GdkWindow*                                              GdkWindow2D;
  bool                                                    mute;
  ACE_SYNCH_MUTEX*                                        surfaceLock;
#if GTK_CHECK_VERSION (3,0,0)
  cairo_surface_t*                                        cairoSurface2D;
#else
  GdkPixbuf*                                              pixelBuffer2D;
#endif /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGL_SUPPORT)
  Stream_Module_Visualization_OpenGLInstructions_t*       OpenGLInstructions;
  ACE_SYNCH_MUTEX*                                        OpenGLInstructionsLock;
  GLuint                                                  OpenGLTextureId;
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea*                                              OpenGLWindow;
#else
//  GglaContext*                                      OpenGLContext;
  GglaArea*                                               OpenGLWindow;
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  GtkGLArea*                                              OpenGLWindow;
#else
  GdkGLContext*                                           OpenGLContext;
  GdkWindow*                                              OpenGLWindow;
//  GdkGLDrawable*                                    OpenGLWindow;
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */
  enum Stream_Module_Visualization_SpectrumAnalyzer2DMode spectrumAnalyzer2DMode;
  enum Stream_Module_Visualization_SpectrumAnalyzer3DMode spectrumAnalyzer3DMode;
  unsigned int                                            spectrumAnalyzerResolution;
  bool                                                    sinus;
  double                                                  sinusFrequency;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Test_U_AudioEffect_StreamConfiguration_t*               streamConfiguration;
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Test_U_AudioEffect_ISessionNotify_t*                    subscriber;
  Test_U_AudioEffect_Subscribers_t*                       subscribers;
#endif
  std::string                                             targetFileName;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//extern const char stream_name_string_[];
struct Test_U_AudioEffect_DirectShow_StreamConfiguration;
struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Test_U_AudioEffect_DirectShow_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration> Test_U_AudioEffect_DirectShow_StreamConfiguration_t;
struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , builder (NULL)
   , effect (GUID_NULL)
   , effectOptions ()
   , inputFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {};

  IGraphBuilder*                                       builder;
  CLSID                                                effect;
  union Stream_Decoder_DirectShow_AudioEffectOptions   effectOptions;
  struct _AMMediaType*                                 inputFormat;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t* streamConfiguration;
  Test_U_AudioEffect_DirectShow_ISessionNotify_t*      subscriber;
  Test_U_AudioEffect_DirectShow_Subscribers_t*         subscribers;
};

//extern const char stream_name_string_[];
struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration;
struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration> Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t;
struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , effect (GUID_NULL)
   , effectOptions ()
   , inputFormat (NULL)
   , sampleGrabberNodeId (0)
   , session (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    HRESULT result = MFCreateMediaType (&inputFormat);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
  };

  CLSID                                                     effect;
  std::string                                               effectOptions;
  IMFMediaType*                                             inputFormat;
  TOPOID                                                    sampleGrabberNodeId;
  IMFMediaSession*                                          session;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t* streamConfiguration;
  Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*      subscriber;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t*         subscribers;
};
#endif

struct Test_U_AudioEffect_Statistic
 : Test_U_Statistic_t
{
  Test_U_AudioEffect_Statistic ()
   : Test_U_Statistic_t ()
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
typedef Common_StatisticHandler_T<struct Test_U_AudioEffect_Statistic> Test_U_AudioEffect_StatisticHandler_t;

struct Test_U_AudioEffect_SessionData
 : Test_U_SessionData
{
  Test_U_AudioEffect_SessionData ()
   : Test_U_SessionData ()
   , statistic ()
 #if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , format ()
   , height (0)
   , width (0)
#endif
  {};

  struct Test_U_AudioEffect_Statistic           statistic;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_Module_Device_ALSAConfiguration format;
  unsigned int                                  height; // *TODO*: remove ASAP !
  unsigned int                                  width; // *TODO*: remove ASAP !
#endif
};
typedef Stream_SessionData_T<struct Test_U_AudioEffect_SessionData> Test_U_AudioEffect_SessionData_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_SessionData
 : Test_U_AudioEffect_SessionData
{
  Test_U_AudioEffect_DirectShow_SessionData ()
   : Test_U_AudioEffect_SessionData ()
   , builder (NULL)
   , inputFormat (NULL)
  {};

  IGraphBuilder*       builder;
  struct _AMMediaType* inputFormat;
};
typedef Stream_SessionData_T<struct Test_U_AudioEffect_DirectShow_SessionData> Test_U_AudioEffect_DirectShow_SessionData_t;
struct Test_U_AudioEffect_MediaFoundation_SessionData
 : Test_U_AudioEffect_SessionData
{
  Test_U_AudioEffect_MediaFoundation_SessionData ()
   : Test_U_AudioEffect_SessionData ()
   , inputFormat (NULL)
   , rendererNodeId (0)
   , session (NULL)
  {};

  IMFMediaType*    inputFormat;
  TOPOID           rendererNodeId;
  IMFMediaSession* session;
};
typedef Stream_SessionData_T<struct Test_U_AudioEffect_MediaFoundation_SessionData> Test_U_AudioEffect_MediaFoundation_SessionData_t;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_DirectShow_StreamConfiguration ()
   : Stream_Configuration ()
   , filterGraphConfiguration ()
  {};

  Stream_Module_Device_DirectShow_Graph_t filterGraphConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
  {};
};
#else
struct Test_U_AudioEffect_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_StreamConfiguration ()
   : Stream_Configuration ()
  {};
};
#endif

struct Test_U_AudioEffect_SignalHandlerConfiguration
 : Test_U_SignalHandlerConfiguration
{
  Test_U_AudioEffect_SignalHandlerConfiguration ()
   : Test_U_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {};

  long               actionTimerId;
  Stream_IAllocator* messageAllocator;
  unsigned int       statisticReportingInterval; // statistic collecting interval (second(s)) [0: off]
};

struct Test_U_AudioEffect_Configuration
 : Test_U_Configuration
{
  Test_U_AudioEffect_Configuration ()
   : Test_U_Configuration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , ALSAConfiguration ()
#endif
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Test_U_AudioEffect_StreamConfiguration_t             streamConfiguration;
#endif
  struct Test_U_AudioEffect_SignalHandlerConfiguration signalHandlerConfiguration;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_Configuration
 : Test_U_AudioEffect_Configuration
{
  Test_U_AudioEffect_DirectShow_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , streamConfiguration ()
  {};

  Test_U_AudioEffect_DirectShow_StreamConfiguration_t streamConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_Configuration
 : Test_U_AudioEffect_Configuration
{
  Test_U_AudioEffect_MediaFoundation_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , streamConfiguration ()
  {};

  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t streamConfiguration;
};
#endif

typedef Stream_INotify_T<enum Stream_SessionMessageType> Test_U_AudioEffect_IStreamNotify_t;
typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Stream_State> Test_U_AudioEffect_IStreamControl_t;

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Stream_AllocatorConfiguration> Test_U_AudioEffect_ControlMessage_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_ControlMessage_t,
                                          Test_U_AudioEffect_DirectShow_Message,
                                          Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_DirectShow_ISessionNotify_t> Test_U_AudioEffect_DirectShow_ISubscribe_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_ControlMessage_t,
                                          Test_U_AudioEffect_MediaFoundation_Message,
                                          Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t> Test_U_AudioEffect_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_AudioEffect_ControlMessage_t,
                                          Test_U_AudioEffect_Message,
                                          Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_MessageAllocator_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_ISessionNotify_t> Test_U_AudioEffect_ISubscribe_t;
#endif

//////////////////////////////////////////

struct Test_U_AudioEffect_GTK_ProgressData
 : Test_U_GTK_ProgressData
{
  Test_U_AudioEffect_GTK_ProgressData ()
   : Test_U_GTK_ProgressData ()
   , statistic ()
  {};

  struct Test_U_AudioEffect_Statistic statistic;
};

#if GTK_CHECK_VERSION (3,10,0)
typedef Common_ISetP_T<cairo_surface_t> Test_U_Common_ISet_t;
#else
typedef Common_ISetP_T<GdkPixbuf> Test_U_Common_ISet_t;
#endif
struct Test_U_AudioEffect_GTK_CBDataBase
 : Test_U_GTK_CBData
{
  Test_U_AudioEffect_GTK_CBDataBase ()
   : Test_U_GTK_CBData ()
   , area2D ()
#if defined (GTKGL_SUPPORT)
   , area3D ()
#endif
   , surfaceLock ()
#if GTK_CHECK_VERSION (3,10,0)
   , cairoSurface2D (NULL)
#else
   , pixelBuffer2D (NULL)
#endif
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions ()
#endif
   , isFirst (true)
   , progressData ()
   , progressEventSourceId (0)
   , resizeNotification (NULL)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
#endif
  {};

  GdkRectangle                                     area2D;
#if defined (GTKGL_SUPPORT)
  GdkRectangle                                     area3D;
#endif
  ACE_SYNCH_MUTEX                                  surfaceLock;
#if GTK_CHECK_VERSION (3,10,0)
  cairo_surface_t*                                 cairoSurface2D;
#else
  GdkPixbuf*                                       pixelBuffer2D;
#endif
#if defined (GTKGL_SUPPORT)
  Stream_Module_Visualization_OpenGLInstructions_t OpenGLInstructions;
#endif
  bool                                             isFirst; // first activation ?
  struct Test_U_AudioEffect_GTK_ProgressData       progressData;
  guint                                            progressEventSourceId;
  Test_U_Common_ISet_t*                            resizeNotification;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                                             useMediaFoundation;
#endif
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_GTK_CBData
 : Test_U_AudioEffect_GTK_CBDataBase
{
  Test_U_AudioEffect_DirectShow_GTK_CBData ()
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
 : Test_U_AudioEffect_GTK_CBDataBase
{
  Test_U_AudioEffect_MediaFoundation_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBDataBase ()
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
  Test_U_AudioEffect_GTK_CBData ()
   : Test_U_AudioEffect_GTK_CBDataBase ()
   , configuration (NULL)
   , handle (NULL)
   , stream (NULL)
   , subscribers ()
  {};

  struct Test_U_AudioEffect_Configuration* configuration;
  struct _snd_pcm*                         handle; // (capture) device handle
  Test_U_AudioEffect_Stream*               stream;
  Test_U_AudioEffect_Subscribers_t         subscribers;
};
#endif

struct Test_U_AudioEffect_ThreadData
{
  Test_U_AudioEffect_ThreadData ()
   : CBData (NULL)
   , eventSourceId (0)
   , sessionId (0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , useMediaFoundation (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)
#endif
  {};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_GTK_CBDataBase* CBData;
#else
  struct Test_U_AudioEffect_GTK_CBData*     CBData;
#endif
  guint                                     eventSourceId;
  size_t                                    sessionId;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  bool                                      useMediaFoundation;
#endif
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_ThreadData
 : Test_U_AudioEffect_ThreadData
{
  Test_U_AudioEffect_DirectShow_ThreadData ()
   : Test_U_AudioEffect_ThreadData ()
   , CBData (NULL)
  {};

  struct Test_U_AudioEffect_DirectShow_GTK_CBData* CBData;
};
struct Test_U_AudioEffect_MediaFoundation_ThreadData
 : Test_U_AudioEffect_ThreadData
{
  Test_U_AudioEffect_MediaFoundation_ThreadData ()
   : Test_U_AudioEffect_ThreadData ()
   , CBData (NULL)
  {
    useMediaFoundation = true;
  };

  struct Test_U_AudioEffect_MediaFoundation_GTK_CBData* CBData;
};
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_UI_GtkBuilderDefinition_T<struct Test_U_AudioEffect_DirectShow_GTK_CBData> Test_U_AudioEffect_DirectShow_GtkBuilderDefinition_t;
typedef Common_UI_GtkBuilderDefinition_T<struct Test_U_AudioEffect_MediaFoundation_GTK_CBData> Test_U_AudioEffect_MediaFoundation_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_U_AudioEffect_DirectShow_GTK_CBData> Test_U_AudioEffect_DirectShow_GTK_Manager_t;
typedef Common_UI_GTK_Manager_T<struct Test_U_AudioEffect_MediaFoundation_GTK_CBData> Test_U_AudioEffect_MediaFoundation_GTK_Manager_t;
typedef ACE_Singleton<Test_U_AudioEffect_DirectShow_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> AUDIOEFFECT_UI_DIRECTSHOW_GTK_MANAGER_SINGLETON;
typedef ACE_Singleton<Test_U_AudioEffect_MediaFoundation_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> AUDIOEFFECT_UI_MEDIAFOUNDATION_GTK_MANAGER_SINGLETON;
#else
typedef Common_UI_GtkBuilderDefinition_T<struct Test_U_AudioEffect_GTK_CBData> Test_U_AudioEffect_GtkBuilderDefinition_t;

typedef Common_UI_GTK_Manager_T<struct Test_U_AudioEffect_GTK_CBData> Test_U_AudioEffect_GTK_Manager_t;
typedef ACE_Singleton<Test_U_AudioEffect_GTK_Manager_t,
                      typename ACE_MT_SYNCH::RECURSIVE_MUTEX> AUDIOEFFECT_UI_GTK_MANAGER_SINGLETON;
#endif

#endif
