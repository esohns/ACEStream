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

#ifndef STREAM_LIB_FFMPEG_COMMON_H
#define STREAM_LIB_FFMPEG_COMMON_H

#include <deque>
#include <map>

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/hwcontext.h"
#include "libavutil/pixfmt.h"
#include "libavutil/rational.h"
#include "libavutil/samplefmt.h"
} // extern "C"
#endif /* __cplusplus */

#include "ace/Basic_Types.h"

#include "common_image_common.h"

#include "stream_configuration.h"

// *NOTE*: codec 'extra data' information
struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration
{
  ACE_UINT8* data;
  ACE_UINT32 size;
};
typedef std::map<enum AVCodecID, struct Stream_MediaFramework_FFMPEG_SessionData_CodecConfiguration> Stream_MediaFramework_FFMPEG_SessionData_CodecConfigurationMap_t;
typedef Stream_MediaFramework_FFMPEG_SessionData_CodecConfigurationMap_t::const_iterator Stream_MediaFramework_FFMPEG_SessionData_CodecConfigurationMapIterator_t;

//////////////////////////////////////////

struct Stream_MediaFramework_FFMPEG_AllocatorConfiguration
 : Stream_AllocatorConfiguration
{
  Stream_MediaFramework_FFMPEG_AllocatorConfiguration ()
   : Stream_AllocatorConfiguration ()
  {
    paddingBytes = AV_INPUT_BUFFER_PADDING_SIZE;
  }
};

//////////////////////////////////////////

struct Stream_MediaFramework_FFMPEG_FormatNegotiationCBData
{
  Stream_MediaFramework_FFMPEG_FormatNegotiationCBData ()
   : preferredFormat (NULL)
   , negotiatedFormat (NULL)
  {}

  enum AVPixelFormat* preferredFormat;
  enum AVPixelFormat* negotiatedFormat;
};

struct Stream_MediaFramework_FFMPEG_CodecConfiguration
{
  Stream_MediaFramework_FFMPEG_CodecConfiguration ()
   : codecId (AV_CODEC_ID_NONE)
   , deviceType (AV_HWDEVICE_TYPE_NONE)
   , flags (0)
   , flags2 (0)
   , format (AV_PIX_FMT_NONE)
   , padInputBuffers (true)
   , parserFlags (0)
   , profile (FF_PROFILE_UNKNOWN)
   , useParser (true)
  {}

  enum AVCodecID      codecId;         // encoder-/decoder-
  enum AVHWDeviceType deviceType;      // encoder-/decoder-
  int                 flags;           // codec-
  int                 flags2;          // codec-
  enum AVPixelFormat  format;          // preferred output-
  bool                padInputBuffers; // zero AV_INPUT_BUFFER_PADDING_SIZE byte(s) beyound wr_ptr() ?
  int                 parserFlags;     // parser-
  int                 profile;         // codec-
  bool                useParser;       // use av_parser_parse2() to frame chunks, decode packet headers, etc ?
};

//////////////////////////////////////////

struct Stream_MediaFramework_FFMPEG_VideoMediaType
{
  Stream_MediaFramework_FFMPEG_VideoMediaType ()
   : codecId (AV_CODEC_ID_NONE)
   , format (AV_PIX_FMT_NONE)
   , frameRate ()
   , resolution ()
  {
    frameRate.den = 1;
    frameRate.num = 0;
  }

  enum AVCodecID            codecId;
  enum AVPixelFormat        format;
  struct AVRational         frameRate;
  Common_Image_Resolution_t resolution;
};
typedef std::deque<struct Stream_MediaFramework_FFMPEG_VideoMediaType> Stream_MediaFramework_FFMPEG_VideoFormats_t;
typedef Stream_MediaFramework_FFMPEG_VideoFormats_t::iterator Stream_MediaFramework_FFMPEG_VideoFormatsIterator_t;

struct Stream_MediaFramework_FFMPEG_AudioMediaType
{
  Stream_MediaFramework_FFMPEG_AudioMediaType ()
   : codecId (AV_CODEC_ID_NONE)
   , format (AV_SAMPLE_FMT_NONE)
   , channels (0)
   , sampleRate (0)
  {}

  enum AVCodecID      codecId;
  enum AVSampleFormat format;
  unsigned int        channels;
  unsigned int        sampleRate;
};
typedef std::deque<struct Stream_MediaFramework_FFMPEG_AudioMediaType> Stream_MediaFramework_FFMPEG_AudioFormats_t;
typedef Stream_MediaFramework_FFMPEG_AudioFormats_t::iterator Stream_MediaFramework_FFMPEG_AudioFormatsIterator_t;

struct Stream_MediaFramework_FFMPEG_MediaType
{
  Stream_MediaFramework_FFMPEG_MediaType ()
   : audio ()
   , video ()
  {}

  struct Stream_MediaFramework_FFMPEG_AudioMediaType audio;
  struct Stream_MediaFramework_FFMPEG_VideoMediaType video;
};
typedef std::deque<struct Stream_MediaFramework_FFMPEG_MediaType> Stream_MediaFramework_FFMPEG_Formats_t;
typedef Stream_MediaFramework_FFMPEG_Formats_t::iterator Stream_MediaFramework_FFMPEG_FormatsIterator_t;

#endif
