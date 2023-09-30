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

#include "mfobjects.h"
#include "strmif.h"
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#include "linux/videodev2.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (LIBCAMERA_SUPPORT)
#include "libcamera/pixel_format.h"
#endif // LIBCAMERA_SUPPORT

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/hwcontext.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"
#include "libavutil/samplefmt.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT

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

  static bool isRGB (REFGUID,                          // media subtype
                     enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isRGB32 (REFGUID,                          // media subtype
                       enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isChromaLuminance (REFGUID,                          // media subtype
                                 enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  // *NOTE*: as used in struct tagBITMAPINFOHEADER.biBitCount
  static WORD toBitCount (REFGUID, // media subtype
                          enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static std::string mediaFormatTypeToString (REFGUID); // media format type
  static std::string mediaSubTypeToString (REFGUID,                          // media subtype
                                           enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  static std::string mediaMajorTypeToString (REFGUID, // media major type
                                             enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);

  static unsigned int frameSize (const struct _AMMediaType&);
  static unsigned int frameSize (const IMFMediaType*);

#if defined (FFMPEG_SUPPORT)
  static WORD AVSampleFormatToFormatTag (enum AVSampleFormat); // format
  // *NOTE*: as used in struct tWAVEFORMATEX.wBitsPerSample
  static WORD AVSampleFormatToBitCount (enum AVSampleFormat); // format
  static struct _GUID AVSampleFormatToMediaSubType (enum AVSampleFormat); // format
  static struct _GUID AVPixelFormatToMediaSubType (enum AVPixelFormat); // format
#endif // FFMPEG_SUPPORT
#else
  static bool initialize ();
  static void finalize ();

  // X11
  static unsigned int v4lFormatToBitDepth (__u32); // format (fourcc)
#if defined (FFMPEG_SUPPORT)
  static unsigned int ffmpegFormatToBitDepth (enum AVPixelFormat); // format
#endif // FFMPEG_SUPPORT

  // v4l
  static bool isRGB (__u32); // format (fourcc)
#if defined (FFMPEG_SUPPORT)
  static enum _snd_pcm_format ffmpegFormatToALSAFormat (enum AVSampleFormat); // format
  static __u32 ffmpegFormatToV4lFormat (enum AVPixelFormat); // format
  static enum AVPixelFormat v4lFormatToffmpegFormat (__u32); // format (fourcc)
  static enum AVCodecID v4lFormatToffmpegCodecId (__u32); // format (fourcc)
#endif // FFMPEG_SUPPORT
  static unsigned int frameSize (const std::string&,                                 // device identifier
                                 const struct Stream_MediaFramework_V4L_MediaType&); // format
  inline static std::string v4lFormatToString (__u32 format_in) { std::string result; result += ((char)(format_in & 0x000000FF)); result += ((char)((format_in >> 8) & 0x000000FF)); result += ((char)((format_in >> 16) & 0x000000FF)); result += ((char)((format_in >> 24) & 0x000000FF)); return result; }

#if defined (LIBCAMERA_SUPPORT)
  // libCamera
#if defined (FFMPEG_SUPPORT)
  static libcamera::PixelFormat ffmpegFormatToLibCameraFormat (enum AVPixelFormat); // format
  static enum AVPixelFormat libCameraFormatToffmpegFormat (const libcamera::PixelFormat&); // format
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
  // ffmpeg
  static bool isAcceleratedFormat (enum AVPixelFormat);
  static int AVPixelFormatToBitCount (enum AVPixelFormat); // format
  static enum AVPixelFormat AVHWDeviceTypeToPixelFormat (enum AVHWDeviceType);
  static enum AVPixelFormat AVHWDeviceTypeToIntermediatePixelFormat (enum AVHWDeviceType);
  static enum AVCodecID ffmpegFormatToffmpegCodecId (enum AVSampleFormat); // format
  inline static std::string pixelFormatToString (enum AVPixelFormat format_in) { std::string result = ((format_in == AV_PIX_FMT_NONE) ? ACE_TEXT_ALWAYS_CHAR ("NONE") : av_get_pix_fmt_name (format_in)); return result; }
  inline static std::string sampleFormatToString (enum AVSampleFormat format_in) { std::string result = ((format_in == AV_SAMPLE_FMT_NONE) ? ACE_TEXT_ALWAYS_CHAR ("NONE") : av_get_sample_fmt_name (format_in)); return result; }
#endif // FFMPEG_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools (const Stream_MediaFramework_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_Tools& operator= (const Stream_MediaFramework_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_FormatTypeToStringMap;
  // *TODO*: move these ASAP
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_DirectShow_MediaSubTypeToStringMap;
  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_MediaFoundation_MediaSubTypeToStringMap;
#endif // ACE_WIN32 || ACE_WIN64
};

#endif
