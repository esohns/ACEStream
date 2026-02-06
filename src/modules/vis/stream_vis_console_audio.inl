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
#include "strmif.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_timer_manager_common.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType,
          enum Stream_Visualization_SpectrumAnalyzer_2DMode AnalyzerMode>
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  AnalyzerMode>::Stream_Module_Vis_Console_Audio_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , channels_ (0)
 , frameSize_ (0)
 , normalizationFactor_ (0)
 , iterator_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::Stream_Module_Vis_Console_Audio_T"));

}



template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType,
          enum Stream_Visualization_SpectrumAnalyzer_2DMode AnalyzerMode>
bool
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  AnalyzerMode>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::initialize"));

  if (inherited::isInitialized_)
  {
    channels_ = 0;
    frameSize_ = 0;
    normalizationFactor_ = static_cast<ValueType> (0);
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
          typename MediaType,
          typename ValueType,
          enum Stream_Visualization_SpectrumAnalyzer_2DMode AnalyzerMode>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  AnalyzerMode>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  iterator_.buffer_ = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
  ACE_UINT64 frames_i = static_cast<ACE_UINT64> (message_inout->length ()) / frameSize_;

  float max_f;
  uint32_t level_i;
  for (ACE_UINT32 c = 0; c < channels_; ++c)
  {
    max_f = 0.0f;
    for (ACE_UINT64 n = 0; n < frames_i; ++n)
      max_f = fmaxf (max_f, fabsf (static_cast<float> (iterator_.get (n, c) * normalizationFactor_)));

    level_i = static_cast<uint32_t> (fminf (fmaxf (max_f * 30.0f, 0.0f), 39.0f));

    ACE_OS::printf (ACE_TEXT_ALWAYS_CHAR ("channel %d: |%*s%*s| peak:%.2f\n"),
                    c + 1,
                    level_i + 1, ACE_TEXT_ALWAYS_CHAR ("*"), 40 - level_i, ACE_TEXT_ALWAYS_CHAR (""),
                    max_f);
  } // end FOR

  /* move cursor back up */
  ACE_OS::printf (ACE_TEXT_ALWAYS_CHAR ("%c[%dA"),
                  0x1b,
                  channels_);

  ACE_OS::fflush (stdout);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType,
          enum Stream_Visualization_SpectrumAnalyzer_2DMode AnalyzerMode>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  AnalyzerMode>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      unsigned int bytes_per_sample_i;
      bool is_signed_sample_format_b, is_integral_sample_format_b;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      bytes_per_sample_i = audio_info_p->wBitsPerSample / 8;
      is_signed_sample_format_b = audio_info_p->wBitsPerSample > 8; // signed if > 8 bit/sample
      is_integral_sample_format_b =
        !Stream_MediaFramework_DirectSound_Tools::isFloat (*audio_info_p);
      channels_ = audio_info_p->nChannels;
      frameSize_ = audio_info_p->nChannels * bytes_per_sample_i;
      ACE_ASSERT (frameSize_ == audio_info_p->nBlockAlign);
      iterator_.initialize (frameSize_,
                            bytes_per_sample_i,
                            !is_integral_sample_format_b,
                            is_signed_sample_format_b,
                            0x0123); // all Win32 sound data is little endian
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      bytes_per_sample_i = snd_pcm_format_width (media_type_s.format) / 8;
      is_signed_sample_format_b =
        snd_pcm_format_signed (media_type_s.format) ? true : false;
      is_integral_sample_format_b =
        snd_pcm_format_float (media_type_s.format) ? false : true;
      channels_ = media_type_s.channels;
      frameSize_ = bytes_per_sample_i * media_type_s.channels;
      iterator_.initialize (frameSize_,
                            bytes_per_sample_i,
                            !is_integral_sample_format_b,
                            is_signed_sample_format_b,
                            snd_pcm_format_little_endian (media_type_s.format) ? 0x0123 : 0x3210);
#endif // ACE_WIN32 || ACE_WIN64

      // *NOTE*: integral types need normalization --> compute factor once
      if (is_integral_sample_format_b)
        switch (bytes_per_sample_i)
        {
          case 1:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int8_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint8_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          case 2:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int16_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint16_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          case 4:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int32_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint32_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          case 8:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int64_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint64_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown sample size (was: %u), aborting\n"),
                        inherited::mod_->name (),
                        bytes_per_sample_i));
            goto error;
          }
        } // end SWITCH
      else
        normalizationFactor_ = 1;

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
      break;
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType,
          enum Stream_Visualization_SpectrumAnalyzer_2DMode AnalyzerMode>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  AnalyzerMode>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::Stream_Module_Vis_Console_Audio_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , inherited4 (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_CHANNELS,
               STREAM_VIS_SPECTRUMANALYZER_DEFAULT_NUMBER_OF_BINS,
               STREAM_VIS_SPECTRUMANALYZER_DEFAULT_SAMPLE_RATE)
 , channels_ (0)
 , frameSize_ (0)
 , handler_ (this,
             false)
 , iterator_ (NULL)
 , normalizationFactor_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::Stream_Module_Vis_Console_Audio_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::~Stream_Module_Vis_Console_Audio_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::~Stream_Module_Vis_Console_Audio_T"));

  long timer_id = handler_.get_2 ();
  if (unlikely (timer_id != -1))
  {
    typename Common_Timer_Manager_t::INTERFACE_T* itimer_manager_p =
      ((inherited::configuration_ && inherited::configuration_->timerManager) ? inherited::configuration_->timerManager
                                                                              : COMMON_TIMERMANAGER_SINGLETON::instance ());
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
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
bool
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::initialize (const ConfigurationType& configuration_in,
                                                                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::initialize"));

  if (inherited::isInitialized_)
  {
    channels_ = 0;
    frameSize_ = 0;

    long timer_id = handler_.get_2 ();
    if (unlikely (timer_id != -1))
    {
      typename Common_Timer_Manager_t::INTERFACE_T* itimer_manager_p =
        ((inherited::configuration_ && inherited::configuration_->timerManager) ? inherited::configuration_->timerManager
                                                                                : COMMON_TIMERMANAGER_SINGLETON::instance ());
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

    normalizationFactor_ = static_cast<ValueType> (0);
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
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::handleDataMessage (DataMessageType*& message_inout,
                                                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  unsigned int number_of_samples;
  unsigned int samples_to_write;
  unsigned int offset = 0;
  unsigned int tail_slot;
  ACE_Message_Block* message_block_p = message_inout;

next:
  number_of_samples =
    static_cast<unsigned int> (message_block_p->length ()) / frameSize_;

  do
  {
    samples_to_write = std::min (inherited4::slots_, number_of_samples);
//    bufferedSamples_ += samples_to_write;
    iterator_.buffer_ =
      reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ()) + offset;
    for (unsigned int i = 0; i < inherited4::channels_; ++i)
    {
      // step1a: copy the inbound sample data into the buffer array
      // *TODO*: in principle, this step can be avoided by keeping track of the
      //         'current' slot (see also: Common_Math_FFT::CopyIn())

      // make space for inbound samples at the end of the buffer,
      // shifting previous samples towards the beginning
      tail_slot = inherited4::slots_ - samples_to_write;
      ACE_OS::memmove (&(inherited4::buffer_[i][0]), &(inherited4::buffer_[i][samples_to_write]), tail_slot * sizeof (ValueType));

      // copy the sample data to the tail end of the buffer as ValueType
      for (unsigned int j = 0; j < samples_to_write; ++j)
        inherited4::buffer_[i][tail_slot + j] = iterator_.get (j, i) * normalizationFactor_;

      // step1b: apply window function ?
      switch (inherited::configuration_->spectrumAnalyzerConfiguration->windowFunction)
      {
        case STREAM_VISUALIZATION_WINDOWFUNCTION_NONE:
          break;
        case STREAM_VISUALIZATION_WINDOWFUNCTION_BLACKMAN:
        { // *NOTE*: alpha = 0.16
          for (unsigned int j = 0; j < samples_to_write; ++j)
            inherited4::buffer_[i][tail_slot + j] *= (static_cast<ValueType> (0.42) - static_cast<ValueType> (0.5) * std::cos ((static_cast<ValueType> (2.0 * M_PI) * j) / static_cast<ValueType> (samples_to_write - 1)) + static_cast<ValueType> (0.08) * std::cos ((static_cast<ValueType> (4.0 * M_PI) * j) / static_cast<ValueType> (samples_to_write - 1)));
          break;
        }
        case STREAM_VISUALIZATION_WINDOWFUNCTION_HAMMING:
        {
          for (unsigned int j = 0; j < samples_to_write; ++j)
            inherited4::buffer_[i][tail_slot + j] *= (static_cast<ValueType> (0.54) - static_cast<ValueType> (0.46) * std::cos ((static_cast<ValueType> (2.0 * M_PI) * j) / static_cast<ValueType> (samples_to_write - 1)));
          break;
        }
        case STREAM_VISUALIZATION_WINDOWFUNCTION_HANN:
        {
          for (unsigned int j = 0; j < samples_to_write; ++j)
            inherited4::buffer_[i][tail_slot + j] *= (static_cast<ValueType> (0.5) * (static_cast<ValueType> (1.0) - std::cos (static_cast<ValueType> (2.0 * M_PI) * (j / static_cast<ValueType> (samples_to_write - 1)))));
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown window function type (was: %d), continuing\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->spectrumAnalyzerConfiguration->windowFunction));
          break;
        }
      } // end SWITCH

      // step1c: process sample data ?
      //if (compute_fft_b)
      //{
        // initialize the FFT working set buffer(, transform to complex)
        inherited4::Setup (i);

//        if (bufferedSamples_ >= inherited4::slots_)
//        {
          // compute FFT
        inherited4::Compute (i);
//        } // end IF
      //} // end IF
    } // end FOR
//    if (bufferedSamples_ >= inherited4::slots_)
//      bufferedSamples_ -= inherited4::slots_;

    offset += (iterator_.dataSampleSize_ * samples_to_write);
    number_of_samples -= samples_to_write;
    if (likely (!number_of_samples))
    {
      message_block_p = message_block_p->cont ();
      if (unlikely (message_block_p))
      {
        offset = 0;
        goto next;
      } // end IF
    } // end IF
  } while (message_block_p);

  //if (compute_fft_b)
  //  inherited4::ComputeMaxValue ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  typename Common_Timer_Manager_t::INTERFACE_T* itimer_manager_p =
    ((inherited::configuration_ && inherited::configuration_->timerManager) ? inherited::configuration_->timerManager
                                                                            : COMMON_TIMERMANAGER_SINGLETON::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->spectrumAnalyzerConfiguration);
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      unsigned int sample_rate_i;
      unsigned int bytes_per_sample_i;
      bool is_signed_sample_format_b, is_integral_sample_format_b;
      suseconds_t display_interval_us;
      ACE_Time_Value display_interval;
      long timer_id = -1;
#if defined(ACE_WIN32) || defined(ACE_WIN64)
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      sample_rate_i = audio_info_p->nSamplesPerSec;
      bytes_per_sample_i = audio_info_p->wBitsPerSample / 8;
      is_signed_sample_format_b = audio_info_p->wBitsPerSample > 8; // signed if > 8 bit/sample
      is_integral_sample_format_b =
        !Stream_MediaFramework_DirectSound_Tools::isFloat (*audio_info_p);
      channels_ = audio_info_p->nChannels;
      frameSize_ = audio_info_p->nChannels * bytes_per_sample_i;
      ACE_ASSERT (frameSize_ == audio_info_p->nBlockAlign);
      iterator_.initialize (frameSize_,
                            bytes_per_sample_i,
                            !is_integral_sample_format_b,
                            is_signed_sample_format_b,
                            0x0123); // all Win32 sound data is little endian
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      sample_rate_i = media_type_s.rate;
      bytes_per_sample_i = snd_pcm_format_width (media_type_s.format) / 8;
      is_signed_sample_format_b =
        snd_pcm_format_signed (media_type_s.format) ? true : false;
      is_integral_sample_format_b =
        snd_pcm_format_float (media_type_s.format) ? false : true;
      channels_ = media_type_s.channels;
      frameSize_ = bytes_per_sample_i * media_type_s.channels;
      iterator_.initialize (frameSize_,
                            bytes_per_sample_i,
                            !is_integral_sample_format_b,
                            is_signed_sample_format_b,
                            snd_pcm_format_little_endian (media_type_s.format) ? 0x0123 : 0x3210);
#endif // ACE_WIN32 || ACE_WIN64
      inherited4::Initialize (channels_,
                              inherited::configuration_->spectrumAnalyzerConfiguration->numberOfBins,
                              sample_rate_i);

      // *NOTE*: integral types need normalization --> compute factor once
      if (is_integral_sample_format_b)
        switch (bytes_per_sample_i)
        {
          case 1:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int8_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint8_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          case 2:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int16_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint16_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          case 4:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int32_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint32_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          case 8:
            if (is_signed_sample_format_b)
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<int64_t> (bytes_per_sample_i, is_signed_sample_format_b));
            else
              normalizationFactor_ = 1 / static_cast<ValueType> (Common_Tools::max<uint64_t> (bytes_per_sample_i, is_signed_sample_format_b));
            break;
          default:
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: invalid/unknown sample size (was: %u), aborting\n"),
                        inherited::mod_->name (),
                        bytes_per_sample_i));
            goto error;
          }
        } // end SWITCH
      else
        normalizationFactor_ = 1;

      // start display update timer
      display_interval_us =
        static_cast<suseconds_t> (1000000L / STREAM_VIS_SPECTRUMANALYZER_DEFAULT_FRAME_RATE);
      display_interval.set (0, display_interval_us);
      timer_id =
        itimer_manager_p->schedule_timer (&handler_,         // event handler handle
                                          NULL,              // asynchronous completion token
                                          COMMON_TIME_NOW,   // first wakeup time
                                          display_interval); // interval
      if (unlikely (timer_id == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      handler_.set (timer_id);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: scheduled display update timer (id: %d) for interval %#T\n"),
                  inherited::mod_->name (),
                  timer_id,
                  &display_interval));

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      long timer_id = handler_.get_2 ();
      if (likely (timer_id != -1))
      {
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

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename ValueType>
void
Stream_Module_Vis_Console_Audio_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  MediaType,
                                  ValueType,
                                  STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM>::handle (const void* arg_in)
{
  //STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_Console_Audio_T::handle"));

  ACE_UNUSED_ARG (arg_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->spectrumAnalyzerConfiguration);

#define NUMBER_OF_BINS_TO_DISPLAY_PER_CHANNEL 10
  static int scale_x =
    inherited::configuration_->spectrumAnalyzerConfiguration->numberOfBins / NUMBER_OF_BINS_TO_DISPLAY_PER_CHANNEL;

  ACE_UINT32 level_i;
  for (ACE_UINT32 c = 0; c < channels_; ++c)
  {
    for (ACE_UINT32 i = 0; i < NUMBER_OF_BINS_TO_DISPLAY_PER_CHANNEL; ++i)
    {
      unsigned int slot_number_i = (i * scale_x) + (scale_x / 2); // *NOTE*: display the middle bin of each 'scale_x' bin block
      ValueType magnitude = inherited4::Magnitude2 (slot_number_i, c, false);

      level_i = static_cast<uint32_t> (fminf (fmaxf (magnitude * 30.0f, 0.0f), 39.0f));

      ACE_OS::printf (ACE_TEXT_ALWAYS_CHAR ("|%*c%*c|\n"),
                      level_i + 1, '*', 40 - level_i, ' ');
    } // end FOR

    if (c < channels_ - 1)
      ACE_OS::printf (ACE_TEXT_ALWAYS_CHAR ("%*c\n"),
                      40 + 2 + 1, '+');
  } // end FOR

  /* move cursor back up */
  ACE_OS::printf (ACE_TEXT_ALWAYS_CHAR ("%c[%dA"),
                  0x1b, // ESC
                  channels_ * NUMBER_OF_BINS_TO_DISPLAY_PER_CHANNEL + (channels_ - 1));

  ACE_OS::fflush (stdout);
}
