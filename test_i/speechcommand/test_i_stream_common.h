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

#ifndef TEST_I_STREAM_COMMON_H
#define TEST_I_STREAM_COMMON_H

#include <list>

#if defined (GUI_SUPPORT)
#if defined (OPENGL_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#else
#include "GL/gl.h"
#endif // ACE_WIN32 || ACE_WIN64
#endif // OPENGL_SUPPORT
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_messagequeue.h"

#include "stream_dec_common.h"

#include "stream_dev_common.h"

#include "stream_stat_common.h"
#include "stream_stat_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_common.h"
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT

#include "test_i_common.h"
#include "test_i_configuration.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_Message;
class Test_I_MediaFoundation_Message;
#else
class Test_I_Message;
#endif // ACE_WIN32 || ACE_WIN64
template <typename SessionDataType,
          typename UserDataType>
class Test_I_SessionMessage_T;

//////////////////////////////////////////

struct Test_I_Statistic
 : Stream_Statistic
{
  Test_I_Statistic ()
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
typedef Common_StatisticHandler_T<struct Test_I_Statistic> Test_I_StatisticHandler_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_SpeechCommand_DirectShow_SessionData;
struct Test_I_SpeechCommand_DirectShow_StreamState
 : Stream_State
{
  Test_I_SpeechCommand_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_SpeechCommand_DirectShow_SessionData* sessionData;
};

class Test_I_SpeechCommand_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct _AMMediaType,
                                        struct Test_I_SpeechCommand_DirectShow_StreamState,
                                        struct Test_I_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_SpeechCommand_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct _AMMediaType,
                                   struct Test_I_SpeechCommand_DirectShow_StreamState,
                                   struct Test_I_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_SpeechCommand_DirectShow_SessionData& operator+= (const Test_I_SpeechCommand_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Test_I_SpeechCommand_DirectShow_StreamState,
                                  struct Test_I_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_SpeechCommand_DirectShow_SessionData> Test_I_SpeechCommand_DirectShow_SessionData_t;

class Test_I_SpeechCommand_MediaFoundation_SessionData;
struct Test_I_SpeechCommand_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_SpeechCommand_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_SpeechCommand_MediaFoundation_SessionData* sessionData;
};

class Test_I_SpeechCommand_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        IMFMediaType*,
                                        struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                                        struct Test_I_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_SpeechCommand_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   IMFMediaType*,
                                   struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                                   struct Test_I_Statistic,
                                   struct Stream_UserData> ()
   , rendererNodeId (0)
   , session (NULL)
  {}

  Test_I_SpeechCommand_MediaFoundation_SessionData& operator+= (const Test_I_SpeechCommand_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Test_I_SpeechCommand_MediaFoundation_StreamState,
                                  struct Test_I_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }

  TOPOID           rendererNodeId;
  IMFMediaSession* session;
};
typedef Stream_SessionData_T<Test_I_SpeechCommand_MediaFoundation_SessionData> Test_I_SpeechCommand_MediaFoundation_SessionData_t;
#else
class Test_I_SpeechCommand_ALSA_SessionData;
struct Test_I_SpeechCommand_ALSA_StreamState
 : Stream_State
{
    Test_I_SpeechCommand_ALSA_StreamState ()
     : Stream_State ()
     , sessionData (NULL)
    {}

    Test_I_SpeechCommand_ALSA_SessionData* sessionData;
};

class Test_I_SpeechCommand_ALSA_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct Stream_MediaFramework_ALSA_MediaType,
                                        struct Test_I_SpeechCommand_ALSA_StreamState,
                                        struct Test_I_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_SpeechCommand_ALSA_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType,
                                   struct Test_I_SpeechCommand_ALSA_StreamState,
                                   struct Test_I_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_SpeechCommand_ALSA_SessionData& operator+= (const Test_I_SpeechCommand_ALSA_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct Stream_MediaFramework_ALSA_MediaType,
                                  struct Test_I_SpeechCommand_ALSA_StreamState,
                                  struct Test_I_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_SpeechCommand_ALSA_SessionData> Test_I_SpeechCommand_ALSA_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_SessionMessage_T<Test_I_SpeechCommand_DirectShow_SessionData_t,
                                struct Stream_UserData> Test_I_DirectShow_SessionMessage_t;
typedef Test_I_SessionMessage_T<Test_I_SpeechCommand_MediaFoundation_SessionData_t,
                                struct Stream_UserData> Test_I_MediaFoundation_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_DirectShow_Message,
                                          Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_MessageAllocator_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_MediaFoundation_Message,
                                          Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Test_I_SpeechCommand_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_DirectShow_Message,
                                    Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_ISessionNotify_t;
typedef Stream_ISessionDataNotify_T<Test_I_SpeechCommand_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_MediaFoundation_Message,
                                    Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_ISessionNotify_t;

typedef std::list<Test_I_DirectShow_ISessionNotify_t*> Test_I_DirectShow_Subscribers_t;
typedef Test_I_DirectShow_Subscribers_t::iterator Test_I_DirectShow_SubscribersIterator_t;
typedef std::list<Test_I_MediaFoundation_ISessionNotify_t*> Test_I_MediaFoundation_Subscribers_t;
typedef Test_I_MediaFoundation_Subscribers_t::iterator Test_I_MediaFoundation_SubscribersIterator_t;
#else
typedef Test_I_SessionMessage_T<Test_I_SpeechCommand_ALSA_SessionData_t,
                                struct Stream_UserData> Test_I_ALSA_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_ALSA_SessionMessage_t> Test_I_ALSA_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Test_I_SpeechCommand_ALSA_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_ALSA_SessionMessage_t> Test_I_ALSA_ISessionNotify_t;

typedef std::list<Test_I_ALSA_ISessionNotify_t*> Test_I_ALSA_Subscribers_t;
typedef Test_I_ALSA_Subscribers_t::iterator Test_I_ALSA_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration
 : Test_I_DirectShow_ModuleHandlerConfiguration
{
  Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_DirectShow_ModuleHandlerConfiguration ()
   , bufferSize (MODULE_STAT_ANALYSIS_DEFAULT_BUFFER_SIZE)
   , deviceIdentifier ()
   , dispatch (NULL)
   , effect ()
   , effectOptions ()
   , hotWords ()
   , manageSoX (false)
   , modelFile ()
   , mute (false)
   , scorerFile ()
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions (NULL)
   , OpenGLInstructionsLock (NULL)
   , OpenGLTextureId (0)
#endif /* GTKGL_SUPPORT */
#endif // GTK_SUPPORT
   , spectrumAnalyzer2DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzerResolution (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
#endif // GUI_SUPPORT
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  unsigned int                                      bufferSize; // statistic analysis
  struct Stream_Device_Identifier                   deviceIdentifier; // capture/render
  Stream_Statistic_IDispatch_t*                     dispatch;
  std::string                                       effect;
  std::vector<std::string>                          effectOptions;
  Stream_Decoder_DeepSpeech_HotWords_t              hotWords;
  bool                                              manageSoX;
  std::string                                       modelFile;
  bool                                              mute;
  std::string                                       scorerFile;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_GTKGL_Instructions_t*        OpenGLInstructions;
  ACE_SYNCH_MUTEX*                                  OpenGLInstructionsLock;
  GLuint                                            OpenGLTextureId;
#endif /* GTKGL_SUPPORT */
#endif // GTK_SUPPORT
  enum Stream_Visualization_SpectrumAnalyzer_2DMode spectrumAnalyzer2DMode;
  unsigned int                                      spectrumAnalyzerResolution;
#endif // GUI_SUPPORT
  Test_I_DirectShow_ISessionNotify_t*               subscriber;
  Test_I_DirectShow_Subscribers_t*                  subscribers;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  GdkWindow*                                        window;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
};

struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_MediaFoundation_ModuleHandlerConfiguration
{
  Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_MediaFoundation_ModuleHandlerConfiguration ()
   , bufferSize (MODULE_STAT_ANALYSIS_DEFAULT_BUFFER_SIZE)
   , deviceIdentifier ()
   , dispatch (NULL)
   , effect ()
   , effectOptions ()
   , hotWords ()
   , manageSoX (false)
   , modelFile ()
   , mute (false)
   , scorerFile ()
#if defined(GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions (NULL)
   , OpenGLInstructionsLock (NULL)
   , OpenGLTextureId (0)
#endif /* GTKGL_SUPPORT */
#endif // GTK_SUPPORT
   , spectrumAnalyzer2DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzerResolution (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
#endif // GUI_SUPPORT
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  unsigned int                                      bufferSize; // statistic analysis
  struct Stream_Device_Identifier                   deviceIdentifier; // capture/render
  Stream_Statistic_IDispatch_t*                     dispatch;
  std::string                                       effect;
  std::vector<std::string>                          effectOptions;
  Stream_Decoder_DeepSpeech_HotWords_t              hotWords;
  bool                                              manageSoX;
  std::string                                       modelFile;
  bool                                              mute;
  std::string                                       scorerFile;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_GTKGL_Instructions_t*        OpenGLInstructions;
  ACE_SYNCH_MUTEX*                                  OpenGLInstructionsLock;
  GLuint                                            OpenGLTextureId;
#endif /* GTKGL_SUPPORT */
#endif // GTK_SUPPORT
  enum Stream_Visualization_SpectrumAnalyzer_2DMode spectrumAnalyzer2DMode;
  unsigned int                                      spectrumAnalyzerResolution;
#endif // GUI_SUPPORT
  Test_I_MediaFoundation_ISessionNotify_t*          subscriber;
  Test_I_MediaFoundation_Subscribers_t*             subscribers;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  GdkWindow*                                        window;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
};
#else
struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration
 : Test_I_ALSA_ModuleHandlerConfiguration
{
  Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration ()
   : Test_I_ALSA_ModuleHandlerConfiguration ()
   , bufferSize (MODULE_STAT_ANALYSIS_DEFAULT_BUFFER_SIZE)
   , deviceIdentifier ()
   , dispatch (NULL)
   , effect ()
   , effectOptions ()
   , hotWords ()
   , manageSoX (false)
   , modelFile ()
   , mute (false)
   , scorerFile ()
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
   , OpenGLInstructions (NULL)
   , OpenGLInstructionsLock (NULL)
   , OpenGLTextureId (0)
#endif /* GTKGL_SUPPORT */
#endif // GTK_SUPPORT
   , spectrumAnalyzer2DMode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , spectrumAnalyzerResolution (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_BUFFER_SIZE)
#endif // GUI_SUPPORT
   , subscriber (NULL)
   , subscribers (NULL)
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  unsigned int                                      bufferSize; // statistic analysis
  struct Stream_Device_Identifier                   deviceIdentifier; // capture/render
  Stream_Statistic_IDispatch_t*                     dispatch;
  std::string                                       effect;
  std::vector<std::string>                          effectOptions;
  Stream_Decoder_DeepSpeech_HotWords_t              hotWords;
  bool                                              manageSoX;
  std::string                                       modelFile;
  bool                                              mute;
  std::string                                       scorerFile;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_GTKGL_Instructions_t*        OpenGLInstructions;
  ACE_SYNCH_MUTEX*                                  OpenGLInstructionsLock;
  GLuint                                            OpenGLTextureId;
#endif /* GTKGL_SUPPORT */
#endif // GTK_SUPPORT
  enum Stream_Visualization_SpectrumAnalyzer_2DMode spectrumAnalyzer2DMode;
  unsigned int                                      spectrumAnalyzerResolution;
#endif // GUI_SUPPORT
  Test_I_ALSA_ISessionNotify_t*                     subscriber;
  Test_I_ALSA_Subscribers_t*                        subscribers;
#if defined (GUI_SUPPORT)
#if defined (GTK_SUPPORT)
  GdkWindow*                                        window;
#endif // GTK_SUPPORT
#endif // GUI_SUPPORT
};
#endif // ACE_WIN32 || ACE_WIN64

//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_DirectShow_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , capturer (STREAM_DEVICE_CAPTURER_INVALID)
   , filterGraphConfiguration ()
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
   , format ()
  {
    capturer = STREAM_DEVICE_CAPTURER_DIRECTSHOW;
    renderer = STREAM_DEVICE_RENDERER_DIRECTSHOW;
    ACE_OS::memset (&format, 0, sizeof (struct _AMMediaType));
  }

  enum Stream_Device_Capturer              capturer;
  Stream_MediaFramework_DirectShow_Graph_t filterGraphConfiguration;
  enum Stream_Device_Renderer              renderer;
  struct _AMMediaType                      format;
};

struct Test_I_MediaFoundation_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_MediaFoundation_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , capturer (STREAM_DEVICE_CAPTURER_INVALID)
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
   , format (NULL)
  {
    capturer = STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION;
    renderer = STREAM_DEVICE_RENDERER_MEDIAFOUNDATION;
  }

  enum Stream_Device_Capturer capturer;
  enum Stream_Device_Renderer renderer;
  IMFMediaType*               format;
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_DirectShow_StreamConfiguration,
                               struct Test_I_SpeechCommand_DirectShow_ModuleHandlerConfiguration> Test_I_DirectShow_StreamConfiguration_t;
typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Test_I_SpeechCommand_DirectShow_StreamState> Test_I_DirectShow_IStreamControlBase_t;

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_MediaFoundation_StreamConfiguration,
                               struct Test_I_SpeechCommand_MediaFoundation_ModuleHandlerConfiguration> Test_I_MediaFoundation_StreamConfiguration_t;
typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Test_I_SpeechCommand_MediaFoundation_StreamState> Test_I_MediaFoundation_IStreamControlBase_t;
#else
struct Test_I_ALSA_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_ALSA_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , capturer (STREAM_DEVICE_CAPTURER_INVALID)
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
   , format ()
  {
    capturer = STREAM_DEVICE_CAPTURER_ALSA;
    renderer = STREAM_DEVICE_RENDERER_ALSA;
  }

  enum Stream_Device_Capturer                 capturer;
  enum Stream_Device_Renderer                 renderer;
  struct Stream_MediaFramework_ALSA_MediaType format;
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_ALSA_StreamConfiguration,
                               struct Test_I_SpeechCommand_ALSA_ModuleHandlerConfiguration> Test_I_ALSA_StreamConfiguration_t;

typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Test_I_SpeechCommand_ALSA_StreamState> Test_I_ALSA_IStreamControlBase_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
