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

#ifndef STREAM_LIB_V4L_COMMON_H
#define STREAM_LIB_V4L_COMMON_H

//#include <deque>
#include <list>
#include <string>
#include <utility>

#include "linux/videodev2.h"

#include "ace/OS.h"

typedef std::list<std::pair<__u32, std::string> > Stream_MediaFramework_V4L_CaptureFormats_t;
typedef Stream_MediaFramework_V4L_CaptureFormats_t::iterator Stream_MediaFramework_V4L_CaptureFormatsIterator_t;

struct Stream_MediaFramework_V4L_MediaType
{
  Stream_MediaFramework_V4L_MediaType ()
   : format ()
   , frameRate ()
  {
    ACE_OS::memset (&format, 0, sizeof (struct v4l2_pix_format));
    ACE_OS::memset (&frameRate, 0, sizeof (struct v4l2_fract));
    frameRate.denominator = 1;
  }

  bool operator== (struct Stream_MediaFramework_V4L_MediaType rhs_in)
  {
    return (((format.pixelformat == rhs_in.format.pixelformat) &&
             (format.width == rhs_in.format.width)             &&
             (format.height == rhs_in.format.height)) &&
            ((frameRate.numerator == rhs_in.frameRate.numerator) &&
             (frameRate.denominator == rhs_in.frameRate.denominator)));
  }

  struct v4l2_pix_format format;
  struct v4l2_fract      frameRate;
};
//typedef std::deque<struct Stream_MediaFramework_V4L_MediaType> Stream_MediaFramework_V4L_Formats_t;
//typedef Stream_MediaFramework_V4L_Formats_t::iterator Stream_MediaFramework_V4L_FormatsIterator_t;

#endif
