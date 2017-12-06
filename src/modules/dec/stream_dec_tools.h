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

#ifndef STREAM_MODULE_DEC_TOOLS_H
#define STREAM_MODULE_DEC_TOOLS_H

#include <map>
#include <string>

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixfmt.h"
#include "libavutil/pixdesc.h"
}
#endif /* __cplusplus */

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#else
#include "alsa/asoundlib.h"

#include "sox.h"
#endif

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"

#include "stream_dec_common.h"
#include "stream_dec_exports.h"

class Stream_Dec_Export Stream_Module_Decoder_Tools
{
 public:
  static void initialize ();

  static bool isCompressedVideo (enum AVPixelFormat); // pixel format

  static bool isChromaLuminance (enum AVPixelFormat); // pixel format
  static bool isRGB (enum AVPixelFormat); // pixel format

  static std::string errorToString (int); // libav error

  inline static std::string FOURCCToString (ACE_UINT32 fourCC_in) { return std::string (reinterpret_cast<char*> (&fourCC_in), 4); };
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool isChromaLuminance (REFGUID,       // media subtype
                                 bool = false); // ? media foundation : direct show
  static bool isRGB (REFGUID,       // media subtype
                     bool = false); // ? media foundation : direct show

  // *NOTE*: supports non-RGB AND non-Chroma-Luminance types only
  static enum AVCodecID mediaTypeSubTypeToAVCodecID (REFGUID,       // media type subtype
                                                     bool = false); // ? media foundation : direct show
  // *NOTE*: supports RGB and Chroma-Luminance types only
  static enum AVPixelFormat mediaTypeSubTypeToAVPixelFormat (REFGUID,       // media type subtype
                                                             bool = false); // ? media foundation : direct show

  static std::string mediaSubTypeToString (REFGUID,       // media subtype
                                           bool = false); // ? media foundation : direct show
#else
  static enum AVCodecID AVPixelFormatToAVCodecID (enum AVPixelFormat); // pixel format
#endif
  static std::string compressionFormatToString (enum Stream_Decoder_CompressionFormatType);

  // *WARNING*: this may crash if the format is 'unknown'
  inline static std::string pixelFormatToString (enum AVPixelFormat format_in) { std::string result = av_get_pix_fmt_name (format_in); return result; };

  static bool convert (struct SwsContext*, // context ? : use sws_getCachedContext()
                       unsigned int,       // source width
                       unsigned int,       // source height
                       enum AVPixelFormat, // source pixel format
                       uint8_t*[],         // source buffer(s)
                       unsigned int,       // target width
                       unsigned int,       // target height
                       enum AVPixelFormat, // target pixel format
                       uint8_t*[]);        // target buffer(s)
  static bool scale (struct SwsContext*, // context ? : use sws_getCachedContext()
                     unsigned int,       // source width
                     unsigned int,       // source height
                     enum AVPixelFormat, // source pixel format
                     uint8_t*[],         // source buffer(s)
                     unsigned int,       // target width
                     unsigned int,       // target height
                     enum AVPixelFormat, // target pixel format
                     uint8_t*[]);        // target buffer(s)

  // *NOTE*: write a sinus waveform into the target buffer in the specified
  //         audio format
  // *WARNING*: make sure the data buffer contains enough space to hold the
  //            sample data
  static void sinus (double,       // frequency (Hz)
                     unsigned int, // sample rate (Hz)
                     unsigned int, // 'data' sample size (bytes)
                     unsigned int, // #channels
                     char*,        // target buffer
                     unsigned int, // #'data' samples to write
                     double&);     // (return value:) current phase

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  static void ALSAToSoX (enum _snd_pcm_format,       // format
                         sox_rate_t,                 // sample rate
                         unsigned,                   // channels
                         struct sox_encodinginfo_t&, // return value: format
                         struct sox_signalinfo_t&);  // return value: format
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools (const Stream_Module_Decoder_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools& operator= (const Stream_Module_Decoder_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct less_guid
  {
    inline bool operator () (const struct _GUID& lhs_in, const struct _GUID& rhs_in) const { return (lhs_in.Data1 < rhs_in.Data1); }
  };
  typedef std::map<struct _GUID, std::string, less_guid> GUID_TO_STRING_MAP_T;
  typedef GUID_TO_STRING_MAP_T::const_iterator GUID_TO_STRING_MAP_ITERATOR_T;

  static GUID_TO_STRING_MAP_T Stream_DirectShowMediaSubTypeToStringMap;
  static GUID_TO_STRING_MAP_T Stream_MediaFoundationMediaSubTypeToStringMap;
#endif
};

#endif
