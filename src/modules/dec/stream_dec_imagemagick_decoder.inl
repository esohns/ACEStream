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

#include <limits>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif /* __cplusplus */
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "MagickWand/MagickWand.h"

#include "ace/Log_Msg.h"

#if defined (_DEBUG)
#include "common_file_tools.h"
#endif // _DEBUG

#include "common_image_tools.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_ImageMagick_Decoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::Stream_Decoder_ImageMagick_Decoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , context_ (NULL)
 , outputFormat_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ImageMagick_Decoder_T::Stream_Decoder_ImageMagick_Decoder_T"));

//  InitializeMagick (NULL);
  //MagickWandGenesis ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_ImageMagick_Decoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::~Stream_Decoder_ImageMagick_Decoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ImageMagick_Decoder_T::~Stream_Decoder_ImageMagick_Decoder_T"));

  //MagickWandTerminus ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_ImageMagick_Decoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ImageMagick_Decoder_T::initialize"));

//  int result = -1;

  if (inherited::isInitialized_)
  {
    if (context_)
      DestroyMagickWand (context_);
    context_ = NULL;
  } // end IF

  context_ = NewMagickWand ();
  if (!context_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

//  MagickSetImageType (context_, TrueColorType);
//  MagickSetImageColorspace (context_, sRGBColorspace);

  inherited2::getMediaType (configuration_in.outputFormat,
                            STREAM_MEDIATYPE_VIDEO,
                            outputFormat_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited2::getMediaType (outputFormat_,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_s);
  if (unlikely (InlineIsEqualGUID (media_type_s.subtype, GUID_NULL)))
#else
#if defined (FFMPEG_SUPPORT)
  if (unlikely (outputFormat_.format == AV_PIX_FMT_NONE))
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no output format specified, using default\n"),
                inherited::mod_->name ()));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    inherited2::setFormat (MEDIASUBTYPE_RGB32,
#else
#if defined (FFMPEG_SUPPORT)
    inherited2::setFormat (AV_PIX_FMT_RGB32,
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
                           outputFormat_);
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
          typename MediaType>
void
Stream_Decoder_ImageMagick_Decoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ImageMagick_Decoder_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  DataMessageType* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  unsigned char* data_p = NULL;
  size_t size_i = 0, size_2 = 0;
  typename DataMessageType::DATA_T message_data_2;

  const typename DataMessageType::DATA_T& message_data_r =
      message_inout->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
#else
#if defined (FFMPEG_SUPPORT)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  inherited2::getMediaType (message_data_r.format,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_s);
  size_i =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Stream_MediaFramework_DirectShow_Tools::toFramesize (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
    static_cast<unsigned int> (av_image_get_buffer_size (outputFormat_.format,
                                                         media_type_s.resolution.width,
                                                         media_type_s.resolution.height,
                                                         1));
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (size_i);

  message_block_p = inherited::allocateMessage (size_i);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                inherited::mod_->name (),
                size_i));
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF
  message_p = static_cast<DataMessageType*> (message_block_p);
  ACE_ASSERT (message_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  message_data_2.format = *Stream_MediaFramework_DirectShow_Tools::copy (outputFormat_);
#else
  message_data_2.format = outputFormat_;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
#else
#if defined (FFMPEG_SUPPORT)
  ACE_ASSERT (media_type_s.codecId == AV_CODEC_ID_NONE);
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  MagickBooleanType result = MagickSetFormat (context_, ACE_TEXT_ALWAYS_CHAR ("RGB"));
  ACE_ASSERT (result == MagickTrue);
  result = MagickSetSize (context_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          resolution_s.cx, resolution_s.cy);
#else
                          media_type_s.resolution.width,
                          media_type_s.resolution.height);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result == MagickTrue);

  result =
    MagickReadImageBlob (context_,
                         reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()),
                         message_inout->length ());
  ACE_ASSERT (result == MagickTrue);
  message_inout->release (); message_inout = NULL;

  result = MagickSetImageDepth (context_, 8);
//  result = MagickSetImageChannelDepth (context_,
//                                       AllChannels,
//                                       8);
  ACE_ASSERT (result == MagickTrue);
  result = MagickSetImageFormat (context_, ACE_TEXT_ALWAYS_CHAR ("RGBA"));
  ACE_ASSERT (result == MagickTrue);
//result = MagickSetImageAlphaChannel (context_,
//                                     OpaqueAlphaChannel);
//ACE_ASSERT (result == MagickTrue);
//  result = MagickSetImageAlphaChannel (context_,
//                                       ActivateAlphaChannel);
//  ACE_ASSERT (result == MagickTrue);

  data_p = MagickGetImageBlob (context_, // was: MagickWriteImageBlob
                               &size_2);
  ACE_ASSERT (data_p);
  ACE_ASSERT (size_i <= size_2);
  // *IMPORTANT NOTE*: crashes in release()...(needs MagickRelinquishMemory())
  message_p->base (reinterpret_cast<char*> (data_p),
                   size_2,
                   ACE_Message_Block::DONT_DELETE); // own image data, but relinquish() in dtor
  message_p->wr_ptr (size_2);
  message_data_2.relinquishMemory = data_p;
  message_p->initialize (message_data_2,
                         message_p->sessionId (),
                         NULL);

//#if defined (_DEBUG)
//  std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
//  if (!Common_File_Tools::store (filename_string,
//                                 data_p,
//                                 size_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
//                ACE_TEXT (filename_string.c_str ())));
//    message_block_p->release (); message_block_p = NULL;
//    return;
//  } // end IF
//#endif // _DEBUG

  int result_2 = inherited::put_next (message_block_p, NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_ImageMagick_Decoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ImageMagick_Decoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
      break;
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}
