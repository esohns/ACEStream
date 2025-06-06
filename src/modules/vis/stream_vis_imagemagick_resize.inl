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

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
#include "wand/wand_api.h"
#else
#if defined (ACE_LINUX)
#if defined (IS_UBUNTU_LINUX) // *NOTE*: github "*-latest" runners lag behind:
#include "wand/MagickWand.h" //          - Ubuntu 'noble' still is on ImageMagick-6
#else
#include "MagickWand/MagickWand.h"
#endif // IS_UBUNTU_LINUX
#else
#include "MagickWand/MagickWand.h"
#endif // ACE_LINUX
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK

#include "ace/Log_Msg.h"

#include "common_image_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_v4l_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Visualization_ImageMagickResize_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                         MediaType>::Stream_Visualization_ImageMagickResize_T (ISTREAM_T* stream_in)
#else
                                         MediaType>::Stream_Visualization_ImageMagickResize_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , sourceResolution_ ()
 , targetResolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize_T::Stream_Visualization_ImageMagickResize_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Visualization_ImageMagickResize_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::context_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  unsigned int result = MagickTrue;
  unsigned char* data_p = NULL;
  size_t size_i = 0;
  int result_2 = -1;
  Stream_SessionId_t session_id = message_inout->sessionId ();
  DataMessageType* message_p = NULL;
  typename DataMessageType::DATA_T message_data_s;
  //message_data_s.format = message_data_r.format;

  // allocate a message buffer for the next frame
  message_p = inherited::allocateMessage (1);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                1));
    goto error;
  } // end IF
  // sanity check(s)

  result = MagickSetSize (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          sourceResolution_.cx, sourceResolution_.cy);
#else
                          sourceResolution_.width, sourceResolution_.height);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result == MagickTrue);
  result =
    MagickSetFormat (inherited::context_,
                     ACE_TEXT_ALWAYS_CHAR ("RGBA")); // *TODO*: make this configurable !
  ACE_ASSERT (result == MagickTrue);

  result = MagickReadImage (inherited::context_,
                            ACE_TEXT_ALWAYS_CHAR ("xc:black"));
  ACE_ASSERT (result == MagickTrue);
//  result =
//    MagickNewImage (inherited::context_,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//                    resolution_s.cx, resolution_s.cy,
//#else
//                    message_data_r.format.resolution.width, message_data_r.format.resolution.height,
//#endif // ACE_WIN32 || ACE_WIN64
//                    pixelContext_);
//  ACE_ASSERT (result == MagickTrue);

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
  result =
    MagickReadImageBlob (inherited::context_,
                         reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()),
                         message_inout->length ());
#else
  result =
    MagickImportImagePixels (inherited::context_,
                             0, 0,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             sourceResolution_.cx, sourceResolution_.cy,
#else
                             sourceResolution_.width, sourceResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                             ACE_TEXT_ALWAYS_CHAR ("RGBA"), // *TODO*: make this configurable !
                             CharPixel,
                             message_inout->rd_ptr ());
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
  if (unlikely (result != MagickTrue))
  {
     ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to MagickImportImagePixels(): \"%s\", returning\n"),
                 inherited::mod_->name (),
                 ACE_TEXT (Common_Image_Tools::errorToString (inherited::context_).c_str ())));
     goto error;
  } // end IF

  message_inout->release (); message_inout = NULL;

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
  result = MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              targetResolution_.cx, targetResolution_.cy,
#else
                              targetResolution_.width, targetResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                              CubicFilter,
                              1.0); // blur
#else
  result =
#if (MagickLibVersion >= 0x700)
    MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       targetResolution_.cx, targetResolution_.cy,
#else
                       targetResolution_.width, targetResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                       CubicFilter);
#else
    MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       targetResolution_.cx, targetResolution_.cy,
#else
                       targetResolution_.width, targetResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                       CubicFilter,
                       1.0); // do not blur
#endif // MagickLibVersion >= 0x700
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
  ACE_ASSERT (result == MagickTrue);

  // *IMPORTANT NOTE*: "...Set the image depth to 8. If you are using the Q16
  //                   version of ImageMagick, the image depth is promoted from
  //                   8 to 16 when the image is resized. ..."
  result = MagickSetImageDepth (inherited::context_, 8);
  ACE_ASSERT (result == MagickTrue);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_ASSERT (Common_Image_Tools::stringToCodecId (MagickGetImageFormat (inherited::context_)) == AV_CODEC_ID_NONE);
#endif // ACE_WIN32 || ACE_WIN64

  data_p = MagickGetImageBlob (inherited::context_, // was: MagickWriteImageBlob
                               &size_i);
  if (unlikely (!data_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MagickGetImageBlob(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (inherited::context_).c_str ())));
    goto error;
  } // end IF
  // *TODO*: crashes in release()...(needs MagickRelinquishMemory())
  message_p->base (reinterpret_cast<char*> (data_p),
                   size_i,
                   ACE_Message_Block::DONT_DELETE); // own image data, but relinquish() in dtor
  message_p->wr_ptr (size_i);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_DirectShow_Tools::setResolution (targetResolution_,
                                                         message_data_2.format);
#else
  message_data_s.format.resolution = targetResolution_;
#endif // ACE_WIN32 || ACE_WIN64
  message_data_s.relinquishMemory = data_p;
  message_p->initialize (message_data_s,
                         session_id,
                         NULL);

//#if defined (_DEBUG)
//  Common_File_Tools::store (ACE_TEXT_ALWAYS_CHAR ("output.rgb"),
//                            reinterpret_cast<uint8_t*> (data_p),
//                            size_i);
//#endif // _DEBUG

  // forward the converted frame
  result_2 = inherited::put_next (message_p, NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    goto error;
  } // end IF
  message_p = NULL;

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Visualization_ImageMagickResize_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inference
  ACE_ASSERT (!session_data_r.formats.empty ());
  const MediaType& media_type_r = session_data_r.formats.back ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!inherited::context_);
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());

      MediaType media_type_s;
      Common_Image_Resolution_t resolution_s;

      // stash input format
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);
      inherited::inputFormat_ = media_type_2.format;
      sourceResolution_ = media_type_2.resolution;
      // sanity check(s)
      if (unlikely (Stream_Module_Decoder_Tools::isCompressedVideo (inherited::inputFormat_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: input format (was: %s) is compressed; cannot resize, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::inputFormat_).c_str ())));
        goto error;
      } // end IF
      // *TODO*: remove type inferences
      if (unlikely (!inherited::configuration_->window ||
                    (!inherited::configuration_->outputFormat.resolution.width ||
                     !inherited::configuration_->outputFormat.resolution.height)))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: output size and/or -window not set, continuing\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
                    sourceResolution_.cx, sourceResolution_.cy,
                    inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
                    sourceResolution_.width, sourceResolution_.height,
                    inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      targetResolution_ = inherited::configuration_->outputFormat.resolution;
#else
      targetResolution_ = inherited::configuration_->outputFormat.resolution;
#endif // ACE_WIN32 || ACE_WIN64
      media_type_s = media_type_r;
      inherited::setResolution (resolution_s,
                                media_type_s);
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.formats.push_back (media_type_s);
      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      int result = -1;
      int flags_i = 0;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.cx, media_type_2.resolution.cy,
                  inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.width, media_type_2.resolution.height,
                  inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
      sourceResolution_ = media_type_2.resolution;

      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Visualization_ImageMagickResize1_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                          MediaType>::Stream_Visualization_ImageMagickResize1_T (ISTREAM_T* stream_in)
#else
                                          MediaType>::Stream_Visualization_ImageMagickResize1_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , targetResolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize1_T::Stream_Visualization_ImageMagickResize1_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Visualization_ImageMagickResize1_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          MediaType>::initialize (const ConfigurationType& configuration_in,
                                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize1_T::initialize"));

  if (!inherited::initialize (configuration_in,
                              allocator_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Decoder_LibAVConverter_T::initialize, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (inherited::context_);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Visualization_ImageMagickResize1_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize1_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  unsigned int result = MagickTrue;
  unsigned char* data_p = NULL;
  size_t size_i = 0;
  int result_2 = -1;
  Stream_SessionId_t session_id = message_inout->sessionId ();
  const typename DataMessageType::DATA_T& message_data_r =
      message_inout->getR ();
  typename DataMessageType::DATA_T message_data_2;
  message_data_2.format = message_data_r.format;
  DataMessageType* message_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
#else
  struct Stream_MediaFramework_V4L_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64

  //#if defined (_DEBUG)
  //  Common_File_Tools::store (ACE_TEXT_ALWAYS_CHAR ("input.rgb"),
  //                            reinterpret_cast<uint8_t*>
  //                            (message_inout->rd_ptr ()),
  //                            message_inout->length ());
  //#endif // _DEBUG

  // allocate a message buffer for the next frame
  message_p = inherited::allocateMessage (1);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                1));
    goto error;
  } // end IF
  // sanity check(s)

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited::getMediaType (message_data_r.format,
                           STREAM_MEDIATYPE_VIDEO,
                           media_type_s);
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              resolution_s.cx, resolution_s.cy,
              targetResolution_.cx, targetResolution_.cy));

  ACE_ASSERT (Stream_MediaFramework_Tools::isRGB32 (media_type_s.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW));
#else
  ACE_OS::memset (&media_type_s, 0, sizeof (struct Stream_MediaFramework_V4L_MediaType));
  inherited::getMediaType (inherited::configuration_->outputFormat,
                           STREAM_MEDIATYPE_VIDEO,
                           media_type_s);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              message_data_r.format.resolution.width, message_data_r.format.resolution.height,
              media_type_s.format.width, media_type_s.format.height));

  ACE_ASSERT (message_data_r.format.codecId == AV_CODEC_ID_NONE);
  ACE_ASSERT (Stream_Module_Decoder_Tools::isRGB32 (message_data_r.format.format));
#endif // ACE_WIN32 || ACE_WIN64

  result = MagickSetSize (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          resolution_s.cx, resolution_s.cy);
#else
                          message_data_r.format.resolution.width, message_data_r.format.resolution.height);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result == MagickTrue);
  result =
    MagickSetFormat (inherited::context_,
                     ACE_TEXT_ALWAYS_CHAR ("RGBA")); // *TODO*: make this configurable !
  ACE_ASSERT (result == MagickTrue);

  result = MagickReadImage (inherited::context_,
                            ACE_TEXT_ALWAYS_CHAR ("xc:black"));
  ACE_ASSERT (result == MagickTrue);
//  result =
//    MagickNewImage (inherited::context_,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//                    resolution_s.cx, resolution_s.cy,
//#else
//                    message_data_r.format.resolution.width, message_data_r.format.resolution.height,
//#endif // ACE_WIN32 || ACE_WIN64
//                    pixelContext_);
//  ACE_ASSERT (result == MagickTrue);

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
  result =
    MagickReadImageBlob (inherited::context_,
                         reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()),
                         message_inout->length ());
#else
  result =
    MagickImportImagePixels (inherited::context_,
                             0, 0,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             resolution_s.cx, resolution_s.cy,
#else
                             message_data_r.format.resolution.width, message_data_r.format.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                             ACE_TEXT_ALWAYS_CHAR ("RGBA"), // *TODO*: make this configurable !
                             CharPixel,
                             message_inout->rd_ptr ());
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
  if (unlikely (result != MagickTrue))
  {
     ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to MagickImportImagePixels(): \"%s\", returning\n"),
                 inherited::mod_->name (),
                 ACE_TEXT (Common_Image_Tools::errorToString (inherited::context_).c_str ())));
     goto error;
  } // end IF

  message_inout->release (); message_inout = NULL;

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
  result = MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              targetResolution_.cx, targetResolution_.cy,
#else
                              targetResolution_.width, targetResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                              CubicFilter,
                              1.0); // blur
#else
  result =
#if (MagickLibVersion >= 0x700)
    MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       targetResolution_.cx, targetResolution_.cy,
#else
                       targetResolution_.width, targetResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                       CubicFilter);
#else
    MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       targetResolution_.cx, targetResolution_.cy,
#else
                       targetResolution_.width, targetResolution_.height,
#endif // ACE_WIN32 || ACE_WIN64
                       CubicFilter,
                       1.0); // do not blur
#endif // MagickLibVersion >= 0x700
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
  ACE_ASSERT (result == MagickTrue);

  // *IMPORTANT NOTE*: "...Set the image depth to 8. If you are using the Q16
  //                   version of ImageMagick, the image depth is promoted from
  //                   8 to 16 when the image is resized. ..."
  result = MagickSetImageDepth (inherited::context_, 8);
  ACE_ASSERT (result == MagickTrue);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  ACE_ASSERT (Common_Image_Tools::stringToCodecId (MagickGetImageFormat (inherited::context_)) == AV_CODEC_ID_NONE);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (IMAGEMAGICK_IS_GRAPHICSMAGICK)
  data_p = MagickWriteImageBlob (inherited::context_,
                                 &size_i);
#else
  data_p = MagickGetImageBlob (inherited::context_,
                               &size_i);
#endif // IMAGEMAGICK_IS_GRAPHICSMAGICK
  if (unlikely (!data_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MagickGetImageBlob(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (inherited::context_).c_str ())));
    goto error;
  } // end IF
  // *TODO*: crashes in release()...(needs MagickRelinquishMemory())
  message_p->base (reinterpret_cast<char*> (data_p),
                   size_i,
                   ACE_Message_Block::DONT_DELETE); // own image data, but relinquish() in dtor
  message_p->wr_ptr (size_i);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_DirectShow_Tools::setResolution (targetResolution_,
                                                         message_data_2.format);
#else
  message_data_2.format.resolution.width = targetResolution_.width;
  message_data_2.format.resolution.height = targetResolution_.height;
#endif // ACE_WIN32 || ACE_WIN64
  message_data_2.relinquishMemory = data_p;
  message_p->initialize (message_data_2,
                         session_id,
                         NULL);

//#if defined (_DEBUG)
//  Common_File_Tools::store (ACE_TEXT_ALWAYS_CHAR ("output.rgb"),
//                            reinterpret_cast<uint8_t*> (data_p),
//                            size_i);
//#endif // _DEBUG

  // forward the converted frame
  result_2 = inherited::put_next (message_p, NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    goto error;
  } // end IF
  message_p = NULL;

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Visualization_ImageMagickResize1_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize1_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (!session_data_r.formats.empty ());
      MediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (MediaType));
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      targetResolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (inherited::configuration_->outputFormat);
#else
      targetResolution_ =
        inherited::getResolution (inherited::configuration_->outputFormat);
#endif // ACE_WIN32 || ACE_WIN64
      inherited::setResolution (targetResolution_,
                                media_type_s);
      session_data_r.formats.push_back (media_type_s);

      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}
