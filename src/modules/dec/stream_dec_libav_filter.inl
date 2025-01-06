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
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/channel_layout.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
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
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::Stream_Decoder_LibAVFilter_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , bufferSourceContext_ (NULL)
 , bufferFormatContext_ (NULL)
 , bufferSinkContext_ (NULL)
 , filterGraph_ (NULL)
 , frame_ (NULL)
 , frame_2 (NULL)
 , frameSize_ (0)
 , outputFrameSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::Stream_Decoder_LibAVFilter_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::~Stream_Decoder_LibAVFilter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::~Stream_Decoder_LibAVFilter_T"));

  if (filterGraph_)
    avfilter_graph_free (&filterGraph_);

  if (frame_)
  {
    av_frame_unref (frame_);
    av_frame_free (&frame_);
  } // end IF
  if (frame_2)
  {
    av_frame_unref (frame_2);
    av_frame_free (&frame_2);
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
bool
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (filterGraph_)
      avfilter_graph_free (&filterGraph_);

    if (frame_)
    {
      av_frame_unref (frame_);
      av_frame_free (&frame_); ACE_ASSERT (!frame_);
    } // end IF
    if (frame_2)
    {
      av_frame_unref (frame_2);
      av_frame_free (&frame_2);
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

  filterGraph_ = avfilter_graph_alloc ();
  ACE_ASSERT (filterGraph_);

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
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  DataMessageType* message_p = NULL;
  struct AVPacket packet_s;
  ACE_Message_Block* message_block_p = NULL;
  uint8_t* data_p = NULL;
  size_t   data_size_i = 0;
  bool abort_session_on_error = true;

  message_block_p = message_inout;
  do
  {
    /* use the parser to split the data into frames */
    data_p = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());
    data_size_i = message_block_p->length ();
    while (data_size_i > 0)
    {
      av_init_packet (&packet_s);

      packet_s.data = data_p;
      packet_s.size = static_cast<int> (data_size_i);
      data_size_i = 0;
      if (!packet_s.size)
        continue;

      ACE_ASSERT (!message_p);
      if (!filterPacket (packet_s,
                         message_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Decoder_LibAVFilter_T::filterPacket(), aborting\n"),
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
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::handleSessionMessage"));

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
      struct Stream_MediaFramework_FFMPEG_AudioMediaType media_type_2;
      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_2);
      MediaType media_type_3;
      ACE_OS::memset (&media_type_3, 0, sizeof (MediaType));

      char args_a[BUFSIZ];
      const AVFilter* filter_p = avfilter_get_by_name (ACE_TEXT_ALWAYS_CHAR ("abuffer"));
      ACE_ASSERT (filter_p);
      const AVFilter* filter_2 = avfilter_get_by_name (ACE_TEXT_ALWAYS_CHAR ("aformat"));
      ACE_ASSERT (filter_2);
      const AVFilter* filter_3 = avfilter_get_by_name (ACE_TEXT_ALWAYS_CHAR ("abuffersink"));
      ACE_ASSERT (filter_3);

      AVFilterInOut* outputs_p = avfilter_inout_alloc ();
      ACE_ASSERT (outputs_p);
      AVFilterInOut* inputs_p = avfilter_inout_alloc ();
      ACE_ASSERT (inputs_p);

      //enum AVSampleFormat out_sample_fmts[] = { media_type_2.format, (enum AVSampleFormat)-1 };
      //struct AVChannelLayout channel_layout_s;
      //av_channel_layout_default (&channel_layout_s, media_type_2.channels);
      //uint64_t out_channel_layouts[] = { channel_layout_s.u.mask, -1};
      //int out_sample_rates[] = { media_type_2.sampleRate, -1};
      //AVFilterLink* outlink_p = NULL;

      frameSize_ =
        av_get_bytes_per_sample (media_type_s.format) * media_type_s.channels;
      outputFrameSize_ =
        av_get_bytes_per_sample (media_type_2.format) * media_type_2.channels;

      /* buffer audio source: the decoded frames from the decoder will be inserted here. */
      ACE_OS::snprintf (args_a, sizeof (char[BUFSIZ]),
                        ACE_TEXT_ALWAYS_CHAR ("time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%llx"),
                        1, media_type_s.sampleRate,
                        media_type_s.sampleRate,
                        av_get_sample_fmt_name (media_type_s.format),
                        Stream_Module_Decoder_Tools::channelsToMask (media_type_s.channels));
      int result = avfilter_graph_create_filter (&bufferSourceContext_,
                                                 filter_p,
                                                 ACE_TEXT_ALWAYS_CHAR ("in"),
                                                 args_a,
                                                 NULL,
                                                 filterGraph_);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avfilter_graph_create_filter(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      /* format audio: make sure the output format is what is requested */
      ACE_OS::snprintf (args_a, sizeof (char[BUFSIZ]),
                        ACE_TEXT_ALWAYS_CHAR ("sample_rates=%d:sample_fmts=%s:channel_layouts=0x%llx"),
                        media_type_2.sampleRate,
                        av_get_sample_fmt_name (media_type_2.format),
                        Stream_Module_Decoder_Tools::channelsToMask (media_type_2.channels));
      result = avfilter_graph_create_filter (&bufferFormatContext_,
                                             filter_2,
                                             ACE_TEXT_ALWAYS_CHAR ("format"),
                                             args_a,
                                             NULL,
                                             filterGraph_);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avfilter_graph_create_filter(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      /* buffer audio sink: to terminate the filter chain. */
      result = avfilter_graph_create_filter (&bufferSinkContext_,
                                             filter_3,
                                             ACE_TEXT_ALWAYS_CHAR ("out"),
                                             NULL,
                                             NULL,
                                             filterGraph_);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avfilter_graph_create_filter(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      /* Connect the filters;
       * in this simple case the filters just form a linear chain. */
      result = avfilter_link (bufferSourceContext_, 0, bufferFormatContext_, 0);
      ACE_ASSERT (result >= 0);
      result = avfilter_link (bufferFormatContext_, 0, bufferSinkContext_, 0);
      ACE_ASSERT (result >= 0);

      ///* Endpoints for the filter graph. */
      //outputs_p->name       = av_strdup (ACE_TEXT_ALWAYS_CHAR ("in"));
      //outputs_p->filter_ctx = bufferSourceContext_;
      //outputs_p->pad_idx    = 0;
      //outputs_p->next       = NULL;
      //inputs_p->name        = av_strdup (ACE_TEXT_ALWAYS_CHAR ("out"));
      //inputs_p->filter_ctx  = bufferSinkContext_;
      //inputs_p->pad_idx     = 0;
      //inputs_p->next        = NULL;

      //result =
      //  avfilter_graph_parse (filterGraph_,
      //                        inherited::configuration_->filtersDescription.c_str (),
      //                        inputs_p,
      //                        outputs_p,
      //                        NULL);
      //if (unlikely (result < 0))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to avfilter_graph_parse(\"%s\"): \"%s\", aborting\n"),
      //              inherited::mod_->name (),
      //              ACE_TEXT (inherited::configuration_->filtersDescription.c_str ()),
      //              ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      //  goto error;
      //} // end IF
      avfilter_inout_free (&inputs_p);
      avfilter_inout_free (&outputs_p);

      result = avfilter_graph_config (filterGraph_,
                                      NULL);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to avfilter_graph_config(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      /* Print summary of the sink buffer
       * Note: args_a buffer is reused to store channel layout string */
      //outlink_p = bufferSinkContext_->inputs[0];
      //result = av_channel_layout_describe (&outlink_p->ch_layout,
      //                                     args_a, sizeof (char[BUFSIZ]));
      //if (unlikely (result < 0))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to av_channel_layout_describe(): \"%s\", aborting\n"),
      //              inherited::mod_->name (),
      //              ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      //  goto error;
      //} // end IF
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: output: srate:%dHz fmt:%s chlayout:%s\n"),
      //            inherited::mod_->name (),
      //            outlink_p->sample_rate,
      //            ACE_TEXT (av_x_if_null (av_get_sample_fmt_name ((enum AVSampleFormat)outlink_p->format), ACE_TEXT_ALWAYS_CHAR ("?"))),
      //            ACE_TEXT (args_a)));

      ACE_ASSERT (!frame_);
      frame_ = av_frame_alloc ();
      if (unlikely (!frame_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to av_frame_alloc(): \"%m\", aborting\n")));
        goto error;
      } // end IF
      av_channel_layout_default (&frame_->ch_layout,
                                 media_type_s.channels);
      frame_->sample_rate = media_type_s.sampleRate;
      frame_->format = media_type_s.format;

      frame_2 = av_frame_alloc ();
      if (unlikely (!frame_2))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to av_frame_alloc(): \"%m\", aborting\n")));
        goto error;
      } // end IF

      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_3);
      session_data_r.formats.push_back (media_type_3);

      goto continue_;

error:
      avfilter_inout_free (&inputs_p);
      avfilter_inout_free (&outputs_p);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (likely (frame_))
        drainBuffers (message_inout->sessionId ());

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
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::filterPacket (struct AVPacket& packet_in,
                                                       DataMessageType*& message_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::filterPacket"));

  // sanity check(s)
  ACE_ASSERT (!message_inout);
  ACE_ASSERT (frame_);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  /* push the audio data from decoded frame into the filtergraph */  
  frame_->data[0] = packet_in.data;
  frame_->nb_samples = packet_in.size / frameSize_;
  result = av_buffersrc_add_frame_flags (bufferSourceContext_,
                                         frame_,
                                         0);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to av_buffersrc_add_frame_flags(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  /* pull filtered audio from the filtergraph */
  while (true)
  {
    result = av_buffersink_get_frame (bufferSinkContext_,
                                      frame_2);
    if (result == AVERROR (EAGAIN) || result == AVERROR_EOF)
    {
      frame_->data[0] = NULL;
      frame_->nb_samples = 0;

      return true; // no more data
    } // end IF
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to av_buffersink_get_frame(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Image_Tools::errorToString (result).c_str ())));
      break;
    } // end IF
    ACE_ASSERT (frame_2->data[0]);
    ACE_ASSERT (frame_2->nb_samples);

    // --> successfully filtered some frames

    message_block_p =
      inherited::allocateMessage (frame_2->nb_samples * outputFrameSize_);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  frame_2->nb_samples * outputFrameSize_));
      av_frame_unref (frame_2);
      break;
    } // end IF
    //message_block_p->base (reinterpret_cast<char*> (frame_2->data[0]),
    //                       frame_2->nb_samples * frameSize_,
    //                       0); // own image data
    //message_block_p->wr_ptr (frame_2->nb_samples * frameSize_);
    result =
      message_block_p->copy (reinterpret_cast<char*> (frame_2->data[0]),
                             frame_2->nb_samples * outputFrameSize_);
    ACE_ASSERT (result != -1);

    ACE_ASSERT (message_block_p);
    if (!message_inout)
      message_inout = static_cast<DataMessageType*> (message_block_p);
    else
    { // append
      ACE_Message_Block* message_block_2 = message_inout;
      while (message_block_2->cont ())
        message_block_2 = message_block_2->cont ();
      message_block_2->cont (message_block_p);
    } // end ELSE

    // clean up
    //ACE_OS::memset (frame_2->data, 0, sizeof (uint8_t*[8]));
    av_frame_unref (frame_2);
  } // end WHILE

  frame_->data[0] = NULL;
  frame_->nb_samples = 0;

  return false;
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
Stream_Decoder_LibAVFilter_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::drainBuffers (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVFilter_T::drainBuffers"));

  // sanity check(s)
  ACE_ASSERT (frame_);
}
