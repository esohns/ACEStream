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

#include "libv4l2.h"
#include "linux/videodev2.h"

#include "stream_macros.h"

#include "stream_dev_tools.h"

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_CamSource_V4L_T<LockType,
                              SessionMessageType,
                              ProtocolMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::Stream_Module_CamSource_V4L_T (LockType* lock_in,
                                                                                      bool autoStart_in)
 : inherited (lock_in,      // lock handle
              autoStart_in, // auto-start ?
              true)         // generate sesssion messages ?
 , captureFileDescriptor_ (-1)
 , overlayFileDescriptor_ (-1)
 , bufferMap_ ()
#if defined (_DEBUG)
 , debug_ (false)
#endif
 , hasFinished_ (false)
 , isPassive_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::Stream_Module_CamSource_V4L_T"));

}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
Stream_Module_CamSource_V4L_T<LockType,
                              SessionMessageType,
                              ProtocolMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::~Stream_Module_CamSource_V4L_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::~Stream_Module_CamSource_V4L_T"));

  int result = -1;

  if (!isPassive_)
    if (captureFileDescriptor_ != -1)
    {
      result = v4l2_close (captureFileDescriptor_);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                    captureFileDescriptor_));
    } // end IF
  if (overlayFileDescriptor_ != -1)
  {
    result = v4l2_close (overlayFileDescriptor_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  overlayFileDescriptor_));
  } // end IF
}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataType>
//void
//Stream_Module_CamSource_V4L_T<SessionMessageType,
//                            MessageType,
//                            ModuleHandlerConfigurationType,
//                            SessionDataType>::handleDataMessage (MessageType*& message_inout,
//                                                                 bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::handleDataMessage"));
//
//  ssize_t bytes_written = -1;
//
//  // don't care (implies yes per default, if part of a stream)
//  ACE_UNUSED_ARG (passMessageDownstream_out);
//
//  // sanity check(s)
//  if (!connection_)
//  {
////    ACE_DEBUG ((LM_ERROR,
////                ACE_TEXT ("failed to open db connection, returning\n")));
//    return;
//  } // end IF
//}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
void
Stream_Module_CamSource_V4L_T<LockType,
                              SessionMessageType,
                              ProtocolMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::initialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (inherited::configuration_->streamConfiguration);
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      int toggle = 1;

      // step0: retain current format as session data
      if (!Stream_Module_Device_Tools::getCaptureFormat (captureFileDescriptor_,
                                                         session_data_r.format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::getCaptureFormat(%d): \"%m\", aborting\n"),
                    captureFileDescriptor_));
        goto error;
      } // end IF

      // step1: fill buffer queue(s)
      if (captureFileDescriptor_ != -1)
        if (!Stream_Module_Device_Tools::initializeBuffers<ProtocolMessageType> (captureFileDescriptor_,
                                                                                 inherited::configuration_->method,
                                                                                 inherited::configuration_->buffers,
                                                                                 bufferMap_,
                                                                                 inherited::configuration_->streamConfiguration->messageAllocator))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeBuffers(%d): \"%m\", aborting\n"),
                      captureFileDescriptor_));
          goto error;
        } // end IF

      // step2: start stream
      if (captureFileDescriptor_ != -1)
      {
        result = v4l2_ioctl (captureFileDescriptor_,
                             VIDIOC_STREAMON,
                             &toggle);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                      captureFileDescriptor_, ACE_TEXT ("VIDIOC_STREAMON")));
          goto error;
        } // end IF
      } // end IF
      if (overlayFileDescriptor_ != -1)
      {
        result = v4l2_ioctl (overlayFileDescriptor_,
                             VIDIOC_OVERLAY,
                             &toggle);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                      overlayFileDescriptor_, ACE_TEXT ("VIDIOC_OVERLAY")));
          goto error;
        } // end IF
      } // end IF

      break;

error:
      // *TODO*: remove type inference
      session_data_r.aborted = true;

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {

      int toggle = 0;
      bool shutdown = true;

//      // *NOTE*: if the stream is being shut down due to an external event (i.e.
//      //         peer has closed the connection, ...), the stream is finished(),
//      //         which enqueues STREAM_SESSION_END, and control lands here while
//      //         processing is still ongoing (see svc()). The VIDIOC_DQBUF calls
//      //         in the buffer finalization routine below will then contend and
//      //         potentially deadlock
//      //         --> stop it first
//      if (!hasFinished_)
//      {
//        hasFinished_ = true;
//        inherited::shutdown ();
//        shutdown = false;

//        // *TODO*: wait for the processing thread and flush buffers
//      } // end IF

      // step1: empty buffer queue(s)
      if (captureFileDescriptor_ != -1)
        Stream_Module_Device_Tools::finalizeBuffers<ProtocolMessageType> (captureFileDescriptor_,
                                                                          inherited::configuration_->method,
                                                                          bufferMap_);

      // step2: stop stream
      if (overlayFileDescriptor_ != -1)
      {
        result = v4l2_ioctl (overlayFileDescriptor_,
                             VIDIOC_OVERLAY,
                             &toggle);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_ioctl(%d,%u): \"%m\", continuing\n"),
                      overlayFileDescriptor_, ACE_TEXT ("VIDIOC_OVERLAY")));
      } // end IF
      if (captureFileDescriptor_ != -1)
      {
        toggle = 1; // ?
        result = v4l2_ioctl (captureFileDescriptor_,
                             VIDIOC_STREAMOFF,
                             &toggle);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_ioctl(%d,%u): \"%m\", continuing\n"),
                      captureFileDescriptor_, ACE_TEXT ("VIDIOC_STREAMOFF")));
      } // end IF

      // step3: deallocate device buffer queue slots
      struct v4l2_requestbuffers request_buffers;
      ACE_OS::memset (&request_buffers, 0, sizeof (struct v4l2_requestbuffers));
      request_buffers.count = 0;
      request_buffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      request_buffers.memory = inherited::configuration_->method;
      result = v4l2_ioctl (captureFileDescriptor_,
                           VIDIOC_REQBUFS,
                           &request_buffers);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                    captureFileDescriptor_, ACE_TEXT ("VIDIOC_REQBUFS")));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("de-allocated %d device buffer slot(s)\n"),
                    inherited::configuration_->buffers));

//      if (captureFileDescriptor_ != -1)
//      {
//        result = v4l2_close (captureFileDescriptor_);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
//                      captureFileDescriptor_));
//      } // end IF
//      if (overlayFileDescriptor_ != -1)
//      {
//        result = v4l2_close (overlayFileDescriptor_);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
//                      overlayFileDescriptor_));
//      } // end IF

      if (shutdown)
        inherited::shutdown ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_CamSource_V4L_T<LockType,
                              SessionMessageType,
                              ProtocolMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

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
                ACE_TEXT ("failed to putStatisticMessage(), aborting\n")));
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
//void
//Stream_Module_CamSource_V4L_T<LockType,
//                              SessionMessageType,
//                              ProtocolMessageType,
//                              ConfigurationType,
//                              StreamStateType,
//                              SessionDataType,
//                              SessionDataContainerType,
//                              StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::report"));

//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
bool
Stream_Module_CamSource_V4L_T<LockType,
                              SessionMessageType,
                              ProtocolMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::initialize"));

  bool result = false;
  int result_2 = -1;

  if (inherited::initialized_)
  {
    if (!isPassive_)
      if (captureFileDescriptor_ != -1)
      {
        result_2 = v4l2_close (captureFileDescriptor_);
        if (result_2 == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                      captureFileDescriptor_));
        captureFileDescriptor_ = -1;
      } // end IF
    if (overlayFileDescriptor_ != -1)
    {
      result_2 = v4l2_close (overlayFileDescriptor_);
      if (result_2 == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                    overlayFileDescriptor_));
      overlayFileDescriptor_ = -1;
    } // end IF

    debug_ = false;
    isPassive_ = false;
  } // end IF

  // *NOTE*: use O_NONBLOCK with a reactor (v4l2_select()) or proactor
  //         (v4l2_poll())
  // *TODO*: support O_NONBLOCK
  //  int open_mode = O_RDONLY;
  int open_mode =
      ((configuration_in.method == V4L2_MEMORY_MMAP) ? O_RDWR : O_RDONLY);
  // *TODO*: remove type inference
  captureFileDescriptor_ = configuration_in.fileDescriptor;
  if (captureFileDescriptor_ != -1)
    isPassive_ = true;
  else
  {
    // *TODO*: remove type inference
    captureFileDescriptor_ = v4l2_open (configuration_in.device.c_str (),
                                        open_mode);
    if (captureFileDescriptor_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                  ACE_TEXT (configuration_in.device.c_str ()), open_mode));
      return false;
    } // end IF
  } // end ELSE
  ACE_ASSERT (captureFileDescriptor_ != -1);

  // *TODO*: remove type inference
  if (configuration_in.v4l2Window &&
      Stream_Module_Device_Tools::canOverlay (captureFileDescriptor_))
  {
    overlayFileDescriptor_ = v4l2_open (configuration_in.device.c_str (),
                                        open_mode);
    if (overlayFileDescriptor_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                  ACE_TEXT (configuration_in.device.c_str ()), open_mode));
      goto error;
    } // end IF
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened v4l2 device \"%s\" (fd: %d)...\n"),
              ACE_TEXT (configuration_in.device.c_str ()),
              captureFileDescriptor_));

  // *TODO*: remove type inference
  if (!Stream_Module_Device_Tools::setCaptureFormat (captureFileDescriptor_,
                                                     configuration_in.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(%d): \"%m\", aborting\n"),
                captureFileDescriptor_));
    goto error;
  } // end IF

  // *TODO*: remove type inference
  if (!Stream_Module_Device_Tools::setFrameRate (captureFileDescriptor_,
                                                 configuration_in.frameRate))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFrameRate(%d), returning\n"),
                captureFileDescriptor_));
    goto error;
  } // end IF

  if (Stream_Module_Device_Tools::canStream (captureFileDescriptor_))
    if (!Stream_Module_Device_Tools::initializeCapture (captureFileDescriptor_,
                                                        configuration_in.method,
                                                        const_cast<ConfigurationType&> (configuration_in).buffers))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeCapture(%d): \"%m\", aborting\n"),
                  captureFileDescriptor_));
      goto error;
    } // end IF
  if (overlayFileDescriptor_ != -1)
  {
    ACE_ASSERT (configuration_in.v4l2Window);
    if (!Stream_Module_Device_Tools::initializeOverlay (overlayFileDescriptor_,
                                                        *configuration_in.v4l2Window))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeOverlay(%d): \"%m\", aborting\n"),
                  overlayFileDescriptor_));
      goto error;
    } // end IF
  } // end IF

  result = inherited::initialize (configuration_in);
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));
    goto error;
  } // end IF

  return result;

error:
  if (captureFileDescriptor_ != -1)
  {
    result_2 = v4l2_close (captureFileDescriptor_);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  captureFileDescriptor_));
    captureFileDescriptor_ = -1;
  } // end IF
  if (overlayFileDescriptor_ != -1)
  {
    result_2 = v4l2_close (overlayFileDescriptor_);
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
                  overlayFileDescriptor_));
    overlayFileDescriptor_ = -1;
  } // end IF

  return false;
}

template <typename LockType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
int
Stream_Module_CamSource_V4L_T<LockType,
                              SessionMessageType,
                              ProtocolMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_V4L_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::initialized_);

  int result = -1;
  int result_2 = -1;
  int error = 0;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  bool stop_processing = false;
  struct v4l2_buffer buffer;
  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer.memory = V4L2_MEMORY_USERPTR;
  struct v4l2_event event;
  ACE_OS::memset (&event, 0, sizeof (struct v4l2_event));
  Stream_Module_Device_BufferMapIterator_t iterator;
//  unsigned int queued, done = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  const SessionDataType& session_data_r = inherited::sessionData_->get ();

  // step1: start processing data...
//   ACE_DEBUG ((LM_DEBUG,
//               ACE_TEXT ("entering processing loop...\n")));
  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (result == 0)
    {
      ACE_ASSERT (message_block_p);
      message_type = message_block_p->msg_type ();
      switch (message_type)
      {
        case ACE_Message_Block::MB_STOP:
        {
          // clean up
          message_block_p->release ();
          message_block_p = NULL;

          // *NOTE*: when close()d manually (i.e. user abort, ...),
          //         'hasFinished_' will not have been set at this stage
          if (!hasFinished_)
          {
            hasFinished_ = true;
            // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
            //         --> continue
            inherited::finished ();
            // *NOTE*: (if passive,) STREAM_SESSION_END has been processed
            //         --> done
            if (inherited::thr_count_ == 0)
              goto done; // finished processing

            continue;
          } // end IF

done:
          result_2 = 0;

          goto continue_; // STREAM_SESSION_END has been processed
        }
        default:
          break;
      } // end SWITCH

      // process
      // *NOTE*: fire-and-forget message_block_p here
      inherited::handleMessage (message_block_p,
                                stop_processing);
      if (stop_processing)
      {
        // *IMPORTANT NOTE*: message_block_p has already been released() !

        hasFinished_ = true;
        // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
        //         --> continue
        inherited::finished ();

        continue;
      } // end IF
    } // end IF
    else if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

        if (!hasFinished_)
        {
          hasFinished_ = true;
          // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
          //         --> continue
          inherited::finished ();
        } // end IF

        break;
      } // end IF
    } // end IF

    // session aborted ?
    if (session_data_r.aborted)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("session aborted...\n")));

      hasFinished_ = true;
      // *NOTE*: (if active,) this enqueues STREAM_SESSION_END
      //         --> continue
      inherited::finished ();

      continue;
    } // end IF

#if defined (_DEBUG)
    // log device status to kernel log ?
    if (debug_)
    {
      result = v4l2_ioctl (captureFileDescriptor_,
                           VIDIOC_LOG_STATUS);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                    captureFileDescriptor_, ACE_TEXT ("VIDIOC_LOG_STATUS")));
    } // end IF
#endif

//    // dequeue pending events
//    result = v4l2_ioctl (captureFileDescriptor_,
//                         VIDIOC_DQEVENT,
//                         &event);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
//                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
//    } // end IF
//    else
//    {
//      for (unsigned int i = 0;
//           i < event.pending;
//           ++i)
//      {
//        result = v4l2_ioctl (captureFileDescriptor_,
//                             VIDIOC_DQEVENT,
//                             &event);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
//                      captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
//      } // end FOR
//    } // end ELSE

//    queued =
//        Stream_Module_Device_Tools::queued (captureFileDescriptor_,
//                                            inherited::configuration_->buffers,
//                                            done);
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("#queued/done buffers: %u/%u...\n"),
//                queued, done));

    // *NOTE*: blocks until:
    //         - a buffer is available
    //         - a frame has been written by the device
    result = v4l2_ioctl (captureFileDescriptor_,
                         VIDIOC_DQBUF,
                         &buffer);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQBUF")));
      break;
    } // end IF
    if (buffer.flags & V4L2_BUF_FLAG_ERROR)
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: streaming error (fd: %d, index: %d), continuing\n"),
                  inherited::mod_->name (),
                  captureFileDescriptor_, buffer.index));

    iterator = bufferMap_.find (buffer.index);
    ACE_ASSERT (iterator != bufferMap_.end ());
    message_block_p = (*iterator).second;
    message_block_p->reset ();
    message_block_p->wr_ptr (buffer.bytesused);

    result = inherited::put_next (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

      // clean up
      message_block_p->release ();

      break;
    } // end IF
  } while (true);

continue_:
  return result_2;
}
