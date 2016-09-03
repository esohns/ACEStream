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
#include "amvideo.h"
#include "mmiscapi.h"
#include "aviriff.h"
#include "dvdmedia.h"
#include "fourcc.h"
#include "mfobjects.h"
#include "uuids.h"
#else
#include "linux/videodev2.h"
extern "C"
{
#include "libavcodec/avcodec.h"

#include "libavformat/avio.h"
//#include "libavformat/raw.h"
//#include "libavformat/riff.h"
}
#endif

//#include "ace/FILE_Addr.h"
//#include "ace/FILE_Connector.h"
#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_ReaderTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       SessionDataContainerType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_ReaderTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_ReaderTask_T::~Stream_Decoder_AVIEncoder_ReaderTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
int
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_ReaderTask_T<ACE_SYNCH_USE,
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::Stream_Decoder_AVIEncoder_WriterTask_T ()
 : inherited ()
 , isFirst_ (true)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
 , formatContext_ (NULL)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::Stream_Decoder_AVIEncoder_WriterTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::~Stream_Decoder_AVIEncoder_WriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::~Stream_Decoder_AVIEncoder_WriterTask_T"));

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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::initialize"));

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    isFirst_ = true;
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
      formatContext_ = NULL;
    } // end IF
#endif

    inherited::isInitialized_ = false;
  } // end IF

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

  return inherited::initialize (configuration_in);

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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());
  if (session_data_r.targetFileName.empty ())
    return; // nothing to do

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;
#else
  unsigned int riff_chunk_size = 0;
#endif

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // *TODO*: remove type inference
  message_block_p =
    allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("allocateMessage(%d) failed: \"%m\", returning\n"),
                inherited::configuration_->streamConfiguration->bufferSize));
    goto error;
  } // end IF

  if (isFirst_)
  {
    isFirst_ = false;

    if (!generateHeader (message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader(), returning\n")));
      goto error;
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      struct v4l2_format* format_p = NULL;
      struct v4l2_fract* frame_rate_p = NULL;

      int result = -1;
      enum AVCodecID codec_id = AV_CODEC_ID_RAWVIDEO; // RGB
      AVCodec* codec_p = NULL;
      AVCodecContext* codec_context_p = NULL;
      AVStream* stream_p = NULL;

      // sanity check(s)
      ACE_ASSERT (session_data_r.format);

      format_p = getFormat (session_data_r.format);
      if (!format_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve media format, aborting\n")));
        goto error;
      } // end IF

      frame_rate_p = getFrameRate (session_data_r,
                                   session_data_r.format);
      if (!frame_rate_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve frame rate, aborting\n")));
        goto error;
      } // end IF

      ACE_ASSERT (formatContext_);
      ACE_ASSERT (formatContext_->oformat);

      formatContext_->oformat->audio_codec = AV_CODEC_ID_NONE;
      switch (format_p->fmt.pix.pixelformat)
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
                      format_p->fmt.pix.pixelformat));
          goto error;
        }
      } // end SWITCH
      formatContext_->oformat->video_codec = codec_id;

      codec_p = avcodec_find_encoder (codec_id);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_find_encoder(%d) failed: \"%m\", returning\n"),
                    codec_id));
        goto error;
      } // end IF
      ACE_ASSERT (!codec_context_p);
      codec_context_p = avcodec_alloc_context3 (codec_p);
      if (!codec_context_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("avcodec_alloc_context3() failed: \"%m\", returning\n")));
        goto error;
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
          (format_p->fmt.pix.sizeimage *
           frame_rate_p->denominator   *
           8);
      codec_context_p->codec_id = codec_id;
      codec_context_p->width = format_p->fmt.pix.width;
      codec_context_p->height = format_p->fmt.pix.height;
      codec_context_p->time_base.num = frame_rate_p->numerator;
      codec_context_p->time_base.den = frame_rate_p->denominator;
//      codec_context_p->gop_size = 10;
//      codec_context_p->max_b_frames = 1;

      // transform v4l format to libavformat type (AVPixelFormat)
      switch (format_p->fmt.pix.pixelformat)
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
                      format_p->fmt.pix.pixelformat));
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
          (format_p->fmt.pix.sizeimage *
           frame_rate_p->denominator    *
           8);
      stream_p->codec->codec_id = codec_id;
      // stream_p->codec->codec_tag = 0; //if I comment this line write header works.
    //  stream_p->codec->codec_type = codec_->type;

      stream_p->codec->pix_fmt = codec_context_p->pix_fmt;
      stream_p->codec->width = format_p->fmt.pix.width;
      stream_p->codec->height = format_p->fmt.pix.height;

      stream_p->time_base.num = frame_rate_p->numerator;
      stream_p->time_base.den = frame_rate_p->denominator;
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
    case STREAM_SESSION_MESSAGE_END:
    {
      int result = -1;
      ACE_Message_Block* message_block_p = NULL;

      if (session_data_r.targetFileName.empty ())
        goto continue_2; // nothing to do

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      // *TODO*: remove type inference
      message_block_p =
        allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("allocateMessage(%d) failed: \"%m\", continuing\n"),
                    inherited::configuration_->streamConfiguration->bufferSize));
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
          typename SessionDataType>
DataMessageType*
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_block_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (inherited::configuration_->messageAllocator)
  {
allocate:
    try {
      message_block_p =
        static_cast<DataMessageType*> (inherited::configuration_->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_block_p &&
        !inherited::configuration_->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      DataMessageType (requestedSize_in));
  if (!message_block_p)
  {
    if (inherited::configuration_->messageAllocator)
    {
      if (inherited::configuration_->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
  } // end IF

  return message_block_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (messageBlock_inout);

  int result = -1;
  const SessionDataType& session_data_r = inherited::sessionData_->get ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _riffchunk RIFF_chunk;

  // sanity check(s)
  ACE_ASSERT (session_data_r.format);

  struct _AMMediaType* media_type_p = NULL;
  media_type_p = getFormat (session_data_r.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve media type, aborting\n")));
    return false;
  } // end IF

  struct _rifflist RIFF_list;
  struct _avimainheader AVI_header_avih;
  struct _avistreamheader AVI_header_strh;
  FOURCCMap fourcc_map;
  unsigned int pad_bytes = 0;
  struct tagBITMAPINFOHEADER AVI_header_strf;

  if ((media_type_p->formattype != FORMAT_VideoInfo) &&
      (media_type_p->formattype != FORMAT_VideoInfo2))
  {
    OLECHAR GUID_string[CHARS_IN_GUID];
    ACE_OS::memset (GUID_string, 0, sizeof (GUID_string));
    StringFromGUID2 (media_type_p->formattype,
                     GUID_string, sizeof (GUID_string));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (GUID_string)));
    goto error;
  } // end IF

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  if (media_type_p->formattype == FORMAT_VideoInfo)
    video_info_header_p =
      (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
  else if (media_type_p->formattype == FORMAT_VideoInfo2)
    video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;

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
    ((media_type_p->formattype == FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
                                                    : static_cast<DWORD> (video_info_header2_p->AvgTimePerFrame));
  AVI_header_avih.dwMaxBytesPerSec =
    ((media_type_p->formattype == FORMAT_VideoInfo) ? video_info_header_p->dwBitRate
                                                    : video_info_header2_p->dwBitRate) / 8;
  AVI_header_avih.dwPaddingGranularity = STREAM_DECODER_AVI_JUNK_CHUNK_ALIGN;
  AVI_header_avih.dwFlags = AVIF_WASCAPTUREFILE;
  //AVI_header_avih.dwTotalFrames = 0; // unreliable
  //AVI_header_avih.dwInitialFrames = 0;
  AVI_header_avih.dwStreams = 1;
  //AVI_header_avih.dwSuggestedBufferSize = 0; // unreliable
  AVI_header_avih.dwWidth =
    ((media_type_p->formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biWidth
                                                    : video_info_header2_p->bmiHeader.biWidth);
  AVI_header_avih.dwHeight =
    ((media_type_p->formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader.biHeight
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
  fourcc_map.SetFOURCC (&media_type_p->subtype);
  AVI_header_strh.fccHandler = fourcc_map.GetFOURCC ();
  //AVI_header_strh.fccHandler = 0;
  //AVI_header_strh.dwFlags = 0;
  //AVI_header_strh.wPriority = 0;
  //AVI_header_strh.wLanguage = 0;
  //AVI_header_strh.dwInitialFrames = 0;
  // *NOTE*: dwRate / dwScale == fps
  AVI_header_strh.dwScale = 10000; // 100th nanoseconds --> seconds ???
  AVI_header_strh.dwRate =
    ((media_type_p->formattype == FORMAT_VideoInfo) ? static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)
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
    ((media_type_p->formattype == FORMAT_VideoInfo) ? video_info_header_p->bmiHeader
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

  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  goto continue_;

error:
  if (media_type_p)
    Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  return false;

continue_:
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

  return true;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
AM_MEDIA_TYPE*
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;
  if (!Stream_Module_Device_Tools::copyMediaType (*format_in,
                                                  result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::copyMediaType(), aborting\n")));
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);

  return result_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
AM_MEDIA_TYPE*
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (format_in),
                                        GUID_NULL,
                                        &result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);

  return result_p;
}
#else
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
struct v4l2_format*
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::getFormat_impl (const struct _snd_pcm_hw_params*)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFormat_impl"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
struct v4l2_fract*
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataContainerType,
                                       SessionDataType>::getFrameRate_impl (const SessionDataType&,
                                                                            const struct _snd_pcm_hw_params*)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::getFrameRate_impl"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_AVIEncoder_WriterTask_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
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

////////////////////////////////////////////////////////////////////////////////

static sox_bool
sox_overwrite_permitted (char const * filename_in)
{
  return sox_true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::Stream_Decoder_WAVEncoder_T ()
 : inherited ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
// , SFInfo_ ()
// , SNDFile_ (NULL)
 , encodingInfo_ ()
 , signalInfo_ ()
 , outputFile_ (NULL)
#endif
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::Stream_Decoder_WAVEncoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//  ACE_OS::memset (&SFInfo_, 0, sizeof (struct SF_INFO));
  ACE_OS::memset (&encodingInfo_, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&signalInfo_, 0, sizeof (struct sox_signalinfo_t));
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::~Stream_Decoder_WAVEncoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::~Stream_Decoder_WAVEncoder_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  int result = -1;
  if (outputFile_)
  {
    result = sox_close (outputFile_);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(), continuing\n")));
  } // end IF

  result = sox_quit ();
  if (result != SOX_SUCCESS)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_quit(): \"%m\", continuing\n")));
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
bool
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
    result = sox_quit ();
    if (result != SOX_SUCCESS)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_quit(): \"%m\", aborting\n")));
      return false;
    } // end IF
#endif

    inherited::isInitialized_ = false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  result = sox_init ();
  if (result != SOX_SUCCESS)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sox_init(): \"%m\", aborting\n")));
    return false;
  } // end IF
#endif

  return inherited::initialize (configuration_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleDataMessage"));

  // sanity check(s)
  if (!inherited::sessionData_)
    return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());
  if (session_data_r.targetFileName.empty ())
#else
  if (!outputFile_)
#endif
    return; // nothing to do

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_Message_Block* message_block_p = NULL;
  struct _riffchunk RIFF_chunk;
#else
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // *TODO*: remove type inference
  message_block_p =
    inherited::allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("Stream_Decoder_AVIEncoder_WriterTask_T::allocateMessage(%d) failed: \"%m\", returning\n"),
                inherited::configuration_->streamConfiguration->bufferSize));
    goto error;
  } // end IF

  if (inherited::isFirst_)
  {
    inherited::isFirst_ = false;

    if (!generateHeader (message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Decoder_WAVEncoder_T::generateHeader(), returning\n")));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p);

  // db (--> Uncompressed video frame)
  ACE_OS::memset (&RIFF_chunk, 0, sizeof (struct _riffchunk));
  RIFF_chunk.fcc = FCC ('00db');
  RIFF_chunk.cb = message_inout->length ();
  if (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
    RIFF_chunk.cb = ACE_SWAP_LONG (RIFF_chunk.cb);
  result = message_block_p->copy (reinterpret_cast<char*> (&RIFF_chunk),
                                  sizeof (struct _riffchunk));

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

  return;
#else
  // *IMPORTANT NOTE*: sox_write() expects signed 32-bit samples
  //                   --> convert the data in memory first
  size_t samples_read = 0;
  /* Temporary store whilst copying. */
  sox_sample_t samples[STREAM_DECODER_SOX_SAMPLE_BUFFERS];
  sox_format_t* memory_buffer_p =
      sox_open_mem_read (message_inout->rd_ptr (),
                         message_inout->length (),
                         &signalInfo_,
                         &encodingInfo_,
                         ACE_TEXT_ALWAYS_CHAR ("sox"));
//                         NULL);
  if (!memory_buffer_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sox_open_mem_read(): \"%m\", returning\n"),
                ACE_TEXT (inherited::mod_->name ())));
    goto error;
  } // end IF

  while ((samples_read = sox_read (memory_buffer_p,
                                   samples,
                                   STREAM_DECODER_SOX_SAMPLE_BUFFERS)))
    if (sox_write (outputFile_,
                   samples,
                   samples_read) != samples_read)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sox_write(): \"%m\", returning\n"),
                  ACE_TEXT (inherited::mod_->name ())));
      goto error;
    } // end IF

error:
  if (memory_buffer_p)
  {
    result = sox_close (memory_buffer_p);
    if (result != SOX_SUCCESS)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sox_close(), continuing\n")));
  } // end IF

  return;
#endif
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType,
//          typename SessionDataType>
//void
//Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ConfigurationType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            SessionDataContainerType,
//                            SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                    bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleSessionMessage"));

//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);

//  // sanity check(s)
//  ACE_ASSERT (inherited::isInitialized_);
//  ACE_ASSERT (inherited::sessionData_);

//  SessionDataType& session_data_r =
//    const_cast<SessionDataType&> (inherited::sessionData_->get ());
//  int result = -1;

//  switch (message_inout->type ())
//  {
//    case STREAM_SESSION_MESSAGE_BEGIN:
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//      ACE_ASSERT (inherited::formatContext_);
//      ACE_ASSERT (inherited::formatContext_->oformat);

////      enum AVCodecID codec_id = AV_CODEC_ID_FIRST_AUDIO;
////      enum AVSampleFormat sample_format = AV_SAMPLE_FMT_NONE;
////      AVCodec* codec_p = NULL;
////      AVCodecContext* codec_context_p = NULL;
////      AVStream* stream_p = NULL;
//      enum _snd_pcm_format format = SND_PCM_FORMAT_UNKNOWN;
//      unsigned int channels = 0;
//      unsigned int sample_rate = 0;
//      int subunit_direction = 0;

////      inherited::formatContext_->oformat->audio_codec = AV_CODEC_ID_NONE;
//      result = snd_pcm_hw_params_get_format (inherited::configuration_->format,
//                                             &format);
//      ACE_ASSERT (result >= 0);
//      switch (format)
//      {
//        // PCM 'formats'
//        case SND_PCM_FORMAT_S16_LE:
////          codec_id = AV_CODEC_ID_PCM_S16LE;
////          sample_format = AV_SAMPLE_FMT_S16;
//          break;
//        case SND_PCM_FORMAT_S16_BE:
////          codec_id = AV_CODEC_ID_PCM_S16BE;
////          sample_format = AV_SAMPLE_FMT_S16;
//          break;
//        case SND_PCM_FORMAT_U16_LE:
////          codec_id = AV_CODEC_ID_PCM_U16LE;
////          sample_format = AV_SAMPLE_FMT_S16;
//          break;
//        case SND_PCM_FORMAT_U16_BE:
////          codec_id = AV_CODEC_ID_PCM_U16BE;
////          sample_format = AV_SAMPLE_FMT_S16;
//          break;
//        case SND_PCM_FORMAT_S8:
////          codec_id = AV_CODEC_ID_PCM_S8;
////          sample_format = AV_SAMPLE_FMT_U8;
//          break;
//        case SND_PCM_FORMAT_U8:
////          codec_id = AV_CODEC_ID_PCM_U8;
////          sample_format = AV_SAMPLE_FMT_U8;
//          break;
//        case SND_PCM_FORMAT_MU_LAW:
////          codec_id = AV_CODEC_ID_PCM_MULAW;
//          break;
//        case SND_PCM_FORMAT_A_LAW:
////          codec_id = AV_CODEC_ID_PCM_ALAW;
//          break;
//        case SND_PCM_FORMAT_S32_LE:
////          codec_id = AV_CODEC_ID_PCM_S32LE;
////          sample_format = AV_SAMPLE_FMT_S32;
//          break;
//        case SND_PCM_FORMAT_S32_BE:
////          codec_id = AV_CODEC_ID_PCM_S32BE;
////          sample_format = AV_SAMPLE_FMT_S32;
//          break;
//        case SND_PCM_FORMAT_U32_LE:
////          codec_id = AV_CODEC_ID_PCM_U32LE;
////          sample_format = AV_SAMPLE_FMT_S32;
//          break;
//        case SND_PCM_FORMAT_U32_BE:
////          codec_id = AV_CODEC_ID_PCM_U32BE;
////          sample_format = AV_SAMPLE_FMT_S32;
//          break;
//        case SND_PCM_FORMAT_S24_LE:
////          codec_id = AV_CODEC_ID_PCM_S24LE;
//          break;
//        case SND_PCM_FORMAT_S24_BE:
////          codec_id = AV_CODEC_ID_PCM_S24BE;
//          break;
//        case SND_PCM_FORMAT_U24_LE:
////          codec_id = AV_CODEC_ID_PCM_U24LE;
//          break;
//        case SND_PCM_FORMAT_U24_BE:
////          codec_id = AV_CODEC_ID_PCM_U24BE;
//          break;
//        case SND_PCM_FORMAT_FLOAT_LE:
////          codec_id = AV_CODEC_ID_PCM_F32LE;
////          sample_format = AV_SAMPLE_FMT_FLT;
//          break;
//        case SND_PCM_FORMAT_FLOAT_BE:
////          codec_id = AV_CODEC_ID_PCM_F32BE;
////          sample_format = AV_SAMPLE_FMT_FLT;
//          break;
//        case SND_PCM_FORMAT_FLOAT64_LE:
////          codec_id = AV_CODEC_ID_PCM_F64LE;
////          sample_format = AV_SAMPLE_FMT_FLT;
//          break;
//        case SND_PCM_FORMAT_FLOAT64_BE:
////          codec_id = AV_CODEC_ID_PCM_F64BE;
////          sample_format = AV_SAMPLE_FMT_FLT;
//          break;
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown audio frame format (was: %d), returning\n"),
//                      format));
//          goto error;
//        }
//      } // end SWITCH
////      inherited::formatContext_->oformat->video_codec = codec_id;

////      codec_p = avcodec_find_encoder (codec_id);
////      if (!codec_p)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("avcodec_find_encoder(%d) failed: \"%m\", returning\n"),
////                    codec_id));
////        goto error;
////      } // end IF
////      ACE_ASSERT (!codec_context_p);
////      codec_context_p = avcodec_alloc_context3 (codec_p);
////      if (!codec_context_p)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("avcodec_alloc_context3() failed: \"%m\", returning\n")));
////        goto error;
////      } // end IF
////      result = avcodec_get_context_defaults3 (codec_context_p,
////                                              codec_p);
////      if (result < 0)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("avcodec_get_context_defaults3() failed: \"%s\", returning\n"),
////                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
////        goto error;
////      } // end IF

//      result =
//          snd_pcm_hw_params_get_channels (inherited::configuration_->format,
//                                          &channels);
//      ACE_ASSERT (result >= 0);
//      result = snd_pcm_hw_params_get_rate (inherited::configuration_->format,
//                                           &sample_rate, &subunit_direction);
//      ACE_ASSERT (result >= 0);
////      codec_context_p->bit_rate =
////          (snd_pcm_format_width (format) * channels * sample_rate);
////      codec_context_p->channels = channels;
////      codec_context_p->sample_fmt = sample_format;
////      codec_context_p->sample_rate = sample_rate;
////      codec_context_p->codec_id = codec_id;

////      result = avcodec_open2 (codec_context_p,
////                              codec_p,
////                              NULL);
////      if (result < 0)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("avcodec_open2(%d) failed: \"%s\", returning\n"),
////                    codec_id,
////                    ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
////        goto error;
////      } // end IF

////      ACE_ASSERT (!inherited::formatContext_->streams);
////      stream_p = avformat_new_stream (inherited::formatContext_,
////                                      codec_p);
////      if (!stream_p)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("avformat_new_stream() failed: \"%m\", returning\n")));
////        goto error;
////      } // end IF
////      ACE_ASSERT (stream_p->codec);
////      inherited::formatContext_->streams[0] = stream_p;

////      // *TODO*: why does this need to be reset ?
////      codec_context_p->bit_rate =
////          (snd_pcm_format_width (format) * channels * sample_rate);
////      stream_p->codec->codec_id = codec_id;

//      goto continue_;

//error:
////      if (codec_context_p)
////        avcodec_free_context (&codec_context_p);

//      break;

//continue_:
//#endif

//      break;
//    }
//    case STREAM_SESSION_MESSAGE_END:
//    {
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      bool close_file = false;
//      ACE_FILE_IO file_IO;
//      if (!Common_File_Tools::open (session_data_r.targetFileName, // FQ file name
//                                    O_RDWR | O_BINARY,             // flags
//                                    file_IO))                      // return value: stream
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Common_File_Tools::open(\"%s\"), returning\n"),
//                    ACE_TEXT (session_data_r.targetFileName.c_str ())));
//        break;
//      } // end IF
//      close_file = true;

//      // sanity check(s)
//      ACE_ASSERT (session_data_r.format);

//      struct _AMMediaType* media_type_p = NULL;
//      unsigned char* wave_header_p = NULL;
//      unsigned int wave_header_size = 0;

//      media_type_p = inherited::getFormat (session_data_r.format);
//      if (!media_type_p)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to retrieve media type, returning\n")));
//        break;
//      } // end IF
//      ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

//      wave_header_size =
//        (sizeof (struct _rifflist)  +
//         sizeof (struct _riffchunk) +
//         media_type_p->cbFormat     +
//         sizeof (struct _riffchunk));
//      ACE_NEW_NORETURN (wave_header_p,
//                        unsigned char[wave_header_size]);
//      if (!wave_header_p)
//      {
//        ACE_DEBUG ((LM_CRITICAL,
//                    ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
//        goto error_2;
//      } // end IF
//      ssize_t result_2 = file_IO.recv_n (wave_header_p, wave_header_size);
//      if (result_2 != wave_header_size)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_FILE_IO::recv_n(%d): \"%m\", returning\n"),
//                    wave_header_size));
//        goto error_2;
//      } // end IF

//      struct _rifflist* RIFF_wave_p =
//        reinterpret_cast<struct _rifflist*> (wave_header_p);
//      struct _riffchunk* RIFF_chunk_fmt_p =
//        reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
//      struct _riffchunk* RIFF_chunk_data_p =
//        reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
//                                              media_type_p->cbFormat);

//      // update RIFF header sizes
//      RIFF_chunk_data_p->cb = session_data_r.currentStatistic.bytes;
//      RIFF_wave_p->cb = session_data_r.currentStatistic.bytes +
//                        wave_header_size         -
//                        sizeof (struct _riffchunk);

//      result_2 = file_IO.send_n (wave_header_p, wave_header_size);
//      if (result_2 != wave_header_size)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_FILE_IO::send_n(%d): \"%m\", returning\n"),
//                    wave_header_size));
//        goto error_2;
//      } // end IF

//      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
//      delete [] wave_header_p;
//      result = file_IO.close ();
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
//      close_file = false;

//      goto continue_2;

//error_2:
//      if (media_type_p)
//        Stream_Module_Device_Tools::deleteMediaType (media_type_p);
//      if (wave_header_p)
//        delete [] wave_header_p;
//      if (close_file)
//      {
//        result = file_IO.close ();
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
//      } // end IF

//continue_2:
//#else
////      WAVContext* WAV_context_p = inherited::formatContext_->priv_data;
////      AVIOContext* IO_context_p = inherited::formatContext_->pb;
////      int64_t fmt, data;//, fact;

////      if (inherited::formatContext_->nb_streams != 1)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("WAVE files have exactly one stream (was: %d), aborting\n"),
////                    inherited::formatContext_->nb_streams));
////        goto error_2;
////      } // end IF

////      if (WAV_context_p->rf64 == RF64_ALWAYS)
////      {
////        ffio_wfourcc (IO_context_p, "RF64");
////        avio_wl32 (IO_context_p, -1); /* RF64 chunk size: use size in ds64 */
////      } // end IF
////      else
////      {
////        ffio_wfourcc (IO_context_p, "RIFF");
////        avio_wl32 (IO_context_p, 0); /* file length */
////      } // end ELSE
////      ffio_wfourcc (IO_context_p, "WAVE");

////      if (WAV_context_p->rf64 != RF64_NEVER)
////      {
////        /* write empty ds64 chunk or JUNK chunk to reserve space for ds64 */
////        ffio_wfourcc (IO_context_p,
////                      ((WAV_context_p->rf64 == RF64_ALWAYS) ? "ds64"
////                                                            : "JUNK"));
////        avio_wl32 (IO_context_p, 28); /* chunk size */
////        WAV_context_p->ds64 = avio_tell (IO_context_p);
////        ffio_fill (IO_context_p, 0, 28);
////      } // end IF

////      /* format header */
////      fmt = ff_start_tag (IO_context_p, "fmt ");
////      if (ff_put_wav_header (IO_context_p,
////                             inherited::formatContext_->streams[0]->codec) < 0)
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("%s codec not supported in WAVE format, aborting\n"),
////                    (inherited::formatContext_->streams[0]->codec->codec ? inherited::formatContext_->streams[0]->codec->codec->name
////                                                                         : "NONE")));
////        goto error_2;
////      } // end IF
////      ff_end_tag (IO_context_p, fmt);

////      if ((inherited::formatContext_->streams[0]->codec->codec_tag != 0x01) && /* hence for all other than PCM */
////          IO_context_p->seekable)
////      {
////        WAV_context_p->fact_pos = ff_start_tag (IO_context_p, "fact");
////        avio_wl32 (IO_context_p, 0);
////        ff_end_tag (IO_context_p, WAV_context_p->fact_pos);
////      } // end IF

////      if (WAV_context_p->write_bext)
////        bwf_write_bext_chunk (inherited::formatContext_);

////      av_set_pts_info (inherited::formatContext_->streams[0],
////                       64,
////                       1,
////                       inherited::formatContext_->streams[0]->codec->sample_rate);
////      WAV_context_p->maxpts = WAV_context_p->last_duration = 0;
////      WAV_context_p->minpts = INT64_MAX;

////      /* info header */
////      ff_riff_write_info (inherited::formatContext_);

////      /* data header */
////      WAV_context_p->data = ff_start_tag (IO_context_p, "data");
////      data = ff_start_tag (IO_context_p, "data");

////      avio_flush (IO_context_p);

//error_2:
////      av_free (WAV_context_p);
////      avformat_free_context (inherited::formatContext_);
////      inherited::formatContext_ = NULL;
//#endif

//      break;
//    }
//    default:
//      break;
//  } // end SWITCH
//}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType,
//          typename SessionDataType>
//bool
//Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
//                            TimePolicyType,
//                            ConfigurationType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            SessionDataContainerType,
//                            SessionDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::generateHeader"));

//  // sanity check(s)
//  ACE_ASSERT (inherited::sessionData_);
//  ACE_ASSERT (messageBlock_inout);

//  int result = -1;
//  const SessionDataType& session_data_r = inherited::sessionData_->get ();
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  // sanity check(s)
//  ACE_ASSERT (session_data_r.format);

//  struct _AMMediaType* media_type_p = NULL;
//  media_type_p = inherited::getFormat (session_data_r.format);
//  if (!media_type_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to retrieve media type, aborting\n")));
//    return false;
//  } // end IF
//  ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

//  //ACE_ASSERT (media_type_p->pbFormat);
//  //struct tWAVEFORMATEX* waveformatex_p =
//  //  reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
//  //ACE_ASSERT (waveformatex_p);

//  struct _rifflist* RIFF_wave_p =
//    reinterpret_cast<struct _rifflist*> (messageBlock_inout->wr_ptr ());
//  struct _riffchunk* RIFF_chunk_fmt_p =
//    reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
//  struct _riffchunk* RIFF_chunk_data_p =
//    reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
//                                          media_type_p->cbFormat);

//  RIFF_chunk_data_p->fcc = FCC ('data');
//  RIFF_chunk_data_p->cb = 0; // update here

//  RIFF_chunk_fmt_p->fcc = FCC ('fmt ');
//  RIFF_chunk_fmt_p->cb = media_type_p->cbFormat;
//  ACE_OS::memcpy (RIFF_chunk_fmt_p + 1,
//                  media_type_p->pbFormat,
//                  RIFF_chunk_fmt_p->cb);

//  RIFF_wave_p->fcc = FCC ('RIFF');
//  RIFF_wave_p->cb = 0 + // ... and here
//                    (sizeof (struct _rifflist)  +
//                     sizeof (struct _riffchunk) +
//                     media_type_p->cbFormat     +
//                     sizeof (struct _riffchunk)) -
//                     sizeof (struct _riffchunk);
//  RIFF_wave_p->fccListType = FCC ('WAVE');

//  messageBlock_inout->wr_ptr (RIFF_wave_p->cb + sizeof (struct _riffchunk));

//  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

//  goto continue_;

//error:
//  return false;

//continue_:
//#else
//  ACE_ASSERT (!inherited::formatContext_->pb);
//  inherited::formatContext_->pb =
//    avio_alloc_context (reinterpret_cast<unsigned char*> (messageBlock_inout->wr_ptr ()), // buffer handle
//                        messageBlock_inout->capacity (),          // buffer size
//                        1,                                        // write flag
//                        messageBlock_inout,                       // act
//                        NULL,                                     // read callback
//                        stream_decoder_aviencoder_libav_write_cb, // write callback
//                        NULL);                                    // seek callback
//  if (!inherited::formatContext_->pb)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("avio_alloc_context() failed: \"%m\", aborting\n")));
//    return false;
//  } // end IF

//  result = avformat_write_header (inherited::formatContext_, // context handle
//                                  NULL);                     // options
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("avformat_write_header() failed: \"%s\", aborting\n"),
//                ACE_TEXT (Stream_Module_Decoder_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//  avio_flush (inherited::formatContext_->pb);
//#endif

//  return true;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType>
void
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->get ());
  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//      enum _snd_pcm_format format = SND_PCM_FORMAT_UNKNOWN;
//      unsigned int channels = 0;
//      unsigned int sample_rate = 0;
//      int subunit_direction = 0;
////      int sndfile_format = 0;

//      result = snd_pcm_hw_params_get_format (inherited::configuration_->format,
//                                             &format);
//      ACE_ASSERT (result >= 0);
//      switch (format)
//      {
//        // PCM 'formats'
//        case SND_PCM_FORMAT_S16_LE:
////          sndfile_format = SF_FORMAT_PCM_16;
//          break;
//        case SND_PCM_FORMAT_S16_BE:
////          sndfile_format = SF_FORMAT_PCM_16;
//          break;
//        case SND_PCM_FORMAT_U16_LE:
////          sndfile_format = SF_FORMAT_PCM_16;
//          break;
//        case SND_PCM_FORMAT_U16_BE:
////          sndfile_format = SF_FORMAT_PCM_16;
//          break;
//        case SND_PCM_FORMAT_S8:
////          sndfile_format = SF_FORMAT_PCM_S8;
//          break;
//        case SND_PCM_FORMAT_U8:
////          sndfile_format = SF_FORMAT_PCM_U8;
//          break;
//        case SND_PCM_FORMAT_MU_LAW:
////          sndfile_format = SF_FORMAT_ULAW;
//          break;
//        case SND_PCM_FORMAT_A_LAW:
////          sndfile_format = SF_FORMAT_ALAW;
//          break;
//        case SND_PCM_FORMAT_S32_LE:
////          sndfile_format = SF_FORMAT_PCM_32;
//          break;
//        case SND_PCM_FORMAT_S32_BE:
////          sndfile_format = SF_FORMAT_PCM_32;
//          break;
//        case SND_PCM_FORMAT_U32_LE:
////          sndfile_format = SF_FORMAT_PCM_32;
//          break;
//        case SND_PCM_FORMAT_U32_BE:
////          sndfile_format = SF_FORMAT_PCM_32;
//          break;
//        case SND_PCM_FORMAT_S24_LE:
////          sndfile_format = SF_FORMAT_PCM_24;
//          break;
//        case SND_PCM_FORMAT_S24_BE:
////          sndfile_format = SF_FORMAT_PCM_24;
//          break;
//        case SND_PCM_FORMAT_U24_LE:
////          sndfile_format = SF_FORMAT_PCM_24;
//          break;
//        case SND_PCM_FORMAT_U24_BE:
////          sndfile_format = SF_FORMAT_PCM_24;
//          break;
//        case SND_PCM_FORMAT_FLOAT_LE:
////          sndfile_format = SF_FORMAT_FLOAT;
//          break;
//        case SND_PCM_FORMAT_FLOAT_BE:
////          sndfile_format = SF_FORMAT_FLOAT;
//          break;
//        case SND_PCM_FORMAT_FLOAT64_LE:
////          sndfile_format = SF_FORMAT_DOUBLE;
//          break;
//        case SND_PCM_FORMAT_FLOAT64_BE:
////          sndfile_format = SF_FORMAT_DOUBLE;
//          break;
//        default:
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("invalid/unknown audio frame format (was: %d), aborting\n"),
//                      format));
//          goto error;
//        }
//      } // end SWITCH

//      result =
//          snd_pcm_hw_params_get_channels (inherited::configuration_->format,
//                                          &channels);
//      ACE_ASSERT (result >= 0);
//      result = snd_pcm_hw_params_get_rate (inherited::configuration_->format,
//                                           &sample_rate, &subunit_direction);
//      ACE_ASSERT (result >= 0);

//      ACE_ASSERT (!SNDFile_);
//      SNDFile_ = sf_open (inherited::configuration_->targetFileName.c_str (),
//                          SFM_WRITE,
//                          &SFInfo_);
//      if (!SNDFile_)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to sf_open(\"%s\"): \"%s\", aborting\n"),
//                    ACE_TEXT (inherited::configuration_->targetFileName.c_str ()),
//                    ACE_TEXT (sf_strerror (NULL))));
//        goto error;
//      } // end IF

//      result = sf_format_check (&SFInfo_);
//      if (!result)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to sf_format_check(): \"%s\", aborting\n"),
//                    ACE_TEXT (sf_strerror (&SFInfo_))));
//        goto error;
//      } // end IF

//      sox_comments_t comments = ;
      struct sox_oob_t oob_data;
      ACE_OS::memset (&oob_data, 0, sizeof (struct sox_oob_t));
//      oob_data.comments = comments;
//      oob_data.instr;
//      oob_data.loops;
      Stream_Module_Decoder_Tools::ALSA2SOX (inherited::configuration_->format,
                                             encodingInfo_,
                                             signalInfo_);
      ACE_ASSERT (!outputFile_);
      outputFile_ =
          sox_open_write (inherited::configuration_->targetFileName.c_str (),
                          &signalInfo_,
                          &encodingInfo_,
                          ACE_TEXT_ALWAYS_CHAR (STREAM_DECODER_SOX_WAV_FORMATTYPE_STRING),
                          &oob_data,
                          sox_overwrite_permitted);
      if (!outputFile_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to sox_open_write(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (inherited::configuration_->targetFileName.c_str ())));
        goto error;
      } // end IF

      goto continue_;

error:
//      if (SNDFile_)
//      {
//        result = sf_close (SNDFile_);
//        if (!result)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to sf_close(): \"%s\", continuing\n"),
//                      ACE_TEXT (sf_strerror (&SFInfo_))));
//        SNDFile_ = NULL;
//      } // end IF

      if (outputFile_)
      {
        result = sox_close (outputFile_);
        if (result != SOX_SUCCESS)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sox_close(), continuing\n")));
        outputFile_ = NULL;
      } // end IF

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
#endif

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      bool close_file = false;
      ACE_FILE_IO file_IO;
      if (!Common_File_Tools::open (session_data_r.targetFileName, // FQ file name
                                    O_RDWR | O_BINARY,             // flags
                                    file_IO))                      // return value: stream
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::open(\"%s\"), returning\n"),
                    ACE_TEXT (session_data_r.targetFileName.c_str ())));
        break;
      } // end IF
      close_file = true;

      // sanity check(s)
      ACE_ASSERT (session_data_r.format);

      struct _AMMediaType* media_type_p = NULL;
      unsigned char* wave_header_p = NULL;
      unsigned int wave_header_size = 0;

      media_type_p = inherited::getFormat (session_data_r.format);
      if (!media_type_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to retrieve media type, returning\n")));
        break;
      } // end IF
      ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

      wave_header_size =
        (sizeof (struct _rifflist)  +
         sizeof (struct _riffchunk) +
         media_type_p->cbFormat     +
         sizeof (struct _riffchunk));
      ACE_NEW_NORETURN (wave_header_p,
                        unsigned char[wave_header_size]);
      if (!wave_header_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
        goto error_2;
      } // end IF
      ssize_t result_2 = file_IO.recv_n (wave_header_p, wave_header_size);
      if (result_2 != wave_header_size)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_IO::recv_n(%d): \"%m\", returning\n"),
                    wave_header_size));
        goto error_2;
      } // end IF

      struct _rifflist* RIFF_wave_p =
        reinterpret_cast<struct _rifflist*> (wave_header_p);
      struct _riffchunk* RIFF_chunk_fmt_p =
        reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
      struct _riffchunk* RIFF_chunk_data_p =
        reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
                                              media_type_p->cbFormat);

      // update RIFF header sizes
      RIFF_chunk_data_p->cb = session_data_r.currentStatistic.bytes;
      RIFF_wave_p->cb = session_data_r.currentStatistic.bytes +
                        wave_header_size         -
                        sizeof (struct _riffchunk);

      result_2 = file_IO.send_n (wave_header_p, wave_header_size);
      if (result_2 != wave_header_size)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_IO::send_n(%d): \"%m\", returning\n"),
                    wave_header_size));
        goto error_2;
      } // end IF

      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      delete [] wave_header_p;
      result = file_IO.close ();
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
      close_file = false;

      goto continue_2;

error_2:
      if (media_type_p)
        Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      if (wave_header_p)
        delete [] wave_header_p;
      if (close_file)
      {
        result = file_IO.close ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_FILE_IO::close(): \"%m\", continuing\n")));
      } // end IF

continue_2:
#else
//      if (SNDFile_)
//      {
//        result = sf_close (SNDFile_);
//        if (!result)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to sf_close(): \"%s\", continuing\n"),
//                      ACE_TEXT (sf_strerror (&SFInfo_))));
//        SNDFile_ = NULL;
//      } // end IF

      if (outputFile_)
      {
        result = sox_close (outputFile_);
        if (result != SOX_SUCCESS)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to sox_close(), continuing\n")));
        outputFile_ = NULL;
      } // end IF

//error_2:
#endif

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
          typename SessionDataType>
bool
Stream_Decoder_WAVEncoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType,
                            SessionDataType>::generateHeader (ACE_Message_Block* messageBlock_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WAVEncoder_T::generateHeader"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (messageBlock_inout);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  int result = -1;
  const SessionDataType& session_data_r = inherited::sessionData_->get ();

  // sanity check(s)
  ACE_ASSERT (session_data_r.format);

  struct _AMMediaType* media_type_p = NULL;
  media_type_p = inherited::getFormat (session_data_r.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve media type, aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

  //ACE_ASSERT (media_type_p->pbFormat);
  //struct tWAVEFORMATEX* waveformatex_p =
  //  reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
  //ACE_ASSERT (waveformatex_p);

  struct _rifflist* RIFF_wave_p =
    reinterpret_cast<struct _rifflist*> (messageBlock_inout->wr_ptr ());
  struct _riffchunk* RIFF_chunk_fmt_p =
    reinterpret_cast<struct _riffchunk*> (RIFF_wave_p + 1);
  struct _riffchunk* RIFF_chunk_data_p =
    reinterpret_cast<struct _riffchunk*> (reinterpret_cast<BYTE*> (RIFF_chunk_fmt_p + 1) +
                                          media_type_p->cbFormat);

  RIFF_chunk_data_p->fcc = FCC ('data');
  RIFF_chunk_data_p->cb = 0; // update here

  RIFF_chunk_fmt_p->fcc = FCC ('fmt ');
  RIFF_chunk_fmt_p->cb = media_type_p->cbFormat;
  ACE_OS::memcpy (RIFF_chunk_fmt_p + 1,
                  media_type_p->pbFormat,
                  RIFF_chunk_fmt_p->cb);

  RIFF_wave_p->fcc = FCC ('RIFF');
  RIFF_wave_p->cb = 0 + // ... and here
                    (sizeof (struct _rifflist)  +
                     sizeof (struct _riffchunk) +
                     media_type_p->cbFormat     +
                     sizeof (struct _riffchunk)) -
                     sizeof (struct _riffchunk);
  RIFF_wave_p->fccListType = FCC ('WAVE');

  messageBlock_inout->wr_ptr (RIFF_wave_p->cb + sizeof (struct _riffchunk));

  Stream_Module_Device_Tools::deleteMediaType (media_type_p);

  goto continue_;

error:
  return false;

continue_:
#else
#endif

  return true;
}
