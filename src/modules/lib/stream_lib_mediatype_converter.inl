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
AM_MEDIA_TYPE
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  struct _AMMediaType* result_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
                                                             result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                inherited::mod_->name ()));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}

template <typename MediaType>
AM_MEDIA_TYPE
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const IMFMediaType*& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _AMMediaType* result_p = NULL;

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (&const_cast<IMFMediaType&> (mediaType_in),
                                        GUID_NULL,
                                        &result_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}

template <typename MediaType>
AM_MEDIA_TYPE
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const struct Stream_MediaFramework_FFMPEG_MediaType& mediaType_in)
{
  ACE_ASSERT (false); // *TODO*

  struct _AMMediaType* result_p = NULL;
//  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
//                                                             result_p))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
//                inherited::mod_->name ()));
//    return struct _AMMediaType (); // *TODO*: will crash
//  } // end IF
//  ACE_ASSERT (result_p);

  return *result_p;
}

template <typename MediaType>
struct Stream_MediaFramework_FFMPEG_MediaType
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  struct Stream_MediaFramework_FFMPEG_MediaType result_s;
  ACE_OS::memset (&result_s, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

  result_s.format =
      Stream_Module_Decoder_Tools::mediaSubTypeToAVPixelFormat (subtype_s,
                                                                STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
  result_s.frameRate =
      Stream_MediaFramework_DirectShow_Tools::toFramerate (mediaType_in);
  result_s.resolution =
      Stream_MediaFramework_DirectShow_Tools::toResolution (mediaType_in);

  return result_s;
}

template <typename MediaType>
struct Stream_MediaFramework_FFMPEG_MediaType
Stream_MediaFramework_MediaTypeConverter_T<MediaType>::getMediaType_impl (const IMFMediaType*& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  struct Stream_MediaFramework_FFMPEG_MediaType result_s;
  ACE_OS::memset (&result_s, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _AMMediaType media_type_s = getMediaType_impl (mediaType_in);
  result_s = getMediaType_impl (media_type_s);

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return result_s;
}
#else
template <typename MediaType,
          typename SessionDataType>
struct Stream_MediaFramework_FFMPEG_MediaType
Stream_MediaFramework_MediaTypeConverter_T<MediaType,
                                           SessionDataType>::getMediaType_impl (const struct Stream_MediaFramework_V4L_MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaTypeConverter_T::getMediaType_impl"));

  struct Stream_MediaFramework_FFMPEG_MediaType result_s;
  ACE_OS::memset (&result_s, 0, sizeof (struct Stream_MediaFramework_FFMPEG_MediaType));

  result_s.format =
      Stream_Device_Tools::v4l2FormatToffmpegFormat (mediaType_in.format.pixelformat);
  result_s.resolution.width = mediaType_in.format.width;
  result_s.resolution.height = mediaType_in.format.height;
  result_s.frameRate.den = mediaType_in.frameRate.denominator;
  result_s.frameRate.num = mediaType_in.frameRate.numerator;

  return result_s;
}
#endif // ACE_WIN32 || ACE_WIN64
