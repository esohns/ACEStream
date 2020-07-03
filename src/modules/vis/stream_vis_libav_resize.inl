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
#include "libavcodec/avcodec.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#endif /* __cplusplus */

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_tools.h"
#include "common_file_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_lib_ffmpeg_common.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Visualization_LibAVResize_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   MediaType>::Stream_Visualization_LibAVResize_T (ISTREAM_T* stream_in)
#else
                                   MediaType>::Stream_Visualization_LibAVResize_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , sourceResolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize_T::Stream_Visualization_LibAVResize_T"));

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
Stream_Visualization_LibAVResize_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (unlikely (!inherited::context_))
    return; // nothing to do

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  int line_sizes_a[AV_NUM_DATA_POINTERS];
  uint8_t* data_a[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (&data_a, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));

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
  ACE_ASSERT (inherited::buffer_);
//  ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
  ACE_ASSERT (inherited::frame_);

  result =
      av_image_fill_linesizes (line_sizes_a,
                               inherited::inputFormat_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               static_cast<int> (sourceResolution_.cx));
#else
                               static_cast<int> (sourceResolution_.width));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (data_a,
                              inherited::inputFormat_,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              static_cast<int> (sourceResolution_.cy),
#else
                              static_cast<int> (sourceResolution_.height),
#endif // ACE_WIN32 || ACE_WIN64
                              reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                              line_sizes_a);
  ACE_ASSERT (result >= 0);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     sourceResolution_.cx, sourceResolution_.cy,
                                                     inherited::inputFormat_,
                                                     data_a,
                                                     inherited::outputFormat_.resolution.cx, inherited::outputFormat_.resolution.cy,
                                                     inherited::frame_->data)))
#else
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     sourceResolution_.width, sourceResolution_.height,
                                                     inherited::inputFormat_,
                                                     data_a,
                                                     inherited::outputFormat_.resolution.width, inherited::outputFormat_.resolution.height,
                                                     inherited::frame_->data)))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::scale(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  inherited::buffer_->wr_ptr (inherited::frameSize_);
  inherited::buffer_->initialize (message_inout->sessionId (),
                                  NULL);
  inherited::buffer_->set (message_inout->type ());
  message_inout->release (); message_inout = NULL;

  // forward the converted frame
  result = inherited::put_next (inherited::buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  inherited::buffer_ = NULL;

//#if defined (_DEBUG)
//    std::string filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
//    if (!Common_File_Tools::store (filename_string,
//                                   data[0],
//                                   inherited::decodeFrameSize_))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      goto error;
//    } // end IF
//#endif

  // allocate a message buffer for the next frame
  inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
  if (unlikely (!inherited::buffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                inherited::frameSize_));
    goto error;
  } // end IF
//  av_frame_unref (inherited::frame_);
  result =
      av_image_fill_linesizes (inherited::frame_->linesize,
                               inherited::inputFormat_,
                               static_cast<int> (inherited::frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (inherited::frame_->data,
                              inherited::inputFormat_,
                              static_cast<int> (inherited::frame_->height),
                              reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                              inherited::frame_->linesize);
  ACE_ASSERT (result >= 0);

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

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
Stream_Visualization_LibAVResize_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!inherited::context_);
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      const MediaType& media_type_r = session_data_r.formats.front ();

      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());

      int flags_i = 0;
      MediaType media_type_s;
      int result = -1;
      Common_Image_Resolution_t resolution_s;

      // remember input format
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);
      inherited::inputFormat_ = media_type_2.format;
      sourceResolution_ = media_type_2.resolution;

      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_3;
      inherited::getMediaType (inherited::configuration_->outputFormat,
                               media_type_3);

      // sanity check(s)
      if (unlikely (Stream_Module_Decoder_Tools::isCompressedVideo (inherited::inputFormat_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: input format (was: %d) is compressed; cannot resize, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (inherited::inputFormat_).c_str ())));
        goto error;
      } // end IF
      // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((sourceResolution_.cx == inherited::outputFormat_.resolution.cx) &&
                    (sourceResolution_.cy == inherited::outputFormat_.resolution.cy)))
#else
      if (unlikely ((sourceResolution_.width == inherited::outputFormat_.resolution.width) &&
                    (sourceResolution_.height == inherited::outputFormat_.resolution.height)))
#endif // ACE_WIN32 || ACE_WIN64
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: output size is input size, nothing to do\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
                    sourceResolution_.cx, sourceResolution_.cy,
                    inherited::outputFormat_.resolution.cx, inherited::outputFormat_.resolution.cy));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
                    sourceResolution_.width, sourceResolution_.height,
                    inherited::outputFormat_.resolution.width, inherited::outputFormat_.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

      // initialize conversion context
      flags_i =
          (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
      inherited::context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                sourceResolution_.cx, sourceResolution_.cy, inherited::inputFormat_,
                                media_type_3.resolution.cx, media_type_3.resolution.cy, inherited::inputFormat_,
#else
                                sourceResolution_.width, sourceResolution_.height, inherited::inputFormat_,
                                media_type_3.resolution.width, media_type_3.resolution.height, inherited::inputFormat_,
#endif // ACE_WIN32 || ACE_WIN64
                                flags_i,                      // flags
                                NULL, NULL,
                                0);                           // parameters
      if (unlikely (!inherited::context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      resolution_s.cx = media_type_3.resolution.cx;
      resolution_s.cy = media_type_3.resolution.cy;
#else
      resolution_s.width = media_type_3.resolution.width;
      resolution_s.height = media_type_3.resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
      media_type_s = media_type_r;
      inherited::setResolution (resolution_s,
                                media_type_s);
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.formats.push_front (media_type_s);
      } // end lock scope

      ACE_ASSERT (!inherited::frame_);
      inherited::frame_ = av_frame_alloc ();
      if (unlikely (!inherited::frame_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      inherited::frame_->format = media_type_3.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->width = media_type_3.resolution.cx;
      inherited::frame_->height = media_type_3.resolution.cy;
#else
      inherited::frame_->width = media_type_3.resolution.width;
      inherited::frame_->height = media_type_3.resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::frameSize_ =
          av_image_get_buffer_size (static_cast<enum AVPixelFormat> (inherited::frame_->format),
                                    inherited::frame_->width, inherited::frame_->height,
                                    1); // *TODO*: linesize alignment
      ACE_ASSERT (inherited::frameSize_ >= 0);
      ACE_ASSERT (!inherited::buffer_);
      inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
      if (unlikely (!inherited::buffer_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::frameSize_));
        goto error;
      } // end IF
      ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
      result =
          av_image_fill_linesizes (inherited::frame_->linesize,
                                   static_cast<enum AVPixelFormat> (inherited::frame_->format),
                                   static_cast<int> (inherited::frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (inherited::frame_->data,
                                  static_cast<enum AVPixelFormat> (inherited::frame_->format),
                                  static_cast<int> (inherited::frame_->height),
                                  reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                                  inherited::frame_->linesize);
      ACE_ASSERT (result >= 0);

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      if (!inherited::context_)
        break; // nothing to do

      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      const MediaType& media_type_r = session_data_r.formats.front ();

      int result = -1;
      int flags_i = 0;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.cx, media_type_2.resolution.cy,
                  inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.width, media_type_2.resolution.height,
                  inherited::outputFormat_.resolution.width, inherited::outputFormat_.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

      // initialize conversion context
      ACE_ASSERT (inherited::context_);
      sws_freeContext (inherited::context_); inherited::context_ = NULL;
      flags_i =
          (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
      inherited::context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                media_type_2.resolution.cx, media_type_2.resolution.cy, inherited::inputFormat_,
                                inherited::outputFormat_.resolution.cx, inherited::outputFormat_.resolution.cy, inherited::inputFormat_,
#else
                                media_type_2.resolution.width, media_type_2.resolution.height, inherited::inputFormat_,
                                inherited::outputFormat_.resolution.width, inherited::outputFormat_.resolution.height, inherited::inputFormat_,
#endif // ACE_WIN32 || ACE_WIN64
                                flags_i,                      // flags
                                NULL, NULL,
                                0);                           // parameters
      if (unlikely (!inherited::context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // initialize frame buffer
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF
      ACE_ASSERT (inherited::frame_);
      //  frame_->format = session_data_r.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->height = media_type_2.resolution.cy;
      inherited::frame_->width = media_type_2.resolution.cx;
#else
      inherited::frame_->height = media_type_2.resolution.height;
      inherited::frame_->width = media_type_2.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::frameSize_ =
        av_image_get_buffer_size (inherited::inputFormat_,
                                  inherited::frame_->width, inherited::frame_->height,
                                  1); // *TODO*: linesize alignment

      inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
      if (!inherited::buffer_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::frameSize_));
        goto error_2;
      } // end IF
      result =
          av_image_fill_linesizes (inherited::frame_->linesize,
                                   inherited::inputFormat_,
                                   static_cast<int> (inherited::frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (inherited::frame_->data,
                                  inherited::inputFormat_,
                                  static_cast<int> (inherited::frame_->height),
                                  reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                                  inherited::frame_->linesize);
      ACE_ASSERT (result >= 0);

      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF

      if (inherited::context_)
      {
        sws_freeContext (inherited::context_); inherited::context_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Visualization_LibAVResize1_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   MediaType>::Stream_Visualization_LibAVResize1_T (ISTREAM_T* stream_in)
#else
                                   MediaType>::Stream_Visualization_LibAVResize1_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize1_T::Stream_Visualization_LibAVResize1_T"));

}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename MediaType>
//bool
//Stream_Visualization_LibAVResize1_T<ACE_SYNCH_USE,
//                                   TimePolicyType,
//                                   ConfigurationType,
//                                   ControlMessageType,
//                                   DataMessageType,
//                                   SessionMessageType,
//                                   MediaType>::initialize (const ConfigurationType& configuration_in,
//                                                           Stream_IAllocator* allocator_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize1_T::initialize"));

//  int result = -1;

//  if (!inherited::initialize (configuration_in,
//                              allocator_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_Decoder_LibAVConverter_T::initialize, aborting\n"),
//                inherited::mod_->name ()));
//    return false;
//  } // end IF

//  // sanity check(s)
//  ACE_ASSERT (inherited::frame_);
//  ACE_ASSERT (inherited::buffer_);

//  inherited::frame_->format =
//      inherited::configuration_->outputFormat.format;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  inherited::frame_->width =
//      inherited::configuration_->outputFormat.resolution.cx;
//  inherited::frame_->height =
//      inherited::configuration_->outputFormat.resolution.cy;
//#else
//  inherited::frame_->width =
//      inherited::configuration_->outputFormat.resolution.width;
//  inherited::frame_->height =
//      inherited::configuration_->outputFormat.resolution.height;
//#endif // ACE_WIN32 || ACE_WIN64
//  inherited::frameSize_ =
//      av_image_get_buffer_size (static_cast<enum AVPixelFormat> (inherited::frame_->format),
//                                inherited::frame_->width, inherited::frame_->height,
//                                1); // *TODO*: linesize alignment
//  ACE_ASSERT (inherited::frameSize_ >= 0);
//  inherited::buffer_->release (); inherited::buffer_ = NULL;
//  inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
//  if (unlikely (!inherited::buffer_))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
//                inherited::mod_->name (),
//                inherited::frameSize_));
//    return false;
//  } // end IF
//  ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
//  result =
//      av_image_fill_linesizes (inherited::frame_->linesize,
//                               static_cast<enum AVPixelFormat> (inherited::frame_->format),
//                               static_cast<int> (inherited::frame_->width));
//  ACE_ASSERT (result >= 0);
//  result =
//      av_image_fill_pointers (inherited::frame_->data,
//                              static_cast<enum AVPixelFormat> (inherited::frame_->format),
//                              static_cast<int> (inherited::frame_->height),
//                              reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
//                              inherited::frame_->linesize);
//  ACE_ASSERT (result >= 0);

//  return true;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Visualization_LibAVResize1_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize1_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  int flags_i = 0;
  int line_sizes_a[AV_NUM_DATA_POINTERS];
  uint8_t* data_a[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (&data_a, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
#if defined (_DEBUG)
    std::string filename_string;
#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (inherited::buffer_);
//  ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
  ACE_ASSERT (inherited::frame_);

  const typename DataMessageType::DATA_T& message_data_r =
      message_inout->getR ();
  typename DataMessageType::DATA_T& message_data_2 =
      const_cast<typename DataMessageType::DATA_T&> (inherited::buffer_->getR ());

  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  }
  ACE_ASSERT (!message_inout->cont ());

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              message_data_r.format.resolution.cx, message_data_r.format.resolution.cy,
              inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              message_data_r.format.resolution.width, message_data_r.format.resolution.height,
              inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

  // initialize conversion context
  flags_i =
      (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
       SWS_FAST_BILINEAR);
  inherited::context_ =
      sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            message_data_r.format.resolution.cx, message_data_r.format.resolution.cy, message_data_r.format.format,
                            inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy, message_data_r.format.format,
#else
                            message_data_r.format.resolution.width, message_data_r.format.resolution.height, message_data_r.format.format,
                            inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height, message_data_r.format.format,
#endif // ACE_WIN32 || ACE_WIN64
                            flags_i,                      // flags
                            NULL, NULL,
                            0);                           // parameters
  if (unlikely (!inherited::context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  result =
      av_image_fill_linesizes (line_sizes_a,
                               message_data_r.format.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               static_cast<int> (message_data_r.format.resolution.cx));
#else
                               static_cast<int> (message_data_r.format.resolution.width));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (data_a,
                              message_data_r.format.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              static_cast<int> (message_data_r.format.resolution.cy),
#else
                              static_cast<int> (message_data_r.format.resolution.height),
#endif // ACE_WIN32 || ACE_WIN64
                              reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                              line_sizes_a);
  ACE_ASSERT (result >= 0);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     message_data_r.format.resolution.cx, message_data_r.format.resolution.cy, message_data_r.format.format,
                                                     data_a,
                                                     inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy,
                                                     inherited::frame_->data)))
#else
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     message_data_r.format.resolution.width, message_data_r.format.resolution.height, message_data_r.format.format,
                                                     data_a,
                                                     inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height,
                                                     inherited::frame_->data)))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::scale(), returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  message_data_2.format = inherited::configuration_->outputFormat;
//  output_frame_size_i =
//      av_image_get_buffer_size (inherited::configuration_->outputFormat.format,
//                                inherited::configuration_->outputFormat.width,
//                                inherited::configuration_->outputFormat.height,
//                                1);
  inherited::buffer_->wr_ptr (inherited::frameSize_);
  inherited::buffer_->initialize (message_inout->sessionId (),
                                  NULL);
  inherited::buffer_->set (message_inout->type ());
  message_inout->release (); message_inout = NULL;

#if defined (_DEBUG)
    filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
    if (!Common_File_Tools::store (filename_string,
                                   inherited::frame_->data[0],
                                   inherited::frameSize_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
                  ACE_TEXT (filename_string.c_str ())));
      goto error;
    } // end IF
#endif // _DEBUG

  // forward the converted frame
  result = inherited::put_next (inherited::buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  inherited::buffer_ = NULL;

  // allocate a message buffer for the next frame
  inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
  if (unlikely (!inherited::buffer_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                inherited::frameSize_));
    goto error;
  } // end IF
  ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
  result =
      av_image_fill_linesizes (inherited::frame_->linesize,
                               static_cast<enum AVPixelFormat> (inherited::frame_->format),
                               static_cast<int> (inherited::frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (inherited::frame_->data,
                              static_cast<enum AVPixelFormat> (inherited::frame_->format),
                              static_cast<int> (inherited::frame_->height),
                              reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                              inherited::frame_->linesize);
  ACE_ASSERT (result >= 0);

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Visualization_LibAVResize1_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize1_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inference
  ACE_ASSERT (!session_data_r.formats.empty ());
  const MediaType& media_type_r = session_data_r.formats.front ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      int result = -1;

      // sanity check(s)
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_3;
      inherited::getMediaType (inherited::configuration_->outputFormat,
                               media_type_3);

      ACE_ASSERT (!inherited::frame_);
      inherited::frame_ = av_frame_alloc ();
      if (unlikely (!inherited::frame_))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      inherited::frame_->format = media_type_3.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->width = media_type_3.resolution.cx;
      inherited::frame_->height = media_type_3.resolution.cy;
#else
      inherited::frame_->width = media_type_3.resolution.width;
      inherited::frame_->height = media_type_3.resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::frameSize_ =
          av_image_get_buffer_size (static_cast<enum AVPixelFormat> (inherited::frame_->format),
                                    inherited::frame_->width, inherited::frame_->height,
                                    1); // *TODO*: linesize alignment
      ACE_ASSERT (inherited::frameSize_ >= 0);
      ACE_ASSERT (!inherited::buffer_);
      inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
      if (unlikely (!inherited::buffer_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::frameSize_));
        goto error;
      } // end IF
      ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
      result =
          av_image_fill_linesizes (inherited::frame_->linesize,
                                   static_cast<enum AVPixelFormat> (inherited::frame_->format),
                                   static_cast<int> (inherited::frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (inherited::frame_->data,
                                  static_cast<enum AVPixelFormat> (inherited::frame_->format),
                                  static_cast<int> (inherited::frame_->height),
                                  reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                                  inherited::frame_->linesize);
      ACE_ASSERT (result >= 0);

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      if (!inherited::context_)
        break; // nothing to do

      int result = -1;
      int flags_i = 0;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);

#if defined (_DEBUG)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.cx, media_type_2.resolution.cy,
                  inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.width, media_type_2.resolution.height,
                  inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64
#endif // _DEBUG

      // initialize conversion context
      ACE_ASSERT (inherited::context_);
      sws_freeContext (inherited::context_); inherited::context_ = NULL;
      flags_i =
          (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
      inherited::context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                media_type_2.resolution.cx, media_type_2.resolution.cy, inherited::inputFormat_,
                                inherited::configuration_->outputFormat.resolution.cx, inherited::configuration_->outputFormat.resolution.cy, inherited::inputFormat_,
#else
                                media_type_2.resolution.width, media_type_2.resolution.height, inherited::inputFormat_,
                                inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height, inherited::inputFormat_,
#endif // ACE_WIN32 || ACE_WIN64
                                flags_i,                      // flags
                                NULL, NULL,
                                0);                           // parameters
      if (unlikely (!inherited::context_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // initialize frame buffer
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF
      ACE_ASSERT (inherited::frame_);
      //  frame_->format = session_data_r.format;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->height = media_type_2.resolution.cy;
      inherited::frame_->width = media_type_2.resolution.cx;
#else
      inherited::frame_->height = media_type_2.resolution.height;
      inherited::frame_->width = media_type_2.resolution.width;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::frameSize_ =
        av_image_get_buffer_size (inherited::inputFormat_,
                                  inherited::frame_->width, inherited::frame_->height,
                                  1); // *TODO*: linesize alignment

      inherited::buffer_ = inherited::allocateMessage (inherited::frameSize_);
      if (!inherited::buffer_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Task_Base_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::frameSize_));
        goto error_2;
      } // end IF
      result =
          av_image_fill_linesizes (inherited::frame_->linesize,
                                   inherited::inputFormat_,
                                   static_cast<int> (inherited::frame_->width));
      ACE_ASSERT (result >= 0);
      result =
          av_image_fill_pointers (inherited::frame_->data,
                                  inherited::inputFormat_,
                                  static_cast<int> (inherited::frame_->height),
                                  reinterpret_cast<uint8_t*> (inherited::buffer_->wr_ptr ()),
                                  inherited::frame_->linesize);
      ACE_ASSERT (result >= 0);

      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF

      if (inherited::context_)
      {
        sws_freeContext (inherited::context_); inherited::context_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
