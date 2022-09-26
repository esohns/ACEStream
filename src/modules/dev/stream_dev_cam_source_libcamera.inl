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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
                                    UserDataType>::Stream_Module_CamSource_LibCamera_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , camera_ (NULL)
 , cameraConfiguration_ (nullptr)
 , cameraManager_ (NULL)
 , cameraStream_ (NULL)
 , frameBufferAllocator_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_LibCamera_T::Stream_Module_CamSource_LibCamera_T"));

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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
                                    UserDataType>::~Stream_Module_CamSource_LibCamera_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_LibCamera_T::~Stream_Module_CamSource_LibCamera_T"));

  if (camera_)
  {
    camera_->stop ();
    camera_->release ();
  } // end IF

  if (cameraManager_)
  {
    cameraManager_->stop ();
    delete cameraManager_; cameraManager_ = NULL;
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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_LibCamera_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  DataMessageType* message_p = NULL;
  typename DataMessageType::DATA_T* message_data_p = NULL;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (camera_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      struct Stream_MediaFramework_LibCamera_MediaType media_type_s;

      // step1: set capture format ?
      ACE_ASSERT (!session_data_r.formats.empty ());
      media_type_s = getMediaType (session_data_r.formats.back ());

      libcamera::StreamRoles roles_a;
      libcamera::StreamConfiguration* configuration_p = NULL;
//      libcamera::FrameBuffer* buffer_p = NULL;
      int status_i = -1;
      std::unique_ptr<libcamera::Request> request_p;

      roles_a.push_back (libcamera::StreamRole::Viewfinder);
      cameraConfiguration_ = camera_->generateConfiguration (roles_a);
      if (!cameraConfiguration_.get ())
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to libcamera::Camera::generateConfiguration(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (!cameraConfiguration_->empty ());
      configuration_p = &cameraConfiguration_->at (0);
      configuration_p->pixelFormat = media_type_s.format;
      configuration_p->size = media_type_s.resolution;
      status_i = cameraConfiguration_->validate ();
      switch (status_i)
      {
        case libcamera::CameraConfiguration::Adjusted:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: adjusted camera configuration to \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (configuration_p->toString ().c_str ())));
          break;
        }
        case libcamera::CameraConfiguration::Valid:
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: using camera configuration \"%s\"\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (configuration_p->toString ().c_str ())));
          break;
        }
        case libcamera::CameraConfiguration::Invalid:
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to libcamera::CameraConfiguration::validate(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        }
      } // end SWITCH

      result = camera_->acquire ();
      ACE_ASSERT (result == 0);
      result = camera_->configure (cameraConfiguration_.get ());
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to libcamera::Camera::configure(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: configured camera\n"),
                  inherited::mod_->name ()));

      // step2: fill buffer queue(s)
      cameraStream_ = configuration_p->stream ();
      ACE_ASSERT (cameraStream_);
      ACE_ASSERT (frameBufferAllocator_);
      result = frameBufferAllocator_->allocate (cameraStream_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to allocate capture buffers, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      for (std::vector<std::unique_ptr<libcamera::FrameBuffer> >::const_iterator iterator = frameBufferAllocator_->buffers (cameraStream_).begin ();
           iterator != frameBufferAllocator_->buffers (cameraStream_).end ();
           ++iterator)
      {
        /* Map memory buffers and cache the mappings */
        const libcamera::FrameBuffer::Plane& plane_r =
            (*iterator)->planes ().front ();
        void* memory_p =
            ::mmap (NULL, plane_r.length, PROT_READ, MAP_SHARED, plane_r.fd.fd (), 0);
        mappedBuffers_[(*iterator).get ()] =
            std::make_pair (memory_p, plane_r.length);

        try {
          message_block_p =
              static_cast<ACE_Message_Block*> (inherited::allocator_->malloc (plane_r.length));
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                      plane_r.length));
          message_block_p = NULL;
          goto error;
        }
        ACE_ASSERT (message_block_p);
        message_block_p->base (static_cast<char*> (memory_p),
                               plane_r.length,
                               ACE_Message_Block::DONT_DELETE);
        message_block_p->wr_ptr (plane_r.length);

        request_p = camera_->createRequest (reinterpret_cast<uint64_t> (message_block_p));
        if (!request_p.get ())
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to allocate request, aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        message_p = dynamic_cast<DataMessageType*> (message_block_p);
        ACE_ASSERT (message_p);
        message_data_p =
            &const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
        message_data_p->buffer = (*iterator).get ();
        message_data_p->camera = camera_.get ();
        message_data_p->request = request_p.get ();
        message_data_p->stream = cameraStream_;

        result = request_p->addBuffer (cameraStream_, (*iterator).get ());
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to add buffer to request, aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        requests_.push_back (std::move (request_p));
      } // end FOR

      // step3: start stream
      result = camera_->start ();
      if (result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to libcamera::Camera::start(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      camera_->requestCompleted.connect (this, &OWN_TYPE_T::requestComplete);

      /* Queue all requests. */
      for (std::vector<std::unique_ptr<libcamera::Request> >::iterator iterator = requests_.begin ();
           iterator != requests_.end ();
           ++iterator)
      {
        result = camera_->queueRequest ((*iterator).get ());
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to libcamera::Camera::queueRequest(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end FOR

      break;

error:
      camera_->requestCompleted.disconnect (this, &OWN_TYPE_T::requestComplete);

      for (std::vector<std::unique_ptr<libcamera::Request> >::iterator iterator = requests_.begin ();
           iterator != requests_.end ();
           ++iterator)
      {
        message_block_p = reinterpret_cast<ACE_Message_Block*> ((*iterator)->cookie ());
        ACE_ASSERT (message_block_p);
        message_p = dynamic_cast<DataMessageType*> (message_block_p);
        ACE_ASSERT (message_p);
        message_data_p =
          &const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
        message_data_p->release = true;
        message_p->release ();
      } // end FOR
      requests_.clear ();

      camera_->stop ();
      result = camera_->release ();

      for (std::map<libcamera::FrameBuffer*, std::pair<void*, ACE_UINT32> >::iterator iterator = mappedBuffers_.begin ();
           iterator != mappedBuffers_.end ();
           ++iterator)
        munmap ((*iterator).second.first, (*iterator).second.second);
      mappedBuffers_.clear ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // step1: empty buffer queue(s)
      camera_->requestCompleted.disconnect (this, &OWN_TYPE_T::requestComplete);
      camera_->stop ();
      for (std::vector<std::unique_ptr<libcamera::Request> >::iterator iterator = requests_.begin ();
           iterator != requests_.end ();
           ++iterator)
      {
        message_block_p = reinterpret_cast<ACE_Message_Block*> ((*iterator)->cookie ());
        ACE_ASSERT (message_block_p);
        message_p = dynamic_cast<DataMessageType*> (message_block_p);
        ACE_ASSERT (message_p);
        message_data_p =
          &const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
        message_data_p->release = true;
        message_p->release ();
      } // end FOR
      requests_.clear ();

      // step2: stop stream
      result = camera_->release ();

      // step3: deallocate device buffer queue slots
      for (std::map<libcamera::FrameBuffer*, std::pair<void*, ACE_UINT32> >::iterator iterator = mappedBuffers_.begin ();
           iterator != mappedBuffers_.end ();
           ++iterator)
        munmap ((*iterator).second.first, (*iterator).second.second);
      mappedBuffers_.clear ();

      if (camera_)
      {
        camera_ = nullptr;
      } // end IF

      if (cameraManager_)
      {
        cameraManager_->stop ();
        delete cameraManager_; cameraManager_ = NULL;
      } // end IF

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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_LibCamera_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
//  data_out.dataMessages = 0;
//  data_out.droppedMessages = 0;
//  data_out.bytes = 0;
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
//Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
//                                    SessionMessageType,
//                                    DataMessageType,
//                                    ConfigurationType,
//                                    StreamStateType,
//                                    SessionDataType,
//                                    SessionDataContainerType,
//                                    StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_LibCamera_T::report"));

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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CamSource_LibCamera_T::initialize"));

  int result = -1;

  if (unlikely (inherited::isInitialized_))
  {
    if (camera_)
    {
      camera_->stop ();
      camera_->release ();
      camera_ = nullptr;
    } // end IF

    if (cameraManager_)
    {
      cameraManager_->stop ();
      delete cameraManager_; cameraManager_ = NULL;
    } // end IF

    if (frameBufferAllocator_)
    {
      delete frameBufferAllocator_; frameBufferAllocator_ = NULL;
    } // end IF
  } // end IF

  if (!cameraManager_)
    ACE_NEW_NORETURN (cameraManager_,
                      libcamera::CameraManager ());
  ACE_ASSERT (cameraManager_);
  result = cameraManager_->start ();
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to libcamera::CameraManager::start(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  ACE_ASSERT (!configuration_in.deviceIdentifier.identifier.empty ());
  ACE_ASSERT (!camera_);
  camera_ =
      Stream_Device_Tools::getCamera (cameraManager_,
                                      configuration_in.deviceIdentifier.identifier);
  if (!camera_.get ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Device_Tools::getCamera() (was: \"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.deviceIdentifier.identifier.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (!frameBufferAllocator_);
  ACE_NEW_NORETURN (frameBufferAllocator_,
                    libcamera::FrameBufferAllocator (camera_));
  ACE_ASSERT (frameBufferAllocator_);

  return inherited::initialize (configuration_in,
                                allocator_in);
error:
  if (cameraManager_)
  {
    cameraManager_->stop ();
    delete cameraManager_; cameraManager_ = NULL;
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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_LibCamera_T::svc"));

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
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool release_lock = false;
  int result = -1;
  int result_2 = -1;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
  bool stop_processing = false;
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
        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage

        // signal the controller ?
        if (!has_finished)
        {
          has_finished = true;
          // enqueue(/process) STREAM_SESSION_END
          inherited::STATE_MACHINE_T::finished ();
        } // end IF

        if (inherited::thr_count_ > 1)
        {
          result_2 = inherited::putq (message_block_p, NULL);
          if (result_2 == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
        } // end IF
        else
          message_block_p->release ();

        // has STREAM_SESSION_END been processed ?
        if (!inherited::sessionEndProcessed_)
          continue; // process STREAM_SESSION_END

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
        message_block_p = NULL;

        if (release_lock)
          stream_p->unlock (false);

        // finished ?
        if (stop_processing)
        {
          // *IMPORTANT NOTE*: message_block_p has already been released() !

          if (!has_finished)
          {
            has_finished = true;
            // enqueue(/process) STREAM_SESSION_END
            inherited::STATE_MACHINE_T::finished ();
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
#if defined (_DEBUG)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session %u aborted\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId));
#endif // _DEBUG
        has_finished = true;
        // enqueue(/process) STREAM_SESSION_END
        inherited::STATE_MACHINE_T::finished ();
      } // end IF
    } // end lock scope
  } while (true);
  result = -1;

done:
  return result;
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
Stream_Module_CamSource_LibCamera_T<ACE_SYNCH_USE,
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
                                    UserDataType>::requestComplete (libcamera::Request* request_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_LibCamera_T::requestComplete"));

  // sanity check(s)
  ACE_ASSERT (request_in);
  if (request_in->status () == libcamera::Request::RequestCancelled)
    return;
  const std::map<const libcamera::Stream*, libcamera::FrameBuffer*>& buffer_map_r =
      request_in->buffers ();
  ACE_ASSERT (buffer_map_r.count (cameraStream_));
  libcamera::FrameBuffer* buffer_p = buffer_map_r.at (cameraStream_);
  ACE_ASSERT (buffer_p);

  ACE_Message_Block* message_block_p = reinterpret_cast<ACE_Message_Block*> (request_in->cookie ());
  ACE_ASSERT (message_block_p);
  int result = inherited::putq (message_block_p, NULL);
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::putq(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
}
