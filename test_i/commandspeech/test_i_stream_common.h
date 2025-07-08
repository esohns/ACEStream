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

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "stream_common.h"
#include "stream_configuration.h"
#include "stream_messagequeue.h"

#include "stream_dec_common.h"

#include "stream_dev_common.h"

#include "stream_stat_common.h"
#include "stream_stat_defines.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"
#if defined (GTK_SUPPORT)
#include "stream_vis_gtk_common.h"
#endif // GTK_SUPPORT

#include "test_i_common.h"
#include "test_i_configuration.h"

#include "test_i_session_message.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_DirectShow_Message;
class Test_I_MediaFoundation_Message;
#else
class Test_I_Message;
#endif // ACE_WIN32 || ACE_WIN64

//////////////////////////////////////////

#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Test_I_CommandSpeech_DirectShow_SessionData;
struct Test_I_CommandSpeech_DirectShow_StreamState
 : Stream_State
{
  Test_I_CommandSpeech_DirectShow_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_CommandSpeech_DirectShow_SessionData* sessionData;
};

class Test_I_CommandSpeech_DirectShow_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct _AMMediaType,
                                        struct Test_I_CommandSpeech_DirectShow_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_CommandSpeech_DirectShow_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct _AMMediaType,
                                   struct Test_I_CommandSpeech_DirectShow_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_CommandSpeech_DirectShow_SessionData& operator+= (const Test_I_CommandSpeech_DirectShow_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct _AMMediaType,
                                  struct Test_I_CommandSpeech_DirectShow_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_CommandSpeech_DirectShow_SessionData> Test_I_CommandSpeech_DirectShow_SessionData_t;

class Test_I_CommandSpeech_MediaFoundation_SessionData;
struct Test_I_CommandSpeech_MediaFoundation_StreamState
 : Stream_State
{
  Test_I_CommandSpeech_MediaFoundation_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_I_CommandSpeech_MediaFoundation_SessionData* sessionData;
};

class Test_I_CommandSpeech_MediaFoundation_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        IMFMediaType*,
                                        struct Test_I_CommandSpeech_MediaFoundation_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_CommandSpeech_MediaFoundation_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   IMFMediaType*,
                                   struct Test_I_CommandSpeech_MediaFoundation_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , rendererNodeId (0)
   , session (NULL)
  {}

  Test_I_CommandSpeech_MediaFoundation_SessionData& operator+= (const Test_I_CommandSpeech_MediaFoundation_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  IMFMediaType*,
                                  struct Test_I_CommandSpeech_MediaFoundation_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }

  TOPOID           rendererNodeId;
  IMFMediaSession* session;
};
typedef Stream_SessionData_T<Test_I_CommandSpeech_MediaFoundation_SessionData> Test_I_CommandSpeech_MediaFoundation_SessionData_t;
#else
class Test_I_CommandSpeech_ALSA_SessionData;
struct Test_I_CommandSpeech_ALSA_StreamState
 : Stream_State
{
    Test_I_CommandSpeech_ALSA_StreamState ()
     : Stream_State ()
     , sessionData (NULL)
    {}

    Test_I_CommandSpeech_ALSA_SessionData* sessionData;
};

class Test_I_CommandSpeech_ALSA_SessionData
 : public Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                        struct Stream_MediaFramework_ALSA_MediaType,
                                        struct Test_I_CommandSpeech_ALSA_StreamState,
                                        struct Stream_Statistic,
                                        struct Stream_UserData>
{
 public:
  Test_I_CommandSpeech_ALSA_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                   struct Stream_MediaFramework_ALSA_MediaType,
                                   struct Test_I_CommandSpeech_ALSA_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
  {}

  Test_I_CommandSpeech_ALSA_SessionData& operator+= (const Test_I_CommandSpeech_ALSA_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Test_I_SessionData,
                                  struct Stream_MediaFramework_ALSA_MediaType,
                                  struct Test_I_CommandSpeech_ALSA_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);
    return *this;
  }
};
typedef Stream_SessionData_T<Test_I_CommandSpeech_ALSA_SessionData> Test_I_CommandSpeech_ALSA_SessionData_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
typedef Test_I_SessionMessage_T<Test_I_CommandSpeech_DirectShow_SessionData_t,
                                struct Stream_UserData> Test_I_DirectShow_SessionMessage_t;
typedef Test_I_SessionMessage_T<Test_I_CommandSpeech_MediaFoundation_SessionData_t,
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

typedef Stream_ISessionDataNotify_T<Test_I_CommandSpeech_DirectShow_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_DirectShow_Message,
                                    Test_I_DirectShow_SessionMessage_t> Test_I_DirectShow_ISessionNotify_t;
typedef Stream_ISessionDataNotify_T<Test_I_CommandSpeech_MediaFoundation_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_MediaFoundation_Message,
                                    Test_I_MediaFoundation_SessionMessage_t> Test_I_MediaFoundation_ISessionNotify_t;

typedef std::list<Test_I_DirectShow_ISessionNotify_t*> Test_I_DirectShow_Subscribers_t;
typedef Test_I_DirectShow_Subscribers_t::iterator Test_I_DirectShow_SubscribersIterator_t;
typedef std::list<Test_I_MediaFoundation_ISessionNotify_t*> Test_I_MediaFoundation_Subscribers_t;
typedef Test_I_MediaFoundation_Subscribers_t::iterator Test_I_MediaFoundation_SubscribersIterator_t;
#else
typedef Test_I_SessionMessage_T<Test_I_CommandSpeech_ALSA_SessionData_t,
                                struct Stream_UserData> Test_I_ALSA_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Message,
                                          Test_I_ALSA_SessionMessage_t> Test_I_ALSA_MessageAllocator_t;

typedef Stream_ISessionDataNotify_T<Test_I_CommandSpeech_ALSA_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_I_Message,
                                    Test_I_ALSA_SessionMessage_t> Test_I_ALSA_ISessionNotify_t;

typedef std::list<Test_I_ALSA_ISessionNotify_t*> Test_I_ALSA_Subscribers_t;
typedef Test_I_ALSA_Subscribers_t::iterator Test_I_ALSA_SubscribersIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration
 : Test_I_DirectShow_ModuleHandlerConfiguration
{
  Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration ()
   : Test_I_DirectShow_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , effect ()
   , effectOptions ()
   , manageFlite (false)
   , manageSoX (false)
   , mute (false)
   , queue (NULL)
   , spectrumAnalyzerConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , voice ()
   , voiceDirectory ()
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  struct Stream_Device_Identifier                   deviceIdentifier; // render-
  std::string                                       effect;
  std::vector<std::string>                          effectOptions;
  bool                                              manageFlite;
  bool                                              manageSoX;
  bool                                              mute;
  ACE_Message_Queue_Base*                           queue;
  struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration* spectrumAnalyzerConfiguration;
  Test_I_DirectShow_ISessionNotify_t*               subscriber;
  Test_I_DirectShow_Subscribers_t*                  subscribers;
  std::string                                       voice;
  std::string                                       voiceDirectory;
#if defined (GTK_SUPPORT)
  GdkWindow*                                        window;
#endif // GTK_SUPPORT
};

struct Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration
 : Test_I_MediaFoundation_ModuleHandlerConfiguration
{
  Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration ()
   : Test_I_MediaFoundation_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , effect ()
   , effectOptions ()
   , manageFlite (false)
   , manageSoX (false)
   , mute (false)
   , queue (NULL)
   , spectrumAnalyzerConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , voice ()
   , voiceDirectory ()
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  struct Stream_Device_Identifier                   deviceIdentifier; // render-
  std::string                                       effect;
  std::vector<std::string>                          effectOptions;
  bool                                              manageFlite;
  bool                                              manageSoX;
  bool                                              mute;
  ACE_Message_Queue_Base*                           queue;
  struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration* spectrumAnalyzerConfiguration;
  Test_I_MediaFoundation_ISessionNotify_t*          subscriber;
  Test_I_MediaFoundation_Subscribers_t*             subscribers;
  std::string                                       voice;
  std::string                                       voiceDirectory;
#if defined (GTK_SUPPORT)
  GdkWindow*                                        window;
#endif // GTK_SUPPORT
};
#else
struct Test_I_CommandSpeech_ALSA_ModuleHandlerConfiguration
 : Test_I_ALSA_ModuleHandlerConfiguration
{
  Test_I_CommandSpeech_ALSA_ModuleHandlerConfiguration ()
   : Test_I_ALSA_ModuleHandlerConfiguration ()
   , deviceIdentifier ()
   , effect ()
   , effectOptions ()
   , manageFlite (false)
   , manageSoX (false)
   , mute (false)
   , queue (NULL)
   , spectrumAnalyzerConfiguration (NULL)
   , subscriber (NULL)
   , subscribers (NULL)
   , voice ()
   , voiceDirectory ()
#if defined (GTK_SUPPORT)
   , window (NULL)
#endif // GTK_SUPPORT
  {
    concurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE;
  }

  struct Stream_Device_Identifier                   deviceIdentifier; // render-
  std::string                                       effect;
  std::vector<std::string>                          effectOptions;
  bool                                              manageFlite;
  bool                                              manageSoX;
  bool                                              mute;
  ACE_Message_Queue_Base*                           queue;
  struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration* spectrumAnalyzerConfiguration;
  Test_I_ALSA_ISessionNotify_t*                     subscriber;
  Test_I_ALSA_Subscribers_t*                        subscribers;
  std::string                                       voice;
  std::string                                       voiceDirectory;
#if defined (GTK_SUPPORT)
  GdkWindow*                                        window;
#endif // GTK_SUPPORT
};
#endif // ACE_WIN32 || ACE_WIN64

//extern const char stream_name_string_[];
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_I_DirectShow_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_DirectShow_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , filterGraphConfiguration ()
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
  {
    renderer = STREAM_DEVICE_RENDERER_DIRECTSHOW;
  }

  Stream_MediaFramework_DirectShow_Graph_t filterGraphConfiguration;
  enum Stream_Device_Renderer              renderer;
};

struct Test_I_MediaFoundation_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_MediaFoundation_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
  {
    renderer = STREAM_DEVICE_RENDERER_MEDIAFOUNDATION;
  }

  enum Stream_Device_Renderer renderer;
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_DirectShow_StreamConfiguration,
                               struct Test_I_CommandSpeech_DirectShow_ModuleHandlerConfiguration> Test_I_DirectShow_StreamConfiguration_t;
typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Test_I_CommandSpeech_DirectShow_StreamState> Test_I_DirectShow_IStreamControlBase_t;

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_MediaFoundation_StreamConfiguration,
                               struct Test_I_CommandSpeech_MediaFoundation_ModuleHandlerConfiguration> Test_I_MediaFoundation_StreamConfiguration_t;
typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Test_I_CommandSpeech_MediaFoundation_StreamState> Test_I_MediaFoundation_IStreamControlBase_t;
#else
struct Test_I_ALSA_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_ALSA_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
   , renderer (STREAM_DEVICE_RENDERER_INVALID)
  {
    renderer = STREAM_DEVICE_RENDERER_ALSA;
  }

  enum Stream_Device_Renderer renderer;
};

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_ALSA_StreamConfiguration,
                               struct Test_I_CommandSpeech_ALSA_ModuleHandlerConfiguration> Test_I_ALSA_StreamConfiguration_t;

typedef Stream_IStreamControlBase_T<enum Stream_ControlType,
                                    enum Stream_StateMachine_ControlState,
                                    struct Test_I_CommandSpeech_ALSA_StreamState> Test_I_ALSA_IStreamControlBase_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
