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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "MagickWand/MagickWand.h"
#else
#include "wand/magick_wand.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_image_tools.h"

#include "stream_macros.h"

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
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!inherited::context_))
    return; // nothing to do

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;

  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  }
  ACE_ASSERT (!message_inout->cont ());

  // sanity check(s)
  ACE_ASSERT (inherited::buffer_);
//  ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);

//  if (unlikely (!Stream_Module_Decoder_Tools::convert (inherited::context_,
//                                                       sourceResolution_.width, sourceResolution_.height, inherited::inputFormat_,
//                                                       data_a,
//                                                       inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height, inherited::inputFormat_,
//                                                       inherited::frame_->data)))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
//                inherited::mod_->name ()));
//    goto error;
//  } // end IF
  inherited::buffer_->wr_ptr (inherited::frameSize_);
  inherited::buffer_->initialize (message_inout->sessionId (),
                                  NULL);
  inherited::buffer_->set (message_inout->type ());
  message_inout->release (); message_inout = NULL;

//#if defined (_DEBUG)
//    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
//    if (!Common_File_Tools::store (filename_string,
//                                   data[0],
//                                   inherited::decodeFrameSize_))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      goto error;
//    } // end IF
//#endif

  // forward the converted frame
  result = inherited::put_next (inherited::buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  inherited::buffer_ = NULL;

  // allocate a message buffer for the next frame
  inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
  if (unlikely (!inherited::buffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                inherited::frameSize_));
    goto error;
  } // end IF

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

//      int flags_i = 0;
      MediaType media_type_s;
//      int result = -1;
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
      resolution_s.cx = inherited::configuration_->outputFormat.resolution.cx;
      resolution_s.cy = inherited::configuration_->outputFormat.resolution.cy;
#else
      resolution_s.width =
          inherited::configuration_->outputFormat.resolution.width;
      resolution_s.height =
          inherited::configuration_->outputFormat.resolution.height;
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
      // sanity check(s)
      if (!inherited::context_)
        break; // nothing to do

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

      // initialize conversion context
      ACE_ASSERT (inherited::context_);
      sws_freeContext (inherited::context_); inherited::context_ = NULL;
      flags_i =
          (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
      inherited::context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                media_type_2.resolution.cx, media_type_2.resolution.cy, inherited::inputFormat_,
#else
                                media_type_2.resolution.width, media_type_2.resolution.height, inherited::inputFormat_,
#endif // ACE_WIN32 || ACE_WIN64
                                inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height, inherited::inputFormat_,
                                flags_i,                      // flags
                                NULL, NULL,
                                0);                           // parameters
      if (unlikely (!inherited::context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // initialize frame buffer
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF
      ACE_ASSERT (inherited::frame_);
      //  frame_->format = session_data_r.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->height = media_type_2.resolution.cy;
      inherited::frame_->width = media_type_2.resolution.cx;
#else
      inherited::frame_->height = media_type_2.resolution.height;
      inherited::frame_->width = media_type_2.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::frameSize_ =
        av_image_get_buffer_size (inherited::inputFormat_,
                                  inherited::frame_->width, inherited::frame_->height,
                                  1); // *TODO*: linesize alignment

      inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
      if (!inherited::buffer_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::frameSize_));
        goto error_2;
      } // end IF
      result =
          av_image_fill_linesizes (inherited::frame_->linesize,
                                   inherited::inputFormat_,
                                   static_cast<int> (inherited::frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (inherited::frame_->data,
                                  inherited::inputFormat_,
                                  static_cast<int> (inherited::frame_->height),
                                  reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                                  inherited::frame_->linesize);
      ACE_ASSERT (result >= 0);

      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF

      if (inherited::context_)
      {
        sws_freeContext (inherited::context_); inherited::context_ = NULL;
      } // end IF

      break;
    }
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
 , pixelContext_ (NULL)
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
Stream_Visualization_ImageMagickResize1_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          MediaType>::~Stream_Visualization_ImageMagickResize1_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_ImageMagickResize1_T::~Stream_Visualization_ImageMagickResize1_T"));

  if (pixelContext_)
    DestroyPixelWand (pixelContext_);
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

  pixelContext_ = NewPixelWand ();
  ACE_ASSERT (pixelContext_);
  PixelSetColor (pixelContext_,
                 ACE_TEXT_ALWAYS_CHAR ("WHITE"));

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
#if defined (_DEBUG)
  std::string filename_string;
#endif // _DEBUG
  int result_2 = -1;
  Stream_SessionId_t session_id = message_inout->sessionId ();
  const typename DataMessageType::DATA_T& message_data_r =
      message_inout->getR ();
  typename DataMessageType::DATA_T message_data_2;
  message_data_2.format = message_data_r.format;
  DataMessageType* message_p = NULL;

  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  }
  ACE_ASSERT (!message_inout->cont ());

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
//  ACE_ASSERT (message_p->capacity () >= frameSize_);

//  message_data_p =
//      const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited::getMediaType (message_data_r.format,
                           STREAM_MEDIATYPE_VIDEO,
                           media_type_s);
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
  Common_Image_Resolution_t resolution_2 =
    Stream_MediaFramework_DirectShow_Tools::toResolution (inherited::configuration_->outputFormat);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              resolution_s.cx, resolution_s.cy,
              resolution_2.cx, resolution_2.cy));

  ACE_ASSERT (Stream_MediaFramework_Tools::isRGB32 (media_type_s.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              message_data_r.format.resolution.width, message_data_r.format.resolution.height,
              inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height));

  ACE_ASSERT (message_data_r.format.codec == AV_CODEC_ID_NONE);
  ACE_ASSERT (Stream_Module_Decoder_Tools::isRGB32 (message_data_r.format.format));
#endif // ACE_WIN32 || ACE_WIN64

  result = MagickSetSize (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          resolution_s.cx, resolution_s.cy);
#else
                          message_data_r.format.resolution.width, message_data_r.format.resolution.height);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result == MagickTrue);
  result = MagickSetFormat (inherited::context_,
                            ACE_TEXT_ALWAYS_CHAR ("RGBA"));
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

  //result =
  //  MagickReadImageBlob (inherited::context_,
  //                       reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()),
  //                       message_inout->length ());
  result =
    MagickImportImagePixels (inherited::context_,
                             0, 0,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             resolution_s.cx, resolution_s.cy,
#else
                             message_data_r.format.resolution.width, message_data_r.format.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                             ACE_TEXT_ALWAYS_CHAR ("RGBA"),
                             CharPixel,
                             message_inout->rd_ptr ());
  if (unlikely (result != MagickTrue))
  {
     ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("%s: failed to MagickImportImagePixels(): \"%s\", returning\n"),
                 inherited::mod_->name (),
                 ACE_TEXT (Common_Image_Tools::errorToString (inherited::context_).c_str ())));
     goto error;
  } // end IF

  message_inout->release (); message_inout = NULL;

  result = MagickSetImageFormat (inherited::context_,
                                 ACE_TEXT_ALWAYS_CHAR ("RGBA"));
  ACE_ASSERT (result == MagickTrue);

  result =
//#if MAGICKWAND_CHECK_VERSION(7,0,0)
//    MagickResizeImage (inherited::context_,
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//                       resolution_2.cx, resolution_2.cy,
//#else
//                       inherited::configuration_->outputFormat.resolution.width,
//                       inherited::configuration_->outputFormat.resolution.height,
//#endif // ACE_WIN32 || ACE_WIN64
//                       LanczosFilter);
//#else
    MagickResizeImage (inherited::context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       resolution_2.cx, resolution_2.cy,
#else
                       inherited::configuration_->outputFormat.resolution.width,
                       inherited::configuration_->outputFormat.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                       LanczosFilter,
                       1.0); // do not blur
//#endif // MAGICKWAND_CHECK_VERSION(7,0,0)
  ACE_ASSERT (result == MagickTrue);

//  // Set the compression quality to 95 (high quality = low compression)
//  result = MagickSetImageCompressionQuality (context_,100);
//  ACE_ASSERT (result == MagickTrue);
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
  Stream_MediaFramework_DirectShow_Tools::setResolution (resolution_2,
                                                         message_data_2.format);
#else
  message_data_2.format.resolution =
    inherited::configuration_->outputFormat.resolution;
#endif // ACE_WIN32 || ACE_WIN64
  //message_data_2.format.resolution.cx = -message_data_2.format.resolution.cx;
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
      Common_Image_Resolution_t resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (inherited::configuration_->outputFormat);
      inherited::setResolution (resolution_s,
#else
      inherited::setResolution (inherited::configuration_->outputFormat.resolution,
#endif // ACE_WIN32 || ACE_WIN64
                                media_type_s);
      //media_type_s.resolution.cx = -media_type_s.resolution.cx;
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
