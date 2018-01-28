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
#endif

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

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
                              SessionDataContainerType>::paddingBuffer[FF_INPUT_BUFFER_PADDING_SIZE];
#endif

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
#endif
 : inherited (stream_in)
 , buffer_ (NULL)
// , buffer_ ()
// , bufferRef_ ()
 , codecContext_ (NULL)
 , codecFormatHeight_ (0)
 , codecFrameSize_ (0)
 , codecId_ (AV_CODEC_ID_NONE)
 , codecProfile_ (FF_PROFILE_UNKNOWN)
 , currentFrame_ (NULL)
 , decodeContext_ (NULL)
 , decodeFormat_ (STREAM_DECODER_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT)
 , decodeFrameSize_ (0)
 , format_ (AV_PIX_FMT_NONE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_T::Stream_Decoder_LibAVDecoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&(OWN_TYPE_T::paddingBuffer),
                  0,
                  AV_INPUT_BUFFER_PADDING_SIZE);
#else
  ACE_OS::memset (&(OWN_TYPE_T::paddingBuffer),
                  0,
                  FF_INPUT_BUFFER_PADDING_SIZE);
#endif
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

  if (codecContext_)
  {
    result = avcodec_close (codecContext_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                  inherited::mod_->name (),
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

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF

    if (codecContext_)
    {
      result = avcodec_close (codecContext_);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_close() failed: \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      avcodec_free_context (&codecContext_);
    } // end IF
    codecFormatHeight_ = 0;
    codecFrameSize_ = 0;
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
    decodeFormat_ = STREAM_DECODER_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT;
    decodeFrameSize_ = 0;
    format_ = AV_PIX_FMT_NONE;
  } // end IF

#if defined (_DEBUG)
  //av_log_set_callback (Stream_Decoder_LibAVDecoder_LoggingCB);
  // *NOTE*: this level logs all messages
  //av_log_set_level (std::numeric_limits<int>::max ());
#endif
  av_register_all ();
//  avcodec_register_all ();

  unsigned int decode_height, decode_width, width;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.inputFormat);

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;

  format_ =
      Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (configuration_in.inputFormat->subtype,
                                                                    (configuration_in.mediaFramework == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION));

  if (configuration_in.inputFormat->formattype == FORMAT_VideoInfo)
  { ACE_ASSERT (configuration_in.inputFormat->pbFormat);
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (configuration_in.inputFormat->pbFormat);
    ACE_ASSERT (video_info_header_p);

    codecFormatHeight_ =
      static_cast<unsigned int> (std::abs (video_info_header_p->bmiHeader.biHeight));
    width =
      static_cast<unsigned int> (video_info_header_p->bmiHeader.biWidth);
    decode_height = codecFormatHeight_;
    decode_width = width;
  } // end IF
  else if (configuration_in.inputFormat->formattype == FORMAT_VideoInfo2)
  { ACE_ASSERT (configuration_in.inputFormat->pbFormat);
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (configuration_in.inputFormat->pbFormat);
    ACE_ASSERT (video_info_header2_p);

    codecFormatHeight_ =
      static_cast<unsigned int> (std::abs (video_info_header2_p->bmiHeader.biHeight));
    width =
      static_cast<unsigned int> (video_info_header2_p->bmiHeader.biWidth);
    decode_height = codecFormatHeight_;
    decode_width = width;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (configuration_in.inputFormat->formattype).c_str ())));
    return false;
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (configuration_in.format != AV_PIX_FMT_NONE);

  format_ = configuration_in.format;

  codecFormatHeight_ = configuration_in.sourceFormat.height;
  width = configuration_in.sourceFormat.width;
  decode_height = codecFormatHeight_;
  decode_width = width;
#endif

  // *TODO*: remove type inferences
  codecId_ = configuration_in.codecId;
  if ((codecId_ == AV_CODEC_ID_NONE) &&
      Stream_Module_Decoder_Tools::isCompressedVideo (format_))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: codec id not set, best-guessing based on the input pixel format (was: %s)\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (format_).c_str ())));
    codecId_ = Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecId (format_);
  } // end IF
  if (codecId_ == AV_CODEC_ID_NONE)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: invalid codec id, continuing\n"),
                inherited::mod_->name ()));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: using codec \"%s\" (id: %d)\n"),
                inherited::mod_->name (),
                ACE_TEXT (avcodec_get_name (codecId_)), codecId_));
  //codecProfile_ = configuration_in.codecProfile;

  currentFrame_ = av_frame_alloc ();
  if (!currentFrame_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
//  currentFrame_->format = configuration_in.outputFormat;
  currentFrame_->height = codecFormatHeight_;
  currentFrame_->width = width;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (configuration_in.outputFormat);

  decodeFormat_ =
    Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (configuration_in.outputFormat->subtype,
                                                                  (configuration_in.mediaFramework == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION));
  if (decodeFormat_ == AV_PIX_FMT_NONE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (configuration_in.outputFormat->subtype, (configuration_in.mediaFramework == STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)).c_str ())));
    return false;
  } // end IF
#else
  // sanity check(s)
  ACE_ASSERT (configuration_in.outputFormat != AV_PIX_FMT_NONE);

  decodeFormat_ = configuration_in.outputFormat;
#endif

  decodeFrameSize_ =
    av_image_get_buffer_size (decodeFormat_,
                              decode_width,
                              decode_height,
                              1); // *TODO*: linesize alignment

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
  if (codecId_ == AV_CODEC_ID_NONE)
    return; // nothing to do

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  unsigned int padding_bytes =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    AV_INPUT_BUFFER_PADDING_SIZE;
#else
    FF_INPUT_BUFFER_PADDING_SIZE;
#endif
  DataMessageType* message_p = NULL;
  int got_frame = 0;
  struct AVPacket packet_s;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  ACE_Message_Block* message_block_3 = NULL;
  int line_sizes[AV_NUM_DATA_POINTERS];
  uint8_t* data[AV_NUM_DATA_POINTERS];

  // *TODO*: remove ASAP (see: stream_task_base.inl:627)
  if (!codecContext_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: codec not (yet) initialized: dropping 'early' data message, returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

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
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to DataMessageType::clone(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  message_inout->release ();
  message_inout = NULL;

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
  if (!message_p)
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
  { ACE_ASSERT ((message_block_2->capacity () -
                 message_block_2->size ()) >= padding_bytes);
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
    //result = avcodec_send_packet (codecContext_,
    //                              &packet_s);
//#else
    result = avcodec_decode_video2 (codecContext_,
                                    currentFrame_,
                                    &got_frame,
                                    &packet_s);
//#endif
    if (result < 0)
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
//    result = avcodec_receive_frame (codecContext_,
//                                    currentFrame_);
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
    buffer_->wr_ptr (codecFrameSize_);
    buffer_->initialize (message_p->sessionId (),
                         NULL);
    buffer_->set (message_p->type ());
    message_block_2 = buffer_;
    buffer_ = NULL;

//#if defined (_DEBUG)
//    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.yuv");
//    if (!Common_Image_Tools::storeToFile (codecContext_->width, codecContext_->height, codecContext_->pix_fmt,
//                                          static_cast<uint8_t**> (currentFrame_->data),
//                                          filename_string))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_Image_Tools::storeToFile(\"%s\"), returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      goto error;
//    } // end IF
//#endif

    // convert pixel format of the decoded frame ?
    if (decodeContext_)
    {
      message_block_3 = inherited::allocateMessage (decodeFrameSize_);
      if (!message_block_3)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    decodeFrameSize_));
        goto error;
      } // end IF
      DataMessageType* message_2 =
          dynamic_cast<DataMessageType*> (message_block_3);
      if (!message_2)
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
                                   decodeFormat_,
                                   static_cast<int> (currentFrame_->width));
      ACE_ASSERT (result >= 0);
      ACE_OS::memset (&data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
      result =
          av_image_fill_pointers (data,
                                  decodeFormat_,
                                  static_cast<int> (currentFrame_->height),
                                  reinterpret_cast<uint8_t*> (message_block_3->wr_ptr ()),
                                  line_sizes);
      ACE_ASSERT (result >= 0);
      if (!Stream_Module_Decoder_Tools::convert (decodeContext_,
                                                 codecContext_->width, codecContext_->height, codecContext_->pix_fmt,
                                                 static_cast<uint8_t**> (currentFrame_->data),
                                                 currentFrame_->width, currentFrame_->height, decodeFormat_,
                                                 data))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      message_2->wr_ptr (decodeFrameSize_);
      message_2->initialize (message_p->sessionId (),
                             NULL);
      message_2->set (message_p->type ());

//#if defined (_DEBUG)
//    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
//    if (!Common_File_Tools::store (filename_string,
//                                   data[0],
//                                   decodeFrameSize_))
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
    av_frame_unref (currentFrame_);
    buffer_ = inherited::allocateMessage (codecFrameSize_);
    if (!buffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  codecFrameSize_));
      goto error;
    } // end IF
    result =
        av_image_fill_linesizes (currentFrame_->linesize,
                                 codecContext_->pix_fmt,
                                 static_cast<int> (currentFrame_->width));
    ACE_ASSERT (result >= 0);
    result =
        av_image_fill_pointers (currentFrame_->data,
                                codecContext_->pix_fmt,
                                static_cast<int> (codecFormatHeight_),
                                reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                currentFrame_->linesize);
    ACE_ASSERT (result >= 0);

    // forward the decoded frame
    result = inherited::put_next (message_block_2, NULL);
    if (result == -1)
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
  message_p->release ();

  return;

error:
  if (message_inout)
  {
    message_inout->release ();
    message_inout = NULL;
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
  if (codecId_ == AV_CODEC_ID_NONE)
    return; // nothing to do
  ACE_ASSERT (inherited::isInitialized_);

  const SessionDataContainerType& session_data_container_r =
    message_inout->getR ();
  typename SessionDataContainerType::DATA_T& session_data_r =
    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      int result = -1;
      struct AVCodec* codec_p = NULL;
      struct AVDictionary* dictionary_p = NULL;
      int flags, flags2;
      unsigned int buffer_size = decodeFrameSize_;

      codec_p = avcodec_find_decoder (codecId_);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_find_decoder(%d) failed: \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    codecId_));
        goto error;
      } // end IF
      codecContext_ = avcodec_alloc_context3 (codec_p);
      if (!codecContext_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_alloc_context3() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (codecContext_);
//      result = avcodec_get_context_defaults3 (codecContext_,
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
      if (!codec_parameters_p)
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

      codecContext_->opaque = &decodeFormat_;
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
      codecContext_->get_format = stream_decoder_libav_getformat_cb;
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
//      codecContext_->bits_per_coded_sample = 12;
      codecContext_->lowres = 0;
      //codecContext_->thread_count = 0;
      // *TODO*: support multithreaded decoding ?
      //codecContext_->thread_count = Common_Tools::getNumberOfCPUs (true);
      //codecContext_->thread_type = FF_THREAD_SLICE;
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
      //codecContext_->dump_separator = ',';
      //codecContext_->hw_frames_ctx = NULL;
      //codecContext_->max_pixels = 0;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      result = avcodec_parameters_to_context (codecContext_,
                                              codec_parameters_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_parameters_to_context() failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      avcodec_parameters_free (&codec_parameters_p);
      codec_parameters_p = NULL;
#endif
      ACE_ASSERT (codecContext_->pix_fmt == AV_PIX_FMT_NONE);

//      result = av_dict_set (&dictionary_p,
//                            NULL, NULL, 0);
      result = av_dict_set (&dictionary_p,
                            ACE_TEXT_ALWAYS_CHAR ("foo"),
                            ACE_TEXT_ALWAYS_CHAR ("bar"),
                            0);
      ACE_ASSERT (result >= 0);
      ACE_ASSERT (dictionary_p);
      result = avcodec_open2 (codecContext_,
                              codec_p,
                              &dictionary_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: avcodec_open2(%d) failed: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    codecId_,
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));

        av_dict_free (&dictionary_p);

        goto error;
      } // end IF
      ACE_ASSERT (codecContext_->pix_fmt != AV_PIX_FMT_NONE);
      av_dict_free (&dictionary_p);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: initialized codec (id was: %d), output pixel format: %s)\n"),
                  inherited::mod_->name (),
                  codecId_,
                  ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (codecContext_->pix_fmt).c_str ())));

      // sanity check(s)
      ACE_ASSERT (!buffer_);
      ACE_ASSERT (currentFrame_);
      ACE_ASSERT (currentFrame_->width);

//      currentFrame_->format = codecContext_->pix_fmt;
      //currentFrame_->width = codecContext_->width;
      //currentFrame_->height = codecContext_->height;

      codecFrameSize_ =
        av_image_get_buffer_size (codecContext_->pix_fmt,
                                  currentFrame_->width,
                                  codecFormatHeight_,
                                  1); // *TODO*: linesize alignment

      if (codecContext_->pix_fmt != decodeFormat_)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: converting codec output pixel format %s to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (codecContext_->pix_fmt).c_str ()),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (decodeFormat_).c_str ())));

        flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                     SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
        decodeContext_ =
            sws_getCachedContext (NULL,
                                  currentFrame_->width, codecFormatHeight_, codecContext_->pix_fmt,
                                  currentFrame_->width, codecFormatHeight_, decodeFormat_,
                                  flags,                        // flags
                                  NULL, NULL,
                                  0);                           // parameters
        if (!decodeContext_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
          goto error;
        } // end IF

        buffer_size = codecFrameSize_;
      } // end IF

      // initialize frame buffer
      buffer_ = inherited::allocateMessage (buffer_size);
      if (!buffer_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    buffer_size));
        goto error;
      } // end IF

      result = av_image_fill_linesizes (currentFrame_->linesize,
                                        codecContext_->pix_fmt,
                                        static_cast<int> (currentFrame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (currentFrame_->data,
                                  codecContext_->pix_fmt,
                                  static_cast<int> (codecFormatHeight_),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  currentFrame_->linesize);
      ACE_ASSERT (result >= 0);

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
//      session_data_r.format = codecFormat_;

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      unsigned int width = 0;
      unsigned int buffer_size = decodeFrameSize_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (session_data_r.inputFormat);

      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;

      if (session_data_r.inputFormat->formattype == FORMAT_VideoInfo)
      { ACE_ASSERT (session_data_r.inputFormat->pbFormat);
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.inputFormat->pbFormat);
        ACE_ASSERT (video_info_header_p);

        codecFormatHeight_ =
          static_cast<unsigned int> (abs (video_info_header_p->bmiHeader.biHeight));
        width =
          static_cast<unsigned int> (video_info_header_p->bmiHeader.biWidth);
      } // end IF
      else if (session_data_r.inputFormat->formattype == FORMAT_VideoInfo2)
      { ACE_ASSERT (session_data_r.inputFormat->pbFormat);
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (session_data_r.inputFormat->pbFormat);
        ACE_ASSERT (video_info_header2_p);

        codecFormatHeight_ =
          static_cast<unsigned int> (abs (video_info_header2_p->bmiHeader.biHeight));
        width =
          static_cast<unsigned int> (video_info_header2_p->bmiHeader.biWidth);
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown media type format type (was: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (session_data_r.inputFormat->formattype).c_str ())));
        break;
      } // end ELSE
#else // *TODO*
      ACE_ASSERT (false);
#endif
      if (currentFrame_)
      {
        av_frame_free (&currentFrame_);
        currentFrame_ = NULL;
      } // end IF

      // sanity check(s)
      ACE_ASSERT (codecContext_);

      codecFrameSize_ =
        av_image_get_buffer_size (codecContext_->pix_fmt,
                                  width,
                                  codecFormatHeight_,
                                  1); // *TODO*: linesize alignment

      currentFrame_ = av_frame_alloc ();
      if (!currentFrame_)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
    //  currentFrame_->format = session_data_r.format;
      currentFrame_->height = codecFormatHeight_;
      currentFrame_->width = width;

      if (decodeContext_)
      {
        sws_freeContext (decodeContext_);
        decodeContext_ = NULL;

        int flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                     SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
        decodeContext_ =
            sws_getCachedContext (NULL,
                                  width, codecFormatHeight_, codecContext_->pix_fmt,
                                  width, codecFormatHeight_, decodeFormat_,
                                  flags,                        // flags
                                  NULL, NULL,
                                  0);                           // parameters
        if (!decodeContext_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          break;
        } // end IF

        buffer_size = codecFrameSize_;
      } // end IF

      decodeFrameSize_ =
        av_image_get_buffer_size (decodeFormat_,
                                  width,
                                  codecFormatHeight_,
                                  1); // *TODO*: linesize alignment

      if (buffer_)
      {
        buffer_->release ();
        buffer_ = NULL;
      } // end IF

      // initialize frame buffer
      buffer_ = inherited::allocateMessage (buffer_size);
      if (!buffer_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                    inherited::mod_->name (),
                    buffer_size));
        break;
      } // end IF
      int result =
          av_image_fill_linesizes (currentFrame_->linesize,
                                   codecContext_->pix_fmt,
                                   static_cast<int> (width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (currentFrame_->data,
                                  codecContext_->pix_fmt,
                                  static_cast<int> (codecFormatHeight_),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  currentFrame_->linesize);
      ACE_ASSERT (result >= 0);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: modified frame resolution to %ux%u\n"),
                  inherited::mod_->name (),
                  width, codecFormatHeight_));

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
