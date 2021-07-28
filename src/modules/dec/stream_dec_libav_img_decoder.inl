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

#ifdef __cplusplus
extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#endif /* __cplusplus */

#include "ace/Log_Msg.h"

#include "common_tools.h"

#if defined (_DEBUG)
#include "common_file_tools.h"

#include "common_image_tools.h"
#endif // _DEBUG

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_dec_libav_decoder.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

// initialize statics
#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
char
Stream_Decoder_LibAV_ImageDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
char
Stream_Decoder_LibAV_ImageDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
/*                              MediaType>::paddingBuffer[FF_INPUT_BUFFER_PADDING_SIZE];*/
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_LibAV_ImageDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Decoder_LibAV_ImageDecoder_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Decoder_LibAV_ImageDecoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , codecId_ (AV_CODEC_ID_NONE)
 , outputFormat_ ()
// , profile_ (FF_PROFILE_UNKNOWN)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_ImageDecoder_T::Stream_Decoder_LibAV_ImageDecoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&(OWN_TYPE_T::paddingBuffer),
                  0,
                  AV_INPUT_BUFFER_PADDING_SIZE);
#else
  ACE_OS::memset (&(OWN_TYPE_T::paddingBuffer),
                  0,
                  AV_INPUT_BUFFER_PADDING_SIZE);
//                  FF_INPUT_BUFFER_PADDING_SIZE);
#endif // ACE_WIN32 || ACE_WIN64
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
Stream_Decoder_LibAV_ImageDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_ImageDecoder_T::initialize"));

//  int result = -1;

  if (inherited::isInitialized_)
  {
    codecId_ = AV_CODEC_ID_NONE;

//    outputFormat_ = STREAM_DEC_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT;
//    profile_ = FF_PROFILE_UNKNOWN;
  } // end IF

#if defined (_DEBUG)
  if (configuration_in.debug)
  {
    av_log_set_callback (stream_decoder_libav_log_cb);
    // *NOTE*: this level logs all messages
    av_log_set_level (std::numeric_limits<int>::max ());
  } // end IF
#endif // _DEBUG
  av_register_all ();
//  avcodec_register_all ();

  // *TODO*: remove type inferences
  codecId_ = configuration_in.codecId;
#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: using codec \"%s\" (id: %d)\n"),
              inherited::mod_->name (),
              ACE_TEXT (avcodec_get_name (codecId_)), codecId_));
#endif // _DEBUG
  //profile_ = configuration_in.codecProfile;

  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            outputFormat_);
  if (unlikely (outputFormat_.format == AV_PIX_FMT_NONE))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: no output format specified, using codec (id: %d) native\n"),
                inherited::mod_->name (),
                codecId_));

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
Stream_Decoder_LibAV_ImageDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_ImageDecoder_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (codecId_ == AV_CODEC_ID_NONE))
    return; // nothing to do

  // initialize return value(s)
  passMessageDownstream_out = false;

//  unsigned int padding_bytes =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    AV_INPUT_BUFFER_PADDING_SIZE;
//#else
//      AV_INPUT_BUFFER_PADDING_SIZE;
////    FF_INPUT_BUFFER_PADDING_SIZE;
//#endif // ACE_WIN32 || ACE_WIN64
  DataMessageType* message_p = NULL;
  ACE_Message_Block* message_block_p = NULL;
  uint8_t* data_a[4] = { reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()), 0, 0, 0 }, * data_2[4] = { 0, 0, 0, 0 };
  unsigned int length_i = 0;

//  // step2: (re-)pad [see above] the buffer chain
//  // *IMPORTANT NOTE*: the message length does not change
//  for (message_block_2 = message_block_p;
//       message_block_2;
//       message_block_2 = message_block_2->cont ())
//  { ACE_ASSERT ((message_block_2->capacity () - message_block_2->size ()) >= padding_bytes);
//    ACE_OS::memset (message_block_2->wr_ptr (), 0, padding_bytes);
//  } // end FOR

  message_block_p = inherited::allocateMessage (1);
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                inherited::mod_->name (),
                1));
    return;
  } // end IF
  message_p = dynamic_cast<DataMessageType*> (message_block_p);
  ACE_ASSERT (message_p);

  if (unlikely (!Common_Image_Tools::load (data_a,
                                           message_inout->length (),
                                           codecId_,
                                           outputFormat_.format,
                                           outputFormat_.resolution,
                                           data_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_Image_Tools::load(), returning\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
  ACE_ASSERT (data_2[0]);
  message_inout->release (); message_inout = NULL;
  length_i =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      static_cast<unsigned int> (av_image_get_buffer_size (outputFormat_.format,
                                                           outputFormat_.resolution.cx,
                                                           outputFormat_.resolution.cy,
                                                           1));
#else
      static_cast<unsigned int> (av_image_get_buffer_size (outputFormat_.format,
                                                           outputFormat_.resolution.width,
                                                           outputFormat_.resolution.height,
                                                           1));
#endif // ACE_WIN32 || ACE_WIN64
  message_p->base (reinterpret_cast<char*> (data_2[0]),
                   length_i,
                   0); // --> delete[] the data in dtor
  message_p->wr_ptr (length_i);
  typename DataMessageType::DATA_T data_s;
  data_s.format = outputFormat_;
  message_p->initialize (data_s,
                         message_p->sessionId (),
                         NULL);

#if defined (_DEBUG)
    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
    if (!Common_File_Tools::store (filename_string,
                                   data_2[0],
                                   length_i))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
                  ACE_TEXT (filename_string.c_str ())));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
#endif // _DEBUG

  int result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
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
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_LibAV_ImageDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_ImageDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  if (unlikely (codecId_ == AV_CODEC_ID_NONE))
    return; // nothing to do

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
