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
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
}
#endif /* __cplusplus */

#include "ace/Log_Msg.h"

#include "common_file_tools.h"
#include "common_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"
#include "stream_dec_tools.h"

#include "stream_lib_ffmpeg_common.h"

template <typename TaskType,
          typename MediaType>
Stream_Decoder_LibAVConverter_T<TaskType,
                                MediaType>::Stream_Decoder_LibAVConverter_T (typename TaskType::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , buffer_ (NULL)
 , context_ (NULL)
 , frame_ (NULL)
 , frameSize_ (0)
 , inputFormat_ (AV_PIX_FMT_NONE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::Stream_Decoder_LibAVConverter_T"));

}

template <typename TaskType,
          typename MediaType>
Stream_Decoder_LibAVConverter_T<TaskType,
                                MediaType>::~Stream_Decoder_LibAVConverter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::~Stream_Decoder_LibAVConverter_T"));

  if (unlikely (buffer_))
    buffer_->release ();

  if (unlikely (context_))
    sws_freeContext (context_);

  if (unlikely (frame_))
  {
    av_frame_unref (frame_);
    av_frame_free (&frame_);
  } // end IF
}

template <typename TaskType,
          typename MediaType>
bool
Stream_Decoder_LibAVConverter_T<TaskType,
                                MediaType>::initialize (const typename TaskType::CONFIGURATION_T& configuration_in,
                                                        Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF

    if (context_)
    {
      sws_freeContext (context_); context_ = NULL;
    } // end IF

    if (frame_)
    {
      av_frame_unref (frame_);
      av_frame_free (&frame_); ACE_ASSERT (frame_ == NULL);
    } // end IF

    frameSize_ = 0;
    inputFormat_ = AV_PIX_FMT_NONE;
  } // end IF

#if defined (_DEBUG)
  if (configuration_in.debug)
  {
    av_log_set_callback (stream_decoder_libav_log_cb);
    // *NOTE*: this level logs all messages
    av_log_set_level (std::numeric_limits<int>::max ());
  } // end IF
#endif // _DEBUG

//continue_:
  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename TaskType,
          typename MediaType>
void
Stream_Decoder_LibAVConverter_T<TaskType,
                                MediaType>::handleDataMessage (typename TaskType::DATA_MESSAGE_T*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  int line_sizes_a[AV_NUM_DATA_POINTERS];
  uint8_t* data_a[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (&data_a, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));

  // sanity check(s)
  ACE_ASSERT (buffer_ && context_ && frame_);
//  ACE_ASSERT (buffer_->capacity () >= frameSize_);

  result = av_image_fill_linesizes (line_sizes_a,
                                    inputFormat_,
                                    static_cast<int> (frame_->width));
  ACE_ASSERT (result >= 0);
  result =
    av_image_fill_pointers (data_a,
                            inputFormat_,
                            static_cast<int> (frame_->height),
                            reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                            line_sizes_a);
  ACE_ASSERT (result >= 0);
  if (unlikely (!Stream_Module_Decoder_Tools::convert (context_,
                                                       frame_->width, frame_->height, inputFormat_,
                                                       data_a,
                                                       frame_->width, frame_->height, static_cast<AVPixelFormat> (frame_->format),
                                                       frame_->data,
                                                       inherited::configuration_->flipImage)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_->wr_ptr (frameSize_);
  buffer_->initialize (message_inout->sessionId (),
                       NULL);
  buffer_->set (message_inout->type ());
  message_inout->release (); message_inout = NULL;

  // forward the converted frame
  result = inherited::put_next (buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_ = NULL;

  // allocate a message buffer for the next frame
  buffer_ = inherited::allocateMessage (frameSize_);
  if (unlikely (!buffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                frameSize_));
    goto error;
  } // end IF
//  av_frame_unref (frame_);
  result = av_image_fill_linesizes (frame_->linesize,
                                    static_cast<AVPixelFormat> (frame_->format),
                                    static_cast<int> (frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (frame_->data,
                              static_cast<AVPixelFormat> (frame_->format),
                              static_cast<int> (frame_->height),
                              reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                              frame_->linesize);
  ACE_ASSERT (result >= 0);

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <typename TaskType,
          typename MediaType>
void
Stream_Decoder_LibAVConverter_T<TaskType,
                                MediaType>::handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  const typename TaskType::SESSION_MESSAGE_T::DATA_T& session_data_container_r =
    message_inout->getR ();
  typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
    const_cast<typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T&> (session_data_container_r.getR ());
  // *TODO*: remove type inference
  ACE_ASSERT (!session_data_r.formats.empty ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      MediaType media_type_2;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_3;
      int flags = 0;
      int result = -1;

      // sanity check(s)
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      if (unlikely (Stream_Module_Decoder_Tools::isCompressedVideo (media_type_s.format)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: input format is compressed (was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_s.format).c_str ())));
        goto error;
      } // end IF
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);
      inputFormat_ = media_type_s.format;
      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_3);
      if (unlikely ((inputFormat_ == media_type_3.format) &&
                    !inherited::configuration_->flipImage))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: output format (was: \"%s\") is input format, nothing to do\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_3.format).c_str ())));
        goto continue_2; // nothing to do
      } // end iF

      // initialize conversion context
      ACE_ASSERT (!context_);
      flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
        SWS_FULL_CHR_H_INP | SWS_BICUBIC | SWS_ACCURATE_RND | SWS_BITEXACT);
      context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                media_type_s.resolution.cx, media_type_s.resolution.cy, inputFormat_,
                                media_type_s.resolution.cx, media_type_s.resolution.cy, media_type_3.format,
#else
                                media_type_s.resolution.width, media_type_s.resolution.height, inputFormat_,
                                media_type_s.resolution.width, media_type_s.resolution.height, media_type_3.format,
#endif // ACE_WIN32 || ACE_WIN64
                                flags,                        // flags
                                NULL, NULL,
                                0);                           // parameters
      if (unlikely (!context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // sanity check(s)
      if (unlikely (media_type_3.format == AV_PIX_FMT_NONE))
        goto continue_;
      if (likely (inputFormat_ != media_type_3.format))
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: converting pixel format %s to %s\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inputFormat_).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_3.format).c_str ())));

      // initialize frame buffer
      ACE_ASSERT (!frame_);
      frame_ = av_frame_alloc ();
      if (unlikely (!frame_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      frame_->format = media_type_3.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      frame_->height = media_type_s.resolution.cy;
      frame_->width = media_type_s.resolution.cx;
#else
      frame_->height = media_type_s.resolution.height;
      frame_->width = media_type_s.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64

      ACE_ASSERT (!buffer_);
      frameSize_ =
        av_image_get_buffer_size (static_cast<enum AVPixelFormat> (frame_->format),
                                  frame_->width, frame_->height,
                                  1); // *TODO*: linesize alignment
      ACE_ASSERT (frameSize_ >= 0);
      buffer_ = inherited::allocateMessage (frameSize_);
      if (unlikely (!buffer_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    frameSize_));
        goto error;
      } // end IF
      ACE_ASSERT (buffer_->capacity () >= frameSize_);
      result =
          av_image_fill_linesizes (frame_->linesize,
                                   static_cast<enum AVPixelFormat> (frame_->format),
                                   static_cast<int> (frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (frame_->data,
                                  static_cast<enum AVPixelFormat> (frame_->format),
                                  static_cast<int> (frame_->height),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  frame_->linesize);
      ACE_ASSERT (result >= 0);
      ACE_UNUSED_ARG (result);

continue_:
      if (inputFormat_ != media_type_3.format)
      { ACE_ASSERT (session_data_r.lock);
        inherited2::setFormat (media_type_3.format,
                               media_type_2);
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
          session_data_r.formats.push_back (media_type_2);
        } // end lock scope
      } // end IF

continue_2:
      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      int result = -1;

      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      if (!inherited::configuration_->handleResize)
        break;

      ACE_ASSERT (inherited::sessionData_);
      typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
        const_cast<typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);

      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      inherited2::getMediaType (inherited::configuration_->outputFormat,
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);

      // inputFormat_ = media_type_2.format;

      ACE_ASSERT (frame_);
      frame_->format = media_type_s.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      frame_->height = media_type_2.resolution.cy;
      frame_->width = media_type_2.resolution.cx;
#else
      frame_->height = media_type_2.resolution.height;
      frame_->width = media_type_2.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64

      frameSize_ =
        av_image_get_buffer_size (static_cast<AVPixelFormat> (frame_->format),
                                  frame_->width, frame_->height,
                                  1); // *TODO*: linesize alignment

      if (context_)
      {
        sws_freeContext (context_); context_ = NULL;
      } // end IF

      int flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                   SWS_FULL_CHR_H_INP | SWS_BICUBIC | SWS_ACCURATE_RND | SWS_BITEXACT);
      context_ =
          sws_getCachedContext (NULL,
                                frame_->width, frame_->height, inputFormat_,
                                frame_->width, frame_->height, static_cast<AVPixelFormat> (frame_->format),
                                flags,                        // flags
                                NULL, NULL,
                                0);                           // parameters
      if (!context_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // initialize frame buffer
      if (buffer_)
      {
        buffer_->release (); buffer_ = NULL;
      } // end IF

      buffer_ = inherited::allocateMessage (frameSize_);
      if (!buffer_)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    frameSize_));
        goto error_2;
      } // end IF
      result = av_image_fill_linesizes (frame_->linesize,
                                        static_cast<AVPixelFormat> (frame_->format),
                                        static_cast<int> (frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (frame_->data,
                                  static_cast<AVPixelFormat> (frame_->format),
                                  static_cast<int> (frame_->height),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  frame_->linesize);
      ACE_ASSERT (result >= 0);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: modified resolution to %ux%u\n"),
                  inherited::mod_->name (),
                  frame_->width, frame_->height));
      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        buffer_->release (); buffer_ = NULL;
      } // end IF
      if (context_)
      {
        sws_freeContext (context_); context_ = NULL;
      } // end IF
      if (frame_)
      {
        av_frame_unref (frame_);
        av_frame_free (&frame_);
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

template <typename TaskType,
          typename MediaType>
Stream_Decoder_LibAVConverter1_T<TaskType,
                                 MediaType>::Stream_Decoder_LibAVConverter1_T (typename TaskType::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter1_T::Stream_Decoder_LibAVConverter1_T"));

}

template <typename TaskType,
          typename MediaType>
bool
Stream_Decoder_LibAVConverter1_T<TaskType,
                                 MediaType>::initialize (const typename TaskType::CONFIGURATION_T& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter1_T::initialize"));

//  int result = -1;
//  int flags = 0;

  if (unlikely (inherited::isInitialized_))
  {

  } // end IF

#if defined (_DEBUG)
//  av_log_set_callback (Stream_Decoder_LibAVDecoder_LoggingCB);
  // *NOTE*: this level logs all messages
//  av_log_set_level (std::numeric_limits<int>::max ());
#endif // _DEBUG

//continue_:
  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <typename TaskType,
          typename MediaType>
void
Stream_Decoder_LibAVConverter1_T<TaskType,
                                 MediaType>::handleDataMessage (typename TaskType::DATA_MESSAGE_T*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter1_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  typename TaskType::DATA_MESSAGE_T* message_p = NULL;
  struct SwsContext* context_p = NULL;
//  unsigned char* data_p = NULL;
  int flags_i = 0;
  size_t size_i = 0;//, size_2 = 0;
  AVFrame* frame_p = NULL;
  typename TaskType::DATA_MESSAGE_T::DATA_T& message_data_r =
      const_cast<typename TaskType::DATA_MESSAGE_T::DATA_T&> (message_inout->getR ());
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s, media_type_2;
  typename TaskType::DATA_MESSAGE_T::DATA_T message_data_2;
  int line_sizes[AV_NUM_DATA_POINTERS];
  uint8_t* data[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&line_sizes, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (&data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));

  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  }
  ACE_ASSERT (!message_inout->cont ());

  inherited2::getMediaType (message_data_r.format,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_s);
  inherited2::getMediaType (inherited::configuration_->outputFormat,
                            STREAM_MEDIATYPE_VIDEO,
                            media_type_2);
  ACE_ASSERT (inherited::configuration_);
  size_i =
      static_cast<unsigned int> (av_image_get_buffer_size (media_type_2.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                           media_type_s.resolution.cx,
                                                           media_type_s.resolution.cy,
#else
                                                           media_type_s.resolution.width,
                                                           media_type_s.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                                                           1));
  ACE_ASSERT (size_i);

  message_block_p =
    inherited::allocateMessage (static_cast<unsigned int> (size_i));
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                inherited::mod_->name (),
                size_i));
    goto error;
  } // end IF
  message_p = static_cast<typename TaskType::DATA_MESSAGE_T*> (message_block_p);
  message_data_2.format = inherited::configuration_->outputFormat;
  message_p->initialize (message_data_2,
                         message_p->sessionId (),
                         NULL);

  // initialize conversion context
  flags_i = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
  context_p =
      sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_type_s.resolution.cx, media_type_s.resolution.cy, media_type_s.format,
                            media_type_s.resolution.cx, media_type_s.resolution.cy, media_type_2.format,
#else
                            media_type_s.resolution.width, media_type_s.resolution.height, media_type_s.format,
                            media_type_s.resolution.width, media_type_s.resolution.height, inherited::configuration_->outputFormat.format,
#endif // ACE_WIN32 || ACE_WIN64
                            flags_i,                      // flags
                            NULL, NULL,
                            0);                           // parameters
  if (unlikely (!context_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  if (likely ((media_type_s.format != media_type_2.format)))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: converting pixel format %s to %s\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_s.format).c_str ()),
                ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (media_type_2.format).c_str ())));

  // initialize frame buffer
  frame_p = av_frame_alloc ();
  if (unlikely (!frame_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  frame_p->format = media_type_2.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  frame_p->height = media_type_s.resolution.cy;
  frame_p->width = media_type_s.resolution.cx;
#else
  frame_p->height = media_type_s.resolution.height;
  frame_p->width = media_type_s.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64

  result =
      av_image_fill_linesizes (frame_p->linesize,
                               static_cast<enum AVPixelFormat> (frame_p->format),
                               static_cast<int> (frame_p->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (frame_p->data,
                              static_cast<enum AVPixelFormat> (frame_p->format),
                              static_cast<int> (frame_p->height),
                              reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                              frame_p->linesize);
  ACE_ASSERT (result >= 0);

  result =
    av_image_fill_linesizes (line_sizes,
                             media_type_s.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                             static_cast<int> (media_type_s.resolution.cx));
#else
                             static_cast<int> (media_type_s.resolution.width));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (data,
                              media_type_s.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              static_cast<int> (media_type_s.resolution.cy),
#else
                              static_cast<int> (media_type_s.resolution.height),
#endif // ACE_WIN32 || ACE_WIN64
        reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                              line_sizes);
  ACE_ASSERT (result >= 0);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!Stream_Module_Decoder_Tools::convert (context_p,
                                                       media_type_s.resolution.cx, media_type_s.resolution.cy,
                                                       media_type_s.format,
                                                       data,
                                                       frame_p->width, frame_p->height, static_cast<AVPixelFormat> (frame_p->format),
                                                       frame_p->data)))
#else
  if (unlikely (!Stream_Module_Decoder_Tools::convert (context_p,
                                                       media_type_s.resolution.width, media_type_s.resolution.height,
                                                       media_type_s.format,
                                                       data,
                                                       frame_p->width, frame_p->height, static_cast<AVPixelFormat> (frame_p->format),
                                                       frame_p->data)))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  message_block_p->wr_ptr (size_i);
  message_p->set (message_inout->type ());
  message_inout->release (); message_inout = NULL;

//#if defined (_DEBUG)
//  Common_File_Tools::store (ACE_TEXT_ALWAYS_CHAR ("output.rgb"),
//                            reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ()),
//                            size_i);
//#endif // _DEBUG

  sws_freeContext (context_p); context_p = NULL;
  av_frame_free (&frame_p); frame_p = NULL;

  // forward the converted frame
  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  message_block_p = NULL;

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF
  if (frame_p)
  {
    av_frame_free (&frame_p); frame_p = NULL;
  } // end IF
  if (message_block_p)
  {
    message_block_p->release (); message_block_p = NULL;
  } // end IF
  if (context_p)
  {
    sws_freeContext (context_p); context_p = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <typename TaskType,
          typename MediaType>
void
Stream_Decoder_LibAVConverter1_T<TaskType,
                                 MediaType>::handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter1_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

//  const SessionDataContainerType& session_data_container_r =
//    message_inout->getR ();
//  typename SessionDataContainerType::DATA_T& session_data_r =
//    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      break;

//error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    default:
      break;
  } // end SWITCH
}
