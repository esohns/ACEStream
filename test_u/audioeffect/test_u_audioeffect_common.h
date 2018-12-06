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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfapi.h>
#include <mfidl.h>
#include <strmif.h>
#else
#include "alsa/asoundlib.h"

#if defined (__cplusplus)
extern "C"
{
#include "libavutil/rational.h"
}
#endif /* __cplusplus */
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTKGL_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <gl/GL.h>
#else
#include <GL/gl.h>
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTKGL_SUPPORT
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#else
#include "gtk/gtkgl.h" // gtkglext
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_inotify.h"
#include "common_statistic_handler.h"
#include "common_tools.h"

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_gl_common.h"
#include "common_ui_gtk_manager.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_istreamcontrol.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dec_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_common.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_stat_common.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "test_u_common.h"
#include "test_u_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "test_u_gtk_common.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

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
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_MessageData
{
  Test_U_AudioEffect_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMediaSample* sample;
  double        sampleTime;
};
struct Test_U_AudioEffect_MediaFoundation_MessageData
{
  Test_U_AudioEffect_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
  {}

  IMFSample* sample;
  LONGLONG   sampleTime;
};
#else
struct Test_U_AudioEffect_MessageData
{
  Test_U_AudioEffect_MessageData ()
   : deviceHandle (NULL)
   , release (false)
  {}

  struct _snd_pcm* deviceHandle; // (capture) device handle
  bool             release;
};
#endif // ACE_WIN32 || ACE_WIN64

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
#endif // ACE_WIN32 || ACE_WIN64
typedef Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType> Test_U_AudioEffect_IDispatch_t;
struct Test_U_AudioEffect_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , area2D ()
#if defined (GTKGL_SUPPORT)
   , area3D ()
#endif /* GTKGL_SUPPORT */
#endif // GTK_USE
#endif // GUI_SUPPORT
   , audioOutput (0)
   , deviceIdentifier ()
   , dispatch (NULL)
   , fps (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , GdkWindow2D (NULL)
#endif // GTK_USE
#endif // GUI_SUPPORT
   , mute (false)
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   , surfaceLock (NULL)
#if GTK_CHECK_VERSION(3,11,0)
   , cairoSurface2D (NULL)
#else
   , pixelBuffer2D (NULL)
#endif /* GTK_CHECK_VERSION(3,11,0) */
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions (NULL)
   , OpenGLInstructionsLock (NULL)
   , OpenGLTextureId (0)
#endif /* GTKGL_SUPPORT */
#endif // GTK_USE
   , spectrumAnalyzer2DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzer3DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_3DMODE)
#endif // GUI_SUPPORT
   , spectrumAnalyzerResolution (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
   , sinus (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS)
   , sinusFrequency (TEST_U_STREAM_AUDIOEFFECT_DEFAULT_SINUS_FREQUENCY)
   , targetFileName ()
  {}

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkRectangle                                      area2D;
#if defined (GTKGL_SUPPORT)
  GdkRectangle                                      area3D;
#endif /* GTKGL_SUPPORT */
#endif // GTK_USE
#endif // GUI_SUPPORT
  int                                               audioOutput;
  // *PORTABILITY*: Win32: (usb) device path
  //                UNIX : (ALSA/OSS/...) device file path (e.g. "/dev/snd/pcmC0D0c", "/dev/dsp" (Linux))
  std::string                                       deviceIdentifier;
  Test_U_AudioEffect_IDispatch_t*                   dispatch;
  unsigned int                                      fps;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  GdkWindow*                                        GdkWindow2D;
#endif // GTK_USE
#endif // GUI_SUPPORT
  bool                                              mute;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  ACE_SYNCH_MUTEX*                                  surfaceLock;
#if GTK_CHECK_VERSION(3,11,0)
  cairo_surface_t*                                  cairoSurface2D;
#else
  GdkPixbuf*                                        pixelBuffer2D;
#endif /* GTK_CHECK_VERSION(3,11,0) */
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_OpenGL_Instructions_t*       OpenGLInstructions;
  ACE_SYNCH_MUTEX*                                  OpenGLInstructionsLock;
  GLuint                                            OpenGLTextureId;
#endif /* GTKGL_SUPPORT */
#endif // GTK_USE
  enum Stream_Visualization_SpectrumAnalyzer_2DMode spectrumAnalyzer2DMode;
  enum Stream_Visualization_SpectrumAnalyzer_3DMode spectrumAnalyzer3DMode;
#endif // GUI_SUPPORT
  unsigned int                                      spectrumAnalyzerResolution;
  bool                                              sinus;
  double                                            sinusFrequency;
  std::string                                       targetFileName;
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
  {}

  IGraphBuilder*                                            builder;
  CLSID                                                     effect;
  union Stream_MediaFramework_DirectShow_AudioEffectOptions effectOptions;
  struct _AMMediaType*                                      inputFormat;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t*      streamConfiguration;
  Test_U_AudioEffect_DirectShow_ISessionNotify_t*           subscriber;
  Test_U_AudioEffect_DirectShow_Subscribers_t*              subscribers;
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  }

  CLSID                                                     effect;
  std::string                                               effectOptions;
  IMFMediaType*                                             inputFormat;
  TOPOID                                                    sampleGrabberNodeId;
  IMFMediaSession*                                          session;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t* streamConfiguration;
  Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*      subscriber;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t*         subscribers;
};
#else
struct Test_U_AudioEffect_ALSA_StreamConfiguration;
struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration;
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_AllocatorConfiguration,
                               struct Test_U_AudioEffect_ALSA_StreamConfiguration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration> Test_U_AudioEffect_ALSA_StreamConfiguration_t;
struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , asynchPlayback (false)
   , captureDeviceHandle (NULL)
   , effect ()
   , effectOptions ()
   , manageSoX (false)
   , playbackDeviceHandle (NULL)
   , sourceFormat ()
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    deviceIdentifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_ALSA_DEFAULT_DEVICE_NAME);
  }

  // *NOTE*: current capturing is asynchronous (SIGIO), so asynchronous playback
  //         is not possible (playback eventually hogs all threads and starves)
  bool                                           asynchPlayback;
  struct _snd_pcm*                               captureDeviceHandle;
  std::string                                    effect;
  std::vector<std::string>                       effectOptions;
  bool                                           manageSoX;
  struct _snd_pcm*                               playbackDeviceHandle;
  struct Stream_MediaFramework_ALSA_MediaType    sourceFormat;
  Test_U_AudioEffect_ALSA_StreamConfiguration_t* streamConfiguration;
  Test_U_AudioEffect_ISessionNotify_t*           subscriber;
  Test_U_AudioEffect_Subscribers_t*              subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

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
  {}

  double       amplitudeAverage;
  double       amplitudeVariance;
  double       streakAverage;
  unsigned int streakCount;
  double       streakVariance;
  double       volumeAverage;
  double       volumeVariance;
};
typedef Common_StatisticHandler_T<struct Test_U_AudioEffect_Statistic> Test_U_AudioEffect_StatisticHandler_t;

class Test_U_AudioEffect_SessionData
 : public Test_U_ALSA_SessionData
{
 public:
  Test_U_AudioEffect_SessionData ()
   : Test_U_ALSA_SessionData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , resolution ()
#endif // ACE_WIN32 || ACE_WIN64
   , statistic ()
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Common_UI_Resolution_t              resolution; // *TODO*: remove ASAP !
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_U_AudioEffect_Statistic statistic;
};
typedef Stream_SessionData_T<Test_U_AudioEffect_SessionData> Test_U_AudioEffect_SessionData_t;

struct Test_U_AudioEffect_StreamState
 : Stream_State
{
  Test_U_AudioEffect_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct Test_U_AudioEffect_SessionData* sessionData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_SessionData
 : Test_U_AudioEffect_SessionData
{
  Test_U_AudioEffect_DirectShow_SessionData ()
   : Test_U_AudioEffect_SessionData ()
   , builder (NULL)
   , inputFormat (NULL)
  {}

  IGraphBuilder*       builder;
  struct _AMMediaType* inputFormat;
};
typedef Stream_SessionData_T<struct Test_U_AudioEffect_DirectShow_SessionData> Test_U_AudioEffect_DirectShow_SessionData_t;

struct Test_U_AudioEffect_DirectShow_StreamState
 : Stream_State
{
  Test_U_AudioEffect_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct Test_U_AudioEffect_DirectShow_SessionData* sessionData;
};

struct Test_U_AudioEffect_MediaFoundation_SessionData
 : Test_U_AudioEffect_SessionData
{
  Test_U_AudioEffect_MediaFoundation_SessionData ()
   : Test_U_AudioEffect_SessionData ()
   , inputFormat (NULL)
   , rendererNodeId (0)
   , session (NULL)
  {}

  IMFMediaType*    inputFormat;
  TOPOID           rendererNodeId;
  IMFMediaSession* session;
};
typedef Stream_SessionData_T<struct Test_U_AudioEffect_MediaFoundation_SessionData> Test_U_AudioEffect_MediaFoundation_SessionData_t;

struct Test_U_AudioEffect_MediaFoundation_StreamState
 : Stream_State
{
  Test_U_AudioEffect_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct Test_U_AudioEffect_MediaFoundation_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_DirectShow_StreamConfiguration ()
   : Stream_Configuration ()
   , filterGraphConfiguration ()
  {}

  Stream_MediaFramework_DirectShow_Graph_t filterGraphConfiguration;
};

struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration ()
   : Stream_Configuration ()
  {}
};
#else
struct Test_U_AudioEffect_ALSA_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_ALSA_StreamConfiguration ()
   : Stream_Configuration ()
   , format (NULL)
  {}

  struct Stream_MediaFramework_ALSA_MediaType* format;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_AudioEffect_SignalHandlerConfiguration
 : Test_U_SignalHandlerConfiguration
{
  Test_U_AudioEffect_SignalHandlerConfiguration ()
   : Test_U_SignalHandlerConfiguration ()
   , actionTimerId (-1)
   , messageAllocator (NULL)
   , statisticReportingInterval (0)
  {}

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
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , streamConfiguration ()
#endif // ACE_WIN32 || ACE_WIN64
   , signalHandlerConfiguration ()
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_Device_ALSAConfiguration               ALSAConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Test_U_AudioEffect_ALSA_StreamConfiguration_t        streamConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_U_AudioEffect_SignalHandlerConfiguration signalHandlerConfiguration;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_Configuration
 : Test_U_AudioEffect_Configuration
{
  Test_U_AudioEffect_DirectShow_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , streamConfiguration ()
  {}

  Test_U_AudioEffect_DirectShow_StreamConfiguration_t streamConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_Configuration
 : Test_U_AudioEffect_Configuration
{
  Test_U_AudioEffect_MediaFoundation_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , streamConfiguration ()
  {}

  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_AudioEffect_DirectShow_Message,
                                          Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_MessageAllocator_t;

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_AudioEffect_DirectShow_StreamState> Test_U_AudioEffect_DirectShow_IStreamControl_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_DirectShow_ISessionNotify_t> Test_U_AudioEffect_DirectShow_ISubscribe_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_AudioEffect_MediaFoundation_Message,
                                          Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_MessageAllocator_t;

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_AudioEffect_MediaFoundation_StreamState> Test_U_AudioEffect_MediaFoundation_IStreamControl_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t> Test_U_AudioEffect_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_AudioEffect_Message,
                                          Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_MessageAllocator_t;

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_AudioEffect_StreamState> Test_U_AudioEffect_IStreamControl_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_ISessionNotify_t> Test_U_AudioEffect_ISubscribe_t;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (GUI_SUPPORT)
struct Test_U_AudioEffect_ProgressData
#if defined (GTK_USE)
 : Test_U_GTK_ProgressData
#else
 : Test_U_UI_ProgressData
#endif // GTK_USE
{
  Test_U_AudioEffect_ProgressData ()
#if defined (GTK_USE)
   : Test_U_GTK_ProgressData ()
#else
   : Test_U_UI_ProgressData ()
#endif // GTK_USE
   , statistic ()
  {}

  struct Test_U_AudioEffect_Statistic statistic;
};

#if defined (GTK_USE)
#if GTK_CHECK_VERSION(3,10,0)
typedef Common_ISetP_T<cairo_surface_t> Test_U_Common_ISet_t;
#else
typedef Common_ISetP_T<GdkPixbuf> Test_U_Common_ISet_t;
#endif // GTK_CHECK_VERSION(3,10,0)
#endif // GTK_USE
struct Test_U_AudioEffect_UI_CBDataBase
#if defined (GTK_USE)
 : Test_U_GTK_CBData
#else
 : Test_U_UI_CBData
#endif // GTK_USE
{
  Test_U_AudioEffect_UI_CBDataBase ()
#if defined (GTK_USE)
   : Test_U_GTK_CBData ()
#else
   : Test_U_UI_CBData ()
#endif // GTK_USE
#if defined (GTK_USE)
   , area2D ()
#if defined (GTKGL_SUPPORT)
   , area3D ()
#endif // GTKGL_SUPPORT
   , surfaceLock ()
#if GTK_CHECK_VERSION(3,10,0)
   , cairoSurface2D (NULL)
#else
   , pixelBuffer2D (NULL)
#endif // GTK_CHECK_VERSION(3,10,0)
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions ()
#endif // GTKGL_SUPPORT
#endif // GTK_USE
   , isFirst (true)
   , progressData ()
#if defined (GTK_USE)
   , resizeNotification (NULL)
#endif // GTK_USE
   , stream (NULL)
  {}

#if defined (GTK_USE)
  GdkRectangle                               area2D;
#if defined (GTKGL_SUPPORT)
  GdkRectangle                               area3D;
#endif // GTKGL_SUPPORT
  ACE_SYNCH_MUTEX                            surfaceLock;
#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_t*                           cairoSurface2D;
#else
  GdkPixbuf*                                 pixelBuffer2D;
#endif // GTK_CHECK_VERSION(3,10,0)
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_OpenGL_Instructions_t OpenGLInstructions;
#endif // GTKGL_SUPPORT
#endif // GTK_USE
  bool                                       isFirst; // first activation ?
  struct Test_U_AudioEffect_ProgressData     progressData;
#if defined (GTK_USE)
  Test_U_Common_ISet_t*                      resizeNotification;
#endif // GTK_USE
  Stream_IStreamControlBase*                 stream;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_UI_CBData
 : Test_U_AudioEffect_UI_CBDataBase
{
  Test_U_AudioEffect_DirectShow_UI_CBData ()
   : Test_U_AudioEffect_UI_CBDataBase ()
   , configuration (NULL)
   , streamConfiguration (NULL)
   , subscribers ()
  {}

  struct Test_U_AudioEffect_DirectShow_Configuration* configuration;
  IAMStreamConfig*                                    streamConfiguration;
  Test_U_AudioEffect_DirectShow_Subscribers_t         subscribers;
};

struct Test_U_AudioEffect_MediaFoundation_UI_CBData
 : Test_U_AudioEffect_UI_CBDataBase
{
  Test_U_AudioEffect_MediaFoundation_UI_CBData ()
   : Test_U_AudioEffect_UI_CBDataBase ()
   , configuration (NULL)
   , subscribers ()
  {}

  struct Test_U_AudioEffect_MediaFoundation_Configuration* configuration;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t         subscribers;
};
#else
struct Test_U_AudioEffect_UI_CBData
 : Test_U_AudioEffect_UI_CBDataBase
{
  Test_U_AudioEffect_UI_CBData ()
   : Test_U_AudioEffect_UI_CBDataBase ()
   , configuration (NULL)
   , handle (NULL)
   , subscribers ()
  {}

  struct Test_U_AudioEffect_Configuration* configuration;
  struct _snd_pcm*                         handle; // (capture) device handle
  Test_U_AudioEffect_Subscribers_t         subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#else
 : Test_U_UI_ThreadData
#endif // GTK_USE
{
  Test_U_AudioEffect_DirectShow_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
#else
   : Test_U_UI_ThreadData ()
#endif // GTK_USE
   , CBData (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Test_U_AudioEffect_DirectShow_UI_CBData* CBData;
};

struct Test_U_AudioEffect_MediaFoundation_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#else
 : Test_U_UI_ThreadData
#endif // GTK_USE
{
  Test_U_AudioEffect_MediaFoundation_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
#else
   : Test_U_UI_ThreadData ()
#endif // GTK_USE
   , CBData (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
  }

  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* CBData;
};
#else
struct Test_U_AudioEffect_ThreadData
#if defined (GTK_USE)
 : Test_U_GTK_ThreadData
#else
 : Test_U_UI_ThreadData
#endif // GTK_USE
{
  Test_U_AudioEffect_ThreadData ()
#if defined (GTK_USE)
   : Test_U_GTK_ThreadData ()
#else
   : Test_U_UI_ThreadData ()
#endif // GTK_USE
   , CBData (NULL)
  {}

  struct Test_U_AudioEffect_UI_CBData* CBData;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTK_USE)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Test_U_AudioEffect_DirectShow_UI_CBData> Test_U_AudioEffect_DirectShow_GtkBuilderDefinition_t;
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Test_U_AudioEffect_MediaFoundation_UI_CBData> Test_U_AudioEffect_MediaFoundation_GtkBuilderDefinition_t;
#else
typedef Common_UI_GtkBuilderDefinition_T<Common_UI_GTK_State_t,
                                         struct Test_U_AudioEffect_UI_CBData> Test_U_AudioEffect_GtkBuilderDefinition_t;
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTK_USE

#endif // GUI_SUPPORT

#endif
