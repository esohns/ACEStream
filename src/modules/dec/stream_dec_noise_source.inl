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

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_tools.h"

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
          typename TimerManagerType,
          typename MediaType>
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          TimerManagerType,
                          MediaType>::Stream_Dec_Noise_Source_T (ISTREAM_T* stream_in)
 : inherited (stream_in,                            // stream handle
              false,                                // auto-start ?
              STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency
              true)                                 // generate session messages ?
 , inherited2 ()
 , frameSize_ (0)
 , handler_ (this,
             false)
 , mediaType_ ()
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , task_ (NULL)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::Stream_Dec_Noise_Source_T"));

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
          typename TimerManagerType,
          typename MediaType>
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          TimerManagerType,
                          MediaType>::~Stream_Dec_Noise_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::~Stream_Dec_Noise_Source_T"));

  long timer_id = handler_.get ();
  if (unlikely (timer_id != -1))
  {
    typename TimerManagerType::INTERFACE_T* itimer_manager_p =
      ((inherited::configuration_ && inherited::configuration_->timerManager) ? inherited::configuration_->timerManager
                                                                              : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
    ACE_ASSERT (itimer_manager_p);

    const void* act_p = NULL;
    int result = itimer_manager_p->cancel_timer (timer_id,
                                                 &act_p);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  timer_id));
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_MediaFramework_DirectShow_Tools::free (mediaType_);

  if (unlikely (task_))
    AvRevertMmThreadCharacteristics (task_);
#endif // ACE_WIN32 || ACE_WIN64
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
          typename TimerManagerType,
          typename MediaType>
bool
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          TimerManagerType,
                          MediaType>::initialize (const ConfigurationType& configuration_in,
                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    frameSize_ = 0;

    long timer_id = handler_.get ();
    if (unlikely (timer_id != -1))
    {
      ACE_ASSERT (inherited::configuration_);
      typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
         : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
      ACE_ASSERT (itimer_manager_p);

      const void* act_p = NULL;
      int result = itimer_manager_p->cancel_timer (timer_id,
                                                   &act_p);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    timer_id));
      handler_.set (-1);
    } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    Stream_MediaFramework_DirectShow_Tools::free (mediaType_);

    if (unlikely (task_))
    {
      AvRevertMmThreadCharacteristics (task_);
      task_ = NULL;
    } // end IF
#else
    ACE_OS::memset (&mediaType_, 0, sizeof (struct Stream_MediaFramework_ALSA_MediaType));
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
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
          typename TimerManagerType,
          typename MediaType>
void
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          TimerManagerType,
                          MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_Time_Value interval;

      // schedule regular statistic collection
      if (inherited::configuration_->statisticCollectionInterval !=
          ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                            NULL,                                                                     // asynchronous completion token
                                            COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                            inherited::configuration_->statisticCollectionInterval);                  // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    inherited::mod_->name (),
//                    inherited::timerId_,
//                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      // get media type / frame size
      ACE_ASSERT (inherited::configuration_->generatorConfiguration);
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (mediaType_);
      inherited2::getMediaType (session_data_r.formats.back (),
                                mediaType_);
      ACE_ASSERT (InlineIsEqualGUID (mediaType_.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (mediaType_.pbFormat);
      frameSize_ =
        (waveformatex_p->wBitsPerSample / 8) * waveformatex_p->nChannels;
      ACE_ASSERT (frameSize_ == waveformatex_p->nBlockAlign);
      inherited::configuration_->generatorConfiguration->samplesPerSecond =
        waveformatex_p->nSamplesPerSec;
      inherited::configuration_->generatorConfiguration->bytesPerSample =
        waveformatex_p->wBitsPerSample / 8;
      inherited::configuration_->generatorConfiguration->numberOfChannels =
        waveformatex_p->nChannels;
      // *NOTE*: Microsoft(TM) uses signed little endian
      inherited::configuration_->generatorConfiguration->isLittleEndianFormat =
        true;
      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0–255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      inherited::configuration_->generatorConfiguration->isSignedFormat =
        !(inherited::configuration_->generatorConfiguration->bytesPerSample == 1);
#else
      inherited2::getMediaType (session_data_r.formats.back (),
                                mediaType_);
      frameSize_ =
        (snd_pcm_format_width (mediaType_.format.format) / 8) * mediaType_.format.channels;
      inherited::configuration_->generatorConfiguration->samplesPerSecond =
        mediaType_.format.rate;
      inherited::configuration_->generatorConfiguration->bytesPerSample =
        snd_pcm_format_width (mediaType_.format.format) / 8);
      inherited::configuration_->generatorConfiguration->numberOfChannels =
        mediaType_.format.channels;
      inherited::configuration_->generatorConfiguration->isLittleEndianFormat =
        (snd_pcm_format_is_little_endian (mediaType_.format.format) == 1);
      inherited::configuration_->generatorConfiguration->isSignedFormat =
        (snd_pcm_format_is_signed (mediaType_.format.format) == 1);
#endif // ACE_WIN32 || ACE_WIN64

      // determine interval size from buffer size
      // *IMPORTANT NOTE*: larger buffers are more efficient, but introduce more
      //                   latency
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
      suseconds_t buffer_time_us =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        static_cast<suseconds_t> ((inherited::configuration_->allocatorConfiguration->defaultBufferSize / static_cast<double> (waveformatex_p->nAvgBytesPerSec)) * 1000000.0);
#else
        static_cast<suseconds_t> ((inherited::configuration_->allocatorConfiguration->defaultBufferSize / static_cast<double> (frameSize_ * mediaType_.format.rate)) * 1000000.0);
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      DWORD task_index_i = 0;
      ACE_ASSERT (!task_);
      task_ =
        AvSetMmThreadCharacteristics (TEXT (STREAM_LIB_WASAPI_CAPTURE_DEFAULT_TASKNAME),
                                      &task_index_i);
      if (!task_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to AvSetMmThreadCharacteristics(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64

      // start sample generator timer
      ACE_ASSERT (buffer_time_us > 5000);
      interval.set (0, buffer_time_us - 5000);
      long timer_id =
        itimer_manager_p->schedule_timer (&handler_,                  // event handler handle
                                          NULL,                       // asynchronous completion token
                                          COMMON_TIME_NOW + interval, // first wakeup time
                                          interval);                  // interval
      if (unlikely (timer_id == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      handler_.set (timer_id);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: scheduled generator timer (id: %d) for interval %#T\n"),
                  inherited::mod_->name (),
                  timer_id,
                  &interval));

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      long timer_id = handler_.get ();
      if (likely (timer_id != -1))
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (timer_id,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      timer_id));
        handler_.set (-1);
      } // end IF

      if (likely (inherited::timerId_ != -1))
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      if (unlikely (task_))
      {
        AvRevertMmThreadCharacteristics (task_);
        task_ = NULL;
      } // end IF
#endif // ACE_WIN32 || ACE_WIN64

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      {
        Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,  // wait for completion ?
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
          typename TimerManagerType,
          typename MediaType>
void
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          TimerManagerType,
                          MediaType>::handle (const void* act_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::handle"));

  ACE_UNUSED_ARG (act_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::allocator_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->generatorConfiguration);

  ACE_Message_Block* message_block_p = NULL;
  unsigned int number_of_frames_i = 0;
  int result = -1;

  // step1: allocate buffer
  try {
    message_block_p =
      static_cast<ACE_Message_Block*> (inherited::allocator_->malloc (inherited::configuration_->allocatorConfiguration->defaultBufferSize));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    message_block_p = NULL;
  }
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // step2: write frames
  ACE_ASSERT ((inherited::configuration_->allocatorConfiguration->defaultBufferSize % frameSize_) == 0);
  number_of_frames_i =
    inherited::configuration_->allocatorConfiguration->defaultBufferSize / frameSize_;
  switch (inherited::configuration_->generatorConfiguration->type)
  {
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH:
    {
      ACE_ASSERT (false); // *TODO*
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE:
    {
      Stream_Module_Decoder_Tools::sinus (inherited::configuration_->generatorConfiguration->frequency,
                                          inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                          inherited::configuration_->generatorConfiguration->bytesPerSample,
                                          inherited::configuration_->generatorConfiguration->numberOfChannels,
                                          inherited::configuration_->generatorConfiguration->isSignedFormat,
                                          inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                          reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                          number_of_frames_i,
                                          inherited::configuration_->generatorConfiguration->phase);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE:
    {
      ACE_ASSERT (false); // *TODO*
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE:
    {
      ACE_ASSERT (false); // *TODO*
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unkown noise type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->generatorConfiguration->type));
      goto error;
    }
  } // end SWITCH
  message_block_p->wr_ptr (number_of_frames_i * frameSize_);

  // step3: push data downstream
  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  return;

error:
  if (message_block_p)
    message_block_p->release ();

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
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
          typename TimerManagerType,
          typename MediaType>
bool
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          TimerManagerType,
                          MediaType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data

  //// step2: send the information downstream
  //if (!inherited::putStatisticMessage (data_out)) // data container
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n")));
  //              inherited::mod_->name ()));
  //  return false;
  //} // end IF

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
//Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}
