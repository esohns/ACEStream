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
#include "stream_messageallocatorheap_base.h"

#include "stream_dec_defines.h"

// forward declarations
class Stream_RIFFDecoder_SessionMessage;
class Stream_RIFFDecoder_Message;

struct Test_U_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  inline Test_U_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    // *NOTE*: this facilitates (message block) data buffers to be scanned with
    //         'flex's yy_scan_buffer() method
    buffer = STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE;
  };
};

struct Stream_RIFFDecoder_StreamConfiguration;
struct Stream_RIFFDecoder_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  inline Stream_RIFFDecoder_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , active (false)
   , fileName ()
   , printProgressDot (true)
   , streamConfiguration (NULL)
  {};

  bool                                    active;
  std::string                             fileName;
  bool                                    printProgressDot;
  Stream_RIFFDecoder_StreamConfiguration* streamConfiguration;
};

struct Stream_RIFFDecoder_SessionData
  : Stream_SessionData
{
  inline Stream_RIFFDecoder_SessionData ()
    : Stream_SessionData ()
    , frameSize (0)
  {};

  unsigned int frameSize;
};
typedef Stream_SessionData_T<Stream_RIFFDecoder_SessionData> Stream_RIFFDecoder_SessionData_t;

struct Stream_RIFFDecoder_StreamConfiguration
 : Stream_Configuration
{
  inline Stream_RIFFDecoder_StreamConfiguration ()
   : Stream_Configuration ()
   , moduleHandlerConfiguration (NULL)
  {};

  Stream_RIFFDecoder_ModuleHandlerConfiguration* moduleHandlerConfiguration;
};

struct Stream_RIFFDecoder_Configuration
{
  inline Stream_RIFFDecoder_Configuration ()
   : moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , streamUserData ()
  {};

  Stream_ModuleConfiguration                    moduleConfiguration;
  Stream_RIFFDecoder_ModuleHandlerConfiguration moduleHandlerConfiguration;
  Stream_RIFFDecoder_StreamConfiguration        streamConfiguration;

  Stream_UserData                               streamUserData;
};

typedef Stream_MessageAllocatorHeapBase_T<Test_U_AllocatorConfiguration,

                                          Stream_RIFFDecoder_Message,
                                          Stream_RIFFDecoder_SessionMessage> Stream_RIFFDecoder_MessageAllocator_t;

#endif
