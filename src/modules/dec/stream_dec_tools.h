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
#include "libavutil/pixdesc.h"
}
#endif /* __cplusplus */

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <strmif.h>
#include <vadefs.h>
#include <windef.h>
#else
#include "alsa/asoundlib.h"

#include "sox.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"

#include "stream_lib_common.h"

#include "stream_dec_common.h"

// forward declarations
enum AVPixelFormat;
struct SwsContext;

//void Stream_Dec_Export stream_decoder_libav_log_cb (void*, int, const char*, va_list);
void stream_decoder_libav_log_cb (void*, int, const char*, va_list);

class Stream_Module_Decoder_Tools
{
 public:
  static void initialize ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static bool isCompressed (REFGUID,                                                              // media subtype
                            REFGUID,                                                              // device category
                            enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isCompressedAudio (REFGUID,                                                              // media subtype
                                 enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
  static bool isCompressedVideo (REFGUID, // media subtype
                                 enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
#endif // ACE_WIN32 || ACE_WIN64
  static bool isCompressedVideo (enum AVPixelFormat); // pixel format

  static bool isChromaLuminance (enum AVPixelFormat); // pixel format
  static bool isRGB (enum AVPixelFormat); // pixel format

  static std::string errorToString (int); // libav error

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: supports non-RGB AND non-Chroma-Luminance types only
  static enum AVCodecID mediaSubTypeToAVCodecId (REFGUID,                                                              // media subtype
                                                 enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);
  // *NOTE*: supports RGB and Chroma-Luminance types only
  static enum AVPixelFormat mediaSubTypeToAVPixelFormat (REFGUID,                                                              // media subtype
                                                         enum Stream_MediaFramework_Type = MODULE_LIB_DEFAULT_MEDIAFRAMEWORK);

  // -------------------------------------
  // filter graphs / topologies
  // *TODO*: remove these ASAP

  // direct show
  // *NOTE*: loads a filter graph (source side)
  static bool loadAudioRendererGraph (const struct _AMMediaType&,                                       // media type
                                      const int,                                                        // output handle [0: null]
                                      IGraphBuilder*,                                                   // graph handle
                                      REFGUID,                                                          // DMO effect CLSID [GUID_NULL: no effect]
                                      const union Stream_MediaFramework_DirectShow_AudioEffectOptions&, // DMO effect options
                                      Stream_MediaFramework_DirectShow_GraphConfiguration_t&);          // return value: graph layout
  static bool loadVideoRendererGraph (REFGUID,                                                 // device category (GUID_NULL: retain first filter w/o input pins)
                                      const struct _AMMediaType&,                              // capture media type (i.e. capture device output)
                                      const struct _AMMediaType&,                              // output media type (sample grabber-)
                                      const HWND,                                              // window handle [NULL: NullRenderer]
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
  static bool loadAudioRendererTopology (const std::string&,                   // device name ("FriendlyName")
                                         IMFMediaType*,                        // [return value] sample grabber sink input media type handle
                                         const IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
                                         int,                                  // audio output handle [0: do not use tee/renderer]
                                         TOPOID&,                              // return value: sample grabber sink node id
                                         TOPOID&,                              // return value: audio renderer sink node id
                                         IMFTopology*&);                       // input/return value: topology handle
  static bool loadVideoRendererTopology (const std::string&,                   // device name ("FriendlyName")
                                         const IMFMediaType*,                  // sample grabber sink input media type handle
                                         const IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle [NULL: do not use tee/grabber]
                                         const HWND,                           // window handle [NULL: do not use tee/EVR]
                                         TOPOID&,                              // return value: sample grabber sink node id
                                         TOPOID&,                              // return value: EVR sink node id
                                         IMFTopology*&);                       // input/return value: topology handle
  static bool loadVideoRendererTopology (const IMFMediaType*, // input media type handle
                                         const HWND,          // window handle [NULL: do not use tee/EVR]
                                         TOPOID&,             // return value: EVR sink node id
                                         IMFTopology*&);      // input/return value: topology handle

  static bool loadTargetRendererTopology (const std::string&,  // URL
                                          const IMFMediaType*, // media source output media type handle
                                          const HWND,          // window handle [NULL: do not use tee/EVR]
                                          TOPOID&,             // return value: EVR sink node id
                                          IMFTopology*&);      // input/return value: topology handle
#endif // ACE_WIN32 || ACE_WIN64
  static enum AVCodecID AVPixelFormatToAVCodecId (enum AVPixelFormat); // pixel format

  static std::string compressionFormatToString (enum Stream_Decoder_CompressionFormatType);

  // *WARNING*: this may crash if the format is 'unknown'
  inline static std::string pixelFormatToString (enum AVPixelFormat format_in) { std::string result = av_get_pix_fmt_name (format_in); return result; }

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
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools (const Stream_Module_Decoder_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools& operator= (const Stream_Module_Decoder_Tools&))
};

#endif
