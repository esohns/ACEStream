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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 // *WORKAROUND*: mfobjects.h includes cguid.h, which requires this
#define __CGUID_H__
#include "ks.h"
#include "guiddef.h"
#undef GetObject
#include "mfidl.h"
#include "mfobjects.h"
#include "strmif.h"
#include "vadefs.h"
#include "windef.h"
#endif // ACE_WIN32 || ACE_WIN64

#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavutil/pixdesc.h"
#include "libavutil/pixfmt.h"

#include "libswscale/swscale.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT

#include "ace/Basic_Types.h"
#include "ace/Date_Time.h"
#include "ace/Global_Macros.h"

#include "stream_lib_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_defines.h"
#include "stream_lib_directshow_common.h"
#include "stream_lib_directsound_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_common.h"

#if defined (FFMPEG_SUPPORT)
void stream_decoder_libav_log_cb (void*, int, const char*, va_list);
#endif // FFMPEG_SUPPORT

class Stream_Module_Decoder_Tools
{
 public:
  // MPEG4
  // *IMPORTANT NOTE*: return value is localtime
  static ACE_Date_Time mpeg4ToDateTime (ACE_UINT64); // seconds since UTC 1904-01-01 zero

  // MPEGTS: see ITU-T Rec. H.222.0 Table 2.45
  static enum Stream_MediaType_Type streamIdToMediaType (unsigned short); // stream id

#if defined (FFMPEG_SUPPORT)
  // *NOTE*: 'packed' formats only
  static bool isPackedIntegerPCM (enum AVSampleFormat); // sample format
  static bool isPackedRealPCM (enum AVSampleFormat); // sample format
  static uint64_t channelsToLayout (unsigned int); // #channels

  inline static bool isCompressedVideo (enum AVPixelFormat format_in) { return (!Stream_Module_Decoder_Tools::isRGB (format_in) && !Stream_Module_Decoder_Tools::isChromaLuminance (format_in)); }

  static bool isChromaLuminance (enum AVPixelFormat); // pixel format
  static bool isRGB (enum AVPixelFormat); // pixel format
  static bool isRGB32 (enum AVPixelFormat); // pixel format
#endif // FFMPEG_SUPPORT

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (FFMPEG_SUPPORT)
  static enum AVSampleFormat to (const struct tWAVEFORMATEX&); // format
  // *NOTE*: supports RGB and Chroma-Luminance types only
  static enum AVPixelFormat mediaSubTypeToAVPixelFormat (REFGUID,                                                              // media subtype
                                                         enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
  // *NOTE*: supports non-RGB AND non-Chroma-Luminance types only
  static enum AVCodecID mediaSubTypeToAVCodecId (REFGUID,                                                              // media subtype
                                                 enum Stream_MediaFramework_Type = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK);
#endif // FFMPEG_SUPPORT

  // -------------------------------------
  // filter graphs / topologies
  // *TODO*: remove these ASAP

  // direct show
  // *NOTE*: loads a filter graph (source side)
  static bool loadAudioRendererGraph (REFGUID,                                                           // device category (GUID_NULL: retain first filter w/o input pins)
                                      const struct _AMMediaType&,                                        // media type
                                      const struct _AMMediaType&,                                        // (sample grabber-) output media type
                                      bool,                                                              // add sample grabber ?
                                      int,                                                               // output handle [-1: null renderer]
                                      IGraphBuilder*,                                                    // graph handle
                                      REFGUID,                                                           // DMO effect CLSID [GUID_NULL: no effect]
                                      const union Stream_MediaFramework_DirectSound_AudioEffectOptions&, // DMO effect options
                                      Stream_MediaFramework_DirectShow_GraphConfiguration_t&);           // return value: graph layout
  static bool loadVideoRendererGraph (REFGUID,                                                 // device category (GUID_NULL: retain first filter w/o input pins)
                                      const struct _AMMediaType&,                              // capture media type (i.e. capture device output)
                                      const struct _AMMediaType&,                              // output media type (sample grabber-)
                                      HWND,                                                    // window handle [NULL: NullRenderer]
                                      IGraphBuilder*,                                          // graph builder handle
                                      Stream_MediaFramework_DirectShow_GraphConfiguration_t&); // return value: graph configuration
  // *NOTE*: loads a filter graph (target side). If the first parameter is NULL,
  //         the filter with the name of the second parameter is expected to be
  //         part of the graph (sixth parameter) already
  static bool loadTargetRendererGraph (IBaseFilter*,                                            // source filter handle
                                       const std::wstring&,                                     // source filter name
                                       const struct _AMMediaType&,                              // input media type
                                       HWND,                                                    // window handle [NULL: NullRenderer]
                                       IGraphBuilder*&,                                         // return value: graph handle
                                       IAMBufferNegotiation*&,                                  // return value: source filter output pin buffer allocator configuration handle
                                       Stream_MediaFramework_DirectShow_GraphConfiguration_t&); // return value: graph layout

  // media foundation
  static bool loadAudioRendererTopology (REFGUID,                        // device identifier
                                         REFGUID,                        // device category
                                         bool,                           // use framework source ? : retain any existing media source
                                         IMFMediaType*&,                 // input/return value: capture media type handle
                                         const IMFMediaType*,            // (sample grabber sink-) output media type handle [NULL: use default sink/stream media type]
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                         IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
#else
                                         IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle [NULL: do not use tee/grabber]
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                         int,                            // output handle [-1: do not use [tee/]renderer]
                                         REFGUID,                        // audio effect CLSID [GUID_NULL: no effect]
                                         const std::string&,             // audio effect options *TODO*
                                         IMFTopology*&);                 // input/return value: topology handle
  static bool loadVideoRendererTopology (REFGUID,                        // device identifier
                                         const IMFMediaType*,            // sample grabber sink input media type handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                         IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
#else
                                         IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle [NULL: do not use tee/grabber]
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                         HWND,                           // window handle [NULL: do not use tee/EVR]
                                         TOPOID&,                        // return value: sample grabber sink node id
                                         TOPOID&,                        // return value: EVR sink node id
                                         IMFTopology*&);                 // input/return value: topology handle
  static bool loadVideoRendererTopology (const IMFMediaType*, // input media type handle
                                         HWND,                // window handle [NULL: do not use tee/EVR]
                                         TOPOID&,             // return value: EVR sink node id
                                         IMFTopology*&);      // input/return value: topology handle

  static bool loadTargetRendererTopology (const std::string&,  // URL
                                          const IMFMediaType*, // media source output media type handle
                                          HWND,                // window handle [NULL: do not use tee/EVR]
                                          TOPOID&,             // return value: EVR sink node id
                                          IMFTopology*&);      // input/return value: topology handle
  static bool updateRendererTopology (IMFTopology*,         // topology handle
                                      const IMFMediaType*); // (new) source media type handle
#endif // ACE_WIN32 || ACE_WIN64

  static std::string compressionFormatToString (enum Stream_Decoder_CompressionFormatType);

#if defined (FFMPEG_SUPPORT)
  static enum AVCodecID AVPixelFormatToAVCodecId (enum AVPixelFormat); // pixel format
  static enum AVCodecID filenameExtensionToAVCodecId (const std::string&); // filename extension

  static std::string audioFormatToString (enum AVSampleFormat);

  static bool convert (struct SwsContext*, // context ? : use sws_getCachedContext()
                       unsigned int,       // source width
                       unsigned int,       // source height
                       enum AVPixelFormat, // source pixel format
                       uint8_t*[],         // source buffer(s)
                       unsigned int,       // target width
                       unsigned int,       // target height
                       enum AVPixelFormat, // target pixel format
                       uint8_t*[],         // target buffer(s)
                       bool = false);      // flip vertically ?
  static bool scale (struct SwsContext*, // context ? : use sws_getCachedContext()
                     unsigned int,       // source width
                     unsigned int,       // source height
                     enum AVPixelFormat, // source pixel format
                     uint8_t*[],         // source buffer(s)
                     unsigned int,       // target width
                     unsigned int,       // target height
                     uint8_t*[],         // target buffer(s)
                     bool = false);      // flip vertically ?
#endif // FFMPEG_SUPPORT

#if defined (OPENCV_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static int mediaSubTypeToOpenCVFormat (REFGUID);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
  static int AVPixelFormatToOpenCVFormat (enum AVPixelFormat);
#endif // FFMPEG_SUPPORT
#endif // OPENCV_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools (const Stream_Module_Decoder_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools& operator= (const Stream_Module_Decoder_Tools&))
};

#endif
