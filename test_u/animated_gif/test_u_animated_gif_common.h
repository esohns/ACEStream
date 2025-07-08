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

#ifndef TEST_U_ANIMATED_GIF_COMMON_H
#define TEST_U_ANIMATED_GIF_COMMON_H

#include <list>
#include <map>
#include <string>

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "common_isubscribe.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
#include "stream_isessionnotify.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

#include "test_u_common.h"

// forward declarations
class Stream_IAllocator;
class Test_U_Stream;
class Test_U_Message;
class Test_U_SessionMessage;

typedef Stream_SessionDataMediaBase_T<struct Test_U_SessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                      struct _AMMediaType,
#else
#if defined (FFMPEG_SUPPORT)
                                      struct Stream_MediaFramework_FFMPEG_VideoMediaType,
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                                      struct Test_U_StreamState,
                                      struct Stream_Statistic,
                                      struct Stream_UserData> Test_U_AnimatedGIF_SessionData;
typedef Stream_SessionData_T<Test_U_AnimatedGIF_SessionData> Test_U_AnimatedGIF_SessionData_t;

typedef Stream_ISessionDataNotify_T<Test_U_AnimatedGIF_SessionData,
                                    enum Stream_SessionMessageType,
                                    Test_U_Message,
                                    Test_U_SessionMessage> Test_U_ISessionNotify_t;
typedef std::list<Test_U_ISessionNotify_t*> Test_U_Subscribers_t;
typedef Test_U_Subscribers_t::iterator Test_U_SubscribersIterator_t;

//extern const char stream_name_string_[];
struct Test_U_AnimatedGIF_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Stream_Configuration,
                               struct Test_U_AnimatedGIF_ModuleHandlerConfiguration> Test_U_StreamConfiguration_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Test_U_AnimatedGIF_ModuleHandlerConfiguration
 : Stream_DirectShow_ModuleHandlerConfiguration
{
  Test_U_AnimatedGIF_ModuleHandlerConfiguration ()
   : Stream_DirectShow_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , fileIdentifier ()
   , printProgressDot (false)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  struct Common_File_Identifier fileIdentifier; // source-/target-
  bool                          printProgressDot;
  Test_U_ISessionNotify_t*      subscriber; // (initial) subscriber
  Test_U_Subscribers_t*         subscribers;
};
#else
struct Test_U_AnimatedGIF_ModuleHandlerConfiguration
 : Stream_V4L_ModuleHandlerConfiguration
{
  Test_U_AnimatedGIF_ModuleHandlerConfiguration ()
   : Stream_V4L_ModuleHandlerConfiguration ()
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , fileIdentifier ()
   , printProgressDot (false)
   , subscriber (NULL)
   , subscribers (NULL)
  {}

#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  struct Common_File_Identifier fileIdentifier; // source-/target-
  bool                          printProgressDot;
  Test_U_ISessionNotify_t*      subscriber; // (initial) subscriber
  Test_U_Subscribers_t*         subscribers;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Test_U_StreamState
 : Stream_State
{
  Test_U_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  Test_U_AnimatedGIF_SessionData* sessionData;
};

struct Test_U_AnimatedGIF_Configuration
 : Test_U_Configuration
{
  Test_U_AnimatedGIF_Configuration ()
   : Test_U_Configuration ()
   , signalHandlerConfiguration ()
   , streamConfiguration ()
  {}

  struct Test_U_SignalHandlerConfiguration signalHandlerConfiguration;
  Test_U_StreamConfiguration_t             streamConfiguration;
};

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_U_Message,
                                          Test_U_SessionMessage> Test_U_MessageAllocator_t;
typedef Common_ISubscribe_T<Test_U_ISessionNotify_t> Test_U_ISubscribe_t;

//////////////////////////////////////////

struct Test_U_AnimatedGIF_ProgressData
 : Test_U_ProgressData
{
  Test_U_AnimatedGIF_ProgressData ()
   : Test_U_ProgressData ()
   , copied (0)
   , size (0)
  {}

  size_t copied; // bytes
  size_t size; // bytes
};

struct Test_U_AnimatedGIF_UI_CBData
 : Test_U_UI_CBData
{
  Test_U_AnimatedGIF_UI_CBData ()
   : Test_U_UI_CBData ()
   , configuration (NULL)
   , progressData ()
   , stream (NULL)
   , subscribers ()
  {}

  struct Test_U_AnimatedGIF_Configuration* configuration;
  struct Test_U_AnimatedGIF_ProgressData   progressData;
  Test_U_Stream*                           stream;
  Test_U_Subscribers_t                     subscribers;
};

struct Test_U_AnimatedGIF_ThreadData
 : Test_U_ThreadData
{
  Test_U_AnimatedGIF_ThreadData ()
   : Test_U_ThreadData ()
  {}
};

#endif
