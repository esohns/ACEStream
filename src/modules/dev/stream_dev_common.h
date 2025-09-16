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

#include <list>
#include <string>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <map>
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}

#include "linux/videodev2.h"

#if defined (LIBPIPEWIRE_SUPPORT)
#include "spa/param/audio/format-utils.h"
#include "spa/param/video/format-utils.h"

#include "pipewire/pipewire.h"
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_lib_directsound_tools.h"
#else
#include "stream_lib_common.h"
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_defines.h"

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_mediafoundation_tools.h"
#else
class ACE_Message_Block;
class ACE_Message_Queue_Base;
class Stream_IAllocator;
struct Stream_Statistic;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef std::map<__u32, ACE_Message_Block*> Stream_Device_BufferMap_t;
typedef Stream_Device_BufferMap_t::const_iterator Stream_Device_BufferMapIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

enum Stream_Device_Capturer
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *** audio ONLY (!) ***
  STREAM_DEVICE_CAPTURER_WAVEIN = 0,
  STREAM_DEVICE_CAPTURER_WASAPI,
  // *** video ONLY (!) ***
  STREAM_DEVICE_CAPTURER_VFW,
  // *** audio/video ***
  STREAM_DEVICE_CAPTURER_DIRECTSHOW,
  STREAM_DEVICE_CAPTURER_MEDIAFOUNDATION,
#else
  // *** audio ONLY (!) ***
  STREAM_DEVICE_CAPTURER_ALSA = 0,
  // *** video ONLY (!) ***
  STREAM_DEVICE_CAPTURER_LIBCAMERA,
  STREAM_DEVICE_CAPTURER_V4L2,
  // *** A/V ***
  STREAM_DEVICE_CAPTURER_PIPEWIRE,
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  STREAM_DEVICE_CAPTURER_MAX,
  STREAM_DEVICE_CAPTURER_INVALID
};

enum Stream_Device_Renderer
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *** audio ONLY (!) ***
  STREAM_DEVICE_RENDERER_WAVEOUT = 0,
  STREAM_DEVICE_RENDERER_WASAPI,
  // *** video ONLY (!) ***
  // *** audio/video ***
  STREAM_DEVICE_RENDERER_DIRECTSHOW,
  STREAM_DEVICE_RENDERER_MEDIAFOUNDATION,
#else
  // *** audio ONLY (!) ***
  STREAM_DEVICE_RENDERER_ALSA = 0,
  // *** video ONLY (!) ***
  STREAM_DEVICE_RENDERER_V4L2,
  // *** A/V ***
  STREAM_DEVICE_RENDERER_PIPEWIRE,
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  STREAM_DEVICE_RENDERER_MAX,
  STREAM_DEVICE_RENDERER_INVALID
};

struct Stream_Device_Identifier
{
  Stream_Device_Identifier ()
   : description ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
   , identifier ()
   , identifierDiscriminator (Stream_Device_Identifier::GUID)
#else
   , fileDescriptor (-1)
   , identifier ()
#endif // ACE_WIN32 || ACE_WIN64
  {}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inline void clear ()
  {
    switch (identifierDiscriminator)
    {
      case ID:
      {
        identifier._id = std::numeric_limits<ULONG>::max ();
        break;
      }
      case GUID:
      {
        identifier._guid = GUID_NULL;
        break;
      }
      case STRING:
      {
        ACE_OS::memset (identifier._string, 0, sizeof (char[BUFSIZ]));
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown discriminator type (was: %d), continuing\n"),
                    identifierDiscriminator));
        break;
      }
    } // end SWITCH
  }
  inline bool empty ()
  {
    switch (identifierDiscriminator)
    {
      case ID:
        return (identifier._id == std::numeric_limits<ULONG>::max ());
      case GUID:
        return InlineIsEqualGUID (identifier._guid, GUID_NULL);
      case STRING:
        return (!ACE_OS::strlen (identifier._string));
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type discriminator (was: %d), aborting\n"),
                    identifierDiscriminator));
        break;
      }
    } // end SWITCH
    return false; // *TODO*: false negative !
  }
  inline operator ULONG () const
  {
    switch (identifierDiscriminator)
    {
      case ID:
        return identifier._id;
      case GUID:
        return Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToWaveDeviceId (identifier._guid);
      case STRING:
        ACE_ASSERT (false); // *TODO*
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type discriminator (was: %d), aborting\n"),
                    identifierDiscriminator));
        break;
      }
    } // end SWITCH
    return -1;
  }
  inline operator struct _GUID () const
  {
    switch (identifierDiscriminator)
    {
      case ID:
        return Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (identifier._id,
                                                                                       true); // *TODO*: this may be wrong!
      case GUID:
        return identifier._guid;
      case STRING:
        return Stream_Device_MediaFoundation_Tools::symbolicLinkToGUID (identifier._string);
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown type discriminator (was: %d), aborting\n"),
                    identifierDiscriminator));
        break;
      }
    } // end SWITCH
    return GUID_NULL;
  }
#else
  inline void clear () { ACE_OS::close (fileDescriptor); fileDescriptor = -1; identifier.clear (); }
  inline bool empty () { return ((fileDescriptor == -1) && identifier.empty ()); }
#endif // ACE_WIN32 || ACE_WIN64

  std::string            description;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  union identifierType
  {
    ULONG        _id;
    struct _GUID _guid;
    char         _string[BUFSIZ];

    identifierType ()
     : _guid (GUID_NULL)
    {}
  }                      identifier;
  enum discriminatorType
  {
    ID = 0,
    GUID,
    STRING,
    INVALID
  };
  enum discriminatorType identifierDiscriminator;
#else
  int                    fileDescriptor;
  std::string            identifier;
#endif // ACE_WIN32 || ACE_WIN64
};
typedef std::list<struct Stream_Device_Identifier> Stream_Device_List_t;
typedef Stream_Device_List_t::const_iterator Stream_Device_ListIterator_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Common_AllocatorConfiguration;
struct Stream_Device_ALSA_Capture_AsynchCBData
{
  Stream_IAllocator*                          allocator;
  struct Common_AllocatorConfiguration*       allocatorConfiguration;
//  struct _snd_pcm_channel_area*               areas;
//  struct Stream_MediaFramework_ALSA_MediaType format;
  unsigned int                                frameSize; // bytesPerSample * format.channels
  ACE_Message_Queue_Base*                     queue;
  Stream_Statistic*                           statistic;
};

struct Stream_Device_ALSA_Playback_AsynchCBData
{
//  struct _snd_pcm_channel_area* areas;
  ACE_Message_Block*      currentBuffer;
  unsigned int            frameSize;
  ACE_Message_Queue_Base* queue;
};

#if defined (LIBPIPEWIRE_SUPPORT)
struct Stream_Device_Pipewire_Capture_CBData
{
  Stream_IAllocator*                          allocator;
  struct Common_AllocatorConfiguration*       allocatorConfiguration;
  struct spa_audio_info                       audioFormat;
  struct spa_video_info                       videoFormat;
  unsigned int                                frameSize; // bytesPerSample * format.channels | video-
  ACE_Message_Queue_Base*                     queue;
  struct Stream_Statistic*                    statistic;
  struct pw_stream*                           stream;
};

struct Stream_Device_Pipewire_Playback_CBData
{
  ACE_Message_Block*      buffer;
  struct spa_audio_info   format;
  unsigned int            frameSize; // bytesPerSample * format.channels
  ACE_Message_Queue_Base* queue;
  struct pw_stream*       stream;
};
#endif // LIBPIPEWIRE_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#endif
