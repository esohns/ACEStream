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

#ifndef STREAM_LIB_TOOLS_H
#define STREAM_LIB_TOOLS_H

#include <string>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <map>

#include <guiddef.h>
#include <mfobjects.h>
#include <strmif.h>
#else
#include "alsa/asoundlib.h"
#endif // ACE_WIN32 || ACE_WIN64

#ifdef __cplusplus
extern "C"
{
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
}
#endif // __cplusplus

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "common_image_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#else
#include "stream_lib_v4l_common.h"

// forward declarations
typedef double sox_rate_t;
struct sox_encodinginfo_t;
struct sox_signalinfo_t;
#endif // ACE_WIN32 || ACE_WIN64

class Stream_MediaFramework_Tools
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  friend class Stream_MediaFramework_DirectShow_Tools;
  friend class Stream_MediaFramework_MediaFoundation_Tools;
#endif // ACE_WIN32 || ACE_WIN64

 public:
  inline static std::string FOURCCToString (ACE_UINT32 fourCC_in) { return std::string (reinterpret_cast<char*> (&fourCC_in), 4); }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool initialize (enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static void finalize (enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static bool isCompressed (REFGUID,                                                              // media subtype
                            REFGUID,                                                              // device category
                            enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static bool isCompressedAudio (REFGUID,                                                              // media subtype
                                 enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isCompressedVideo (REFGUID,                                                              // media subtype
                                 enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static bool isChromaLuminance (REFGUID,                          // media subtype
                                 enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isRGB (REFGUID,                          // media subtype
                     enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  // *NOTE*: as used in struct tagBITMAPINFOHEADER.biBitCount
  static WORD toBitCount (REFGUID,                          // media subtype
                          enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static std::string mediaFormatTypeToString (REFGUID); // media format type
  static std::string mediaSubTypeToString (REFGUID,                          // media subtype
                                           enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static std::string mediaMajorTypeToString (REFGUID, // media major type
                                             enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static unsigned int frameSize (const struct _AMMediaType&);
  static unsigned int frameSize (const IMFMediaType*);

  static struct _GUID AVPixelFormatToMediaSubType (enum AVPixelFormat);
#else
  static bool initialize ();
  static void finalize ();

  // ALSA
  static void ALSAToSoX (enum _snd_pcm_format,       // format
                         sox_rate_t,                 // sample rate
                         unsigned,                   // channels
                         struct sox_encodinginfo_t&, // return value: format
                         struct sox_signalinfo_t&);  // return value: format

  // ffmpeg
  static __u32 ffmpegFormatToV4L2Format (enum AVPixelFormat); // format

  // v4l
  static enum AVPixelFormat v4l2FormatToffmpegFormat (__u32); // format (fourcc)
  static unsigned int toFrameSize (const struct Stream_MediaFramework_V4L_MediaType&);

#if defined (LIBCAMERA_SUPPORT)
  // libCamera
  static libcamera::PixelFormat ffmpegFormatToLibCameraFormat (enum AVPixelFormat); // format
  static enum AVPixelFormat libCameraFormatToffmpegFormat (const libcamera::PixelFormat&); // format
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

  // ffmpeg
  inline static std::string pixelFormatToString (enum AVPixelFormat format_in) { std::string result = ((format_in == AV_PIX_FMT_NONE) ? ACE_TEXT_ALWAYS_CHAR ("") : av_get_pix_fmt_name (format_in)); return result; }

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools (const Stream_MediaFramework_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools& operator= (const Stream_MediaFramework_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_FormatTypeToStringMap;
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap;
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap;
#endif // ACE_WIN32 || ACE_WIN64
};

#endif
