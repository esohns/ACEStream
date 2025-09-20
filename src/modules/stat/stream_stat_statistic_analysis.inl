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

#include <cmath>

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "stream_macros.h"

#include "stream_stat_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType,
          typename ValueType>
Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     MediaType,
                                     ValueType>::Stream_Statistic_StatisticAnalysis_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , amplitudeM_ (0.0)
 , amplitudeS_ (0.0)
 , streak_ (0)
 , streakCount_ (0)
 , streakM_ (0.0)
 , streakS_ (0.0)
 , volumeM_ (0.0)
 , volumeS_ (0.0)
 , eventDispatcher_ (NULL)
 , iterator_ (NULL)
 , frameCount_ (0)
 , sampleIsSigned_ (false)
 , signedSampleModifier_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::Stream_Statistic_StatisticAnalysis_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType,
          typename ValueType>
bool
Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     MediaType,
                                     ValueType>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    // (re-)activate() the message queue
    // *NOTE*: as this is a 'passive' object, the queue needs to be explicitly
    //         (re-)activate()d (see below)
    inherited::msg_queue (NULL);

    amplitudeM_ = 0.0;
    amplitudeS_ = 0.0;

    streak_ = 0;
    streakCount_ = 0;
    streakM_ = 0.0;
    streakS_ = 0.0;
    volumeM_ = 0.0;
    volumeS_ = 0.0;

    eventDispatcher_ = NULL;
    iterator_.buffer_ = NULL;
    frameCount_ = 0;
    sampleIsSigned_ = false;
    signedSampleModifier_ = 0.0;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType,
          typename ValueType>
void
Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     MediaType,
                                     ValueType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  ACE_ASSERT (iterator_.frameSize_);
  //ACE_ASSERT (message_inout->length () % iterator_.frameSize_ == 0);
  ACE_ASSERT (iterator_.sampleSize_);
  //ACE_ASSERT (message_inout->length () % iterator_.sampleSize_ == 0);

  unsigned int number_of_frames =
    static_cast<unsigned int> (message_inout->length () / iterator_.frameSize_);
  unsigned int frames_to_write;
  unsigned int offset = 0;
  unsigned int tail_slot;

  while (number_of_frames)
  {
    iterator_.buffer_ =
      reinterpret_cast<uint8_t*> (message_inout->rd_ptr ()) + offset;
    for (unsigned int i = 0; i < inherited3::channels_; ++i)
    {
      frames_to_write = std::min (number_of_frames, inherited3::slots_);

      // make space for inbound samples at the end of the buffer, shifting
      // previous samples towards the beginning
      tail_slot = inherited3::slots_ - frames_to_write;
      ACE_OS::memmove (&(inherited3::buffer_[i][0]),
                       &(inherited3::buffer_[i][frames_to_write]),
                       tail_slot * sizeof (ValueType));

      // copy the sample data to the tail end of the buffer, transform to
      // ValueType
      for (unsigned int j = 0; j < frames_to_write; ++j)
        inherited3::buffer_[i][tail_slot + j] = iterator_.get (j, i);

      // analyze sample data
      Process (i, tail_slot, tail_slot + frames_to_write - 1);

      // accumulate offset
      offset += (iterator_.sampleSize_ * frames_to_write);
    } // end FOR

    number_of_frames -= frames_to_write;
  } // end WHILE
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType,
          typename ValueType>
void
Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     MediaType,
                                     ValueType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      bool result_2 = false;

      unsigned int num_channels = 0;
      unsigned int frame_size = 0;
      unsigned int sample_size = 0;
      unsigned int sample_rate;
      int sample_byte_order = ACE_BYTE_ORDER;
      bool is_floating_point_b = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      // sanity check(s)
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      ACE_ASSERT (media_type_s.pbFormat);

      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      num_channels = waveformatex_p->nChannels;
      sample_size = waveformatex_p->wBitsPerSample / 8;
      //frame_size = waveformatex_p->nBlockAlign;
      frame_size = (waveformatex_p->nChannels * sample_size);
      sample_rate = waveformatex_p->nSamplesPerSec;
      // *NOTE*: apparently, all Win32 sound data is little endian only
      sample_byte_order = 0x0123; // ACE_LITTLE_ENDIAN
      is_floating_point_b =
        (waveformatex_p->wFormatTag == WAVE_FORMAT_IEEE_FLOAT);

      // *NOTE*: "...If the audio contains 8 bits per sample, the audio samples
      //         are unsigned values. (Each audio sample has the range 0–255.)
      //         If the audio contains 16 bits per sample or higher, the audio
      //         samples are signed values. ..."
      sampleIsSigned_ = !(sample_size == 1);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      num_channels = media_type_s.channels;
      sample_size = snd_pcm_format_width (media_type_s.format) / 8;
      frame_size = media_type_s.channels * sample_size;
      sample_rate = media_type_s.rate;
      sample_byte_order =
        ((snd_pcm_format_little_endian (media_type_s.format) == 1) ? 0x0123 // ACE_LITTLE_ENDIAN
                                                                   : (snd_pcm_format_big_endian (media_type_s.format) == 1) ? 0x3210 // ACE_BIG_ENDIAN
                                                                                                                            : -1); // N/A
      is_floating_point_b = (snd_pcm_format_linear (media_type_s.format) == 0);

      sampleIsSigned_ =
        (snd_pcm_format_signed (media_type_s.format) == 1) || is_floating_point_b;
#endif // ACE_WIN32 || ACE_WIN64
      result_2 = iterator_.initialize (frame_size,
                                       sample_size,
                                       is_floating_point_b,
                                       sampleIsSigned_,
                                       sample_byte_order);
      if (unlikely (!result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize sample iterator, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      signedSampleModifier_ =
        is_floating_point_b ? static_cast<ValueType> (1.0)
                            : static_cast<ValueType> ((1ULL << ((8 * sample_size) - 1)) - 1);

      result_2 =
        inherited3::Initialize (num_channels,
                                inherited::configuration_->bufferSize,
                                sample_rate);
      if (unlikely (!result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_Math_Sample_T::initialize(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      session_data_r.statistic.amplitudeAverage = amplitudeM_;
      session_data_r.statistic.amplitudeVariance =
        (frameCount_ ? amplitudeS_ / (double)(frameCount_ - 1) : 0.0);
      session_data_r.statistic.streakAverage = streakM_;
      session_data_r.statistic.streakCount = streakCount_;
      session_data_r.statistic.streakVariance =
        (frameCount_ ? streakS_ / (double)(frameCount_ - 1) : 0.0);
      session_data_r.statistic.volumeAverage = volumeM_;
      session_data_r.statistic.volumeVariance =
        (frameCount_ ? volumeS_ / (double)(frameCount_ - 1) : 0.0);
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    default:
      break;
  } // end SWITCH
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename ValueType,
//          unsigned int Aggregation>
//void
//Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
//                                  TimePolicyType,
//                                  ConfigurationType,
//                                  ControlMessageType,
//                                  DataMessageType,
//                                  SessionMessageType,
//                                  SessionDataType,
//                                  SessionDataContainerType,
//                                  ValueType,
//                                  Aggregation>::reset ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::reset"));

//  // trigger a render update
//  // *NOTE*: (as long as it is single thread-based,) rendering a frame creates
//  //         too much workload for the timer dispatch context and delays the
//  //         dispatch of (relatively more important other) scheduled tasks
//  //         --> avoid 'laggy' applications
//  // *TODO*: depending on the platform (and the timer dispatch 'mode'), this may
//  //         be unnecessary (i.e. if the timer mechanism is signal-handler
//  //         based (, or the timer dispatch uses a thread pool itself))
//  inherited::control (ACE_Message_Block::MB_EVENT);
//}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename ValueType,
//          unsigned int Aggregation>
//int
//Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
//                                  TimePolicyType,
//                                  ConfigurationType,
//                                  ControlMessageType,
//                                  DataMessageType,
//                                  SessionMessageType,
//                                  SessionDataType,
//                                  SessionDataContainerType,
//                                  ValueType,
//                                  Aggregation>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::svc"));
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::mod_);
//  //ACE_ASSERT (inherited::sessionData_);
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: analyzer thread (ID: %t) starting...\n"),
//              inherited::mod_->name ()));
//
//  int error = 0;
//  ACE_Message_Block* message_block_p = NULL;
//  int result = 0;
//  int result_2 = -1;
//  //const SessionDataType& session_data_r = inherited::sessionData_->get ();
//  //  unsigned int queued, done = 0;
//
//  // process update events
//  do
//  {
//    message_block_p = NULL;
//    result_2 = inherited::getq (message_block_p, NULL);
//    if (unlikely (result_2 == -1))
//    {
//      error = ACE_OS::last_error ();
//      if (error != EWOULDBLOCK) // Win32: 10035
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
//                    inherited::mod_->name ()));
//      break;
//    } // end IF
//    ACE_ASSERT (message_block_p);
//
//    switch (message_block_p->msg_type ())
//    {
//      case ACE_Message_Block::MB_STOP:
//      {
//        // clean up
//        message_block_p->release ();
//        message_block_p = NULL;
//
//        goto done;
//      }
//      default:
//      {
//        // clean up
//        message_block_p->release ();
//        message_block_p = NULL;
//
//        update ();
//
//        break;
//      }
//    } // end SWITCH
//  } while (true);
//  result = -1;
//
//done:
//  return result;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType,
          typename ValueType>
void
Stream_Statistic_StatisticAnalysis_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     StatisticContainerType,
                                     SessionDataType,
                                     SessionDataContainerType,
                                     MediaType,
                                     ValueType>::Process (unsigned int channel_in,
                                                          unsigned int startIndex_in,
                                                          unsigned int endIndex_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Statistic_StatisticAnalysis_T::Process"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (endIndex_in < inherited3::slots_);

  static bool in_peak = false;
  static bool was_in_peak = false;
  static bool in_streak = false;
  static bool was_in_streak = false;
  static bool in_volume = false;
  static bool was_in_volume = false;

  ValueType abs_value;
  double abs_value_d, frame_count_d, difference, old_mean, std_deviation, streak_d;

  for (unsigned int j = 0; j < (endIndex_in - startIndex_in + 1); ++j, ++frameCount_)
  {
    abs_value =
      (sampleIsSigned_ ? inherited3::buffer_[channel_in][startIndex_in + j] + signedSampleModifier_
                       : inherited3::buffer_[channel_in][startIndex_in + j]);
    abs_value_d = static_cast<double> (abs_value);
    frame_count_d = static_cast<double> (frameCount_);

    // step1: 'attack' detection
    old_mean = (frameCount_ ? amplitudeM_ : abs_value_d);
    amplitudeM_ =
      (frameCount_ ? old_mean + ((abs_value_d - old_mean) / frame_count_d) : abs_value_d);
    difference = abs_value_d - old_mean;
    amplitudeS_ =
      amplitudeS_ + ((abs_value_d - old_mean) * (abs_value_d - amplitudeM_));
    std_deviation = (frameCount_ ? std::sqrt (amplitudeS_ / frame_count_d) : 0.0);

    was_in_peak = in_peak;
    in_peak =
      (std::abs (difference) > (MODULE_STAT_ANALYSIS_PEAK_DETECTION_DEVIATION_RANGE * std_deviation));

    // step2a: 'sustain' detection (streak)
    if (difference <= 0.0)
    { // --> amplitude drop
      streak_ = 0;
      streak_d = 0.0;
      in_streak = false;

      in_volume = false;
      volumeM_ = 0.0;
      volumeS_ = 0.0;

      goto continue_; // --> streak-end
    } // end IF

    ++streak_;
    streak_d = static_cast<double> (streak_);

    old_mean = (frameCount_ ? streakM_ : abs_value_d);
    streakM_ =
      (frameCount_ ? old_mean + ((streak_d - old_mean) / frame_count_d) : streak_d);
    difference = (streak_d - old_mean);
    streakS_ = streakS_ + ((streak_d - old_mean) * (streak_d - streakM_));
    std_deviation = (frameCount_ ? std::sqrt (streakS_ / frame_count_d) : 0.0);
    was_in_streak = in_streak;
    in_streak =
      (std::abs (difference) >= (MODULE_STAT_ANALYSIS_ACTIVITY_DETECTION_DEVIATION_RANGE * std_deviation));

    if (unlikely (in_streak))
    {
      if (!was_in_streak)
        ++streakCount_;
      goto continue_2;
    } // end IF
    // --> !in_streak
    if (unlikely (was_in_streak))
    {
      streak_ = 0;
      streak_d = 0.0;

      in_volume = false;
      volumeM_ = 0.0;
      volumeS_ = 0.0;
    } // end IF

    // step2b: 'sustain' detection (volume)
    old_mean = (frameCount_ ? volumeM_ : abs_value_d);
    volumeM_ =
      (frameCount_ ? old_mean + ((abs_value_d - old_mean) / frame_count_d) : abs_value_d);
    difference = abs_value_d - old_mean;
    volumeS_ =
      volumeS_ + ((abs_value_d - old_mean) * (abs_value_d - volumeM_));
    std_deviation = (frameCount_ ? std::sqrt (volumeS_ / frame_count_d) : 0.0);
    was_in_volume = in_volume;
    ACE_UNUSED_ARG (was_in_volume);
    in_volume =
      (std::abs (difference) > (MODULE_STAT_ANALYSIS_ACTIVITY_DETECTION_DEVIATION_RANGE * std_deviation));

continue_2:
    if (unlikely (inherited::configuration_->dispatch &&
                  ((in_streak && !was_in_streak) || in_volume)))  // <-- 'activity' ?
    {
      try {
        inherited::configuration_->dispatch->dispatch (STREAM_STATISTIC_ANALYSIS_EVENT_ACTIVITY);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Common_IDispatch_T::dispatch(), continuing\n"),
                    inherited::mod_->name ()));
      }
    } // end IF

continue_:
    if (unlikely (inherited::configuration_->dispatch &&
                  (in_peak && !was_in_peak))) // <-- 'peak' ?
    {
      try {
        inherited::configuration_->dispatch->dispatch (STREAM_STATISTIC_ANALYSIS_EVENT_PEAK);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Common_IDispatch_T::dispatch(), continuing\n"),
                    inherited::mod_->name ()));
      }
    } // end IF
  } // end FOR
}
