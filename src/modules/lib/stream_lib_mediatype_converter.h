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
#include <mfobjects.h>
#include <strmif.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_lib_ffmpeg_common.h"

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
//  template <typename T> MediaType getMediaType (const T& mediaType_in) { MediaType result; getMediaType_impl (mediaType_in, result); return result; }
//  template <typename T, typename U> T getMediaType_2 (const U& mediaType_in) { T result; getMediaType_impl (mediaType_in, result); return result; }
//  template <typename T, typename U> T getMediaType_2 (const U&) { T result; ACE_ASSERT (false); ACE_NOTSUP_RETURN (result); ACE_NOTREACHED (return result;) }

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  // *IMPORTANT NOTE*: return values need to be Stream_Module_Device_DirectShow_Tools::free'd !
//  void getMediaType_impl (const struct _AMMediaType&, struct _AMMediaType&);
//  void getMediaType_impl (const struct _AMMediaType&, struct Stream_MediaFramework_FFMPEG_MediaType&);

//  void getMediaType_impl (const IMFMediaType*&, struct _AMMediaType&);
//  void getMediaType_impl (const IMFMediaType*&, struct Stream_MediaFramework_FFMPEG_MediaType&);

//  void getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType&, struct _AMMediaType&);
//  void getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType&, IMFMediaType*&);
#else
  inline void getMediaType (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  void getMediaType (const struct Stream_MediaFramework_V4L_MediaType&, struct Stream_MediaFramework_FFMPEG_MediaType&);

  void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType&, struct Stream_MediaFramework_V4L_MediaType&);

  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
  inline void getMediaType (const struct Stream_MediaFramework_ALSA_MediaType&, struct Stream_MediaFramework_FFMPEG_MediaType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // ACE_WIN32 || ACE_WIN64
  inline void getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out) { mediaType_out = mediaType_in; }

 private:
//  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T (const Stream_MediaFramework_MediaTypeConverter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaTypeConverter_T& operator= (const Stream_MediaFramework_MediaTypeConverter_T&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *IMPORTANT NOTE*: return values need to be Stream_Module_Device_DirectShow_Tools::free'd !
  void getMediaType_impl (const struct _AMMediaType&, struct _AMMediaType&);
  void getMediaType_impl (const struct _AMMediaType&, struct Stream_MediaFramework_FFMPEG_MediaType&);

  void getMediaType_impl (const IMFMediaType*&, struct _AMMediaType&);
  void getMediaType_impl (const IMFMediaType*&, struct Stream_MediaFramework_FFMPEG_MediaType&);

  void getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType&, struct _AMMediaType&);
  void getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType&, IMFMediaType*&);
#else
//  inline void getMediaType_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in, struct Stream_MediaFramework_V4L_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
//  void getMediaType_impl (const struct Stream_MediaFramework_V4L_MediaType&, struct Stream_MediaFramework_FFMPEG_MediaType&);

//  inline void getMediaType_impl (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in, struct Stream_MediaFramework_ALSA_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
//  inline void getMediaType_impl (const struct Stream_MediaFramework_ALSA_MediaType&, struct Stream_MediaFramework_FFMPEG_MediaType&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif // ACE_WIN32 || ACE_WIN64
//  inline void getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in, struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out) { mediaType_out = mediaType_in; }
};

// include template definition
#include "stream_lib_mediatype_converter.inl"

#endif
