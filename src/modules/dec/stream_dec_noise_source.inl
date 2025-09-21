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
#include "avrt.h"
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/OS.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "common_tools.h"

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_noise_tools.h"

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
          typename MediaType>
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          MediaType>::Stream_Dec_Noise_Source_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , phase_ (0.0)
 , realDistribution_ ()
 , integerDistribution_ ()
 , signedIntegerDistribution_ ()
 , realDistribution_2 ()
 , multipliers_ (NULL)
 , history_ (NULL)
#if defined (LIBNOISE_SUPPORT)
 , noiseModule_ ()
#endif // LIBNOISE_SUPPORT
 , bufferSize_ (0)
 , frameSize_ (0)
 , handler_ (this,
             false)
 , mediaType_ ()
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
 //, task_ (NULL)
//#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::Stream_Dec_Noise_Source_T"));

#if defined (LIBNOISE_SUPPORT)
  noiseModule_.SetSeed (static_cast<int> (Common_Tools::randomSeed));
#endif // LIBNOISE_SUPPORT
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
          typename MediaType>
Stream_Dec_Noise_Source_T<ACE_SYNCH_USE,
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
                          MediaType>::~Stream_Dec_Noise_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::~Stream_Dec_Noise_Source_T"));

  if (multipliers_)
    delete [] multipliers_;
  if (history_)
    delete [] history_;

  long timer_id = handler_.get_2 ();
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

  //if (unlikely (task_))
  //  AvRevertMmThreadCharacteristics (task_);
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
          typename StatisticContainerType,
          typename SessionManagerType,
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
                          StatisticContainerType,
                          SessionManagerType,
                          TimerManagerType,
                          MediaType>::initialize (const ConfigurationType& configuration_in,
                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    phase_ = 0.0;
    realDistribution_.reset ();
    integerDistribution_.reset ();
    signedIntegerDistribution_.reset ();
    realDistribution_2.reset ();

    bufferSize_ = 0;
    frameSize_ = 0;

    long timer_id = handler_.get_2 ();
    if (unlikely (timer_id != -1))
    {
      ACE_ASSERT (inherited::configuration_);
      typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (inherited::configuration_->timerManager ? inherited::configuration_->timerManager : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
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

    //if (unlikely (task_))
    //{
    //  AvRevertMmThreadCharacteristics (task_);
    //  task_ = NULL;
    //} // end IF
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
          typename StatisticContainerType,
          typename SessionManagerType,
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
                          StatisticContainerType,
                          SessionManagerType,
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

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_Time_Value interval;
      long timer_id = -1;
      suseconds_t buffer_time_us = 0;
      long double a = 1.0l;

      // schedule regular statistic collection
      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
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
                                STREAM_MEDIATYPE_AUDIO,
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
      inherited::configuration_->generatorConfiguration->isFloatFormat =
        Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p);
      // *NOTE*: Microsoft(TM) uses signed little endian
      inherited::configuration_->generatorConfiguration->isLittleEndianFormat =
        true;
      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      inherited::configuration_->generatorConfiguration->isSignedFormat =
        !(inherited::configuration_->generatorConfiguration->bytesPerSample == 1);
#else
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                mediaType_);
      frameSize_ =
        (snd_pcm_format_width (mediaType_.format) / 8) * mediaType_.channels;
      inherited::configuration_->generatorConfiguration->samplesPerSecond =
        mediaType_.rate;
      inherited::configuration_->generatorConfiguration->bytesPerSample =
        snd_pcm_format_width (mediaType_.format) / 8;
      inherited::configuration_->generatorConfiguration->numberOfChannels =
        mediaType_.channels;
      inherited::configuration_->generatorConfiguration->isFloatFormat =
        (snd_pcm_format_float (mediaType_.format) == 1);
      inherited::configuration_->generatorConfiguration->isLittleEndianFormat =
        (snd_pcm_format_little_endian (mediaType_.format) == 1);
      inherited::configuration_->generatorConfiguration->isSignedFormat =
        (snd_pcm_format_signed (mediaType_.format) == 1);
#endif // ACE_WIN32 || ACE_WIN64

      // determine interval size from buffer size
      // *IMPORTANT NOTE*: larger buffers are more efficient, but introduce more
      //                   latency
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
      bufferSize_ =
        static_cast<unsigned int> (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
      if (bufferSize_ % frameSize_)
        bufferSize_ += frameSize_ - (bufferSize_ % frameSize_);
      ACE_ASSERT ((bufferSize_ % frameSize_) == 0);
      buffer_time_us =
#if defined (ACE_WIN32) || defined (ACE_WIN64)
        static_cast<suseconds_t> ((bufferSize_ * 1000000.0) / static_cast<double> (frameSize_ * waveformatex_p->nSamplesPerSec));
#else
        static_cast<suseconds_t> ((bufferSize_ * 1000000.0) / static_cast<double> (frameSize_ * mediaType_.rate));
#endif // ACE_WIN32 || ACE_WIN64

      // initialize noise generator
      if (!inherited::configuration_->generatorConfiguration->isFloatFormat)
      {
        if (inherited::configuration_->generatorConfiguration->isSignedFormat)
          switch (inherited::configuration_->generatorConfiguration->bytesPerSample)
          {
            case 1:
            {
              SIGNED_INTEGER_DISTRIBUTION_T::param_type parameters_s (std::numeric_limits<int8_t>::min (),
                                                                      std::numeric_limits<int8_t>::max ());
              signedIntegerDistribution_.param (parameters_s);
              break;
            }
            case 2:
            {
              SIGNED_INTEGER_DISTRIBUTION_T::param_type parameters_s (std::numeric_limits<int16_t>::min (),
                                                                      std::numeric_limits<int16_t>::max ());
              signedIntegerDistribution_.param (parameters_s);
              break;
            }
            case 4:
            {
              SIGNED_INTEGER_DISTRIBUTION_T::param_type parameters_s (std::numeric_limits<int32_t>::min (),
                                                                      std::numeric_limits<int32_t>::max ());
              signedIntegerDistribution_.param (parameters_s);
              break;
            }
            case 8:
            {
              SIGNED_INTEGER_DISTRIBUTION_T::param_type parameters_s (std::numeric_limits<int64_t>::min (),
                                                                      std::numeric_limits<int64_t>::max ());
              signedIntegerDistribution_.param (parameters_s);
              break;
            }
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: invalid/unknown sample size (was: %u), aborting\n"),
                          inherited::mod_->name (),
                          inherited::configuration_->generatorConfiguration->bytesPerSample));
              goto error;
            }
          } // end SWITCH
        else
          switch (inherited::configuration_->generatorConfiguration->bytesPerSample)
          {
            case 1:
            {
              INTEGER_DISTRIBUTION_T::param_type parameters_s (0,
                                                               std::numeric_limits<uint8_t>::max ());
              integerDistribution_.param (parameters_s);
              break;
            }
            case 2:
            {
              INTEGER_DISTRIBUTION_T::param_type parameters_s (0,
                                                               std::numeric_limits<uint16_t>::max ());
              integerDistribution_.param (parameters_s);
              break;
            }
            case 4:
            {
              INTEGER_DISTRIBUTION_T::param_type parameters_s (0,
                                                               std::numeric_limits<uint32_t>::max ());
              integerDistribution_.param (parameters_s);
              break;
            }
            case 8:
            {
              INTEGER_DISTRIBUTION_T::param_type parameters_s (0,
                                                               std::numeric_limits<uint64_t>::max ());
              integerDistribution_.param (parameters_s);
              break;
            }
            default:
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: invalid/unknown sample size (was: %u), aborting\n"),
                          inherited::mod_->name (),
                          inherited::configuration_->generatorConfiguration->bytesPerSample));
              goto error;
            }
          } // end SWITCH
      } // end IF
      else
        switch (inherited::configuration_->generatorConfiguration->bytesPerSample)
        {
          case 4:
          { ACE_ASSERT (ACE_SIZEOF_FLOAT == 4);
            REAL_DISTRIBUTION_T::param_type parameters_s (-1.0f,
                                                          1.0f);
            realDistribution_.param (parameters_s);
            break;
          }
          case 8:
          { ACE_ASSERT (ACE_SIZEOF_DOUBLE == 8);
            REAL_DISTRIBUTION_T::param_type parameters_s (-1.0,
                                                          1.0);
            realDistribution_.param (parameters_s);
            break;
          }
          case 16:
          { ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
            REAL_DISTRIBUTION_T::param_type parameters_s (-1.0l,
                                                          1.0l);
            realDistribution_.param (parameters_s);
            break;
          }
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown sample size (was: %u), aborting\n"),
                        inherited::mod_->name (),
                        inherited::configuration_->generatorConfiguration->bytesPerSample));
            goto error;
          }
        } // end SWITCH
     
      // *NOTE*: see also: https://sampo.kapsi.fi/PinkNoise/PinkNoise.java
      REAL_DISTRIBUTION_T::param_type parameters_s (0.0l, 1.0l);
      realDistribution_2.param (parameters_s);

      ACE_NEW_NORETURN (multipliers_,
                        long double[inherited::configuration_->generatorConfiguration->poles]);
      ACE_ASSERT (multipliers_);
      for (int i = 0;
           i < inherited::configuration_->generatorConfiguration->poles;
           i++)
      {
        a =
          (i - inherited::configuration_->generatorConfiguration->alpha / 2.0l) * a / (i + 1);
        multipliers_[i] = a;
      } // end FOR
      ACE_NEW_NORETURN (history_,
                        long double[inherited::configuration_->generatorConfiguration->poles]);
      ACE_ASSERT (history_);
      ACE_OS::memset (history_, 0, sizeof (long double) * inherited::configuration_->generatorConfiguration->poles);
      for (int i = 0;
           i < inherited::configuration_->generatorConfiguration->poles;
           i++)
      {
        long double x = Common_Tools::getRandomNumber (realDistribution_2) - 0.5l;
        for (int j = 0;
             j < inherited::configuration_->generatorConfiguration->poles;
             j++)
          x -= multipliers_[j] * history_[j];
        history_[i] = x;
      } // end FOR

#if defined (LIBNOISE_SUPPORT)
      noiseModule_.SetFrequency (inherited::configuration_->generatorConfiguration->perlin_frequency);
      noiseModule_.SetOctaveCount (inherited::configuration_->generatorConfiguration->octaves);
      noiseModule_.SetPersistence (inherited::configuration_->generatorConfiguration->persistence);
      noiseModule_.SetLacunarity (inherited::configuration_->generatorConfiguration->lacunarity);
      noiseModule_.SetNoiseQuality ((noise::NoiseQuality)inherited::configuration_->generatorConfiguration->quality);
#endif // LIBNOISE_SUPPORT

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      DWORD task_index_i = 0;
//      ACE_ASSERT (!task_);
//      task_ =
//        AvSetMmThreadCharacteristics (TEXT (STREAM_LIB_WASAPI_CAPTURE_DEFAULT_TASKNAME),
//                                      &task_index_i);
//      if (!task_)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to AvSetMmThreadCharacteristics(): \"%m\", aborting\n"),
//                    inherited::mod_->name ()));
//        goto error;
//      } // end IF
//#endif // ACE_WIN32 || ACE_WIN64

      // start sample generator timer
      interval.set (0, buffer_time_us);
      timer_id =
        itimer_manager_p->schedule_timer (&handler_,       // event handler handle
                                          NULL,            // asynchronous completion token
                                          COMMON_TIME_NOW, // first wakeup time
                                          interval);       // interval
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

      if (multipliers_)
      {
        delete [] multipliers_; multipliers_ = NULL;
      } // end IF
      if (history_)
      {
        delete [] history_; history_ = NULL;
      } // end IF

      long timer_id = handler_.get_2 ();
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

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//      if (likely (task_))
//      {
//        AvRevertMmThreadCharacteristics (task_);
//        task_ = NULL;
//      } // end IF
//#endif // ACE_WIN32 || ACE_WIN64

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
          typename StatisticContainerType,
          typename SessionManagerType,
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
                          StatisticContainerType,
                          SessionManagerType,
                          TimerManagerType,
                          MediaType>::handle (const void* act_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dec_Noise_Source_T::handle"));

  ACE_UNUSED_ARG (act_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::allocator_);
  ACE_ASSERT (inherited::configuration_->generatorConfiguration);

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;

  // step1: allocate buffer
  try {
    message_block_p =
      static_cast<ACE_Message_Block*> (inherited::allocator_->malloc (bufferSize_));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                inherited::mod_->name (),
                bufferSize_));
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
  switch (inherited::configuration_->generatorConfiguration->type)
  {
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SAWTOOTH:
    {
      Stream_Module_Decoder_Noise_Tools::sawtooth (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                   inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                   inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                   inherited::configuration_->generatorConfiguration->isFloatFormat,
                                                   inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                   inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                   reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                   bufferSize_ / frameSize_,
                                                   inherited::configuration_->generatorConfiguration->amplitude,
                                                   inherited::configuration_->generatorConfiguration->waveform_frequency, 
                                                   phase_);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SINE:
    {
      Stream_Module_Decoder_Noise_Tools::sinus (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                inherited::configuration_->generatorConfiguration->isFloatFormat,
                                                inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                bufferSize_ / frameSize_,
                                                inherited::configuration_->generatorConfiguration->amplitude,
                                                inherited::configuration_->generatorConfiguration->waveform_frequency, 
                                                phase_);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_SQUARE:
    {
      Stream_Module_Decoder_Noise_Tools::square (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                 inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                 inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                 inherited::configuration_->generatorConfiguration->isFloatFormat,
                                                 inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                 inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                 reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                 bufferSize_ / frameSize_,
                                                 inherited::configuration_->generatorConfiguration->amplitude,
                                                 inherited::configuration_->generatorConfiguration->waveform_frequency, 
                                                 phase_);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_TRIANGLE:
    {
      Stream_Module_Decoder_Noise_Tools::triangle (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                   inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                   inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                   inherited::configuration_->generatorConfiguration->isFloatFormat,
                                                   inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                   inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                   reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                   bufferSize_ / frameSize_,
                                                   inherited::configuration_->generatorConfiguration->amplitude,
                                                   inherited::configuration_->generatorConfiguration->waveform_frequency, 
                                                   phase_);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_NOISE:
    {
      if (inherited::configuration_->generatorConfiguration->isFloatFormat)
        Stream_Module_Decoder_Noise_Tools::noise (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                  inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                  inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                  inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                  inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                  reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                  bufferSize_ / frameSize_,
                                                  inherited::configuration_->generatorConfiguration->amplitude,
                                                  realDistribution_);
      else if (inherited::configuration_->generatorConfiguration->isSignedFormat)
        Stream_Module_Decoder_Noise_Tools::noise (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                  inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                  inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                  inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                  inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                  reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                  bufferSize_ / frameSize_,
                                                  inherited::configuration_->generatorConfiguration->amplitude,
                                                  signedIntegerDistribution_);
      else
        Stream_Module_Decoder_Noise_Tools::noise (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                  inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                  inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                  inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                  inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                  reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                  bufferSize_ / frameSize_,
                                                  inherited::configuration_->generatorConfiguration->amplitude,
                                                  integerDistribution_);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PINK_NOISE:
    {
      Stream_Module_Decoder_Noise_Tools::pink_noise (inherited::configuration_->generatorConfiguration->samplesPerSecond,
                                                     inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                     inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                     inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                     inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                     inherited::configuration_->generatorConfiguration->isFloatFormat,
                                                     reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                     bufferSize_ / frameSize_,
                                                     inherited::configuration_->generatorConfiguration->amplitude,
                                                     inherited::configuration_->generatorConfiguration->poles,
                                                     realDistribution_2,
                                                     multipliers_,
                                                     history_);
      break;
    }
#if defined (LIBNOISE_SUPPORT)
    case STREAM_MEDIAFRAMEWORK_SOUNDGENERATOR_PERLIN_NOISE:
    {
      Stream_Module_Decoder_Noise_Tools::perlin_noise (noiseModule_,
                                                       inherited::configuration_->generatorConfiguration->bytesPerSample,
                                                       inherited::configuration_->generatorConfiguration->numberOfChannels,
                                                       inherited::configuration_->generatorConfiguration->isFloatFormat,
                                                       inherited::configuration_->generatorConfiguration->isSignedFormat,
                                                       inherited::configuration_->generatorConfiguration->isLittleEndianFormat,
                                                       reinterpret_cast<uint8_t*> (message_block_p->wr_ptr ()),
                                                       bufferSize_ / frameSize_,
                                                       inherited::configuration_->generatorConfiguration->amplitude,
                                                       inherited::configuration_->generatorConfiguration->step,
                                                       inherited::configuration_->generatorConfiguration->x, inherited::configuration_->generatorConfiguration->y, inherited::configuration_->generatorConfiguration->z);
      break;
    }
#endif // LIBNOISE_SUPPORT
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unkown noise type (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->generatorConfiguration->type));
      goto error;
    }
  } // end SWITCH
  message_block_p->wr_ptr (bufferSize_);

  // step3: push data downstream
  result = this->put (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::put(): \"%m\", aborting\n"),
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
          typename StatisticContainerType,
          typename SessionManagerType,
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
                          StatisticContainerType,
                          SessionManagerType,
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
