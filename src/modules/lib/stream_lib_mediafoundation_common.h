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

#include "mfobjects.h"

typedef std::deque<IMFMediaType*> Stream_MediaFramework_MediaFoundation_Formats_t;
typedef Stream_MediaFramework_MediaFoundation_Formats_t::iterator Stream_MediaFramework_MediaFoundation_FormatsIterator_t;

class Stream_IStreamControlBase;
struct IMFMediaSession;
class ACE_Message_Queue_Base;
struct Stream_MediaFramework_MediaFoundation_Configuration
{
  Stream_MediaFramework_MediaFoundation_Configuration ()
   : controller (NULL)
   , mediaSession (NULL)
   , mediaType (NULL)
   , queue (NULL)
  {}

  Stream_IStreamControlBase* controller;
  IMFMediaSession*           mediaSession;
  IMFMediaType*              mediaType; // input-
  ACE_Message_Queue_Base*    queue;
};

#endif
