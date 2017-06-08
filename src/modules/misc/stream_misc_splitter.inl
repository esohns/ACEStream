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
}
#endif

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::Stream_Module_Splitter_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , buffer_ (NULL)
 , defragment_ (false)
 , PDUSize_ (STREAM_MESSAGE_DATA_BUFFER_SIZE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::Stream_Module_Splitter_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::~Stream_Module_Splitter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::~Stream_Module_Splitter_T"));

  if (buffer_)
    buffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  if (!buffer_)
  {
    buffer_ = message_inout;
    message_block_p = buffer_;
  } // end IF
  else
  {
    message_block_p = buffer_;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();
    message_block_p->cont (message_inout);
    message_block_p = message_inout;
  } // end ELSE
  ACE_ASSERT (message_block_p);

continue_:
  // message_block_p points at the trailing fragment

  unsigned int total_length = buffer_->total_length ();
  // *TODO*: remove type inference
  if (total_length < PDUSize_)
    return; // done

  // received enough data --> (split and) forward
  ACE_Message_Block* message_block_2 = NULL;
  unsigned int remainder = (total_length - PDUSize_);
  if (remainder)
  {
    message_block_2 = message_block_p->duplicate ();
    if (!message_block_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    message_block_2->rd_ptr (message_block_p->length () - remainder);
    ACE_ASSERT (message_block_2->length () == remainder);

    message_block_p->reset ();
    message_block_p->wr_ptr (message_block_2->rd_ptr ());
    message_block_p = buffer_;

    buffer_ = message_block_2;
  } // end IF
  else
  {
    message_block_p = buffer_;
    buffer_ = NULL;
  } // end IF
  total_length = message_block_p->total_length ();
  ACE_ASSERT (total_length == PDUSize_);

  if (defragment_)
  {
    IDATA_MESSAGE_T* idata_message_p =
      dynamic_cast<IDATA_MESSAGE_T*> (message_block_p);
    if (!idata_message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IDataMessage_T*>(0x%@), returning\n"),
                  inherited::mod_->name (),
                  message_block_p));

      // clean up
      message_block_p->release ();

      return;
    } // end IF
    try {
      idata_message_p->defragment ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                  inherited::mod_->name ()));

      // clean up
      message_block_p->release ();

      return;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p->length () == PDUSize_);

  int result_2 = inherited::put_next (message_block_p, NULL);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));

    // clean up
    message_block_p->release ();

    return;
  } // end IF

  // *NOTE*: more than one frame may have been received
  //         --> split again ?
  if (buffer_)
  {
    message_block_p = buffer_;
    goto continue_;
  } // end IF
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataType>
//void
//Stream_Module_Splitter_T<ACE_SYNCH_USE,
//                         TimePolicyType,
//                         ConfigurationType,
//                         ControlMessageType,
//                         DataMessageType,
//                         SessionMessageType,
//                         SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
//                                                                 bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::handleSessionMessage"));
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//
//  switch (message_inout->type ())
//  {
//    case STREAM_SESSION_MESSAGE_BEGIN:
//      break;
//    case STREAM_SESSION_MESSAGE_END:
//    default:
//      break;
//  } // end SWITCH
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
bool
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF
    defragment_ = false;
  } // end IF

  // *TODO*: remove type inferences
  defragment_ = configuration_in.crunch;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (configuration_in.format);

  struct _AMMediaType* media_type_p = getFormat (configuration_in.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to getFormat(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  PDUSize_ = media_type_p->lSampleSize;

  // clean up
  Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
#else
  PDUSize_ =
      av_image_get_buffer_size (configuration_in.format,
                                configuration_in.width,
                                configuration_in.height,
                                1); // *TODO*: linesize alignment
#endif

  return inherited::initialize (configuration_in,
                                allocator_in);
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
AM_MEDIA_TYPE*
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*format_in,
                                                             result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n")));
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
          typename SessionDataType>
AM_MEDIA_TYPE*
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::getFormat_impl"));

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
          typename SessionDataType>
struct v4l2_format*
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::getFormat_impl (const Stream_Module_Device_ALSAConfiguration&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::getFormat_impl"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}
#endif

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::Stream_Module_SplitterH_T (ISTREAM_T* stream_in,
                                                                              ACE_SYNCH_MUTEX_T* lock_in,
                                                                              bool autoStart_in,
                                                                              enum Stream_HeadModuleConcurrency concurrency_in,
                                                                              bool generateSessionMessages_in)
 : inherited (stream_in,
              lock_in,
              autoStart_in,
              concurrency_in,
              generateSessionMessages_in)
 , buffer_ (NULL)
 , PDUSize_ (STREAM_MESSAGE_DATA_BUFFER_SIZE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::Stream_Module_SplitterH_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::~Stream_Module_SplitterH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::~Stream_Module_SplitterH_T"));

  if (buffer_)
    buffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  if (!buffer_)
  {
    buffer_ = message_inout;
    message_block_p = buffer_;
  } // end IF
  else
  {
    message_block_p = buffer_;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();
    message_block_p->cont (message_inout);
    message_block_p = message_inout;
  } // end ELSE
  ACE_ASSERT (message_block_p);

continue_:
  // message_block_p points at the trailing fragment

  unsigned int frame_size = 0;
  unsigned int total_length = buffer_->total_length ();
  // *TODO*: remove type inference
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (inherited::configuration_->format);

  //if (total_length < inherited::configuration_->format->lSampleSize)
  HRESULT result =
      inherited::configuration_->format->GetUINT32 (MF_MT_SAMPLE_SIZE,
                                                    &frame_size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_SAMPLE_SIZE): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
#else
  frame_size = inherited::configuration_->format.fmt.pix.sizeimage;
#endif
  if (total_length < frame_size)
    return; // done

  // received enough data --> (split and) forward
  ACE_Message_Block* message_block_2 = NULL;
  unsigned int remainder = (total_length - frame_size);
  if (remainder)
  {
    message_block_2 = message_block_p->duplicate ();
    if (!message_block_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF
    message_block_2->rd_ptr (message_block_p->length () - remainder);
    ACE_ASSERT (message_block_2->length () == remainder);

    message_block_p->reset ();
    message_block_p->wr_ptr (message_block_2->rd_ptr ());
    message_block_p = buffer_;

    buffer_ = message_block_2;
  } // end IF
  else
  {
    message_block_p = buffer_;
    buffer_ = NULL;
  } // end IF
  total_length = message_block_p->total_length ();
  ACE_ASSERT (total_length == frame_size);

  int result_2 = inherited::put_next (message_block_p, NULL);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));

    // clean up
    message_block_p->release ();

    return;
  } // end IF

  // *NOTE*: more than one frame may have been received
  //         --> split again ?
  if (buffer_)
  {
    message_block_p = buffer_;
    goto continue_;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    case STREAM_SESSION_MESSAGE_END:
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!inherited::putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to putStatisticMessage(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_SplitterH_T<ACE_SYNCH_USE,
//                          SessionMessageType,
//                          ProtocolMessageType,
//                          ConfigurationType,
//                          StreamStateType,
//                          SessionDataType,
//                          SessionDataContainerType,
//                          StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::initialize (const ConfigurationType& configuration_in,
                                                               Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (configuration_in.format);

  struct _AMMediaType* media_type_p = getFormat (configuration_in.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to getFormat(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  PDUSize_ = media_type_p->lSampleSize;

  // clean up
  Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
#else
  PDUSize_ = configuration_in.format.fmt.pix.sizeimage;
#endif

  return inherited::initialize (configuration_in,
                                allocator_in);
}

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Module_SplitterH_T<ACE_SYNCH_USE,
//                              SessionMessageType,
//                              ProtocolMessageType,
//                              ConfigurationType,
//                              StreamStateType,
//                              SessionDataType,
//                              SessionDataContainerType,
//                              StatisticContainerType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_V4L_T::svc"));

//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  ACE_ASSERT (isInitialized_);

//  int result = -1;
//  int result_2 = -1;
//  ACE_Message_Block* message_block_p = NULL;
//  ACE_Time_Value no_wait = COMMON_TIME_NOW;
//  int message_type = -1;
//  bool finished = false;
//  bool stop_processing = false;
//  struct v4l2_buffer buffer;
//  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
//  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//  buffer.memory = V4L2_MEMORY_USERPTR;
//  struct v4l2_event event;
//  ACE_OS::memset (&event, 0, sizeof (struct v4l2_event));
//  INDEX2BUFFER_MAP_ITERATOR_T iterator;
//  unsigned int queued, done = 0;

//  // step1: start processing data...
////   ACE_DEBUG ((LM_DEBUG,
////               ACE_TEXT ("entering processing loop...\n")));
//  do
//  {
//    message_block_p = NULL;
//    result = inherited::getq (message_block_p,
//                              &no_wait);
//    if (result == 0)
//    {
//      ACE_ASSERT (message_block_p);
//      message_type = message_block_p->msg_type ();
//      switch (message_type)
//      {
//        case ACE_Message_Block::MB_STOP:
//        {
//          // clean up
//          message_block_p->release ();
//          message_block_p = NULL;

//          // *NOTE*: when close()d manually (i.e. user abort), 'finished' will not
//          //         have been set at this stage

//          // signal the controller ?
//          if (!finished)
//          {
//            ACE_DEBUG ((LM_DEBUG,
//                        ACE_TEXT ("session aborted...\n")));

//            finished = true;
//            inherited::finished (); // *NOTE*: enqueues STREAM_SESSION_END
//          } // end IF
//          continue;
//        }
//        default:
//          break;
//      } // end SWITCH

//      // process
//      // *NOTE*: fire-and-forget message_block_p here
//      inherited::handleMessage (message_block_p,
//                                stop_processing);
//      if (stop_processing)
//      {
////        SessionMessageType* session_message_p = NULL;
////        // downcast message
////        session_message_p = dynamic_cast<SessionMessageType*> (message_block_p);
////        if (!session_message_p)
////        {
////          if (inherited::module ())
////            ACE_DEBUG ((LM_ERROR,
////                        ACE_TEXT ("%s: dynamic_cast<SessionMessageType*>(0x%@) failed (type was: %d), aborting\n"),
////                        inherited::name (),
////                        message_block_p,
////                        message_type));
////          else
////            ACE_DEBUG ((LM_ERROR,
////                        ACE_TEXT ("dynamic_cast<SessionMessageType*>(0x%@) failed (type was: %d), aborting\n"),
////                        message_block_p,
////                        message_type));
////          break;
////        } // end IF
////        if (session_message_p->type () == STREAM_SESSION_END)
//          result_2 = 0; // success
//        goto done; // finished processing
//      } // end IF
//    } // end IF
//    else if (result == -1)
//    {
//      int error = ACE_OS::last_error ();
//      if (error != EWOULDBLOCK) // Win32: 10035
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

//        // signal the controller ?
//        if (!finished)
//        {
//          finished = true;
//          inherited::finished ();
//        } // end IF

//        break;
//      } // end IF

//      // session aborted ? (i.e. connection failed)
//      ACE_ASSERT (inherited::sessionData_);
//      const SessionDataType& session_data_r = inherited::sessionData_->get ();
//      if (session_data_r.aborted)
//      {
//        inherited::shutdown ();
//        continue;
//      } // end IF
//    } // end IF

//    // log device status to kernel log
//    if (debug_)
//    {
//      result = v4l2_ioctl (captureFileDescriptor_,
//                           VIDIOC_LOG_STATUS);
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
//                    captureFileDescriptor_, ACE_TEXT ("VIDIOC_LOG_STATUS")));
//    } // end IF

////    // dequeue pending events
////    result = v4l2_ioctl (captureFileDescriptor_,
////                         VIDIOC_DQEVENT,
////                         &event);
////    if (result == -1)
////    {
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
////                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
////    } // end IF
////    else
////    {
////      for (unsigned int i = 0;
////           i < event.pending;
////           ++i)
////      {
////        result = v4l2_ioctl (captureFileDescriptor_,
////                             VIDIOC_DQEVENT,
////                             &event);
////        if (result == -1)
////          ACE_DEBUG ((LM_ERROR,
////                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
////                      captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
////      } // end FOR
////    } // end ELSE

////    queued =
////        Stream_Module_Device_Tools::queued (captureFileDescriptor_,
////                                            inherited::configuration_->buffers,
////                                            done);
////    ACE_DEBUG ((LM_DEBUG,
////                ACE_TEXT ("#queued/done buffers: %u/%u...\n"),
////                queued, done));

//    // *NOTE*: blocks until:
//    //         - a buffer is availbale
//    //         - a frame has been written by the device
//    result = v4l2_ioctl (captureFileDescriptor_,
//                         VIDIOC_DQBUF,
//                         &buffer);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
//                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQBUF")));
//      break;
//    } // end IF
//    if (buffer.flags & V4L2_BUF_FLAG_ERROR)
//      ACE_DEBUG ((LM_WARNING,
//                  ACE_TEXT ("%s: streaming error (fd: %d, index: %d), continuing\n"),
//                  inherited::mod_->name (),
//                  captureFileDescriptor_, buffer.index));

////    // sanity check(s)
////    ACE_ASSERT (buffer.reserved);
////    message_block_p = reinterpret_cast<ACE_Message_Block*> (buffer.reserved);
//    iterator = inherited::configuration_->bufferMap.find (buffer.index);
//    ACE_ASSERT (iterator != inherited::configuration_->bufferMap.end ());
//    message_block_p = (*iterator).second;

//    result = inherited::put_next (message_block_p, NULL);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

//      // clean up
//      message_block_p->release ();

//      break;
//    } // end IF

//    buffer.reserved = 0;
//  } while (true);

//done:
//  return result_2;
//}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
AM_MEDIA_TYPE*
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*format_in,
                                                             result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n")));
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);

  return result_p;
}
template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
AM_MEDIA_TYPE*
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::getFormat_impl"));

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
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
struct v4l2_format*
Stream_Module_SplitterH_T<ACE_SYNCH_USE,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ConfigurationType,
                          StreamControlType,
                          StreamNotificationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::getFormat_impl (const Stream_Module_Device_ALSAConfiguration&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::getFormat_impl"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);
  ACE_NOTREACHED (return NULL;)
}
#endif
