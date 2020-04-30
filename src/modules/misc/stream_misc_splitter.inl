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
#else
#if defined (__cplusplus)
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif // __cplusplus
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "stream_defines.h"
#include "stream_macros.h"


template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Splitter1_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                         MediaType>::Stream_Module_Splitter1_T (ISTREAM_T* stream_in)
#else
                         MediaType>::Stream_Module_Splitter1_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , buffer_ (NULL)
 , PDUSize_ (STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter1_T::Stream_Module_Splitter1_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Module_Splitter1_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         MediaType>::~Stream_Module_Splitter1_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter1_T::~Stream_Module_Splitter1_T"));

  if (buffer_)
    buffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Module_Splitter1_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter1_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  if (unlikely (!buffer_))
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
    if (unlikely (!message_block_2))
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

  if (inherited::configuration_->crunch)
  {
    IDATA_MESSAGE_T* idata_message_p =
      dynamic_cast<IDATA_MESSAGE_T*> (message_block_p);
    if (unlikely (!idata_message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IDataMessage_T*>(0x%@), returning\n"),
                  inherited::mod_->name (),
                  message_block_p));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
    try {
      idata_message_p->defragment ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
  } // end IF
  ACE_ASSERT (message_block_p->length () == PDUSize_);

  int result_2 = inherited::put_next (message_block_p, NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
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
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Module_Splitter1_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         MediaType>::initialize (const ConfigurationType& configuration_in,
                                                 Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter1_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            media_type_s);
  PDUSize_ = media_type_s.lSampleSize;

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

  return inherited::initialize (configuration_in,
                                allocator_in);
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                         MediaType>::Stream_Module_Splitter_T (ISTREAM_T* stream_in)
#else
                         MediaType>::Stream_Module_Splitter_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , inherited2 ()
 , buffer_ (NULL)
 , PDUSize_ (STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::Stream_Module_Splitter_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType,
                         MediaType>::~Stream_Module_Splitter_T ()
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
          typename SessionDataType,
          typename MediaType>
void
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType,
                         MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  if (unlikely (!buffer_))
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
    if (unlikely (!message_block_2))
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

  if (inherited::configuration_->crunch)
  {
    IDATA_MESSAGE_T* idata_message_p =
      dynamic_cast<IDATA_MESSAGE_T*> (message_block_p);
    if (unlikely (!idata_message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to dynamic_cast<Stream_IDataMessage_T*>(0x%@), returning\n"),
                  inherited::mod_->name (),
                  message_block_p));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
    try {
      idata_message_p->defragment ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IDataMessage_T::defragment(), returning\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      return;
    } // end IF
    ACE_ASSERT (message_block_p->length () == PDUSize_);
  } // end IF

  int result_2 = inherited::put_next (message_block_p, NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
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
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename MediaType>
bool
Stream_Module_Splitter_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType,
                         MediaType>::initialize (const ConfigurationType& configuration_in,
                                                 Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  inherited2::getMediaType (configuration_in.outputFormat,
                            media_type_s);
  PDUSize_ = media_type_s.lSampleSize;

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
  struct Stream_MediaFramework_FFMPEG_MediaType media_type_s;
  inherited2::getMediaType (configuration_in.outputFormat,
                            media_type_s);
  PDUSize_ =
      av_image_get_buffer_size (media_type_s.format,
                                media_type_s.resolution.width,
                                media_type_s.resolution.height,
                                1); // *TODO*: linesize alignment
#endif // ACE_WIN32 || ACE_WIN64

  return inherited::initialize (configuration_in,
                                allocator_in);
}

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
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename MediaType>
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
                          StatisticContainerType,
                          StatisticHandlerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          MediaType>::Stream_Module_SplitterH_T (ISTREAM_T* stream_in,
#else
                          MediaType>::Stream_Module_SplitterH_T (typename inherited::ISTREAM_T* stream_in,
#endif // ACE_WIN32 || ACE_WIN64
                                                                 ACE_SYNCH_MUTEX_T* lock_in,
                                                                 bool autoStart_in,
                                                                 enum Stream_HeadModuleConcurrency concurrency_in,
                                                                 bool generateSessionMessages_in)
 : inherited (stream_in,
              lock_in,
              autoStart_in,
              concurrency_in,
              generateSessionMessages_in)
 , inherited2 ()
 , buffer_ (NULL)
 , PDUSize_ (STREAM_MESSAGE_DEFAULT_DATA_BUFFER_SIZE)
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
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename MediaType>
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
                          StatisticContainerType,
                          StatisticHandlerType,
                          MediaType>::~Stream_Module_SplitterH_T ()
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
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename MediaType>
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
                          StatisticContainerType,
                          StatisticHandlerType,
                          MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  ACE_Message_Block* message_block_p = NULL;
  if (unlikely (!buffer_))
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
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_SAMPLE_SIZE): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
#else
  frame_size = inherited::configuration_->format.fmt.pix.sizeimage;
#endif // ACE_WIN32 || ACE_WIN64
  if (total_length < frame_size)
    return; // done

  // received enough data --> (split and) forward
  ACE_Message_Block* message_block_2 = NULL;
  unsigned int remainder = (total_length - frame_size);
  if (remainder)
  {
    message_block_2 = message_block_p->duplicate ();
    if (unlikely (!message_block_2))
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
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
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
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename MediaType>
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
                          StatisticContainerType,
                          StatisticHandlerType,
                          MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
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
          typename StatisticContainerType,
          typename StatisticHandlerType,
          typename MediaType>
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
                          StatisticContainerType,
                          StatisticHandlerType,
                          MediaType>::initialize (const ConfigurationType& configuration_in,
                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
  } // end IF

  // *TODO*: remove type inferences
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // sanity check(s)
  ACE_ASSERT (configuration_in.format);

  struct _AMMediaType* media_type_p = getFormat (configuration_in.format);
  if (unlikely (!media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to getFormat(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  PDUSize_ = media_type_p->lSampleSize;

  // clean up
  Stream_Module_Device_DirectShow_Tools::delete_ (media_type_p);
#else
  PDUSize_ = configuration_in.format.fmt.pix.sizeimage;
#endif

  return inherited::initialize (configuration_in,
                                allocator_in);
}
