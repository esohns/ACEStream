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

#include "ace/Synch.h"
#include "stream_dec_tools.h"

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <winnt.h>
#include <guiddef.h>
#include <amvideo.h>
//#include <combaseapi.h>
#include <dmodshow.h>
#include <dmoreg.h>
// *WARNING*: "...Note Header files ksproxy.h and dsound.h define similar but
//            incompatible versions of the IKsPropertySet interface.
//            Applications that require the KS proxy module should use the
//            version defined in ksproxy.h.The DirectSound version of
//            IKsPropertySet is described in the DirectSound reference pages in
//            the Microsoft Windows SDK documentation.
//            If an application must include both ksproxy.h and dsound.h,
//            whichever header file the compiler scans first is the one whose
//            definition of IKsPropertySet is used by the compiler. ..."
//#include <MMReg.h>
#include <WinNT.h>
#include <Guiddef.h>
#include <Ks.h>
#include <KsProxy.h>
#include <MMSystem.h>
#define INITGUID
#include <dsound.h>
#include <dvdmedia.h>
#include <fourcc.h>
#include <mediaobj.h>
#include <mfapi.h>
#include <mfidl.h>
#include <qedit.h>
#include <strmif.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <vfwmsgs.h>
#include <wmcodecdsp.h>
#endif // ACE_WIN32 || ACE_WIN64

#include <cmath>

#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"

#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/pixfmt.h"

#include "libswscale/swscale.h"
}
#endif // __cplusplus

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_string_tools.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_tools.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_common.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_defines.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_defines.h"

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

//void
//Stream_Module_Decoder_Tools::initialize ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::initialize"));
//
//}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Module_Decoder_Tools::isCompressed (REFGUID subType_in,
                                           REFGUID deviceCategory_in,
                                           enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isCompressed"));

  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
        return Stream_Module_Decoder_Tools::isCompressedAudio (subType_in,
                                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
      if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
        return Stream_Module_Decoder_Tools::isCompressedVideo (subType_in,
                                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW);

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      // *TODO*
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

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
}

bool
Stream_Module_Decoder_Tools::isCompressedAudio (REFGUID subType_in,
                                                enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isCompressedAudio"));

  // *TODO*: this is probably incomplete
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      return (!InlineIsEqualGUID (subType_in, MEDIASUBTYPE_PCM) &&
              !InlineIsEqualGUID (subType_in, MEDIASUBTYPE_IEEE_FLOAT));
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      return (!InlineIsEqualGUID (subType_in, MFAudioFormat_PCM) &&
              !InlineIsEqualGUID (subType_in, MFAudioFormat_Float));
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      break;
    }
  } // end SWITCH

  return false;
}

bool
Stream_Module_Decoder_Tools::isCompressedVideo (REFGUID subType_in,
                                                enum Stream_MediaFramework_Type mediaFramework_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isCompressedVideo"));

  // *TODO*: this is probably incomplete
  return (!Stream_MediaFramework_Tools::isRGB (subType_in,
                                               mediaFramework_in) &&
          !Stream_MediaFramework_Tools::isChromaLuminance (subType_in,
                                                           mediaFramework_in));
}
#endif

bool
Stream_Module_Decoder_Tools::isCompressedVideo (enum AVPixelFormat format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::isCompressedVideo"));

  return (!Stream_Module_Decoder_Tools::isRGB (format_in) &&
          !Stream_Module_Decoder_Tools::isChromaLuminance (format_in));
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
    // *NOTE*: libav does not specify a pixel format for MJPEG, it is a
    //         'compressed' format) --> map this deprecated format
//    case AV_PIX_FMT_YUVJ422P:  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
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
#endif
      return true;
    default: break;
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
    case AV_PIX_FMT_RGBA64BE:     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    case AV_PIX_FMT_RGBA64LE:     ///< packed RGBA 16:16:16:16, 64bpp, 16R, 16G, 16B, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    case AV_PIX_FMT_BGRA64BE:     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as big-endian
    case AV_PIX_FMT_BGRA64LE:     ///< packed RGBA 16:16:16:16, 64bpp, 16B, 16G, 16R, 16A, the 2-byte value for each R/G/B/A component is stored as little-endian
    case AV_PIX_FMT_0RGB:        ///< packed RGB 8:8:8, 32bpp, XRGBXRGB...   X=unused/undefined
    case AV_PIX_FMT_RGB0:        ///< packed RGB 8:8:8, 32bpp, RGBXRGBX...   X=unused/undefined
    case AV_PIX_FMT_0BGR:        ///< packed BGR 8:8:8, 32bpp, XBGRXBGR...   X=unused/undefined
    case AV_PIX_FMT_BGR0:        ///< packed BGR 8:8:8, 32bpp, BGRXBGRX...   X=unused/undefined
      return true;
    default:
      break;
  } // end SWITCH

  return false;
}

std::string
Stream_Module_Decoder_Tools::errorToString (int error_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::errorToString"));

  // initialize return value(s)
  std::string return_value;

  int result_2 = -1;
  char buffer_a[AV_ERROR_MAX_STRING_SIZE];
  ACE_OS::memset (buffer_a, 0, sizeof (char[AV_ERROR_MAX_STRING_SIZE]));

  result_2 = av_strerror (error_in,
                          buffer_a,
                          sizeof (char[AV_ERROR_MAX_STRING_SIZE]));
  if (unlikely (result_2))
    ACE_DEBUG ((LM_ERROR,
                ((result_2 < 0) ? ACE_TEXT ("failed to av_strerror(%d), cannot find error description: \"%m\", continuing\n")
                                : ACE_TEXT ("failed to av_strerror(%d): \"%m\", continuing\n")),
                error_in));
  return_value = buffer_a;

  return return_value;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
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
        result = AV_PIX_FMT_RGB555;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB565))
        result = AV_PIX_FMT_RGB565;
      // *IMPORTANT NOTE*: MEDIASUBTYPE_RGB24 actually has a 'BGR24' memory layout
      //                   see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd407253(v=vs.85).aspx
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB24))
        result = AV_PIX_FMT_BGR24;
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32))
        result = AV_PIX_FMT_RGB32;
      //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32))
        result = AV_PIX_FMT_ARGB;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2R10G10B10))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2B10G10R10))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32_D3D_DX7_RT))
        result = AV_PIX_FMT_RGB32;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB16_D3D_DX7_RT))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB32_D3D_DX7_RT))
        result = AV_PIX_FMT_ARGB;
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444_D3D_DX7_RT))
      //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555_D3D_DX7_RT))
      else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_RGB32_D3D_DX9_RT))
        result = AV_PIX_FMT_RGB32;
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
        result = AV_PIX_FMT_P016;
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

struct _GUID
Stream_Module_Decoder_Tools::AVPixelFormatToMediaSubType (enum AVPixelFormat pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::AVPixelFormatToMediaSubType"));

  struct _GUID result = GUID_NULL;

  switch (pixelFormat_in)
  {
    case AV_PIX_FMT_MONOBLACK:
      return MEDIASUBTYPE_RGB1;
    case AV_PIX_FMT_RGB4:
      return MEDIASUBTYPE_RGB4;
    case AV_PIX_FMT_RGB8:
      return MEDIASUBTYPE_RGB8;
    case AV_PIX_FMT_RGB555:
      return MEDIASUBTYPE_RGB555;
    case AV_PIX_FMT_RGB565:
      return MEDIASUBTYPE_RGB565;
    case AV_PIX_FMT_RGB24:
      return MEDIASUBTYPE_RGB24;
    case AV_PIX_FMT_RGB32:
      return MEDIASUBTYPE_RGB32;
    //else if (IsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB1555))
    case AV_PIX_FMT_ARGB:
      return MEDIASUBTYPE_ARGB32;
    //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_ARGB4444))
    //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2R10G10B10))
    //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_A2B10G10R10))
    case AV_PIX_FMT_YUVA444P:
      return MEDIASUBTYPE_AYUV;
    case AV_PIX_FMT_YUYV422:
      return MEDIASUBTYPE_YUY2;
    case AV_PIX_FMT_UYVY422:
      return MEDIASUBTYPE_UYVY;
    case AV_PIX_FMT_P016:
      return MEDIASUBTYPE_IMC1;
    case AV_PIX_FMT_NV12:
      return MEDIASUBTYPE_NV12;
    //else if (InlineIsEqualGUID (mediaSubType_in, MEDIASUBTYPE_IMC3))
    case AV_PIX_FMT_NV21:
      return MEDIASUBTYPE_IMC4;
    case AV_PIX_FMT_YUV420P:
      return MEDIASUBTYPE_YV12;
    case AV_PIX_FMT_UYYVYY411:
      return MEDIASUBTYPE_Y411;
    case AV_PIX_FMT_YVYU422:
      return MEDIASUBTYPE_YVYU;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %d \"%s\"), aborting\n"),
                  pixelFormat_in, ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (pixelFormat_in).c_str ())));
      break;
    }
  } // end SWITCH

  return result;
}
#endif

enum AVCodecID
Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId (enum AVPixelFormat pixelFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId"));

  enum AVCodecID result = AV_CODEC_ID_NONE;

  switch (pixelFormat_in)
  { // *NOTE*: libav does not specify a pixel format for MJPEG, it is a
    //         'compressed' format) --> map this deprecated format
    case AV_PIX_FMT_YUVJ422P:
      result = AV_CODEC_ID_MJPEG; break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %s), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (pixelFormat_in).c_str ())));
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

void
Stream_Module_Decoder_Tools::sinus (double frequency_in,
                                    unsigned int sampleRate_in,
                                    unsigned int sampleSize_in, // 'data'-
                                    unsigned int channels_in,
                                    uint8_t* buffer_in,
                                    unsigned int samplesToWrite_in, // #'data' samples
                                    double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::sinus"));

  static double maximum_phase_d = 2.0 * M_PI;
  double step_d =
    (maximum_phase_d * frequency_in) / static_cast<double> (sampleRate_in);
  unsigned int bytes_per_sample_i = sampleSize_in / channels_in;
  unsigned int maximum_value_i = (1 << ((bytes_per_sample_i * 8) - 1)) - 1;
  double phase_d = phase_inout;
  int value_i = 0;
  uint8_t* pointer_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value_i = static_cast<int> (std::sin (phase_d) * maximum_value_i);
    for (unsigned int j = 0; j < channels_in; ++j)
    {
      for (unsigned int k = 0; k < bytes_per_sample_i; ++k)
      {
        if (likely (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN))
          *(pointer_p + k) = (value_i >> (k * 8)) & 0xFF;
        else
          *(pointer_p + bytes_per_sample_i - 1 - k) =
            (value_i >> (k * 8)) & 0xFF;
      } // end FOR
      pointer_p += bytes_per_sample_i;
    } // end FOR
    phase_d += step_d;
    if (unlikely (phase_d >= maximum_phase_d))
      phase_d -= maximum_phase_d;
  } // end FOR
  phase_inout = phase_d;
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
                                      uint8_t* targetBuffers_in[])
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::convert"));

  // sanity check(s)
  if (unlikely (!sws_isSupportedInput (sourcePixelFormat_in) ||
                !sws_isSupportedOutput (targetPixelFormat_in)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("unsupported format conversion (was: %s --> %s), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (sourcePixelFormat_in).c_str ()),
                ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (targetPixelFormat_in).c_str ())));
    return false;
  } // end IF
//  ACE_ASSERT (sourcePixelFormat_in != targetPixelFormat_in);

// *TODO*: define a balanced scaler parametrization that suits most
//         applications, or expose this as a parameter
  int flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
               SWS_BICUBIC);
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
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (errno).c_str ())));
    return false;
  } // end IF

  bool result = false;
  int result_2 = -1;
  int in_linesize[AV_NUM_DATA_POINTERS];
  int out_linesize[AV_NUM_DATA_POINTERS];
  result_2 = av_image_fill_linesizes (in_linesize,
                                      sourcePixelFormat_in,
                                      static_cast<int> (sourceWidth_in));
  ACE_ASSERT (result_2 >= 0);
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
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  // *NOTE*: ffmpeg returns fewer than the expected number of rows in some cases
  // *TODO*: find out when and why (support off-by-one rounding)
  else if (unlikely (result_2 != static_cast<int> (targetHeight_in))      &&
                     (result_2 != static_cast<int> (targetHeight_in + 1)) &&
                     (result_2 != static_cast<int> (targetHeight_in - 1)))
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
                                    uint8_t* targetBuffers_in[])
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::scale"));

  // *TODO*: define a balanced scaler parametrization that suits most
  //         applications, or expose this as a parameter
  int flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
               SWS_BICUBIC);
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
  int out_linesize[AV_NUM_DATA_POINTERS];
  result_2 = av_image_fill_linesizes (in_linesize,
                                      pixelFormat_in,
                                      static_cast<int> (sourceWidth_in));
  ACE_ASSERT (result_2 >= 0);
  result_2 = av_image_fill_linesizes (out_linesize,
                                      pixelFormat_in,
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
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result_2).c_str ())));
    goto clean;
  } // end IF
  // *NOTE*: ffmpeg returns fewer than the expected number of rows in some cases
  // *TODO*: find out when and why (support off-by-one rounding)
  else if (unlikely (result_2 != static_cast<int> (targetHeight_in))      &&
                     (result_2 != static_cast<int> (targetHeight_in + 1)) &&
                     (result_2 != static_cast<int> (targetHeight_in - 1)))
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool
Stream_Module_Decoder_Tools::loadAudioRendererGraph (const struct _AMMediaType& mediaType_in,
                                                     const int audioOutput_in,
                                                     IGraphBuilder* IGraphBuilder_in,
                                                     REFGUID effect_in,
                                                     const union Stream_MediaFramework_DirectShow_AudioEffectOptions& effectOptions_in,
                                                     Stream_MediaFramework_DirectShow_GraphConfiguration_t& graphConfiguration_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadAudioRendererGraph"));

  HRESULT result = E_FAIL;
  struct _GUID GUID_s = GUID_NULL;
  struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;

  // initialize return value(s)
  graphConfiguration_out.clear ();

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  // *TODO*: add source filter name
  if (!Stream_MediaFramework_DirectShow_Tools::reset (IGraphBuilder_in,
                                                      CLSID_AudioInputDeviceCategory))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::reset(), aborting\n")));
    return false;
  } // end IF

  IBaseFilter* filter_p = NULL, *filter_2 = NULL;
  IBaseFilter* filter_3 = NULL, *filter_4 = NULL;
  IDMOWrapperFilter* wrapper_filter_p = NULL;

  //// encode PCM --> WAV ?
  //struct _GUID converter_CLSID = WAV_Colour;
  //std::wstring converter_name = STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CONVERT_PCM;
  //if (mediaType_in.subtype == MEDIASUBTYPE_WAVE)
  //{
  //  converter_CLSID = CLSID_MjpegDec;
  //  converter_name = STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
  //} // end IF
  //else if (mediaType_in.subtype == MEDIASUBTYPE_PCM)
  //{
  //  // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
  //  converter_CLSID = CLSID_AVIDec;
  //  converter_name = STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
  //} // end IF
  //else
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
  //              ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (mediaType_in.subtype).c_str ())));
  //  return false;
  //} // end IF

  //result = IGraphBuilder_in->AddFilter (filter_p,
  //                                      converter_name.c_str ());
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  //  goto error;
  //} // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("added \"%s\"\n"),
  //            ACE_TEXT_WCHAR_TO_TCHAR (converter_name.c_str ())));

  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_2));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_SampleGrabber).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_2);
  result =
    IGraphBuilder_in->AddFilter (filter_2,
                                 STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB;
  graphConfiguration_out.push_back (graph_entry);

  // add effect DMO ?
//add_effect:
  if (InlineIsEqualGUID (effect_in, GUID_NULL))
    goto continue_;

  result = CoCreateInstance (CLSID_DMOWrapperFilter, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_3));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_DMOWrapperFilter).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_3);
  result = filter_3->QueryInterface (IID_PPV_ARGS (&wrapper_filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IDMOWrapperFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
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
  if (InlineIsEqualGUID (effect_in, GUID_DSCFX_CLASS_AEC))
  {
    IDirectSoundCaptureFXAec* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundCaptureFXAec,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundCaptureFXAec): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.AECOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundCaptureFXAec::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
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
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXChorus): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.chorusOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXChorus::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_COMPRESSOR))
  {
    IDirectSoundFXCompressor* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXCompressor,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXCompressor): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.compressorOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXCompressor::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_DISTORTION))
  {
    IDirectSoundFXDistortion* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXDistortion,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXDistortion): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.distortionOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXDistortion::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_ECHO))
  {
    IDirectSoundFXEcho* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXEcho,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXEcho): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.echoOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXEcho::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_PARAMEQ))
  {
    IDirectSoundFXParamEq* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXParamEq,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXParamEq): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.equalizerOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXParamEq::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_FLANGER))
  {
    IDirectSoundFXFlanger* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXFlanger,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXFlanger): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.flangerOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXFlanger::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_GARGLE))
  {
    IDirectSoundFXGargle* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXGargle,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXGargle): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.gargleOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXGargle::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_STANDARD_I3DL2REVERB))
  {
    IDirectSoundFXI3DL2Reverb* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXI3DL2Reverb,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXI3DL2Reverb): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.reverbOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXI3DL2Reverb::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  else if (InlineIsEqualGUID (effect_in, GUID_DSFX_WAVES_REVERB))
  {
    IDirectSoundFXWavesReverb* effect_p = NULL;
    result = wrapper_filter_p->QueryInterface (IID_IDirectSoundFXWavesReverb,
                                               (void**)&effect_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDMOWrapperFilter::QueryInterface(IID_IDirectSoundFXWavesReverb): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    result = effect_p->SetAllParameters (&effectOptions_in.wavesReverbOptions);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirectSoundFXWavesReverb::SetAllParameters(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

      effect_p->Release ();

      goto error;
    } // end IF
    effect_p->Release ();
  } // end ELSE IF
  //////////////////////////////////////
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown effect (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (effect_in).c_str ())));
  wrapper_filter_p->Release ();
  wrapper_filter_p = NULL;
  result =
    IGraphBuilder_in->AddFilter (filter_3,
                                 STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_EFFECT_AUDIO;
  graphConfiguration_out.push_back (graph_entry);

continue_:
  // send to an output (waveOut) ?
  if (audioOutput_in > 0)
  {
    GUID_s = CLSID_AudioRender;
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_AUDIO;
  } // end IF
  else
  {
    GUID_s = CLSID_NullRenderer;
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL;
  } // end ELSE
  result = CoCreateInstance (GUID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_4));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (filter_4);
  result =
    IGraphBuilder_in->AddFilter (filter_4,
                                 graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  graphConfiguration_out.push_back (graph_entry);

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
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (filter_4)
    filter_4->Release ();
  if (wrapper_filter_p)
    wrapper_filter_p->Release ();

  return true;

error:
  if (filter_p)
    filter_p->Release ();
  if (filter_2)
    filter_2->Release ();
  if (filter_3)
    filter_3->Release ();
  if (filter_4)
    filter_4->Release ();
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
  FOURCCMap fourcc_map;
  struct _AMMediaType* media_type_p = NULL;

  // initialize return value(s)
  for (Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t iterator = graphConfiguration_out.begin ();
       iterator != graphConfiguration_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_MediaFramework_DirectShow_Tools::delete_ ((*iterator).mediaType);
  graphConfiguration_out.clear ();

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    graph_entry.filterName =
      STREAM_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    graph_entry.filterName =
      STREAM_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
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
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType = NULL;

  // step1: decompress ?
  if (InlineIsEqualGUID (outputFormat_in.subtype, captureFormat_in.subtype) ||
      !Stream_Module_Decoder_Tools::isCompressedVideo (captureFormat_in.subtype,
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

  fourcc_map.SetFOURCC (&graph_entry.mediaType->subtype);
  switch (fourcc_map.GetFOURCC ())
  {
    // *** compressed types ***
    case FCC ('H264'):
    {
      CLSID_s = CLSID_CMPEG2VidDecoderDS;
      graph_entry.filterName =
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264;
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
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
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
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_s).c_str ()),
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
  graphConfiguration_out.push_back (graph_entry);
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
  if (InlineIsEqualGUID (graph_entry.mediaType->subtype, outputFormat_in.subtype) ||
      skip_decode)
    goto grab;

  preferred_subtype = outputFormat_in.subtype;
  if (Stream_MediaFramework_Tools::isRGB (graph_entry.mediaType->subtype,
                                          STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  {
    CLSID_s = CLSID_Colour;
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_RGB;
  } // end IF
  else if (Stream_MediaFramework_Tools::isChromaLuminance (graph_entry.mediaType->subtype,
                                                           STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
  {
    // *NOTE*: the AVI Decompressor supports decoding YUV-formats to RGB
    CLSID_s = CLSID_AVIDec;
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI;
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
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_s).c_str ()),
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
  graphConfiguration_out.push_back (graph_entry);
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

grab:
  if (skip_grab)
    goto render;

  graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB;
  result = CoCreateInstance (CLSID_SampleGrabber, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_SampleGrabber).c_str ()),
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
  graphConfiguration_out.push_back (graph_entry);
  graph_entry.mediaType =
    Stream_MediaFramework_DirectShow_Tools::copy (outputFormat_in);
  if (!graph_entry.mediaType)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(), aborting\n")));
    goto error;
  } // end IF

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
                                                     STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_AVI) &&
        Stream_MediaFramework_DirectShow_Tools::has (graphConfiguration_out,
                                                     STREAM_LIB_DIRECTSHOW_FILTER_NAME_GRAB) &&
        !InlineIsEqualGUID (CLSID_s, CLSID_VideoRenderer))
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("graph has 'AVI Decompressor' and 'Sample Grabber'; using default video renderer...\n")));
      CLSID_s = STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER;
    } // end IF
    graph_entry.filterName = STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO;
  } // end IF
  else
  {
    // *NOTE*: connect()ing the 'sample grabber' to the 'null renderer' breaks
    //         any connection between the 'AVI decompressor' and the 'sample
    //         grabber' (go ahead, try it in with graphedit.exe)
    CLSID_s = CLSID_NullRenderer;
    graph_entry.filterName = STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL;
  } // end ELSE
  result = CoCreateInstance (CLSID_s, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_s).c_str ()),
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
  graphConfiguration_out.push_back (graph_entry);

  return true;

error:
  for (Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t iterator = graphConfiguration_out.begin ();
       iterator != graphConfiguration_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_MediaFramework_DirectShow_Tools::delete_ ((*iterator).mediaType);
  graphConfiguration_out.clear ();
  if (pin_p)
    pin_p->Release ();
  if (filter_p)
    filter_p->Release ();
  if (graph_entry.mediaType)
    Stream_MediaFramework_DirectShow_Tools::delete_ (graph_entry.mediaType);

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
  FOURCCMap fourcc_map;
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

  // initialize return value(s)
  for (Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t iterator = graphConfiguration_out.begin ();
       iterator != graphConfiguration_out.end ();
       ++iterator)
    if ((*iterator).mediaType)
      Stream_MediaFramework_DirectShow_Tools::delete_ ((*iterator).mediaType);
  graphConfiguration_out.clear ();

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
                ACE_TEXT (Common_Tools::GUIDToString (graph_entry.mediaType->formattype).c_str ())));
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

  if (!Stream_Module_Decoder_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
                                                       STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
    goto decode;

decompress:
  fourcc_map.SetFOURCC (&graph_entry.mediaType->subtype);
  switch (fourcc_map.GetFOURCC ())
  {
    // *** compressed types ***
    case FCC ('H264'):
    {
      CLSID_s = CLSID_CMPEG2VidDecoderDS;
      graph_entry.filterName =
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_H264;
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
        STREAM_DEC_DIRECTSHOW_FILTER_NAME_DECOMPRESS_MJPG;
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
                ACE_TEXT (Common_Tools::GUIDToString (CLSID_s).c_str ()),
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

  if (Stream_Module_Decoder_Tools::isCompressedVideo (graph_entry.mediaType->subtype,
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
  if (InlineIsEqualGUID (graph_entry.mediaType->subtype, STREAM_DEC_DIRECTSHOW_FILTER_VIDEO_RENDERER_DEFAULT_FORMAT) ||
      skip_decode)
    goto grab;

  preferred_subtype =
    STREAM_DEC_DIRECTSHOW_FILTER_VIDEO_RENDERER_DEFAULT_FORMAT;
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
    graph_entry.filterName =
      STREAM_DEC_DIRECTSHOW_FILTER_NAME_CONVERT_YUV;
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
      DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
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
      DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
      i_media_object_p->Release (); i_media_object_p = NULL;
      i_dmo_wrapper_filter_p->Release (); i_dmo_wrapper_filter_p = NULL;
      goto error;
    } // end IF
    DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
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
    DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
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
    DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
    i_media_object_p->Release (); i_media_object_p = NULL;
    goto error;
  } // end IF
  DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
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
                ACE_TEXT (Common_Tools::GUIDToString (dmo_media_type_p->formattype).c_str ())));
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
    DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
    i_media_object_p->Release (); i_media_object_p = NULL;
    goto error;
  } // end IF
  i_media_object_p->Release (); i_media_object_p = NULL;

  graph_entry.filterName =
    STREAM_DEC_DIRECTSHOW_FILTER_NAME_RESIZER_VIDEO;
  result =
    IGraphBuilder_out->AddFilter (filter_2,
                                  graph_entry.filterName.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    DeleteMediaType (reinterpret_cast<struct _AMMediaType*> (dmo_media_type_p)); dmo_media_type_p = NULL;
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
    (windowHandle_in ? STREAM_DEC_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO
                     : STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL);

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
                  ACE_TEXT (Common_Tools::GUIDToString (CLSID_s).c_str ()),
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
Stream_Module_Decoder_Tools::loadAudioRendererTopology (const std::string& deviceIdentifier_in,
                                                        IMFMediaType* mediaType_inout,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                        IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                        IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                        int audioOutput_in,
                                                        TOPOID& sampleGrabberSinkNodeId_out,
                                                        TOPOID& rendererNodeId_out,
                                                        IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadAudioRendererTopology"));

  // sanity check(s)
  ACE_ASSERT (mediaType_inout);

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
  //IMFAudioProcessorControl* video_processor_control_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;
  if (!topology_inout)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (deviceIdentifier_in,
                                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                         media_source_p,
                                                                         NULL, // do not load a dummy sink
                                                                         topology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
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
  result = mediaType_inout->GetCount (&item_count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  if (!item_count)
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
  media_type_p =
    Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_inout);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::copy(), aborting\n")));
    goto error;
  } // end IF
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
  if (!Stream_Module_Decoder_Tools::isCompressedAudio (sub_type,
                                                       STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
    goto continue_;

  //if (!media_source_p)
  //{
  //  result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
  //                                      IID_PPV_ARGS (&media_source_p));
  //  ACE_ASSERT (SUCCEEDED (result));
  //} // end IF
  //if (!Stream_MediaFramework_MediaFoundation_Tools::getCaptureFormat (media_source_p,
  //                                                                    media_type_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
  //  goto error;
  //} // end IF
  //media_source_p->Release (); media_source_p = NULL;

  //IMFAttributes* attributes_p = NULL;
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
                  ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_AUDIO_DECODER).c_str ()),
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
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
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
                                                                       media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
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
    transform_p->Release (); transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder for \"%s\": \"%s\"...\n"),
                node_id,
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                ACE_TEXT (module_string.c_str ())));

    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (!Stream_Module_Decoder_Tools::isCompressedAudio (sub_type,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
      break; // done
  } // end WHILE

continue_:
  // step3: add tee node ?
  if ((!sampleGrabberSinkCallback_in && !(audioOutput_in > 0)) ||
      ((sampleGrabberSinkCallback_in && !(audioOutput_in > 0)) || // XOR
      (!sampleGrabberSinkCallback_in &&  (audioOutput_in > 0))))
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
              ACE_TEXT ("added tee node (id: %q)...\n"),
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
  ACE_ASSERT (activate_p);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

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
              ACE_TEXT ("added sample grabber sink node (id: %q)...\n"),
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
  // step5: add audio renderer sink ?
  if (!(audioOutput_in > 0))
    goto continue_4;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateAudioRendererActivate (&activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAudioRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
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
Stream_Module_Decoder_Tools::loadVideoRendererTopology (const std::string& deviceIdentifier_in,
                                                        const IMFMediaType* mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                        IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                        IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                        HWND windowHandle_in,
                                                        TOPOID& sampleGrabberSinkNodeId_out,
                                                        TOPOID& rendererNodeId_out,
                                                        IMFTopology*& topology_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::loadVideoRendererTopology"));

  bool release_topology = false;
  struct _GUID sub_type = GUID_NULL;
  MFT_REGISTER_TYPE_INFO mft_register_type_info = { GUID_NULL, GUID_NULL };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  IMFVideoProcessorControl2* video_processor_control_p = NULL;
#elif COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFVideoProcessorControl* video_processor_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00)
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;
  if (!topology_inout)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (deviceIdentifier_in,
                                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                         media_source_p,
                                                                         NULL, // do not load a dummy sink
                                                                         topology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      goto error;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
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
  if (!item_count)
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
  if (!Stream_Module_Decoder_Tools::isCompressedVideo (sub_type,
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
                  ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    module_string =
      Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));

clean:
    for (UINT32 i = 0; i < number_of_decoders; i++)
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
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
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
                                                                       media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (media_type_p);
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
    transform_p->Release (); transform_p = NULL;

    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder for \"%s\": \"%s\"...\n"),
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

    if (!Stream_Module_Decoder_Tools::isCompressedVideo (sub_type,
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_PROCESSOR,
#else
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
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
                ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
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
  //            ACE_TEXT ("added transform node (id: %q)...\n"),
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  ACE_ASSERT (SUCCEEDED (result));
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
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  transform_p->Release (); transform_p = NULL;

  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added processor for \"%s\": \"%s\"...\n"),
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
              ACE_TEXT ("added tee node (id: %q)...\n"),
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

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
              ACE_TEXT ("%q: added sample grabber sink node...\n"),
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
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
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration(), aborting\n")));
    goto error;
  } // end IF

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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  IMFTransform* transform_p = NULL;
  TOPOID node_id = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
  IMFVideoProcessorControl2* video_processor_control_p = NULL;
#elif COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFVideoProcessorControl* video_processor_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00)
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  int i = 0;
  BOOL is_compressed = false;

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
                  ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    module_string =
      Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

    result =
      decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
    ACE_ASSERT (SUCCEEDED (result));

clean:
    for (UINT32 i = 0; i < number_of_decoders; i++)
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
    //            ACE_TEXT ("added transform node (id: %q)...\n"),
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
                                                                       media_type_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      goto error;
    } // end IF
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
    transform_p->Release (); transform_p = NULL;

    sub_type_2 = sub_type;
    result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                    &sub_type);
    ACE_ASSERT (SUCCEEDED (result));
    // debug info
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%q: added decoder \"%s\": \"%s\" --> \"%s\"...\n"),
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_PROCESSOR,
#else
  if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
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
                ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
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
  CoTaskMemFree (decoders_p); decoders_p = NULL;
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
  //            ACE_TEXT ("added transform node (id: %q)...\n"),
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  result =
    transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: (for some unknown reason,) this does nothing...
  result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
  //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
  ACE_ASSERT (SUCCEEDED (result));
  video_processor_control_p->Release (); video_processor_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  transform_p->Release (); transform_p = NULL;

  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  ACE_ASSERT (SUCCEEDED (result));
  // debug info
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added processor \"%s\": \"%s\" --> \"%s\"...\n"),
              node_id,
              ACE_TEXT (module_string.c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

continue_:
  // step5: add video renderer sink ?
  //if (!windowHandle_in)
  //  goto continue_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added renderer \"%s\": \"%s\" -->...\n"),
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

  // initialize return value(s)
  rendererNodeId_out = 0;

  if (!topology_inout)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (!Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology (URL_in,
                                                                          media_source_p,
                                                                          topology_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology(\"%s\"), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (media_source_p);
    release_topology = true;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  } // end IF
  ACE_ASSERT (topology_inout);

  // step1: retrieve source node
  IMFTopologyNode* source_node_p = NULL;
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  IMFTransform* transform_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  if (!media_source_p)
  {
    result = source_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&media_source_p));
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  IMFMediaType* media_type_p = NULL;
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

  if (!Stream_Module_Decoder_Tools::isCompressedVideo (sub_type,
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
                  ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
    if (!Stream_Module_Decoder_Tools::isCompressedVideo (sub_type,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))
      break; // done
  } // end WHILE
  media_type_p->Release (); media_type_p = NULL;

continue_:
  // step5: add video renderer sink ?
  if (!windowHandle_in)
    goto continue_2;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  result = topology_node_p->SetObject (activate_p);
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release (); activate_p = NULL;
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
#endif
