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

#ifndef STREAM_LIB_DIRECTSHOW_COMMON_H
#define STREAM_LIB_DIRECTSHOW_COMMON_H

#include <deque>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "strmif.h"

#include "ace/OS.h"

#include "stream_lib_defines.h"

// forward declarations
class ACE_Message_Queue_Base;
class Stream_IAllocator;

typedef std::deque<struct _AMMediaType> Stream_MediaFramework_DirectShow_Formats_t;
typedef Stream_MediaFramework_DirectShow_Formats_t::iterator Stream_MediaFramework_DirectShow_FormatsIterator_t;

struct Stream_MediaFramework_DirectShow_AudioVideoFormat
{
  Stream_MediaFramework_DirectShow_AudioVideoFormat ()
   : audio ()
   , video ()
  {
    ACE_OS::memset (&audio, 0, sizeof (struct _AMMediaType));
    ACE_OS::memset (&video, 0, sizeof (struct _AMMediaType));
  }

  struct _AMMediaType audio;
  struct _AMMediaType video;
};
typedef std::deque<struct Stream_MediaFramework_DirectShow_AudioVideoFormat> Stream_MediaFramework_DirectShow_AudioVideoFormats_t;
typedef Stream_MediaFramework_DirectShow_AudioVideoFormats_t::iterator Stream_MediaFramework_DirectShow_AudioVideoFormatsIterator_t;

struct Stream_MediaFramework_DirectShow_FilterPinConfiguration
{
  Stream_MediaFramework_DirectShow_FilterPinConfiguration ()
   : allocatorProperties (NULL)
   , buffer (NULL)
   , format (NULL)
   , hasMediaSampleBuffers (false)
   , isTopToBottom (false)
   , queue (NULL)
   , setSampleTimes (false)
  {}

  struct _AllocatorProperties* allocatorProperties;
  ACE_Message_Block*           buffer;
  struct _AMMediaType*         format; // (preferred) media type handle
  bool                         hasMediaSampleBuffers;
  // *NOTE*: some image formats have a bottom-to-top memory layout; in
  //         DirectShow, this is reflected by a positive biHeight; see also:
  //         https://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx
  //         --> set this if the frame data is top-to-bottom
  bool                         isTopToBottom; // video-frame memory layout
  ACE_Message_Queue_Base*      queue; // (inbound) buffer queue handle
  // *NOTE*: if this is 'false', the renderer will output frames ASAP. This is
  //         the correct setting for 'capture' streams. Set this to 'true' for
  //         'playback' streams, where the source may receive the frames faster
  //         than appropriate for playback
  bool                         setSampleTimes;
};

struct Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Stream_MediaFramework_DirectShow_FilterConfiguration ()
   : allocator (NULL)
   , allocatorProperties (NULL)
   , module (NULL)
   , pinConfiguration (NULL)
  {}

  Stream_IAllocator*                                              allocator; // message-
  struct _AllocatorProperties*                                    allocatorProperties; // IMediaSample-
  Stream_Module_t*                                                module;
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration;
};

typedef std::vector<std::wstring> Stream_MediaFramework_DirectShow_Graph_t;
typedef Stream_MediaFramework_DirectShow_Graph_t::iterator Stream_MediaFramework_DirectShow_GraphIterator_t;
typedef Stream_MediaFramework_DirectShow_Graph_t::const_iterator Stream_MediaFramework_DirectShow_GraphConstIterator_t;
struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry
{
  Stream_MediaFramework_DirectShow_GraphConfigurationEntry ()
   : filterName ()
   , mediaType (NULL)
   , connectDirect (false)
  {}

  // *NOTE*: apparently, some filters (e.g. Video Resizer DSP DMO) need to
  //         connect to their downstream peer 'direct'ly
  bool                 connectDirect; // use IGraphBuilder::ConnectDirect() ? : IPin::Connect()
  std::wstring         filterName;
  struct _AMMediaType* mediaType; // ? media type to connect to the downstream
                                  //   input pin with : use input media type
};
typedef std::list<struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry> Stream_MediaFramework_DirectShow_GraphConfiguration_t;
typedef Stream_MediaFramework_DirectShow_GraphConfiguration_t::iterator Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t;
typedef Stream_MediaFramework_DirectShow_GraphConfiguration_t::const_iterator Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t;

typedef std::pair<DWORD, DWORD> Stream_MediaFrameWork_DirectSound_StatisticValue_t;
typedef std::map<enum _AM_AUDIO_RENDERER_STAT_PARAM, Stream_MediaFrameWork_DirectSound_StatisticValue_t> Stream_MediaFrameWork_DirectSound_Statistics_t;
typedef Stream_MediaFrameWork_DirectSound_Statistics_t::const_iterator Stream_MediaFrameWork_DirectSound_StatisticsIterator_t;

#endif
