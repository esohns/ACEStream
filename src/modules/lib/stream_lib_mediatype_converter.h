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

#ifndef STREAM_LIB_MEDIATYPE_CONVERTER_H
#define STREAM_LIB_MEDIATYPE_CONVERTER_H

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 // *WORKAROUND*: mfobjects.h includes cguid.h, which requires this
#define __CGUID_H__
#include "ks.h"
#include "mfobjects.h"
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

#include "stream_lib_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#else
#include "stream_lib_alsa_common.h"
#include "stream_lib_v4l_common.h"
#if defined (LIBCAMERA_SUPPORT)
#include "stream_lib_libcamera_common.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

#include "stream_lib_tools.h"

template <typename MediaType>
class Stream_MediaFramework_MediaTypeConverter_T
{
 public:
  Stream_MediaFramework_MediaTypeConverter_T ();
  inline virtual ~Stream_MediaFramework_MediaTypeConverter_T () {}

 protected:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: struct _AMMediaType return values need to be Stream_MediaFramework_DirectShow_Tools::free'd !
  inline void getMediaType (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_out) { Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in, mediaType_out); }
  void getMediaType (const struct _AMMediaType&, // media type
                     enum Stream_MediaType_Type, // media type type
                     struct Stream_MediaFramework_DirectShow_AudioVideoFormat&);
  void getMediaType (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat&, // media type
                     enum Stream_MediaType_Type,                                      // media type type
                     struct _AMMediaType&);
  inline void getMediaType (const struct _AMMediaType& mediaType_in, enum Stream_MediaType_Type, struct _AMMediaType& mediaType_out) { bool result = Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in, mediaType_out); ACE_ASSERT (result); }
  void getMediaType (const struct _AMMediaType&, enum Stream_MediaType_Type, IMFMediaType*&);

  inline void getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_out) { Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in, mediaType_out); }
  void getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat&, // media type
                     enum Stream_MediaType_Type,                                           // media type type
                     struct _AMMediaType&);
  void getMediaType (const IMFMediaType*,        // media type
                     enum Stream_MediaType_Type, // media type type
                     struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat&);
  void getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat&, // media type
                     enum Stream_MediaType_Type,                                           // media type type
                     IMFMediaType*&);
  void getMediaType (const IMFMediaType*, enum Stream_MediaType_Type, IMFMediaType*&);
  void getMediaType (const IMFMediaType*, enum Stream_MediaType_Type, struct _AMMediaType&);

#if defined (FFMPEG_SUPPORT)
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out) { mediaType_out = mediaType_in.audio; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { mediaType_out = mediaType_in.video; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_inout) { mediaType_inout = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout = mediaType_in; }

  inline void getMediaType (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out) { getMediaType (mediaType_in.audio, type_in, mediaType_out); }
  inline void getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out) { getMediaType (mediaType_in.audio, type_in, mediaType_out); }
  void getMediaType (const struct _AMMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType&);
  inline void getMediaType (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { getMediaType (mediaType_in.video, type_in, mediaType_out); }
  inline void getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { getMediaType (mediaType_in.video, type_in, mediaType_out); }
  void getMediaType (const struct _AMMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);
  void getMediaType (const struct _AMMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_MediaType&);

  // helper methods
  void setFormat (enum AVSampleFormat, struct _AMMediaType&);
  inline void setFormat (enum AVSampleFormat format_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.audio.format = format_in; }
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.video.format = format_in; }
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout.format = format_in; }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.video.resolution = resolution_in; }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout.resolution = resolution_in; }
  inline void setSampleRate (const unsigned int sampleRate_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.audio.sampleRate = sampleRate_in; }
  void setSampleRate (unsigned int, struct _AMMediaType&);
  inline void setChannels (const unsigned int channels_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.audio.channels = channels_in; }
  void setChannels (unsigned int, struct _AMMediaType&);
  inline void free_ (struct Stream_MediaFramework_FFMPEG_MediaType&) {}
  inline void free_ (struct _AMMediaType& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::free (mediaType_inout); }

  void getMediaType (const IMFMediaType*, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType&);
  void getMediaType (const IMFMediaType*, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);

  void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType&, enum Stream_MediaType_Type, struct _AMMediaType&);
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_AudioMediaType&, enum Stream_MediaType_Type, struct _AMMediaType&);
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, enum Stream_MediaType_Type, struct _AMMediaType&);
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType&, enum Stream_MediaType_Type, IMFMediaType*&);
#endif // FFMPEG_SUPPORT

  inline Common_Image_Resolution_t getResolution (const struct _AMMediaType& mediaType_in) { return Stream_MediaFramework_DirectShow_Tools::toResolution (mediaType_in); }
  inline Common_Image_Resolution_t getResolution (const IMFMediaType* mediaType_in) { return Stream_MediaFramework_MediaFoundation_Tools::toResolution (mediaType_in); }

  inline void set (struct _AMMediaType& mediaType_in, enum Stream_MediaType_Type, struct _AMMediaType& mediaType_out) { mediaType_out = mediaType_in; }
  void set (struct _AMMediaType&,                                       // media type
            enum Stream_MediaType_Type,                                 // media type type
            struct Stream_MediaFramework_DirectShow_AudioVideoFormat&); // return value: media type
  inline void set (IMFMediaType* mediaType_in, enum Stream_MediaType_Type, IMFMediaType* mediaType_out) { mediaType_out = mediaType_in; }
  void set (IMFMediaType*,                                                   // media type
            enum Stream_MediaType_Type,                                      // media type type
            struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat&); // return value: media type

  inline void setFormat (REFGUID format_in, struct _AMMediaType& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setFormat (format_in, mediaType_inout); }
  inline void setResolution (const Common_Image_Resolution_t resolution_in, struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_in, mediaType_inout.video); }
  inline void setResolution (const Common_Image_Resolution_t resolution_in, struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_inout) { Stream_MediaFramework_MediaFoundation_Tools::setResolution (resolution_in, mediaType_inout.video); }
  inline void setResolution (const Common_Image_Resolution_t resolution_in, struct _AMMediaType& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_in, mediaType_inout); }
  inline void setResolution (const Common_Image_Resolution_t resolution_in, IMFMediaType* mediaType_inout) { Stream_MediaFramework_MediaFoundation_Tools::setResolution (resolution_in, mediaType_inout); }
  inline void free_ (struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in) { Stream_MediaFramework_DirectShow_Tools::free (mediaType_in); }
  inline void free_ (IMFMediaType* mediaType_in) { mediaType_in->Release (); }

#if defined (FFMPEG_SUPPORT)
  // ffmpeg
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setFormat (Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (format_in), mediaType_inout.video); }
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_inout) { Stream_MediaFramework_MediaFoundation_Tools::setFormat (Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (format_in), mediaType_inout.video); }
  inline void setFormat (enum AVPixelFormat format_in, struct _AMMediaType& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setFormat (Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (format_in), mediaType_inout); }
  inline void setFormat (enum AVPixelFormat format_in, IMFMediaType* mediaType_inout) { Stream_MediaFramework_MediaFoundation_Tools::setFormat (Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (format_in), mediaType_inout); }

  void setFramerate (const struct AVRational&, struct _AMMediaType&);
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_out) { setFramerate (rate_in, mediaType_out.video); }
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out) { mediaType_out.video.frameRate = rate_in; }
#endif // FFMPEG_SUPPORT
#else
  // ALSA
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_out) { mediaType_out = mediaType_in.audio; }
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_ALSA_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#if defined (FFMPEG_SUPPORT)
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_out) { getMediaType (mediaType_in.audio, type_in, mediaType_out); }
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out) { getMediaType (mediaType_in.audio, type_in, mediaType_out); }
  void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType&);
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // FFMPEG_SUPPORT

  inline Common_Image_Resolution_t getResolution (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in) { Common_Image_Resolution_t result; return result; }

#if defined (FFMPEG_SUPPORT)
  // ffmpeg
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { getMediaType (mediaType_in.video, type_in, mediaType_out); }
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_AudioMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_ALSA_MediaType&);
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_V4L_MediaType&);
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out) { mediaType_out = mediaType_in.audio; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { mediaType_out = mediaType_in.video; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { mediaType_out = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out) { ACE_UNUSED_ARG (mediaType_in); struct Stream_MediaFramework_FFMPEG_AudioMediaType dummy; mediaType_out = dummy; }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_ALSA_MediaType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // FFMPEG_SUPPORT

  // V4L
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { mediaType_out = mediaType_in.video; }
  inline void getMediaType (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
#if defined (FFMPEG_SUPPORT)
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { getMediaType (mediaType_in.video, type_in, mediaType_out); }
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_out) { getMediaType (mediaType_in, type_in, mediaType_out.video); }
  void getMediaType (const struct Stream_MediaFramework_V4L_MediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);
  void getMediaType (const struct Stream_MediaFramework_V4L_MediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType&);
#endif // FFMPEG_SUPPORT

  inline void getMediaType (const struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_in, enum Stream_MediaType_Type type_in, struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_out) { mediaType_out = mediaType_in; }

  inline void setFormat (__u32 format_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_inout) { mediaType_inout.format.pixelformat = format_in; }
  inline Common_Image_Resolution_t getResolution (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in) { Common_Image_Resolution_t result; result.height = mediaType_in.format.height; result.width = mediaType_in.format.width; return result; }

#if defined (FFMPEG_SUPPORT)
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_inout) { mediaType_inout.video.format.pixelformat = Stream_MediaFramework_Tools::ffmpegFormatToV4lFormat (format_in); }
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_inout) { mediaType_inout.format.pixelformat = Stream_MediaFramework_Tools::ffmpegFormatToV4lFormat (format_in); }
  inline void setFormat (enum AVSampleFormat format_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_inout) { mediaType_inout.format = Stream_MediaFramework_Tools::ffmpegFormatToALSAFormat (format_in); }
  inline void setFormat (enum AVSampleFormat format_in, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_inout) { mediaType_inout.format = format_in; }
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout.format = format_in; }
  inline void setFormat (enum AVSampleFormat format_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.audio.format = format_in; }
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.video.format = format_in; }
  inline void setSampleRate (unsigned int sampleRate_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_inout) { mediaType_inout.rate = sampleRate_in; }
  inline void setSampleRate (unsigned int sampleRate_in, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_inout) { mediaType_inout.sampleRate = sampleRate_in; }
  inline void setSampleRate (unsigned int sampleRate_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.audio.sampleRate = sampleRate_in; }
  inline void setChannels (unsigned int channels_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_inout) { mediaType_inout.channels = channels_in; }
  inline void setChannels (unsigned int channels_in, struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_inout) { mediaType_inout.channels = channels_in; }
  inline void setChannels (unsigned int channels_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.audio.channels = channels_in; }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_inout) { mediaType_inout.video.resolution = resolution_in; }
  inline Common_Image_Resolution_t getResolution (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in) { Common_Image_Resolution_t result; result = mediaType_in.resolution; return result; }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout.resolution = resolution_in; }
  inline void free_ (struct Stream_MediaFramework_ALSA_MediaType&) {}
  inline void free_ (struct Stream_MediaFramework_FFMPEG_AudioMediaType&) {}
  inline void free_ (struct Stream_MediaFramework_FFMPEG_MediaType&) {}
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out) { mediaType_out.frameRate = rate_in; }
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out) { mediaType_out.video.frameRate = rate_in; }
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { mediaType_out.frameRate.numerator = rate_in.num; mediaType_out.frameRate.denominator = rate_in.den; }
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_out) { mediaType_out.video.frameRate.numerator = rate_in.num; mediaType_out.video.frameRate.denominator = rate_in.den; }
#endif // FFMPEG_SUPPORT
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_ALSA_V4L_Format& mediaType_inout) { mediaType_inout.video.format.width = resolution_in.width; mediaType_inout.video.format.height = resolution_in.height; }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_inout) { mediaType_inout.format.width = resolution_in.width; mediaType_inout.format.height = resolution_in.height; }
  inline void free_ (struct Stream_MediaFramework_ALSA_V4L_Format&) {}
  inline void free_ (struct Stream_MediaFramework_V4L_MediaType&) {}

#if defined (LIBCAMERA_SUPPORT)
  // libCamera
#if defined (FFMPEG_SUPPORT)
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_inout) { mediaType_inout.format = Stream_MediaFramework_Tools::ffmpegFormatToLibCameraFormat (format_in); }
  inline void setFramerate (const struct AVRational& rate_in, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_out) { mediaType_out.frameRateNumerator = rate_in.num; mediaType_out.frameRateDenominator = rate_in.den; }
#endif // FFMPEG_SUPPORT
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_inout) { mediaType_inout.resolution.width = resolution_in.width; mediaType_inout.resolution.height = resolution_in.height; }
  inline void free_ (struct Stream_MediaFramework_LibCamera_MediaType&) {}

  inline void getMediaType (const struct Stream_MediaFramework_LibCamera_MediaType& mediaType_in, enum Stream_MediaType_Type, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
#if defined (FFMPEG_SUPPORT)
  void getMediaType (const struct Stream_MediaFramework_LibCamera_MediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);
  void getMediaType (const struct Stream_MediaFramework_LibCamera_MediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_FFMPEG_AudioMediaType&);
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, enum Stream_MediaType_Type, struct Stream_MediaFramework_LibCamera_MediaType&);
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T (const Stream_MediaFramework_MediaTypeConverter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T& operator= (const Stream_MediaFramework_MediaTypeConverter_T&))
};

// include template definition
#include "stream_lib_mediatype_converter.inl"

#endif
