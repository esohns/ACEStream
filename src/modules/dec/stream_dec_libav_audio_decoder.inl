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

#include "common_tools.h"
#if defined (_DEBUG)
#include "common_file_tools.h"
#endif // _DEBUG

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_lib_ffmpeg_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::Stream_Decoder_LibAVAudioDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , codecId_ (AV_CODEC_ID_NONE)
 , context_ (NULL)
 , format_ (AV_SAMPLE_FMT_NONE)
 , sampleRate_ (0)
 , frame_ (NULL)
 , frameSize_ (0)
 , outputFormat_ (AV_SAMPLE_FMT_NONE)
 , outputFrameSize_ (0)
 , outputSampleRate_ (0)
 , outputChannels_ (0)
 , parserContext_ (NULL)
 , parserPosition_ (0)
 , profile_ (FF_PROFILE_UNKNOWN)
 , transformContext_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::Stream_Decoder_LibAVAudioDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::~Stream_Decoder_LibAVAudioDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::~Stream_Decoder_LibAVAudioDecoder_T"));

  int result = -1;

  if (frame_)
    av_frame_free (&frame_);

  if (context_)
    avcodec_free_context (&context_);

  if (parserContext_)
    av_parser_close (parserContext_);

  if (transformContext_)
    swr_free (&transformContext_);
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
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    codecId_ = AV_CODEC_ID_NONE;
    if (context_)
      avcodec_free_context (&context_);
    format_ = AV_SAMPLE_FMT_NONE;
    sampleRate_ = 0;
    if (frame_)
      av_frame_free (&frame_);
    frameSize_ = 0;
    outputFormat_ = STREAM_DEC_DEFAULT_LIBAV_OUTPUT_SAMPLE_FORMAT;
    outputFrameSize_ = 0;
    outputSampleRate_ = 0;
    outputChannels_ = 0;
    if (parserContext_)
    {
      av_parser_close (parserContext_); parserContext_ = NULL;
    } // end IF
    parserPosition_ = 0;
    profile_ = FF_PROFILE_UNKNOWN;
    if (transformContext_)
    {
      swr_free (&transformContext_); transformContext_ = NULL;
    } // end IF
  } // end IF

#if defined (_DEBUG)
  if (configuration_in.debug)
  {
    av_log_set_callback (stream_decoder_libav_log_cb);
    // *NOTE*: this level logs all messages
    av_log_set_level (std::numeric_limits<int>::max ());
  } // end IF
#endif // _DEBUG
//  av_register_all ();
//  avcodec_register_all ();

  // *TODO*: remove type inferences
  codecId_ = configuration_in.codecId;
//  if (unlikely (codecId_ == AV_CODEC_ID_NONE))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: invalid codec, aborting\n"),
//                inherited::mod_->name ()));
//    return false;
//  } // end IF

  struct Stream_MediaFramework_FFMPEG_AudioMediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            STREAM_MEDIATYPE_AUDIO,
                            media_type_s);
  outputFormat_ = media_type_s.format;
  if (unlikely (outputFormat_ == AV_SAMPLE_FMT_NONE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid output format, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  outputFrameSize_ =
    av_get_bytes_per_sample (outputFormat_) * media_type_s.channels;
  outputSampleRate_ = media_type_s.sampleRate;
  outputChannels_ = media_type_s.channels;

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
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (context_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  DataMessageType* message_p = NULL;
  struct AVPacket packet_s;
  ACE_Message_Block* message_block_p = NULL;
  uint8_t* data_p = NULL;
  size_t data_size_i = 0;

  // *NOTE*: ffmpeg processes data in 'chunks' and supports/requires memory
  //         alignment, as well as 'padding' bytes.
  //         Note that as the data may arrive in fragmented pieces (e.g. over a
  //         network), the required preprocessing overhead may defeat the whole
  //         benefit of these features.
  // *IMPORTANT NOTE*: defragment()ing the incoming buffers may not be allowed,
  //                   as that memory may (!) belong to previous frame(s) !
  // *IMPORTANT NOTE*: padding the data beyond wr_ptr() may not be allowed, as
  //                   that memory may (!) belong to the next frame(s) !

  // step2: (re-)pad [see above] the buffer chain
  // *IMPORTANT NOTE*: the message length does not change
  for (message_block_p = message_inout;
       message_block_p;
       message_block_p = message_block_p->cont ())
  { if (((message_block_p->capacity () - message_block_p->size ()) >= AV_INPUT_BUFFER_PADDING_SIZE))
      ACE_OS::memset (message_block_p->wr_ptr (), 0, AV_INPUT_BUFFER_PADDING_SIZE);
  } // end FOR

  message_block_p = message_inout;
  do
  {
    /* use the parser to split the data into frames */
    data_p = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());
    data_size_i = message_block_p->length ();
    while (data_size_i > 0)
    {
      av_init_packet (&packet_s);

      if (likely (parserContext_))
      {
        result =
          av_parser_parse2 (parserContext_,
                            context_,
                            &packet_s.data,
                            &packet_s.size,
                            data_p,
                            static_cast<int> (data_size_i),
                            AV_NOPTS_VALUE,
                            AV_NOPTS_VALUE,
                            parserPosition_);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to av_parser_parse2(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
          goto error;
        } // end IF
        parserPosition_ += result;
        data_p      += result;
        data_size_i -= result;
      } // end IF
      else
      {
        packet_s.data = data_p;
        packet_s.size = static_cast<int> (data_size_i);
        data_size_i = 0;
      } // end ELSE
      if (!packet_s.size)
        continue;

      ACE_ASSERT (!message_p);
      if (!decodePacket (packet_s,
                         message_p))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: failed to Stream_Decoder_LibAVAudioDecoder_T::decodePacket(), continuing\n"),
                    inherited::mod_->name ()));
        continue;
      } // end IF
      if (!message_p)
        continue;

      // forward the decoded frame
      message_p->initialize (message_inout->sessionId (),
                             NULL);
      message_p->set (message_inout->type ());
      result = inherited::put_next (message_p, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      message_p = NULL;
    } // end WHILE

    message_block_p = message_block_p->cont ();
    if (!message_block_p)
      break;
  } while (true);
  message_inout->release (); message_inout = NULL;

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF
  if (message_p)
    message_p->release ();

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
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    { ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());

      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct Stream_MediaFramework_FFMPEG_AudioMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      MediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_2);

      if (codecId_ == AV_CODEC_ID_NONE)
      {
        if (media_type_s.codec == AV_CODEC_ID_NONE)
          break;
        codecId_ = media_type_s.codec;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: using codec \"%s\" (id: %d)\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (avcodec_get_name (codecId_)), codecId_));
      //profile_ = configuration_in.codecProfile;

      int result = -1;
      const struct AVCodec* codec_p = NULL;
      struct AVCodecParameters* codec_parameters_p = NULL;
      struct AVDictionary* dictionary_p = NULL;
      //int flags, flags2;
      //int debug_i = FF_DEBUG_PICT_INFO | FF_DEBUG_RC | FF_DEBUG_BITSTREAM |
      //              FF_DEBUG_MB_TYPE | FF_DEBUG_QP;
      int debug_i = FF_DEBUG_PICT_INFO | FF_DEBUG_BUGS;

      codec_p = avcodec_find_decoder (codecId_);
      if (unlikely (!codec_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_decoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    codecId_));
        goto error;
      } // end IF

      ACE_ASSERT (!parserContext_);
      parserContext_ = av_parser_init (codecId_);
      if (!parserContext_)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: av_parser_init(\"%s\"[%d]) failed: \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (avcodec_get_name (codecId_)), codecId_));
      } // end IF
      else
      {
        parserContext_->flags |= PARSER_FLAG_FETCHED_OFFSET;
        parserContext_->flags |= PARSER_FLAG_USE_CODEC_TS;
      } // end ELSE

      context_ = avcodec_alloc_context3 (codec_p);
      if (unlikely (!context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (context_);

      //codec_parameters_p = avcodec_parameters_alloc ();
      //if (unlikely (!codec_parameters_p))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to avcodec_parameters_alloc(): \"%m\", aborting\n"),
      //              inherited::mod_->name ()));
      //  goto error;
      //} // end IF
      //codec_parameters_p->codec_type = AVMEDIA_TYPE_AUDIO;
      //codec_parameters_p->codec_id = codecId_;
      ////codec_parameters_p->codec_tag = ;
      ////codec_parameters_p->extradata = NULL;
      ////codec_parameters_p->extradata_size = 0;
      //codec_parameters_p->format = outputFormat_;
      ////codec_parameters_p->bit_rate = 200000;
      //codec_parameters_p->bits_per_coded_sample =
      //  av_get_bytes_per_sample (outputFormat_) * 8;
      //codec_parameters_p->bits_per_raw_sample =
      //  av_get_bytes_per_sample (outputFormat_) * 8;
      //// codec_parameters_p->profile = FF_PROFILE_H264_HIGH;
      ////codec_parameters_p->level = ;
      ////codec_parameters_p->width = 0;
      ////codec_parameters_p->height = 0;
      ////codec_parameters_p->sample_aspect_ratio.num = 1;
      ////codec_parameters_p->sample_aspect_ratio.den = 1;
      ////codec_parameters_p->field_order = AV_FIELD_UNKNOWN;
      ////codec_parameters_p->color_range = ;
      ////codec_parameters_p->color_primaries = ;
      ////codec_parameters_p->color_trc = ;
      ////codec_parameters_p->color_space = ;
      ////codec_parameters_p->chroma_location = ;
      ////codec_parameters_p->video_delay = 0;
      //codec_parameters_p->channel_layout =
      //    Stream_Module_Decoder_Tools::channelsToLayout (outputChannels_);
      //codec_parameters_p->channels = outputChannels_;
      //codec_parameters_p->sample_rate = outputSampleRate_;
      //codec_parameters_p->block_align =
      //  av_get_bytes_per_sample (outputFormat_) * outputChannels_;
      //codec_parameters_p->frame_size =
      //  av_get_bytes_per_sample (outputFormat_) * outputChannels_;
      //codec_parameters_p->initial_padding = 0;
      //codec_parameters_p->trailing_padding = 0;
      //codec_parameters_p->seek_preroll = 0;

      //flags = AV_CODEC_FLAG_UNALIGNED      |
      //        //AV_CODEC_FLAG_QSCALE         |
      ////        AV_CODEC_FLAG_4MV            |
      //        AV_CODEC_FLAG_OUTPUT_CORRUPT |
      //        //AV_CODEC_FLAG_QPEL           |
      //        //AV_CODEC_FLAG_DROPCHANGED          |
      //        //AV_CODEC_FLAG_PASS1          |
      //        //AV_CODEC_FLAG_PASS2          |
      //        AV_CODEC_FLAG_LOOP_FILTER    |
      //        //AV_CODEC_FLAG_GRAY           |
      //        //AV_CODEC_FLAG_PSNR           |
      //        AV_CODEC_FLAG_TRUNCATED      |
      //        //AV_CODEC_FLAG_INTERLACED_DCT |
      //        AV_CODEC_FLAG_LOW_DELAY      |
      //        //AV_CODEC_FLAG_GLOBAL_HEADER  |
      //        AV_CODEC_FLAG_BITEXACT;//       |
      //        //AV_CODEC_FLAG_AC_PRED        |
      ////AV_CODEC_FLAG_INTERLACED_ME  |
      ////AV_CODEC_FLAG_CLOSED_GOP;

      //flags2 = AV_CODEC_FLAG2_FAST          |
      ////         AV_CODEC_FLAG2_NO_OUTPUT           |
      ////         AV_CODEC_FLAG2_LOCAL_HEADER        |
      ////         AV_CODEC_FLAG2_DROP_FRAME_TIMECODE |
      //         AV_CODEC_FLAG2_CHUNKS        |
      //         //AV_CODEC_FLAG2_IGNORE_CROP   |
      //         AV_CODEC_FLAG2_SHOW_ALL      |
      //         //AV_CODEC_FLAG2_EXPORT_MVS    |
      //         AV_CODEC_FLAG2_SKIP_MANUAL;
      //// AV_CODEC_FLAG2_RO_FLUSH_NOOP

      //result = avcodec_parameters_to_context (context_,
      //                                        codec_parameters_p);
      //if (unlikely (result < 0))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: avcodec_parameters_to_context() failed: \"%s\", aborting\n"),
      //              inherited::mod_->name (),
      //              ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      //  goto error;
      //} // end IF
      //avcodec_parameters_free (&codec_parameters_p); codec_parameters_p = NULL;
#if defined (_DEBUG)
      context_->debug = (inherited::configuration_->debug ? debug_i : 0);
#endif // _DEBUG
      context_->err_recognition = 0;

//      result = av_dict_set (&dictionary_p,
//                            NULL, NULL,
//                            0);
      result = av_dict_set (&dictionary_p,
                            ACE_TEXT_ALWAYS_CHAR ("foo"), ACE_TEXT_ALWAYS_CHAR ("bar"),
                            0);
      ACE_ASSERT (result >= 0);
      ACE_ASSERT (dictionary_p);
      result = avcodec_open2 (context_,
                              context_->codec,
                              &dictionary_p);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2(%d) failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    codecId_,
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        av_dict_free (&dictionary_p); dictionary_p = NULL;
        goto error;
      } // end IF
      av_dict_free (&dictionary_p); dictionary_p = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: initialized codec %s; decoded sample format: %s\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (avcodec_get_name (codecId_)),
                  ACE_TEXT (Stream_MediaFramework_Tools::sampleFormatToString (context_->sample_fmt).c_str ())));

      if (context_->sample_fmt != outputFormat_)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: converting decoded sample format %s to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::sampleFormatToString (context_->sample_fmt).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_Tools::sampleFormatToString (outputFormat_).c_str ())));

        AVChannelLayout channel_layout_out_s;
        result =
          av_channel_layout_from_mask (&channel_layout_out_s,
                                       Stream_Module_Decoder_Tools::channelsToMask (outputChannels_));
        if (unlikely (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to av_channel_layout_from_mask(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
          goto error;
        } // end IF
        AVChannelLayout channel_layout_in_s;
        result =
          av_channel_layout_from_mask (&channel_layout_in_s,
                                       Stream_Module_Decoder_Tools::channelsToMask (outputChannels_)); // *TODO* should be input-
        if (unlikely (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to av_channel_layout_from_mask(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
          goto error;
        } // end IF

        result =
          swr_alloc_set_opts2 (&transformContext_,
                               &channel_layout_out_s, // out_ch_layout
                               outputFormat_,         // out_sample_fmt
                               outputSampleRate_,     // out_sample_rate
                               &channel_layout_in_s,  // in_ch_layout
                               context_->sample_fmt,  // in_sample_fmt
                               outputSampleRate_,     // in_sample_rate
                               0,                     // log_offset
                               NULL);                 // log_ctx
        if (unlikely (result || !transformContext_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to swr_alloc_set_opts2(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
          goto error;
        } // end IF
        result = swr_init (transformContext_);
        if (unlikely (result < 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to swr_init(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
          goto error;
        } // end IF
      } // end IF

      format_ = context_->sample_fmt;
      sampleRate_ = context_->sample_rate;
      frameSize_ =
        av_get_bytes_per_sample (context_->sample_fmt) * outputChannels_;

      ACE_ASSERT (!frame_);
      frame_ = av_frame_alloc ();
      if (unlikely (!frame_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to av_frame_alloc(): \"%m\", aborting\n")));
        goto error;
      } // end IF

      if (outputFormat_ != media_type_s.format)
      { ACE_ASSERT (session_data_r.lock);
        inherited2::setFormat (outputFormat_,
                               media_type_2);
        inherited2::setSampleRate (outputSampleRate_,
                                   media_type_2);
        inherited2::setChannels (outputChannels_,
                                 media_type_2);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          session_data_r.formats.push_back (media_type_2);
        } // end lock scope
      } // end IF
      else
        inherited2::free_ (media_type_2);

      goto continue_;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (codec_parameters_p)
        avcodec_parameters_free (&codec_parameters_p);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (likely (context_))
      {
        if (likely (frame_))
          drainBuffers (message_inout->sessionId ());
        avcodec_free_context (&context_); context_ = NULL;
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
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::decodePacket (struct AVPacket& packet_in,
                                                             DataMessageType*& message_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::decodePacket"));

  // sanity check(s)
  ACE_ASSERT (!message_inout);
  ACE_ASSERT (context_);
  ACE_ASSERT (frame_);
  ACE_ASSERT (frameSize_);

  int result = avcodec_send_packet (context_,
                                    &packet_in);
  if (unlikely (result))
  { // *NOTE*: most likely cause: some spurious AAC decoding error
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to avcodec_send_packet(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  // pixel format/resolution may have changed
  if (unlikely ((context_->sample_fmt != format_)                        ||
                (context_->sample_rate != static_cast<int> (sampleRate_))))
  {
    if (transformContext_)
    {
      swr_free (&transformContext_); transformContext_ = NULL;
    } // end IF

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reinit occurred; converting decoded sample format \"%s\" (@ %u Hz) to \"%s\" (@ %u Hz)\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_MediaFramework_Tools::sampleFormatToString (context_->sample_fmt).c_str ()),
                context_->sample_rate,
                ACE_TEXT (Stream_MediaFramework_Tools::sampleFormatToString (outputFormat_).c_str ()),
                outputSampleRate_));

    AVChannelLayout channel_layout_out_s;
    result =
      av_channel_layout_from_mask (&channel_layout_out_s,
                                    Stream_Module_Decoder_Tools::channelsToMask (outputChannels_)); // *TODO* should be input-
    if (unlikely (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to av_channel_layout_from_mask(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    result =
      swr_alloc_set_opts2 (&transformContext_,
                           &channel_layout_out_s, // out_ch_layout
                           outputFormat_,         // out_sample_fmt
                           outputSampleRate_,     // out_sample_rate
                           &context_->ch_layout,  // in_ch_layout
                           context_->sample_fmt,  // in_sample_fmt
                           context_->sample_rate, // in_sample_rate
                           0,                     // log_offset
                           NULL);                 // log_ctx
    if (unlikely (result || !transformContext_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to swr_alloc_set_opts2(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    result = swr_init (transformContext_);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to swr_init(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF

    format_ = context_->sample_fmt;
    sampleRate_ = context_->sample_rate;
    frameSize_ =
      av_get_bytes_per_sample (context_->sample_fmt) * context_->ch_layout.nb_channels;
  } // end IF

  result = avcodec_receive_frame (context_,
                                  frame_);
  if (result < 0)
  {
    if (likely (result == AVERROR (EAGAIN)))
      return true;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to avcodec_receive_frame(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (frame_->data);

  // --> successfully decoded a frame

  ACE_Message_Block* message_block_p = NULL;

  // convert sample format / sample rate ?
  if (transformContext_)
  {
    result = swr_get_out_samples (transformContext_, frame_->nb_samples);
    ACE_ASSERT (result >= 0);

    message_block_p = inherited::allocateMessage (outputFrameSize_ * result);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  outputFrameSize_ * result));
      av_frame_unref (frame_);
      return false;
    } // end IF

    uint8_t* data_a[AV_NUM_DATA_POINTERS];
    ACE_OS::memset (&data_a, 0, sizeof (uint8_t* [AV_NUM_DATA_POINTERS]));
    data_a[0] = reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ());
    result =
      swr_convert (transformContext_,
                   data_a, result,
                   const_cast<const uint8_t**>(static_cast<uint8_t**> (frame_->data)), frame_->nb_samples);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to swr_convert(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      av_frame_unref (frame_);
      message_block_p->release ();
      return false;
    } // end IF
    message_block_p->wr_ptr (outputFrameSize_ * result);
  } // end IF
  else
  {
    message_block_p =
      inherited::allocateMessage (frameSize_ * frame_->nb_samples);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  frameSize_ * frame_->nb_samples));
      av_frame_unref (frame_);
      return false;
    } // end IF
    message_block_p->base (reinterpret_cast<char*> (frame_->data),
                           frameSize_ * frame_->nb_samples,
                           0); // own image data
    message_block_p->wr_ptr (frameSize_ * frame_->nb_samples);
  } // end ELSE
  ACE_ASSERT (message_block_p);
  message_inout = static_cast<DataMessageType*> (message_block_p);

  // clean up
  ACE_OS::memset (frame_->data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
  av_frame_unref (frame_);

  return true;
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
Stream_Decoder_LibAVAudioDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::drainBuffers (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVAudioDecoder_T::drainBuffers"));

  // sanity check(s)
  ACE_ASSERT (context_);

  struct AVPacket packet_s;
  ACE_OS::memset (&packet_s, 0, sizeof (struct AVPacket));
  DataMessageType* message_p = NULL;
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  av_init_packet (&packet_s);

  result = avcodec_send_packet (context_,
                                &packet_s);
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to avcodec_send_packet(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return;
  } // end IF

  do
  {
    result = avcodec_receive_frame (context_,
                                    frame_);
    if (result < 0)
    {
      if (result == AVERROR_EOF)
        break;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to avcodec_receive_frame(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    ACE_ASSERT (frame_->data);

    // --> successfully decoded a frame

    // convert pixel format of the decoded frame ?
    if (transformContext_)
    {
      result = swr_get_out_samples (transformContext_, frame_->nb_samples);
      ACE_ASSERT (result >= 0);

      message_block_p = inherited::allocateMessage (outputFrameSize_ * result);
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                    inherited::mod_->name (),
                    outputFrameSize_ * result));
        av_frame_unref (frame_);
        return;
      } // end IF

      uint8_t* data_a[AV_NUM_DATA_POINTERS];
      ACE_OS::memset (&data_a, 0, sizeof (uint8_t* [AV_NUM_DATA_POINTERS]));
      data_a[0] = reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ());

      result =
        swr_convert (transformContext_,
                     data_a, result,
                     const_cast<const uint8_t**>(static_cast<uint8_t**> (frame_->data)), frame_->nb_samples);
      if (unlikely (result < 0))
      {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to swr_convert(): \"%s\", returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
          av_frame_unref (frame_);
          message_block_p->release ();
          return;
      } // end IF
      message_block_p->wr_ptr (outputFrameSize_ * result);
    } // end IF
    else
    {
      message_block_p = inherited::allocateMessage (frameSize_);
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                    inherited::mod_->name (),
                    frameSize_));
        av_frame_unref (frame_);
        return;
      } // end IF
      message_block_p->base (reinterpret_cast<char*> (frame_->data),
                             frameSize_,
                             0); // own image data
      message_block_p->wr_ptr (frameSize_);
    } // end ELSE
    ACE_ASSERT (message_block_p);
    message_p = static_cast<DataMessageType*> (message_block_p);

    // clean up
    ACE_OS::memset (frame_->data, 0, sizeof (uint8_t * [8]));
    av_frame_unref (frame_);

    // forward the decoded frame
    message_p->initialize (sessionId_in,
                           NULL);
    //message_p->set (message_2->type ());
    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      message_p->release ();
      return;
    } // end IF
    message_p = NULL;
  } while (true);

  // flush transform context buffer
  if (transformContext_)
  {
    message_block_p =
      inherited::allocateMessage (outputFrameSize_ * outputSampleRate_ * 5);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                  inherited::mod_->name (),
                  outputFrameSize_ * outputSampleRate_ * 5));
      av_frame_unref (frame_);
      return;
    } // end IF

    uint8_t* data_a[AV_NUM_DATA_POINTERS];
    ACE_OS::memset (&data_a, 0, sizeof (uint8_t* [AV_NUM_DATA_POINTERS]));
    data_a[0] = reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ());

    result = swr_convert (transformContext_,
                          data_a, outputSampleRate_ * 5,
                          NULL, 0);
    if (unlikely (result < 0))
    {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to swr_convert(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        av_frame_unref (frame_);
        message_block_p->release ();
        return;
    } // end IF
    message_block_p->wr_ptr (outputFrameSize_ * result);
    ACE_ASSERT (message_block_p);
    message_p = static_cast<DataMessageType*> (message_block_p);

    // forward the decoded frame
    message_p->initialize (sessionId_in,
                           NULL);
    //message_p->set (message_2->type ());
    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      message_p->release ();
      return;
    } // end IF
    message_p = NULL;
  } // end IF
}
