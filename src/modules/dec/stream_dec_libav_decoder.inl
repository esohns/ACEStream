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

#include "common_image_tools.h"
#endif // _DEBUG

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

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
          typename SessionDataContainerType>
char
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
char
Stream_Decoder_LibAVDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
/*                              SessionDataContainerType>::paddingBuffer[FF_INPUT_BUFFER_PADDING_SIZE];*/
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              SessionDataContainerType>::Stream_Decoder_LibAVDecoder_T (ISTREAM_T* stream_in)
#else
                              SessionDataContainerType>::Stream_Decoder_LibAVDecoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , buffer_ (NULL)
// , buffer_ ()
// , bufferRef_ ()
 , codecId_ (AV_CODEC_ID_NONE)
 , context_ (NULL)
 , format_ (AV_PIX_FMT_NONE)
 , formatHeight_ (0)
 , frame_ (NULL)
 , frameSize_ (0)
 , outputFormat_ (STREAM_DEC_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT)
 , outputFrameSize_ (0)
 , profile_ (FF_PROFILE_UNKNOWN)
 , transformContext_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::Stream_Decoder_LibAVDecoder_T"));

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

  if (buffer_)
    buffer_->release ();

  if (context_)
  {
    result = avcodec_close (context_);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
    avcodec_free_context (&context_);
  } // end IF

  if (frame_)
    av_frame_free (&frame_);

  if (transformContext_)
    sws_freeContext (transformContext_);
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

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF

    codecId_ = AV_CODEC_ID_NONE;
    if (context_)
    {
      result = avcodec_close (context_);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      avcodec_free_context (&context_);
    } // end IF
    format_ = AV_PIX_FMT_NONE;
    formatHeight_ = 0;
    if (frame_)
    {
      av_frame_free (&frame_); frame_ = NULL;
    } // end IF
    frameSize_ = 0;
    outputFormat_ = STREAM_DEC_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT;
    outputFrameSize_ = 0;
    profile_ = FF_PROFILE_UNKNOWN;
    if (transformContext_)
    {
      sws_freeContext (transformContext_); transformContext_ = NULL;
    } // end IF
  } // end IF

#if defined (_DEBUG)
  //av_log_set_callback (Stream_Decoder_LibAVDecoder_LoggingCB);
  // *NOTE*: this level logs all messages
  //av_log_set_level (std::numeric_limits<int>::max ());
#endif // _DEBUG^^^
  av_register_all ();
//  avcodec_register_all ();

  // *TODO*: remove type inferences
  codecId_ = configuration_in.codecId;

  frame_ = av_frame_alloc ();
  if (unlikely (!frame_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  outputFormat_ = getFormat (configuration_in.outputFormat);
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

  // sanity check(s)
  if (unlikely (codecId_ == AV_CODEC_ID_NONE))
    return; // nothing to do
  ACE_ASSERT (context_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  unsigned int padding_bytes =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    AV_INPUT_BUFFER_PADDING_SIZE;
#else
      AV_INPUT_BUFFER_PADDING_SIZE;
//    FF_INPUT_BUFFER_PADDING_SIZE;
#endif // ACE_WIN32 || ACE_WIN64
  DataMessageType* message_p = NULL;
  int got_frame = 0;
  struct AVPacket packet_s;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  ACE_Message_Block* message_block_3 = NULL;
  int line_sizes[AV_NUM_DATA_POINTERS];
  uint8_t* data[AV_NUM_DATA_POINTERS];

  // *NOTE*: apparently (the implementation is of the finest "spaghetti" and
  //         needs careful analysis) ffmpeg processes data in 'chunks' and
  //         therefore supports/requires memory alignment, as well as 'padding'
  //         bytes. Note that as the data may arrive in fragmented bits and
  //         pieces, the required preprocessing overhead may defeat the whole
  //         benefit of these features
  // *TODO*: find/implement a suitably balanced tradeoff that suits most
  //         scenarios

  // step1: make a 'deep' copy that implements memory alignment and supports
  //        padding
  // *TODO*: reduce overhead by making this optional (i.e. implement data
  //         messages that acquire these features on allocation)
  message_block_p = message_inout->clone ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to DataMessageType::clone(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  message_inout->release (); message_inout = NULL;

  // *NOTE*: the new buffer chain is already 'crunch'ed, i.e. aligned to base

  // *NOTE*: for the sake of efficiency and performance, reduce the number of
  //         loops (see below) by defragment()ing (before (re-)padding [see
  //         above]) the (chain of-) buffer(s)

  // *IMPORTANT NOTE*: apparently, some codecs (e.g. H264) do not support
  //                   chunked input very well (i.e. cannot handle consecutive
  //                   NAL unit fragments). Note how, unfortunately, this
  //                   entails estimating the maximum NAL unit size
  //                   --> defragment the buffer chain

  message_p = dynamic_cast<DataMessageType*> (message_block_p);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<DataMessageType*>(0x%@): \"%m\", returning\n"),
                inherited::mod_->name (),
                message_block_p));
    goto error;
  } // end IF
  try {
    message_p->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  }
  ACE_ASSERT (!message_p->cont ());

  // step2: (re-)pad [see above] the buffer chain
  // *IMPORTANT NOTE*: the message length does not change
  for (message_block_2 = message_block_p;
       message_block_2;
       message_block_2 = message_block_2->cont ())
  { ACE_ASSERT ((message_block_2->capacity () - message_block_2->size ()) >= padding_bytes);
    ACE_OS::memset (message_block_2->wr_ptr (), 0, padding_bytes);
  } // end FOR

  do
  {
    got_frame = 0;

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

    // *TODO*: use avcodec_send_packet rather than avcodec_decode_video2
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
    //result = avcodec_send_packet (context_,
    //                              &packet_s);
//#else
    result = avcodec_decode_video2 (context_,
                                    frame_,
                                    &got_frame,
                                    &packet_s);
//#endif
    if (unlikely (result < 0))
    {
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("%s: failed to avcodec_send_packet(): \"%s\", returning\n"),
      ////            ACE_TEXT ("%s: failed to avcodec_decode_video2(): \"%s\", returning\n"),
      //            inherited::mod_->name (),
      //            ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
    ACE_ASSERT (result == packet_s.size);
    if (!got_frame)
    {
      message_block_p = message_block_p->cont ();
      if (!message_block_p)
        break;
      continue;
    } // end IF
//#endif

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//next_frame:
//    result = avcodec_receive_frame (context_,
//                                    frame_);
//    if (result < 0)
//    {
//      if (result != AVERROR (EAGAIN))
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to avcodec_receive_frame(): \"%s\", returning\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//      goto loop_outer;
//    } // end IF
//#endif

    // successfully decoded a frame
    ACE_ASSERT (buffer_);
    buffer_->wr_ptr (frameSize_);
    buffer_->initialize (message_p->sessionId (),
                         NULL);
    buffer_->set (message_p->type ());
    message_block_2 = buffer_;
    buffer_ = NULL;

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
//#endif

    // convert pixel format of the decoded frame ?
    if (transformContext_)
    { ACE_ASSERT (outputFormat_ != AV_PIX_FMT_NONE);
      message_block_3 = inherited::allocateMessage (outputFrameSize_);
      if (unlikely (!message_block_3))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    outputFrameSize_));
        goto error;
      } // end IF
      DataMessageType* message_2 =
          dynamic_cast<DataMessageType*> (message_block_3);
      if (unlikely (!message_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to dynamic_cast<DataMessageType*>(0x%@): \"%m\", returning\n"),
                    inherited::mod_->name (),
                    message_block_3));
        goto error;
      } // end IF

      ACE_OS::memset (&line_sizes, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
      result =
          av_image_fill_linesizes (line_sizes,
                                   outputFormat_,
                                   static_cast<int> (frame_->width));
      ACE_ASSERT (result >= 0);
      ACE_OS::memset (&data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
      result =
          av_image_fill_pointers (data,
                                  outputFormat_,
                                  static_cast<int> (frame_->height),
                                  reinterpret_cast<uint8_t*> (message_block_3->wr_ptr ()),
                                  line_sizes);
      ACE_ASSERT (result >= 0);
      if (unlikely (!Stream_Module_Decoder_Tools::convert (transformContext_,
                                                           context_->width, context_->height, context_->pix_fmt,
                                                           static_cast<uint8_t**> (frame_->data),
                                                           frame_->width, frame_->height, outputFormat_,
                                                           data)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      message_2->wr_ptr (outputFrameSize_);
      message_2->initialize (message_p->sessionId (),
                             NULL);
      message_2->set (message_p->type ());

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
//#endif

      message_block_2->release ();
      message_block_2 = message_block_3;
    } // end IF

    // allocate a message buffer for the next frame
    ACE_ASSERT (!buffer_);
    av_frame_unref (frame_);
    buffer_ = inherited::allocateMessage (frameSize_);
    if (unlikely (!buffer_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  frameSize_));
      goto error;
    } // end IF
    result =
        av_image_fill_linesizes (frame_->linesize,
                                 context_->pix_fmt,
                                 static_cast<int> (frame_->width));
    ACE_ASSERT (result >= 0);
    result =
        av_image_fill_pointers (frame_->data,
                                context_->pix_fmt,
                                static_cast<int> (formatHeight_),
                                reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                frame_->linesize);
    ACE_ASSERT (result >= 0);

    // forward the decoded frame
    result = inherited::put_next (message_block_2, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    message_block_2 = NULL;

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    goto next_frame;
//
//loop_outer:
//#endif
    message_block_p = message_block_p->cont ();
    if (!message_block_p)
      break;
  } while (true);
  message_p->release (); message_p = NULL;

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF
  if (message_p)
    message_p->release ();
  if (message_block_2)
    message_block_2->release ();
  if (message_block_3)
    message_block_3->release ();

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
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
  ACE_ASSERT (message_inout);
  if (unlikely (codecId_ == AV_CODEC_ID_NONE))
    return; // nothing to do

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const SessionDataContainerType& session_data_container_r =
        message_inout->getR ();
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.getR ());

      enum AVPixelFormat input_format_e = AV_PIX_FMT_NONE;
      Common_UI_Resolution_t resolution_s;
      unsigned int decode_height, decode_width;
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      input_format_e = getFormat (session_data_r.formats.front ());
      resolution_s = getResolution (session_data_r.formats.front ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      formatHeight_ = static_cast<unsigned int> (std::abs (resolution_s.cy));
      decode_height = formatHeight_;
      decode_width = static_cast<unsigned int> (resolution_s.cx);
#else
      formatHeight_ = resolution_s.height;
      decode_height = formatHeight_;
      decode_width = resolution_s.width;
#endif // ACE_WIN32 || ACE_WIN64

      if ((codecId_ == AV_CODEC_ID_NONE) &&
          Stream_Module_Decoder_Tools::isCompressedVideo (input_format_e))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: codec id not set, best-guessing based on the input pixel format (was: %s)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (input_format_e).c_str ())));
        codecId_ =
            Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId (input_format_e);
      } // end IF
      if (codecId_ == AV_CODEC_ID_NONE)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid codec id, continuing\n"),
                    inherited::mod_->name ()));
#if defined (_DEBUG)
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: using codec \"%s\" (id: %d)\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (avcodec_get_name (codecId_)), codecId_));
#endif // _DEBUG
      //profile_ = configuration_in.codecProfile;

    //  frame_->format = configuration_in.outputFormat;
      frame_->height = formatHeight_;
      frame_->width = decode_width;

      outputFrameSize_ =
        av_image_get_buffer_size (outputFormat_,
                                  static_cast<int> (frame_->width),
                                  static_cast<int> (frame_->height),
                                  1); // *TODO*: linesize alignment

      int result = -1;
      struct AVCodec* codec_p = NULL;
      struct AVDictionary* dictionary_p = NULL;
      int flags, flags2;
      unsigned int buffer_size = 0;

      codec_p = avcodec_find_decoder (codecId_);
      if (unlikely (!codec_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_decoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    codecId_));
        goto error;
      } // end IF
      context_ = avcodec_alloc_context3 (codec_p);
      if (unlikely (!context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (context_);
//      result = avcodec_get_context_defaults3 (context_,
//                                              codec_p);
//      if (result < 0)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("avcodec_get_context_defaults3() failed: \"%s\", aborting\n"),
//                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//        goto error;
//      } // end IF

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      // sanity check(s)
//      ACE_ASSERT (inherited::configuration_);
//      ACE_ASSERT (inherited::configuration_->format);
//
//      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
//      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
//      LONG width, height;
//
//      if (inherited::configuration_->format->formattype == FORMAT_VideoInfo)
//      { ACE_ASSERT (inherited::configuration_->format->pbFormat);
//        video_info_header_p =
//          reinterpret_cast<struct tagVIDEOINFOHEADER*> (inherited::configuration_->format->pbFormat);
//        ACE_ASSERT (video_info_header_p);
//        width = video_info_header_p->bmiHeader.biWidth;
//        height = video_info_header_p->bmiHeader.biHeight;
//      } // end IF
//      else if (inherited::configuration_->format->formattype == FORMAT_VideoInfo2)
//      { ACE_ASSERT (inherited::configuration_->format->pbFormat);
//        video_info_header2_p =
//          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (inherited::configuration_->format->pbFormat);
//        ACE_ASSERT (video_info_header2_p);
//        width = video_info_header2_p->bmiHeader.biWidth;
//        height = video_info_header2_p->bmiHeader.biHeight;
//      } // end ELSE IF
//      else
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: invalid/unknown media type format type (was: \"%s\"), aborting\n"),
//                    ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (inherited::configuration_->format->formattype).c_str ()),
//                    inherited::mod_->name ()));
//        goto error;
//      } // end ELSE
//#endif

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
      //        AV_CODEC_FLAG_4MV            |
              AV_CODEC_FLAG_OUTPUT_CORRUPT |
              AV_CODEC_FLAG_QPEL           |
              //AV_CODEC_FLAG_PASS1          |
              //AV_CODEC_FLAG_PASS2          |
              AV_CODEC_FLAG_LOOP_FILTER    |
              //AV_CODEC_FLAG_GRAY           |
              //AV_CODEC_FLAG_PSNR           |
              AV_CODEC_FLAG_TRUNCATED      |
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
              AV_CODEC_FLAG_TRUNCATED      |
              //AV_CODEC_FLAG_INTERLACED_DCT |
              AV_CODEC_FLAG_LOW_DELAY      |
              //AV_CODEC_FLAG_GLOBAL_HEADER  |
              AV_CODEC_FLAG_BITEXACT;//       |
              //AV_CODEC_FLAG_AC_PRED        |
      //AV_CODEC_FLAG_INTERLACED_ME  |
      //AV_CODEC_FLAG_CLOSED_GOP;
//      if (codec_p->capabilities & CODEC_CAP_TRUNCATED)
//        flags |= CODEC_FLAG_TRUNCATED;
      if (codec_p->capabilities & AV_CODEC_CAP_TRUNCATED)
        flags |= AV_CODEC_FLAG_TRUNCATED;

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
      context_->opaque = &format_;
      //context_->bit_rate = bit_rate;
      context_->flags = flags;
      context_->flags2 = flags2;
      //context_->extradata = NULL;
      //context_->extradata_size = 0;
      context_->time_base.num = 1;
      context_->time_base.den = 1;
      context_->ticks_per_frame =
        (((codecId_ == AV_CODEC_ID_H264) ||
          (codecId_ == AV_CODEC_ID_MPEG2VIDEO)) ? 2 : 1);
      //context_->width = width;
      //context_->height = height;
      //context_->coded_width = width;
      //context_->coded_height = height;
//      context_->pix_fmt = AV_PIX_FMT_NONE;
      context_->pix_fmt = AV_PIX_FMT_YUV420P;
      //context_->draw_horiz_band = NULL;
//      context_->get_format = stream_decoder_libav_getformat_cb;
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
      context_->debug = (inherited::configuration_->debug ? 1 : 0);
//      context_->debug_mv = (inherited::configuration_->debug ? 1 : 0);
#endif // _DEBUG
//      context_->err_recognition = 0;
      context_->reordered_opaque = 0;
//      context_->hwaccel_context = NULL;
//      context_->idct_algo = FF_IDCT_AUTO;
//      context_->bits_per_coded_sample = 0;
//      context_->lowres = 0;
//      context_->thread_count = 1;
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
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
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
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        av_dict_free (&dictionary_p); dictionary_p = NULL;
        goto error;
      } // end IF
//      ACE_ASSERT (context_->pix_fmt != AV_PIX_FMT_NONE);
      av_dict_free (&dictionary_p); dictionary_p = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: initialized codec %s; decoded pixel format: %s\n"),
                  inherited::mod_->name (),
//                  ACE_TEXT (context_->codec_name),
                  ACE_TEXT (codec_p->long_name),
                  ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (context_->pix_fmt).c_str ())));

      // sanity check(s)
      ACE_ASSERT (frame_);
      ACE_ASSERT (frame_->width);

//      frame_->format = context_->pix_fmt;
      //frame_->width = context_->width;
      //frame_->height = context_->height;

      frameSize_ =
        av_image_get_buffer_size (context_->pix_fmt,
                                  frame_->width,
                                  formatHeight_,
                                  1); // *TODO*: linesize alignment
//      ACE_ASSERT (frameSize_ != 4294967274);
      buffer_size = frameSize_;

      if (context_->pix_fmt != outputFormat_)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: converting decoded pixel format %s to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (context_->pix_fmt).c_str ()),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (outputFormat_).c_str ())));

        flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                 SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
        transformContext_ =
            sws_getCachedContext (NULL,
                                  frame_->width, formatHeight_, context_->pix_fmt,
                                  frame_->width, formatHeight_, outputFormat_,
                                  flags,                        // flags
                                  NULL, NULL,
                                  0);                           // parameters
        if (unlikely (!transformContext_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
          goto error;
        } // end IF
        buffer_size = frameSize_;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (!buffer_);

      // initialize frame buffer
      buffer_ = inherited::allocateMessage (buffer_size);
      if (unlikely (!buffer_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    buffer_size));
        goto error;
      } // end IF

      result = av_image_fill_linesizes (frame_->linesize,
                                        context_->pix_fmt,
                                        static_cast<int> (frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (frame_->data,
                                  context_->pix_fmt,
                                  static_cast<int> (formatHeight_),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  frame_->linesize);
      ACE_ASSERT (result >= 0);

      goto continue_;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (codec_parameters_p)
        avcodec_parameters_free (&codec_parameters_p);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
//      session_data_r.format = codecFormat_;
      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      unsigned int width = 0;
      unsigned int buffer_size = frameSize_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (!session_data_r.formats.empty ());
      Common_UI_Resolution_t resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (session_data_r.formats.front ());
      formatHeight_ = static_cast<unsigned int> (::abs (resolution_s.cy));
      width = static_cast<unsigned int> (resolution_s.cx);
#else
      ACE_ASSERT (false); // *TODO*
//      formatHeight_ = session_data_r.inputFormat.height;
//      width =
//        static_cast<unsigned int> (session_data_r.inputFormat.width);
#endif // ACE_WIN32 || ACE_WIN64
      if (frame_)
      {
        av_frame_free (&frame_); frame_ = NULL;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (context_);

      frameSize_ =
        av_image_get_buffer_size (context_->pix_fmt,
                                  width,
                                  formatHeight_,
                                  1); // *TODO*: linesize alignment
      buffer_size = frameSize_;

      frame_ = av_frame_alloc ();
      if (unlikely (!frame_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
    //  frame_->format = session_data_r.format;
      frame_->height = formatHeight_;
      frame_->width = width;

      if (transformContext_)
      {
        sws_freeContext (transformContext_); transformContext_ = NULL;

        int flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                     SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
        transformContext_ =
            sws_getCachedContext (NULL,
                                  width, formatHeight_, context_->pix_fmt,
                                  width, formatHeight_, outputFormat_,
                                  flags,                        // flags
                                  NULL, NULL,
                                  0);                           // parameters
        if (unlikely (!transformContext_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          break;
        } // end IF
        buffer_size = frameSize_;
      } // end IF

      outputFrameSize_ =
        av_image_get_buffer_size (outputFormat_,
                                  width,
                                  formatHeight_,
                                  1); // *TODO*: linesize alignment

      if (buffer_)
      {
        buffer_->release (); buffer_ = NULL;
      } // end IF

      // initialize frame buffer
      buffer_ = inherited::allocateMessage (buffer_size);
      if (unlikely (!buffer_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                    inherited::mod_->name (),
                    buffer_size));
        break;
      } // end IF
      int result =
          av_image_fill_linesizes (frame_->linesize,
                                   context_->pix_fmt,
                                   static_cast<int> (width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (frame_->data,
                                  context_->pix_fmt,
                                  static_cast<int> (formatHeight_),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  frame_->linesize);
      ACE_ASSERT (result >= 0);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: modified frame resolution to %ux%u\n"),
                  inherited::mod_->name (),
                  width, formatHeight_));

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (context_)
      {
        avcodec_free_context (&context_); context_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
