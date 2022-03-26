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

#include "guiddef.h"
#else
#include <deque>
#endif // ACE_WIN32 || ACE_WIN64

#include <set>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#include "stream_lib_v4l_common.h"
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
#if defined (LIBCAMERA_SUPPORT)
  STREAM_MEDIAFRAMEWORK_LIBCAMERA,
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  STREAM_MEDIAFRAMEWORK_MAX,
  STREAM_MEDIAFRAMEWORK_INVALID
};

enum Stream_MediaFramework_SoundGeneratorType
{
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH = 0,
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE,
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE,
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_TRIANGLE,
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE,
  ////////////////////////////////////////
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_MAX,
  STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_INVALID
};

struct Stream_MediaFramework_SoundGeneratorConfiguration
{
  // media type
  unsigned int                                  samplesPerSecond;
  unsigned int                                  bytesPerSample; // #bytes/(mono-)-
  unsigned int                                  numberOfChannels;
  bool                                          isFloatFormat; // i.e. IEEE-
  bool                                          isLittleEndianFormat;
  bool                                          isSignedFormat;

  // waveform generators
  double                                        amplitude; // [0.0-1.0]
  double                                        frequency;

  enum Stream_MediaFramework_SoundGeneratorType type;
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

typedef std::set<uint8_t> Stream_MediaFramework_Sound_SampleResolutions_t;
typedef Stream_MediaFramework_Sound_SampleResolutions_t::const_iterator Stream_MediaFramework_Sound_SampleResolutionsIterator_t;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Stream_MediaFramework_ALSA_V4L_Format
{
  Stream_MediaFramework_ALSA_V4L_Format ()
   : audio ()
   , video ()
  {}

  struct Stream_MediaFramework_ALSA_MediaType audio;
  struct Stream_MediaFramework_V4L_MediaType video;
};
typedef std::deque<struct Stream_MediaFramework_ALSA_V4L_Format> Stream_MediaFramework_ALSA_V4L_Formats_t;
typedef Stream_MediaFramework_ALSA_V4L_Formats_t::iterator Stream_MediaFramework_ALSA_V4L_FormatsIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
