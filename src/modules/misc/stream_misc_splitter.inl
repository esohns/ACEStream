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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Module_Splitter_T<SessionMessageType,
                         MessageType,
                         ConfigurationType,
                         SessionDataType>::Stream_Module_Splitter_T ()
 : inherited ()
 , configuration_ (NULL)
 , currentBuffer_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::Stream_Module_Splitter_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
Stream_Module_Splitter_T<SessionMessageType,
                         MessageType,
                         ConfigurationType,
                         SessionDataType>::~Stream_Module_Splitter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::~Stream_Module_Splitter_T"));

  if (currentBuffer_)
    currentBuffer_->release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Module_Splitter_T<SessionMessageType,
                         MessageType,
                         ConfigurationType,
                         SessionDataType>::handleDataMessage (MessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  if (!currentBuffer_)
    currentBuffer_ = message_inout;
  else
    currentBuffer_->cont (message_inout);

  if (currentBuffer_->total_length () < configuration_->frameSize)
    return; // done

  // got enough data --> split and forward
  unsigned int count = 0;
  ACE_Message_Block* message_block_p = currentBuffer_;
  do
  {
    count += message_block_p->length ();
    message_block_p = message_block_p->cont ();

    if (count >= configuration_->frameSize)
      break;
  } while (true);
  ACE_ASSERT (message_block_p);

  currentBuffer_ = message_block_p->duplicate ();
  if (!currentBuffer_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MessageType::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
  currentBuffer_->cont (NULL);
  currentBuffer_->rd_ptr (count - configuration_->frameSize);

  message_block_p->reset ();
  message_block_p->wr_ptr (count - configuration_->frameSize);
  int result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));

    // clean up
    message_block_p->release ();

    return;
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
void
Stream_Module_Splitter_T<SessionMessageType,
                         MessageType,
                         ConfigurationType,
                         SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      //// *TODO*: remove type inferences
      //const typename SessionMessageType::SESSION_DATA_T& session_data_container_r =
      //  message_inout->get ();
      //const SessionDataType& session_data_r = session_data_container_r.get ();

      break;
    }
    case STREAM_SESSION_END:
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
bool
Stream_Module_Splitter_T<SessionMessageType,
                         MessageType,
                         ConfigurationType,
                         SessionDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::initialize"));

  configuration_ =
    &const_cast<ConfigurationType&> (configuration_in);

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType>
const ConfigurationType&
Stream_Module_Splitter_T<SessionMessageType,
                         MessageType,
                         ConfigurationType,
                         SessionDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Splitter_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

/////////////////////////////////////////

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::Stream_Module_SplitterH_T (bool isActive_in,
                                                                              bool autoStart_in)
 : inherited (NULL,         // lock handle
              isActive_in,  // active ?
              autoStart_in, // auto-start ?
              true,         // *NOTE*: when working in 'passive' mode, enabling
                            //         this utilizes the calling thread. Note
                            //         that this potentially renders state
                            //         transitions during processing a tricky
                            //         affair, as the calling thread may be
                            //         holding the lock --> check carefully
              true)         // generate sesssion messages ?
 , configuration_ (NULL)
 , currentBuffer_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::Stream_Module_SplitterH_T"));

}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::~Stream_Module_SplitterH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::~Stream_Module_SplitterH_T"));

  if (currentBuffer_)
    currentBuffer_->release ();
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  ACE_Message_Block* message_block_p = NULL;
  if (!currentBuffer_)
  {
    currentBuffer_ = message_inout;
    message_block_p = currentBuffer_;
  } // end IF
  else
  {
    message_block_p = currentBuffer_;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();
    message_block_p->cont (message_inout);
  } // end ELSE
  ACE_ASSERT (message_block_p);

  unsigned int total_length = currentBuffer_->total_length ();
  if (total_length < configuration_->frameSize)
    return; // done

  // received enough data --> (split and) forward
  message_block_p = currentBuffer_;
  unsigned int remainder = (total_length -
                            configuration_->frameSize);
  if (remainder)
  {
    currentBuffer_ = message_inout->duplicate ();
    if (!currentBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to MessageType::duplicate(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

      // clean up
      message_block_p->release ();

      return;
    } // end IF

    currentBuffer_->rd_ptr (remainder);

    message_block_p->reset ();
    message_block_p->wr_ptr (remainder);
  } // end IF
  else
    currentBuffer_ = NULL;

  int result = inherited::put_next (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));

    // clean up
    message_block_p->release ();

    return;
  } // end IF
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
//      ACE_ASSERT (inherited::sessionData_);

//      SessionDataType& session_data_r =
//          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      break;
    }
    case STREAM_SESSION_END:
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::collect"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0.0;
  data_out.timeStamp = COMMON_TIME_NOW;

  // *TODO*: collect socket statistics information
  //         (and propagate it downstream ?)

  // step1: send the container downstream
  if (!putStatisticMessage (data_out)) // data container
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
    return false;
  } // end IF

  return true;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::report () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::report"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::initialize"));

  if (isInitialized_)
  {
    configuration_ = NULL;

    if (currentBuffer_)
    {
      currentBuffer_->release ();
      currentBuffer_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  configuration_ =
    &const_cast<ConfigurationType&> (configuration_in);

  isInitialized_ = inherited::initialize (configuration_in);
  if (!isInitialized_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    return false;
  } // end IF

  return true;
}

//template <typename LockType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Module_SplitterH_T<LockType,
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

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_SplitterH_T<LockType,
                          SessionMessageType,
                          ProtocolMessageType,
                          ConfigurationType,
                          StreamStateType,
                          SessionDataType,
                          SessionDataContainerType,
                          StatisticContainerType>::putStatisticMessage (const StatisticContainerType& statisticData_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_SplitterH_T::putStatisticMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // step1: update session state
  SessionDataType& session_data_r =
      const_cast<SessionDataType&> (inherited::sessionData_->get ());
  // *TODO*: remove type inferences
  session_data_r.currentStatistic = statisticData_in;

  // *TODO*: attach stream state information to the session data

//  // step2: create session data object container
//  SessionDataContainerType* session_data_p = NULL;
//  ACE_NEW_NORETURN (session_data_p,
//                    SessionDataContainerType (inherited::sessionData_,
//                                              false));
//  if (!session_data_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate SessionDataContainerType: \"%m\", aborting\n")));
//    return false;
//  } // end IF

  // step3: send the statistic data downstream
//  // *NOTE*: fire-and-forget session_data_p here
  // *TODO*: remove type inference
  return inherited::putSessionMessage (STREAM_SESSION_STATISTIC,
                                       *inherited::sessionData_,
                                       inherited::configuration_->streamConfiguration->messageAllocator);
}
