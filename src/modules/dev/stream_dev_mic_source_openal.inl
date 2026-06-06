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

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "common_image_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::Stream_Dev_Mic_Source_OpenAL_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , device_ (NULL)
 , frameSize_ (0)
 , sessionId_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::Stream_Dev_Mic_Source_OpenAL_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::~Stream_Dev_Mic_Source_OpenAL_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::~Stream_Dev_Mic_Source_OpenAL_T"));

  if (device_)
  {
    alcCaptureStop (device_);
    alcCaptureCloseDevice (device_);
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::initialize"));

  bool result;

  if (inherited::isInitialized_)
  {
    if (device_)
    {
      alcCaptureStop (device_);
      alcCaptureCloseDevice (device_); device_ = NULL;
    } // end IF
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::initialize(), aborting\n"),
                inherited::mod_->name ()));

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
         typename StatisticContainerType,
         typename SessionManagerType,
         typename TimerManagerType,
         typename UserDataType,
         typename MediaType>
void
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (sessionId_);

  message_inout->initialize (sessionId_,
                             NULL);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::handleSessionMessage"));

  int result;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      inherited::isHighPriorityStop_ = true;
      inherited::change (STREAM_STATE_SESSION_STOPPING);

      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      MediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));
      ALCenum error_e;

      sessionId_ = session_data_r.sessionId;

      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                            NULL,                                                                     // asynchronous completion token
                                            COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                            inherited::configuration_->statisticCollectionInterval);                  // interval
        if (inherited::timerId_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    inherited::timerId_,
                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_2);

      frameSize_ =
        (inherited2::getBitsPerSample (media_type_2) / 8) *
         inherited2::getChannels (media_type_2);

      // *TODO*: remove type inferences
      if (!initialize_OpenAL (inherited::configuration_->deviceIdentifier,
                              media_type_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_OpenAL(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (device_);

      alcCaptureStart (device_);
      error_e = alcGetError (device_);
      ACE_ASSERT (error_e == ALC_NO_ERROR);

      inherited2::free_ (media_type_2);

      break;

error:
     inherited2::free_ (media_type_2);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      // *NOTE*: only process the first 'session end' message
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (unlikely (inherited::sessionEndProcessed_))
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (device_)
      {
        alcCaptureStop (device_);
        alcCaptureCloseDevice (device_); device_ = NULL;
      } // end IF

      if (inherited::timerId_ != -1)
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      {
        Common_ITask* itask_p = this; // *TODO*: is there no other way ?
        itask_p->stop (false,                           // wait for completion ?
                       inherited::isHighPriorityStop_); // high priority ?
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data

  //// step2: send the information downstream
  //if (!inherited::putStatisticMessage (data_out)) // data container
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n")));
  //  return false;
  //} // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::initialize_OpenAL (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                              const MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::initialize_OpenAL"));

  // sanity check(s)
  ACE_ASSERT (!device_);

  // *TODO*: open a specific device corresponding to the first argument
  const ALCchar* default_device_name_p =
    alcGetString (NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
  ACE_ASSERT (default_device_name_p);
  unsigned int number_of_channels_i = inherited2::getChannels (mediaType_in);
  unsigned int bytes_per_sample_i =
    inherited2::getBitsPerSample (mediaType_in) / 8;
  ALCenum error_e;

  device_ =
    alcCaptureOpenDevice (default_device_name_p,
                          inherited2::getSampleRate (mediaType_in),
                          bytes_per_sample_i == 1 ? number_of_channels_i == 1 ? AL_FORMAT_MONO8
                                                                              : AL_FORMAT_STEREO8
                                                  : number_of_channels_i == 1 ? AL_FORMAT_MONO16
                                                                              : AL_FORMAT_STEREO16,
                          STREAM_DEV_OPENAL_DEFAULT_BUFFER_SIZE);
  error_e = alcGetError (device_);
  if (unlikely (!device_ || (error_e != ALC_NO_ERROR)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to alcCaptureOpenDevice(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
int
Stream_Dev_Mic_Source_OpenAL_T<ACE_SYNCH_USE,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               ConfigurationType,
                               StreamControlType,
                               StreamNotificationType,
                               StreamStateType,
                               StatisticContainerType,
                               SessionManagerType,
                               TimerManagerType,
                               UserDataType,
                               MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_OpenAL_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
    Common_Error_Tools::setThreadName (inherited::threadName_,
                                       NULL);
#else
    Common_Error_Tools::setThreadName (inherited::threadName_,
                                       0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT ("")),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? inherited::grp_id_
                                                                                             : -1)));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);

  int error = 0;
  bool has_finished = false;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = ACE_OS::gettimeofday ();
  bool release_lock = false;
  int result = -1;
  int result_2 = -1;
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  bool stop_processing = false;
  typename inherited::ISTREAM_T* stream_p =
    const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
  ALint num_available_frames_i = 0;
  size_t frames_to_read_i = 0;
  ALCenum error_e = alGetError ();

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
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
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
          inherited::finished (false); // recurse upstream ?
        } // end IF

        if (inherited::thr_count_ > 1)
        {
          result_2 = inherited::putq (message_block_p, NULL);
          if (result_2 == -1)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_block_p->release (); message_block_p = NULL;
          } // end IF
          message_block_p = NULL;
        } // end IF
        else
        {
          message_block_p->release (); message_block_p = NULL;
        } // end ELSE

        // has STREAM_SESSION_END been processed ?
        if (!inherited::sessionEndProcessed_)
          continue; // process STREAM_SESSION_END

        result = 0;

        goto done; // STREAM_SESSION_END has been processed
      }
      default:
      { ACE_ASSERT (stream_p);
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
            inherited::change (STREAM_STATE_SESSION_STOPPING);
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
        // enqueue(/process) STREAM_SESSION_END
        inherited::change (STREAM_STATE_SESSION_STOPPING);
      } // end IF
    } // end lock scope

    alcGetIntegerv (device_, ALC_CAPTURE_SAMPLES, 1, &num_available_frames_i);
    error_e = alcGetError (device_);
    ACE_ASSERT (error_e == ALC_NO_ERROR);
    if (unlikely (num_available_frames_i <= 0))
      continue;

    ACE_ASSERT (!message_block_p);
    message_block_p =
      // inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
      inherited::allocateMessage (num_available_frames_i * frameSize_);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u) failed: \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  // inherited::configuration_->allocatorConfiguration->defaultBufferSize));
                  num_available_frames_i * frameSize_));
      break;
    } // end IF

    frames_to_read_i =
      std::min (message_block_p->space () / frameSize_, static_cast<size_t> (num_available_frames_i));
    alcCaptureSamples (device_, (ALCvoid*)message_block_p->wr_ptr (), frames_to_read_i);
    error_e = alcGetError (device_);
    ACE_ASSERT (error_e == ALC_NO_ERROR);
    message_block_p->wr_ptr (static_cast<unsigned int> (frames_to_read_i) * frameSize_);
    session_data_r.statistic.capturedFrames += frames_to_read_i;

    result_2 = this->put (message_block_p, NULL);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      break;
    } // end IF
  } while (true);
  result = -1;

done:
  return result;
}
