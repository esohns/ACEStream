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

#ifndef STREAM_LIB_LIBCAMERA_COMMON_H
#define STREAM_LIB_LIBCAMERA_COMMON_H

//#include <deque>
#include <list>
#include <string>
#include <utility>

#include "libcamera/pixel_format.h"
#include "libcamera/formats.h"
#include "libcamera/geometry.h"

#include "ace/Basic_Types.h"

typedef std::list<std::pair<libcamera::PixelFormat, std::string> > Stream_MediaFramework_LibCamera_CaptureFormats_t;
typedef Stream_MediaFramework_LibCamera_CaptureFormats_t::iterator Stream_MediaFramework_LibCamera_CaptureFormatsIterator_t;

struct Stream_MediaFramework_LibCamera_MediaType
{
  Stream_MediaFramework_LibCamera_MediaType ()
   : format ()
   , frameRateNumerator (0)
   , frameRateDenominator (1)
   , resolution ()
  {}

  bool operator== (struct Stream_MediaFramework_LibCamera_MediaType rhs_in)
  {
    return (((format == rhs_in.format)                             &&
             (resolution.width == rhs_in.resolution.width)         &&
             (resolution.height == rhs_in.resolution.height))      &&
            ((frameRateNumerator == rhs_in.frameRateNumerator) &&
             (frameRateDenominator == rhs_in.frameRateDenominator)));
  }

  libcamera::PixelFormat format;
  ACE_UINT32             frameRateNumerator;
  ACE_UINT32             frameRateDenominator;
  libcamera::Size        resolution;
};
//typedef std::deque<struct Stream_MediaFramework_V4L_MediaType> Stream_MediaFramework_V4L_Formats_t;
//typedef Stream_MediaFramework_V4L_Formats_t::iterator Stream_MediaFramework_V4L_FormatsIterator_t;

#endif
