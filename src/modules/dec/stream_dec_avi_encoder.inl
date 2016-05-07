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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "aviriff.h"
#include "dvdmedia.h"
#include "fourcc.h"
//#include "streams.h"
#else
#include "linux/videodev2.h"
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avio.h"
}
#endif

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_ReaderTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::~Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
int
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::put (ACE_Message_Block* messageBlock_in,
                                                              ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_IOCTL:
    {
      SessionDataType* session_data_p =
          reinterpret_cast<SessionDataType*> (messageBlock_in->base ());
      ACE_ASSERT (session_data_p);

      // *TODO*: remove type inference
      if (!postProcessHeader (session_data_p->targetFileName))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader(\"%s\"), aborting\n"),
                    ACE_TEXT (session_data_p->targetFileName.c_str ())));
        return -1;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                  messageBlock_in->msg_type ()));
      return -1;
    }
  } // end SWITCH

  return 0;
}

template <typename TaskSynchType,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_ReaderTask_T<TaskSynchType,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::postProcessHeader (const std::string& filename_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::postProcessHeader"));

  ACE_UNUSED_ARG (filename_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
}

////////////////////////////////////////////////////////////////////////////////

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_WriterTask_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , isFirst_ (true)
 , isInitialized_ (false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , formatContext_ (NULL)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::Stream_Decoder_AVIEncoder_WriterTask_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::~Stream_Decoder_AVIEncoder_WriterTask_T"));

  if (sessionData_)
    sessionData_->decrease ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  int result = -1;

  if (formatContext_)
  {
    if (formatContext_->streams)
      if (formatContext_->streams[0]->codec)
      {
        result = avcodec_close (formatContext_->streams[0]->codec);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
                      ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
      } // end IF

    avformat_free_context (formatContext_);
  } // end IF
#endif
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  int result = -1;

  if (isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    configuration_ = NULL;
    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isFirst_ = true;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    if (formatContext_)
    {
      if (formatContext_->streams)
        if (formatContext_->streams[0]->codec)
        {
          result = avcodec_close (formatContext_->streams[0]->codec);
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("avcodec_close() failed: \"%s\", continuing\n"),
                        ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        } // end IF


      avformat_free_context (formatContext_);
      formatContext_ = NULL;
    } // end IF
#endif

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  av_register_all ();
//  avcodec_register_all ();

  AVOutputFormat* output_format_p =
      av_guess_format (ACE_TEXT_ALWAYS_CHAR ("avi"), // short name
                       NULL,                         // file name
                       NULL);                        // MIME-type
  if (!output_format_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("av_guess_format(\"%s\") failed, aborting\n"),
                ACE_TEXT ("avi")));
    return false;
  } // end IF
  ACE_ASSERT (!formatContext_);
  formatContext_ = avformat_alloc_context ();
  if (!formatContext_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("avformat_alloc_context() failed, aborting\n")));
    return false;
  } // end IF
  formatContext_->oformat = output_format_p;
//  result =
//      avformat_alloc_output_context2 (&formatContext_, // return value: format context handle
//                                      output_format_p, // output format handle
//                                      NULL,            // format name
//                                      NULL);           // filename
//  if ((result < 0) || !formatContext_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("avformat_alloc_output_context2() failed: \"%s\", aborting\n"),
//                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
  ACE_ASSERT (formatContext_->oformat);
#endif

  isInitialized_ = true;

  return isInitialized_;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//error:
//  if (formatContext_)
//  {
//    if (formatContext_->streams[0])
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
#endif

  return false;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
const ConfigurationType&
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;
#endif

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);
  ACE_ASSERT (isInitialized_);

  // initialize driver ?
  if (isFirst_)
  {
    isFirst_ = false;

    // *TODO*: remove type inference
    message_block_p =
      allocateMessage (configuration_->streamConfiguration->bufferSize);
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                  configuration_->streamConfiguration->bufferSize));
      return;
    } // end IF
    ACE_ASSERT (message_block_p);

    if (!generateHeader (message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader(), returning\n")));

      // clean up
      message_block_p->release ();

      return;
    } // end IF

//    result = inherited::put_next (message_block_p, NULL);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", returning\n")));

//      // clean up
//      message_block_p->release ();

//      return;
//    } // end IF
//    message_block_p = NULL;
  } // end IF
  else
  {
    // *TODO*: remove type inference
    message_block_p =
        allocateMessage (configuration_->streamConfiguration->bufferSize);
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                  configuration_->streamConfiguration->bufferSize));
      return;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // db (--> Uncompressed video frame)
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = FCC ('00db');
  RIFF_chunk.cb = message_inout->length ();
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));
#else
  unsigned int riff_chunk_size = 0;
  result = message_block_p->copy (ACE_TEXT_ALWAYS_CHAR ("00db"),
                                  4);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
    goto error;
  } // end IF
  riff_chunk_size = message_inout->length ();
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    riff_chunk_size = ACE_SWAP_LONG (riff_chunk_size);
  result = message_block_p->copy (reinterpret_cast<char*> (&riff_chunk_size),
                                  4);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
    goto error;
  } // end IF
#endif

  message_block_p->cont (message_inout);
  result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", returning\n")));
    goto error;
  } // end IF

  return;

error:
  if (message_block_p)
    message_block_p->release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      sessionData_ =
        &const_cast<SessionDataContainerType&> (message_inout->get ());
      sessionData_->increase ();
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (sessionData_->get ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      int result = -1;
      enum AVCodecID codec_id = AV_CODEC_ID_RAWVIDEO; // RGB
      AVCodec* codec_p = NULL;
      AVCodecContext* codec_context_p = NULL;
      AVStream* stream_p = NULL;

      formatContext_->oformat->audio_codec = AV_CODEC_ID_NONE;
      switch (session_data_r.format.fmt.pix.pixelformat)
      {
        // RGB formats
        case V4L2_PIX_FMT_BGR24:
        case V4L2_PIX_FMT_RGB24:
          break;
        // luminance-chrominance formats
        case V4L2_PIX_FMT_YUV420: // 'YU12'
        case V4L2_PIX_FMT_YVU420: // 'YV12'
        case V4L2_PIX_FMT_YUYV:
          codec_id = AV_CODEC_ID_CYUV; // AV_CODEC_ID_YUV4 ?
          break;
        // compressed formats
        // *NOTE*: "... MJPEG, or at least the MJPEG in AVIs having the MJPG
        //         fourcc, is restricted JPEG with a fixed -- and *omitted* --
        //         Huffman table. The JPEG must be YCbCr colorspace, it must be
        //         4:2:2, and it must use basic Huffman encoding, not arithmetic
        //         or progressive. . . . You can indeed extract the MJPEG frames
        //         and decode them with a regular JPEG decoder, but you have to
        //         prepend the DHT segment to them, or else the decoder won't
        //         have any idea how to decompress the data. The exact table
        //         necessary is given in the OpenDML spec. ..."
        case V4L2_PIX_FMT_MJPEG:
          codec_id = AV_CODEC_ID_MJPEG;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown pixel format (was: %d), returning\n"),
                      session_data_r.format.fmt.pix.pixelformat));
          return;
        }
      } // end SWITCH
      formatContext_->oformat->video_codec = codec_id;

      codec_p = avcodec_find_encoder (codec_id);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_find_encoder(%d) failed: \"%m\", returning\n"),
                    codec_id));
        return;
      } // end IF
      ACE_ASSERT (!codec_context_p);
      codec_context_p = avcodec_alloc_context3 (codec_p);
      if (!codec_context_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_alloc_context3() failed: \"%m\", returning\n")));
        return;
      } // end IF
      result = avcodec_get_context_defaults3 (codec_context_p,
                                              codec_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_get_context_defaults3() failed: \"%s\", returning\n"),
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      codec_context_p->bit_rate =
          (session_data_r.format.fmt.pix.sizeimage *
           session_data_r.frameRate.denominator    *
           8);
      codec_context_p->codec_id = codec_id;
      codec_context_p->width = session_data_r.format.fmt.pix.width;
      codec_context_p->height = session_data_r.format.fmt.pix.height;
      codec_context_p->time_base.num = session_data_r.frameRate.numerator;
      codec_context_p->time_base.den = session_data_r.frameRate.denominator;
//      codec_context_p->gop_size = 10;
//      codec_context_p->max_b_frames = 1;

      // transform v4l format to libavformat type (AVPixelFormat)
      switch (session_data_r.format.fmt.pix.pixelformat)
      {
        // RGB formats
        case V4L2_PIX_FMT_BGR24:
          codec_context_p->pix_fmt = AV_PIX_FMT_BGR24;
          break;
        case V4L2_PIX_FMT_RGB24:
          codec_context_p->pix_fmt = AV_PIX_FMT_RGB24;
          break;
        // luminance-chrominance formats
        case V4L2_PIX_FMT_YUV420: // 'YU12'
          codec_context_p->pix_fmt = AV_PIX_FMT_YUV420P;
          break;
        case V4L2_PIX_FMT_YUYV:
          codec_context_p->pix_fmt = AV_PIX_FMT_YUYV422;
          break;
        // compressed formats
        // *NOTE*: "... MJPEG, or at least the MJPEG in AVIs having the MJPG
        //         fourcc, is restricted JPEG with a fixed -- and *omitted* --
        //         Huffman table. The JPEG must be YCbCr colorspace, it must be
        //         4:2:2, and it must use basic Huffman encoding, not arithmetic
        //         or progressive. . . . You can indeed extract the MJPEG frames
        //         and decode them with a regular JPEG decoder, but you have to
        //         prepend the DHT segment to them, or else the decoder won't
        //         have any idea how to decompress the data. The exact table
        //         necessary is given in the OpenDML spec. ..."
        case V4L2_PIX_FMT_MJPEG:
          codec_context_p->pix_fmt = AV_PIX_FMT_YUVJ422P;
          break;
        // *TODO*: ATM, libav cannot handle YVU formats
        case V4L2_PIX_FMT_YVU420: // 'YV12'
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown pixel format (was: %d), returning\n"),
                      session_data_r.format.fmt.pix.pixelformat));
          break;
        }
      } // end SWITCH
//      codec_context_p->pix_fmt = codec_->pix_fmts[0];

      result = avcodec_open2 (codec_context_p,
                              codec_p,
                              NULL);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_open2(%d) failed: \"%s\", returning\n"),
                    codec_id,
                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF

      ACE_ASSERT (!formatContext_->streams);
      stream_p = avformat_new_stream (formatContext_,
                                      codec_p);
      if (!stream_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avformat_new_stream() failed: \"%m\", returning\n")));
        goto error;
      } // end IF
      ACE_ASSERT (stream_p->codec);
      formatContext_->streams[0] = stream_p;

      // *TODO*: why does this need to be reset ?
      stream_p->codec->bit_rate =
          (session_data_r.format.fmt.pix.sizeimage *
           session_data_r.frameRate.denominator    *
           8);
      stream_p->codec->codec_id = codec_id;
      // stream_p->codec->codec_tag = 0; //if I comment this line write header works.
    //  stream_p->codec->codec_type = codec_->type;

      stream_p->codec->pix_fmt = codec_context_p->pix_fmt;
      stream_p->codec->width =
          session_data_r.format.fmt.pix.width;
      stream_p->codec->height =
          session_data_r.format.fmt.pix.height;

      stream_p->time_base.num =
          session_data_r.frameRate.numerator;
      stream_p->time_base.den =
          session_data_r.frameRate.denominator;
//      stream_p->codec->time_base.num =
//          session_data_r.frameRate.numerator;
//      stream_p->codec->time_base.den =
//          session_data_r.frameRate.denominator;

      goto continue_;

error:
      if (codec_context_p)
        avcodec_free_context (&codec_context_p);

      break;

continue_:
#endif

      break;
    }
    case STREAM_SESSION_END:
    {
      // sanity check(s)
      ACE_ASSERT (configuration_);
      ACE_ASSERT (configuration_->streamConfiguration);

      int result = -1;

      // *TODO*: remove type inference
      ACE_Message_Block* message_block_p =
        allocateMessage (configuration_->streamConfiguration->bufferSize);
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("allocateMessage(%d) failed: \"%m\", continuing\n"),
                    configuration_->streamConfiguration->bufferSize));
        goto continue_2;
      } // end IF
      ACE_ASSERT (message_block_p);

      if (!generateIndex (message_block_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex(): \"%m\", continuing\n")));

        // clean up
        message_block_p->release ();

        goto continue_2;
      } // end IF

      result = inherited::put_next (message_block_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", continuing\n")));

        // clean up
        message_block_p->release ();

        goto continue_2;
      } // end IF

continue_2:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      avformat_free_context (formatContext_);
      formatContext_ = NULL;
#endif

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
MessageType*
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage"));

  // initialize return value(s)
  MessageType* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);

  if (configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      message_block_p =
        static_cast<MessageType*> (configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      MessageType (requestedSize_in));
  if (!message_block_p)
  {
    if (configuration_->streamConfiguration->messageAllocator)
    {
      if (configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate MessageType: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate MessageType: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (sessionData_);
  ACE_ASSERT (messageBlock_inout);

  int result = -1;
  const SessionDataType& session_data_r = sessionData_->get ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;

  // sanity check(s)
  ACE_ASSERT (session_data_r.format);

  struct _rifflist RIFF_list;
  struct _avimainheader AVI_header_avih;
  struct _avistreamheader AVI_header_strh;
  FOURCCMap fourcc_map;
  unsigned int pad_bytes = 0;
  struct tagBITMAPINFOHEADER AVI_header_strf;

  struct _AMMediaType media_type;
  HRESULT result_2 = MFInitAMMediaTypeFromMFMediaType (session_data_r.format,
                                                       GUID_NULL,
                                                       &media_type);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%m\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return false;
  } // end IF
  //if ((session_data_r.format->formattype != FORMAT_VideoInfo) &&
  //    (session_data_r.format->formattype != FORMAT_VideoInfo2))
  if ((media_type.formattype != FORMAT_VideoInfo) &&
      (media_type.formattype != FORMAT_VideoInfo2))
  {
    OLECHAR GUID_string[39];
    ACE_OS::memset (&GUID_string, 0, sizeof (GUID_string));
    int nCount =
      StringFromGUID2 (media_type.formattype,
                       GUID_string, sizeof (GUID_string));
    ACE_ASSERT (nCount == 39);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    goto error;
  } // end IF

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  //if (session_data_r.format->formattype == FORMAT_VideoInfo)
  if (media_type.formattype == FORMAT_VideoInfo)
    video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type.pbFormat;
  //else if (session_data_r.format->formattype == FORMAT_VideoInfo2)
  else if (media_type.formattype == FORMAT_VideoInfo2)
    video_info_header2_p = (struct tagVIDEOINFOHEADER2*)media_type.pbFormat;

  // RIFF header
  ACE_OS::memset (&RIFF_list, 0, sizeof (struct _rifflist));
  RIFF_list.fcc = FCC ('RIFF');
  // *NOTE*: in a streaming scenario, this would need to be added AFTER the
  //         file has been written (or the disc runs out of space), which is
  //         impossible until/unless this value is preconfigured in some way.
  //         Notice how this oversight confounds the whole standard
  // sizeof (fccListType) [4] + sizeof (data) --> == total (file) size - 8
  RIFF_list.cb = sizeof (FOURCC)                     +
                 sizeof (struct _rifflist)           + // hdrl
                 sizeof (struct _avimainheader)      +
                 // sizeof (LIST strl)
                 sizeof (struct _rifflist)           +
                 sizeof (struct _avistreamheader)    + // strh
                 sizeof (struct _riffchunk)          + // strf
                 sizeof (struct tagBITMAPINFOHEADER) + // strf
                 sizeof (struct _riffchunk)          + // JUNK
                 1820                                + // pad bytes
                 sizeof (struct _rifflist)           + // movi
                 sizeof (struct _riffchunk)          + // 00db
                 messageBlock_inout->length ();        // (part of) frame
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
  RIFF_list.fccListType = FCC ('AVI ');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                     sizeof (struct _rifflist));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // hdrl
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST data)
  RIFF_list.cb = sizeof (FOURCC)                    +
                 sizeof (struct _avimainheader)     +
                 // sizeof (LIST strl)
                 sizeof (struct _rifflist)          +
                 sizeof (struct _avistreamheader)   + // strh
                 sizeof (struct _riffchunk)         + // strf
                 sizeof (struct tagBITMAPINFOHEADER); // strf
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
  RIFF_list.fccListType = FCC ('hdrl');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                      sizeof (struct _rifflist));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // *NOTE*: "...the 'hdrl' list begins with the main AVI header, which is
  //         contained in an 'avih' chunk. ..."
  ACE_OS::memset (&AVI_header_avih, 0, sizeof (struct _avimainheader));
  AVI_header_avih.fcc = ckidMAINAVIHEADER;
  AVI_header_avih.cb = sizeof (struct _avimainheader) - 8;
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    AVI_header_avih.cb = ACE_SWAP_LONG (AVI_header_avih.cb);
  AVI_header_avih.dwMicroSecPerFrame =
    ((media_type.formattype == FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
                                                 : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame));
  AVI_header_avih.dwMaxBytesPerSec =
    ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->dwBitRate
                                                 : video_info_header2_p->dwBitRate) / 8;
  AVI_header_avih.dwPaddingGranularity = STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN;
  AVI_header_avih.dwFlags = AVIF_WASCAPTUREFILE;
  //AVI_header_avih.dwTotalFrames = 0; // unreliable
  //AVI_header_avih.dwInitialFrames = 0;
  AVI_header_avih.dwStreams = 1;
  //AVI_header_avih.dwSuggestedBufferSize = 0; // unreliable
  AVI_header_avih.dwWidth =
    ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biWidth
                                                 : video_info_header2_p->bmiHeader.biWidth);
  AVI_header_avih.dwHeight =
    ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biHeight
                                                 : video_info_header2_p->bmiHeader.biHeight);
  //AVI_header_avih.dwReserved = {0, 0, 0, 0};
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_avih),
                              sizeof (struct _avimainheader));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // *NOTE*: "One or more 'strl' lists follow the main header. A 'strl' list
  //         is required for each data stream. Each 'strl' list contains
  //         information about one stream in the file, and must contain a
  //         stream header chunk ('strh') and a stream format chunk ('strf').
  //         ..."
  // strl
  RIFF_list.fcc = FCC ('LIST');
  // sizeof (fccListType) [4] + sizeof (LIST data)
  RIFF_list.cb = sizeof (FOURCC)                   +
                 sizeof (struct _avistreamheader)  + // strh
                 sizeof (struct _riffchunk)        + // strf
                 sizeof (struct tagBITMAPINFOHEADER); // strf
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_list.cb = ACE_SWAP_LONG (RIFF_list.cb);
  RIFF_list.fccListType = ckidSTREAMLIST;
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                      sizeof (struct _rifflist));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // strl --> strh
  ACE_OS::memset (&AVI_header_strh, 0, sizeof (struct _avistreamheader));
  AVI_header_strh.fcc = ckidSTREAMHEADER;
  AVI_header_strh.cb = sizeof (struct _avistreamheader) - 8;
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    AVI_header_strh.cb = ACE_SWAP_LONG (AVI_header_strh.cb);
  AVI_header_strh.fccType = streamtypeVIDEO;
  fourcc_map.SetFOURCC (&media_type.subtype);
  AVI_header_strh.fccHandler = fourcc_map.GetFOURCC ();
  //AVI_header_strh.fccHandler = 0;
  //AVI_header_strh.dwFlags = 0;
  //AVI_header_strh.wPriority = 0;
  //AVI_header_strh.wLanguage = 0;
  //AVI_header_strh.dwInitialFrames = 0;
  // *NOTE*: dwRate / dwScale == fps
  AVI_header_strh.dwScale = 10000; // 100th nanoseconds --> seconds ???
  AVI_header_strh.dwRate =
    ((media_type.formattype == FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
                                                 : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame));
  //AVI_header_strh.dwStart = 0;
  //AVI_header_strh.dwLength = 0;
  //AVI_header_strh.dwSuggestedBufferSize = 0;
  AVI_header_strh.dwQuality = -1; // default
                                  //AVI_header_strh.dwSampleSize = 0;
                                  //AVI_header_strh.rcFrame = {0, 0, 0, 0};
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strh),
                              sizeof (struct _avistreamheader));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // strl --> strf
  // *NOTE*: there is no definition for AVI stream format chunks, as their
  //         contents differ, depending on the stream type
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = ckidSTREAMFORMAT;
  RIFF_chunk.cb = sizeof (struct tagBITMAPINFOHEADER);
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                      sizeof (struct _riffchunk));
  ACE_OS::memset (&AVI_header_strf, 0, sizeof (struct tagBITMAPINFOHEADER));
  AVI_header_strf =
    ((media_type.formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
                                                 : video_info_header2_p->bmiHeader);
  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_strf),
                              sizeof (struct tagBITMAPINFOHEADER));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  // strl --> strd
  // strl --> strn

  // --> END strl

  // insert JUNK chunk to align the 'movi' chunk at 2048 bytes
  // --> should speed up CD-ROM access
  pad_bytes = (AVI_header_avih.dwPaddingGranularity -
               messageBlock_inout->length () - 8 - 12);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("inserting JUNK chunk (%d pad byte(s))...\n"),
              pad_bytes));
  RIFF_chunk.fcc = FCC ('JUNK');
  RIFF_chunk.cb = pad_bytes;
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                      sizeof (struct _riffchunk));
  ACE_OS::memset (messageBlock_inout->wr_ptr (), 0, pad_bytes);
  messageBlock_inout->wr_ptr (RIFF_chunk.cb);

  // movi
  RIFF_list.fcc = FCC ('LIST');
  // *NOTE*: see above
  // sizeof (fccListType) [4] + sizeof (LIST data)
  RIFF_list.cb = sizeof (FOURCC)              +
                 sizeof (struct _riffchunk)   + // 00db
                 messageBlock_inout->length (); // (part of) frame
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  RIFF_list.fccListType = FCC ('movi');
  result = messageBlock_inout->copy (reinterpret_cast<char*> (&RIFF_list),
                                      sizeof (struct _rifflist));
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  //RIFF_chunk.fcc = FCC ('00db');
  //RIFF_chunk.cb = message_inout->length ();
  //if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
  //  RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  //result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
  //                                sizeof (struct _riffchunk));

  // clean up
  Stream_Module_Device_Tools::freeMediaType (media_type);

  goto continue_;

error:
  Stream_Module_Device_Tools::freeMediaType (media_type);

  return false;
#else
  ACE_ASSERT (!formatContext_->pb);
  formatContext_->pb =
    avio_alloc_context (reinterpret_cast<unsigned char*> (messageBlock_inout->wr_ptr ()), // buffer handle
                        messageBlock_inout->capacity (),          // buffer size
                        1,                                        // write flag
                        messageBlock_inout,                       // act
                        NULL,                                     // read callback
                        stream_decoder_aviencoder_libav_write_cb, // write callback
                        NULL);                                    // seek callback
  if (!formatContext_->pb)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("avio_alloc_context() failed: \"%m\", aborting\n")));
    return false;
  } // end IF

  result = avformat_write_header (formatContext_, // context handle
                                  NULL);          // options
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("avformat_write_header() failed: \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  avio_flush (formatContext_->pb);
#endif
continue_:
  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<SessionMessageType,
                                       MessageType,
                                       ConfigurationType,
                                       SessionDataContainerType,
                                       SessionDataType>::generateIndex (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateIndex"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);

  int result = -1;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _avisuperindex AVI_header_index;
  ACE_OS::memset (&AVI_header_index, 0, sizeof (struct _avisuperindex));
//  AVI_header_index.fcc = 0;
//  AVI_header_index.cb = 0;
//  AVI_header_index.wLongsPerEntry = 0;
//  AVI_header_index.bIndexSubType = 0;
//  AVI_header_index.bIndexType = 0;
//  AVI_header_index.nEntriesInUse = 0;
//  AVI_header_index.dwChunkId = 0;
//  AVI_header_index.dwReserved = 0;
//  AVI_header_index.aIndex = 0;

//  struct _avisuperindex_entry AVI_header_index_entry_0;
//  ACE_OS::memset (&AVI_header_index_entry_0, 0, sizeof (struct _avisuperindex_entry));
//  struct _avisuperindex_entry AVI_header_index_entry_1;
//  ACE_OS::memset (&AVI_header_index_entry_1, 0, sizeof (struct _avisuperindex_entry));
//  struct _avisuperindex_entry AVI_header_index_entry_2;
//  ACE_OS::memset (&AVI_header_index_entry_2, 0, sizeof (struct _avisuperindex_entry));
//  struct _avisuperindex_entry AVI_header_index_entry_3;
//  ACE_OS::memset (&AVI_header_index_entry_3, 0, sizeof (struct _avisuperindex_entry));

  result =
    messageBlock_inout->copy (reinterpret_cast<char*> (&AVI_header_index),
                              sizeof (struct _avisuperindex));
#else
  // sanity check(s)
  ACE_ASSERT (formatContext_);

  result = av_write_trailer (formatContext_);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("av_write_trailer() failed: \"%s\", continuing\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
#endif

  return true;
}
