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

#ifndef STREAM_LIB_MEDIAFOUNDATION_COMMON_H
#define STREAM_LIB_MEDIAFOUNDATION_COMMON_H

#include <deque>
#include <list>

#undef GetObject
#include "mfidl.h"
#include "mfobjects.h"

// forward declarations
class Stream_IStreamControlBase;
class ACE_Message_Queue_Base;

typedef std::deque<IMFMediaType*> Stream_MediaFramework_MediaFoundation_Formats_t;
typedef Stream_MediaFramework_MediaFoundation_Formats_t::iterator Stream_MediaFramework_MediaFoundation_FormatsIterator_t;

struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat
{
  Stream_MediaFramework_MediaFoundation_AudioVideoFormat ()
   : audio (NULL)
   , video (NULL)
  {}

  IMFMediaType* audio;
  IMFMediaType* video;
};
typedef std::deque<struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat> Stream_MediaFramework_MediaFoundation_AudioVideoFormats_t;
typedef Stream_MediaFramework_MediaFoundation_AudioVideoFormats_t::iterator Stream_MediaFramework_MediaFoundation_AudioVideoFormatsIterator_t;

struct Stream_MediaFramework_MediaFoundation_Configuration
{
  Stream_MediaFramework_MediaFoundation_Configuration ()
   : controller (NULL)
   , mediaEventGenerator (NULL)
   , mediaSession (NULL)
   , mediaType (NULL)
   , queue (NULL)
  {}

  Stream_IStreamControlBase* controller;
  IMFMediaEventGenerator*    mediaEventGenerator; // media session-
  IMFMediaSession*           mediaSession;
  IMFMediaType*              mediaType; // input-
  ACE_Message_Queue_Base*    queue;
};

typedef std::list<IMFTopologyNode*> TOPOLOGY_PATH_T;
typedef TOPOLOGY_PATH_T::iterator TOPOLOGY_PATH_ITERATOR_T;
typedef std::list<TOPOLOGY_PATH_T> TOPOLOGY_PATHS_T;
typedef TOPOLOGY_PATHS_T::iterator TOPOLOGY_PATHS_ITERATOR_T;

#endif
