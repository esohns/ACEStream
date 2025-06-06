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
#include "libavformat/avformat.h"
#include "libavutil/channel_layout.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
}
#endif /* __cplusplus */

#include "ace/Log_Msg.h"

#include "common_tools.h"
#if defined (_DEBUG)
#include "common_file_tools.h"
#endif // _DEBUG

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_lib_common.h"
#include "stream_lib_ffmpeg_common.h"
#include "stream_lib_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Test_I_AVSave_Encoder_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                        MediaType>::Test_I_AVSave_Encoder_T (ISTREAM_T* stream_in)
#else
                        MediaType>::Test_I_AVSave_Encoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVSave_Encoder_T::Test_I_AVSave_Encoder_T"));

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
Test_I_AVSave_Encoder_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataContainerType,
                        MediaType>::initialize (const ConfigurationType& configuration_in,
                                                Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVSave_Encoder_T::initialize"));

  if (inherited::isInitialized_)
  {
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
Test_I_AVSave_Encoder_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataContainerType,
                        MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVSave_Encoder_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (!inherited::headerWritten_))
    return; // --> not fully initialized (yet)
  if (unlikely (!inherited::formatContext_))
    return; // --> disregard 'late' messages

  int result = -1;
  ACE_Message_Block* message_block_p = message_inout;
  AVCodecContext* codec_context_p = NULL;
  AVFrame* frame_p = NULL;
  AVStream* stream_p = NULL;

  switch (message_inout->getMediaType ())
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      codec_context_p = inherited::audioCodecContext_;
      frame_p = inherited::audioFrame_;
      ACE_ASSERT (message_block_p->length () % inherited::audioFrameSize_ == 0);
      frame_p->nb_samples =
        static_cast<int> (message_block_p->length ()) / static_cast<int> (inherited::audioFrameSize_);
      frame_p->pts =
        av_rescale_q (inherited::audioSamples_,
                      {1, inherited::audioCodecContext_->sample_rate},
                      inherited::audioCodecContext_->time_base);
      inherited::audioSamples_ += frame_p->nb_samples;
      stream_p = inherited::audioStream_;
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      codec_context_p = inherited::videoCodecContext_;
      frame_p = inherited::videoFrame_;
      ACE_ASSERT (message_block_p->length () / inherited::videoFrameSize_ == 1);
      frame_p->duration = 1;
      //frame_p->flags |= AV_FRAME_FLAG_KEY;
      frame_p->key_frame = 1;
      frame_p->nb_samples = 1;
      frame_p->pts = inherited::videoSamples_;
      ++inherited::videoSamples_;
      stream_p = inherited::videoStream_;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message media type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  message_inout->getMediaType ()));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (codec_context_p && frame_p && stream_p);

  do
  { ACE_ASSERT (message_block_p);
    frame_p->buf[0] =
      av_buffer_create (reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ()),
                        message_block_p->length (), NULL, NULL, 0);
    frame_p->data[0] = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());
    frame_p->buf[0]->data = frame_p->data[0];
    frame_p->buf[0]->size = message_block_p->length ();

    // send the frame to the encoder
    result = avcodec_send_frame (codec_context_p, frame_p);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to avcodec_send_frame(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    while (result >= 0)
    {
      struct AVPacket packet_s = { 0 };
      result = avcodec_receive_packet (codec_context_p, &packet_s);
      if (result == AVERROR (EAGAIN) || result == AVERROR_EOF)
        break;
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avcodec_receive_packet(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end ELSE IF

      /* rescale output packet timestamp values from codec to stream timebase */
      packet_s.pts =
        av_rescale_q_rnd (packet_s.pts,
                          codec_context_p->time_base,
                          stream_p->time_base,
                          static_cast<enum AVRounding> (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
      //packet_s.dts = AV_NOPTS_VALUE;
      packet_s.dts = 
        av_rescale_q_rnd (packet_s.dts,
                          codec_context_p->time_base,
                          stream_p->time_base,
                          static_cast<enum AVRounding> (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));

      packet_s.time_base = stream_p->time_base;
      switch (message_inout->getMediaType ())
      {
        case STREAM_MEDIATYPE_AUDIO:
          break;
        case STREAM_MEDIATYPE_VIDEO:
        {
          packet_s.duration = 1;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown message media type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      message_inout->getMediaType ()));
          goto error;
        }
      } // end SWITCH
      packet_s.duration = av_rescale_q (packet_s.duration,
                                        codec_context_p->time_base,
                                        stream_p->time_base);
      //av_packet_rescale_ts (&packet_s,
      //                      codec_context_p->time_base,
      //                      stream_p->time_base);
      packet_s.stream_index = stream_p->index;

      /* Write the frame to the media file. */
//      result = av_write_frame (formatContext_, &packet_s);
      result = av_interleaved_write_frame (inherited::formatContext_,
                                           &packet_s);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to av_interleaved_write_frame(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        av_packet_unref (&packet_s);
        goto error;
      } // end IF
      av_packet_unref (&packet_s);
    } // end WHILE

    message_block_p = message_block_p->cont ();
    if (!message_block_p)
      break;
  } while (true);

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
Test_I_AVSave_Encoder_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataContainerType,
                        MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVSave_Encoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inferences
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (!session_data_r.targetFileName.empty ());

      int result = -1;
      struct Stream_MediaFramework_FFMPEG_AudioMediaType audio_media_type_s;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType video_media_type_s;
      const struct AVOutputFormat* output_format_p = NULL;
      // *NOTE*: derive these from the specified input formats
      enum AVCodecID video_coded_id = AV_CODEC_ID_RAWVIDEO;
      enum AVCodecID audio_coded_id = AV_CODEC_ID_NONE;

      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_AUDIO,
                               audio_media_type_s);
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               video_media_type_s);

      // *IMPORTANT NOTE*: initialize the format/output context only once
      if (!inherited::isFirst_)
        goto continue_;
      inherited::isFirst_ = false;

      output_format_p = av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"),
                                         NULL,
                                         NULL);
      if (unlikely (!output_format_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: av_guess_format(\"avi\") failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result =
        avformat_alloc_output_context2 (&(inherited::formatContext_),
                                        const_cast<struct AVOutputFormat*> (output_format_p),
                                        NULL,
                                        NULL);
      if (unlikely ((result < 0) || !inherited::formatContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_alloc_output_context2() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      result =
        avio_open (&(inherited::formatContext_->pb),
                   ACE_TEXT_ALWAYS_CHAR (session_data_r.targetFileName.c_str ()),
                   AVIO_FLAG_WRITE);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avio_open(\"%s\") failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT_ALWAYS_CHAR (session_data_r.targetFileName.c_str ())));
        goto error;
      } // end IF

continue_:
      switch (message_inout->getMediaType ())
      {
        case STREAM_MEDIATYPE_AUDIO:
          goto audio;
        case STREAM_MEDIATYPE_VIDEO:
        {
          inherited::videoFrame_->format = video_media_type_s.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          inherited::videoFrame_->height =
              static_cast<unsigned int> (std::abs (video_media_type_s.resolution.cy));
          inherited::videoFrame_->width =
            static_cast<unsigned int> (video_media_type_s.resolution.cx);
#else
          inherited::videoFrame_->height = video_media_type_s.resolution.height;
          inherited::videoFrame_->width = video_media_type_s.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64

          goto video;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown message media type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      message_inout->getMediaType ()));
          goto error;
        }
      } // end SWITCH

audio:
      audio_coded_id =
        Stream_MediaFramework_Tools::ffmpegFormatToffmpegCodecId (audio_media_type_s.format);
      inherited::formatContext_->audio_codec =
        avcodec_find_encoder (audio_coded_id);
      if (unlikely (!inherited::formatContext_->audio_codec))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    audio_coded_id));
        goto error;
      } // end IF
      inherited::formatContext_->audio_codec_id = audio_coded_id;

      inherited::audioCodecContext_ =
        avcodec_alloc_context3 (inherited::formatContext_->audio_codec);
      if (unlikely (!inherited::audioCodecContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (inherited::audioCodecContext_);

      inherited::audioFrameSize_ =
        (audio_media_type_s.channels * av_get_bytes_per_sample (audio_media_type_s.format));
      av_channel_layout_default (&inherited::audioCodecContext_->ch_layout,
                                 audio_media_type_s.channels);
      inherited::audioCodecContext_->sample_fmt = audio_media_type_s.format;
      inherited::audioCodecContext_->bit_rate =
        inherited::audioFrameSize_ * audio_media_type_s.sampleRate * 8;
      inherited::audioCodecContext_->sample_rate =
        audio_media_type_s.sampleRate;
      inherited::audioCodecContext_->time_base.num = 1;
      inherited::audioCodecContext_->time_base.den = 
        inherited::audioCodecContext_->sample_rate;

      inherited::audioFrame_->ch_layout =
        inherited::audioCodecContext_->ch_layout;
      inherited::audioFrame_->format =
        inherited::audioCodecContext_->sample_fmt;
      inherited::audioFrame_->sample_rate =
        inherited::audioCodecContext_->sample_rate;
      inherited::audioFrame_->time_base = inherited::audioCodecContext_->time_base;

      result = avcodec_open2 (inherited::audioCodecContext_,
                              inherited::audioCodecContext_->codec,
                              NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2() failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: initialized codec %s; encoded audio format: %s\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::audioCodecContext_->codec->long_name),
                  ACE_TEXT (Stream_Module_Decoder_Tools::audioFormatToString (inherited::audioCodecContext_->sample_fmt).c_str ())));

      ++inherited::numberOfStreamsInitialized_;

      goto continue_2;

video:
      inherited::formatContext_->video_codec =
        avcodec_find_encoder (video_coded_id);
      if (unlikely (!inherited::formatContext_->video_codec))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    video_coded_id));
        goto error;
      } // end IF
      inherited::formatContext_->video_codec_id = video_coded_id;

      inherited::videoFrame_->time_base.num = 1;
      inherited::videoFrame_->time_base.den = video_media_type_s.frameRate.num;

      inherited::videoCodecContext_ =
        avcodec_alloc_context3 (inherited::formatContext_->video_codec);
      if (unlikely (!inherited::videoCodecContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (inherited::videoCodecContext_);

      inherited::videoCodecContext_->bits_per_raw_sample =
        Stream_MediaFramework_Tools::AVPixelFormatToBitCount (static_cast<AVPixelFormat> (inherited::videoFrame_->format), true);
      ACE_ASSERT (inherited::videoCodecContext_->codec_id == video_coded_id);
      inherited::videoCodecContext_->pix_fmt =
          static_cast<AVPixelFormat> (inherited::videoFrame_->format);
      inherited::videoFrameSize_ =
        av_image_get_buffer_size (static_cast<AVPixelFormat> (inherited::videoFrame_->format),
                                  inherited::videoFrame_->width,
                                  inherited::videoFrame_->height,
                                  1); // *TODO*: linesize alignment

      inherited::videoCodecContext_->bit_rate =
        inherited::videoFrameSize_ * inherited::videoFrame_->time_base.den * 8;
      /* Resolution must be a multiple of two. */
      inherited::videoCodecContext_->width = inherited::videoFrame_->width;
      inherited::videoCodecContext_->height = inherited::videoFrame_->height;
      inherited::videoCodecContext_->framerate = video_media_type_s.frameRate;
      /* timebase: This is the fundamental unit of time (in seconds) in terms
       * of which frame timestamps are represented. For fixed-fps content,
       * timebase should be 1/framerate and timestamp increments should be
       * identical to 1. */
      inherited::videoCodecContext_->time_base = inherited::videoFrame_->time_base;
      //inherited::videoCodecContext_->pkt_timebase = inherited::videoFrame_->time_base;
      inherited::videoCodecContext_->frame_size = inherited::videoFrameSize_;

      result = avcodec_open2 (inherited::videoCodecContext_,
                              inherited::videoCodecContext_->codec,
                              NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2() failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: initialized codec %s; encoded pixel format: %s\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::videoCodecContext_->codec->long_name),
                  ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::videoCodecContext_->pix_fmt).c_str ())));

      result =
        av_image_fill_linesizes (inherited::videoFrame_->linesize,
                                 static_cast<AVPixelFormat> (inherited::videoFrame_->format),
                                 static_cast<int> (inherited::videoFrame_->width));
      ACE_ASSERT (result >= 0);

      ++inherited::numberOfStreamsInitialized_;

      goto continue_2;

error:
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (codec_parameters_p)
//        avcodec_parameters_free (&codec_parameters_p);
//#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_2:
      if (inherited::numberOfStreamsInitialized_ < 2)
        goto continue_3;

      inherited::videoStream_ =
        avformat_new_stream (inherited::formatContext_,
                             inherited::formatContext_->video_codec);
      if (!inherited::videoStream_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      inherited::videoStream_->time_base = inherited::videoFrame_->time_base;
      inherited::videoStream_->avg_frame_rate = video_media_type_s.frameRate;
      inherited::videoStream_->codecpar = avcodec_parameters_alloc ();
      avcodec_parameters_from_context (inherited::videoStream_->codecpar,
                                       inherited::videoCodecContext_);

      inherited::audioStream_ =
        avformat_new_stream (inherited::formatContext_,
                             inherited::formatContext_->audio_codec);
      if (!inherited::audioStream_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      inherited::audioStream_->time_base =
        inherited::audioCodecContext_->time_base;
      inherited::audioStream_->codecpar = avcodec_parameters_alloc ();
      avcodec_parameters_from_context (inherited::audioStream_->codecpar,
                                       inherited::audioCodecContext_);

      // *NOTE*: on windows, vlc does not show an image at all, unless this is set (the images appear upside down though :-()
      // *TODO*: is this required on linux too ?
      result =
        av_opt_set (inherited::formatContext_->priv_data,
                    ACE_TEXT_ALWAYS_CHAR ("flipped_raw_rgb"), ACE_TEXT_ALWAYS_CHAR ("true"),
                    0);
      ACE_ASSERT (result >= 0);
      result = avformat_write_header (inherited::formatContext_, NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_write_header() failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      inherited::headerWritten_ = true;

continue_3:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      int result = -1;

      // *IMPORTANT NOTE*: finalize the format context (and everything else) only once
      if (!inherited::isLast_)
      {
        inherited::isLast_ = true;
        goto continue_4;
      } // end IF

      if (!inherited::headerWritten_)
        goto continue_5;

      result = av_write_trailer (inherited::formatContext_);
      if (unlikely (result < 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: av_write_trailer() failed: \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      if (!(inherited::formatContext_->oformat->flags & AVFMT_NOFILE) &&
          inherited::formatContext_->pb)
      {
        result = avio_close (inherited::formatContext_->pb);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: avio_close() failed: \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      } // end IF

continue_5:
      avformat_free_context (inherited::formatContext_); inherited::formatContext_ = NULL;
      if (inherited::audioCodecContext_)
      {
        avcodec_free_context (&(inherited::audioCodecContext_)); ACE_ASSERT (!inherited::audioCodecContext_);
      } // end IF
      if (inherited::videoCodecContext_)
      {
        avcodec_free_context (&(inherited::videoCodecContext_)); ACE_ASSERT (!inherited::videoCodecContext_);
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      { Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,  // wait for completion ?
                       false); // high priority ?
      } // end IF

continue_4:
      break;
    }
    default:
      break;
  } // end SWITCH
}
