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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename MediaType>
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::Stream_MediaFramework_MediaTypeConverter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::Stream_MediaFramework_MediaTypeConverter_T"));

}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (FFMPEG_SUPPORT)
template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct _AMMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_AudioMediaType));

  mediaType_out.format =
      Stream_MediaFramework_DirectShow_Tools::toAVSampleFormat (mediaType_in);
  mediaType_out.channels =
      Stream_MediaFramework_DirectShow_Tools::toChannels (mediaType_in);
  mediaType_out.sampleRate =
      Stream_MediaFramework_DirectShow_Tools::toFramerate (mediaType_in);
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct _AMMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  mediaType_out.format =
      Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat (mediaType_in.subtype,
                                                                STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  mediaType_out.frameRate.den = 1;
  mediaType_out.frameRate.num =
      Stream_MediaFramework_DirectShow_Tools::toFramerate (mediaType_in);
  mediaType_out.resolution =
      Stream_MediaFramework_DirectShow_Tools::toResolution (mediaType_in);
}
#endif // FFMPEG_SUPPORT

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  switch (type_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in.audio,
                                                    mediaType_out);
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in.video,
                                                    mediaType_out);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::set (struct _AMMediaType& mediaType_in,
                                                            enum Stream_MediaType_Type type_in,
                                                            struct Stream_MediaFramework_DirectShow_AudioVideoFormat& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::set"));

  switch (type_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      Stream_MediaFramework_DirectShow_Tools::free (mediaType_out.audio);
      mediaType_out.audio = mediaType_in;
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      Stream_MediaFramework_DirectShow_Tools::free (mediaType_out.video);
      mediaType_out.video = mediaType_in;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::set (IMFMediaType* mediaType_in,
                                                            enum Stream_MediaType_Type type_in,
                                                            struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::set"));

  switch (type_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      if (mediaType_out.audio)
        mediaType_out.audio->Release ();
      mediaType_out.audio = mediaType_in;
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      if (mediaType_out.video)
        mediaType_out.video->Release ();
      mediaType_out.video = mediaType_in;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct _AMMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     IMFMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  HRESULT result = MFCreateMediaType (&mediaType_out);
  if (unlikely (FAILED (result) || !mediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
  result =
    MFInitMediaTypeFromAMMediaType (mediaType_out,
                                    &mediaType_in);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFInitMediaTypeFromAMMediaType(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaType_out->Release (); mediaType_out = NULL;
    return;
  } // end IF
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  IMFMediaType* media_type_p = NULL;
  getMediaType (mediaType_in,
                type_in,
                media_type_p);
  ACE_ASSERT (media_type_p);
  getMediaType (media_type_p,
                type_in,
                mediaType_out);
  media_type_p->Release ();
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_MediaFoundation_AudioVideoFormat& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     IMFMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  switch (type_in)
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      mediaType_out =
        Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in.audio);
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      mediaType_out =
        Stream_MediaFramework_MediaFoundation_Tools::copy (mediaType_in.video);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type type (was: %d), returning\n"),
                  type_in));
      return;
    }
  } // end SWITCH
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const IMFMediaType* mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     IMFMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF
  
  HRESULT result = MFCreateMediaType (&mediaType_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (mediaType_out);
  result =
    const_cast<IMFMediaType*> (mediaType_in)->CopyAllItems (mediaType_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::CopyAllItems(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    mediaType_out->Release (); mediaType_out = NULL;
    return;
  } // end IF
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const IMFMediaType* mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct _AMMediaType& mediaType_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  // initialize return value(s)
  Stream_MediaFramework_DirectShow_Tools::free (mediaType_inout);

  struct _AMMediaType* media_type_p = NULL;
  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (mediaType_in),
                                        GUID_NULL,
                                        &media_type_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_type_p);
  mediaType_inout = *media_type_p;

  CoTaskMemFree (media_type_p); media_type_p = NULL;
}

#if defined (FFMPEG_SUPPORT)
template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const IMFMediaType* mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  getMediaType (mediaType_in,
                type_in,
                media_type_s);
  getMediaType (media_type_s,
                type_in,
                mediaType_out);

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const IMFMediaType* mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  getMediaType (mediaType_in,
                type_in,
                media_type_s);
  getMediaType (media_type_s,
                type_in,
                mediaType_out);

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  Stream_MediaFramework_DirectShow_Tools::free (mediaType_out);

  struct _AMMediaType* media_type_p =
    Stream_MediaFramework_DirectShow_Tools::to (mediaType_in);
  ACE_ASSERT (media_type_p);
  mediaType_out = *media_type_p;
}
#endif // FFMPEG_SUPPORT
#else
#if defined (FFMPEG_SUPPORT)
template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_ALSA_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));

  mediaType_out.format =
      Stream_MediaFramework_Tools::ffmpegFormatToALSAFormat (mediaType_in.format);
  mediaType_out.channels = mediaType_in.channels;
  mediaType_out.rate = mediaType_in.sampleRate;
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_V4L_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));

  mediaType_out.format.pixelformat =
      Stream_MediaFramework_Tools::ffmpegFormatToV4lFormat (mediaType_in.format);
  mediaType_out.format.width = mediaType_in.resolution.width;
  mediaType_out.format.height = mediaType_in.resolution.height;
  mediaType_out.frameRate.numerator = mediaType_in.frameRate.num;
  mediaType_out.frameRate.denominator = mediaType_in.frameRate.den;
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_AudioMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  mediaType_out.format =
      Stream_MediaFramework_Tools::ALSAFormatToffmpegFormat (mediaType_in.format);
  mediaType_out.channels = mediaType_in.channels;
  mediaType_out.sampleRate = mediaType_in.rate;
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  mediaType_out.format =
      Stream_MediaFramework_Tools::v4lFormatToffmpegFormat (mediaType_in.format.pixelformat);
  mediaType_out.resolution.width = mediaType_in.format.width;
  mediaType_out.resolution.height = mediaType_in.format.height;
  mediaType_out.frameRate.den = mediaType_in.frameRate.denominator;
  mediaType_out.frameRate.num = mediaType_in.frameRate.numerator;
}
#endif // FFMPEG_SUPPORT

#if defined (LIBCAMERA_SUPPORT)
#if defined (FFMPEG_SUPPORT)
template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_LibCamera_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  mediaType_out.format =
      Stream_MediaFramework_Tools::ffmpegFormatToLibCameraFormat (mediaType_in.format);
  mediaType_out.resolution.width = mediaType_in.resolution.width;
  mediaType_out.resolution.height = mediaType_in.resolution.height;
  mediaType_out.frameRateNumerator = mediaType_in.frameRate.num;
  mediaType_out.frameRateDenominator = mediaType_in.frameRate.den;
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType (const struct Stream_MediaFramework_LibCamera_MediaType& mediaType_in,
                                                                     enum Stream_MediaType_Type type_in,
                                                                     struct Stream_MediaFramework_FFMPEG_VideoMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_UNUSED_ARG (type_in);

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_VideoMediaType));

  mediaType_out.format =
      Stream_MediaFramework_Tools::libCameraFormatToffmpegFormat (mediaType_in.format);
  mediaType_out.resolution.width = mediaType_in.resolution.width;
  mediaType_out.resolution.height = mediaType_in.resolution.height;
  mediaType_out.frameRate.num = mediaType_in.frameRateNumerator;
  mediaType_out.frameRate.den = mediaType_in.frameRateDenominator;
}
#endif // FFMPEG_SUPPORT
#endif // LIBCAMERA_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
