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
#include "ace/OS.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_RGB_HFlip_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           MediaType>::Stream_Decoder_RGB_HFlip_T (ISTREAM_T* stream_in)
#else
                           MediaType>::Stream_Decoder_RGB_HFlip_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , bytesPerPixel_ (0)
 , resolution_ ({0, 0})
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_RGB_HFlip_T::Stream_Decoder_RGB_HFlip_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Decoder_RGB_HFlip_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           MediaType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_RGB_HFlip_T::initialize"));

  if (inherited::isInitialized_)
  {
    bytesPerPixel_ = 0;
    resolution_ = {0, 0};
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_RGB_HFlip_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_RGB_HFlip_T::handleDataMessage"));

  uint8_t* data_p = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  uint8_t* data_2;
  int offset_i;

  switch (bytesPerPixel_)
  {
    case 3:
    {
      uint8_t r, g, b;
#if defined (ACE_WIN64) || defined (ACE_WIN64)
      for (LONG y = 0; y < resolution_.cy; ++y)
      {
        data_2 = data_p;
        for (LONG x = 0; x < resolution_.cx / 2; ++x)
        {
          r = data_p[0]; g = data_p[1]; b = data_p[2];
          offset_i = (resolution_.cx - x) * 3;
          data_p[0] = data_2[offset_i - 3];
          data_p[1] = data_2[offset_i - 2];
          data_p[2] = data_2[offset_i - 1];
          data_2[offset_i - 3] = r;
          data_2[offset_i - 2] = g;
          data_2[offset_i - 1] = b;
          data_p += 3;
        } // end FOR
        data_p += (resolution_.cx / 2) * 3;
      } // end FOR
#else
      for (unsigned int y = 0; y < resolution_.height; ++y)
      {
        data_2 = data_p;
        for (unsigned int x = 0; x < resolution_.width / 2; ++x)
        {
          r = data_p[0]; g = data_p[1]; b = data_p[2];
          offset_i = (resolution_.width - x) * 3;
          data_p[0] = data_2[offset_i - 3];
          data_p[1] = data_2[offset_i - 2];
          data_p[2] = data_2[offset_i - 1];
          data_2[offset_i - 3] = r;
          data_2[offset_i - 2] = g;
          data_2[offset_i - 1] = b;
          data_p += 3;
        } // end FOR
        data_p += (resolution_.width / 2) * 3;
      } // end FOR
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    case 4:
    {
      uint8_t r, g, b, a;
#if defined (ACE_WIN64) || defined (ACE_WIN64)
      for (LONG y = 0; y < resolution_.cy; ++y)
      {
        data_2 = data_p;
        for (LONG x = 0; x < resolution_.cx / 2; ++x)
        {
          r = data_p[0]; g = data_p[1]; b = data_p[2]; a = data_p[3];
          offset_i = (resolution_.cx - x) * 4;
          data_p[0] = data_2[offset_i - 4];
          data_p[1] = data_2[offset_i - 3];
          data_p[2] = data_2[offset_i - 2];
          data_p[3] = data_2[offset_i - 1];
          data_2[offset_i - 4] = r;
          data_2[offset_i - 3] = g;
          data_2[offset_i - 2] = b;
          data_2[offset_i - 1] = a;
          data_p += 4;
        } // end FOR
        data_p += (resolution_.cx / 2) * 4;
      } // end FOR
#else
      for (unsigned int y = 0; y < resolution_.height; ++y)
      {
        data_2 = data_p;
        for (unsigned int x = 0; x < resolution_.width / 2; ++x)
        {
          r = data_p[0]; g = data_p[1]; b = data_p[2]; a = data_p[3];
          offset_i = (resolution_.width - x) * 4;
          data_p[0] = data_2[offset_i - 4];
          data_p[1] = data_2[offset_i - 3];
          data_p[2] = data_2[offset_i - 2];
          data_p[3] = data_2[offset_i - 1];
          data_2[offset_i - 4] = r;
          data_2[offset_i - 3] = g;
          data_2[offset_i - 2] = b;
          data_2[offset_i - 1] = a;
          data_p += 4;
        } // end FOR
        data_p += (resolution_.width / 2) * 4;
      } // end FOR
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid bytes-per-pixel value (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  bytesPerPixel_));
      goto error;
    }
  } // end SWITCH

  return;

error:
  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_RGB_HFlip_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_RGB_HFlip_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);

      bytesPerPixel_ =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_s) / 8;
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      if (!Stream_MediaFramework_Tools::isRGB (media_type_s.subtype,
                                               STREAM_MEDIAFRAMEWORK_DIRECTSHOW))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid media subtype format (was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_type_s.subtype).c_str ())));
        goto error;
      } // end IF
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      bytesPerPixel_ =
        Stream_MediaFramework_Tools::ffmpegFormatToBitDepth (media_type_s.format) / 8;
      resolution_ = media_type_s.resolution;
      if (!Stream_Module_Decoder_Tools::isRGB (media_type_s.format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid pixel format (was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_s.format).c_str ())));
        goto error;
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64

      break;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);

      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      resolution_ = media_type_s.resolution;
#endif // ACE_WIN32 || ACE_WIN64

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}
