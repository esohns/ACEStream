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

template <typename TaskType,
          typename MediaType>
Stream_Visualization_LibAVResize_T<TaskType,
                                   MediaType>::Stream_Visualization_LibAVResize_T (typename TaskType::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , formatsIndex_ (0)
 , sourceResolution_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize_T::Stream_Visualization_LibAVResize_T"));

}

template <typename TaskType,
          typename MediaType>
void
Stream_Visualization_LibAVResize_T<TaskType,
                                   MediaType>::handleDataMessage (typename TaskType::DATA_MESSAGE_T*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = -1;
  int line_sizes_a[AV_NUM_DATA_POINTERS];
  uint8_t* data_a[AV_NUM_DATA_POINTERS];
  ACE_OS::memset (&line_sizes_a, 0, sizeof (int[AV_NUM_DATA_POINTERS]));
  ACE_OS::memset (&data_a, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));

  // sanity check(s)
  ACE_ASSERT (inherited::buffer_);
  //ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::frame_);
  ACE_ASSERT (!message_inout->cont ());

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
                                                     inherited::frame_->width, inherited::frame_->height,
                                                     inherited::frame_->data,
                                                     false))) // flip image ?
#else
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     sourceResolution_.width, sourceResolution_.height,
                                                     inherited::inputFormat_,
                                                     data_a,
                                                     inherited::frame_->width, inherited::frame_->height,
                                                     inherited::frame_->data,
                                                     false))) // flip image ?
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

// #if defined (_DEBUG)
//    if (!Common_File_Tools::store (ACE_TEXT_ALWAYS_CHAR ("output.data"),
//                                   data_a[0],
//                                   inherited::frameSize_))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
//                  ACE_TEXT ("output.data")));
//      goto error;
//    } // end IF
// #endif // _DEBUG

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

template <typename TaskType,
          typename MediaType>
void
Stream_Visualization_LibAVResize_T<TaskType,
                                   MediaType>::handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*& message_inout,
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
      typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
        const_cast<typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      const MediaType& media_type_r = session_data_r.formats.back ();

      int flags_i = 0;
      MediaType media_type_s;
      int result = -1;
      Common_Image_Resolution_t resolution_s;

      // remember input format
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_2);
      inherited::inputFormat_ = media_type_2.format;
      sourceResolution_ = media_type_2.resolution;

      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_3;
      inherited::getMediaType (inherited::configuration_->outputFormat,
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_3);

      // sanity check(s)
      if (unlikely (Stream_Module_Decoder_Tools::isCompressedVideo (inherited::inputFormat_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: input format (was: %s) is compressed; cannot resize, aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::inputFormat_).c_str ())));
        goto error;
      } // end IF
      // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely ((sourceResolution_.cx == media_type_3.resolution.cx) &&
                    (sourceResolution_.cy == media_type_3.resolution.cy)))
#else
      if (unlikely ((sourceResolution_.width == media_type_3.resolution.width) &&
                    (sourceResolution_.height == media_type_3.resolution.height)))
#endif // ACE_WIN32 || ACE_WIN64
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: output size is input size, nothing to do\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %s %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::inputFormat_).c_str ()),
                    sourceResolution_.cx, sourceResolution_.cy,
                    media_type_3.resolution.cx, media_type_3.resolution.cy));
#else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %s %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::inputFormat_).c_str ()),
                    sourceResolution_.width, sourceResolution_.height,
                    media_type_3.resolution.width, media_type_3.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

      // initialize conversion context
      flags_i = //(SWS_FAST_BILINEAR); // interpolation
        (SWS_FULL_CHR_H_INP | SWS_BICUBIC | SWS_ACCURATE_RND | SWS_BITEXACT);
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
      ACE_OS::memset (&media_type_s, 0, sizeof (MediaType));
      inherited::getMediaType (media_type_r,
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);
#else
      resolution_s.width = media_type_3.resolution.width;
      resolution_s.height = media_type_3.resolution.height;
      media_type_s = media_type_r;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::setResolution (resolution_s,
                                media_type_s);
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.formats.push_back (media_type_s);
        formatsIndex_ =
          static_cast<unsigned int> (session_data_r.formats.size () - 1);
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
      inherited::frame_->format = inherited::inputFormat_;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->width = media_type_3.resolution.cx;
      inherited::frame_->height = media_type_3.resolution.cy;
#else
      inherited::frame_->width = media_type_3.resolution.width;
      inherited::frame_->height = media_type_3.resolution.height;
#endif // ACE_WIN32 || ACE_WIN64
      inherited::frameSize_ =
          av_image_get_buffer_size (inherited::inputFormat_,
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

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      int result = -1;
      int flags_i = 0;
      MediaType media_type_s;

      ACE_ASSERT (inherited::sessionData_);
      typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
        const_cast<typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (session_data_r.lock);
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        if (formatsIndex_ &&
            session_data_r.formats.size () >= formatsIndex_)
        {
          typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T::MEDIAFORMATS_ITERATOR_T
            iterator = session_data_r.formats.begin ();
          std::advance (iterator, formatsIndex_);
          session_data_r.formats.erase (iterator, session_data_r.formats.end ());
          formatsIndex_ = 0;
        } // end IF
        inherited::getMediaType (session_data_r.formats.back (),
                                 STREAM_MEDIATYPE_VIDEO,
                                 media_type_2);
      } // end lock scope
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_3;
      inherited::getMediaType (inherited::configuration_->outputFormat,
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_3);

      sourceResolution_ = media_type_2.resolution;
      if (inherited::context_)
      {
        sws_freeContext (inherited::context_); inherited::context_ = NULL;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((sourceResolution_.cy == media_type_3.resolution.cy) &&
          (sourceResolution_.cx == media_type_3.resolution.cx))
#else
      if ((sourceResolution_.height == media_type_3.resolution.height) &&
          (sourceResolution_.width == media_type_3.resolution.width))
#endif // ACE_WIN32 || ACE_WIN64
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: output size is input size, nothing to do\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %s %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::inputFormat_).c_str ()),
                  sourceResolution_.cx, sourceResolution_.cy,
                  media_type_3.resolution.cx, media_type_3.resolution.cy));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %s %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_Tools::pixelFormatToString (inherited::inputFormat_).c_str ()),
                  sourceResolution_.width, sourceResolution_.height,
                  media_type_3.resolution.width, media_type_3.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

      // initialize conversion context
      // *TODO*: use passed-in flags here
      flags_i =
        SWS_FULL_CHR_H_INP | SWS_BICUBIC | SWS_ACCURATE_RND | SWS_BITEXACT; // interpolation
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
        goto error_2;
      } // end IF

      // initialize frame buffer
      if (inherited::buffer_)
      {
        inherited::buffer_->release (); inherited::buffer_ = NULL;
      } // end IF
      // *WARNING*: this might be the first time frame_ is initialized !
      if (unlikely (!inherited::frame_))
      {
        inherited::frame_ = av_frame_alloc ();
        if (unlikely (!inherited::frame_))
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: av_frame_alloc() failed: \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error_2;
        } // end IF
      } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited::frame_->height = media_type_3.resolution.cy;
      inherited::frame_->width = media_type_3.resolution.cx;
#else
      inherited::frame_->height = media_type_3.resolution.height;
      inherited::frame_->width = media_type_3.resolution.width;
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

      ACE_OS::memset (&media_type_s, 0, sizeof (MediaType));
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        inherited::getMediaType (session_data_r.formats.back (),
                                 STREAM_MEDIATYPE_VIDEO,
                                 media_type_s);
        inherited::setResolution (media_type_3.resolution,
                                  media_type_s);
        session_data_r.formats.push_back (media_type_s);
        formatsIndex_ =
          static_cast<unsigned int> (session_data_r.formats.size () - 1);
      } // end lock scope

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

template <typename TaskType,
          typename MediaType>
Stream_Visualization_LibAVResize1_T<TaskType,
                                    MediaType>::Stream_Visualization_LibAVResize1_T (typename TaskType::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize1_T::Stream_Visualization_LibAVResize1_T"));

}

template <typename TaskType,
          typename MediaType>
void
Stream_Visualization_LibAVResize1_T<TaskType,
                                    MediaType>::handleDataMessage (typename TaskType::DATA_MESSAGE_T*& message_inout,
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
//#if defined (_DEBUG)
//    std::string filename_string;
//#endif // _DEBUG

  // sanity check(s)
  ACE_ASSERT (inherited::buffer_);
//  ACE_ASSERT (inherited::buffer_->capacity () >= inherited::frameSize_);
  ACE_ASSERT (inherited::frame_);

  typename TaskType::DATA_MESSAGE_T::DATA_T& message_data_2 =
      const_cast<typename TaskType::DATA_MESSAGE_T::DATA_T&> (inherited::buffer_->getR ());
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
  inherited::getMediaType (message_data_2.format,
                           STREAM_MEDIATYPE_VIDEO,
                           media_type_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  inherited::getMediaType (inherited::configuration_->outputFormat,
                           STREAM_MEDIATYPE_VIDEO,
                           media_type_2);
  ACE_ASSERT (media_type_s.format == media_type_2.format);
  if ((media_type_s.resolution.cx == media_type_2.resolution.cx) &&
      (media_type_s.resolution.cy == media_type_2.resolution.cy))
#else
  ACE_ASSERT (media_type_s.format == inherited::configuration_->outputFormat.format);
  if ((media_type_s.resolution.width == inherited::configuration_->outputFormat.resolution.width) &&
      (media_type_s.resolution.height == inherited::configuration_->outputFormat.resolution.height))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: output size is input size, nothing to do\n"),
                inherited::mod_->name ()));
    passMessageDownstream_out = true;
    return;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              media_type_s.resolution.cx, media_type_s.resolution.cy,
              media_type_2.resolution.cx, media_type_2.resolution.cy));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
              inherited::mod_->name (),
              media_type_s.resolution.width, media_type_s.resolution.height,
              inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

  // initialize conversion context
  flags_i =
      (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
       SWS_BICUBIC);
  inherited::context_ =
      sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            media_type_s.resolution.cx, media_type_s.resolution.cy, media_type_s.format,
                            media_type_2.resolution.cx, media_type_2.resolution.cy, media_type_s.format,
#else
                            media_type_s.resolution.width, media_type_s.resolution.height, media_type_s.format,
                            inherited::configuration_->outputFormat.resolution.width, inherited::configuration_->outputFormat.resolution.height, media_type_s.format,
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
                               media_type_s.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                               static_cast<int> (media_type_s.resolution.cx));
#else
                               static_cast<int> (media_type_s.resolution.width));
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (data_a,
                              media_type_s.format,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              static_cast<int> (media_type_s.resolution.cy),
#else
                              static_cast<int> (media_type_s.resolution.height),
#endif // ACE_WIN32 || ACE_WIN64
                              reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                              line_sizes_a);
  ACE_ASSERT (result >= 0);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     media_type_s.resolution.cx, media_type_s.resolution.cy, media_type_s.format,
                                                     data_a,
                                                     media_type_2.resolution.cx, media_type_2.resolution.cy,
                                                     inherited::frame_->data)))
#else
  if (unlikely (!Stream_Module_Decoder_Tools::scale (inherited::context_,
                                                     media_type_s.resolution.width, media_type_s.resolution.height, media_type_s.format,
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

    //filename_string = ACE_TEXT_ALWAYS_CHAR ("output.rgb");
    //if (!Common_File_Tools::store (filename_string,
    //                               inherited::frame_->data[0],
    //                               inherited::frameSize_))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to Common_File_Tools::store(\"%s\"), returning\n"),
    //              ACE_TEXT (filename_string.c_str ())));
    //  goto error;
    //} // end IF

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

template <typename TaskType,
          typename MediaType>
void
Stream_Visualization_LibAVResize1_T<TaskType,
                                    MediaType>::handleSessionMessage (typename TaskType::SESSION_MESSAGE_T*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Visualization_LibAVResize1_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T& session_data_r =
    const_cast<typename TaskType::SESSION_MESSAGE_T::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inference
  ACE_ASSERT (!session_data_r.formats.empty ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      int result = -1;

      // sanity check(s)
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_3;
      inherited::getMediaType (inherited::configuration_->outputFormat,
                               STREAM_MEDIATYPE_VIDEO,
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
      int result = -1;
      int flags_i = 0;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_2;
      // sanity check(s)
      // update configuration
      ACE_ASSERT (inherited::configuration_);
      inherited::getMediaType (inherited::configuration_->outputFormat,
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_2);
      ACE_ASSERT (inherited::frame_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if ((static_cast<unsigned int> (inherited::frame_->height) == media_type_s.resolution.cy) &&
          (static_cast<unsigned int> (inherited::frame_->width) == media_type_s.resolution.cx))
#else
      if ((static_cast<unsigned int> (inherited::frame_->height) == media_type_s.resolution.height) &&
          (static_cast<unsigned int> (inherited::frame_->width) == media_type_s.resolution.width))
#endif // ACE_WIN32 || ACE_WIN64
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: output size is input size, nothing to do\n"),
                    inherited::mod_->name ()));
        break; // does not concern 'this'
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.cx, media_type_2.resolution.cy,
                  media_type_s.resolution.cx, media_type_s.resolution.cy));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
                  media_type_2.resolution.width, media_type_2.resolution.height,
                  media_type_s.resolution.width, media_type_s.resolution.height));
#endif // ACE_WIN32 || ACE_WIN64

      // initialize conversion context
      ACE_ASSERT (inherited::context_);
      sws_freeContext (inherited::context_); inherited::context_ = NULL;
      flags_i =
          (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_BICUBIC);
      inherited::context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                media_type_2.resolution.cx, media_type_2.resolution.cy, inherited::inputFormat_,
                                media_type_s.resolution.cx, media_type_s.resolution.cy, inherited::inputFormat_,
#else
                                media_type_2.resolution.width, media_type_2.resolution.height, inherited::inputFormat_,
                                media_type_s.resolution.width, media_type_s.resolution.height, inherited::inputFormat_,
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
      inherited::frame_->height = media_type_s.resolution.cy;
      inherited::frame_->width = media_type_s.resolution.cx;
#else
      inherited::frame_->height = media_type_s.resolution.height;
      inherited::frame_->width = media_type_s.resolution.width;
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
