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

#ifndef TEST_U_RIFFDECODER_COMMON_H
#define TEST_U_RIFFDECODER_COMMON_H

#include <string>

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_inotify.h"
//#include "stream_messageallocatorheap_base.h"

#include "stream_dec_defines.h"

#include "test_u_common.h"

#include "test_u_riffdecoder_defines.h"

// forward declarations
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;
class Test_U_RIFFDecoder_Message;
class Test_U_RIFFDecoder_SessionMessage;

struct Test_U_RIFFDecoder_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_U_RIFFDecoder_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    defaultBufferSize = TEST_U_RIFFDECODER_DEFAULT_BUFFER_SIZE;

    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    paddingBytes = COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

//extern const char stream_name_string_[];
struct Test_U_RIFFDecoder_ModuleHandlerConfiguration;
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Test_U_RIFFDecoder_AllocatorConfiguration,
                               struct Stream_Configuration,
                               struct Stream_ModuleConfiguration,
                               struct Test_U_RIFFDecoder_ModuleHandlerConfiguration> Test_U_RIFFDecoder_StreamConfiguration_t;
struct Test_U_RIFFDecoder_ModuleHandlerConfiguration
 : Test_U_ModuleHandlerConfiguration
{
  inline Test_U_RIFFDecoder_ModuleHandlerConfiguration ()
   : Test_U_ModuleHandlerConfiguration ()
   , streamConfiguration (NULL)
  {};

  Test_U_RIFFDecoder_StreamConfiguration_t* streamConfiguration;
};

struct Test_U_RIFFDecoder_SessionData
 : Stream_SessionData
{
  inline Test_U_RIFFDecoder_SessionData ()
   : Stream_SessionData ()
   , frameSize (0)
  {};

  unsigned int frameSize;
};
typedef Stream_SessionData_T<struct Test_U_RIFFDecoder_SessionData> Test_U_RIFFDecoder_SessionData_t;

struct Test_U_RIFFDecoder_Configuration
 : Test_U_Configuration
{
  inline Test_U_RIFFDecoder_Configuration ()
   : Test_U_Configuration ()
   , parserConfiguration ()
   , streamConfiguration ()
  {};

  struct Common_ParserConfiguration        parserConfiguration;
  Test_U_RIFFDecoder_StreamConfiguration_t streamConfiguration;
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType,
                                struct Test_U_RIFFDecoder_AllocatorConfiguration> Test_U_RIFFDecoder_ControlMessage_t;

//template <typename AllocatorConfigurationType,
//          typename CommandType,
//          typename ControlMessageType,
//          typename SessionMessageType>
//class Stream_MessageBase_T;
//typedef Stream_MessageBase_T<struct Stream_AllocatorConfiguration,
//                             CommandType,
//                             Test_U_ControlMessage_t,
//                             Test_U_SessionMessage_t> Test_U_Message_t;

//typedef Stream_SessionData_T<Stream_SessionData> Test_U_SessionData_t;
//template <typename AllocatorConfigurationType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename UserDataType,
//          typename ControlMessageType,
//          typename DataMessageType>
//class Stream_SessionMessageBase_T;
//typedef Stream_SessionMessageBase_T<struct Stream_AllocatorConfiguration,
//                                    enum Stream_SessionMessageType,
//                                    Test_U_SessionData_t,
//                                    struct Stream_UserData,
//                                    Test_U_ControlMessage_t,
//                                    Test_U_Message_t> Test_U_SessionMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Test_U_RIFFDecoder_AllocatorConfiguration,
                                          Test_U_ControlMessage_t,
                                          Test_U_RIFFDecoder_Message,
                                          Test_U_RIFFDecoder_SessionMessage> Test_U_RIFFDecoder_MessageAllocator_t;

typedef Stream_INotify_T<enum Stream_SessionMessageType> Test_U_RIFFDecoder_IStreamNotify_t;

#endif
