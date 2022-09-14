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

#include "libv4l2.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_dev_tools.h"

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
          typename UserDataType>
Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
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
                              UserDataType>::Stream_Module_CamSource_V4L_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , captureFileDescriptor_ (-1)
 , overlayFileDescriptor_ (-1)
 , bufferMap_ ()
 , isPassive_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::Stream_Module_CamSource_V4L_T"));

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
          typename UserDataType>
Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
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
                              UserDataType>::~Stream_Module_CamSource_V4L_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::~Stream_Module_CamSource_V4L_T"));

  int result = -1;

  if (likely (!isPassive_))
    if (unlikely (captureFileDescriptor_ != -1))
    {
      result = v4l2_close (captureFileDescriptor_);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to v4l2_close(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    captureFileDescriptor_));
    } // end IF
  if (unlikely (overlayFileDescriptor_ != -1))
  {
    result = v4l2_close (overlayFileDescriptor_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to v4l2_close(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  overlayFileDescriptor_));
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
          typename UserDataType>
void
Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
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
                              UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      int toggle = 1;
      struct Stream_MediaFramework_V4L_MediaType media_type_s;

      if (isPassive_)
        goto next;

      // step1: set capture format ?
      ACE_ASSERT (captureFileDescriptor_ != -1);
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);

      if (unlikely (!Stream_Device_Tools::setFormat (captureFileDescriptor_,
                                                     media_type_s.format)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Device_Tools::setFormat(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    captureFileDescriptor_));
        goto error;
      } // end IF
      // *TODO*: remove type inference
      if (unlikely (!Stream_Device_Tools::setFrameRate (captureFileDescriptor_,
                                                        media_type_s.frameRate)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Device_Tools::setFrameRate(%d), returning\n"),
                    inherited::mod_->name (),
                    captureFileDescriptor_));
        goto error;
      } // end IF

next:
      // step2: set buffering method
      if (unlikely (!Stream_Device_Tools::initializeCapture (captureFileDescriptor_,
                                                             inherited::configuration_->method,
                                                             inherited::configuration_->buffers)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Device_Tools::initializeCapture(%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    captureFileDescriptor_));
        goto error;
      } // end IF
      if (unlikely (overlayFileDescriptor_ != -1))
      { ACE_ASSERT (false); // *TODO*
        struct v4l2_window window_s;
        ACE_OS::memset (&window_s, 0, sizeof (struct v4l2_window));
    //    window_s.bitmap = configuration_in.window;
//        window_s.w = inherited::configuration_->area;
        if (!Stream_Device_Tools::initializeOverlay (overlayFileDescriptor_,
                                                     window_s))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Device_Tools::initializeOverlay(%d): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      overlayFileDescriptor_));
          goto error;
        } // end IF
      } // end IF

      // step3: fill buffer queue(s)
      if (likely (captureFileDescriptor_ != -1))
        if (unlikely (!Stream_Device_Tools::initializeBuffers<DataMessageType> (captureFileDescriptor_,
                                                                                inherited::configuration_->method,
                                                                                inherited::configuration_->buffers,
                                                                                bufferMap_,
                                                                                inherited::allocator_,
                                                                                inherited::configuration_->allocatorConfiguration)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Device_Tools::initializeBuffers(%d): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      captureFileDescriptor_));
          goto error;
        } // end IF

      // step4: start stream
      if (likely (captureFileDescriptor_ != -1))
      {
        result = v4l2_ioctl (captureFileDescriptor_,
                             VIDIOC_STREAMON,
                             &toggle);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      captureFileDescriptor_, ACE_TEXT ("VIDIOC_STREAMON")));
          goto error;
        } // end IF
      } // end IF
      if (unlikely (overlayFileDescriptor_ != -1))
      {
        result = v4l2_ioctl (overlayFileDescriptor_,
                             VIDIOC_OVERLAY,
                             &toggle);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      overlayFileDescriptor_, ACE_TEXT ("VIDIOC_OVERLAY")));
          goto error;
        } // end IF
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
//      ACE_ASSERT (false); // *TODO*
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      int toggle = 0;
      //bool shutdown = true;

      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // step1: empty buffer queue(s)
      if (likely (captureFileDescriptor_ != -1))
        Stream_Device_Tools::finalizeBuffers<DataMessageType> (captureFileDescriptor_,
                                                               inherited::configuration_->method,
                                                               bufferMap_);

      // step2: stop stream
      if (unlikely (overlayFileDescriptor_ != -1))
      {
        result = v4l2_ioctl (overlayFileDescriptor_,
                             VIDIOC_OVERLAY,
                             &toggle);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      overlayFileDescriptor_, ACE_TEXT ("VIDIOC_OVERLAY")));
      } // end IF
      if (likely (captureFileDescriptor_ != -1))
      {
        toggle = 1; // ?
        result = v4l2_ioctl (captureFileDescriptor_,
                             VIDIOC_STREAMOFF,
                             &toggle);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                      inherited::mod_->name (),
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
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    captureFileDescriptor_, ACE_TEXT ("VIDIOC_REQBUFS")));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: de-allocated %d device buffer slot(s)\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->buffers));

//      if (likely (captureFileDescriptor_ != -1))
//      {
//        result = v4l2_close (captureFileDescriptor_);
//        if (unlikely (result == -1))
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
//                      captureFileDescriptor_));
//      } // end IF
//      if (unlikely (overlayFileDescriptor_ != -1))
//      {
//        result = v4l2_close (overlayFileDescriptor_);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to v4l2_close(%d): \"%m\", continuing\n"),
//                      overlayFileDescriptor_));
//      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      { Common_ITask* itask_p = this;
        itask_p->stop (false,  // wait ?
                       false); // high priority ?
      } // end IF

      break;
    }
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
          typename UserDataType>
bool
Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
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
                              UserDataType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::collect"));

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
  if (unlikely (!inherited::putStatisticMessage (data_out)))
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
//          typename DataMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
//                              SessionMessageType,
//                              DataMessageType,
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
          typename UserDataType>
bool
Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
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
                              UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_V4L_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    if (!isPassive_)
      if (captureFileDescriptor_ != -1)
      {
        result = v4l2_close (captureFileDescriptor_);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to v4l2_close(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      captureFileDescriptor_));
        captureFileDescriptor_ = -1;
      } // end IF
    if (overlayFileDescriptor_ != -1)
    {
      result = v4l2_close (overlayFileDescriptor_);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to v4l2_close(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    overlayFileDescriptor_));
      overlayFileDescriptor_ = -1;
    } // end IF
    isPassive_ = false;
  } // end IF

  // *NOTE*: use O_NONBLOCK with a reactor (v4l2_select()) or proactor
  //         (v4l2_poll()) for asynchronous operation
  // *TODO*: support O_NONBLOCK
  //  int open_mode = O_RDONLY;
  int open_mode_i =
      ((configuration_in.method == V4L2_MEMORY_MMAP) ? O_RDWR : O_RDONLY);

  // *TODO*: remove type inferences
  captureFileDescriptor_ = configuration_in.deviceIdentifier.fileDescriptor;
  if (unlikely (captureFileDescriptor_ != -1))
  {
    isPassive_ = true;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode: using v4l2 device \"%s\" (fd: %d)\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.deviceIdentifier.identifier.c_str ()),
                captureFileDescriptor_));
  } // end IF
  else
  {
    // *TODO*: remove type inference
    captureFileDescriptor_ =
        v4l2_open (configuration_in.deviceIdentifier.identifier.c_str (),
                   open_mode_i);
    if (unlikely (captureFileDescriptor_ == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.deviceIdentifier.identifier.c_str ()),
                  open_mode_i));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: opened v4l2 device \"%s\" (fd: %d)\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.deviceIdentifier.identifier.c_str ()),
                captureFileDescriptor_));
  } // end ELSE
  ACE_ASSERT (captureFileDescriptor_ != -1);

  // *TODO*: remove type inference
  if (unlikely (Stream_Device_Tools::canOverlay (captureFileDescriptor_)))
  {
    overlayFileDescriptor_ =
        v4l2_open (configuration_in.deviceIdentifier.identifier.c_str (),
                   open_mode_i);
    if (overlayFileDescriptor_ == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to v4l2_open(\"%s\",%u): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.deviceIdentifier.identifier.c_str ()),
                  open_mode_i));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: opened v4l2 device \"%s\" (fd: %d)\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.deviceIdentifier.identifier.c_str ()),
                overlayFileDescriptor_));
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);

error:
  if (!isPassive_ &&
      (captureFileDescriptor_ != -1))
  {
    result = v4l2_close (captureFileDescriptor_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to v4l2_close(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  captureFileDescriptor_));
    captureFileDescriptor_ = -1;
  } // end IF
  if (overlayFileDescriptor_ != -1)
  {
    result = v4l2_close (overlayFileDescriptor_);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to v4l2_close(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  overlayFileDescriptor_));
    overlayFileDescriptor_ = -1;
  } // end IF

  return false;
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
          typename UserDataType>
int
Stream_Module_CamSource_V4L_T<ACE_SYNCH_USE,
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
                              UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_V4L_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              inherited::grp_id_));

  int error = 0;
  bool has_finished = false;
  ACE_Message_Block* message_block_p = NULL;
  DataMessageType* message_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool release_lock = false;
  int result = -1;
  int result_2 = -1;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
  bool stop_processing = false;

  struct v4l2_buffer buffer_s;
  ACE_OS::memset (&buffer_s, 0, sizeof (struct v4l2_buffer));
  buffer_s.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buffer_s.memory = inherited::configuration_->method;
//  struct v4l2_event event_s;
//  ACE_OS::memset (&event_s, 0, sizeof (struct v4l2_event));
  Stream_Device_BufferMapIterator_t iterator;
  typename inherited::ISTREAM_T* stream_p =
      const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());

  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p,
                                &no_wait);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Linux: 11 | Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
      goto continue_;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        if (unlikely (inherited::isHighPriorityStop_))
        {
          if (likely (!inherited::abortSent_))
            inherited::control (STREAM_CONTROL_ABORT,
                                false); // forward upstream ?
        } // end IF

        // *IMPORTANT NOTE*: when close()d manually (i.e. on a user abort),
        //                   the stream may not have finish()ed at this point
        bool finish_b = true;
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (inherited::sessionEndSent_ || inherited::sessionEndProcessed_)
            finish_b = false;
        } // end lock scope
        if (likely (finish_b))
        {
          // enqueue(/process) STREAM_SESSION_END
          inherited::finished (false); // recurse upstream ?
          message_block_p->release (); message_block_p = NULL;
          continue;
        } // end IF

        // *NOTE*: this is racy; the penultimate thread may have left svc() and
        //         not have decremented thr_count_ yet. In this case, the
        //         stop-message might remain in the queue during shutdown (or,
        //         even worse-) during re-initialization...
        // *TODO*: ward against this scenario
        if (unlikely (inherited::thr_count_ > 1))
        {
          result_2 =
            (inherited::isHighPriorityStop_ ? inherited::ungetq (message_block_p, NULL)
                                            : inherited::putq (message_block_p, NULL));
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_block_p->release (); message_block_p = NULL;
            return -1;
          } // end IF
        } // end IF
        else
        {
          message_block_p->release (); message_block_p = NULL;
        } // end ELSE
        inherited::isHighPriorityStop_ = false;

        result = 0;

        goto done; // STREAM_SESSION_END has been processed
      }
      default:
      {
        // sanity check(s)
        ACE_ASSERT (stream_p);

        // grab lock if processing is 'non-concurrent'
        if (!inherited::configuration_->hasReentrantSynchronousSubDownstream)
          release_lock = stream_p->lock (true);

        inherited::handleMessage (message_block_p,
                                  stop_processing);

        if (release_lock)
          stream_p->unlock (false);

        // finished ?
        if (stop_processing)
        {
          // *IMPORTANT NOTE*: message_block_p has already been released() !

          bool finish_b = true;
          { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
            if (inherited::sessionEndSent_ || inherited::sessionEndProcessed_)
              finish_b = false;
          } // end lock scope
          if (likely (finish_b))
          {
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?
          } // end IF

          continue;
        } // end IF

        break;
      }
    } // end SWITCH

continue_:
    // session aborted ?
    // sanity check(s)
    // *TODO*: remove type inferences
    ACE_ASSERT (session_data_r.lock);
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock, result);
      if (session_data_r.aborted &&
          !has_finished)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session %u aborted\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId));
        has_finished = true;

        bool finish_b = true;
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (inherited::sessionEndSent_ || inherited::sessionEndProcessed_)
            finish_b = false;
        } // end lock scope
        if (likely (finish_b))
        {
          // enqueue(/process) STREAM_SESSION_END
          inherited::finished (false); // recurse upstream ?
        } // end IF
      } // end IF
    } // end lock scope

#if defined (_DEBUG)
    // log device status to kernel log ?
    if (unlikely (inherited::configuration_->debug))
    {
      result = v4l2_ioctl (captureFileDescriptor_,
                           VIDIOC_LOG_STATUS);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    captureFileDescriptor_, ACE_TEXT ("VIDIOC_LOG_STATUS")));
    } // end IF
#endif // _DEBUG

//    // dequeue pending events
//    result = v4l2_ioctl (captureFileDescriptor_,
//                         VIDIOC_DQEVENT,
//                         &event_s);
//    if (result == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
//                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
//    } // end IF
//    else
//    {
//      for (unsigned int i = 0;
//           i < event_s.pending;
//           ++i)
//      {
//        result = v4l2_ioctl (captureFileDescriptor_,
//                             VIDIOC_DQEVENT,
//                             &event_s);
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
    result_2 = v4l2_ioctl (captureFileDescriptor_,
                           VIDIOC_DQBUF,
                           &buffer_s);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQBUF")));
      break;
    } // end IF
//    ACE_ASSERT (buffer_s.flags & V4L2_BUF_FLAG_DONE);
    if (unlikely (buffer_s.flags & V4L2_BUF_FLAG_ERROR))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: streaming error (fd: %d, index: %d), continuing\n"),
                  inherited::mod_->name (),
                  captureFileDescriptor_, buffer_s.index));

    iterator = bufferMap_.find (buffer_s.index);
    ACE_ASSERT (iterator != bufferMap_.end ());
    message_block_p = (*iterator).second;
    message_block_p->reset ();
    message_block_p->wr_ptr (buffer_s.bytesused);
    message_p = static_cast<DataMessageType*> (message_block_p);
    message_p->initialize (session_data_r.sessionId,
                           NULL);

    result_2 = inherited::put_next (message_block_p, NULL);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      break;
    } // end IF
  } while (true);
  result = -1;

done:
  return result;
}
