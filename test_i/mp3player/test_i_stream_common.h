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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include <list>
#include <map>
#include <string>

//#include "libxml/tree.h"

#include "ace/config-macros.h"
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

#include "test_i_configuration.h"

// forward declarations
class Stream_IAllocator;
class Test_I_Stream_Message;
class Test_I_Stream_SessionMessage;

typedef int Stream_HeaderType_t;
typedef int Stream_CommandType_t;

//struct Test_I_HTTPGet_ConnectionState;
struct Test_I_MP3Player_SessionData
 : Stream_SessionDataMediaBase_T<struct Stream_SessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                 struct _AMMediaType,
#else
                                 struct Stream_MediaFramework_ALSA_MediaType,
#endif // ACE_WIN32 || ACE_WIN64
                                 struct Test_I_MP3Player_StreamState,
                                 struct Stream_Statistic,
                                 struct Stream_UserData>
{
  Test_I_MP3Player_SessionData ()
   : Stream_SessionDataMediaBase_T<struct Stream_SessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   struct _AMMediaType,
#else
                                   struct Stream_MediaFramework_ALSA_MediaType,
#endif // ACE_WIN32 || ACE_WIN64
                                   struct Test_I_MP3Player_StreamState,
                                   struct Stream_Statistic,
                                   struct Stream_UserData> ()
   , targetFileName ()
  {}

  struct Test_I_MP3Player_SessionData& operator+= (const struct Test_I_MP3Player_SessionData& rhs_in)
  {
    // *NOTE*: the idea is to 'merge' the data
    Stream_SessionDataMediaBase_T<struct Stream_SessionData,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                  struct _AMMediaType,
#else
                                  struct Stream_MediaFramework_ALSA_MediaType,
#endif // ACE_WIN32 || ACE_WIN64
                                  struct Test_I_MP3Player_StreamState,
                                  struct Stream_Statistic,
                                  struct Stream_UserData>::operator+= (rhs_in);

    //data += rhs_in.data;
    ////format =
    //parserContext = (parserContext ? parserContext : rhs_in.parserContext);
    targetFileName = (targetFileName.empty () ? rhs_in.targetFileName
                                              : targetFileName);

    return *this;
  }

  //struct Test_I_DataSet                     data; // html handler module
  //enum Stream_Decoder_CompressionFormatType format; // decompressor module
  //struct Test_I_SAXParserContext*           parserContext; // html parser/handler module
  std::string                               targetFileName; // file writer module
};
typedef Stream_SessionData_T<struct Test_I_MP3Player_SessionData> Test_I_MP3Player_SessionData_t;

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
                      struct Test_I_MP3Player_SessionData,
                      Test_I_MP3Player_SessionData_t,
                      Stream_ControlMessage_t,
                      Test_I_Stream_Message,
                      Test_I_Stream_SessionMessage> Test_I_StreamBase_t;
struct Test_I_MP3Player_ModuleHandlerConfiguration
 : Test_I_ModuleHandlerConfiguration
{
  Test_I_MP3Player_ModuleHandlerConfiguration ()
   : Test_I_ModuleHandlerConfiguration ()
   , pushStatisticMessages (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
   , asynchPlayback (false)
   , deviceIdentifier ()
   , playbackDeviceHandle (NULL)
#endif // ACE_WIN32 || ACE_WIN64
  {}

  bool pushStatisticMessages;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  bool             asynchPlayback;
  std::string      deviceIdentifier;
  struct _snd_pcm* playbackDeviceHandle;
#endif // ACE_WIN32 || ACE_WIN64
};

struct Test_I_MP3Player_StreamConfiguration
 : Test_I_StreamConfiguration
{
  Test_I_MP3Player_StreamConfiguration ()
   : Test_I_StreamConfiguration ()
  {}
};

struct Test_I_MP3Player_StreamState
 : Test_I_StreamState
{
  Test_I_MP3Player_StreamState ()
   : Test_I_StreamState ()
   , sessionData (NULL)
  {}

  struct Test_I_MP3Player_SessionData* sessionData;
};

//typedef Stream_IModuleHandler_T<Test_I_Stream_ModuleHandlerConfiguration> Test_I_IModuleHandler_t;
typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Common_AllocatorConfiguration,
                                          Stream_ControlMessage_t,
                                          Test_I_Stream_Message,
                                          Test_I_Stream_SessionMessage> Test_I_MessageAllocator_t;

//typedef Stream_INotify_T<enum Stream_SessionMessageType> Stream_IStreamNotify_t;

#endif
