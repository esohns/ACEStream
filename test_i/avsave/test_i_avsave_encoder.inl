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
 , condition_ (inherited::lock_)
 , isFirst_ (true)
 , isLast_ (false)
 , numberOfStreamsInitialized_ (0)
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
Test_I_AVSave_Encoder_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataContainerType,
                        MediaType>::~Test_I_AVSave_Encoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_AVSave_Encoder_T::~Test_I_AVSave_Encoder_T"));

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
    isFirst_ = true;
    isLast_ = false;
    numberOfStreamsInitialized_ = 0;
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
      frame_p->nb_samples =
        message_block_p->length () / inherited::audioFrameSize_;
      frame_p->pts =
        av_rescale_q (inherited::audioSamples_,
                      (AVRational){1, inherited::audioCodecContext_->sample_rate},
                      inherited::audioCodecContext_->time_base);
      inherited::audioSamples_ += frame_p->nb_samples;
      stream_p = inherited::audioStream_;
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      codec_context_p = inherited::videoCodecContext_;
      frame_p = inherited::videoFrame_;
      frame_p->nb_samples = 1;
      ++inherited::videoSamples_;
      frame_p->pts = inherited::videoSamples_;
      stream_p = inherited::videoStream_;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message media type (was: %d), continuing\n"),
                  inherited::mod_->name (),
                  message_inout->getMediaType ()));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (codec_context_p && frame_p && stream_p);

  do
  { ACE_ASSERT (message_block_p);
    frame_p->data[0] = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());

    // send the frame to the encoder
    result = avcodec_send_frame (codec_context_p, frame_p);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to avcodec_send_frame(): \"%s\", returning\n"),
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
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avcodec_receive_packet(): \"%s\", returning\n"),
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
      packet_s.dts =
        av_rescale_q_rnd (packet_s.dts,
                          codec_context_p->time_base,
                          stream_p->time_base,
                          static_cast<enum AVRounding> (AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
      packet_s.duration = av_rescale_q (packet_s.duration,
                                        codec_context_p->time_base,
                                        stream_p->time_base);
//      av_packet_rescale_ts (&packet_s,
//                            codec_context_p->time_base,
//                            stream_p->time_base);
      packet_s.stream_index = stream_p->index;

      /* Write the frame to the media file. */
//      result = av_write_frame (formatContext_, &packet_s);
      result = av_interleaved_write_frame (inherited::formatContext_,
                                           &packet_s);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to av_interleaved_write_frame(): \"%s\", returning\n"),
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
//  this->notify (STREAM_SESSION_MESSAGE_ABORT);
  ;
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
      bool is_first_b = true;

      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_AUDIO,
                               audio_media_type_s);
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               video_media_type_s);

      // *IMPORTANT NOTE*: initialize the format/output context only once
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (!isFirst_)
        {
          is_first_b = false;
          goto continue_;
        } // end IF
        isFirst_ = false;
      } // end lock scope

      output_format_p =
          const_cast<struct AVOutputFormat*> (av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"),
                                                               NULL,
                                                               NULL));
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
      // *IMPORTANT NOTE*: the workaround to set these directly (see also:
      // https://stackoverflow.com/questions/67882397/change-the-default-audio-and-video-codec-loaded-by-avformat-alloc-output-context)
      //                   does not work (crashes on Windows)
      //output_format_p->audio_codec = audio_coded_id;
      //output_format_p->video_codec = video_coded_id;

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
//      inherited::audioStream_->id = inherited::formatContext_->nb_streams - 1;
      inherited::audioStream_->id = 1;

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
      inherited::audioCodecContext_->channels = audio_media_type_s.channels;
      inherited::audioCodecContext_->sample_fmt = audio_media_type_s.format;
      inherited::audioCodecContext_->bit_rate =
        inherited::audioFrameSize_ * audio_media_type_s.sampleRate * 8;
      inherited::audioCodecContext_->sample_rate =
        audio_media_type_s.sampleRate;
      switch (audio_media_type_s.channels)
      {
        case 1:
          inherited::audioCodecContext_->channel_layout = AV_CH_LAYOUT_MONO;
          break;
        case 2:
          inherited::audioCodecContext_->channel_layout = AV_CH_LAYOUT_STEREO;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown #channels (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      audio_media_type_s.channels));
          goto error;
        }
      } // end SWITCH
//      inherited::audioStream_->time_base.num = 1;
//      inherited::audioStream_->time_base.den =
//        inherited::audioCodecContext_->sample_rate;
      inherited::audioCodecContext_->time_base.num = 1;
        inherited::audioStream_->time_base;
      inherited::audioCodecContext_->time_base.den =
        inherited::audioCodecContext_->sample_rate;

      inherited::audioFrame_->channels =
        inherited::audioCodecContext_->channels;
      inherited::audioFrame_->format =
        inherited::audioCodecContext_->sample_fmt;
      inherited::audioFrame_->channel_layout =
        inherited::audioCodecContext_->channel_layout;
      inherited::audioFrame_->sample_rate =
        inherited::audioCodecContext_->sample_rate;
//      inherited::audioFrame_->time_base =
//        inherited::audioCodecContext_->time_base;

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

//      audioStream_->codec = audioCodecContext_;
      avcodec_parameters_from_context (inherited::audioStream_->codecpar,
                                       inherited::audioCodecContext_);

      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        ++numberOfStreamsInitialized_;
        result = condition_.signal ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Thread_Condition::signal(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end lock scope

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
      inherited::videoStream_->id = 0;
//      inherited::videoStream_->id = inherited::formatContext_->nb_streams - 1;

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

      inherited::videoCodecContext_->codec_id = video_coded_id;

      inherited::videoCodecContext_->pix_fmt =
          static_cast<AVPixelFormat> (inherited::videoFrame_->format);
      inherited::videoFrameSize_ =
        av_image_get_buffer_size (inherited::videoCodecContext_->pix_fmt,
                                  inherited::videoFrame_->width,
                                  inherited::videoFrame_->height,
                                  1); // *TODO*: linesize alignment

//      inherited::videoStream_->time_base.num = 1;
//      inherited::videoStream_->time_base.den = video_media_type_s.frameRate.num;
      inherited::videoStream_->avg_frame_rate.num =
        video_media_type_s.frameRate.num;
      inherited::videoStream_->avg_frame_rate.den =
        video_media_type_s.frameRate.den;

      inherited::videoCodecContext_->bit_rate =
        inherited::videoFrameSize_ * inherited::videoStream_->time_base.den * 8;
      /* Resolution must be a multiple of two. */
      inherited::videoCodecContext_->width = inherited::videoFrame_->width;
      inherited::videoCodecContext_->height = inherited::videoFrame_->height;
      inherited::videoCodecContext_->framerate = video_media_type_s.frameRate;
      /* timebase: This is the fundamental unit of time (in seconds) in terms
       * of which frame timestamps are represented. For fixed-fps content,
       * timebase should be 1/framerate and timestamp increments should be
       * identical to 1. */
      inherited::videoCodecContext_->time_base.num = 1;
      inherited::videoCodecContext_->time_base.den =
        video_media_type_s.frameRate.num;
      inherited::videoCodecContext_->pkt_timebase =
        inherited::videoStream_->time_base;

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

//      videoStream_->codec = inherited::videoCodecContext_;
      avcodec_parameters_from_context (inherited::videoStream_->codecpar,
                                       inherited::videoCodecContext_);

      result = av_image_fill_linesizes (inherited::videoFrame_->linesize,
                                        static_cast<AVPixelFormat> (inherited::videoFrame_->format),
                                        static_cast<int> (inherited::videoFrame_->width));
      ACE_ASSERT (result >= 0);

      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        ++numberOfStreamsInitialized_;
        result = condition_.signal ();
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Thread_Condition::signal(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end lock scope

      goto continue_2;

error:
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (codec_parameters_p)
//        avcodec_parameters_free (&codec_parameters_p);
//#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_2:
      if (!is_first_b)
        goto continue_3;

      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        while (numberOfStreamsInitialized_ < 2)
        {
          result = condition_.wait ();
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Thread_Condition::wait(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            goto error;
          } // end IF
        } // end WHILE
      } // end lock scope

      result = avformat_write_header (inherited::formatContext_, NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_write_header() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      inherited::headerWritten_ = true;

continue_3:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      int result = -1;

      // *IMPORTANT NOTE*: finalize the format context only once
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (!isLast_)
        {
          isLast_ = true;
          goto continue_4;
        } // end IF
      } // end lock scope

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

      avformat_free_context (inherited::formatContext_); inherited::formatContext_ = NULL;

continue_5:
      if (inherited::audioCodecContext_)
      {
        avcodec_free_context (&(inherited::audioCodecContext_)); ACE_ASSERT (!inherited::audioCodecContext_);
      } // end IF
      if (inherited::videoCodecContext_)
      {
        avcodec_free_context (&(inherited::videoCodecContext_)); ACE_ASSERT (!inherited::videoCodecContext_);
      } // end IF

continue_4:
      break;
    }
    default:
      break;
  } // end SWITCH
}
