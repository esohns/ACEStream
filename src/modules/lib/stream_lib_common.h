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

#ifndef STREAM_LIB_COMMON_H
#define STREAM_LIB_COMMON_H

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <map>
#include <string>

#include <Guiddef.h>
#endif // ACE_WIN32 || ACE_WIN64

enum Stream_MediaType_Type
{
  STREAM_MEDIATYPE_AUDIO = 0,
  STREAM_MEDIATYPE_VIDEO,
  ////////////////////////////////////////
  STREAM_MEDIATYPE_MAX,
  STREAM_MEDIATYPE_INVALID
};

enum Stream_MediaFramework_Type
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_MEDIAFRAMEWORK_DIRECTSHOW = 0,
  STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION,
#else
  STREAM_MEDIAFRAMEWORK_ALSA = 0,
  STREAM_MEDIAFRAMEWORK_V4L,
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  STREAM_MEDIAFRAMEWORK_MAX,
  STREAM_MEDIAFRAMEWORK_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_MediaFramework_GUID_OperatorLess
{
  inline bool operator () (REFGUID lhs_in, REFGUID rhs_in) const { return (lhs_in.Data1 < rhs_in.Data1); }
};
typedef std::map<struct _GUID,
                 std::string,
                 struct Stream_MediaFramework_GUID_OperatorLess> Stream_MediaFramework_GUIDToStringMap_t;
typedef Stream_MediaFramework_GUIDToStringMap_t::iterator Stream_MediaFramework_GUIDToStringMapIterator_t;
typedef Stream_MediaFramework_GUIDToStringMap_t::const_iterator Stream_MediaFramework_GUIDToStringMapConstIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
