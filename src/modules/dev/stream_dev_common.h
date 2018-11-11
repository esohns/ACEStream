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

#ifndef STREAM_DEVICE_COMMON_H
#define STREAM_DEVICE_COMMON_H

#include "ace/config-lite.h"

#include <list>
#include <string>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <map>
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
extern "C"
{
#include "alsa/asoundlib.h"
}
#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_IAllocator;
struct Stream_Statistic;
#endif // ACE_WIN32 || ACE_WIN64

typedef std::list<std::string> Stream_Device_List_t;
typedef Stream_Device_List_t::const_iterator Stream_Device_ListIterator_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef std::map<__u32, ACE_Message_Block*> Stream_Device_BufferMap_t;
typedef Stream_Device_BufferMap_t::const_iterator Stream_Device_BufferMapIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

enum Stream_Device_Mode
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_DEVICE_MODE_DIRECTSHOW = 0,
  STREAM_DEVICE_MODE_MEDIAFOUNDATION,
#else
  // *** audio ONLY (!) ***
  STREAM_DEVICE_MODE_ALSA = 0,
  // *** video ONLY (!) ***
  STREAM_DEVICE_MODE_V4L2,
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  STREAM_DEVICE_MODE_MAX,
  STREAM_DEVICE_MODE_INVALID
};

struct Stream_Device_Identifier
{
  Stream_Device_Identifier ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   : identifier ()
   , identifierDiscriminator (Stream_Device_Identifier::GUID)
#else
   : fileDescriptor (-1)
   , identifier ()
#endif // ACE_WIN32 || ACE_WIN64
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  union identifierType
  {
    struct _GUID _guid;
    char         _string[255];

    identifierType ()
     : _guid (GUID_NULL)
    {}
  }                      identifier;
  enum discriminatorType
  {
    GUID = 0,
    STRING,
    INVALID
  };
  enum discriminatorType identifierDiscriminator;

  void clear ()
  {
    switch (identifierDiscriminator)
    {
      case GUID:
      {
        identifier._guid = GUID_NULL;
        break;
      }
      case STRING:
      {
        ACE_OS::memset (identifier._string, 0, sizeof (char[255]));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type (was: %d), continuing\n"),
                    identifierDiscriminator));
        break;
      }
    } // end SWITCH
  }
#else
  int                    fileDescriptor;
  std::string            identifier;
#endif // ACE_WIN32 || ACE_WIN64
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Stream_Device_ALSAConfiguration
{
  struct Stream_MediaFramework_ALSA_MediaType format;
};

struct Stream_Device_ALSA_Capture_AsynchCBData
{
  Stream_IAllocator*            allocator;
  Stream_Statistic*             statistic;

  //  struct _snd_pcm_channel_area* areas;
  unsigned int                  bufferSize;
  unsigned int                  channels;
  enum _snd_pcm_format          format;
  ACE_Message_Queue_Base*       queue;
  unsigned int                  sampleRate;
  unsigned int                  sampleSize;

  double*                       frequency;
  bool                          sinus;
  double                        phase;
};

struct Stream_Device_ALSA_Playback_AsynchCBData
{
//  struct _snd_pcm_channel_area* areas;
  ACE_Message_Block*      currentBuffer;
  ACE_Message_Queue_Base* queue;
  unsigned int            sampleSize;
};
#endif // ACE_WIN32 || ACE_WIN64

#endif
