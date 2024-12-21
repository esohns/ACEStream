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
#include "stdafx.h"

#include "stream_dec_tools.h"

#include <cmath>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "audiosessiontypes.h"
#include "winnt.h"
#include "guiddef.h"
#include "amvideo.h"
#include "dmodshow.h"
#include "dmoreg.h"
// *WARNING*: "...Note Header files ksproxy.h and dsound.h define similar but
//            incompatible versions of the IKsPropertySet interface.
//            Applications that require the KS proxy module should use the
//            version defined in ksproxy.h.The DirectSound version of
//            IKsPropertySet is described in the DirectSound reference pages in
//            the Microsoft Windows SDK documentation.
//            If an application must include both ksproxy.h and dsound.h,
//            whichever header file the compiler scans first is the one whose
//            definition of IKsPropertySet is used by the compiler. ..."
#include "ks.h"
#include "ksproxy.h"
#include "mmsystem.h"
#define INITGUID
#include "dsound.h"
#include "dvdmedia.h"
#include "mediaobj.h"
#include "mfapi.h"
#include "Mferror.h"
#undef GetObject
#include "mfidl.h"
#include "qedit.h"
#include "strmif.h"
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#include "vfwmsgs.h"
#include "wmcodecdsp.h"
//#include "d3d9.h"
//#include "vmr9.h"

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
#include "fourcc.h"
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"

#include "libavutil/avutil.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/log.h"
#include "libavutil/pixfmt.h"

#include "libswscale/swscale.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT

#if defined (OPENCV_SUPPORT)
#include "opencv2/core/hal/interface.h"
#endif // OPENCV_SUPPORT

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_os_tools.h"
#include "common_string_tools.h"

#include "common_error_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_tools.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_common.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_common.h"
#include "stream_lib_directsound_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_defines.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_defines.h"

#if defined (FFMPEG_SUPPORT)
void
stream_decoder_libav_log_cb (void* AVClassStruct_in,
                             int level_in,
                             const char* formatString_in,
                             va_list arguments_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_decoder_libav_log_cb"));

  ACE_UNUSED_ARG (AVClassStruct_in);

  char buffer_a[BUFSIZ];
  int print_prefix = 1;

  av_log_format_line (AVClassStruct_in,
                      level_in,
                      formatString_in,
                      arguments_in,
                      buffer_a,
                      sizeof (char[BUFSIZ]),
                      &print_prefix);

  enum ACE_Log_Priority log_priority_e = LM_DEBUG;
  switch (level_in)
  {
    case AV_LOG_PANIC:
      log_priority_e = LM_EMERGENCY; break;
    case AV_LOG_FATAL:
      log_priority_e = LM_CRITICAL; break;
    case AV_LOG_ERROR:
      log_priority_e = LM_ERROR; break;
    case AV_LOG_WARNING:
      log_priority_e = LM_WARNING; break;
    case AV_LOG_INFO:
    case AV_LOG_VERBOSE:
      log_priority_e = LM_INFO; break;
    case AV_LOG_DEBUG:
      log_priority_e = LM_DEBUG; break;
    case AV_LOG_TRACE:
      log_priority_e = LM_TRACE; break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ffmpeg loglevel (was: %d), continuing\n"),
                  level_in));
      break;
    }
  } // end SWITCH
  ACE_DEBUG ((log_priority_e,
              ACE_TEXT ("%s"),
              ACE_TEXT (buffer_a)));
}
#endif // FFMPEG_SUPPORT

ACE_Date_Time
Stream_Module_Decoder_Tools::mpeg4ToDateTime (ACE_UINT64 secondsSinceMPEG4Epoch_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mpeg4ToDateTime"));

  struct tm tm_s, *tm_p = NULL;
  ACE_OS::memset (&tm_s, 0, sizeof (struct tm));
  tm_s.tm_mday = 1;
  tm_s.tm_year = 4;
  time_t time_i = ACE_OS::mktime (&tm_s) + secondsSinceMPEG4Epoch_in;
  tm_p = ACE_OS::localtime_r (&time_i, &tm_s);
  ACE_ASSERT (tm_p);
  ACE_UNUSED_ARG (tm_p);

  ACE_Date_Time result (tm_s.tm_mday,
                        tm_s.tm_mon + 1,
                        tm_s.tm_year + 1900,
                        tm_s.tm_hour,
                        tm_s.tm_min,
                        tm_s.tm_sec,
                        0,
                        tm_s.tm_wday);

  return result;
}

enum Stream_MediaType_Type
Stream_Module_Decoder_Tools::streamIdToMediaType (unsigned short streamId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::streamIdToMediaType"));

  switch (streamId_in)
  {
    case 3:   // audio_stream_descriptor
    case 15:  // private_data_indicator_descriptor
    case 28:  // MPEG-4_audio_descriptor
    case 43:  // MPEG-2_AAC_audio_descriptor
    case 46:  // MPEG-4_audio_extension_descriptor
      return STREAM_MEDIATYPE_AUDIO;
    case 2:   // video_stream_descriptor
    case 27:  // MPEG-4_video_descriptor
    case 40:  // AVC video descriptor
    case 50:  // J2K video descriptor
    case 52:  // MPEG2_stereoscopic_video_format_descriptor
    case 56:  // HEVC video descriptor
      return STREAM_MEDIATYPE_VIDEO;
    default:
      break;
  } // end SWITCH

  return STREAM_MEDIATYPE_INVALID;
}

#if defined (FFMPEG_SUPPORT)
bool
Stream_Module_Decoder_Tools::isPackedIntegerPCM (enum AVSampleFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isPackedIntegerPCM"));

  switch (format_in)
  {
    case AV_SAMPLE_FMT_U8:
    case AV_SAMPLE_FMT_S16:
    case AV_SAMPLE_FMT_S32:
    case AV_SAMPLE_FMT_S64:
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}

bool
Stream_Module_Decoder_Tools::isPackedRealPCM (enum AVSampleFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isPackedRealPCM"));

  switch (format_in)
  {
    case AV_SAMPLE_FMT_FLT:
    case AV_SAMPLE_FMT_DBL:
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}

uint64_t
Stream_Module_Decoder_Tools::channelsToMask (unsigned int channels_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::channelsToMask"));

  switch (channels_in)
  {
    case 1:
      return AV_CH_LAYOUT_MONO;
    case 2:
      return AV_CH_LAYOUT_STEREO;
    case 3:
      return AV_CH_LAYOUT_2_1;
    case 4:
      return AV_CH_LAYOUT_QUAD;
    case 5:
      return AV_CH_LAYOUT_5POINT0;
    case 6:
      return AV_CH_LAYOUT_6POINT0;
    case 7:
      return AV_CH_LAYOUT_7POINT0;
    case 8:
      return AV_CH_LAYOUT_OCTAGONAL;
    case 16:
      return AV_CH_LAYOUT_HEXADECAGONAL;
    default:
      break;
  } // end SWITCH

  return AV_CH_LAYOUT_STEREO_DOWNMIX;
}

bool
Stream_Module_Decoder_Tools::isChromaLuminance (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isChromaLuminance"));

  switch (format_in)
  {
    case AV_PIX_FMT_YUV420P:   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    case AV_PIX_FMT_YUYV422:   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    case AV_PIX_FMT_YUV422P:   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    case AV_PIX_FMT_YUV444P:   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    case AV_PIX_FMT_YUV410P:   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    case AV_PIX_FMT_YUV411P:   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    case AV_PIX_FMT_YUVJ420P:  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
    case AV_PIX_FMT_YUVJ422P:  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
    case AV_PIX_FMT_YUVJ444P:  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
    case AV_PIX_FMT_UYVY422:   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    case AV_PIX_FMT_UYYVYY411: ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    case AV_PIX_FMT_NV12:      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    case AV_PIX_FMT_NV21:      ///< as above, but U and V bytes are swapped
    case AV_PIX_FMT_YUV440P:   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    case AV_PIX_FMT_YUVJ440P:  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV440P and setting color_range
    case AV_PIX_FMT_YUVA420P:  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    case AV_PIX_FMT_YUV420P16LE:  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV420P16BE:  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV422P16LE:  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV422P16BE:  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P16LE:  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P16BE:  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV420P9BE: ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P9LE: ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV420P10BE:///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P10LE:///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV422P10BE:///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P10LE:///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P9BE: ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P9LE: ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P10BE:///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P10LE:///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV422P9BE: ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P9LE: ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUVA422P:  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    case AV_PIX_FMT_YUVA444P:  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    case AV_PIX_FMT_YUVA420P9BE:  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    case AV_PIX_FMT_YUVA420P9LE:  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    case AV_PIX_FMT_YUVA422P9BE:  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    case AV_PIX_FMT_YUVA422P9LE:  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    case AV_PIX_FMT_YUVA444P9BE:  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    case AV_PIX_FMT_YUVA444P9LE:  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    case AV_PIX_FMT_YUVA420P10BE: ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA420P10LE: ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA422P10BE: ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA422P10LE: ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA444P10BE: ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA444P10LE: ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA420P16BE: ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA420P16LE: ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA422P16BE: ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA422P16LE: ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    case AV_PIX_FMT_YUVA444P16BE: ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    case AV_PIX_FMT_YUVA444P16LE: ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    case AV_PIX_FMT_NV16:         ///< interleaved chroma YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    case AV_PIX_FMT_NV20LE:       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_NV20BE:       ///< interleaved chroma YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YVYU422:   ///< packed YUV 4:2:2, 16bpp, Y0 Cr Y1 Cb
    case AV_PIX_FMT_YUV420P12BE: ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P12LE: ///< planar YUV 4:2:0,18bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV420P14BE: ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    case AV_PIX_FMT_YUV420P14LE: ///< planar YUV 4:2:0,21bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    case AV_PIX_FMT_YUV422P12BE: ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P12LE: ///< planar YUV 4:2:2,24bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV422P14BE: ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    case AV_PIX_FMT_YUV422P14LE: ///< planar YUV 4:2:2,28bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P12BE: ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P12LE: ///< planar YUV 4:4:4,36bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUV444P14BE: ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    case AV_PIX_FMT_YUV444P14LE: ///< planar YUV 4:4:4,42bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    case AV_PIX_FMT_YUVJ411P:    ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples) full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV411P and setting color_range
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    case AV_PIX_FMT_YUV440P10LE: ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    case AV_PIX_FMT_YUV440P10BE: ///< planar YUV 4:4:0,20bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    case AV_PIX_FMT_YUV440P12LE: ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), little-endian
    case AV_PIX_FMT_YUV440P12BE: ///< planar YUV 4:4:0,24bpp, (1 Cr & Cb sample per 1x2 Y samples), big-endian
    case AV_PIX_FMT_AYUV64LE:    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    case AV_PIX_FMT_AYUV64BE:    ///< packed AYUV 4:4:4,64bpp (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    case AV_PIX_FMT_P010LE: ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, little-endian
    case AV_PIX_FMT_P010BE: ///< like NV12, with 10bpp per component, data in the high bits, zeros in the low bits, big-endian
    case AV_PIX_FMT_P016LE: ///< like NV12, with 16bpp per component, little-endian
    case AV_PIX_FMT_P016BE: ///< like NV12, with 16bpp per component, big-endian
#endif // ACE_WIN32 || ACE_WIN64
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}

bool
Stream_Module_Decoder_Tools::isRGB (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isRGB"));

  switch (format_in)
  {
    case AV_PIX_FMT_RGB24:     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    case AV_PIX_FMT_BGR24:     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    case AV_PIX_FMT_BGR8:      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    case AV_PIX_FMT_BGR4:      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    case AV_PIX_FMT_BGR4_BYTE: ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    case AV_PIX_FMT_RGB8:      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    case AV_PIX_FMT_RGB4:      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    case AV_PIX_FMT_RGB4_BYTE: ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    case AV_PIX_FMT_ARGB:      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    case AV_PIX_FMT_RGBA:      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    case AV_PIX_FMT_ABGR:      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    case AV_PIX_FMT_BGRA:      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
    case AV_PIX_FMT_RGB48BE:   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    case AV_PIX_FMT_RGB48LE:   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian
    case AV_PIX_FMT_RGB565BE:  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    case AV_PIX_FMT_RGB565LE:  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    case AV_PIX_FMT_RGB555BE:  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), big-endian   , X=unused/undefined
    case AV_PIX_FMT_RGB555LE:  ///< packed RGB 5:5:5, 16bpp, (msb)1X 5R 5G 5B(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_BGR565BE:  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    case AV_PIX_FMT_BGR565LE:  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    case AV_PIX_FMT_BGR555BE:  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), big-endian   , X=unused/undefined
    case AV_PIX_FMT_BGR555LE:  ///< packed BGR 5:5:5, 16bpp, (msb)1X 5B 5G 5R(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_RGB444LE:  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_RGB444BE:  ///< packed RGB 4:4:4, 16bpp, (msb)4X 4R 4G 4B(lsb), big-endian,    X=unused/undefined
    case AV_PIX_FMT_BGR444LE:  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), little-endian, X=unused/undefined
    case AV_PIX_FMT_BGR444BE:  ///< packed BGR 4:4:4, 16bpp, (msb)4X 4B 4G 4R(lsb), big-endian,    X=unused/undefined
    case AV_PIX_FMT_BGR48BE:   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    case AV_PIX_FMT_BGR48LE:   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian
    case AV_PIX_FMT_RGBA64BE:  ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    case AV_PIX_FMT_RGBA64LE:  ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    case AV_PIX_FMT_BGRA64BE:  ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    case AV_PIX_FMT_BGRA64LE:  ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    case AV_PIX_FMT_0RGB:      ///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    case AV_PIX_FMT_RGB0:      ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    case AV_PIX_FMT_0BGR:      ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    case AV_PIX_FMT_BGR0:      ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}

bool
Stream_Module_Decoder_Tools::isRGB32 (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isRGB32"));

  switch (format_in)
  {
    case AV_PIX_FMT_ARGB: ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    case AV_PIX_FMT_RGBA: ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    case AV_PIX_FMT_ABGR: ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    case AV_PIX_FMT_BGRA: ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (FFMPEG_SUPPORT)
enum AVSampleFormat
Stream_Module_Decoder_Tools::to (const struct tWAVEFORMATEX& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::to"));

  WORD format_tag = format_in.wFormatTag;

do_it:
  switch (format_tag)
  {
    case WAVE_FORMAT_PCM:
    {
      switch (format_in.wBitsPerSample)
      {
        case 8:
          return AV_SAMPLE_FMT_U8;
        case 16:
          return AV_SAMPLE_FMT_S16;
        case 32:
          return AV_SAMPLE_FMT_S32;
        case 64:
          return AV_SAMPLE_FMT_S64;
        default:
          break;
      }
      break;
    }
    case WAVE_FORMAT_IEEE_FLOAT:
    {
      switch (format_in.wBitsPerSample)
      {
        case 32:
          return AV_SAMPLE_FMT_FLT;
        case 64:
          return AV_SAMPLE_FMT_DBL;
        default:
          break;
      }
      break;
    }
    case WAVE_FORMAT_EXTENSIBLE:
    {
      const WAVEFORMATEXTENSIBLE* waveformatextensible_p =
        reinterpret_cast<const WAVEFORMATEXTENSIBLE*> (&format_in);
      if (InlineIsEqualGUID (waveformatextensible_p->SubFormat,
                             KSDATAFORMAT_SUBTYPE_PCM))
        format_tag = WAVE_FORMAT_PCM;
      else if (InlineIsEqualGUID (waveformatextensible_p->SubFormat,
                                  KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
        format_tag = WAVE_FORMAT_IEEE_FLOAT;
      else
        format_tag = 0;
      goto do_it;
    }
    default:
      break;
  } // end SWITCH

  return AV_SAMPLE_FMT_NONE;
}

enum AVCodecID
Stream_Module_Decoder_Tools::mediaSubTypeToAVCodecId (REFGUID mediaSubType_in,
                                                      enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mediaSubTypeToAVCodecId"));

  enum AVCodecID result = AV_CODEC_ID_NONE;

  // sanity check(s)
  if (Stream_MediaFramework_Tools::isRGB (mediaSubType_in,
                                          mediaFramework_in) ||
      Stream_MediaFramework_Tools::isChromaLuminance (mediaSubType_in,
                                                      mediaFramework_in))
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("media type subtype is neither RGB nor Chroma/Luminance (was: \"%s\"), aborting\n"),
    //            ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaSubType_in, useMediaFoundation_in).c_str ())));
    return result;
  } // end IF

  if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_MJPG))
    result = AV_CODEC_ID_MJPEG;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media type subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mediaSubType_in, mediaFramework_in).c_str ())));
  } // end ELSE

  return result;
}

enum AVPixelFormat
Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat (REFGUID mediaSubType_in,
                                                          enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat"));

  enum AVPixelFormat result = AV_PIX_FMT_NONE;

  // sanity check(s)
  if (!Stream_MediaFramework_Tools::isRGB (mediaSubType_in,
                                           mediaFramework_in) &&
      !Stream_MediaFramework_Tools::isChromaLuminance (mediaSubType_in,
                                                       mediaFramework_in))
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("media type subtype is neither RGB nor Chroma/Luminance (was: \"%s\"), aborting\n"),
    //            ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaSubType_in, mediaFramework_in).c_str ())));
    return result;
  } // end IF

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB1))
        result = AV_PIX_FMT_MONOBLACK;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB4))
        result = AV_PIX_FMT_RGB4;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB8))
        result = AV_PIX_FMT_RGB8;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB555))
        result = AV_PIX_FMT_RGB555LE;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB565))
        result = AV_PIX_FMT_RGB565LE;
      // *IMPORTANT NOTE*: MEDIASUBTYPE_RGB24 actually has a 'BGR24' memory layout
      //                   see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd407253(v=vs.85).aspx
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB24))
        result = AV_PIX_FMT_RGB24;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32))
        result = AV_PIX_FMT_BGRA;
      //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32))
        result = AV_PIX_FMT_ARGB;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2R10G10B10))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2B10G10R10))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT))
        result = AV_PIX_FMT_BGRA;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB16_D3D_DX7_RT))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT))
        result = AV_PIX_FMT_ARGB;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444_D3D_DX7_RT))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555_D3D_DX7_RT))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT))
        result = AV_PIX_FMT_BGRA;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB16_D3D_DX9_RT))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32_D3D_DX9_RT))
        result = AV_PIX_FMT_ARGB;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444_D3D_DX9_RT))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555_D3D_DX9_RT))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_AYUV))
        result = AV_PIX_FMT_YUVA444P;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YUY2))
        result = AV_PIX_FMT_YUYV422;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_UYVY))
        result = AV_PIX_FMT_UYVY422;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC1))
        result = AV_PIX_FMT_P016LE;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC2))
        result = AV_PIX_FMT_NV12;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC3))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC4))
        result = AV_PIX_FMT_NV21;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YV12))
        result = AV_PIX_FMT_YUV420P;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_NV12))
        result = AV_PIX_FMT_NV12;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_I420))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IF09))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IYUV))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_Y211))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_Y411))
        result = AV_PIX_FMT_UYYVYY411;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YVU9))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YVYU))
        result = AV_PIX_FMT_YVYU422;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YUYV))
        result = AV_PIX_FMT_YUYV422;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_YUYV))
        result = AV_PIX_FMT_YUYV422;
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media type subtype (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mediaSubType_in, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
      } // end ELSE

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_ASSERT (false); // *TODO*
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
    }
  } // end SWITCH

  return result;
}
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

std::string
Stream_Module_Decoder_Tools::compressionFormatToString (enum Stream_Decoder_CompressionFormatType format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::compressionFormatToString"));

  std::string result = ACE_TEXT_ALWAYS_CHAR ("Invalid");

  switch (format_in)
  {
    case STREAM_COMPRESSION_FORMAT_NONE:
      result = ACE_TEXT_ALWAYS_CHAR ("none"); break;
    case STREAM_COMPRESSION_FORMAT_GZIP:
      result = ACE_TEXT_ALWAYS_CHAR ("gzip"); break;
    case STREAM_COMPRESSION_FORMAT_ZLIB:
      result = ACE_TEXT_ALWAYS_CHAR ("zlib"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return result;
}

#if defined (FFMPEG_SUPPORT)
enum AVCodecID
Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId (enum AVPixelFormat pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId"));

  enum AVCodecID result = AV_CODEC_ID_NONE;

  switch (pixelFormat_in)
  { // *NOTE*: libav does not specify a pixel format for MJPEG, it is a
    //         'compressed' format) --> map this deprecated format
    case AV_PIX_FMT_YUVJ422P:
      result = AV_CODEC_ID_MJPEG;
      break;
    default:
    {
//      ACE_DEBUG ((LM_DEBUG,
//                  ACE_TEXT ("invalid/unknown pixel format (was: %s), aborting\n"),
//                  ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (pixelFormat_in).c_str ())));
      break;
    }
  } // end SWITCH

  return result;
}

enum AVCodecID
Stream_Module_Decoder_Tools::filenameExtensionToAVCodecId (const std::string& filenameExtension_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::filenameExtensionToAVCodecId"));

  std::string extension_string =
      Common_String_Tools::toupper (filenameExtension_in);

  if (extension_string == ACE_TEXT_ALWAYS_CHAR ("PNG"))
    return AV_CODEC_ID_PNG;
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown filename extension (was: \"%s\"), aborting\n"),
                ACE_TEXT (filenameExtension_in.c_str ())));

  return AV_CODEC_ID_NONE;
}

std::string
Stream_Module_Decoder_Tools::audioFormatToString (enum AVSampleFormat format_in)
{
  std::string result;

  if (format_in == AV_SAMPLE_FMT_NONE)
    return result;

  char buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (char[BUFSIZ]));
  av_get_sample_fmt_string (buffer_a, sizeof (char[BUFSIZ]), format_in);
  result = buffer_a;

  return result;
}

bool
Stream_Module_Decoder_Tools::convert (struct SwsContext* context_in,
                                      unsigned int sourceWidth_in,
                                      unsigned int sourceHeight_in,
                                      enum AVPixelFormat sourcePixelFormat_in,
                                      uint8_t* sourceBuffers_in[],
                                      unsigned int targetWidth_in,
                                      unsigned int targetHeight_in,
                                      enum AVPixelFormat targetPixelFormat_in,
                                      uint8_t* targetBuffers_in[],
                                      bool flipVertically_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::convert"));

  // sanity check(s)
  if (unlikely (!sws_isSupportedInput (sourcePixelFormat_in) ||
                !sws_isSupportedOutput (targetPixelFormat_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("unsupported format conversion (was: %s --> %s), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (sourcePixelFormat_in).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (targetPixelFormat_in).c_str ())));
    return false;
  } // end IF
//  ACE_ASSERT (sourcePixelFormat_in != targetPixelFormat_in);

// *TODO*: define a balanced scaler parametrization that suits most
//         applications, or expose this as a parameter
  int flags = ( // SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
    SWS_FULL_CHR_H_INP | SWS_BICUBIC | SWS_ACCURATE_RND | SWS_BITEXACT);
  struct SwsContext* context_p =
      (context_in ? context_in
                  : sws_getCachedContext (NULL,
                                          sourceWidth_in, sourceHeight_in, sourcePixelFormat_in,
                                          targetWidth_in, targetHeight_in, targetPixelFormat_in,
                                          flags,                             // flags
                                          NULL, NULL,
                                          0));                               // parameters
  if (unlikely (!context_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_getCachedContext(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Image_Tools::errorToString (errno).c_str ())));
    return false;
  } // end IF

  bool result = false;
  int result_2 = -1;
  int in_linesize[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (in_linesize, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  int out_linesize[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (out_linesize, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  result_2 =
    av_image_fill_linesizes (in_linesize,
                             sourcePixelFormat_in,
                             static_cast<int> (sourceWidth_in));
  ACE_ASSERT (result_2 >= 0);
  if (unlikely (flipVertically_in))
    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i)
    {
      sourceBuffers_in[i] += in_linesize[i] * (sourceHeight_in - 1);
      in_linesize[i] = -in_linesize[i];
    } // end FOR
  result_2 = av_image_fill_linesizes (out_linesize,
                                      targetPixelFormat_in,
                                      static_cast<int> (targetWidth_in));
  ACE_ASSERT (result_2 >= 0);
  result_2 = sws_scale (context_p,
                        sourceBuffers_in, in_linesize,
                        0, sourceHeight_in,
                        targetBuffers_in, out_linesize);
  if (unlikely (result_2 <= 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_scale(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Image_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  // *NOTE*: ffmpeg returns fewer than the expected number of rows in some cases
  else if (unlikely (result_2 != static_cast<int> (targetHeight_in)))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("sws_scale() returned: %d (expected: %u), continuing\n"),
                result_2, targetHeight_in));
  result = true;

clean:
  if (unlikely (!context_in))
  {
    sws_freeContext (context_p); context_p = NULL;
  } // end IF

  return result;
}

bool
Stream_Module_Decoder_Tools::scale (struct SwsContext* context_in,
                                    unsigned int sourceWidth_in,
                                    unsigned int sourceHeight_in,
                                    enum AVPixelFormat pixelFormat_in,
                                    uint8_t* sourceBuffers_in[],
                                    unsigned int targetWidth_in,
                                    unsigned int targetHeight_in,
                                    uint8_t* targetBuffers_in[],
                                    bool flipVertically_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::scale"));

  // *TODO*: define a balanced scaler parametrization that suits most
  //         applications, or expose this as a parameter
  int flags =
    (SWS_FULL_CHR_H_INP | SWS_BICUBIC | SWS_ACCURATE_RND | SWS_BITEXACT);
  struct SwsContext* context_p =
      (context_in ? context_in
                  : sws_getCachedContext (NULL,
                                          sourceWidth_in, sourceHeight_in, pixelFormat_in,
                                          targetWidth_in, targetHeight_in, pixelFormat_in,
                                          flags,                             // flags
                                          NULL, NULL,
                                          0));                               // parameters
  if (unlikely (!context_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
    return false;
  } // end IF

  bool result = false;
  int result_2 = -1;
  int in_linesize[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&in_linesize, 0, sizeof (in_linesize));
  int out_linesize[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&out_linesize, 0, sizeof (out_linesize));
  result_2 =
    av_image_fill_linesizes (in_linesize,
                             pixelFormat_in,
                             static_cast<int> (sourceWidth_in));
  ACE_ASSERT (result_2 >= 0);
  if (unlikely (flipVertically_in))
    for (int i = 0; i < AV_NUM_DATA_POINTERS; ++i)
    {
      sourceBuffers_in[i] += in_linesize[i] * (sourceHeight_in - 1);
      in_linesize[i] = -in_linesize[i];
    } // end FOR
  result_2 = av_image_fill_linesizes (out_linesize,
                                      pixelFormat_in,
                                      static_cast<int> (targetWidth_in));
  ACE_ASSERT (result_2 >= 0);
//  try {
      result_2 = sws_scale (context_p,
                            sourceBuffers_in, in_linesize,
                            0, sourceHeight_in,
                            targetBuffers_in, out_linesize);
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in sws_scale(), aborting\n")));
//    result_2 = -1;
//  }
  if (unlikely (result_2 <= 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_scale(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Image_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  // *NOTE*: ffmpeg returns fewer than the expected number of rows in some cases
  else if (unlikely (result_2 != static_cast<int> (targetHeight_in)))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("sws_scale() returned: %d (expected: %u), continuing\n"),
                result_2, targetHeight_in));
  result = true;

clean:
  if (unlikely (!context_in))
  {
    sws_freeContext (context_p); context_p = NULL;
  } // end IF

  return result;
}
#endif // FFMPEG_SUPPORT

#if defined (OPENCV_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
int
Stream_Module_Decoder_Tools::mediaSubTypeToOpenCVFormat (REFGUID mediaSubType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::mediaSubTypeToOpenCVFormat"));

  // DirectShow
  /////////////////////////////////////// AUDIO
  // uncompressed audio
  //MEDIASUBTYPE_IEEE_FLOAT
  //MEDIASUBTYPE_PCM

  // MPEG-4 and AAC
  //MEDIASUBTYPE_MPEG_ADTS_AAC
  //MEDIASUBTYPE_MPEG_HEAAC
  //MEDIASUBTYPE_MPEG_LOAS
  //MEDIASUBTYPE_RAW_AAC1

  // Dolby
  //MEDIASUBTYPE_DOLBY_DDPLUS
  //MEDIASUBTYPE_DOLBY_AC3
  //MEDIASUBTYPE_DOLBY_AC3_SPDIF
  //MEDIASUBTYPE_DVM
  //MEDIASUBTYPE_RAW_SPORT
  //MEDIASUBTYPE_SPDIF_TAG_241h

  // miscellaneous
  //MEDIASUBTYPE_DRM_Audio
  //MEDIASUBTYPE_DTS
  //MEDIASUBTYPE_DTS2
  //MEDIASUBTYPE_DVD_LPCM_AUDIO
  //MEDIASUBTYPE_MPEG1AudioPayload
  //MEDIASUBTYPE_MPEG1Packet
  //MEDIASUBTYPE_MPEG1Payload
  //MEDIASUBTYPE_MPEG2_AUDIO
  //MEDIASUBTYPE_PCMAudio_Obsolete
  //MEDIASUBTYPE_MPEG_RAW_AAC

  /////////////////////////////////////// BDA
  //MEDIASUBTYPE_None

  /////////////////////////////////////// DVD
  //MEDIASUBTYPE_DTS
  //MEDIASUBTYPE_DVD_SUBPICTURE
  //MEDIASUBTYPE_SDDS
  //MEDIASUBTYPE_DVD_NAVIGATION_DSI
  //MEDIASUBTYPE_DVD_NAVIGATION_PCI
  //MEDIASUBTYPE_DVD_NAVIGATION_PROVIDER

  /////////////////////////////////////// Line 21
  //MEDIASUBTYPE_Line21_BytePair
  //MEDIASUBTYPE_Line21_GOPPacket
  //MEDIASUBTYPE_Line21_VBIRawData

  /////////////////////////////////////// MPEG-1
  //MEDIASUBTYPE_MPEG1System
  //MEDIASUBTYPE_MPEG1VideoCD
  //MEDIASUBTYPE_MPEG1Packet
  //MEDIASUBTYPE_MPEG1Payload
  //MEDIASUBTYPE_MPEG1Video
  //MEDIASUBTYPE_MPEG1Audio
  //MEDIASUBTYPE_MPEG1AudioPayload

  /////////////////////////////////////// MPEG-2
  // MPEG-2 (splitter)
  //MEDIASUBTYPE_MPEG2_VIDEO
  //MEDIASUBTYPE_DOLBY_AC3
  //MEDIASUBTYPE_DOLBY_AC3_SPDIF
  //MEDIASUBTYPE_MPEG2_AUDIO
  //MEDIASUBTYPE_DVD_LPCM_AUDIO
  // MPEG-2 (demultiplexer)
  //MEDIASUBTYPE_MPEG2_PROGRAM
  //MEDIASUBTYPE_MPEG2_TRANSPORT
  //MEDIASUBTYPE_MPEG2_TRANSPORT_STRIDE
  //MEDIASUBTYPE_ATSC_SI
  //MEDIASUBTYPE_DVB_SI
  //MEDIASUBTYPE_ISDB_SI
  //MEDIASUBTYPE_MPEG2DATA
  // MPEG-2 (kernel)

  /////////////////////////////////////// Stream
  //MEDIASUBTYPE_AIFF
  //MEDIASUBTYPE_Asf
  //MEDIASUBTYPE_Avi
  //MEDIASUBTYPE_AU
  //MEDIASUBTYPE_DssAudio
  //MEDIASUBTYPE_DssVideo
  //MEDIASUBTYPE_MPEG1Audio
  //MEDIASUBTYPE_MPEG1System
  //MEDIASUBTYPE_MPEG1SystemStream
  //MEDIASUBTYPE_MPEG1Video
  //MEDIASUBTYPE_MPEG1VideoCD
  //MEDIASUBTYPE_WAVE

  /////////////////////////////////////// VBI
  //KSDATAFORMAT_SUBTYPE_RAW8
  //MEDIASUBTYPE_TELETEXT
  //MEDIASUBTYPE_VPS
  //MEDIASUBTYPE_WSS

  /////////////////////////////////////// VIDEO
  // analog video
  //MEDIASUBTYPE_AnalogVideo_NTSC_M
  //MEDIASUBTYPE_AnalogVideo_PAL_B
  //MEDIASUBTYPE_AnalogVideo_PAL_D
  //MEDIASUBTYPE_AnalogVideo_PAL_G
  //MEDIASUBTYPE_AnalogVideo_PAL_H
  //MEDIASUBTYPE_AnalogVideo_PAL_I
  //MEDIASUBTYPE_AnalogVideo_PAL_M
  //MEDIASUBTYPE_AnalogVideo_PAL_N
  //MEDIASUBTYPE_AnalogVideo_SECAM_B
  //MEDIASUBTYPE_AnalogVideo_SECAM_D
  //MEDIASUBTYPE_AnalogVideo_SECAM_G
  //MEDIASUBTYPE_AnalogVideo_SECAM_H
  //MEDIASUBTYPE_AnalogVideo_SECAM_K
  //MEDIASUBTYPE_AnalogVideo_SECAM_K1
  //MEDIASUBTYPE_AnalogVideo_SECAM_L

  // directx video acceleration
  //MEDIASUBTYPE_AI44
  //MEDIASUBTYPE_IA44

  // DV video
  //MEDIASUBTYPE_dvsl
  //MEDIASUBTYPE_dvsd
  //MEDIASUBTYPE_dvhd

  // H.264
  //MEDIASUBTYPE_AVC1
  //MEDIASUBTYPE_H264
  //MEDIASUBTYPE_h264
  //MEDIASUBTYPE_X264
  //MEDIASUBTYPE_x264

  // uncompressed RGB (no alpha)
  //MEDIASUBTYPE_RGB1
  //MEDIASUBTYPE_RGB4
  if (mediaSubType_in == MEDIASUBTYPE_RGB8)
    return CV_8UC1;
  //MEDIASUBTYPE_RGB555
  //MEDIASUBTYPE_RGB565
  if (mediaSubType_in == MEDIASUBTYPE_RGB24)
    return CV_8UC3;
  if (mediaSubType_in == MEDIASUBTYPE_RGB32)
    return CV_8UC4;
  // uncompressed RGB (alpha)
  //MEDIASUBTYPE_ARGB1555
  if (mediaSubType_in == MEDIASUBTYPE_ARGB32)
    return CV_8UC4;
  //MEDIASUBTYPE_ARGB4444
  //MEDIASUBTYPE_A2R10G10B10
  //MEDIASUBTYPE_A2B10G10R10

  // video mixing renderer (VMR-7)
  if (mediaSubType_in == MEDIASUBTYPE_RGB32_D3D_DX7_RT)
    return CV_8UC4;
  if (mediaSubType_in == MEDIASUBTYPE_RGB16_D3D_DX7_RT)
    return CV_8UC2;
  if (mediaSubType_in == MEDIASUBTYPE_ARGB32_D3D_DX7_RT)
    return CV_8UC4;
  //MEDIASUBTYPE_ARGB4444_D3D_DX7_RT
  //MEDIASUBTYPE_ARGB1555_D3D_DX7_RT
  // video mixing renderer (VMR-9)
  if (mediaSubType_in == MEDIASUBTYPE_RGB32_D3D_DX9_RT)
    return CV_8UC4;
  if (mediaSubType_in == MEDIASUBTYPE_RGB16_D3D_DX9_RT)
    return CV_8UC2;
  if (mediaSubType_in == MEDIASUBTYPE_ARGB32_D3D_DX9_RT)
    return CV_8UC4;
  //MEDIASUBTYPE_ARGB4444_D3D_DX9_RT
  //MEDIASUBTYPE_ARGB1555_D3D_DX9_RT

  // YUV video
  //MEDIASUBTYPE_AYUV
  //MEDIASUBTYPE_YUY2
  //MEDIASUBTYPE_UYVY
  //MEDIASUBTYPE_IMC1
  //MEDIASUBTYPE_IMC2
  //MEDIASUBTYPE_IMC3
  //MEDIASUBTYPE_IMC4
  //MEDIASUBTYPE_YV12
  //MEDIASUBTYPE_NV12
  // other YUV
  //MEDIASUBTYPE_I420
  //MEDIASUBTYPE_IF09
  //MEDIASUBTYPE_IYUV
  //MEDIASUBTYPE_Y211
  //MEDIASUBTYPE_Y411
  //MEDIASUBTYPE_Y41P
  //MEDIASUBTYPE_YVU9
  //MEDIASUBTYPE_YVYU
  //MEDIASUBTYPE_YUYV

  // miscellaneous
  //MEDIASUBTYPE_CFCC
  //MEDIASUBTYPE_CLJR
  //MEDIASUBTYPE_CPLA
  //MEDIASUBTYPE_CLPL
  //MEDIASUBTYPE_IJPG
  //MEDIASUBTYPE_MDVF
  //MEDIASUBTYPE_MJPG
  //MEDIASUBTYPE_Overlay
  //MEDIASUBTYPE_Plum
  //MEDIASUBTYPE_QTJpeg
  //MEDIASUBTYPE_QTMovie
  //MEDIASUBTYPE_QTRle
  //MEDIASUBTYPE_QTRpza
  //MEDIASUBTYPE_QTSmc
  //MEDIASUBTYPE_TVMJ
  //MEDIASUBTYPE_VPVBI
  //MEDIASUBTYPE_VPVideo
  //MEDIASUBTYPE_WAKE

  ///////////////////////////////////////
  // unknown
  //MEDIASUBTYPE_DVCS
  //MEDIASUBTYPE_DVSD

  // Media Foundation
  if (mediaSubType_in == MFVideoFormat_RGB32)
    return CV_8UC4;
  if (mediaSubType_in == MFVideoFormat_ARGB32)
    return CV_8UC4;
  if (mediaSubType_in == MFVideoFormat_RGB24)
    return CV_8UC3;
  //MFVideoFormat_RGB555
  //MFVideoFormat_RGB565
  if (mediaSubType_in == MFVideoFormat_RGB8)
    return CV_8UC1;
  //MFVideoFormat_L8
  //MFVideoFormat_L16
  //MFVideoFormat_D16
  //MFVideoFormat_AI44
  //MFVideoFormat_AYUV
  //MFVideoFormat_YUY2
  //MFVideoFormat_YVYU
  //MFVideoFormat_YVU9
  //MFVideoFormat_UYVY
  //MFVideoFormat_NV11
  //MFVideoFormat_NV12
  //MFVideoFormat_YV12
  //MFVideoFormat_I420
  //MFVideoFormat_IYUV
  //MFVideoFormat_Y210
  //MFVideoFormat_Y216
  //MFVideoFormat_Y410
  //MFVideoFormat_Y416
  //MFVideoFormat_Y41P
  //MFVideoFormat_Y41T
  //MFVideoFormat_Y42T
  //MFVideoFormat_P210
  //MFVideoFormat_P216
  //MFVideoFormat_P010
  //MFVideoFormat_P016
  //MFVideoFormat_v210
  //MFVideoFormat_v216
  //MFVideoFormat_v410
  //MFVideoFormat_MP43
  //MFVideoFormat_MP4S
  //MFVideoFormat_M4S2
  //MFVideoFormat_MP4V
  //MFVideoFormat_WMV1
  //MFVideoFormat_WMV2
  //MFVideoFormat_WMV3
  //MFVideoFormat_WVC1
  //MFVideoFormat_MSS1
  //MFVideoFormat_MSS2
  //MFVideoFormat_MPG1
  //MFVideoFormat_DVSL
  //MFVideoFormat_DVSD
  //MFVideoFormat_DVHD
  //MFVideoFormat_DV25
  //MFVideoFormat_DV50
  //MFVideoFormat_DVH1
  //MFVideoFormat_DVC
  //MFVideoFormat_H264
  //MFVideoFormat_MJPG
  //MFVideoFormat_420O
  //MFVideoFormat_HEVC
  //MFVideoFormat_HEVC_ES
#if (WINVER >= _WIN32_WINNT_WIN8)
  //MFVideoFormat_H263
#endif // WINVER >= _WIN32_WINNT_WIN8
  //MFVideoFormat_H264_ES
  //MFVideoFormat_MPEG2

  //MFAudioFormat_PCM
  //MFAudioFormat_Float
  //MFAudioFormat_DTS
  //MFAudioFormat_Dolby_AC3_SPDIF
  //MFAudioFormat_DRM
  //MFAudioFormat_WMAudioV8
  //MFAudioFormat_WMAudioV9
  //MFAudioFormat_WMAudio_Lossless
  //MFAudioFormat_WMASPDIF
  //MFAudioFormat_MSP1
  //MFAudioFormat_MP3
  //MFAudioFormat_MPEG
  //MFAudioFormat_AAC
  //MFAudioFormat_ADTS
  //MFAudioFormat_AMR_NB
  //MFAudioFormat_AMR_WB
  //MFAudioFormat_AMR_WP
  //MFAudioFormat_Dolby_AC3
  //MFAudioFormat_Dolby_DDPlus

  return -1;
}
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
int
Stream_Module_Decoder_Tools::AVPixelFormatToOpenCVFormat (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::AVPixelFormatToOpenCVFormat"));

  switch (format_in)
  {
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_BGR8:
    case AV_PIX_FMT_RGB8:
      return CV_8UC1;
    case AV_PIX_FMT_RGB565BE:
    case AV_PIX_FMT_RGB565LE:
    case AV_PIX_FMT_RGB555BE:
    case AV_PIX_FMT_RGB555LE:
    case AV_PIX_FMT_BGR565BE:
    case AV_PIX_FMT_BGR565LE:
    case AV_PIX_FMT_BGR555BE:
    case AV_PIX_FMT_BGR555LE:
    case AV_PIX_FMT_RGB444LE:
    case AV_PIX_FMT_RGB444BE:
    case AV_PIX_FMT_BGR444LE:
    case AV_PIX_FMT_BGR444BE:
      return CV_8UC2;
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_BGR24:
      return CV_8UC3;
    case AV_PIX_FMT_ARGB:
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_ABGR:
    case AV_PIX_FMT_BGRA:
      return CV_8UC4;
    case AV_PIX_FMT_GRAY16BE:
    case AV_PIX_FMT_GRAY16LE:
      return CV_16UC1;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return -1;
}
#endif // FFMPEG_SUPPORT
#endif // OPENCV_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Module_Decoder_Tools::loadAudioRendererGraph (REFGUID deviceCategory_in,
                                                     const struct _AMMediaType& mediaType_in,
                                                     const struct _AMMediaType& outputMediaType_in,
                                                     bool grabSamples_in,
                                                     int audioOutput_in,
                                                     IGraphBuilder* IGraphBuilder_in,
                                                     REFGUID effect_in,
                                                     const union Stream_MediaFramework_DirectSound_AudioEffectOptions& effectOptions_in,
                                                     Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadAudioRendererGraph"));

  HRESULT result = E_FAIL;
  struct _GUID GUID_s = GUID_NULL;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  IDMOWrapperFilter* wrapper_filter_p = NULL;
  IMediaObject* media_object_p = NULL;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  WAVEFORMATEXTENSIBLE* waveformatextensible_p = NULL;
  bool has_resampler_b = false;

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::clear (graphConfiguration_out);

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  if (!Stream_MediaFramework_DirectShow_Tools::reset (IGraphBuilder_in,
                                                      deviceCategory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::reset(), aborting\n")));
    return false;
  } // end IF
  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO_L;
  //else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
  //  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else if (InlineIsEqualGUID (deviceCategory_in, GUID_NULL))
  {
    IEnumFilters* enumerator_p = NULL;
    result = IGraphBuilder_in->EnumFilters (&enumerator_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::EnumFilters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      return false;
    } // end IF
    result = enumerator_p->Next (1, &filter_p, NULL);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IEnumFilters::Next(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      enumerator_p->Release ();
      return false;
    } // end IF
    ACE_ASSERT (filter_p);
    struct _FilterInfo filter_info;
    ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
    result = filter_p->QueryFilterInfo (&filter_info);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      filter_p->Release ();
      enumerator_p->Release ();
      return false;
    } // end IF
    graph_entry.filterName = filter_info.achName;
    if (filter_info.pGraph)
      filter_info.pGraph->Release ();
    filter_p->Release (); filter_p = NULL;
    enumerator_p->Release ();
  } // end ELSE IF
  graph_entry.mediaType =
    Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in);
  ACE_ASSERT (graph_entry.mediaType);
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // step0: add resampler ? (effects require PCM @ 44100 samples/sec)
  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.majortype, MEDIATYPE_Audio));
  //ACE_ASSERT (InlineIsEqualGUID (mediaType_in.subtype, MEDIASUBTYPE_PCM));
  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (mediaType_in.cbFormat >= sizeof (struct tWAVEFORMATEX));
  waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_in.pbFormat);
  if ((waveformatex_p->nSamplesPerSec == 44100) ||
      InlineIsEqualGUID (effect_in, GUID_NULL))
    goto continue_;

  result = CoCreateInstance (CLSID_ACMWrapper, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_ACMWrapper).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result =
    IGraphBuilder_in->AddFilter (filter_p,
                                 STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_L);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.connectDirect = true;
  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_L;
  graph_entry.mediaType =
    Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in);
  ACE_ASSERT (graph_entry.mediaType);
  //// *NOTE*: this effects seems to require lSampleSize of 1 to connect
  //graph_entry.mediaType->lSampleSize = 1;
  waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (graph_entry.mediaType->pbFormat);
  waveformatex_p->nSamplesPerSec = 44100;
  waveformatex_p->nAvgBytesPerSec =
    (waveformatex_p->nSamplesPerSec * waveformatex_p->nBlockAlign);
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.connectDirect = false;
  graph_entry.mediaType =
    Stream_MediaFramework_DirectShow_Tools::copy (*graph_entry.mediaType);
  ACE_ASSERT (graph_entry.mediaType);
  filter_p->Release (); filter_p = NULL;
  has_resampler_b = true;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

  // step1: add effect DMO ?
continue_:
   if (InlineIsEqualGUID (effect_in, GUID_NULL))
    goto continue_2;

  result = CoCreateInstance (CLSID_DMOWrapperFilter, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_DMOWrapperFilter).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result = filter_p->QueryInterface (IID_PPV_ARGS (&wrapper_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (wrapper_filter_p);
  result = wrapper_filter_p->Init (effect_in,
                                   DMOCATEGORY_AUDIO_EFFECT);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDMOWrapperFilter::Init(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  // set effect options
  if (InlineIsEqualGUID (effect_in, CLSID_CWMAudioAEC))
  {
    //IDirectSoundCaptureFXAec* effect_p = NULL;
    //result = wrapper_filter_p->QueryInterface (IID_IDirectSoundCaptureFXAec,
    //                                           (void**)&effect_p);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundCaptureFXAec): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    //  goto error;
    //} // end IF
    //result = effect_p->SetAllParameters (&effectOptions_in.AECOptions);
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to IDirectSoundCaptureFXAec::SetAllParameters(): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    //  effect_p->Release (); effect_p = NULL;
    //  goto error;
    //} // end IF
    //effect_p->Release (); effect_p = NULL;
    IPropertyStore* property_store_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_PPV_ARGS (&property_store_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IPropertyStore): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    struct tagPROPVARIANT property_s;
    PropVariantInit (&property_s);
    property_s.vt = VT_I4;
    // *IMPORTANT NOTE*: when selecting SINGLE_CHANNEL_AEC, the DSP has two
    //                   input pins, which does not work with the DMO wrapper
    //                   filter
    property_s.intVal = SINGLE_CHANNEL_NSAGC;
    result =
      property_store_p->SetValue (MFPKEY_WMAAECMA_SYSTEM_MODE, property_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyStore::SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_store_p->Release (); property_store_p = NULL;
      goto error;
    } // end IF
    PropVariantClear (&property_s);
    property_s.vt = VT_BOOL;
    property_s.boolVal = VARIANT_FALSE;
    result = property_store_p->SetValue (MFPKEY_WMAAECMA_DMO_SOURCE_MODE, property_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyStore::SetValue(MFPKEY_WMAAECMA_DMO_SOURCE_MODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_store_p->Release (); property_store_p = NULL;
      goto error;
    } // end IF
    property_s.boolVal = VARIANT_TRUE;
    result = property_store_p->SetValue (MFPKEY_WMAAECMA_FEATURE_MODE, property_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyStore::SetValue(MFPKEY_WMAAECMA_FEATURE_MODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_store_p->Release (); property_store_p = NULL;
      goto error;
    } // end IF
    //MFPKEY_WMAAECMA_DEVICEPAIR_GUID
    PropVariantClear (&property_s);
    property_s.vt = VT_I4;
    property_s.intVal = 1;
    result = property_store_p->SetValue (MFPKEY_WMAAECMA_FEATR_AES, property_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyStore::SetValue(MFPKEY_WMAAECMA_FEATR_AES): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_store_p->Release (); property_store_p = NULL;
      goto error;
    } // end IF
    //MFPKEY_WMAAECMA_FEATR_AGC // VT_BOOL
    //MFPKEY_WMAAECMA_FEATR_CENTER_CLIP // VT_BOOL
    //MFPKEY_WMAAECMA_FEATR_ECHO_LENGTH // VT_I4
    //MFPKEY_WMAAECMA_FEATR_FRAME_SIZE // VT_I4
    //MFPKEY_WMAAECMA_FEATR_MICARR_MODE // VT_I4
    //MFPKEY_WMAAECMA_FEATR_MICARR_BEAM // VT_I4
    //MFPKEY_WMAAECMA_FEATR_MICARR_PREPROC // VT_BOOL
    //MFPKEY_WMAAECMA_FEATR_NOISE_FILL // VT_BOOL
    //MFPKEY_WMAAECMA_FEATR_NS // VT_I4
    property_s.intVal = AEC_VAD_NORMAL;
    result = property_store_p->SetValue (MFPKEY_WMAAECMA_FEATR_VAD, property_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyStore::SetValue(MFPKEY_WMAAECMA_FEATR_VAD): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_store_p->Release (); property_store_p = NULL;
      goto error;
    } // end IF
    //MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER // VT_BOOL
    //MFPKEY_WMAAECMA_MICARRAY_DESCPTR // VT_BLOB
    //MFPKEY_WMAAECMA_QUALITY_METRICS // VT_BLOB
    //MFPKEY_WMAAECMA_RETRIEVE_TS_STATS // VT_BOOL
    PropVariantClear (&property_s);
    property_store_p->Release (); property_store_p = NULL;
  } // end IF
  //////////////////////////////////////
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_CHORUS))
  {
    IDirectSoundFXChorus* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXChorus,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXChorus): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.chorusOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXChorus::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_COMPRESSOR))
  {
    IDirectSoundFXCompressor* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXCompressor,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXCompressor): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.compressorOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXCompressor::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_DISTORTION))
  {
    IDirectSoundFXDistortion* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXDistortion,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXDistortion): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.distortionOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXDistortion::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_ECHO))
  {
    IDirectSoundFXEcho* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXEcho,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXEcho): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.echoOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXEcho::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_PARAMEQ))
  {
    IDirectSoundFXParamEq* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXParamEq,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXParamEq): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.equalizerOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXParamEq::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_FLANGER))
  {
    IDirectSoundFXFlanger* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXFlanger,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXFlanger): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.flangerOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXFlanger::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_GARGLE))
  {
    IDirectSoundFXGargle* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXGargle,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXGargle): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.gargleOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXGargle::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_I3DL2REVERB))
  {
    IDirectSoundFXI3DL2Reverb* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXI3DL2Reverb,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXI3DL2Reverb): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.reverbOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXI3DL2Reverb::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_WAVES_REVERB))
  {
    IDirectSoundFXWavesReverb* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXWavesReverb,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXWavesReverb): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.wavesReverbOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXWavesReverb::SetAllParameters(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      effect_p->Release (); effect_p = NULL;
      goto error;
    } // end IF
    effect_p->Release (); effect_p = NULL;
  } // end ELSE IF
  //////////////////////////////////////
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown effect (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (effect_in).c_str ())));
  wrapper_filter_p->Release (); wrapper_filter_p = NULL;
  result = filter_p->QueryInterface (IID_PPV_ARGS (&media_object_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMediaObject): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (media_object_p);
  result = media_object_p->AllocateStreamingResources ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaObject::AllocateStreamingResources(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    media_object_p->Release (); media_object_p = NULL;
    goto error;
  } // end IF
  media_object_p->Release (); media_object_p = NULL;
  result =
    IGraphBuilder_in->AddFilter (filter_p,
                                 STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO_L);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO_L;
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;
  filter_p->Release (); filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

continue_2:
  // step2: add sample grabber ?
  if (!grabSamples_in)
    goto continue_3;

  // step2a: add (second) resampler ?
  if (InlineIsEqualGUID (outputMediaType_in.majortype, GUID_NULL))
    goto continue_4;
  // *NOTE*: "...Decompression is only to PCM audio. ..."
  ACE_ASSERT (InlineIsEqualGUID (outputMediaType_in.majortype, MEDIATYPE_Audio));
  ACE_ASSERT (InlineIsEqualGUID (outputMediaType_in.formattype, FORMAT_WaveFormatEx));
  waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (outputMediaType_in.pbFormat);
  ACE_ASSERT (waveformatex_p);
  if (Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("output format is floating point (was: %s); cannot use directshow resampler, continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (outputMediaType_in, true).c_str ())));

  result = CoCreateInstance (CLSID_ACMWrapper, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_ACMWrapper).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result =
    IGraphBuilder_in->AddFilter (filter_p,
                                 (has_resampler_b ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_2_L
                                                  : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_L));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName =
    (has_resampler_b ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_2_L
                     : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RESAMPLER_L);
  ACE_ASSERT (!graph_entry.mediaType);
  graph_entry.mediaType = Stream_MediaFramework_DirectShow_Tools::copy (outputMediaType_in);
  ACE_ASSERT (graph_entry.mediaType);
  // *NOTE*: "...Decompression is only to PCM audio. ..."
  ACE_ASSERT (InlineIsEqualGUID (graph_entry.mediaType->majortype, MEDIATYPE_Audio));
  ACE_ASSERT (InlineIsEqualGUID (graph_entry.mediaType->formattype, FORMAT_WaveFormatEx));
  ACE_ASSERT (graph_entry.mediaType->cbFormat >= sizeof (struct tWAVEFORMATEX));
  waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (graph_entry.mediaType->pbFormat);
  ACE_ASSERT (waveformatex_p);
  if (waveformatex_p->wFormatTag == WAVE_FORMAT_EXTENSIBLE)
    waveformatextensible_p =
      reinterpret_cast<WAVEFORMATEXTENSIBLE*> (graph_entry.mediaType->pbFormat);
  if (((waveformatex_p->wFormatTag != WAVE_FORMAT_EXTENSIBLE) && (waveformatex_p->wFormatTag != WAVE_FORMAT_PCM)) ||
      ((waveformatex_p->wFormatTag == WAVE_FORMAT_EXTENSIBLE) && (!InlineIsEqualGUID (waveformatextensible_p->SubFormat, KSDATAFORMAT_SUBTYPE_PCM))))
  {
    switch (waveformatex_p->wFormatTag)
    {
      case WAVE_FORMAT_IEEE_FLOAT:
      { ACE_ASSERT (waveformatex_p->wBitsPerSample == 32);
        waveformatex_p->wBitsPerSample = 16;
        waveformatex_p->nBlockAlign =
          (waveformatex_p->wBitsPerSample / 8) * waveformatex_p->nChannels;
        waveformatex_p->nAvgBytesPerSec =
          waveformatex_p->nBlockAlign * waveformatex_p->nSamplesPerSec;
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: adjusted output resolution (was: 32 bits) to 16 bits\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
        waveformatex_p->wFormatTag = WAVE_FORMAT_PCM;
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: adjusted output format (was: %d) to %d\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                    WAVE_FORMAT_IEEE_FLOAT, WAVE_FORMAT_PCM));
        graph_entry.mediaType->subtype = MEDIASUBTYPE_PCM;
        graph_entry.mediaType->lSampleSize = waveformatex_p->nBlockAlign;
        break;
      }
      case WAVE_FORMAT_EXTENSIBLE:
      {
        if (InlineIsEqualGUID (waveformatextensible_p->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT))
        { ACE_ASSERT (waveformatextensible_p->Format.wBitsPerSample == 32);
          ACE_ASSERT (waveformatextensible_p->Samples.wValidBitsPerSample == 32);
          waveformatextensible_p->Format.wBitsPerSample = 16;
          waveformatextensible_p->Format.nBlockAlign =
              (waveformatextensible_p->Format.wBitsPerSample / 8) * waveformatextensible_p->Format.nChannels;
          waveformatextensible_p->Format.nAvgBytesPerSec =
            waveformatextensible_p->Format.nBlockAlign * waveformatextensible_p->Format.nSamplesPerSec;
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: adjusted output resolution (was: 32 bits) to 16 bits\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
          waveformatextensible_p->Samples.wValidBitsPerSample = 16;
          waveformatextensible_p->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
          waveformatextensible_p->Format.wFormatTag = WAVE_FORMAT_PCM;
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: adjusted output format (was: \"%s\") to \"%s\"\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                      ACE_TEXT (Common_OS_Tools::GUIDToString (KSDATAFORMAT_SUBTYPE_IEEE_FLOAT).c_str ()),
                      ACE_TEXT (Common_OS_Tools::GUIDToString (KSDATAFORMAT_SUBTYPE_PCM).c_str ())));
          graph_entry.mediaType->subtype = MEDIASUBTYPE_PCM;
          graph_entry.mediaType->lSampleSize =
            waveformatextensible_p->Format.nBlockAlign;
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown output sub-format (was: \"%s\"), aborting\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                      ACE_TEXT (Common_OS_Tools::GUIDToString (waveformatextensible_p->SubFormat).c_str ())));
          goto error;
        } // end ELSE
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown output format (was: %d), aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                    waveformatex_p->wFormatTag));
        goto error;
      }
    } // end SWITCH
  } // end IF
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;
  filter_p->Release (); filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

continue_4:
  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_SampleGrabber).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result =
    IGraphBuilder_in->AddFilter (filter_p,
                                 STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L;
  graphConfiguration_out.push_back (graph_entry);
  filter_p->Release (); filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

continue_3:
  // send to an output (waveOut) ?
  if (audioOutput_in >= 0)
  {
    GUID_s = STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_AUDIO_RENDER;
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO_L;
  } // end IF
  else
  {
    GUID_s = CLSID_NullRenderer;
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL_L;
  } // end ELSE
  result = CoCreateInstance (GUID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  result =
    IGraphBuilder_in->AddFilter (filter_p,
                                 graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graphConfiguration_out.push_back (graph_entry);
  filter_p->Release (); filter_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added \"%s\"\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));

  //result =
  //  ICaptureGraphBuilder2_in->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
  //                                          &PIN_CATEGORY_CAPTURE, NULL,
  //                                          filter_p,
  //                                          filter_2,
  //                                          //NULL,
  //                                          filter_4);
  //if (FAILED (result)) // E_INVALIDARG = 0x80070057, 0x80040217 = VFW_E_CANNOT_CONNECT ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  //  return false;
  //} // end IF

#if defined (_DEBUG)
  Stream_MediaFramework_DirectShow_Tools::dump (graphConfiguration_out);
#endif // _DEBUG

  return true;

error:
  Stream_MediaFramework_DirectShow_Tools::clear (graphConfiguration_out);
  if (filter_p)
    filter_p->Release ();
  if (wrapper_filter_p)
    wrapper_filter_p->Release ();

  return false;
}

bool
Stream_Module_Decoder_Tools::loadVideoRendererGraph (REFGUID deviceCategory_in,
                                                     const struct _AMMediaType& captureFormat_in,
                                                     const struct _AMMediaType& outputFormat_in,
                                                     HWND windowHandle_in,
                                                     IGraphBuilder* IGraphBuilder_in,
                                                     Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadVideoRendererGraph"));

  HRESULT result = E_FAIL;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  IBaseFilter* filter_p = NULL;
  IPin* pin_p = NULL;
  bool result_2 = false;
  bool skip_decode = false;
  bool skip_grab = false;
  bool is_first_decompress = true;
  bool is_partially_connected = false;
  struct _GUID CLSID_s = GUID_NULL;
  struct _GUID preferred_subtype = outputFormat_in.subtype;
  struct _AMMediaType* media_type_p = NULL;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  FOURCCMap fourcc_map;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::clear (graphConfiguration_out);

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO_L;
  else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO_L;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceCategory_in).c_str ())));
    goto error;
  } // end ELSE

  if (!Stream_MediaFramework_DirectShow_Tools::reset (IGraphBuilder_in,
                                                      deviceCategory_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::reset(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (Stream_MediaFramework_DirectShow_Tools::has (IGraphBuilder_in, graph_entry.filterName));
  graph_entry.mediaType =
    Stream_MediaFramework_DirectShow_Tools::copy (captureFormat_in);
  if (!graph_entry.mediaType)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  graphConfiguration_out.push_back (graph_entry); // capture
  graph_entry.mediaType = NULL;

  // step1: decompress ?
  if (InlineIsEqualGUID (outputFormat_in.subtype, captureFormat_in.subtype) ||
      !Stream_MediaFramework_Tools::isCompressedVideo (captureFormat_in.subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  {
    graph_entry.mediaType =
      Stream_MediaFramework_DirectShow_Tools::copy (captureFormat_in);
    if (!graph_entry.mediaType)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
      goto error;
    } // end IF
    goto decode;
  } // end IF

decompress:
  // *NOTE*: the first decompressors' input format is the capture format
  if (is_first_decompress)
  {
    graph_entry.mediaType =
      Stream_MediaFramework_DirectShow_Tools::copy (captureFormat_in);
    if (!graph_entry.mediaType)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (graph_entry.mediaType);

#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  fourcc_map.SetFOURCC (&graph_entry.mediaType->subtype);
  switch (fourcc_map.GetFOURCC ())
#else
  switch (graph_entry.mediaType->subtype.Data1)
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
  {
    // *** compressed types ***
    case FCC ('H264'):
    {
      CLSID_s = CLSID_CMPEG2VidDecoderDS;
      graph_entry.filterName =
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264_L;
      preferred_subtype = MEDIASUBTYPE_NV12;
      // *NOTE*: the EVR video renderer (!) can handle the nv12 chroma type
      //         --> do not decode
      skip_decode = true;
      // *TODO*: for some reason the decoder fails to connect to the sample
      //          grabber
      //         --> do not grab
      skip_grab = true;
      break;
    }
    case FCC ('MJPG'):
    {
      CLSID_s = CLSID_MjpegDec;
      graph_entry.filterName =
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG_L;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  } // end SWITCH

  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graphConfiguration_out.push_back (graph_entry); // decompressor
  graph_entry.mediaType = NULL;

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
    goto error;
  } // end IF
  filter_p->Release (); filter_p = NULL;

  // need another decompressor ?
  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_MediaFramework_DirectShow_Tools::countFormats (pin_p,
                                                             graphConfiguration_out.back ().mediaType->formattype))
    if (!is_partially_connected)
    {
      if (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_in,
                                                            graphConfiguration_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      is_partially_connected = true;
    } // end IF
  if (!Stream_MediaFramework_DirectShow_Tools::hasUncompressedFormat (CLSID_VideoInputDeviceCategory,
                                                                      pin_p,
                                                                      graph_entry.mediaType))
  { ACE_ASSERT (!graph_entry.mediaType);
    if (!Stream_MediaFramework_DirectShow_Tools::getFirstFormat (pin_p,
                                                                 GUID_NULL,
                                                                 true, // top-to-bottom
                                                                 graph_entry.mediaType))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(), aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ())));
      goto error;
    } // end IF
    pin_p->Release (); pin_p = NULL;
    ACE_ASSERT (graph_entry.mediaType);
    if (is_partially_connected)
    {
      if (!Stream_MediaFramework_DirectShow_Tools::disconnect (IGraphBuilder_in))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n")));
        goto error;
      } // end IF
      is_partially_connected = false;
    } // end IF
    goto decompress;
  } // end IF
  if (is_partially_connected)
  {
    if (!Stream_MediaFramework_DirectShow_Tools::disconnect (IGraphBuilder_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    is_partially_connected = false;
  } // end IF
  ACE_ASSERT (graph_entry.mediaType);
  pin_p->Release (); pin_p = NULL;

decode:
  ACE_ASSERT (graph_entry.mediaType);
  if (InlineIsEqualGUID (GUID_NULL, outputFormat_in.subtype)                      ||
      InlineIsEqualGUID (graph_entry.mediaType->subtype, outputFormat_in.subtype) ||
      skip_decode)
    goto grab;

  preferred_subtype = outputFormat_in.subtype;
  if (Stream_MediaFramework_Tools::isRGB (graph_entry.mediaType->subtype,
                                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  {
    CLSID_s = CLSID_Colour;
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_RGB_L;
  } // end IF
  else if (Stream_MediaFramework_Tools::isChromaLuminance (graph_entry.mediaType->subtype,
                                                           STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  {
    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    CLSID_s = CLSID_AVIDec;
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI_L;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
    goto error;
  }

  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graphConfiguration_out.push_back (graph_entry); // decoder
  graph_entry.mediaType = NULL;

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
    goto error;
  } // end IF
  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_MediaFramework_DirectShow_Tools::countFormats (pin_p,
                                                             graphConfiguration_out.back ().mediaType->formattype))
  {
    if (!is_partially_connected)
    {
      if (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_in,
                                                            graphConfiguration_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      is_partially_connected = true;
    } // end IF
  } // end IF
  if (!Stream_MediaFramework_DirectShow_Tools::getFirstFormat (pin_p,
                                                               preferred_subtype,
                                                               true, // top-to-bottom
                                                               graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (preferred_subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
    goto error;
  } // end IF
  pin_p->Release (); pin_p = NULL;
  if (is_partially_connected)
  {
    if (!Stream_MediaFramework_DirectShow_Tools::disconnect (IGraphBuilder_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    is_partially_connected = false;
  } // end IF
  filter_p->Release (); filter_p = NULL;

//flip:
//  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_VMR9_L;
//  result = CoCreateInstance (CLSID_VideoMixingRenderer9, NULL,
//                             CLSCTX_INPROC_SERVER,
//                             IID_PPV_ARGS (&filter_p));
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
//                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_VideoMixingRenderer9).c_str ()),
//                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (filter_p);
//
//  IVMRFilterConfig9* vmr_filter_config_p = NULL;
//  result = filter_p->QueryInterface (IID_PPV_ARGS (&vmr_filter_config_p));
//  ACE_ASSERT (vmr_filter_config_p);
//  vmr_filter_config_p->SetNumberOfStreams (1);
//  vmr_filter_config_p->Release (); vmr_filter_config_p = NULL;
//
//  IVMRMixerControl9* vmr_mixer_control_p = NULL;
//  result = filter_p->QueryInterface (IID_PPV_ARGS (&vmr_mixer_control_p));
//  ACE_ASSERT (vmr_mixer_control_p);
//
//  // Get the current output rectangle and flip vertically
//  VMR9NormalizedRect rcRect;
//  vmr_mixer_control_p->GetOutputRect (0, &rcRect);
//  VMR9NormalizedRect rcNew;
//  rcNew.left = rcRect.left;
//  rcNew.right = rcRect.right;
//  rcNew.top = rcRect.bottom;
//  rcNew.bottom = rcRect.top;
//  vmr_mixer_control_p->SetOutputRect (0, &rcNew);
//  vmr_mixer_control_p->Release (); vmr_mixer_control_p = NULL;
//
//  result = IGraphBuilder_in->AddFilter (filter_p,
//                                        graph_entry.filterName.c_str ());
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
//    goto error;
//  } // end IF
//  filter_p->Release (); filter_p = NULL;
//  graphConfiguration_out.push_back (graph_entry); // VMR9
//  graph_entry.mediaType = NULL;

grab:
  if (skip_grab)
    goto render;

  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L;
  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_SampleGrabber).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  graphConfiguration_out.push_back (graph_entry); // sample grabber
  graph_entry.mediaType = NULL;

render:
  if (windowHandle_in)
  {
    // *NOTE*: connect()ing the 'sample grabber' to the 'EVR video renderer'
    //         breaks any connection between the 'AVI decompressor' and the
    //         'sample grabber' (go ahead, try it in with graphedit.exe)
    //         --> use the 'Video Renderer' instead
    // *TODO*: find out why this happens
    CLSID_s = STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER;
    if (Stream_MediaFramework_DirectShow_Tools::has (graphConfiguration_out,
                                                     STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI_L) &&
        Stream_MediaFramework_DirectShow_Tools::has (graphConfiguration_out,
                                                     STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB_L) &&
        !InlineIsEqualGUID (CLSID_s, CLSID_VideoRenderer))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("graph has 'AVI Decompressor' and 'Sample Grabber'; using default video renderer...\n")));
      CLSID_s = STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER;
    } // end IF
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO_L;
  } // end IF
  else
  {
    // *NOTE*: connect()ing the 'sample grabber' to the 'null renderer' breaks
    //         any connection between the 'AVI decompressor' and the 'sample
    //         grabber' (go ahead, try it in with graphedit.exe)
    CLSID_s = CLSID_NullRenderer;
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL_L;
  } // end ELSE
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_in->AddFilter (filter_p,
                                        graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  graphConfiguration_out.push_back (graph_entry); // renderer

#if defined (_DEBUG)
  Stream_MediaFramework_DirectShow_Tools::dump (graphConfiguration_out);
#endif // _DEBUG

  return true;

error:
  Stream_MediaFramework_DirectShow_Tools::clear (graphConfiguration_out);
  if (pin_p)
    pin_p->Release ();
  if (filter_p)
    filter_p->Release ();
  if (graph_entry.mediaType)
    Stream_MediaFramework_DirectShow_Tools::delete_ (graph_entry.mediaType, false);

  return false;
}

bool
Stream_Module_Decoder_Tools::loadTargetRendererGraph (IBaseFilter* sourceFilter_in,
                                                      const std::wstring& sourceFilterName_in,
                                                      const struct _AMMediaType& sourceMediaType_in,
                                                      HWND windowHandle_in,
                                                      IGraphBuilder*& IGraphBuilder_out,
                                                      IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                                      Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadTargetRendererGraph"));

  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;
  IBaseFilter* filter_2 = NULL;
  IBaseFilter* filter_3 = NULL;
  IPin* pin_p = NULL;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
  bool skip_decode = false;
  bool skip_resize = false;
  bool is_partially_connected = false;
  struct _GUID CLSID_s, CLSID_2;
  struct _GUID preferred_subtype = GUID_NULL;
  bool filter_is_dmo_wrapper = false;
  struct _AMMediaType* media_type_p = NULL;
  IDMOWrapperFilter* i_dmo_wrapper_filter_p = NULL;
  //struct _DMOMediaType* dmo_media_type_p = NULL;
  DMO_MEDIA_TYPE* dmo_media_type_p = NULL;
  IMediaObject* i_media_object_p = NULL;
  IWMResizerProps* i_wmresizer_props_p = NULL;
  DWORD dwFlags = 0;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  FOURCCMap fourcc_map;
#endif // DIRECTSHOW_BASECLASSES_SUPPORT

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::clear (graphConfiguration_out);

  if (!IGraphBuilder_out)
  {
    result =
      CoCreateInstance (CLSID_FilterGraph, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&IGraphBuilder_out));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (IGraphBuilder_out);

    IMediaFilter* media_filter_p = NULL;
    result = IGraphBuilder_out->QueryInterface (IID_PPV_ARGS (&media_filter_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaFilter): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (media_filter_p);
    result = media_filter_p->SetSyncSource (NULL);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaFilter::SetSyncSource(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    media_filter_p->Release ();
  } // end IF
  else
  {
    if (!Stream_MediaFramework_DirectShow_Tools::clear (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_out);
  ACE_ASSERT (!IAMBufferNegotiation_out);

  // instantiate source filter ?
  if (!sourceFilter_in)
  {
    // sanity check(s)
    if (!Stream_MediaFramework_DirectShow_Tools::has (IGraphBuilder_out,
                                                      sourceFilterName_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: not part of the filter graph, aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (sourceFilterName_in.c_str ())));
      goto error;
    } // end IF

    graph_entry.mediaType =
      Stream_MediaFramework_DirectShow_Tools::copy (sourceMediaType_in);
    if (!graph_entry.mediaType)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (graph_entry.mediaType);

    goto continue_;
  } // end IF

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (sourceFilter_in,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (sourceFilter_in).c_str ())));
    goto error;
  } // end IF
  if (!Stream_MediaFramework_DirectShow_Tools::getFirstFormat (pin_p,
                                                               GUID_NULL,
                                                               true, // top-to-bottom
                                                               graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (sourceFilter_in).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ())));
    goto error;
  } // end IF
  pin_p->Release ();
  pin_p = NULL;

  result = IGraphBuilder_out->AddFilter (sourceFilter_in,
                                         sourceFilterName_in.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName = sourceFilterName_in;
continue_:
  graphConfiguration_out.push_back (graph_entry);

  unsigned int source_width, width, source_height, height;
  if (InlineIsEqualGUID (graph_entry.mediaType->formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (InlineIsEqualGUID (sourceMediaType_in.formattype, FORMAT_VideoInfo));
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (graph_entry.mediaType->pbFormat);
    struct tagVIDEOINFOHEADER* video_info_header_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (sourceMediaType_in.pbFormat);
    source_height = abs (video_info_header_p->bmiHeader.biHeight);
    source_width = video_info_header_p->bmiHeader.biWidth;
    height = abs (video_info_header_2->bmiHeader.biHeight);
    width = video_info_header_2->bmiHeader.biWidth;
  } // end IF
  else if (InlineIsEqualGUID (graph_entry.mediaType->formattype, FORMAT_VideoInfo2))
  { ACE_ASSERT (InlineIsEqualGUID (sourceMediaType_in.formattype, FORMAT_VideoInfo2));
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (graph_entry.mediaType->pbFormat);
    struct tagVIDEOINFOHEADER2* video_info_header2_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (sourceMediaType_in.pbFormat);
    source_height = abs (video_info_header2_p->bmiHeader.biHeight);
    source_width = video_info_header2_p->bmiHeader.biWidth;
    height = abs (video_info_header2_2->bmiHeader.biHeight);
    width = video_info_header2_2->bmiHeader.biWidth;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown format type (was: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (graph_entry.mediaType->formattype).c_str ())));
    goto error;
  } // end ELSE
  //skip_resize = (source_height == height) && (source_width == width);
  skip_resize = true;

  media_type_p =
    Stream_MediaFramework_DirectShow_Tools::copy (*graph_entry.mediaType);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  graph_entry.mediaType = media_type_p;
  media_type_p = NULL;

  if (!Stream_MediaFramework_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
    goto decode;

decompress:
#if defined (DIRECTSHOW_BASECLASSES_SUPPORT)
  fourcc_map.SetFOURCC (&graph_entry.mediaType->subtype);
  switch (fourcc_map.GetFOURCC ())
#else
  switch (graph_entry.mediaType->subtype.Data1)
#endif // DIRECTSHOW_BASECLASSES_SUPPORT
  {
    // *** compressed types ***
    case FCC ('H264'):
    {
      CLSID_s = CLSID_CMPEG2VidDecoderDS;
      graph_entry.filterName =
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264_L;
      // *NOTE*: the EVR video renderer (!) can handle the nv12 chroma type
      //         --> do not decode
      preferred_subtype = MEDIASUBTYPE_NV12;
      skip_decode = true;
      break;
    }
    case FCC ('MJPG'):
    {
      CLSID_s = CLSID_MjpegDec;
      graph_entry.filterName =
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG_L;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  } // end SWITCH

  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_s).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);

  result = IGraphBuilder_out->AddFilter (filter_p,
                                         graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
    goto error;
  } // end IF
  filter_p->Release ();
  filter_p = NULL;
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // need another decompressor ?
  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_MediaFramework_DirectShow_Tools::countFormats (pin_p,
                                                             graphConfiguration_out.back ().mediaType->formattype))
  {
    if (!is_partially_connected)
    {
      is_partially_connected = true;

      if (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_out,
                                                            graphConfiguration_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("partially connected the graph\n")));
    } // end IF
  } // end IF

  if (!Stream_MediaFramework_DirectShow_Tools::getFirstFormat (pin_p,
                                                               preferred_subtype,
                                                               true, // top-to-bottom
                                                               graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (preferred_subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (graph_entry.mediaType);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: selected output format: %s"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*graph_entry.mediaType, true).c_str ())));

  if (Stream_MediaFramework_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
                                                      STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
    goto decompress;

  if (is_partially_connected)
  {
    is_partially_connected = false;

    if (!Stream_MediaFramework_DirectShow_Tools::disconnect (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("disconnected partially connected graph\n")));
  } // end IF
  pin_p->Release (); pin_p = NULL;

decode:
  if (InlineIsEqualGUID (graph_entry.mediaType->subtype, STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT) ||
      skip_decode)
    goto grab;

  preferred_subtype =
    STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_VIDEO_RENDERER_FORMAT;
  // *NOTE*: the Color Space Converter filter has a seriously broken allocator
  //         --> use the DMO instead
  if (Stream_MediaFramework_Tools::isRGB (graph_entry.mediaType->subtype,
                                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW) ||
      Stream_MediaFramework_Tools::isChromaLuminance (graph_entry.mediaType->subtype,
                                                      STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  //{
  //  CLSID_s = CLSID_Colour;
  //  graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  //} // end IF
  //else if (Stream_Module_Decoder_Tools::isChromaLuminance (graph_entry.mediaType->subtype,
  //                                                         STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  {
    //// *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    //CLSID_s = CLSID_AVIDec;
    CLSID_s = CLSID_DMOWrapperFilter;
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_YUV_L;
    filter_is_dmo_wrapper = true;
    CLSID_2 = CLSID_CColorConvertDMO;
  } // end ELSE IF
  else
  //FOURCCMap fourcc_map_2 (&media_type_p->subtype);
  //switch (fourcc_map_2.GetFOURCC ())
  //{
  //  // *** uncompressed types ***
  //  // RGB types
  //  case FCC ('RGBA'):
  //  {
  //    CLSID_s = CLSID_Colour;
  //    filter_name = STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  //    break;
  //  }
  //  // chroma-luminance types
  //  case FCC ('YUY2'):
  //  {
  //    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
  //    CLSID_s = CLSID_AVIDec;
  //    filter_name = STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  //    break;
  //  }
  //  default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (graph_entry.mediaType->subtype).c_str ())));
      goto error;
    }
  //} // end SWITCH

  // *TODO*: support receiving other formats
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                graph_entry.filterName.c_str (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  if (filter_is_dmo_wrapper)
  {
    result = filter_2->QueryInterface (IID_PPV_ARGS (&i_dmo_wrapper_filter_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (i_dmo_wrapper_filter_p);
    result = i_dmo_wrapper_filter_p->Init (CLSID_2,
                                           DMOCATEGORY_VIDEO_DECODER);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::Init(DMOCATEGORY_VIDEO_DECODER): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;
      goto error;
    } // end IF

    // set input type manually
    // *NOTE*: DMO_MEDIA_TYPE is actually a typedef for AM_MEDIA_TYPE, so this
    //         creates a copy
    dmo_media_type_p =
      Stream_MediaFramework_DirectShow_Tools::toDMOMediaType (*graph_entry.mediaType);
    if (!dmo_media_type_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_Common_DirectShow_Tools::toDMOMediaType(), aborting\n")));
      i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;
      goto error;
    } // end IF
    result = filter_2->QueryInterface (IID_PPV_ARGS (&i_media_object_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IMediaObject): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;
      Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (i_dmo_wrapper_filter_p);
    result = i_media_object_p->SetInputType (0,
                                             dmo_media_type_p,
                                             dwFlags);
    if (FAILED (result)) // E_INVALIDARG: 0x80070057
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMediaObject::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
      i_media_object_p->Release (); i_media_object_p = NULL;
      i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;
      goto error;
    } // end IF
    Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
    i_media_object_p->Release (); i_media_object_p = NULL;
    i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;
  } // end IF
  result =
    IGraphBuilder_out->AddFilter (filter_2,
                                  graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF

  pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_2,
                                                       PINDIR_OUTPUT);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
    goto error;
  } // end IF
  filter_2->Release (); filter_2 = NULL;
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // *NOTE*: IEnumMediaTypes::Next sometimes returns S_FALSE (e.g. Microsoft
  //         (TM) AVI/MJPG decoders) when the filter is not connected
  //         --> (partially) connect the graph (and retry)
  if (!Stream_MediaFramework_DirectShow_Tools::countFormats (pin_p,
                                                             graphConfiguration_out.back ().mediaType->formattype))
  {
    if (!is_partially_connected)
    {
      is_partially_connected = true;

      if (!Stream_MediaFramework_DirectShow_Tools::connect (IGraphBuilder_out,
                                                            graphConfiguration_out))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n")));
        goto error;
      } // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("partially connected the DirectShow filter graph\n")));
    } // end IF
  } // end IF

  if (!Stream_MediaFramework_DirectShow_Tools::getFirstFormat (pin_p,
                                                               preferred_subtype,
                                                               true, // top-to-bottom
                                                               graph_entry.mediaType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::getFirstFormat(\"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (preferred_subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
//#if defined (_DEBUG)
//    Stream_MediaFramework_DirectShow_Tools::countFormats (pin_p,
//                                                          GUID_NULL);
//#endif
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: selected output format: %s"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*graph_entry.mediaType, true).c_str ())));
  pin_p->Release (); pin_p = NULL;
  ACE_ASSERT (graph_entry.mediaType);

  if (is_partially_connected)
  {
    is_partially_connected = false;

    if (!Stream_MediaFramework_DirectShow_Tools::disconnect (IGraphBuilder_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n")));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("disconnected partially connected DirectShow filter graph\n")));
  } // end IF

  if (!Stream_MediaFramework_Tools::isRGB (graph_entry.mediaType->subtype,
                                           STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
    goto decode;

//resize:
  if (skip_resize)
    goto grab;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("scaling video %ux%u --> %ux%u\n"),
              source_width, source_height, width, height));

  CLSID_s = CLSID_DMOWrapperFilter;
  CLSID_2 = CLSID_CResizerDMO;
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                graph_entry.filterName.c_str (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  result = filter_2->QueryInterface (IID_PPV_ARGS (&i_dmo_wrapper_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (i_dmo_wrapper_filter_p);
  result = i_dmo_wrapper_filter_p->Init (CLSID_2, DMOCATEGORY_VIDEO_EFFECT);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDMOWrapperFilter::Init(DMOCATEGORY_VIDEO_EFFECT): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    i_dmo_wrapper_filter_p->Release ();
    goto error;
  } // end IF
  i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;

  result = filter_2->QueryInterface (IID_PPV_ARGS (&i_wmresizer_props_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IWMResizerProps): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (i_wmresizer_props_p);
  result =
    i_wmresizer_props_p->SetFullCropRegion (0, 0, source_width, source_height,
                                            0, 0, width, height);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IWMResizerProps::SetFullCropRegion(0,0,%u,%u,0,0,%u,%u): \"%s\", aborting\n"),
                source_width, source_height,
                width, height,
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    i_wmresizer_props_p->Release ();
    goto error;
  } // end IF
  result = i_wmresizer_props_p->SetInterlaceMode (FALSE); // <-- progressive
  ACE_ASSERT (SUCCEEDED (result));
  result = i_wmresizer_props_p->SetResizerQuality (FALSE); // <-- faster
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IWMResizerProps::SetResizerQuality(false): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    i_wmresizer_props_p->Release ();
    goto error;
  } // end IF
  i_wmresizer_props_p->Release (); i_wmresizer_props_p = NULL;

  // set input type manually
  // *NOTE*: DMO_MEDIA_TYPE is actually a typedef for AM_MEDIA_TYPE, so this
  //         creates a copy
  dmo_media_type_p =
    Stream_MediaFramework_DirectShow_Tools::toDMOMediaType (*graph_entry.mediaType);
  if (!dmo_media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Common_DirectShow_Tools::toDMOMediaType(), aborting\n")));
    goto error;
  } // end IF
  result = filter_2->QueryInterface (IID_PPV_ARGS (&i_media_object_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryInterface(IID_IMediaObject): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (i_media_object_p);
  result = i_media_object_p->SetInputType (0,
                                           dmo_media_type_p,
                                           dwFlags);
  if (FAILED (result)) // E_INVALIDARG: 0x80070057
  {                    // EVENT_E_INTERNALEXCEPTION: 0x80040205
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaObject::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
    i_media_object_p->Release (); i_media_object_p = NULL;
    goto error;
  } // end IF
  Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
  dmo_media_type_p =
    Stream_MediaFramework_DirectShow_Tools::toDMOMediaType (*graph_entry.mediaType);
  if (!dmo_media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Common_DirectShow_Tools::toDMOMediaType(), aborting\n")));
    i_media_object_p->Release (); i_media_object_p = NULL;
    goto error;
  } // end IF
  if (InlineIsEqualGUID (dmo_media_type_p->formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (dmo_media_type_p->pbFormat);
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (dmo_media_type_p->pbFormat);
    video_info_header_p->bmiHeader.biHeight = height;
    video_info_header_p->bmiHeader.biWidth = width;
    //video_info_header_p->bmiHeader.biSizeImage =
    //  DIBSIZE (video_info_header_p->bmiHeader);
    //video_info_header_p->dwBitRate =
    //  (video_info_header_p->bmiHeader.biSizeImage *
    //   video_info_header_p->bmiHeader.biBitCount  *
    //   30);
    //dmo_media_type_p->lSampleSize =
    //  video_info_header_p->bmiHeader.biSizeImage;
    SetRect (&video_info_header_p->rcSource,
             0, 0, source_width, source_height);
    SetRect (&video_info_header_p->rcTarget,
             0, 0, width, height);
  } // end IF
  else if (InlineIsEqualGUID (dmo_media_type_p->formattype, FORMAT_VideoInfo2))
  { ACE_ASSERT (dmo_media_type_p->pbFormat);
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (dmo_media_type_p->pbFormat);
    video_info_header2_p->bmiHeader.biHeight = height;
    video_info_header2_p->bmiHeader.biWidth = width;
    //video_info_header2_p->bmiHeader.biSizeImage =
    //  DIBSIZE (video_info_header2_p->bmiHeader);
    //video_info_header2_p->dwBitRate =
    //  (video_info_header2_p->bmiHeader.biSizeImage *
    //   video_info_header2_p->bmiHeader.biBitCount *
    //   30);
    //dmo_media_type_p->lSampleSize =
    //  video_info_header2_p->bmiHeader.biSizeImage;
    SetRect (&video_info_header2_p->rcSource,
             0, 0, source_width, source_height);
    SetRect (&video_info_header2_p->rcTarget,
             0, 0, width, height);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown format type (was: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (dmo_media_type_p->formattype).c_str ())));
    goto error;
  } // end ELSE
  result = i_media_object_p->SetOutputType (0,
                                            dmo_media_type_p,
                                            dwFlags);
  if (FAILED (result)) // E_INVALIDARG: 0x80070057
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaObject::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
    i_media_object_p->Release (); i_media_object_p = NULL;
    goto error;
  } // end IF
  i_media_object_p->Release (); i_media_object_p = NULL;

  graph_entry.filterName =
    STREAM_DEC_DIRECTSHOW_FILTER_NAME_RESIZER_VIDEO_L;
  result =
    IGraphBuilder_out->AddFilter (filter_2,
                                  graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    Stream_MediaFramework_DirectShow_Tools::delete_ (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p), true); dmo_media_type_p = NULL;
    goto error;
  } // end IF
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.connectDirect = true;
  graph_entry.mediaType =
    reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p);
  dmo_media_type_p = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: set output format: %s\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*graph_entry.mediaType, true).c_str ())));

  // *TODO*: add frame grabber support
grab:

//render:
  // render to a window (e.g. GtkDrawingArea) ?
  graph_entry.filterName =
    (windowHandle_in ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO_L
                     : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL_L);

  result =
    IGraphBuilder_out->FindFilterByName (graph_entry.filterName.c_str (),
                                         &filter_3);
  if (FAILED (result))
  {
    if (result != VFW_E_NOT_FOUND)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF

    CLSID_s =
      (windowHandle_in ? STREAM_LIB_DEFAULT_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER
                       : CLSID_NullRenderer);
    result = CoCreateInstance (CLSID_s, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&filter_3));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_s).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (filter_3);
    result =
      IGraphBuilder_out->AddFilter (filter_3,
                                    graph_entry.filterName.c_str ());
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added \"%s\"\n"),
    //            ACE_TEXT_WCHAR_TO_TCHAR (graph_entry.filterName.c_str ())));
  } // end IF
  ACE_ASSERT (filter_3);
  filter_3->Release (); filter_3 = NULL;
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  //result =
  //  ICaptureGraphBuilder2_in->RenderStream (//&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
  //                                          &PIN_CATEGORY_CAPTURE, NULL,
  //                                          filter_p,
  //                                          filter_2,
  //                                          //NULL,
  //                                          filter_4);
  //if (FAILED (result)) // E_INVALIDARG = 0x80070057, 0x80040217 = VFW_E_CANNOT_CONNECT ?
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to ICaptureGraphBuilder::RenderStream(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  //  return false;
  //} // end IF

  // clean up
  //if (filter_p)
  //  filter_p->Release ();

  if (!Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation (IGraphBuilder_out,
                                                                     sourceFilterName_in,
                                                                     IAMBufferNegotiation_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::getBufferNegotiation(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);

#if defined (_DEBUG)
  Stream_MediaFramework_DirectShow_Tools::dump (graphConfiguration_out);
#endif // _DEBUG

  return true;

error:
  Stream_MediaFramework_DirectShow_Tools::clear (graphConfiguration_out);
  //if (filter_p)
  //  filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (pin_p)
    pin_p->Release ();

  if (IGraphBuilder_out)
  {
    IGraphBuilder_out->Release (); IGraphBuilder_out = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Decoder_Tools::loadAudioRendererTopology (REFGUID deviceIdentifier_in,
                                                        REFGUID deviceCategory_in,
                                                        bool useFrameWorkSource_in,
                                                        IMFMediaType*& mediaType_inout,
                                                        const IMFMediaType* outputMediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                        IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                        IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                        int audioOutput_in,
                                                        REFGUID effect_in,
                                                        const std::string& effectOptions_in, // *TODO*
                                                        IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadAudioRendererTopology"));

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  IMFMediaType* media_type_p = NULL, *media_type_2 = NULL;
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = E_FAIL;
  DWORD number_of_source_nodes = 0, number_of_streams_i = 0;
  IUnknown* unknown_p = NULL;
  UINT32 item_count = 0;
  UINT32 flags = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  IMFActivate** decoders_p = NULL;
#else
  CLSID* decoders_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  flags = (MFT_ENUM_FLAG_SYNCMFT        |
           MFT_ENUM_FLAG_ASYNCMFT       |
           MFT_ENUM_FLAG_HARDWARE       |
           MFT_ENUM_FLAG_FIELDOFUSE     |
           MFT_ENUM_FLAG_LOCALMFT       |
           MFT_ENUM_FLAG_TRANSCODE_ONLY |
           MFT_ENUM_FLAG_SORTANDFILTER);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  struct _GUID GUID_s = GUID_NULL;
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
  //IMFAudioProcessorControl* audio_processor_control_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  BOOL selected_b = FALSE;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  DWORD characteristics_i = 0;
  IMFAttributes* attributes_p = NULL;
  bool is_current_format_b = true;
  bool add_tee_node_b = false;
  bool has_sink_b = false;

  if (topology_inout)
  {
    if (!Stream_MediaFramework_MediaFoundation_Tools::reset (topology_inout,
                                                             (useFrameWorkSource_in ? deviceCategory_in : GUID_NULL)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::reset(), aborting\n")));
      return false;
    } // end IF
    if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (topology_inout,
                                                                      media_source_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), aborting\n")));
      return false;
    } // end ELSE IF
  } // end IF
  else
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    struct Stream_Device_Identifier device_identifier;
    device_identifier.identifier._guid = deviceIdentifier_in;
    if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (device_identifier,
                                                                  deviceCategory_in,
                                                                  media_source_p,
                                                                  NULL, // do not load a dummy sink
                                                                  topology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    release_topology = true;
  } // end ELSE
  ACE_ASSERT (topology_inout);
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  result = topology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));
    collection_p->Release (); collection_p = NULL;
    goto error;
  } // end IF
  if (number_of_source_nodes > 1)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("topology contains several source nodes, continuing\n")));
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release (); unknown_p = NULL;
    goto error;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;

  // step1a: set capture media type
  if (mediaType_inout)
  {
    result = mediaType_inout->GetCount (&item_count);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF
  if (!mediaType_inout || !item_count)
  {
    if (!Stream_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                                mediaType_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default/preset capture format...\n")));
  } // end IF
  ACE_ASSERT (mediaType_inout);
  media_type_p =
    Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  result =
    media_source_p->CreatePresentationDescriptor (&presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result) && presentation_descriptor_p);
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &selected_b,
                                                           &stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result) && stream_descriptor_p);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release (); media_type_handler_p = NULL;

  result = source_node_p->SetOutputPrefType (0,
                                             media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  // step2: add decoder nodes ?
  mft_register_type_info.guidMajorType = MFMediaType_Audio;
  //BOOL is_compressed = false;
  //result = media_type_p->IsCompressedFormat (&is_compressed);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFMediaType::IsCompressedFormat(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF
  if (!Stream_MediaFramework_Tools::isCompressedAudio (sub_type,
                                                       STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
    goto continue_;

  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;
    item_count = 0;
    if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_AUDIO_DECODER,
                                                            flags,
                                                            &mft_register_type_info,    // input type
                                                            NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                            decoders_p,                 // array of decoders
#else
                                                            NULL,                       // attributes
                                                            decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                            item_count))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_AUDIO_DECODER).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (decoders_p);
    if (!item_count)
      goto clean;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    module_string =
      Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);
    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));

clean:
    for (UINT32 i = 0; i < item_count; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p); decoders_p = NULL;
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (false);
    ACE_NOTREACHED (return false;)

clean:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    if (!transform_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("could not find decoder (subtype was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto error;
    } // end IF

    result = transform_p->GetAttributes (&attributes_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    attributes_p->Release ();

    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DRAIN,
                                         MF_TOPONODE_DRAIN_ALWAYS);
    ACE_ASSERT (SUCCEEDED (result));

    result = topology_node_p->SetInputPrefType (0,
                                                media_type_p);
    ACE_ASSERT (SUCCEEDED (result));

    result = topology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release (); media_type_p = NULL;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                                       media_type_p,
                                                                       is_current_format_b))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (likely (!is_current_format_b))
    {
      result = transform_p->SetOutputType (0,
                                           media_type_p,
                                           0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
    } // end IF
    transform_p->Release (); transform_p = NULL;

    result = source_node_p->SetOutputPrefType (0,
                                               media_type_p);
    ACE_ASSERT (SUCCEEDED (result));

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%Q: added decoder for \"%s\": \"%s\"...\n"),
                node_id,
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                ACE_TEXT (module_string.c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (!Stream_MediaFramework_Tools::isCompressedAudio (sub_type,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
      break; // done
  } // end WHILE

continue_:
  // step3: add effect node ?
  if (InlineIsEqualGUID (effect_in, GUID_NULL))
    goto continue_2;

  mft_register_type_info.guidSubtype = sub_type;
  item_count = 0;
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_AUDIO_EFFECT,
                                                          flags,
                                                          &mft_register_type_info,    // input type
                                                          NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                          decoders_p,                 // array of decoders
#else
                                                          NULL,                       // attributes
                                                          decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                          item_count))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_AUDIO_EFFECT).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (decoders_p);
  if (!item_count)
    goto clean_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  for (UINT32 i = 0;
       i < item_count;
       ++i)
  {
    module_string =
      Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[i]);
    GUID_s = GUID_NULL;
    result = decoders_p[i]->GetGUID (MFT_TRANSFORM_CLSID_Attribute,
                                     &GUID_s);
    ACE_ASSERT (SUCCEEDED (result));
    if (!InlineIsEqualGUID (effect_in, GUID_s))
      continue;
    result =
      decoders_p[i]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));
    break;
  } // end FOR

clean_2:
  for (UINT32 i = 0; i < item_count; i++)
    decoders_p[i]->Release ();
  CoTaskMemFree (decoders_p); decoders_p = NULL;
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)

clean_2:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  if (!transform_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("could not find effect (CLSID was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (effect_in).c_str ())));
    goto error;
  } // end IF

  result = transform_p->GetAttributes (&attributes_p);
  if (SUCCEEDED (result) && attributes_p)
  {
    result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    attributes_p->Release (); attributes_p = NULL;
  } // end IF

  result = transform_p->SetInputType (0,
                                      media_type_p,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_DRAIN,
                                       MF_TOPONODE_DRAIN_ALWAYS);
  ACE_ASSERT (SUCCEEDED (result));

  result = topology_node_p->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  media_type_p->Release (); media_type_p = NULL;
  if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                                     media_type_p,
                                                                     is_current_format_b))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (media_type_p);
  if (likely (!is_current_format_b))
  {
retry:
    result = transform_p->SetOutputType (0,
                                         media_type_p,
                                         0);
    if (FAILED (result))
    {
      if ((result == MF_E_INVALIDMEDIATYPE) &&
          (Stream_MediaFramework_MediaFoundation_Tools::isPartial (media_type_p)))
      {
        if (!Stream_MediaFramework_MediaFoundation_Tools::merge (mediaType_inout,
                                                                 media_type_p,
                                                                 true)) // reconfigure ?
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::merge(), aborting\n")));
          transform_p->Release (); transform_p = NULL;
          goto error;
        } // end IF
        goto retry;
      } // end IF
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
  } // end IF
  transform_p->Release (); transform_p = NULL;

  result = source_node_p->SetOutputPrefType (0,
                                             media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%Q: added effect: \"%s\"...\n"),
              node_id,
              ACE_TEXT (module_string.c_str ())));

continue_2:
  // step3: add tee node ?
  add_tee_node_b = (sampleGrabberSinkCallback_in && (audioOutput_in >= 0));
  if (!add_tee_node_b)
    goto continue_3;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  result = topology_node_p->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  result = source_node_p->SetOutputPrefType (0,
                                             media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%Q: added tee node...\n"),
              node_id));

continue_3:
  // step4: add sample grabber sink ?
  if (!sampleGrabberSinkCallback_in)
    goto continue_4;

  // step4a: add resampler ?
  if (outputMediaType_in &&
      !Stream_MediaFramework_MediaFoundation_Tools::match (outputMediaType_in,
                                                           media_type_p))
  {
    node_id = 0;
    if (!Stream_MediaFramework_MediaFoundation_Tools::addResampler (outputMediaType_in,
                                                                    topology_inout,
                                                                    node_id))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addResampler(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (node_id);
    ACE_ASSERT (!topology_node_p);
    result = topology_inout->GetNodeByID (node_id,
                                          &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result) && topology_node_p);
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    if (!add_tee_node_b) // set new output type ?
    {
      media_type_p->Release (); media_type_p = NULL;
      media_type_p =
        Stream_MediaFramework_MediaFoundation_Tools::copy (outputMediaType_in);
      ACE_ASSERT (media_type_p);
    } // end IF
  } // end IF

  node_id = 0;
  if (!Stream_MediaFramework_MediaFoundation_Tools::addGrabber (sampleGrabberSinkCallback_in,
                                                                topology_inout,
                                                                node_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addGrabber(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (node_id);
  ACE_ASSERT (!topology_node_p);
  result = topology_inout->GetNodeByID (node_id,
                                        &topology_node_p);
  ACE_ASSERT (SUCCEEDED (result) && topology_node_p);
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;
  has_sink_b = true;

continue_4:
  // step5: add audio renderer sink ?
  if ((audioOutput_in < 0) && has_sink_b)
    goto continue_5;

  // step5a: add resampler ?
  if ((audioOutput_in >= 0) &&
      !Stream_MediaFramework_MediaFoundation_Tools::canRender (media_type_p,
                                                               media_type_2))
  {
    if (Stream_MediaFramework_MediaFoundation_Tools::isPartial (media_type_2) &&
        !Stream_MediaFramework_MediaFoundation_Tools::merge (media_type_p,
                                                             media_type_2,
                                                             true)) // reconfigure ?
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::merge(), aborting\n")));
      media_type_2->Release (); media_type_2 = NULL;
      goto error;
    } // end IF
    UINT32 value_i = 0;
    result = media_type_2->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                      &value_i);
    ACE_ASSERT (SUCCEEDED (result) && value_i);
    if (unlikely (value_i != 2))
    {
      result = media_type_2->SetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                        2);
      ACE_ASSERT (SUCCEEDED (result));
      Stream_MediaFramework_MediaFoundation_Tools::reconfigure (media_type_2);
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("modified output media type to stereo (#channels was: %d), continuing\n"),
                  value_i));
    } // end IF

    node_id = 0;
    if (!Stream_MediaFramework_MediaFoundation_Tools::addResampler (media_type_2,
                                                                    topology_inout,
                                                                    node_id))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addResampler(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (node_id);
    if (media_type_2)
    {
      media_type_p->Release (); media_type_p = NULL;
      media_type_p = media_type_2;
    } // end IF
  } // end IF

  GUID_s =
    ((audioOutput_in < 0) ? GUID_NULL
                          : Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (audioOutput_in,
                                                                                                    false)); // playback
  node_id = 0;
  if (!Stream_MediaFramework_MediaFoundation_Tools::addRenderer (MFMediaType_Audio,
                                                                 NULL,
                                                                 GUID_s,
                                                                 topology_inout,
                                                                 node_id,
                                                                 true))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addRenderer(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (node_id);

continue_5:
  source_node_p->Release (); source_node_p = NULL;
  media_type_p->Release (); media_type_p = NULL;
  media_source_p->Release (); media_source_p = NULL;

  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    topology_inout->Release (); topology_inout = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Decoder_Tools::loadVideoRendererTopology (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                        const IMFMediaType* mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                        IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                        IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                        HWND windowHandle_in,
                                                        TOPOID& sampleGrabberSinkNodeId_out,
                                                        TOPOID& rendererNodeId_out,
                                                        IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadVideoRendererTopology"));

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  IMFMediaType* media_type_p = NULL;
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = E_FAIL;
  DWORD number_of_source_nodes = 0;
  IUnknown* unknown_p = NULL;
  UINT32 item_count = 0;
  UINT32 flags = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  flags = (MFT_ENUM_FLAG_SYNCMFT        |
           MFT_ENUM_FLAG_ASYNCMFT       |
           MFT_ENUM_FLAG_HARDWARE       |
           MFT_ENUM_FLAG_FIELDOFUSE     |
           MFT_ENUM_FLAG_LOCALMFT       |
           MFT_ENUM_FLAG_TRANSCODE_ONLY |
           MFT_ENUM_FLAG_SORTANDFILTER);
  IMFActivate** decoders_p = NULL;
#else
  CLSID* decoders_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  IMFVideoProcessorControl2* video_processor_control_p = NULL;
#elif COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  IMFVideoProcessorControl* video_processor_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00)
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;
  bool is_current_format_b = true;

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;
  if (!topology_inout)
  {
    if (!Stream_Device_MediaFoundation_Tools::getMediaSource (deviceIdentifier_in,
                                                              MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                              media_source_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (media_source_p);

    if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (media_source_p,
                                                                mediaType_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));
      goto error;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
    if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (deviceIdentifier_in,
                                                                  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                  media_source_p,
                                                                  NULL,
                                                                  topology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
    release_topology = true;
  } // end IF
  else if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (topology_inout,
                                                                         media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), aborting\n")));
    goto error;
  } // end ELSE IF
  ACE_ASSERT (topology_inout);
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  result = topology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));
    collection_p->Release (); collection_p = NULL;
    goto error;
  } // end IF
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release (); unknown_p = NULL;
    goto error;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;

  // step1a: set default capture media type ?
  result = const_cast<IMFMediaType*> (mediaType_in)->GetCount (&item_count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  if (!item_count || (item_count == 4))
  {
    if (!Stream_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                                media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: using default/preset capture format...\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_source_p).c_str ())));
  } // end IF
  else
  {
    media_type_p =
      Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in);
    if (!media_type_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n")));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (media_type_p);
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  // step2: add decoder nodes ?
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  if (!Stream_MediaFramework_Tools::isCompressedVideo (sub_type,
                                                       STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
    goto transform;

  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;
    item_count = 0;
    if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
                                                            flags,
                                                            &mft_register_type_info,    // input type
                                                            NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                            decoders_p,                 // array of decoders
#else
                                                            NULL,                       // attributes
                                                            decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                            item_count))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (decoders_p);
    if (!item_count)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto clean;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
    module_string =
      Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));

clean:
    for (UINT32 i = 0; i < item_count; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p); decoders_p = NULL;
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (false);
    ACE_NOTREACHED (return false;)

clean:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
    ACE_ASSERT (transform_p);

    //result = transform_p->GetAttributes (&attributes_p);
    //ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    //attributes_p->Release ();

    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added transform node (id: %Q)...\n"),
    //            node_id));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release (); media_type_p = NULL;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                                       media_type_p,
                                                                       is_current_format_b))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (likely (!is_current_format_b))
    {
      result = transform_p->SetOutputType (0,
                                           media_type_p,
                                           0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
    } // end IF
    transform_p->Release (); transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%Q: added decoder for \"%s\": \"%s\"...\n"),
                node_id,
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                ACE_TEXT (module_string.c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: output format: \"%s\"...\n"),
                ACE_TEXT (module_string.c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
#endif // _DEBUG

    if (!Stream_MediaFramework_Tools::isCompressedVideo (sub_type,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
      break; // done
  } // end WHILE

transform:
  // transform to RGB ?
  if (Stream_MediaFramework_Tools::isRGB (sub_type,
                                          STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
    goto continue_;

  // *NOTE*: (for reasons yet unknown,) the captured frames are flipped
  //         vertically --> adjust input format accordingly
  // *TODO*: find out why that is
  //result = media_type_p->SetUINT32 (MF_MT_VIDEO_ROTATION,
  //                                  MFVideoRotationFormat_180); // *NOTE*: ccw
  //ACE_ASSERT (SUCCEEDED (result));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("flipping output ...\n")));

  mft_register_type_info.guidSubtype = sub_type;

  decoders_p = NULL;
  item_count = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_PROCESSOR,
#else
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                          flags,
                                                          &mft_register_type_info,    // input type
                                                          NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                          decoders_p,                 // array of decoders
#else
                                                          NULL,                       // attributes
                                                          decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                          item_count))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (decoders_p);
  if (!item_count)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
    goto clean_2;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  module_string =
    Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

  result = decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
  ACE_ASSERT (SUCCEEDED (result));

clean_2:
  for (UINT32 i = 0; i < item_count; i++)
    decoders_p[i]->Release ();
  CoTaskMemFree (decoders_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)

clean_2:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  ACE_ASSERT (transform_p);

  result = transform_p->SetInputType (0,
                                      media_type_p,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added transform node (id: %Q)...\n"),
  //            node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  i = 0;
  while (!Stream_MediaFramework_Tools::isRGB (sub_type,
                                              STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
  {
    media_type_p->Release (); media_type_p = NULL;
    result = transform_p->GetOutputAvailableType (0,
                                                  i,
                                                  &media_type_p);

    ACE_ASSERT (SUCCEEDED (result));
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    ++i;
  } // end WHILE
  //result = media_type_p->DeleteAllItems ();
  //ACE_ASSERT (SUCCEEDED (result));
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_FRAME_RATE);
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_FRAME_SIZE);
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_INTERLACE_MODE);
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_PIXEL_ASPECT_RATIO);
  result = transform_p->SetOutputType (0,
                                       media_type_p,
                                       0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
#if defined (_DEBUG)
  media_type_p->Release (); media_type_p = NULL;
  result = transform_p->GetOutputCurrentType (0,
                                              &media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: output format: \"%s\"...\n"),
              ACE_TEXT (module_string.c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
#endif // _DEBUG

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  if (FAILED (result))
    goto continue_near;
  ACE_ASSERT (video_processor_control_p);
  //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
  //ACE_ASSERT (SUCCEEDED (result));
  //result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
  //ACE_ASSERT (SUCCEEDED (result));
  //result = video_processor_control_p->EnableHardwareEffects (TRUE);
  //ACE_ASSERT (SUCCEEDED (result));
  result =
    video_processor_control_p->SetRotationOverride (MFVideoRotationFormat_180);
  ACE_ASSERT (SUCCEEDED (result));
  video_processor_control_p->Release (); video_processor_control_p = NULL;
continue_near:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  transform_p->Release (); transform_p = NULL;

  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%Q: added processor for \"%s\": \"%s\"...\n"),
              node_id,
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
              ACE_TEXT (module_string.c_str ())));

continue_:
  // step3: add tee node ?
  if ((!sampleGrabberSinkCallback_in && !windowHandle_in) ||
      ((sampleGrabberSinkCallback_in && !windowHandle_in) || // XOR
      (!sampleGrabberSinkCallback_in &&  windowHandle_in)))
    goto continue_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %Q)...\n"),
              node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

continue_2:
  // step4: add sample grabber sink ?
  if (!sampleGrabberSinkCallback_in)
    goto continue_3;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
                                       sampleGrabberSinkCallback_in,
                                       &activate_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release (); activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release (); media_sink_p = NULL;
  //result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->SetCurrentMediaType (media_type_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->Release ();

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&sampleGrabberSinkNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%Q: added sample grabber sink node...\n"),
              sampleGrabberSinkNodeId_out));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release (); topology_node_p = NULL;

continue_3:
  // step5: add video renderer sink ?
  if (!windowHandle_in)
    goto continue_4;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  ACE_ASSERT (activate_p);

  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release (); activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release (); media_sink_p = NULL;
  media_type_handler_p = NULL;
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release (); media_type_handler_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %Q)...\n"),
              rendererNodeId_out));
  result =
    source_node_p->ConnectOutput ((sampleGrabberSinkCallback_in ? 1 : 0),
                                  topology_node_p,
                                  0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release (); topology_node_p = NULL;
  media_source_p->Release (); media_source_p = NULL;
  media_type_p->Release (); media_type_p = NULL;
  source_node_p->Release (); source_node_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration (topology_inout))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration(), continuing\n")));

continue_4:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    topology_inout->Release (); topology_inout = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Decoder_Tools::loadVideoRendererTopology (const IMFMediaType* mediaType_in,
                                                        const HWND windowHandle_in,
                                                        TOPOID& rendererNodeId_out,
                                                        IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadVideoRendererTopology"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (topology_inout);

  bool release_topology = false;
  struct _GUID sub_type, sub_type_2 = GUID_NULL;
  IMFMediaType* media_type_p = NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  IMFActivate* activate_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  std::string module_string;
  IMFTopologyNode* source_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = E_FAIL;
  DWORD number_of_source_nodes = 0;
  IUnknown* unknown_p = NULL;
  UINT32 item_count = 0;
  UINT32 flags = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  flags = (MFT_ENUM_FLAG_SYNCMFT        |
           MFT_ENUM_FLAG_ASYNCMFT       |
           MFT_ENUM_FLAG_HARDWARE       |
           MFT_ENUM_FLAG_FIELDOFUSE     |
           MFT_ENUM_FLAG_LOCALMFT       |
           MFT_ENUM_FLAG_TRANSCODE_ONLY |
           MFT_ENUM_FLAG_SORTANDFILTER);
  IMFActivate** decoders_p = NULL;
#else
  CLSID* decoders_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  IMFVideoProcessorControl2* video_processor_control_p = NULL;
#elif COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  IMFVideoProcessorControl* video_processor_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) || COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;
  BOOL is_compressed = false;
  bool is_current_format_b = true;

  // initialize return value(s)
  rendererNodeId_out = 0;
  
  if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (topology_inout,
                                                                    media_source_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), aborting\n")));
    goto error;
  } // end ELSE IF
  ACE_ASSERT (media_source_p);

  // step1: retrieve source node
  result = topology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));
    collection_p->Release (); collection_p = NULL;
    goto error;
  } // end IF
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release (); unknown_p = NULL;
    goto error;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;

  // step2: (try to-) add decoder nodes ?
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  result =
    const_cast<IMFMediaType*> (mediaType_in)->IsCompressedFormat (&is_compressed);
  ACE_ASSERT (SUCCEEDED (result));
  if (!is_compressed)
    goto transform;

  result =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
                                                       &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_type_p =
    Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;
    item_count = 0;
    if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
                                                            flags,
                                                            &mft_register_type_info,    // input type
                                                            NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                            decoders_p,                 // array of decoders
#else
                                                            NULL,                       // attributes
                                                            decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                            item_count))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (decoders_p);
    if (!item_count)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto clean;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
    module_string =
      Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));

clean:
    for (UINT32 i = 0; i < item_count; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p); decoders_p = NULL;

    //result = transform_p->GetAttributes (&attributes_p);
    //ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    //ACE_ASSERT (SUCCEEDED (result));
    //attributes_p->Release ();
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)

clean:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
    ACE_ASSERT (transform_p);

    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                         MF_CONNECT_DIRECT);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                         TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("added transform node (id: %Q)...\n"),
    //            node_id));
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release (); media_type_p = NULL;
    if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (transform_p,
                                                                       media_type_p,
                                                                       is_current_format_b))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (likely (!is_current_format_b))
    {
      result = transform_p->SetOutputType (0,
                                           media_type_p,
                                           0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
    } // end IF
    transform_p->Release (); transform_p = NULL;

    sub_type_2 = sub_type;
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%Q: added decoder \"%s\": \"%s\" --> \"%s\"...\n"),
                node_id,
                ACE_TEXT (module_string.c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type_2, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

    result = media_type_p->IsCompressedFormat (&is_compressed);
    ACE_ASSERT (SUCCEEDED (result));
    if (!is_compressed)
      break; // done
  } // end WHILE

transform:
  // transform to RGB ?
  // *NOTE*: apparently, the default Video Processer MFT from Micrsoft cannot
  //         transform NV12 to RGB (IMFTopoLoder::Load() fails:
  //         MF_E_INVALIDMEDIATYPE). As the EVR can handle most ChromaLuminance
  //         formats directly, skip this step
  if (Stream_MediaFramework_Tools::isRGB (sub_type,
                                          STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION) ||
      Stream_MediaFramework_Tools::isChromaLuminance (sub_type,
                                                      STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
    goto continue_;

  mft_register_type_info.guidSubtype = sub_type;

  decoders_p = NULL;
  item_count = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_PROCESSOR,
#else
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                          flags,
                                                          &mft_register_type_info,    // input type
                                                          NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                          decoders_p,                 // array of decoders
#else
                                                          NULL,                       // attributes
                                                          decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                          item_count))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (decoders_p);
  if (!item_count)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
    goto clean_2;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  module_string =
    Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

  result = decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
  ACE_ASSERT (SUCCEEDED (result));

clean_2:
  for (UINT32 i = 0; i < item_count; i++)
    decoders_p[i]->Release ();
  CoTaskMemFree (decoders_p); decoders_p = NULL;
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)

clean_2:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
  ACE_ASSERT (transform_p);

  result = transform_p->SetInputType (0,
                                      media_type_p,
                                      0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added transform node (id: %Q)...\n"),
  //            node_id));
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
  source_node_p->Release ();
  source_node_p = topology_node_p;
  topology_node_p = NULL;

  i = 0;
  while (!Stream_MediaFramework_Tools::isRGB (sub_type,
                                              STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
  {
    media_type_p->Release (); media_type_p = NULL;
    result = transform_p->GetOutputAvailableType (0,
                                                  i,
                                                  &media_type_p);

    ACE_ASSERT (SUCCEEDED (result));
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    ++i;
  } // end WHILE
  //result = media_type_p->DeleteAllItems ();
  //ACE_ASSERT (SUCCEEDED (result));
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_FRAME_RATE);
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_FRAME_SIZE);
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_INTERLACE_MODE);
  Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in,
                                                     media_type_p,
                                                     MF_MT_PIXEL_ASPECT_RATIO);
  result = transform_p->SetOutputType (0,
                                       media_type_p,
                                       0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    transform_p->Release (); transform_p = NULL;
    goto error;
  } // end IF
#if defined (_DEBUG)
  media_type_p->Release (); media_type_p = NULL;
  result = transform_p->GetOutputCurrentType (0,
                                              &media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("output format: \"%s\"...\n"),
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p).c_str ())));
#endif // _DEBUG

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: (for some unknown reason,) this does nothing...
  result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
  //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
  ACE_ASSERT (SUCCEEDED (result));
  video_processor_control_p->Release (); video_processor_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
  transform_p->Release (); transform_p = NULL;

  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  ACE_ASSERT (SUCCEEDED (result));
  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%Q: added processor \"%s\": \"%s\" --> \"%s\"...\n"),
              node_id,
              ACE_TEXT (module_string.c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

continue_:
  // step5: add video renderer sink ?
  //if (!windowHandle_in)
  //  goto continue_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  ACE_ASSERT (activate_p);

  // MF_ACTIVATE_CUSTOM_VIDEO_MIXER_ACTIVATE
  // MF_ACTIVATE_CUSTOM_VIDEO_MIXER_CLSID 
  // MF_ACTIVATE_CUSTOM_VIDEO_MIXER_FLAGS 
  // MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_ACTIVATE
  // MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID
  // MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_FLAGS 
  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  module_string =
    Stream_MediaFramework_MediaFoundation_Tools::toString (activate_p);

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release (); activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release (); media_sink_p = NULL;
  media_type_handler_p = NULL;
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->SetCurrentMediaType (media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release (); media_type_handler_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%Q: added renderer \"%s\": \"%s\" -->...\n"),
              rendererNodeId_out,
              ACE_TEXT (module_string.c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

  result =
    source_node_p->ConnectOutput (0,
                                  topology_node_p,
                                  0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release (); topology_node_p = NULL;
  media_source_p->Release (); media_source_p = NULL;
  media_type_p->Release (); media_type_p = NULL;
  source_node_p->Release (); source_node_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration (topology_inout))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration(), continuing\n")));

//continue_2:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();

  return false;
}

bool
Stream_Module_Decoder_Tools::loadTargetRendererTopology (const std::string& URL_in,
                                                         const IMFMediaType* mediaType_in,
                                                         HWND windowHandle_in,
                                                         TOPOID& rendererNodeId_out,
                                                         IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadTargetRendererTopology"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { 0 };
  IMFActivate* activate_p = NULL;
  IMFMediaType* media_type_p = NULL;
  IMFTopologyNode* source_node_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;

  // initialize return value(s)
  rendererNodeId_out = 0;

  if (!topology_inout)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    if (!Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology (URL_in,
                                                                          media_source_p,
                                                                          topology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (URL_in.c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (media_source_p);
    release_topology = true;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  } // end IF
  ACE_ASSERT (topology_inout);

  // step1: retrieve source node
  IMFCollection* collection_p = NULL;
  HRESULT result =
    topology_inout->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));
    collection_p->Release (); collection_p = NULL;
    goto error;
  } // end IF
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&source_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release (); unknown_p = NULL;
    goto error;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;

  // step2: add decoder nodes ?
  UINT32 item_count = 0;
  mft_register_type_info.guidMajorType = MFMediaType_Video;
  UINT32 flags = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  flags = (MFT_ENUM_FLAG_SYNCMFT        |
           MFT_ENUM_FLAG_ASYNCMFT       |
           MFT_ENUM_FLAG_HARDWARE       |
           MFT_ENUM_FLAG_FIELDOFUSE     |
           MFT_ENUM_FLAG_LOCALMFT       |
           MFT_ENUM_FLAG_TRANSCODE_ONLY |
           MFT_ENUM_FLAG_SORTANDFILTER);
  IMFActivate** decoders_p = NULL;
#else
  CLSID* decoders_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
  IMFTransform* transform_p = NULL;
  if (!media_source_p)
  {
    result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&media_source_p));
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  if (!Stream_Device_MediaFoundation_Tools::getCaptureFormat (media_source_p,
                                                              media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF
  media_source_p->Release (); media_source_p = NULL;
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  if (!Stream_MediaFramework_Tools::isCompressedVideo (sub_type,
                                                       STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
    goto continue_;

  while (true)
  {
    mft_register_type_info.guidSubtype = sub_type;
    item_count = 0;
    if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
                                                            flags,
                                                            &mft_register_type_info,    // input type
                                                            NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                            decoders_p,                 // array of decoders
#else
                                                            NULL,                       // attributes
                                                            decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                            item_count))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                  ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (decoders_p);
    if (!item_count)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
      goto error;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));

    for (UINT32 i = 0; i < item_count; i++)
      decoders_p[i]->Release ();
    CoTaskMemFree (decoders_p); decoders_p = NULL;
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (false);
    ACE_NOTREACHED (return false;)

clean:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
    ACE_ASSERT (transform_p);

    result = transform_p->SetInputType (0,
                                        media_type_p,
                                        0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
    result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                   &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
    result = topology_node_p->SetObject (transform_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_inout->AddNode (topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = source_node_p->ConnectOutput (0,
                                           topology_node_p,
                                           0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    source_node_p->Release ();
    source_node_p = topology_node_p;
    topology_node_p = NULL;

    media_type_p->Release (); media_type_p = NULL;
    result = transform_p->GetOutputCurrentType (0,
                                                &media_type_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    transform_p->Release (); transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("added decoder for \"%s\"...\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    if (!Stream_MediaFramework_Tools::isCompressedVideo (sub_type,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
      break; // done
  } // end WHILE
  media_type_p->Release (); media_type_p = NULL;

continue_:
  // step5: add video renderer sink ?
  if (!windowHandle_in)
    goto continue_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  ACE_ASSERT (activate_p);

  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  result = topology_node_p->SetObject (activate_p);
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release (); activate_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_inout->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = source_node_p->ConnectOutput (0,
                                         topology_node_p,
                                         0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_node_p->Release (); topology_node_p = NULL;
  source_node_p->Release (); source_node_p = NULL;

continue_2:
  return true;

error:
  if (media_source_p)
    media_source_p->Release ();
  if (media_type_p)
    media_type_p->Release ();
  if (source_node_p)
    source_node_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (activate_p)
    activate_p->Release ();
  if (release_topology)
  {
    topology_inout->Release (); topology_inout = NULL;
  } // end IF

  return false;
}

bool
Stream_Module_Decoder_Tools::updateRendererTopology (IMFTopology* topology_in,
                                                     const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::updateRendererTopology"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (mediaType_in);

  HRESULT result = E_FAIL;
  TOPOLOGY_PATHS_T paths_s;
  MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  TOPOID node_id = 0;
  bool next_path_b = false;
  IMFMediaType* media_type_p = NULL;

  // step1: walk all topology paths from source to the first transform/sink node
  //        resetting input/output types
  if (!Stream_MediaFramework_MediaFoundation_Tools::parse (topology_in,
                                                           paths_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::parse(), aborting\n")));
    goto error;
  } // end IF

  for (TOPOLOGY_PATHS_ITERATOR_T iterator = paths_s.begin ();
       iterator != paths_s.end ();
       ++iterator)
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
         iterator_2 != (*iterator).end ();
         ++iterator_2)
    {
      result = (*iterator_2)->GetNodeType (&node_type_e);
      ACE_ASSERT (SUCCEEDED (result));
      result = (*iterator_2)->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      switch (node_type_e)
      {
        case MF_TOPOLOGY_SOURCESTREAM_NODE:
        {
          if (!Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat (topology_in,
                                                                             node_id,
                                                                             const_cast<IMFMediaType*> (mediaType_in)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat(), aborting\n")));
            goto error;
          } // end IF
          break;
        }
        case MF_TOPOLOGY_OUTPUT_NODE:
        {
          if (!Stream_MediaFramework_MediaFoundation_Tools::setInputFormat (topology_in,
                                                                            node_id,
                                                                            const_cast<IMFMediaType*> (mediaType_in)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setInputFormat(), aborting\n")));
            goto error;
          } // end IF
          break;
        }
        case MF_TOPOLOGY_TRANSFORM_NODE:
        {
          if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_in,
                                                                             node_id,
                                                                             media_type_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setInputFormat(), aborting\n")));
            goto error;
          } // end IF
          ACE_ASSERT (media_type_p);
          if (!Stream_MediaFramework_MediaFoundation_Tools::setInputFormat (topology_in,
                                                                            node_id,
                                                                            const_cast<IMFMediaType*> (mediaType_in)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setInputFormat(), aborting\n")));
            goto error;
          } // end IF
          if (!Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat (topology_in,
                                                                             node_id,
                                                                             media_type_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat(), aborting\n")));
            goto error;
          } // end IF
          media_type_p->Release (); media_type_p = NULL;
          next_path_b = true;
          break;
        }
        case MF_TOPOLOGY_TEE_NODE:
        {
          if (!Stream_MediaFramework_MediaFoundation_Tools::setInputFormat (topology_in,
                                                                            node_id,
                                                                            const_cast<IMFMediaType*> (mediaType_in)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setInputFormat(), aborting\n")));
            goto error;
          } // end IF
          if (!Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat (topology_in,
                                                                             node_id,
                                                                             const_cast<IMFMediaType*> (mediaType_in)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat(), aborting\n")));
            goto error;
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                      node_type_e));
          goto error;
        }
      } // end SWITCH
      if (next_path_b)
      {
        next_path_b = false;
        break;
      } // end IF
    } // end FOR
  Stream_MediaFramework_MediaFoundation_Tools::clean (paths_s);

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  Stream_MediaFramework_MediaFoundation_Tools::clean (paths_s);

  return false;
}
#endif // ACE_WIN32 || ACE_WIN64
