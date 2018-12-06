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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_dev_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

template <typename MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
         >
#else
          ,typename SessionDataType>
#endif // ACE_WIN32 || ACE_WIN64
Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                          >::Stream_MediaFramework_MediaTypeConverter_T ()
#else
                                          ,SessionDataType>::Stream_MediaFramework_MediaTypeConverter_T ()
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::Stream_MediaFramework_MediaTypeConverter_T"));

}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const struct _AMMediaType& mediaType_in,
                                                                          struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
                                                             &mediaType_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const IMFMediaType*& mediaType_in,
                                                                          struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (&const_cast<IMFMediaType&> (mediaType_in),
                                        GUID_NULL,
                                        &mediaType_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in,
                                                                          struct _AMMediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const struct _AMMediaType& mediaType_in,
                                                                          struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

  mediaType_out.format =
      Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat (subtype_s,
                                                                STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  mediaType_out.frameRate =
      Stream_MediaFramework_DirectShow_Tools::toFramerate (mediaType_in);
  mediaType_out.resolution =
      Stream_MediaFramework_DirectShow_Tools::toResolution (mediaType_in);
}

template <typename MediaType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const IMFMediaType*& mediaType_in,
                                                                          struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _AMMediaType media_type_s;
  getMediaType_impl (mediaType_in,
                     media_type_s);
  getMediaType_impl (media_type_s,
                     mediaType_out);

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
}
#else
//template <typename MediaType,
//          typename SessionDataType>
//void
//Stream_MediaFramework_MediaTypeConverter_T<MediaType,
//                                           SessionDataType>::getMediaType_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in,
//                                                                                struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

//  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

//  mediaType_out.format =
//      Stream_Device_Tools::v4l2FormatToffmpegFormat (mediaType_in.format.pixelformat);
//  mediaType_out.resolution.width = mediaType_in.format.width;
//  mediaType_out.resolution.height = mediaType_in.format.height;
//  mediaType_out.frameRate.den = mediaType_in.frameRate.denominator;
//  mediaType_out.frameRate.num = mediaType_in.frameRate.numerator;
//}

template <typename MediaType,
          typename SessionDataType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType,
                                           SessionDataType>::getMediaType (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in,
                                                                           struct Stream_MediaFramework_V4L_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_V4Ls_MediaType));

  mediaType_out.format.pixelformat =
      Stream_Device_Tools::ffmpegFormatToV4L2Format (mediaType_in.format);
  mediaType_out.format.width = mediaType_in.resolution.width;
  mediaType_out.format.height = mediaType_in.resolution.height;
  mediaType_out.frameRate.denominator = mediaType_in.frameRate.den;
  mediaType_out.frameRate.numerator = mediaType_in.frameRate.num;
}

template <typename MediaType,
          typename SessionDataType>
void
Stream_MediaFramework_MediaTypeConverter_T<MediaType,
                                           SessionDataType>::getMediaType (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in,
                                                                           struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType"));

  ACE_OS::memset (&mediaType_out, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

  mediaType_out.format =
      Stream_Device_Tools::v4l2FormatToffmpegFormat (mediaType_in.format.pixelformat);
  mediaType_out.resolution.width = mediaType_in.format.width;
  mediaType_out.resolution.height = mediaType_in.format.height;
  mediaType_out.frameRate.den = mediaType_in.frameRate.denominator;
  mediaType_out.frameRate.num = mediaType_in.frameRate.numerator;
}

#endif // ACE_WIN32 || ACE_WIN64
