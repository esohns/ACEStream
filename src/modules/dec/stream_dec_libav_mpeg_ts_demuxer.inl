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

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::Stream_Decoder_LibAV_MPEG_TS_Demuxer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , formatContext_ (NULL)
 , IOBuffer_ (NULL)
 , IOCBData_ ()
 , IOContext_ (NULL)
 , queue_ (0,    // max # slots; 0 --> unlimited
           NULL) // notification handle
 , streamIndexToMessageMediaType_ ()
 , audioStreamPacketId_ (0)
 , videoStreamPacketId_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::Stream_Decoder_LibAV_MPEG_TS_Demuxer_T"));

  IOCBData_.queue = &queue_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::~Stream_Decoder_LibAV_MPEG_TS_Demuxer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::~Stream_Decoder_LibAV_MPEG_TS_Demuxer_T"));

  if (IOCBData_.buffer)
    IOCBData_.buffer->release ();

  if (formatContext_)
    avformat_close_input (&formatContext_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::initialize (const ConfigurationType& configuration_in,
                                                               Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (formatContext_)
    {
      avformat_close_input (&formatContext_);
      ACE_ASSERT (formatContext_ == NULL);
      //avformat_free_context (context_); context_ = NULL;
    } // end IF

    if (IOBuffer_)
    {
      //av_free (IOBuffer_);
      IOBuffer_ = NULL;
    } // end IF
    if (IOCBData_.buffer)
    {
      IOCBData_.buffer->release (); IOCBData_.buffer = NULL;
    } // end IF
    if (IOContext_)
    {
      // avio_context_free (&IOContext_);
      IOContext_ = NULL;
    } // end IF

    queue_.flush (true);

    streamIndexToMessageMediaType_.clear ();

    audioStreamPacketId_ = 0;
    videoStreamPacketId_ = 0;
  } // end IF

  formatContext_ = avformat_alloc_context ();
  if (unlikely (!formatContext_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avformat_alloc_context() failed, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  IOBuffer_ = av_malloc (STREAM_DEC_DEFAULT_LIBAV_IO_BUFFER_SIZE);
  if (unlikely (!IOBuffer_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_malloc(%u) failed, aborting\n"),
                inherited::mod_->name (),
                STREAM_DEC_DEFAULT_LIBAV_IO_BUFFER_SIZE));
    avformat_free_context (formatContext_);
    return false;
  } // end IF

  IOContext_ = avio_alloc_context (IOBuffer_, 
                                   STREAM_DEC_DEFAULT_LIBAV_IO_BUFFER_SIZE,
                                   0, // Write flag: 0 means read-only
                                   &IOCBData_,
                                   &acestream_libav_mpeg_ts_demuxer_read_cb,
                                   NULL,
                                   NULL);
  if (unlikely (!IOContext_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avio_alloc_context() failed, aborting\n"),
                inherited::mod_->name ()));
    av_free (IOBuffer_); IOBuffer_ = NULL;
    avformat_free_context (formatContext_); formatContext_ = NULL;
    return false;
  } // end IF
  formatContext_->pb = IOContext_;

  const AVInputFormat* input_format_p =
    av_find_input_format (ACE_TEXT_ALWAYS_CHAR ("mpegts"));
  if (unlikely (!input_format_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: av_find_input_format(mpegts) failed, aborting\n"),
                inherited::mod_->name ()));
    avformat_close_input (&formatContext_);
    return false;
  } // end IF

  int result = avformat_open_input (&formatContext_,
                                    NULL,
                                    input_format_p,
                                    NULL);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: avformat_open_input() failed: \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    avformat_close_input (&formatContext_);
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
          typename MediaType>
void
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = queue_.enqueue (message_inout,
                               NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (unlikely (error != ESHUTDOWN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF
  message_inout = NULL;

  return;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      goto end;

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      int result = inherited::activate ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_T::activate(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      goto continue_;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      stop ();

      if (likely (inherited::thr_count_))
        inherited::wait (false); // wait for message queue ?

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
          typename MediaType>
int
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (formatContext_);

  int result;
  struct AVPacket packet_s = {0};
  DataMessageType* message_p = NULL;
  std::vector<int> stream_ids_to_skip_a;
  static ACE_Time_Value backoff_timeout (STREAM_MESSAGE_ALLOCATION_SOURCE_BACKOFF_TIMEOUT_S, 0);

  result = avformat_find_stream_info (formatContext_, NULL);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to avformat_find_stream_info(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  for (unsigned int i = 0; i < formatContext_->nb_streams; i++)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: pre-processing stream %u: codec %d \"%s\" media type...\n"),
                inherited::mod_->name (),
                i,
                formatContext_->streams[i]->codecpar->codec_id,
                ACE_TEXT (avcodec_get_name (formatContext_->streams[i]->codecpar->codec_id))));
  
    switch (formatContext_->streams[i]->codecpar->codec_type)
    {
      case AVMEDIA_TYPE_AUDIO:
      {
        streamIndexToMessageMediaType_[i] = STREAM_MEDIATYPE_AUDIO;

        //if (media_type_s.audio.codecId != AV_CODEC_ID_NONE)
        //{ // only decode one audio/video stream each...
        //  // *TODO*: use av_find_best_stream() ?
        //  ACE_DEBUG ((LM_DEBUG,
        //              ACE_TEXT ("%s: skipping stream %u: codec %d \"%s\"...\n"),
        //              inherited::mod_->name (),
        //              i,
        //              context_->streams[i]->codecpar->codec_id,
        //              ACE_TEXT (avcodec_get_name (context_->streams[i]->codecpar->codec_id))));
        //  stream_ids_to_skip_a.push_back (i);
        //  break;
        //} // end IF
        //media_type_s.audio.codecId = context_->streams[i]->codecpar->codec_id;
        //if (context_->streams[i]->codecpar->extradata_size)
        //{
        //  codec_configuration_s.size =
        //    context_->streams[i]->codecpar->extradata_size;
        //  ACE_NEW_NORETURN (codec_configuration_s.data,
        //                    ACE_UINT8[context_->streams[i]->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE]);
        //  ACE_ASSERT (codec_configuration_s.data);
        //  ACE_OS::memset (codec_configuration_s.data, 0, context_->streams[i]->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
        //  ACE_OS::memcpy (codec_configuration_s.data,
        //                  context_->streams[i]->codecpar->extradata,
        //                  context_->streams[i]->codecpar->extradata_size);
        //  session_data_r.codecConfiguration.insert (std::make_pair (context_->streams[i]->codecpar->codec_id,
        //                                                            codec_configuration_s));
        //} // end IF
        //media_type_s.audio.format = static_cast<enum AVSampleFormat> (context_->streams[i]->codecpar->format);
        //media_type_s.audio.channels = context_->streams[i]->codecpar->ch_layout.nb_channels;
        //media_type_s.audio.sampleRate = context_->streams[i]->codecpar->sample_rate;
        break;
      }
      case AVMEDIA_TYPE_VIDEO:
      {
        streamIndexToMessageMediaType_[i] = STREAM_MEDIATYPE_VIDEO;

//        if (media_type_s.video.codecId != AV_CODEC_ID_NONE)
//        { // only decode one audio/video stream each...
//          // *TODO*: use av_find_best_stream() ?
//          ACE_DEBUG ((LM_DEBUG,
//                      ACE_TEXT ("%s: skipping stream %u: codec %d \"%s\"...\n"),
//                      inherited::mod_->name (),
//                      i,
//                      context_->streams[i]->codecpar->codec_id,
//                      ACE_TEXT (avcodec_get_name (context_->streams[i]->codecpar->codec_id))));
//          stream_ids_to_skip_a.push_back (i);
//          break;
//        } // end IF
//        media_type_s.video.codecId = context_->streams[i]->codecpar->codec_id;
//        if (context_->streams[i]->codecpar->extradata_size)
//        {
//          codec_configuration_s.size =
//            context_->streams[i]->codecpar->extradata_size;
//          ACE_NEW_NORETURN (codec_configuration_s.data,
//                            ACE_UINT8[context_->streams[i]->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE]);
//          ACE_ASSERT (codec_configuration_s.data);
//          ACE_OS::memset (codec_configuration_s.data, 0, context_->streams[i]->codecpar->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
//          ACE_OS::memcpy (codec_configuration_s.data,
//                          context_->streams[i]->codecpar->extradata,
//                          context_->streams[i]->codecpar->extradata_size);
//          session_data_r.codecConfiguration.insert (std::make_pair (context_->streams[i]->codecpar->codec_id,
//                                                                    codec_configuration_s));
//        } // end IF
//        media_type_s.video.format = static_cast<enum AVPixelFormat> (context_->streams[i]->codecpar->format);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//        media_type_s.video.resolution =
//          { context_->streams[i]->codecpar->width,
//            context_->streams[i]->codecpar->height };
//#else
//        media_type_s.video.resolution =
//          { static_cast<unsigned int> (context_->streams[i]->codecpar->width),
//            static_cast<unsigned int> (context_->streams[i]->codecpar->height) };
//#endif // ACE_WIN32 || ACE_WIN64
//        media_type_s.video.frameRate = context_->streams[i]->avg_frame_rate;
        break;
      }
      case AVMEDIA_TYPE_SUBTITLE:
      case AVMEDIA_TYPE_DATA:
      case AVMEDIA_TYPE_ATTACHMENT:
      {
        stream_ids_to_skip_a.push_back (i);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown codec type (was: %d), aborting\n"),
                    inherited::mod_->name (),
                    context_->streams[i]->codecpar->codec_type));
        goto error;
      }
    } // end SWITCH
  } // end FOR
  //session_data_r.formats.push_back (media_type_s);

  av_init_packet (&packet_s);
  do
  {
    result = av_read_frame (formatContext_,
                            &packet_s);
    if (unlikely (result < 0))
    {
      if (likely (result == AVERROR_EOF))
        goto done; // EOF reached
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to av_read_frame(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    if ((inherited::configuration_->streamIndex >= 0 && packet_s.stream_index != inherited::configuration_->streamIndex) ||
        (std::find (stream_ids_to_skip_a.begin (), stream_ids_to_skip_a.end (), packet_s.stream_index) != stream_ids_to_skip_a.end ()))
    {
      av_packet_unref (&packet_s);
      continue;
    } // end IF

    ACE_ASSERT (packet_s.size);
    message_p =
      inherited::allocateMessage (packet_s.size + inherited::configuration_->allocatorConfiguration->paddingBytes,
                                  &backoff_timeout);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  packet_s.size + inherited::configuration_->allocatorConfiguration->paddingBytes));
      av_packet_unref (&packet_s);
      goto error;
    } // end IF
    message_p->size (packet_s.size);

    result = message_p->copy (reinterpret_cast<char*> (packet_s.data),
                              packet_s.size);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  packet_s.size));
      av_packet_unref (&packet_s);
      message_p->release (); message_p = NULL;
      goto error;
    } // end IF
    message_p->setMediaType (streamIndexToMessageMediaType_[packet_s.stream_index]);
    av_packet_unref (&packet_s);

    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      goto error;
    } // end IF
    message_p = NULL;
  } while (true);
  result = -1;

done:
  return result;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

  return -1;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_LibAV_MPEG_TS_Demuxer_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       MediaType>::stop ()
{
  COMMON_TRACE (ACE_TEXT ("Stream_Decoder_LibAV_MPEG_TS_Demuxer_T::stop"));

  // sanity check(s)
  ACE_ASSERT (!queue_.deactivated ());

  ACE_Message_Block* message_block_p = NULL;
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  int result = queue_.enqueue (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}
