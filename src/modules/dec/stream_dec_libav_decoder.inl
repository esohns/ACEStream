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
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::Stream_Decoder_LibAVDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , codecId_ (AV_CODEC_ID_NONE)
 , context_ (NULL)
 , format_ (AV_PIX_FMT_NONE)
 , formatsIndex_ (0)
 , formatHeight_ (0)
 , formatWidth_ (0)
 , frame_ (NULL)
 , frameSize_ (0)
 , outputFormat_ (AV_PIX_FMT_NONE)
 , outputFrameSize_ (0)
 , parserContext_ (NULL)
 , parserPosition_ (0)
 , profile_ (FF_PROFILE_UNKNOWN)
 , transformContext_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::Stream_Decoder_LibAVDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::~Stream_Decoder_LibAVDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::~Stream_Decoder_LibAVDecoder_T"));

  int result = -1;

  if (frame_)
  {
    av_frame_unref (frame_);
    av_frame_free (&frame_);
  } // end IF

  if (context_)
    avcodec_free_context (&context_);

  if (parserContext_)
    av_parser_close (parserContext_);

  if (transformContext_)
    sws_freeContext (transformContext_);
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
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    codecId_ = AV_CODEC_ID_NONE;
    if (context_)
      avcodec_free_context (&context_);
    format_ = AV_PIX_FMT_NONE;
    formatsIndex_ = 0;
    formatHeight_ = 0;
    formatWidth_ = 0;
    if (frame_)
    {
      av_frame_free (&frame_); ACE_ASSERT (!frame_);
    } // end IF
    frameSize_ = 0;
    outputFormat_ = STREAM_DEC_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT;
    outputFrameSize_ = 0;
    if (parserContext_)
    {
      av_parser_close (parserContext_); parserContext_ = NULL;
    } // end IF
    parserPosition_ = 0;
    profile_ = FF_PROFILE_UNKNOWN;
    if (transformContext_)
    {
      sws_freeContext (transformContext_); transformContext_ = NULL;
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

  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_s);
  outputFormat_ = media_type_s.format;
  if (unlikely (outputFormat_ == AV_PIX_FMT_NONE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid output format, aborting\n"),
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
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (context_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  DataMessageType* message_p = NULL;
  struct AVPacket packet_s;
  ACE_Message_Block* message_block_p = NULL;
  uint8_t* data_p = NULL;
  size_t   data_size_i = 0;
  bool abort_session_on_error = true;

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
  { if (((message_block_p->capacity () - message_block_p->length ()) >= AV_INPUT_BUFFER_PADDING_SIZE))
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
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Decoder_LibAVDecoder_T::decodePacket(), aborting\n"),
                    inherited::mod_->name ()));
        abort_session_on_error = false; // do not abort the whole session; retry
        goto error;
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

  if (abort_session_on_error)
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
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::handleSessionMessage"));

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
      //int debug_i = FF_DEBUG_PICT_INFO | FF_DEBUG_RC | FF_DEBUG_BITSTREAM |
      //              FF_DEBUG_MB_TYPE | FF_DEBUG_QP;
      int debug_i = FF_DEBUG_PICT_INFO;

      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      MediaType media_type_2;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      formatHeight_ =
          static_cast<unsigned int> (std::abs (media_type_s.resolution.cy));
      formatWidth_ = static_cast<unsigned int> (media_type_s.resolution.cx);
#else
      formatHeight_ = media_type_s.resolution.height;
      formatWidth_ = media_type_s.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64

      outputFrameSize_ =
        av_image_get_buffer_size (outputFormat_,
                                  static_cast<int> (formatWidth_),
                                  static_cast<int> (formatHeight_),
                                  1); // *TODO*: linesize alignment

      if (codecId_ == AV_CODEC_ID_NONE)
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: codec id not set, best-guessing based on the input pixel format (was: %s)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_s.format).c_str ())));
        if (Stream_Module_Decoder_Tools::isCompressedVideo (media_type_s.format))
          codecId_ =
            Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId (media_type_s.format);
//        else
//          codecId_ = AV_CODEC_ID_RAWVIDEO;
      } // end IF
      if (codecId_ == AV_CODEC_ID_NONE)
        break;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: using codec \"%s\" (id: %d)\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (avcodec_get_name (codecId_)), codecId_));
      //profile_ = configuration_in.codecProfile;

      int result = -1;
      const struct AVCodec* codec_p = NULL;
      struct AVDictionary* dictionary_p = NULL;
      int flags, flags2;
//      unsigned int buffer_size = 0;

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
        //parserContext_->flags |= PARSER_FLAG_COMPLETE_FRAMES;
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct AVCodecParameters* codec_parameters_p =
        avcodec_parameters_alloc ();
      if (unlikely (!codec_parameters_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avcodec_parameters_alloc(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      codec_parameters_p->codec_type = AVMEDIA_TYPE_VIDEO;
      codec_parameters_p->codec_id = codecId_;
      //codec_parameters_p->codec_tag = ;
      //codec_parameters_p->extradata = NULL;
      //codec_parameters_p->extradata_size = 0;
      codec_parameters_p->format = outputFormat_;
      //codec_parameters_p->bit_rate = 200000;
      //codec_parameters_p->bits_per_coded_sample = 0;
      //codec_parameters_p->bits_per_raw_sample = 0;
      //codec_parameters_p->profile = FF_PROFILE_H264_HIGH;
      //codec_parameters_p->level = ;
      codec_parameters_p->width = formatWidth_;
      codec_parameters_p->height = formatHeight_;
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
      //        AV_CODEC_FLAG_4MV            |
              AV_CODEC_FLAG_OUTPUT_CORRUPT |
              AV_CODEC_FLAG_QPEL           |
              //AV_CODEC_FLAG_DROPCHANGED    |
              //AV_CODEC_FLAG_RECON_FRAME    |
              //AV_CODEC_FLAG_COPY_OPAQUE    |
              //AV_CODEC_FLAG_FRAME_DURATION |
              //AV_CODEC_FLAG_PASS1          |
              //AV_CODEC_FLAG_PASS2          |
              AV_CODEC_FLAG_LOOP_FILTER    |
              //AV_CODEC_FLAG_GRAY           |
              //AV_CODEC_FLAG_PSNR           |
              //AV_CODEC_FLAG_INTERLACED_DCT |
              AV_CODEC_FLAG_LOW_DELAY      |
              //AV_CODEC_FLAG_GLOBAL_HEADER  |
              AV_CODEC_FLAG_BITEXACT;//       |
              //AV_CODEC_FLAG_AC_PRED        |
              //AV_CODEC_FLAG_INTERLACED_ME  |
              //AV_CODEC_FLAG_CLOSED_GOP;

      flags2 = AV_CODEC_FLAG2_FAST          |
      //         AV_CODEC_FLAG2_NO_OUTPUT           |
      //         AV_CODEC_FLAG2_LOCAL_HEADER        |
      //         AV_CODEC_FLAG2_DROP_FRAME_TIMECODE |
               AV_CODEC_FLAG2_CHUNKS        |
               AV_CODEC_FLAG2_IGNORE_CROP   |
               AV_CODEC_FLAG2_SHOW_ALL      |
               AV_CODEC_FLAG2_EXPORT_MVS    |
               AV_CODEC_FLAG2_SKIP_MANUAL;
      // AV_CODEC_FLAG2_RO_FLUSH_NOOP
#else
//      flags = CODEC_FLAG_UNALIGNED      |
//              CODEC_FLAG_QSCALE         |
//              CODEC_FLAG_OUTPUT_CORRUPT |
//              CODEC_FLAG_QPEL           |
//              //CODEC_FLAG_PASS1          |
//              //CODEC_FLAG_PASS2          |
//              CODEC_FLAG_LOOP_FILTER    |
//              //CODEC_FLAG_GRAY           |
////              CODEC_FLAG_TRUNCATED      |
//              //CODEC_FLAG_INTERLACED_DCT |
//              CODEC_FLAG_LOW_DELAY      |
//              //CODEC_FLAG_GLOBAL_HEADER  |
//              CODEC_FLAG_BITEXACT;//       |
//      //CODEC_FLAG_INTERLACED_ME  |
//      //CODEC_FLAG_CLOSED_GOP;
      flags = AV_CODEC_FLAG_UNALIGNED      |
              AV_CODEC_FLAG_QSCALE         |
      //        AV_CODEC_FLAG_4MV            |
              AV_CODEC_FLAG_OUTPUT_CORRUPT |
              AV_CODEC_FLAG_QPEL           |
              //AV_CODEC_FLAG_PASS1          |
              //AV_CODEC_FLAG_PASS2          |
              AV_CODEC_FLAG_LOOP_FILTER    |
              //AV_CODEC_FLAG_GRAY           |
              //AV_CODEC_FLAG_PSNR           |
//              AV_CODEC_FLAG_TRUNCATED      |
              //AV_CODEC_FLAG_INTERLACED_DCT |
              AV_CODEC_FLAG_LOW_DELAY      |
              //AV_CODEC_FLAG_GLOBAL_HEADER  |
              AV_CODEC_FLAG_BITEXACT;//       |
              //AV_CODEC_FLAG_AC_PRED        |
      //AV_CODEC_FLAG_INTERLACED_ME  |
      //AV_CODEC_FLAG_CLOSED_GOP;
//      if (codec_p->capabilities & CODEC_CAP_TRUNCATED)
//        flags |= CODEC_FLAG_TRUNCATED;
//      if (codec_p->capabilities & AV_CODEC_CAP_TRUNCATED)
//        flags |= AV_CODEC_FLAG_TRUNCATED;

//      flags2 = CODEC_FLAG2_FAST          |
//               CODEC_FLAG2_CHUNKS        |
//               CODEC_FLAG2_IGNORE_CROP   |
//               CODEC_FLAG2_SHOW_ALL      |
//               CODEC_FLAG2_EXPORT_MVS    |
//               CODEC_FLAG2_SKIP_MANUAL;
      flags2 = AV_CODEC_FLAG2_FAST          |
      //         AV_CODEC_FLAG2_NO_OUTPUT           |
      //         AV_CODEC_FLAG2_LOCAL_HEADER        |
      //         AV_CODEC_FLAG2_DROP_FRAME_TIMECODE |
               AV_CODEC_FLAG2_CHUNKS        |
               AV_CODEC_FLAG2_IGNORE_CROP   |
               AV_CODEC_FLAG2_SHOW_ALL      |
               AV_CODEC_FLAG2_EXPORT_MVS    |
               AV_CODEC_FLAG2_SKIP_MANUAL;
#endif // ACE_WIN32 || ACE_WIN64
      format_ = outputFormat_; // try
      context_->opaque = &format_;
      //context_->bit_rate = bit_rate;
      context_->flags = flags;
      context_->flags2 = flags2;
      if (session_data_r.codecConfigurationDataSize)
      { ACE_ASSERT (session_data_r.codecConfigurationData);
        ACE_ASSERT (!context_->extradata);
        context_->extradata =
          static_cast<uint8_t*> (av_malloc (session_data_r.codecConfigurationDataSize + AV_INPUT_BUFFER_PADDING_SIZE));
        if (!context_->extradata)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_OS::memcpy (context_->extradata,
                        session_data_r.codecConfigurationData,
                        session_data_r.codecConfigurationDataSize);
        context_->extradata_size = session_data_r.codecConfigurationDataSize;
      } // end IF
      ACE_ASSERT (media_type_s.frameRate.num);
      ACE_ASSERT (media_type_s.frameRate.den == 1);
      context_->time_base = {1, media_type_s.frameRate.num};
      context_->ticks_per_frame =
        (((codecId_ == AV_CODEC_ID_H264) ||
          (codecId_ == AV_CODEC_ID_MPEG2VIDEO)) ? 2 : 1);
      context_->width = formatWidth_;
      context_->height = formatHeight_;
      //context_->coded_width = width;
      //context_->coded_height = height;
//      context_->pix_fmt = AV_PIX_FMT_NONE;
      context_->pix_fmt = outputFormat_;
      //context_->draw_horiz_band = NULL;
      context_->get_format = stream_decoder_libav_getformat_cb;
//      context_->slice_count = 0;
//      context_->slice_offset = NULL;
//      context_->slice_flags = 0;
//      context_->skip_top = 0;
//      context_->skip_bottom = 0;
//      context_->field_order = AV_FIELD_UNKNOWN;
      //context_->get_buffer2 = NULL;
//      context_->refcounted_frames = 0;
      //context_->rc_max_rate = bit_rate;
//      context_->workaround_bugs = FF_BUG_AUTODETECT;
//      context_->strict_std_compliance = FF_COMPLIANCE_NORMAL;
//      context_->error_concealment = 3;
#if defined (_DEBUG)
      context_->debug = (inherited::configuration_->debug ? debug_i : 0);
#endif // _DEBUG
//      context_->debug_mv = (inherited::configuration_->debug ? 1 : 0);
//      context_->err_recognition = 0;
      //context_->reordered_opaque = 0;
//      context_->hwaccel_context = NULL;
//      context_->idct_algo = FF_IDCT_AUTO;
//      context_->bits_per_coded_sample = 0;
//      context_->lowres = 0;
      context_->thread_count = 1;
      // *TODO*: support multithreaded decoding ?
      //context_->thread_count = Common_Tools::getNumberOfCPUs (true);
      //context_->thread_type = 3;
      //context_->thread_safe_callbacks = 0;
      //context_->execute = NULL;
      //context_->execute2 = NULL;
//      context_->skip_loop_filter = AVDISCARD_DEFAULT;
//      context_->skip_idct = AVDISCARD_DEFAULT;
//      context_->skip_frame = AVDISCARD_DEFAULT;
//      context_->pkt_timebase.num = 0;
//      context_->pkt_timebase.den = 1;
      //context_->sub_charenc = NULL;
//      context_->skip_alpha = 0;
      //context_->dump_separator = NULL;
//      context_->codec_whitelist = NULL;
      //context_->hw_frames_ctx = NULL;
//      context_->sub_text_format = FF_SUB_TEXT_FMT_ASS;
      //context_->max_pixels = 2147483647;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = avcodec_parameters_to_context (context_,
                                              codec_parameters_p);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_parameters_to_context() failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      avcodec_parameters_free (&codec_parameters_p); codec_parameters_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
//      ACE_ASSERT (context_->pix_fmt == AV_PIX_FMT_NONE);

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
//      ACE_ASSERT (context_->pix_fmt != AV_PIX_FMT_NONE);
      av_dict_free (&dictionary_p); dictionary_p = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: initialized codec %s; decoded pixel format: %s\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (avcodec_get_name (codecId_)),
                  ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (context_->pix_fmt).c_str ())));

      frameSize_ =
        av_image_get_buffer_size (context_->pix_fmt,
                                  formatWidth_,
                                  formatHeight_,
                                  1); // *TODO*: linesize alignment
//      ACE_ASSERT (frameSize_ != 4294967274);

      if (context_->pix_fmt != outputFormat_)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: converting decoded pixel format %s to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (context_->pix_fmt).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (outputFormat_).c_str ())));

        flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                 SWS_LANCZOS | SWS_ACCURATE_RND | SWS_BITEXACT);
        transformContext_ =
            sws_getCachedContext (NULL,
                                  formatWidth_, formatHeight_, context_->pix_fmt,
                                  formatWidth_, formatHeight_, outputFormat_,
                                  flags,                        // flags
                                  NULL, NULL,                   // filters
                                  0);                           // parameters
        if (unlikely (!transformContext_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
          goto error;
        } // end IF
      } // end IF
      format_ = context_->pix_fmt;

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
        inherited2::setFramerate (media_type_s.frameRate,
                                  media_type_2);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          session_data_r.formats.push_back (media_type_2);
        } // end lock scope
      } // end IF
      else
        inherited2::free_ (media_type_2);
      formatsIndex_ =
        static_cast<unsigned int> (session_data_r.formats.size () - 1);

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
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      MediaType media_type_s;

      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (session_data_r.lock);
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        if ((session_data_r.formats.size () - 1) > formatsIndex_)
        {
          typename SessionDataContainerType::DATA_T::MEDIAFORMATS_ITERATOR_T iterator =
            session_data_r.formats.begin ();
          std::advance (iterator, formatsIndex_ + 1);
          session_data_r.formats.erase (iterator, session_data_r.formats.end ());
        } // end IF
        ACE_ASSERT (!session_data_r.formats.empty ());
        inherited2::getMediaType (session_data_r.formats.back (),
                                  STREAM_MEDIATYPE_VIDEO,
                                  media_type_2);
      } // end lock scope

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Common_Image_Resolution_t resolution_s = { static_cast<LONG> (formatWidth_),
                                                 static_cast<LONG> (formatHeight_) };
#else
      Common_Image_Resolution_t resolution_s = {formatWidth_, formatHeight_};
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::setFormat (outputFormat_,
                             media_type_s);
      inherited2::setResolution (resolution_s,
                                 media_type_s);
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.formats.push_back (media_type_s);
        formatsIndex_ =
          static_cast<unsigned int> (session_data_r.formats.size () - 1);
      } // end lock scope

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
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::decodePacket (struct AVPacket& packet_in,
                                                        DataMessageType*& message_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::decodePacket"));

  // sanity check(s)
  ACE_ASSERT (!message_inout);
  ACE_ASSERT (context_);
  ACE_ASSERT (frame_);
  ACE_ASSERT (frameSize_);

  int result = avcodec_send_packet (context_,
                                    &packet_in);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to avcodec_send_packet(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  // pixel format/resolution may have changed
  if (unlikely ((context_->pix_fmt != format_)      ||
                (context_->width != static_cast<int> (formatWidth_))   ||
                (context_->height != static_cast<int> (formatHeight_))))
  {
    if (transformContext_)
    {
      sws_freeContext (transformContext_); transformContext_ = NULL;
    } // end IF

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reinit occurred; converting decoded pixel format %s to %s (@ %ux%u)\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (context_->pix_fmt).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (outputFormat_).c_str ()),
                context_->width, context_->height));

    int flags = (SWS_FAST_BILINEAR); // interpolation
    transformContext_ =
      sws_getCachedContext (NULL,
                            context_->width, context_->height, context_->pix_fmt,
                            context_->width, context_->height, outputFormat_,
                            flags,                        // flags
                            NULL, NULL,                   // filters
                            0);                           // parameters
    if (unlikely (!transformContext_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
      return false;
    } // end IF

    bool send_resize_b = ((context_->width != static_cast<int> (formatWidth_)) ||
                          (context_->height != static_cast<int> (formatHeight_)));
    format_ = context_->pix_fmt;
    formatWidth_ = context_->width;
    formatHeight_ = context_->height;
    frameSize_ =
      av_image_get_buffer_size (context_->pix_fmt,
                                context_->width,
                                context_->height,
                                1); // *TODO*: linesize alignment
    outputFrameSize_ =
      av_image_get_buffer_size (outputFormat_,
                                static_cast<int> (context_->width),
                                static_cast<int> (context_->height),
                                1); // *TODO*: linesize alignment

    // update session data
    ACE_ASSERT (inherited::sessionData_);
    typename SessionDataContainerType::DATA_T& session_data_r =
      const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
    MediaType media_type_2 = session_data_r.formats.back ();
    inherited2::setFormat (outputFormat_,
                           media_type_2);
    Common_Image_Resolution_t resolution_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    resolution_s.cx = context_->width;
    resolution_s.cy = context_->height;
#else
    resolution_s.width = context_->width;
    resolution_s.height = context_->height;
#endif // ACE_WIN32 || ACE_WIN64
    inherited2::setResolution (resolution_s,
                               media_type_2);
    ACE_ASSERT (session_data_r.lock);
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock, false);
      session_data_r.formats.push_back (media_type_2);
    } // end lock scope

    // send resize notification ?
    if (send_resize_b)
    {
      inherited::sessionData_->increase ();
      SessionDataContainerType* session_data_container_p =
        inherited::sessionData_;
      if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_RESIZE,
                                                   session_data_container_p,
                                                   NULL,
                                                   true))) // expedited ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                    inherited::mod_->name (),
                    STREAM_SESSION_MESSAGE_RESIZE));
    } // end IF
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

  // convert pixel format of the decoded frame ?
  if (transformContext_)
  { ACE_ASSERT (outputFormat_ != AV_PIX_FMT_NONE);
    int line_sizes_a[AV_NUM_DATA_POINTERS];
    ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
    uint8_t* data_a[AV_NUM_DATA_POINTERS];
    ACE_OS::memset (&data_a, 0, sizeof (uint8_t* [AV_NUM_DATA_POINTERS]));

    message_block_p = inherited::allocateMessage (outputFrameSize_);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  outputFrameSize_));
      av_frame_unref (frame_);
      return false;
    } // end IF

    result =
        av_image_fill_linesizes (line_sizes_a,
                                 outputFormat_,
                                 static_cast<int> (frame_->width));
    ACE_ASSERT (result >= 0);
    result =
        av_image_fill_pointers (data_a,
                                outputFormat_,
                                static_cast<int> (frame_->height),
                                reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                line_sizes_a);
    ACE_ASSERT (result >= 0);
    if (unlikely (!Stream_Module_Decoder_Tools::convert (transformContext_,
                                                         context_->width, context_->height, context_->pix_fmt,
                                                         static_cast<uint8_t**> (frame_->data),
                                                         context_->width, context_->height, outputFormat_,
                                                         data_a)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), aborting\n"),
                  inherited::mod_->name ()));
      av_frame_unref (frame_);
      message_block_p->release ();
      return false;
    } // end IF
    message_block_p->wr_ptr (outputFrameSize_);

//#if defined (_DEBUG)
//    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
//    if (!Common_File_Tools::store (filename_string,
//                                   data_a[0],
//                                   frameSize_))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), aborting\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      av_frame_unref (frame_);
//      message_block_p->release ();
//      return false;
//    }  // end IF
//#endif // _DEBUG
  } // end IF
  else
  {
    message_block_p = inherited::allocateMessage (frameSize_);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  frameSize_));
      av_frame_unref (frame_);
      return false;
    } // end IF
    message_block_p->base (reinterpret_cast<char*> (frame_->data),
                           frameSize_,
                           0); // own image data
    message_block_p->wr_ptr (frameSize_);

//#if defined (_DEBUG)
//    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.yuv");
//    if (!Common_File_Tools::store (filename_string,
//                                   static_cast<uint8_t*> (frame_->data[0]),
//                                   frameSize_))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), aborting\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      av_frame_unref (frame_);
//      return false;
//    }  // end IF
//#endif // _DEBUG
  } // end ELSE
  ACE_ASSERT (message_block_p);
  message_inout = static_cast<DataMessageType*> (message_block_p);

  // clean up
  ACE_OS::memset (frame_->data, 0, sizeof (uint8_t * [8]));
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
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::drainBuffers (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::drainBuffers"));

  // sanity check(s)
  ACE_ASSERT (context_);
  ACE_ASSERT (frame_);

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
        return;
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
    { ACE_ASSERT (outputFormat_ != AV_PIX_FMT_NONE);
      int line_sizes_a[AV_NUM_DATA_POINTERS];
      ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
      uint8_t* data_a[AV_NUM_DATA_POINTERS];
      ACE_OS::memset (&data_a, 0, sizeof (uint8_t* [AV_NUM_DATA_POINTERS]));

      message_block_p = inherited::allocateMessage (outputFrameSize_);
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                    inherited::mod_->name (),
                    outputFrameSize_));
        av_frame_unref (frame_);
        return;
      } // end IF

      result =
          av_image_fill_linesizes (line_sizes_a,
                                   outputFormat_,
                                   static_cast<int> (frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (data_a,
                                  outputFormat_,
                                  static_cast<int> (frame_->height),
                                  reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                  line_sizes_a);
      ACE_ASSERT (result >= 0);
      if (unlikely (!Stream_Module_Decoder_Tools::convert (transformContext_,
                                                           context_->width, context_->height, context_->pix_fmt,
                                                           static_cast<uint8_t**> (frame_->data),
                                                           context_->width, context_->height, outputFormat_,
                                                           data_a)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                    inherited::mod_->name ()));
        av_frame_unref (frame_);
        message_block_p->release ();
        return;
      } // end IF
      message_block_p->wr_ptr (outputFrameSize_);

  //#if defined (_DEBUG)
  //    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
  //    if (!Common_File_Tools::store (filename_string,
  //                                   data[0],
  //                                   frameSize_))
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
  //                  ACE_TEXT (filename_string.c_str ())));
  //      goto error;
  //    } // end IF
  //#endif // _DEBUG
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

  //#if defined (_DEBUG)
  //    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.yuv");
  //    if (!Common_Image_Tools::storeToFile (context_->width, context_->height, context_->pix_fmt,
  //                                          static_cast<uint8_t**> (frame_->data),
  //                                          filename_string))
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to Common_Image_Tools::storeToFile(\"%s\"), returning\n"),
  //                  ACE_TEXT (filename_string.c_str ())));
  //      goto error;
  //    } // end IF
  //#endif // _DEBUG
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
}
