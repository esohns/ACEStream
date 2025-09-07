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
#include <map>
#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "common_file_common.h"
#include "common_istatistic.h"
#include "common_isubscribe.h"
#include "common_time_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_data.h"

#include "stream_dec_common.h"
#include "stream_dec_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

#include "stream_dev_common.h"

#include "test_i_configuration.h"
#include "test_i_modules.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

// forward declarations
//struct Test_I_HTTPGet_Configuration;
//struct Test_I_SocketHandlerConfiguration;
extern const char stream_name_string_[];
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_I_MP3Player_StreamConfiguration,
                               struct Test_I_MP3Player_ModuleHandlerConfiguration> Test_I_StreamConfiguration_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Test_I_MP3Player_StreamState,
                      struct Test_I_MP3Player_StreamConfiguration,
                      struct Stream_Statistic,
                      struct Test_I_MP3Player_ModuleHandlerConfiguration,
                      Test_I_SessionManager_t,
                      Stream_ControlMessage_t,
                      Test_I_Stream_Message,
                      Test_I_Stream_SessionMessage> Test_I_StreamBase_t;
struct Test_I_MP3Player_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_MP3Player_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , ALSAConfiguration (NULL)
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
   , codecConfiguration (NULL)
#endif // FFMPEG_SUPPORT
   , deviceIdentifier ()
   , outputFormat ()
   , pushStatisticMessages (true)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_OS::memset (&outputFormat, 0, sizeof (struct _AMMediaType));
#endif // ACE_WIN32 || ACE_WIN64
  }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Stream_MediaFramework_ALSA_Configuration* ALSAConfiguration;
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_CodecConfiguration* codecConfiguration;
#endif // FFMPEG_SUPPORT
  struct Stream_Device_Identifier                  deviceIdentifier;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType                              outputFormat;
#else
  struct Stream_MediaFramework_ALSA_MediaType      outputFormat;
#endif // ACE_WIN32 || ACE_WIN64
  bool                                             pushStatisticMessages;
};

struct Test_I_MP3Player_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_MP3Player_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
  {}
};

struct Test_I_MP3Player_StreamState
 : Stream_State
{
  Test_I_MP3Player_StreamState ()
   : Stream_State ()
   , sessionData (NULL)
  {}

  struct Test_I_MP3Player_SessionData* sessionData;
};

//typedef Stream_IModuleHandler_T<Test_I_Stream_ModuleHandlerConfiguration> Test_I_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
#if defined (FFMPEG_SUPPORT)
                                          struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration,
#else
                                          struct Stream_AllocatorConfiguration,
#endif // FFMPEG_SUPPORT
                                          Stream_ControlMessage_t,
                                          Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

//typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_IStreamNotify_t;

#endif
