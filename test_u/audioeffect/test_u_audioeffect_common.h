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
#include "devicetopology.h"
#include "endpointvolume.h"
#include "Audioclient.h"
#include "mfapi.h"
#undef GetObject
#include "mfidl.h"
#include "strmif.h"
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}

#if defined (FFMPEG_SUPPORT)
#if defined (__cplusplus)
extern "C"
{
#include "libavutil/rational.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#if defined (GTKGL_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#else
#include "GL/gl.h"
#endif // ACE_WIN32 || ACE_WIN64
#endif // GTKGL_SUPPORT
#endif // GTK_USE
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
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
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"
#include "common_inotify.h"
#include "common_statistic_handler.h"
#include "common_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "common_error_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "common_ui_gtk_builder_definition.h"
#include "common_ui_gtk_gl_common.h"
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT
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
#include "stream_lib_directsound_common.h"
#include "stream_lib_mediafoundation_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_common.h"

#include "stream_stat_common.h"
#include "stream_stat_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_cairo_spectrum_analyzer.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "test_u_common.h"
#include "test_u_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT
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

enum Test_U_AudioEffect_SourceType
{
  AUDIOEFFECT_SOURCE_DEVICE,
  AUDIOEFFECT_SOURCE_NOISE,
  AUDIOEFFECT_SOURCE_FILE,
  ////////////////////////////////////////
  AUDIOEFFECT_SOURCE_MAX,
  AUDIOEFFECT_SOURCE_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_MessageData
{
  Test_U_AudioEffect_DirectShow_MessageData ()
   : sample (NULL)
   , sampleTime (0)
   , index (-1)
  {}

  // DirectShow
  IMediaSample*                sample;
  double                       sampleTime;
  // WaveIn
  unsigned int                 index;
};
struct Test_U_AudioEffect_MediaFoundation_MessageData
{
  Test_U_AudioEffect_MediaFoundation_MessageData ()
   : sample (NULL)
   , sampleTime (0)
   , index (-1)
  {}

  // Media Foundation
  IMFSample*   sample;
  LONGLONG     sampleTime;
  // WaveIn
  unsigned int index;
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

struct Test_U_AudioEffect_Statistic
 : Stream_Statistic
{
  Test_U_AudioEffect_Statistic ()
   : Stream_Statistic ()
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_SessionData
 : Test_U_SessionData
#else
class Test_U_AudioEffect_SessionData
 : public Test_U_ALSA_SessionData
#endif // ACE_WIN32 || ACE_WIN64
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 public:
#endif // ACE_WIN32 || ACE_WIN64
  Test_U_AudioEffect_SessionData ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   : Test_U_SessionData ()
#else
   : Test_U_ALSA_SessionData ()
//   , resolution ()
#endif // ACE_WIN32 || ACE_WIN64
   , statistic ()
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//  Common_Image_Resolution_t           resolution; // *TODO*: remove ASAP !
#endif // ACE_WIN32 || ACE_WIN64
  struct Test_U_AudioEffect_Statistic statistic;
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Stream_SessionData_T<Test_U_AudioEffect_SessionData> Test_U_AudioEffect_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_StreamState;
class Test_U_AudioEffect_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_AudioEffect_SessionData,
                                        struct _AMMediaType,
                                        struct Test_U_AudioEffect_DirectShow_StreamState,
                                        struct Test_U_AudioEffect_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_U_AudioEffect_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_AudioEffect_SessionData,
                                   struct _AMMediaType,
                                   struct Test_U_AudioEffect_DirectShow_StreamState,
                                   struct Test_U_AudioEffect_Statistic,
                                   struct Stream_UserData> ()
   , builder (NULL)
  {}

  IGraphBuilder* builder;
};
typedef Stream_SessionData_T<Test_U_AudioEffect_DirectShow_SessionData> Test_U_AudioEffect_DirectShow_SessionData_t;

struct Test_U_AudioEffect_MediaFoundation_StreamState;
class Test_U_AudioEffect_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_U_AudioEffect_SessionData,
                                        IMFMediaType*,
                                        struct Test_U_AudioEffect_MediaFoundation_StreamState,
                                        struct Test_U_AudioEffect_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_U_AudioEffect_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_U_AudioEffect_SessionData,
                                   IMFMediaType*,
                                   struct Test_U_AudioEffect_MediaFoundation_StreamState,
                                   struct Test_U_AudioEffect_Statistic,
                                   struct Stream_UserData> ()
   , rendererNodeId (0)
   , session (NULL)
   , sourceFormat (NULL)
  {}

  TOPOID           rendererNodeId;
  IMFMediaSession* session;
  IMFMediaType*    sourceFormat;
};
typedef Stream_SessionData_T<Test_U_AudioEffect_MediaFoundation_SessionData> Test_U_AudioEffect_MediaFoundation_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_ISessionDataNotify_T<Test_U_AudioEffect_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_DirectShow_Message,
                                    Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_DirectShow_ISessionNotify_t*> Test_U_AudioEffect_DirectShow_Subscribers_t;
typedef Test_U_AudioEffect_DirectShow_Subscribers_t::iterator Test_U_AudioEffect_DirectShow_SubscribersIterator_t;

typedef Stream_ISessionDataNotify_T<Test_U_AudioEffect_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_MediaFoundation_Message,
                                    Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*> Test_U_AudioEffect_MediaFoundation_Subscribers_t;
typedef Test_U_AudioEffect_MediaFoundation_Subscribers_t::iterator Test_U_AudioEffect_MediaFoundation_SubscribersIterator_t;
#else
typedef Stream_ISessionDataNotify_T<Test_U_AudioEffect_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_AudioEffect_Message,
                                    Test_U_AudioEffect_SessionMessage> Test_U_AudioEffect_ISessionNotify_t;
typedef std::list<Test_U_AudioEffect_ISessionNotify_t*> Test_U_AudioEffect_Subscribers_t;
typedef Test_U_AudioEffect_Subscribers_t::iterator Test_U_AudioEffect_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64
struct Test_U_AudioEffect_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , bufferSize (MODULE_STAT_ANALYSIS_DEFAULT_BUFFER_SIZE)
   , deviceIdentifier ()
   , delayConfiguration (NULL)
   , dispatch (NULL)
   , fps (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE)
   , generatorConfiguration (NULL)
#if defined (SOX_SUPPORT)
   , manageSoX (false)
#endif // SOX_SUPPORT
   , mute (false)
#if defined (GUI_SUPPORT)
   , spectrumAnalyzer2DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzerResolution (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
#if defined (GTKGL_SUPPORT)
   , OpenGLTextureId (0)
   , spectrumAnalyzer3DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_3DMODE)
#endif // GTKGL_SUPPORT
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
  {}

  unsigned int                                              bufferSize; // statistic analysis
  struct Stream_Device_Identifier                           deviceIdentifier; // capture/render
  struct Stream_Miscellaneous_DelayConfiguration*           delayConfiguration;
  Stream_Statistic_IDispatch_t*                             dispatch;
  unsigned int                                              fps;
  struct Stream_MediaFramework_SoundGeneratorConfiguration* generatorConfiguration;
#if defined (SOX_SUPPORT)
  bool                                                      manageSoX;
#endif // SOX_SUPPORT
  bool                                                      mute;
#if defined (GUI_SUPPORT)
  enum Stream_Visualization_SpectrumAnalyzer_2DMode         spectrumAnalyzer2DMode;
  unsigned int                                              spectrumAnalyzerResolution;
#if defined (GTKGL_SUPPORT)
  GLuint                                                    OpenGLTextureId;
  enum Stream_Visualization_SpectrumAnalyzer_3DMode         spectrumAnalyzer3DMode;
#endif /* GTKGL_SUPPORT */
#if defined (GTK_SUPPORT)
  GdkWindow*                                                window;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//extern const char stream_name_string_[];
struct Test_U_AudioEffect_DirectShow_StreamConfiguration;
struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_AudioEffect_DirectShow_StreamConfiguration,
                               struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration> Test_U_AudioEffect_DirectShow_StreamConfiguration_t;
struct Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_DirectShow_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , builder (NULL)
   , effect (GUID_NULL)
   , effectOptions ()
   , filterConfiguration (NULL)
   , filterCLSID (GUID_NULL)
   , outputFormat ()
   , push (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_DEFAULT_PUSH)
   , sampleIsDataMessage (false)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

  IGraphBuilder*                                             builder;
  CLSID                                                      effect;
  union Stream_MediaFramework_DirectSound_AudioEffectOptions effectOptions;
  struct Test_U_AudioEffect_DirectShow_FilterConfiguration*  filterConfiguration;
  CLSID                                                      filterCLSID;
  struct _AMMediaType                                        outputFormat;
  // *IMPORTANT NOTE*: 'asynchronous' filters implement IAsyncReader (downstream
  //                   filters 'pull' media samples), 'synchronous' filters
  //                   implement IMemInputPin and 'push' media samples to
  //                   downstream filters
  bool                                                       push;
  bool                                                       sampleIsDataMessage;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t*       streamConfiguration;
  Test_U_AudioEffect_DirectShow_ISessionNotify_t*            subscriber;
  Test_U_AudioEffect_DirectShow_Subscribers_t*               subscribers;
};

//extern const char stream_name_string_[];
struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration;
struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration,
                               struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration> Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t;
struct Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , effect (GUID_NULL)
   , effectOptions ()
   , manageMediaSession (false)
   , mediaFoundationConfiguration (NULL)
   , session (NULL)
   , outputFormat (NULL)
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    //HRESULT result = MFCreateMediaType (&outputFormat);
    //if (FAILED (result))
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", continuing\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  }

  CLSID                                                       effect;
  std::string                                                 effectOptions;
  bool                                                        manageMediaSession;
  struct Stream_MediaFramework_MediaFoundation_Configuration* mediaFoundationConfiguration;
  IMFMediaSession*                                            session;
  IMFMediaType*                                               outputFormat;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t*   streamConfiguration;
  Test_U_AudioEffect_MediaFoundation_ISessionNotify_t*        subscriber;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t*           subscribers;
};
#else
struct Test_U_AudioEffect_ALSA_StreamConfiguration;
struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration;
//extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_AudioEffect_ALSA_StreamConfiguration,
                               struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration> Test_U_AudioEffect_ALSA_StreamConfiguration_t;
struct Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration
 : Test_U_AudioEffect_ModuleHandlerConfiguration
{
  Test_U_AudioEffect_ALSA_ModuleHandlerConfiguration ()
   : Test_U_AudioEffect_ModuleHandlerConfiguration ()
   , ALSAConfiguration (NULL)
   , effect ()
   , effectOptions ()
   , outputFormat ()
   , streamConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
  {
    deviceIdentifier.identifier =
        ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_CAPTURE_DEFAULT_DEVICE_NAME);
  }

  struct Stream_MediaFramework_ALSA_Configuration* ALSAConfiguration;
  std::string                                      effect;
  std::vector<std::string>                         effectOptions;
  struct Stream_MediaFramework_ALSA_MediaType      outputFormat;
  Test_U_AudioEffect_ALSA_StreamConfiguration_t*   streamConfiguration;
  Test_U_AudioEffect_ISessionNotify_t*             subscriber;
  Test_U_AudioEffect_Subscribers_t*                subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_StreamState
 : Stream_State
{
  Test_U_AudioEffect_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_AudioEffect_DirectShow_SessionData* sessionData;
};

struct Test_U_AudioEffect_MediaFoundation_StreamState
 : Stream_State
{
  Test_U_AudioEffect_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_AudioEffect_MediaFoundation_SessionData* sessionData;
};
#else
struct Test_U_AudioEffect_StreamState
 : Stream_State
{
  Test_U_AudioEffect_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_AudioEffect_SessionData* sessionData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_AudioEffect_StreamConfiguration
 : Stream_Configuration
{
  Test_U_AudioEffect_StreamConfiguration ()
   : Stream_Configuration ()
   , capturer (STREAM_DEVICE_CAPTURER_INVALID)
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
   , sourceType (AUDIOEFFECT_SOURCE_INVALID)
  {}

  enum Stream_Device_Capturer        capturer;
  enum Stream_Device_Renderer        renderer;
  enum Test_U_AudioEffect_SourceType sourceType;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_StreamConfiguration
 : Test_U_AudioEffect_StreamConfiguration
{
  Test_U_AudioEffect_DirectShow_StreamConfiguration ()
   : Test_U_AudioEffect_StreamConfiguration ()
   , filterGraphConfiguration ()
   , format ()
  {
    capturer = STREAM_DEVICE_CAPTURER_DIRECTSHOW;
    renderer = STREAM_DEVICE_RENDERER_DIRECTSHOW;
    ACE_OS::memset (&format, 0, sizeof (struct _AMMediaType));
  }

  Stream_MediaFramework_DirectShow_Graph_t filterGraphConfiguration;
  struct _AMMediaType                      format;
};

struct Test_U_AudioEffect_MediaFoundation_StreamConfiguration
 : Test_U_AudioEffect_StreamConfiguration
{
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration ()
   : Test_U_AudioEffect_StreamConfiguration ()
   , format (NULL)
  {
    capturer = STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION;
    renderer = STREAM_DEVICE_RENDERER_MEDIAFOUNDATION;
  }

  IMFMediaType* format;
};
#else
struct Test_U_AudioEffect_ALSA_StreamConfiguration
 : Test_U_AudioEffect_StreamConfiguration
{
  Test_U_AudioEffect_ALSA_StreamConfiguration ()
   : Test_U_AudioEffect_StreamConfiguration ()
   , format ()
  {
    capturer = STREAM_DEVICE_CAPTURER_ALSA;
    renderer = STREAM_DEVICE_RENDERER_ALSA;
  }

  struct Stream_MediaFramework_ALSA_MediaType format;
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
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
 : Test_U_GTK_Configuration
#else
 : Test_U_Configuration
#endif // GTK_USE
#else
 : Test_U_Configuration
#endif // GUI_SUPPORT
{
  Test_U_AudioEffect_Configuration ()
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
   : Test_U_GTK_Configuration ()
#else
   : Test_U_Configuration ()
#endif // GTK_USE
#else
   : Test_U_Configuration ()
#endif // GUI_SUPPORT
   , delayConfiguration ()
   , generatorConfiguration ()
   , signalHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , streamConfiguration ()
#endif // ACE_WIN32 || ACE_WIN64
  {}

  struct Stream_Miscellaneous_DelayConfiguration           delayConfiguration;
  struct Stream_MediaFramework_SoundGeneratorConfiguration generatorConfiguration;
  struct Test_U_AudioEffect_SignalHandlerConfiguration     signalHandlerConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Test_U_AudioEffect_ALSA_StreamConfiguration_t            streamConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AudioEffect_DirectShow_FilterConfiguration
 : Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Test_U_AudioEffect_DirectShow_FilterConfiguration ()
   : Stream_MediaFramework_DirectShow_FilterConfiguration ()
   , module (NULL)
  {}

  Stream_Module_t* module; // handle
};

struct Test_U_AudioEffect_DirectShow_Configuration
 : Test_U_AudioEffect_Configuration
{
  Test_U_AudioEffect_DirectShow_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , allocatorProperties ()
   , filterConfiguration ()
   , pinConfiguration ()
   , streamConfiguration ()
  {
    ACE_OS::memset (&allocatorProperties, 0, sizeof (struct _AllocatorProperties));
    //allocatorProperties_.cBuffers = -1; // <-- use default
    allocatorProperties.cBuffers =
      STREAM_LIB_DIRECTSHOW_AUDIO_DEFAULT_SOURCE_BUFFERS;
    allocatorProperties.cbBuffer = -1; // <-- use default
    // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
    //         if this is -1/0 (why ?)
    //allocatorProperties_.cbAlign = -1;  // <-- use default
    allocatorProperties.cbAlign = 1;
    // *TODO*: IMemAllocator::SetProperties returns E_INVALIDARG (0x80070057)
    //         if this is -1/0 (why ?)
    //allocatorProperties.cbPrefix = -1; // <-- use default
    allocatorProperties.cbPrefix = 0;

    filterConfiguration.allocatorProperties = &allocatorProperties;
    filterConfiguration.pinConfiguration = &pinConfiguration;
    pinConfiguration.allocatorProperties = &allocatorProperties;
  }

  struct _AllocatorProperties                                    allocatorProperties; // IMediaSample-
  struct Test_U_AudioEffect_DirectShow_FilterConfiguration       filterConfiguration;
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration pinConfiguration;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t            streamConfiguration;
};
struct Test_U_AudioEffect_MediaFoundation_Configuration
 : Test_U_AudioEffect_Configuration
{
  Test_U_AudioEffect_MediaFoundation_Configuration ()
   : Test_U_AudioEffect_Configuration ()
   , mediaFoundationConfiguration ()
   , streamConfiguration ()
  {}

  struct Stream_MediaFramework_MediaFoundation_Configuration mediaFoundationConfiguration;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t   streamConfiguration;
};
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_AudioEffect_DirectShow_Message,
                                          Test_U_AudioEffect_DirectShow_SessionMessage> Test_U_AudioEffect_DirectShow_MessageAllocator_t;

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_AudioEffect_DirectShow_StreamState> Test_U_AudioEffect_DirectShow_IStreamControl_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_DirectShow_ISessionNotify_t> Test_U_AudioEffect_DirectShow_ISubscribe_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_AudioEffect_MediaFoundation_Message,
                                          Test_U_AudioEffect_MediaFoundation_SessionMessage> Test_U_AudioEffect_MediaFoundation_MessageAllocator_t;

typedef Stream_IStreamControl_T<enum Stream_ControlType,
                                enum Stream_SessionMessageType,
                                enum Stream_StateMachine_ControlState,
                                struct Test_U_AudioEffect_MediaFoundation_StreamState> Test_U_AudioEffect_MediaFoundation_IStreamControl_t;

typedef Common_ISubscribe_T<Test_U_AudioEffect_MediaFoundation_ISessionNotify_t> Test_U_AudioEffect_MediaFoundation_ISubscribe_t;
#else
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
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
 : Test_U_UI_ProgressData
{
  Test_U_AudioEffect_ProgressData ()
   : Test_U_UI_ProgressData ()
   , bytesPerFrame (0)
   , statistic ()
  {}

  ACE_UINT32                          bytesPerFrame;
  struct Test_U_AudioEffect_Statistic statistic;
};

#if defined (GTK_SUPPORT)
typedef Common_ISetP_T<GdkWindow> Test_U_Common_ISet_t;
#endif // GTK_SUPPORT
struct Test_U_AudioEffect_UI_CBDataBase
 : Test_U_UI_CBData
{
  Test_U_AudioEffect_UI_CBDataBase ()
   : Test_U_UI_CBData ()
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions (NULL)
   , OpenGLInstructionsLock (NULL)
#endif // GTKGL_SUPPORT
   , isFirst (true)
#if defined (GTKGL_SUPPORT)
   , objectRotation (0.0f)
#endif // GTKGL_SUPPORT
   , progressData ()
#if defined (GTK_SUPPORT)
   , resizeNotification (NULL)
   , spectrumAnalyzerCBData ()
#endif // GTK_SUPPORT
   , stream (NULL)
  {}

#if defined (GTKGL_SUPPORT)
  Stream_Visualization_GTKGL_Instructions_t*      OpenGLInstructions;
  ACE_Thread_Mutex*                               OpenGLInstructionsLock;
#endif // GTKGL_SUPPORT
  bool                                            isFirst; // first activation ?
#if defined (GTKGL_SUPPORT)
  GLfloat                                         objectRotation;
#endif // GTKGL_SUPPORT
  struct Test_U_AudioEffect_ProgressData          progressData;
#if defined (GTK_SUPPORT)
  Test_U_Common_ISet_t*                           resizeNotification;
  struct acestream_visualization_gtk_cairo_cbdata spectrumAnalyzerCBData;
#endif // GTK_SUPPORT
  Stream_IStreamControlBase*                      stream;
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
   , boostControl (NULL)
   , captureVolumeControl (NULL)
   , renderVolumeControl (NULL)
  {}

  struct Test_U_AudioEffect_DirectShow_Configuration* configuration;
  IAMStreamConfig*                                    streamConfiguration;
  Test_U_AudioEffect_DirectShow_Subscribers_t         subscribers;
  IAudioVolumeLevel*                                  boostControl;
  IAudioEndpointVolume*                               captureVolumeControl;
  ISimpleAudioVolume*                                 renderVolumeControl;
};

struct Test_U_AudioEffect_MediaFoundation_UI_CBData
 : Test_U_AudioEffect_UI_CBDataBase
{
  Test_U_AudioEffect_MediaFoundation_UI_CBData ()
   : Test_U_AudioEffect_UI_CBDataBase ()
   , configuration (NULL)
   , subscribers ()
   , boostControl (NULL)
   , captureVolumeControl (NULL)
   , renderVolumeControl (NULL)
  {}

  struct Test_U_AudioEffect_MediaFoundation_Configuration* configuration;
  Test_U_AudioEffect_MediaFoundation_Subscribers_t         subscribers;
  IAudioVolumeLevel*                                       boostControl;
  IAudioEndpointVolume*                                    captureVolumeControl;
  ISimpleAudioVolume*                                      renderVolumeControl;
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
 : Test_U_UI_ThreadData
{
  Test_U_AudioEffect_DirectShow_ThreadData ()
   : Test_U_UI_ThreadData ()
   , CBData (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_DIRECTSHOW;
  }

  struct Test_U_AudioEffect_DirectShow_UI_CBData* CBData;
};

struct Test_U_AudioEffect_MediaFoundation_ThreadData
 : Test_U_UI_ThreadData
{
  Test_U_AudioEffect_MediaFoundation_ThreadData ()
   : Test_U_UI_ThreadData ()
   , CBData (NULL)
  {
    mediaFramework = STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION;
  }

  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* CBData;
};
#else
struct Test_U_AudioEffect_ThreadData
 : Test_U_UI_ThreadData
{
  Test_U_AudioEffect_ThreadData ()
   : Test_U_UI_ThreadData ()
   , CBData (NULL)
  {}

  struct Test_U_AudioEffect_UI_CBData* CBData;
};
#endif // ACE_WIN32 || ACE_WIN64
#endif // GUI_SUPPORT

#endif
