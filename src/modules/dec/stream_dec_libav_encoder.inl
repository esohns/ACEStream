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
Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Decoder_LibAVEncoder_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Decoder_LibAVEncoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , audioCodecContext_ (NULL)
 , audioFrame_ (NULL)
 , audioFrameSize_ (0)
 , audioStream_ (NULL)
 , formatContext_ (NULL)
 , headerWritten_ (false)
 , videoCodecContext_ (NULL)
 , videoFrame_ (NULL)
 , videoFrameSize_ (0)
 , videoStream_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVEncoder_T::Stream_Decoder_LibAVEncoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::~Stream_Decoder_LibAVEncoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVEncoder_T::~Stream_Decoder_LibAVEncoder_T"));

  if (formatContext_)
    avformat_free_context (formatContext_);

//  if (audioCodecContext_)
//    avcodec_free_context (&audioCodecContext_);
//  if (videoCodecContext_)
//    avcodec_free_context (&videoCodecContext_);

  if (audioFrame_)
    av_frame_free (&audioFrame_);
  if (videoFrame_)
    av_frame_free (&videoFrame_);
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
Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVEncoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (audioCodecContext_)
      avcodec_free_context (&audioCodecContext_);
    if (audioFrame_)
    {
      av_frame_free (&audioFrame_); ACE_ASSERT (!audioFrame_);
    } // end IF
    audioFrameSize_ = 0;
    audioStream_ = NULL; // *TODO*: how are these freed ?
    if (formatContext_)
    {
      avformat_free_context (formatContext_); formatContext_ = NULL;
    } // end IF
    headerWritten_ = false;
    if (videoCodecContext_)
      avcodec_free_context (&videoCodecContext_);
    if (videoFrame_)
    {
      av_frame_free (&videoFrame_); ACE_ASSERT (!videoFrame_);
    } // end IF
    videoFrameSize_ = 0;
    videoStream_ = NULL; // *TODO*: how are these freed ?
  } // end IF

#if defined (_DEBUG)
  //av_log_set_callback (Stream_Decoder_LibAVDecoder_LoggingCB);
  // *NOTE*: this level logs all messages
  //av_log_set_level (std::numeric_limits<int>::max ());
#endif // _DEBUG
  //av_register_all ();
//  avcodec_register_all ();

  audioFrame_ = av_frame_alloc ();
  if (unlikely (!audioFrame_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  audioFrame_->pts = 0;

  videoFrame_ = av_frame_alloc ();
  if (unlikely (!videoFrame_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  videoFrame_->pts = 0;

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
Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVEncoder_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (!headerWritten_))
    return; // --> not fully initialized (yet)

  int result = -1;
  ACE_Message_Block* message_block_p = message_inout;
  AVCodecContext* codec_context_p = NULL;
  AVFrame* frame_p = NULL;
  AVStream* stream_p = NULL;
//  unsigned int stream_frame_size_i = 0;

  switch (message_inout->getMediaType ())
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      codec_context_p = audioCodecContext_;
      frame_p = audioFrame_;
      frame_p->nb_samples = message_block_p->length () / audioFrameSize_;
      frame_p->pts += frame_p->nb_samples;
      stream_p = audioStream_;
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      codec_context_p = videoCodecContext_;
      frame_p = videoFrame_;
      frame_p->nb_samples = message_block_p->length () / videoFrameSize_;
      ACE_ASSERT (frame_p->nb_samples == 1);
      frame_p->pts += frame_p->nb_samples;
      stream_p = videoStream_;
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
      av_packet_rescale_ts (&packet_s,
                            codec_context_p->time_base,
                            stream_p->time_base);
      packet_s.stream_index = stream_p->index;

      /* Write the frame to the media file. */
//      result = av_write_frame (formatContext_, &packet_s);
      result = av_interleaved_write_frame (formatContext_, &packet_s);
      av_packet_unref (&packet_s);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to av_interleaved_write_frame(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
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
Stream_Decoder_LibAVEncoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVEncoder_T::handleSessionMessage"));

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

      int result = -1;
      struct Stream_MediaFramework_FFMPEG_AudioMediaType audio_media_type_s;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType video_media_type_s;
      const struct AVOutputFormat* output_format_p = NULL;
      // *TODO*: derive these from the specified output format !
      enum AVCodecID video_coded_id = AV_CODEC_ID_RAWVIDEO;
      enum AVCodecID audio_coded_id = AV_CODEC_ID_PCM_S16LE;

      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                audio_media_type_s);
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                video_media_type_s);

      if (formatContext_)
        goto continue_;

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
        avformat_alloc_output_context2 (&formatContext_,
                                        const_cast<struct AVOutputFormat*> (output_format_p),
                                        NULL,
                                        NULL);
      if (unlikely ((result < 0) || !formatContext_))
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

      result = avio_open (&formatContext_->pb,
                          ACE_TEXT_ALWAYS_CHAR (session_data_r.targetFileName.c_str ()),
                          AVIO_FLAG_WRITE);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avio_open() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

continue_:
      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      switch (message_inout->getMediaType ())
      {
        case STREAM_MEDIATYPE_AUDIO:
          goto audio;
        case STREAM_MEDIATYPE_VIDEO:
        {
          videoFrame_->format = video_media_type_s.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          videoFrame_->height =
              static_cast<unsigned int> (std::abs (video_media_type_s.resolution.cy));
          videoFrame_->width =
            static_cast<unsigned int> (video_media_type_s.resolution.cx);
#else
          videoFrame_->height = video_media_type_s.resolution.height;
          videoFrame_->width = video_media_type_s.resolution.width;
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
      formatContext_->audio_codec = avcodec_find_encoder (audio_coded_id);
      if (unlikely (!formatContext_->audio_codec))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    audio_coded_id));
        goto error;
      } // end IF
      formatContext_->audio_codec_id = audio_coded_id;

      audioStream_ = avformat_new_stream (formatContext_, formatContext_->audio_codec);
      if (!audioStream_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      audioStream_->id = formatContext_->nb_streams - 1;

      audioCodecContext_ = avcodec_alloc_context3 (formatContext_->audio_codec);
      if (unlikely (!audioCodecContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (audioCodecContext_);

      audioFrameSize_ =
        (audio_media_type_s.channels * av_get_bytes_per_sample (audio_media_type_s.format));
      audioCodecContext_->channels = audio_media_type_s.channels;
      audioCodecContext_->sample_fmt = audio_media_type_s.format;
      audioCodecContext_->bit_rate =
        audioFrameSize_ * audio_media_type_s.sampleRate * 8;
      audioCodecContext_->sample_rate = audio_media_type_s.sampleRate;
      switch (audio_media_type_s.channels)
      {
        case 1:
          audioCodecContext_->channel_layout = AV_CH_LAYOUT_MONO;
          break;
        case 2:
          audioCodecContext_->channel_layout = AV_CH_LAYOUT_STEREO;
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
      audioStream_->time_base.num = 1;
      audioStream_->time_base.den = audioCodecContext_->sample_rate;
      audioCodecContext_->time_base = audioStream_->time_base;

      audioFrame_->format = audioCodecContext_->sample_fmt;
      audioFrame_->channel_layout = audioCodecContext_->channel_layout;
      audioFrame_->sample_rate = audioCodecContext_->sample_rate;

      result = avcodec_open2 (audioCodecContext_,
                              audioCodecContext_->codec,
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
                  ACE_TEXT (audioCodecContext_->codec->long_name),
                  ACE_TEXT (Stream_Module_Decoder_Tools::audioFormatToString (audioCodecContext_->sample_fmt).c_str ())));

//      audioStream_->codec = audioCodecContext_;
      avcodec_parameters_from_context (audioStream_->codecpar,
                                       audioCodecContext_);

      goto continue_2;

video:
      formatContext_->video_codec = avcodec_find_encoder (video_coded_id);
      if (unlikely (!formatContext_->video_codec))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_encoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    video_coded_id));
        goto error;
      } // end IF
      formatContext_->video_codec_id = video_coded_id;

      videoStream_ = avformat_new_stream (formatContext_, formatContext_->video_codec);
      if (!videoStream_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avformat_new_stream() failed, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      videoStream_->id = formatContext_->nb_streams - 1;

      videoCodecContext_ = avcodec_alloc_context3 (formatContext_->video_codec);
      if (unlikely (!videoCodecContext_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (videoCodecContext_);

      videoCodecContext_->codec_id = video_coded_id;

      videoCodecContext_->pix_fmt =
          static_cast<AVPixelFormat> (videoFrame_->format);
      videoFrameSize_ =
        av_image_get_buffer_size (videoCodecContext_->pix_fmt,
                                  videoFrame_->width,
                                  videoFrame_->height,
                                  1); // *TODO*: linesize alignment

      videoStream_->time_base.num = 1;
      videoStream_->time_base.den = video_media_type_s.frameRate.num;
      videoStream_->avg_frame_rate.num = video_media_type_s.frameRate.num;
      videoStream_->avg_frame_rate.den = video_media_type_s.frameRate.den;

      videoCodecContext_->bit_rate =
          videoFrameSize_ * videoStream_->time_base.den * 8;
      /* Resolution must be a multiple of two. */
      videoCodecContext_->width    = videoFrame_->width;
      videoCodecContext_->height   = videoFrame_->height;
      /* timebase: This is the fundamental unit of time (in seconds) in terms
       * of which frame timestamps are represented. For fixed-fps content,
       * timebase should be 1/framerate and timestamp increments should be
       * identical to 1. */
      videoCodecContext_->time_base = videoStream_->time_base;

      result = avcodec_open2 (videoCodecContext_,
                              videoCodecContext_->codec,
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
                  ACE_TEXT (videoCodecContext_->codec->long_name),
                  ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (videoCodecContext_->pix_fmt).c_str ())));

//      videoStream_->codec = videoCodecContext_;
      avcodec_parameters_from_context (videoStream_->codecpar,
                                       videoCodecContext_);

      result = av_image_fill_linesizes (videoFrame_->linesize,
                                        static_cast<AVPixelFormat> (videoFrame_->format),
                                        static_cast<int> (videoFrame_->width));
      ACE_ASSERT (result >= 0);

      goto continue_2;

error:
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (codec_parameters_p)
//        avcodec_parameters_free (&codec_parameters_p);
//#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_2:
      if (!headerWritten_ && (formatContext_->nb_streams == 2))
      {
        result = avformat_write_header (formatContext_, NULL);
        if (unlikely (result < 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: avformat_write_header() failed: \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        headerWritten_ = true;
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      int result = -1;

      ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);

      if (!formatContext_)
        goto continue_3;

      if (headerWritten_)
      {
        result = av_write_trailer (formatContext_);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: av_write_trailer() failed: \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        if (!(formatContext_->oformat->flags & AVFMT_NOFILE) && formatContext_->pb)
        {
          result = avio_close (formatContext_->pb);
          if (unlikely (result < 0))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: avio_close() failed: \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        } // end IF
      } // end IF

      avformat_free_context (formatContext_); formatContext_ = NULL;

continue_3:
//      if (audioCodecContext_)
//      {
//        avcodec_free_context (&audioCodecContext_); ACE_ASSERT (!audioCodecContext_);
//      } // end IF
//      if (videoCodecContext_)
//      {
//        avcodec_free_context (&videoCodecContext_); ACE_ASSERT (!videoCodecContext_);
//      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
