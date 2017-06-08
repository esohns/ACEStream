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

#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_misc_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ValueType,
          unsigned int Aggregation>
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::Stream_Module_StatisticAnalysis_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 (MODULE_MISC_ANALYSIS_DEFAULT_BUFFER_SIZE,
               MODULE_MISC_SPECTRUMANALYSIS_DEFAULT_SAMPLE_RATE)
 , amplitudeSum_ (0)
 , amplitudeSumSqr_ (0)
 , amplitudeVariance_ (0.0)
 , streak_ (0)
 , streakCount_ (0)
 , streakSum_ (0.0)
 , streakSumSqr_ (0.0)
 , streakVariance_ (0.0)
 , volume_ (0)
 , volumeSum_ (0.0)
 , volumeSumSqr_ (0.0)
 , volumeVariance_ (0.0)
 , eventDispatcher_ (NULL)
 , iterator_ (NULL)
 , sampleCount_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::Stream_Module_StatisticAnalysis_T"));

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
          typename ValueType,
          unsigned int Aggregation>
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::~Stream_Module_StatisticAnalysis_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::~Stream_Module_StatisticAnalysis_T"));

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
          typename ValueType,
          unsigned int Aggregation>
bool
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::initialize (const ConfigurationType& configuration_in,
                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::initialize"));

  if (inherited::isInitialized_)
  {
    // (re-)activate() the message queue
    // *NOTE*: as this is a 'passive' object, the queue needs to be explicitly
    //         (re-)activate()d (see below)
    inherited::msg_queue (NULL);

    amplitudeSum_ = 0;
    amplitudeSumSqr_ = 0;
    amplitudeVariance_ = 0.0;

    streak_ = 0;
    streakCount_ = 0;
    streakSum_ = 0.0;
    streakSumSqr_ = 0.0;
    streakVariance_ = 0.0;
    volume_ = 0;
    volumeSum_ = 0.0;
    volumeSumSqr_ = 0.0;
    volumeVariance_ = 0.0;

    eventDispatcher_ = NULL;
    iterator_.buffer_ = NULL;
  } // end IF

  // *TODO*: remove type inference
  eventDispatcher_ = configuration_in.dispatch;

  if (!inherited::initialize (configuration_in,
                              allocator_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBaseSynch_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _AMMediaType* media_type_p = getFormat (configuration_in.format);
  if (!media_type_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve media type, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);

  ACE_ASSERT (media_type_p->pbFormat);
  struct tWAVEFORMATEX* waveformatex_p =
    reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
  ACE_ASSERT (waveformatex_p);
#endif

  // *TODO*: remove type inferences
  // sanity check(s)
  ACE_ASSERT (configuration_in.format);

  bool result_2 = false;
  unsigned int sample_rate;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  sample_rate = waveformatex_p->nSamplesPerSec;
#else
  sample_rate = configuration_in.format->rate;
#endif
  result_2 =
    inherited2::Initialize (configuration_in.spectrumAnalyzerResolution,
                            sample_rate);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (media_type_p)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
#endif

  return result_2;
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
          typename ValueType,
          unsigned int Aggregation>
void
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::handleDataMessage (DataMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  ACE_ASSERT (message_inout->length () % iterator_.sampleSize_ == 0);
  ACE_ASSERT (message_inout->length () % iterator_.subSampleSize_ == 0);

  unsigned int number_of_samples =
    message_inout->length () / iterator_.sampleSize_;
  unsigned int samples_to_write = 0;
  unsigned int offset = 0;
  unsigned int tail_slot = 0;

  do
  {
    samples_to_write =
      (number_of_samples > inherited2::slots_ ? inherited2::slots_
                                              : number_of_samples);
    iterator_.buffer_ = message_inout->rd_ptr () + offset;
    for (unsigned int i = 0; i < Aggregation; ++i)
    {
      samples_to_write =
          (number_of_samples > inherited2::slots_ ? inherited2::slots_
                                                  : number_of_samples);

      // make space for inbound samples at the end of the buffer, shifting
      // previous samples towards the beginning
      tail_slot = inherited2::slots_ - samples_to_write;
      ACE_OS::memmove (&(inherited2::buffer_[i][0]),
                       &(inherited2::buffer_[i][samples_to_write]),
                       tail_slot * sizeof (ValueType));

      // copy the sample data to the tail end of the buffer, transform to
      // ValueType
      for (unsigned int j = 0; j < samples_to_write; ++j)
        inherited2::buffer_[i][tail_slot + j] = iterator_.get (j, i);
      offset += (iterator_.subSampleSize_ * samples_to_write);

      // analyze sample data
      Process (tail_slot, tail_slot + samples_to_write - 1);
    } // end FOR

    number_of_samples -= samples_to_write;
    if (number_of_samples == 0) break; // done
  } while (true);
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
          typename ValueType,
          unsigned int Aggregation>
void
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

//  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      bool result_2 = false;
//      bool shutdown = false;

      unsigned int sample_size = 0;
      unsigned int sub_sample_size = 0;
      unsigned int sample_rate;
      int sample_byte_order = ACE_BYTE_ORDER;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType* media_type_p = NULL;
      media_type_p = getFormat (session_data_r.format);
      if (!media_type_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to retrieve media type, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (media_type_p->formattype == FORMAT_WaveFormatEx);
      ACE_ASSERT (media_type_p->pbFormat);

      // *NOTE*: apparently, all Win32 sound data is signed 16 bits
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
      ACE_ASSERT (waveformatex_p);
      sample_size = waveformatex_p->nBlockAlign;
      sub_sample_size = (sample_size * 8) /
                         waveformatex_p->wBitsPerSample;
      // *NOTE*: apparently, all Win32 sound data is little endian only
      sample_byte_order = ACE_LITTLE_ENDIAN;

//      channels = waveformatex_p->nChannels;
      sample_rate = waveformatex_p->nSamplesPerSec;

      Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
#else
      sample_size =
        ((snd_pcm_format_width (session_data_r.format.format) / 8) *
          session_data_r.format.channels);
      sub_sample_size = sample_size / session_data_r.format.channels;
      sample_byte_order =
          ((snd_pcm_format_little_endian (session_data_r.format.format) == 1) ? ACE_LITTLE_ENDIAN
                                                                              : -1);

//      channels = session_data_r.format.channels;
      sample_rate = session_data_r.format.rate;
#endif
      result_2 = iterator_.initialize (sample_size,
                                       sub_sample_size,
                                       true,
                                       sample_byte_order);
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize sample iterator, aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      result_2 =
        inherited2::Initialize (inherited::configuration_->spectrumAnalyzerResolution,
                                sample_rate);
      if (!result_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_Math_Sample_T::initialize(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

//        inherited::start ();
//        shutdown = true;

      break;

error:
//      if (shutdown)
//        inherited::stop (false); // wait ?

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      session_data_r.currentStatistic.amplitudeAverage =
          (sampleCount_ ? amplitudeSum_ / sampleCount_ : 0.0);
      session_data_r.currentStatistic.amplitudeVariance = amplitudeVariance_;
      session_data_r.currentStatistic.streakAverage =
          (sampleCount_ ? streakSum_ / sampleCount_ : 0.0);
      session_data_r.currentStatistic.streakCount = streakCount_;
      session_data_r.currentStatistic.streakVariance = streakVariance_;
      session_data_r.currentStatistic.volumeAverage =
          (sampleCount_ ? volumeSum_ / sampleCount_ : 0.0);
      session_data_r.currentStatistic.volumeVariance = volumeVariance_;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *IMPORTANT NOTE*: at this stage no new data should be arriving (i.e.
      //                   the DirectShow graph should have stopped
      //                   --> join with the renderer thread
      if (inherited::thr_count_ > 0)
      {
        inherited::stop (true); // wait ?
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: joined renderer thread\n"),
                    inherited::mod_->name ()));
      } // end IF

      break;
    }
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
//Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
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
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::reset"));

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
//Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
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
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::svc"));
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
//    if (result_2 == -1)
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ValueType,
          unsigned int Aggregation>
AM_MEDIA_TYPE*
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*format_in,
                                                             result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                inherited::mod_->name ()));
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
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ValueType,
          unsigned int Aggregation>
AM_MEDIA_TYPE*
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T::getFormat_impl"));

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
                ACE_TEXT ("%s: failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return NULL;
  } // end IF
  ACE_ASSERT (result_p);

  return result_p;
}
#endif

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename ValueType,
          unsigned int Aggregation>
void
Stream_Module_StatisticAnalysis_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  StatisticContainerType,
                                  SessionDataType,
                                  SessionDataContainerType,
                                  ValueType,
                                  Aggregation>::Process (unsigned int startIndex_in,
                                                         unsigned int endIndex_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_StatisticAnalysis_T::Process"));

  // sanity check(s)
  ACE_ASSERT (endIndex_in < inherited2::slots_);

//  int result = -1;

//  if (lock_)
//  {
//    result = lock_->acquire ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
//    else
//      release_lock = true;
//  } // end IF

  ValueType difference = 0;
  for (unsigned int i = 0; i < Aggregation; ++i)
    for (unsigned int j = 0; j < (endIndex_in - startIndex_in + 1); ++j, ++sampleCount_)
    {
      // step1: 'attack' detection
      static bool in_peak = false;
      static bool was_in_peak = false;

      amplitudeSum_ += inherited2::buffer_[i][startIndex_in + j];
      amplitudeSumSqr_ +=
          (inherited2::buffer_[i][startIndex_in + j] * inherited2::buffer_[i][startIndex_in + j]);
      amplitudeVariance_ =
          ((sampleCount_ > 1) ? (amplitudeSumSqr_ - ((amplitudeSum_ * amplitudeSum_) / (double)sampleCount_)) / (double)(sampleCount_ - 1)
                              : 0.0);
      difference =
          (sampleCount_ ? inherited2::buffer_[i][startIndex_in + j] - (amplitudeSum_ / (double)sampleCount_)
                        : 0.0);

      was_in_peak = in_peak;
      in_peak =
          (abs (difference) > (MODULE_MISC_ANALYSIS_PEAK_DETECTION_DEVIATION_RANGE * sqrt (amplitudeVariance_)));
//      if (in_peak && !was_in_peak)
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("detected peak...\n")));

      // step2: 'sustain' detection
      static bool in_streak = false;
      static bool was_in_streak = false;
      static bool in_volume = false;
      static bool was_in_volume = false;
      if (difference <= 0)
      {
        if (in_streak)
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("detected streak end...\n")));
          ++streakCount_;
        } // end IF
        streak_ = 0;
        in_streak = false;

//        if (in_volume)
//          ACE_DEBUG ((LM_DEBUG,
//                      ACE_TEXT ("detected volume end...\n")));
        volume_ = 0;
        in_volume = false;

        goto continue_;
      } // end IF

      // step2a: 'sustain' detection (streak)
      ++streak_;
      streakSum_ += streak_;
      streakSumSqr_ += (streak_ * streak_);
      streakVariance_ =
          ((sampleCount_ > 1) ? (streakSumSqr_ - ((streakSum_ * streakSum_) / (double)sampleCount_)) / (double)(sampleCount_ - 1)
                              : 0.0);
      difference = (sampleCount_ ? streak_ - (streakSum_ / (double)sampleCount_)
                                 : 0.0);

      was_in_streak = in_streak;
      in_streak =
          (abs (difference) >= (MODULE_MISC_ANALYSIS_ACTIVITY_DETECTION_DEVIATION_RANGE * sqrt (streakVariance_)));
      if (in_streak)
      {
        if (!was_in_streak)
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("detected noise (streak)...\n")));
          //        goto continue_2;
        } // end IF

        goto continue_2;
      } // end IF
      if (was_in_streak)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("detected streak end...\n")));
        streak_ = 0;
        ++streakCount_;
        was_in_streak = false;

        volume_ = 0;
        in_volume = false;
        was_in_volume = false;
      } // end IF

      continue;

      // step2b: 'sustain' detection (volume)
      volume_ += inherited2::buffer_[i][startIndex_in + j];
      volumeSum_ += inherited2::buffer_[i][startIndex_in + j];
      volumeSumSqr_ +=
          (inherited2::buffer_[i][startIndex_in + j] * inherited2::buffer_[i][startIndex_in + j]);
      volumeVariance_ =
          ((sampleCount_ > 1) ? (volumeSumSqr_ - ((volumeSum_ * volumeSum_) / (double)sampleCount_)) / (double)(sampleCount_ - 1)
                              : 0.0);
      difference = (sampleCount_ ? volume_ - (volumeSum_ / (double)sampleCount_)
                                 : 0.0);

      was_in_volume = in_volume;
      in_volume =
          (abs (difference) > (MODULE_MISC_ANALYSIS_ACTIVITY_DETECTION_DEVIATION_RANGE * sqrt (volumeVariance_)));
      if (in_volume)
      {
        if (!was_in_volume)
        {
//          ACE_DEBUG ((LM_DEBUG,
//                      ACE_TEXT ("detected noise (volume)...\n")));
          //        goto continue_2;
        } // end IF

        goto continue_2;
      } // end IF
      if (was_in_volume)
      {
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("detected volume end...\n")));

        volume_ = 0;
        in_volume = false;
        was_in_volume = false;
      } // end IF

continue_2:
      if ((in_streak || in_volume) &&  // <-- 'activity' ?
          eventDispatcher_)
      {
        try {
          eventDispatcher_->dispatch (STREAM_MODULE_STATISTICANALYSIS_EVENT_ACTIVITY);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Common_IDispatch_T::dispatch(), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF

continue_:
      if ((in_peak && !was_in_peak) && // <-- 'peak' ?
          eventDispatcher_)
      {
        try {
          eventDispatcher_->dispatch (STREAM_MODULE_STATISTICANALYSIS_EVENT_PEAK);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Common_IDispatch_T::dispatch(), continuing\n"),
                      inherited::mod_->name ()));
        }
      } // end IF
    } // end FOR

//unlock:
//  if (release_lock)
//  {
//    ACE_ASSERT (lock_);
//    result = lock_->release ();
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n")));
//    release_lock = false;
//  } // end IF
}
