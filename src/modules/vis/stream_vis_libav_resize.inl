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

#if defined (_DEBUG)
//#include "common_file_tools.h"

//#include "common_image_tools.h"
#endif // _DEBUG

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
                               static_cast<int> (inherited::frame_->width));
  ACE_ASSERT (result >= 0);
  result =
      av_image_fill_pointers (data_a,
                              inherited::inputFormat_,
                              static_cast<int> (inherited::frame_->height),
                              reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()),
                              line_sizes_a);
  ACE_ASSERT (result >= 0);
  if (unlikely (!Stream_Module_Decoder_Tools::convert (inherited::context_,
                                                       inherited::frame_->width, inherited::frame_->height, inherited::inputFormat_,
                                                       data_a,
                                                       inherited::frame_->width, inherited::frame_->height, inherited::inputFormat_,
                                                       inherited::frame_->data)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
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
  ACE_ASSERT (inherited::sessionData_);
  typename SessionDataContainerType::DATA_T& session_data_r =
    const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
  // *TODO*: remove type inference
  ACE_ASSERT (!session_data_r.formats.empty ());
  const MediaType& media_type_r = session_data_r.formats.front ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!inherited::context_);
      // *TODO*: remove type inference
      ACE_ASSERT (!session_data_r.formats.empty ());

      int flags_i = 0;
      MediaType media_type_s;
      Common_UI_Resolution_t resolution_s;

      // remember input format
      struct Stream_MediaFramework_FFMPEG_MediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);
      inherited::inputFormat_ = media_type_2.format;
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
      if (unlikely (!inherited::configuration_->window ||
                    (!inherited::configuration_->area.width ||
                     !inherited::configuration_->area.height)))
      {
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: output size and/or -window not set, continuing\n"),
                    inherited::mod_->name ()));
#endif // _DEBUG
        break;
      } // end IF
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                    inherited::mod_->name (),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                    media_type_s.resolution.cx, media_type_s.resolution.cy,
#else
                    media_type_s.resolution.width, media_type_s.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                    inherited::configuration_->area.width, inherited::configuration_->area.height));
#endif // _DEBUG

      // initialize conversion context
      flags_i =
          (//SWS_BILINEAR | SWS_FAST_BILINEAR | // interpolation
           SWS_FAST_BILINEAR);
      inherited::context_ =
          sws_getCachedContext (NULL,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                media_type_s.resolution.cx, media_type_s.resolution.cy, inherited::inputFormat_,
#else
                                media_type_s.resolution.width, media_type_s.resolution.height, inherited::inputFormat_,
#endif // ACE_WIN32 || ACE_WIN64
                                inherited::configuration_->area.width, inherited::configuration_->area.height, inherited::inputFormat_,
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
      resolution_s.cx = inherited::configuration_->area.width;
      resolution_s.cy = inherited::configuration_->area.height;
#else
      resolution_s.width = inherited::configuration_->area.width;
      resolution_s.height = inherited::configuration_->area.height;
#endif // ACE_WIN32 || ACE_WIN64
      media_type_s = media_type_r;
      inherited::setResolution (resolution_s,
                                media_type_s);
      ACE_ASSERT (session_data_r.lock);
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
        session_data_r.formats.push_front (media_type_s);
      } // end lock scope

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
      struct Stream_MediaFramework_FFMPEG_MediaType media_type_2;
      inherited::getMediaType (media_type_r,
                               media_type_2);

#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: resizing %ux%u to %ux%u\n"),
                  inherited::mod_->name (),
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                  media_type_2.resolution.cx, media_type_2.resolution.cy,
#else
                  media_type_2.resolution.width, media_type_2.resolution.height,
#endif // ACE_WIN32 || ACE_WIN64
                  inherited::configuration_->area.width, inherited::configuration_->area.height));
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
#else
                                media_type_2.resolution.width, media_type_2.resolution.height, inherited::inputFormat_,
#endif // ACE_WIN32 || ACE_WIN64
                                inherited::configuration_->area.width, inherited::configuration_->area.height, inherited::inputFormat_,
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
