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

#ifdef __cplusplus
extern "C"
{
#include <libavutil/imgutils.h>
}
#endif /* __cplusplus */

#include <ace/Log_Msg.h>

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::Stream_Decoder_LibAVDecoder_T ()
 : inherited ()
 , allocator_ (NULL)
 , codecContext_ (NULL)
 , codecFormat_ (AV_PIX_FMT_NONE)
 , codecId_ (AV_CODEC_ID_NONE)
 , codecProfile_ (FF_PROFILE_UNKNOWN)
 //, formatContext_ (NULL)
 , currentFrame_ (NULL)
 , decodeContext_ (NULL)
 , decodeFormat_ (AV_PIX_FMT_NONE)
 , decodeHeight_ (0)
 , decodeWidth_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::Stream_Decoder_LibAVDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::~Stream_Decoder_LibAVDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::~Stream_Decoder_LibAVDecoder_T"));

  int result = -1;

  if (codecContext_)
  {
    result = avcodec_close (codecContext_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
    avcodec_free_context (&codecContext_);
  } // end IF
  //if (formatContext_)
  //{
  //  if (formatContext_->streams)
  //    if (formatContext_->streams[0]->codec)
  //    {
  //      result = avcodec_close (formatContext_->streams[0]->codec);
  //      if (result == -1)
  //        ACE_DEBUG ((LM_ERROR,
  //                    ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
  //                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
  //    } // end IF

  //  avformat_free_context (formatContext_);
  //} // end IF

  if (currentFrame_)
    av_frame_free (&currentFrame_);

  if (decodeContext_)
    sws_freeContext (decodeContext_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::initialize"));

  int result = -1;

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.format);
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    allocator_ = NULL;
    if (codecContext_)
    {
      result = avcodec_close (codecContext_);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      avcodec_free_context (&codecContext_);
    } // end IF
    codecFormat_ = AV_PIX_FMT_NONE;
    codecId_ = AV_CODEC_ID_NONE;
    codecProfile_ = FF_PROFILE_UNKNOWN;
  //  if (formatContext_)
  //  {
  //    if (formatContext_->streams)
  //      if (formatContext_->streams[0]->codec)
  //      {
  //        result = avcodec_close (formatContext_->streams[0]->codec);
  //        if (result == -1)
  //          ACE_DEBUG ((LM_ERROR,
  //                      ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
  //                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
  //      } // end IF

  //    avformat_free_context (formatContext_);
  //    formatContext_ = NULL;
  //  } // end IF
    if (currentFrame_)
    {
      av_frame_free (&currentFrame_);
      currentFrame_ = NULL;
    } // end IF

    if (decodeContext_)
    {
      sws_freeContext (decodeContext_);
      decodeContext_ = NULL;
    } // end IF
    decodeFormat_ = AV_PIX_FMT_NONE;
    decodeHeight_ = 0;
    decodeWidth_ = 0;
  } // end IF

  allocator_ = allocator_in;
  // *TODO*: remove type inferences
  codecId_ = configuration_in.codecId;
  //codecProfile_ = configuration_in.codecProfile;
  codecFormat_ = configuration_in.codecFormat;

#if defined (_DEBUG)
  //av_log_set_callback (Stream_Decoder_LibAVDecoder_LoggingCB);
  // *NOTE*: this level logs all messages
  //av_log_set_level (std::numeric_limits<int>::max ());
#endif

  av_register_all ();
//  avcodec_register_all ();

  currentFrame_ = av_frame_alloc ();
  if (!currentFrame_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  int width, height;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;

  if (configuration_in.format->formattype == FORMAT_VideoInfo)
  { ACE_ASSERT (configuration_in.format->pbFormat);
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (configuration_in.format->pbFormat);
    ACE_ASSERT (video_info_header_p);

    height = static_cast<int> (video_info_header_p->bmiHeader.biHeight);
    width = static_cast<int> (video_info_header_p->bmiHeader.biWidth);
  } // end IF
  else if (configuration_in.format->formattype == FORMAT_VideoInfo2)
  { ACE_ASSERT (configuration_in.format->pbFormat);
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (configuration_in.format->pbFormat);
    ACE_ASSERT (video_info_header2_p);

    height = static_cast<int> (video_info_header2_p->bmiHeader.biHeight);
    width = static_cast<int> (video_info_header2_p->bmiHeader.biWidth);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (configuration_in.format->formattype).c_str ())));
    return false;
  } // end ELSE

  decodeFormat_ =
    Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (configuration_in.format->subtype);
  if (decodeFormat_ == AV_PIX_FMT_NONE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (inherited::configuration_->format->subtype, false).c_str ())));
    return false;
  } // end IF
#else
  ACE_ASSERT (configuration_in.pixelBuffer);

  height = configuration_in.height;
  width = configuration_in.width;
  decodeFormat_ = configuration_in.format;
  decodeHeight_ =
      static_cast<unsigned int> (gdk_pixbuf_get_height (configuration_in.pixelBuffer));
  decodeWidth_ =
      static_cast<unsigned int> (gdk_pixbuf_get_width (configuration_in.pixelBuffer));

  if ((decodeHeight_ != height) || (decodeWidth_ != width))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: scaling decompressed frame(s) (size: %ux%u) to %ux%u...\n"),
                inherited::mod_->name (),
                width, height,
                decodeWidth_, decodeHeight_));
#endif

  //decodeContext_ = sws_alloc_context ();
  // *IMPORTANT NOTE*: the DirectShow media type MEDIASUBTYPE_RGB24 actually
  //                   expects BGR24
  decodeContext_ =
    sws_getCachedContext (NULL,
                          width, height, codecFormat_,
                          decodeWidth_, decodeHeight_, decodeFormat_,
                          0,                                 // flags
                          NULL, NULL,
                          0);                                // parameters
  if (!decodeContext_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: sws_getCachedContext() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
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
          typename SessionDataContainerType>
void
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::handleDataMessage"));

//  unsigned int length = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  static char padding_buffer[AV_INPUT_BUFFER_PADDING_SIZE];
  ACE_OS::memset (padding_buffer, 0, AV_INPUT_BUFFER_PADDING_SIZE);
#else
  static char padding_buffer[FF_INPUT_BUFFER_PADDING_SIZE];
  ACE_OS::memset (padding_buffer, 0, FF_INPUT_BUFFER_PADDING_SIZE);
#endif
//  unsigned int free_space = 0;
  int result = -1;
  int got_picture = 0;
  //struct AVBufferRef* buffer_p = NULL;
  //int flags = AV_BUFFER_FLAG_READONLY;
  struct AVPacket packet_s;
  int line_size = 0;
  unsigned int frame_size = 0;
  ACE_Message_Block* message_block_p, *message_block_2 = NULL;
  uint8_t* out_data[4] = { NULL, NULL, NULL, NULL };
  int out_linesize[4] = { 0, 0, 0, 0 };

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (codecContext_);

  message_block_p = message_inout;
  //length = message_inout->length ();
  //free_space = message_inout->space ();
  //result =
  //  message_inout->copy (padding_buffer,
  //                       (free_space < AV_INPUT_BUFFER_PADDING_SIZE ? free_space
  //                                                                  : AV_INPUT_BUFFER_PADDING_SIZE));
  //if (result == -1)
  //{
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
  //              inherited::mod_->name ()));

  //  // clean up
  //  message_inout->release ();
  //  message_inout = NULL;

  //  return;
  //} // end IF
  //message_inout->wr_ptr (message_inout->rd_ptr () + length);

  // *TODO*: find a way to add all available chunks to the AVPacket at once
  //buffer_p =
  //  av_buffer_create (reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
  //                    message_inout->length (),
  //                    Stream_Decoder_LibAVDecoder_NOPFree,
  //                    NULL,
  //                    flags);
  //if (!buffer_p)
  //{
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s: failed to av_buffer_create(): \"%m\", returning\n"),
  //              inherited::mod_->name ()));

  //  // clean up
  //  message_inout->release ();
  //  message_inout = NULL;

  //  return;
  //} // end IF
  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));

    // clean up
    message_inout->release ();
    message_inout = NULL;

    return;
  }

  do
  {
    av_init_packet (&packet_s);
    //packet_s.buf = buffer_p;
    packet_s.data = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());
    //packet_s.pts = AV_NOPTS_VALUE;
    //packet_s.dts = AV_NOPTS_VALUE;
    packet_s.size = message_block_p->length ();
    //packet_s.stream_index = 0;
    //packet_s.flags = 0;
    //packet_s.side_data = NULL;
    //packet_s.side_data_elems = 0;
    //packet_s.duration = 0;
    //packet_s.pos = 0;
    //packet_s.convergence_duration = 0;

    result = avcodec_decode_video2 (codecContext_,
                                    currentFrame_,
                                    &got_picture,
                                    &packet_s);
    //result = avcodec_send_packet (codecContext_, &packet_s);
    if (result < 0)
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: failed to avcodec_send_packet(): \"%s\", returning\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));

      // clean up
      message_inout->release ();
      message_inout = NULL;

      return;
    } // end IF
    ACE_ASSERT (result == packet_s.size);
    if (!got_picture)
    {
      message_block_p = message_block_p->cont ();
      if (!message_block_p)
        break;
      continue;
    } // end IF

    // *TODO*: avoid the memcpy, forward the frame as such in the message data
    //frame_size = ((currentFrame_->width * currentFrame_->height) +
    //              (2 * ((currentFrame_->width * currentFrame_->height) >> 2)));
    frame_size = av_image_get_buffer_size (decodeFormat_,
                                           decodeWidth_,
                                           decodeHeight_,
                                           1); // *TODO*: linesize alignment
    message_block_2 = allocateMessage (frame_size);
    if (!message_block_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to allocateMessage(%u), returning\n"),
                  inherited::mod_->name (),
                  frame_size));

      // clean up
      av_frame_unref (currentFrame_);

      return;
    } // end IF

    // decode the frame ?
    if ((codecContext_->pix_fmt == decodeFormat_) &&
        (codecContext_->height == decodeHeight_)  &&
        (codecContext_->width == decodeWidth_))
    {
//      line_size = av_image_get_linesize (decodeFormat_,
//                                         decodeWidth_,
//                                         1);
      ACE_ASSERT ((codecContext_->height * currentFrame_->linesize[0]) == frame_size);
      result =
          message_block_2->copy (reinterpret_cast<char*> (currentFrame_->data[0]),
                                 frame_size);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", returning\n"),
                    inherited::mod_->name ()));

        // clean up
        av_frame_unref (currentFrame_);
        message_block_2->release ();

        return;
      } // end IF

      goto continue_;
    } // end IF

    out_data[0] = reinterpret_cast<uint8_t*> (message_block_2->wr_ptr ());
    result = av_image_fill_linesizes (out_linesize,
                                      decodeFormat_,
                                      static_cast<int> (decodeWidth_));
    result =
      sws_scale (decodeContext_,
                 currentFrame_->data, currentFrame_->linesize,
                 0, currentFrame_->height,
                 out_data, out_linesize);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sws_scale(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));

      // clean up
      av_frame_unref (currentFrame_);
      message_block_2->release ();

      return;
    } // end IF
    av_frame_unref (currentFrame_);
    message_block_2->wr_ptr (frame_size);
    ACE_ASSERT (message_block_2->length () == frame_size);

continue_:
    // forward the next frame
    result = inherited::put_next (message_block_2, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

      // clean up
      message_block_2->release ();

      return;
    } // end IF

    message_block_p = message_block_p->cont ();
    if (!message_block_p)
      break;
  } while (true);
  message_inout->release ();
  message_inout = NULL;
//decompress:
//  result = avcodec_receive_frame (codecContext_, currentFrame_);
//  if (result < 0)
//  {
//    if (result != AVERROR (EAGAIN))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to avcodec_receive_frame(): \"%s\", returning\n"),
//                  inherited::mod_->name (),
//                  ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//    return;
//  } // end IF
  //goto decompress;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  const SessionDataContainerType& session_data_container_r =
    message_inout->get ();
  typename SessionDataContainerType::DATA_T& session_data_r =
    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      int result = -1;
      struct AVCodec* codec_p = NULL;
      int flags, flags2;

      ACE_ASSERT (!codecContext_);

      codec_p = avcodec_find_decoder (codecId_);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_find_decoder(%d) failed: \"%m\", aborting\n"),
                    codecId_));
        goto error;
      } // end IF
      codecContext_ = avcodec_alloc_context3 (codec_p);
      if (!codecContext_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_alloc_context3() failed: \"%m\", aborting\n")));
        goto error;
      } // end IF
      ACE_ASSERT (codecContext_);
      //result = avcodec_get_context_defaults3 (codecContext_,
      //                                        codec_p);
      //if (result < 0)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("avcodec_get_context_defaults3() failed: \"%s\", aborting\n"),
      //              ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      //  goto error;
      //} // end IF

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
      DWORD bit_rate = 0;
      LONG width, height;

      if (inherited::configuration_->format->formattype == FORMAT_VideoInfo)
      { ACE_ASSERT (inherited::configuration_->format->pbFormat);
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (inherited::configuration_->format->pbFormat);
        ACE_ASSERT (video_info_header_p);
        bit_rate = video_info_header_p->dwBitRate;
        width = video_info_header_p->bmiHeader.biWidth;
        height = video_info_header_p->bmiHeader.biHeight;
      } // end IF
      else if (inherited::configuration_->format->formattype == FORMAT_VideoInfo2)
      { ACE_ASSERT (inherited::configuration_->format->pbFormat);
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (inherited::configuration_->format->pbFormat);
        ACE_ASSERT (video_info_header2_p);
        bit_rate = video_info_header2_p->dwBitRate;
        width = video_info_header2_p->bmiHeader.biWidth;
        height = video_info_header2_p->bmiHeader.biHeight;
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (inherited::configuration_->format->formattype).c_str ())));
        goto error;
      } // end ELSE
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct AVCodecParameters* codec_parameters_p =
        avcodec_parameters_alloc ();
      if (!codec_parameters_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_parameters_alloc(): \"%m\", aborting\n")));
        goto error;
      } // end IF
      codec_parameters_p->codec_type = AVMEDIA_TYPE_VIDEO;
      codec_parameters_p->codec_id = codecId_;
      //codec_parameters_p->codec_tag = ;
      //codec_parameters_p->extradata = NULL;
      //codec_parameters_p->extradata_size = 0;
      //codec_parameters_p->format = AV_PIX_FMT_YUV420P;
      //codec_parameters_p->bit_rate = bit_rate;
      //codec_parameters_p->bits_per_coded_sample = 0;
      //codec_parameters_p->bits_per_raw_sample = 0;
      //codec_parameters_p->profile = FF_PROFILE_H264_HIGH;
      //codec_parameters_p->level = ;
      //codec_parameters_p->width = width;
      //codec_parameters_p->height = height;
      //codec_parameters_p->sample_aspect_ratio.num = 1;
      //codec_parameters_p->sample_aspect_ratio.den = 1;
      //codec_parameters_p->field_order = AV_FIELD_UNKNOWN;
      //codec_parameters_p->color_range = ;
      //codec_parameters_p->color_primaries = ;
      //codec_parameters_p->color_trc = ;
      //codec_parameters_p->color_space = ;
      //codec_parameters_p->chroma_location = ;
      //codec_parameters_p->video_delay = 0;

      flags = AV_CODEC_FLAG_UNALIGNED      |
              AV_CODEC_FLAG_QSCALE         |
              AV_CODEC_FLAG_OUTPUT_CORRUPT |
              AV_CODEC_FLAG_QPEL           |
              //AV_CODEC_FLAG_PASS1          |
              //AV_CODEC_FLAG_PASS2          |
              AV_CODEC_FLAG_LOOP_FILTER    |
              //AV_CODEC_FLAG_GRAY           |
              AV_CODEC_FLAG_TRUNCATED      |
              //AV_CODEC_FLAG_INTERLACED_DCT |
              AV_CODEC_FLAG_LOW_DELAY      |
              //AV_CODEC_FLAG_GLOBAL_HEADER  |
              AV_CODEC_FLAG_BITEXACT;//       |
      //AV_CODEC_FLAG_INTERLACED_ME  |
      //AV_CODEC_FLAG_CLOSED_GOP;

      flags2 = AV_CODEC_FLAG2_FAST          |
               AV_CODEC_FLAG2_CHUNKS        |
               AV_CODEC_FLAG2_IGNORE_CROP   |
               AV_CODEC_FLAG2_SHOW_ALL      |
               AV_CODEC_FLAG2_EXPORT_MVS    |
               AV_CODEC_FLAG2_SKIP_MANUAL;
#else
      flags = CODEC_FLAG_UNALIGNED      |
              CODEC_FLAG_QSCALE         |
              CODEC_FLAG_OUTPUT_CORRUPT |
              CODEC_FLAG_QPEL           |
              //CODEC_FLAG_PASS1          |
              //CODEC_FLAG_PASS2          |
              CODEC_FLAG_LOOP_FILTER    |
              //CODEC_FLAG_GRAY           |
              CODEC_FLAG_TRUNCATED      |
              //CODEC_FLAG_INTERLACED_DCT |
              CODEC_FLAG_LOW_DELAY      |
              //CODEC_FLAG_GLOBAL_HEADER  |
              CODEC_FLAG_BITEXACT;//       |
      //CODEC_FLAG_INTERLACED_ME  |
      //CODEC_FLAG_CLOSED_GOP;

      flags2 = CODEC_FLAG2_FAST          |
               CODEC_FLAG2_CHUNKS        |
               CODEC_FLAG2_IGNORE_CROP   |
               CODEC_FLAG2_SHOW_ALL      |
               CODEC_FLAG2_EXPORT_MVS    |
               CODEC_FLAG2_SKIP_MANUAL;
#endif

      codecContext_->opaque = &codecFormat_;
      //codecContext_->bit_rate = bit_rate;
      codecContext_->flags = flags;
      codecContext_->flags2 = flags2;
      //codecContext_->extradata = NULL;
      //codecContext_->extradata_size = 0;
      //codecContext_->time_base.num = 0;
      //codecContext_->time_base.den = 1;
      codecContext_->ticks_per_frame = 2;
      //codecContext_->delay = 0;
      //codecContext_->width = width;
      //codecContext_->height = height;
      //codecContext_->coded_width = width;
      //codecContext_->coded_height = height;
      //codecContext_->pix_fmt = AV_PIX_FMT_NONE;
      //codecContext_->draw_horiz_band = NULL;
      codecContext_->get_format = Stream_Decoder_LibAVDecoder_GetFormat;
      codecContext_->slice_count = 0;
      codecContext_->slice_offset = NULL;
      codecContext_->slice_flags = 0;
      codecContext_->skip_top = 0;
      codecContext_->skip_bottom = 0;
      codecContext_->field_order = AV_FIELD_UNKNOWN;
      //codecContext_->get_buffer2 = NULL;
      codecContext_->refcounted_frames = 0;
      //codecContext_->rc_max_rate = bit_rate;
      codecContext_->workaround_bugs = 0;
      codecContext_->strict_std_compliance = FF_COMPLIANCE_NORMAL;
      codecContext_->error_concealment = 0;
      codecContext_->debug = 0;
      codecContext_->debug_mv = 0;
      codecContext_->err_recognition = 0;
      codecContext_->reordered_opaque = 0;
      codecContext_->hwaccel_context = NULL;
      codecContext_->idct_algo = FF_IDCT_AUTO;
      codecContext_->bits_per_coded_sample = 12;
      codecContext_->lowres = 0;
      codecContext_->thread_count = Common_Tools::getNumberOfCPUs (false);
      codecContext_->thread_type = FF_THREAD_SLICE;
      //codecContext_->thread_safe_callbacks = 1;
      //codecContext_->execute = NULL;
      //codecContext_->execute2 = NULL;
      codecContext_->skip_loop_filter = AVDISCARD_NONE;
      codecContext_->skip_idct = AVDISCARD_NONE;
      codecContext_->skip_frame = AVDISCARD_NONE;
      codecContext_->pkt_timebase.num = 0;
      codecContext_->pkt_timebase.den = 1;
      //codecContext_->sub_charenc = NULL;
      codecContext_->skip_alpha = 0;
      codecContext_->debug_mv = 0;
      //codecContext_->dump_separator = ',';
      //codecContext_->hw_frames_ctx = NULL;
      //codecContext_->max_pixels = 0;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = avcodec_parameters_to_context (codecContext_,
                                              codec_parameters_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_parameters_to_context() failed: \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      avcodec_parameters_free (&codec_parameters_p);
      codec_parameters_p = NULL;
#endif
      result = avcodec_open2 (codecContext_,
                              codec_p,
                              NULL);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_open2(%d) failed: \"%s\", aborting\n"),
                    codecId_,
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (currentFrame_);
      currentFrame_->format = codecContext_->pix_fmt;
      currentFrame_->width = codecContext_->width;
      currentFrame_->height = codecContext_->height;

      goto continue_;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      // clean up
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (codec_parameters_p)
        avcodec_parameters_free (&codec_parameters_p);
#endif

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (codecContext_)
      {
        avcodec_free_context (&codecContext_);
        codecContext_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
DataMessageType*
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  if (allocator_)
  {
allocate:
    try {
      message_p =
        static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p && !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}
