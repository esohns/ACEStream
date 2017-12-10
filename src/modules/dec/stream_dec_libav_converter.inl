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

//// initialize statics
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType>
//char
//Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
//                                TimePolicyType,
//                                ConfigurationType,
//                                ControlMessageType,
//                                DataMessageType,
//                                SessionMessageType,
//                                SessionDataContainerType>::paddingBuffer[AV_INPUT_BUFFER_PADDING_SIZE];
//#else
//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType>
//char
//Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
//                                TimePolicyType,
//                                ConfigurationType,
//                                ControlMessageType,
//                                DataMessageType,
//                                SessionMessageType,
//                                SessionDataContainerType>::paddingBuffer[FF_INPUT_BUFFER_PADDING_SIZE];
//#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                SessionDataContainerType>::Stream_Decoder_LibAVConverter_T (ISTREAM_T* stream_in)
#else
                                SessionDataContainerType>::Stream_Decoder_LibAVConverter_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , buffer_ (NULL)
 , context_ (NULL)
 , frame_ (NULL)
 , frameSize_ (0)
 , inputFormat_ (AV_PIX_FMT_NONE)
 , outputFormat_ (STREAM_DECODER_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::Stream_Decoder_LibAVConverter_T"));

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  ACE_OS::memset (&(OWN_TYPE_T::paddingBuffer),
//                  0,
//                  AV_INPUT_BUFFER_PADDING_SIZE);
//#else
//  ACE_OS::memset (&(OWN_TYPE_T::paddingBuffer),
//                  0,
//                  FF_INPUT_BUFFER_PADDING_SIZE);
//#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                SessionDataContainerType>::~Stream_Decoder_LibAVConverter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::~Stream_Decoder_LibAVConverter_T"));

  if (likely (buffer_))
    buffer_->release ();

  if (likely (context_))
    sws_freeContext (context_);

  if (likely (frame_))
    av_frame_free (&frame_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF

    if (context_)
    {
      sws_freeContext (context_);
      context_ = NULL;
    } // end IF

    if (frame_)
    {
      av_frame_free (&frame_);
      frame_ = NULL;
    } // end IF

    frameSize_ = 0;
    inputFormat_ = AV_PIX_FMT_NONE;
    outputFormat_ = STREAM_DECODER_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT;
  } // end IF

#if defined (_DEBUG)
  //av_log_set_callback (Stream_Decoder_LibAVDecoder_LoggingCB);
  // *NOTE*: this level logs all messages
  //av_log_set_level (std::numeric_limits<int>::max ());
#endif

  unsigned int height, width;
  int flags = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.format);

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;

  if (likely (configuration_in.format->formattype == FORMAT_VideoInfo))
  { ACE_ASSERT (configuration_in.format->pbFormat);
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (configuration_in.format->pbFormat);
    ACE_ASSERT (video_info_header_p);

    height =
      static_cast<unsigned int> (std::abs (video_info_header_p->bmiHeader.biHeight));
    width =
      static_cast<unsigned int> (video_info_header_p->bmiHeader.biWidth);
  } // end IF
  else if (configuration_in.format->formattype == FORMAT_VideoInfo2)
  { ACE_ASSERT (configuration_in.format->pbFormat);
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (configuration_in.format->pbFormat);
    ACE_ASSERT (video_info_header2_p);

    height =
      static_cast<unsigned int> (std::abs (video_info_header2_p->bmiHeader.biHeight));
    width =
      static_cast<unsigned int> (video_info_header2_p->bmiHeader.biWidth);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (configuration_in.format->formattype).c_str ())));
    return false;
  } // end ELSE
#else
  height = configuration_in.sourceFormat.height;
  width = configuration_in.sourceFormat.width;
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inputFormat_ =
    Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (configuration_in.format->subtype);
  if (unlikely (inputFormat_ == AV_PIX_FMT_NONE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::mediaSubTypeToString (inherited::configuration_->format->subtype, false).c_str ())));
    return false;
  } // end IF
#else
  inputFormat_ = configuration_in.format;
#endif

  if (likely (inputFormat_ != outputFormat_))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: converting pixel format %s to %s\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (inputFormat_).c_str ()),
                ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (outputFormat_).c_str ())));
  else
    outputFormat_ = AV_PIX_FMT_NONE; // 'deactivate' module

  if (unlikely (outputFormat_ == AV_PIX_FMT_NONE))
    goto continue_;

  // initialize conversion context
  flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
  context_ =
      sws_getCachedContext (NULL,
                            width, height, inputFormat_,
                            width, height, outputFormat_,
                            flags,                        // flags
                            NULL, NULL,
                            0);                           // parameters
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sws_getCachedContext(): \"%m\", aborting\n")));
    return false;
  } // end IF

  // initialize frame buffer
  frame_ = av_frame_alloc ();
  if (unlikely (!frame_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
//  frame_->format = configuration_in.format;
  frame_->height = height;
  frame_->width = width;

  frameSize_ =
    av_image_get_buffer_size (outputFormat_,
                              width,
                              height,
                              1); // *TODO*: linesize alignment
  ACE_ASSERT (frameSize_ >= 0);

  buffer_ = inherited::allocateMessage (frameSize_);
  if (unlikely (!buffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                frameSize_));
    return false;
  } // end IF
  ACE_ASSERT (buffer_->capacity () >= frameSize_);
  result = av_image_fill_linesizes (frame_->linesize,
                                    outputFormat_,
                                    static_cast<int> (width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (frame_->data,
                              outputFormat_,
                              static_cast<int> (height),
                              reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                              frame_->linesize);
  ACE_ASSERT (result >= 0);

continue_:
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
Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (outputFormat_ == AV_PIX_FMT_NONE))
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
  uint8_t* data[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));

  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  }
  ACE_ASSERT (!message_inout->cont ());

  // sanity check(s)
  ACE_ASSERT (buffer_);
//  ACE_ASSERT (buffer_->capacity () >= frameSize_);
  ACE_ASSERT (frame_);

  data[0] = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  if (unlikely (!Stream_Module_Decoder_Tools::convert (context_,
                                                       frame_->width, frame_->height, inputFormat_,
                                                       data,
                                                       frame_->width, frame_->height, outputFormat_,
                                                       frame_->data)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_->wr_ptr (frameSize_);
  buffer_->initialize (message_inout->sessionId (),
                       NULL);
  buffer_->set (message_inout->type ());
  message_inout->release ();
  message_inout = NULL;

  // forward the converted frame
  result = inherited::put_next (buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_ = NULL;

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
                                    outputFormat_,
                                    static_cast<int> (frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (frame_->data,
                              outputFormat_,
                              static_cast<int> (frame_->height),
                              reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                              frame_->linesize);
  ACE_ASSERT (result >= 0);

  return;

error:
  if (message_inout)
  {
    message_inout->release ();
    message_inout = NULL;
  } // end IF

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
Stream_Decoder_LibAVConverter_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVConverter_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  const SessionDataContainerType& session_data_container_r =
    message_inout->getR ();
  typename SessionDataContainerType::DATA_T& session_data_r =
    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    { ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.format =
            ((outputFormat_ == AV_PIX_FMT_NONE) ? STREAM_DECODER_DEFAULT_LIBAV_OUTPUT_PIXEL_FORMAT
                                                : outputFormat_);
      } // end lock scope

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      int result = -1;

      unsigned int height = 0, width = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (session_data_r.format);

      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;

      if (session_data_r.format->formattype == FORMAT_VideoInfo)
      { ACE_ASSERT (session_data_r.format->pbFormat);
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.format->pbFormat);
        ACE_ASSERT (video_info_header_p);

        height =
          static_cast<unsigned int> (std::abs (video_info_header_p->bmiHeader.biHeight));
        width =
          static_cast<unsigned int> (video_info_header_p->bmiHeader.biWidth);
      } // end IF
      else if (session_data_r.format->formattype == FORMAT_VideoInfo2)
      { ACE_ASSERT (session_data_r.format->pbFormat);
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (session_data_r.format->pbFormat);
        ACE_ASSERT (video_info_header2_p);

        height =
          static_cast<unsigned int> (std::abs (video_info_header2_p->bmiHeader.biHeight));
        width =
          static_cast<unsigned int> (video_info_header2_p->bmiHeader.biWidth);
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown media type format type (was: \"%s\"), returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Device_Tools::mediaFormatTypeToString (session_data_r.format->formattype).c_str ())));
        break;
      } // end ELSE
#else
      height = session_data_r.sourceFormat.height;
      width = session_data_r.sourceFormat.width;
#endif

      if (buffer_)
      {
        buffer_->release ();
        buffer_ = NULL;
      } // end IF

      ACE_ASSERT (frame_);
      //  frame_->format = session_data_r.format;
      frame_->height = height;
      frame_->width = width;

      frameSize_ =
        av_image_get_buffer_size (outputFormat_,
                                  width,
                                  height,
                                  1); // *TODO*: linesize alignment

      if (context_)
      {
        sws_freeContext (context_);
        context_ = NULL;
      } // end IF

      int flags = (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
                   SWS_FAST_BILINEAR);
      context_ =
          sws_getCachedContext (NULL,
                                width, height, inputFormat_,
                                width, height, outputFormat_,
                                flags,                        // flags
                                NULL, NULL,
                                0);                           // parameters
      if (!context_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF

      // initialize frame buffer
      buffer_ = inherited::allocateMessage (frameSize_);
      if (!buffer_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), returning\n"),
                    inherited::mod_->name (),
                    frameSize_));
        break;
      } // end IF
      result = av_image_fill_linesizes (frame_->linesize,
                                        outputFormat_,
                                        static_cast<int> (width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (frame_->data,
                                  outputFormat_,
                                  static_cast<int> (height),
                                  reinterpret_cast<uint8_t*> (buffer_->wr_ptr ()),
                                  frame_->linesize);
      ACE_ASSERT (result >= 0);

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: modified frame resolution to %ux%u\n"),
                  inherited::mod_->name (),
                  width, height));

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
//      if (context_)
//      {
//        avcodec_free_context (&context_);
//        context_ = NULL;
//      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
