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
#include <ks.h>
#include <mfobjects.h>
#include <strmif.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#else
#include "stream_lib_alsa_common.h"
#include "stream_lib_v4l_common.h"
#if defined (LIBCAMERA_SUPPORT)
#include "stream_lib_libcamera_common.h"
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_ffmpeg_common.h"

#include "stream_lib_tools.h"

template <typename MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         >
#else
          ,typename SessionDataType>
#endif // ACE_WIN32 || ACE_WIN64
class Stream_MediaFramework_MediaTypeConverter_T
{
 public:
  Stream_MediaFramework_MediaTypeConverter_T ();
  inline virtual ~Stream_MediaFramework_MediaTypeConverter_T () {}

 protected:
  // ffmpeg
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout = mediaType_in; }

  // helper methods
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout.format = format_in; }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_inout) { mediaType_inout.resolution = resolution_in; }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: struct _AMMediaType return values need to be Stream_Module_Device_DirectShow_Tools::free'd !
  inline void getMediaType (const struct _AMMediaType& mediaType_in, struct _AMMediaType& mediaType_out) { struct _AMMediaType* media_type_p = Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in); ACE_ASSERT (media_type_p); mediaType_out = *media_type_p; CoTaskMemFree (media_type_p); }
  void getMediaType (const struct _AMMediaType&, IMFMediaType*&);
  void getMediaType (const struct _AMMediaType&, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);

  void getMediaType (const IMFMediaType*, IMFMediaType*&);
  void getMediaType (const IMFMediaType*, struct _AMMediaType&);
  void getMediaType (const IMFMediaType*, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);

  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, struct _AMMediaType&);
//  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, IMFMediaType*&);

  // ffmpeg
  inline void setFormat (enum AVPixelFormat format_in, struct _AMMediaType& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setFormat (Stream_MediaFramework_Tools::AVPixelFormatToMediaSubType (format_in), mediaType_inout); }

  inline void setResolution (const Common_Image_Resolution_t resolution_in, struct _AMMediaType& mediaType_inout) { Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_in, mediaType_inout); }
#else
  // ALSA
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType&, struct Stream_MediaFramework_FFMPEG_VideoMediaType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  // ffmpeg
  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, struct Stream_MediaFramework_V4L_MediaType&);

  // V4L
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_inout) { mediaType_inout.format.pixelformat = Stream_MediaFramework_Tools::ffmpegFormatToV4L2Format (format_in); }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_inout) { mediaType_inout.format.width = resolution_in.width; mediaType_inout.format.height = resolution_in.height; }

  inline void getMediaType (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  void getMediaType (const struct Stream_MediaFramework_V4L_MediaType&, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);

#if defined (LIBCAMERA_SUPPORT)
  // libCamera
  inline void setFormat (enum AVPixelFormat format_in, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_inout) { mediaType_inout.format = Stream_MediaFramework_Tools::ffmpegFormatToLibCameraFormat (format_in); }
  inline void setResolution (const Common_Image_Resolution_t& resolution_in, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_inout) { mediaType_inout.resolution.width = resolution_in.width; mediaType_inout.resolution.height = resolution_in.height; }

  inline void getMediaType (const struct Stream_MediaFramework_LibCamera_MediaType& mediaType_in, struct Stream_MediaFramework_LibCamera_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  void getMediaType (const struct Stream_MediaFramework_LibCamera_MediaType&, struct Stream_MediaFramework_FFMPEG_VideoMediaType&);

  void getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&, struct Stream_MediaFramework_LibCamera_MediaType&);
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T (const Stream_MediaFramework_MediaTypeConverter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T& operator= (const Stream_MediaFramework_MediaTypeConverter_T&))
};

// include template definition
#include "stream_lib_mediatype_converter.inl"

#endif
