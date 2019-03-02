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
#include "MagickWand/MagickWand.h"
#else
#include "wand/magick_wand.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#if defined (_DEBUG)
#include "common_file_tools.h"
#endif // _DEBUG

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                     MediaType>::Stream_Decoder_ImageMagick_Decoder_T (ISTREAM_T* stream_in)
#else
                                     MediaType>::Stream_Decoder_ImageMagick_Decoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , context_ (NULL)
 , outputFormat_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ImageMagick_Decoder_T::Stream_Decoder_ImageMagick_Decoder_T"));

  MagickWandGenesis ();
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

  MagickWandTerminus ();
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

  int result = -1;

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

  MagickSetImageType (context_, TrueColorType);
  MagickSetImageColorspace (context_, sRGBColorspace);

  inherited2::getMediaType (configuration_in.outputFormat,
                            outputFormat_);
  if (unlikely (outputFormat_.format == AV_PIX_FMT_NONE))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no output format specified, using default\n"),
                inherited::mod_->name ()));
    outputFormat_.format == AV_PIX_FMT_RGB24;
  } // end IF
  ACE_ASSERT (outputFormat_.format == AV_PIX_FMT_RGB24);

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

  const typename DataMessageType::DATA_T& message_data_r =
      message_inout->getR ();
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
  inherited2::getMediaType (message_data_r,
                            media_type_s);
  size_i =
      static_cast<unsigned int> (av_image_get_buffer_size (outputFormat_.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                           media_type_s.resolution.cx,
                                                           media_type_s.resolution.cy,
#else
                                                           media_type_s.resolution.width,
                                                           media_type_s.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                                                           1));
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
  message_p = dynamic_cast<DataMessageType*> (message_block_p);
  ACE_ASSERT (message_p);
  message_p->initialize (outputFormat_,
                         message_p->sessionId (),
                         NULL);

  ACE_ASSERT (media_type_s.codec == AV_CODEC_ID_PNG);
  MagickBooleanType result = MagickSetImageFormat (context_, "PNG");
  ACE_ASSERT (result == MagickTrue);

  result = MagickReadImageBlob (context_,
                                message_inout->rd_ptr (),
                                message_inout->length ());
  ACE_ASSERT (result == MagickTrue);
  message_inout->release (); message_inout = NULL;

  result = MagickSetImageFormat (context_, "RGB");
  ACE_ASSERT (result == MagickTrue);

  data_p = MagickGetImageBlob (context_,
                               &size_2);
  ACE_ASSERT (data_p);
  ACE_ASSERT (size_i == size_2);

  message_p->base (reinterpret_cast<char*> (data_p),
                   size_2,
                   0); // 'own' data
  message_p->wr_ptr (size_2);

#if defined (_DEBUG)
  std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
  if (!Common_File_Tools::store (filename_string,
                                 data_p,
                                 size_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
                ACE_TEXT (filename_string.c_str ())));
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
#endif

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
