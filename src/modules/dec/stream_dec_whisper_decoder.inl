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

#include <algorithm>
#include <iterator>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_tools.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_WhisperCppDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::Stream_Decoder_WhisperCppDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , buffer_ (NULL)
 , bufferedMs_ (0)
 , context_ (NULL)
 , parameters_ ()
 , parameters2_ ()
 , sampleSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WhisperCppDecoder_T::Stream_Decoder_WhisperCppDecoder_T"));

  parameters_ = whisper_context_default_params ();

  parameters2_ = whisper_full_default_params (STREAM_DEC_DECODER_WHISPERCPP_DECODER_DEFAULT_SAMPLING_STRATEGY);
  if (unlikely (!parameters_.use_gpu))
    parameters2_.n_threads = Common_Tools::getNumberOfCPUs (true);
  // //parameters2_.n_max_text_ctx = 16;
  // //parameters2_.no_context = false;
  // parameters2_.no_timestamps = true;
  // parameters2_.single_segment = true;
  // parameters2_.print_progress = false;
  // parameters2_.print_timestamps = false;
  // parameters2_.temperature_inc = -1.0f;
  // parameters2_.greedy.best_of = 1;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_WhisperCppDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::~Stream_Decoder_WhisperCppDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WhisperCppDecoder_T::~Stream_Decoder_WhisperCppDecoder_T"));

  if (unlikely (buffer_))
    buffer_->release ();
  if (unlikely (context_))
    whisper_free (context_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Decoder_WhisperCppDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WhisperCppDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (likely (buffer_))
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
    bufferedMs_ = 0;
    if (likely (context_))
    {
      whisper_free (context_); context_ = NULL;
    } // end IF
    sampleSize_ = 0;
  } // end IF

  context_ = whisper_init_from_file_with_params (configuration_in.modelFile.c_str (),
                                                 parameters_);
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to whisper_init_from_file_with_params(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: using model file: \"%s\"\n"),
              inherited::mod_->name (),
              ACE_TEXT (configuration_in.modelFile.c_str ())));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_WhisperCppDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WhisperCppDecoder_T::handleDataMessage"));

  int result = -1;

  // this module may (!) buffer data...
  passMessageDownstream_out = false;

  unsigned int number_of_samples_i = 
    static_cast<unsigned int> (message_inout->length () / sampleSize_);
  bufferedMs_ +=
    static_cast<unsigned int> ((number_of_samples_i * 1000.0) / static_cast<double> (WHISPER_SAMPLE_RATE));
  if (bufferedMs_ < STREAM_DEC_DECODER_WHISPERCPP_DECODER_DEFAULT_BUFFER_LENGTH_MS)
  {
    if (!buffer_)
      buffer_ = message_inout;
    else
      Stream_Tools::append (buffer_, message_inout);
    return;
  } // end IF

  typename DataMessageType::DATA_T* data_p = NULL;
  int n_segments;

  ACE_Message_Block* message_block_p = buffer_;
  Stream_Tools::crunch (message_block_p,
                        inherited::configuration_->messageAllocator);
  if (unlikely (message_block_p != buffer_))
    buffer_ = static_cast<DataMessageType*> (message_block_p);

  number_of_samples_i =
    static_cast<unsigned int> (buffer_->length () / sampleSize_);
  parameters2_.duration_ms = bufferedMs_;
  parameters2_.max_tokens =
    static_cast<int> (4.0f * (int)number_of_samples_i / (float)WHISPER_SAMPLE_RATE);
  if (whisper_full (context_,
                    parameters2_,
                    reinterpret_cast<float*> (buffer_->rd_ptr ()),
                    number_of_samples_i) != 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to whisper_full(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  data_p = &const_cast<typename DataMessageType::DATA_T&> (buffer_->getR ());
  n_segments = whisper_full_n_segments (context_);
  for (int i = 0; i < n_segments; ++i)
  {
    int n_tokens = whisper_full_n_tokens (context_, i);
    for (int j = 0; j < n_tokens; ++j)
    {
      const char* text_p = whisper_full_get_token_text (context_, i, j);
      data_p->words.push_back (text_p);
    } // end FOR
  } // end FOR

  result = inherited::put_next (buffer_, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  buffer_ = NULL;
  bufferedMs_ = 0;

  return;

error:
  if (buffer_)
  {
    buffer_->release (); buffer_ = NULL;
  } // end IF
  bufferedMs_ = 0;

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_WhisperCppDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WhisperCppDecoder_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
        inherited::sessionData_->getR ();
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (context_);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      struct tWAVEFORMATEX* waveformatex_p =
        Stream_MediaFramework_DirectShow_Tools::toWaveFormatEx (media_type_s);
      ACE_ASSERT (waveformatex_p);
      if (unlikely (!Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample format, aborting\n"),
                    inherited::mod_->name ()));
        CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
        goto error;
      } // end IF
      if (unlikely (waveformatex_p->nSamplesPerSec != WHISPER_SAMPLE_RATE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample rate (expected %u; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    WHISPER_SAMPLE_RATE, waveformatex_p->nSamplesPerSec));
        CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
        goto error;
      } // end IF
      if (unlikely (waveformatex_p->wBitsPerSample != 32))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample resolution (expected %d; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    16, waveformatex_p->wBitsPerSample));
        CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
        goto error;
      } // end IF
      if (unlikely (waveformatex_p->nChannels != 1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid channels (expected %d; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    1, waveformatex_p->nChannels));
        CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
        goto error;
      } // end IF
      sampleSize_ = (waveformatex_p->wBitsPerSample / 8);
      CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      if (unlikely (snd_pcm_format_linear (media_type_s.format) == 1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample format (expected \"%s\"; was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_pcm_format_name (SND_PCM_FORMAT_FLOAT)), ACE_TEXT (snd_pcm_format_name (media_type_s.format))));
        goto error;
      } // end IF
      if (unlikely (media_type_s.rate != WHISPER_SAMPLE_RATE))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample rate (expected %u; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    WHISPER_SAMPLE_RATE, media_type_s.rate));
        goto error;
      } // end IF
      if (unlikely (snd_pcm_format_width (media_type_s.format) != 32))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample resolution (expected %d; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    16, snd_pcm_format_width (media_type_s.format)));
        goto error;
      } // end IF
      if (unlikely (media_type_s.channels != 1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid channels (expected %d; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    1, media_type_s.channels));
        goto error;
      } // end IF

      sampleSize_ = (snd_pcm_format_width (media_type_s.format) / 8);
#endif // ACE_WIN32 || ACE_WIN64

      break;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    {
      break;
    }
    default:
      break;
  } // end SWITCH
}
