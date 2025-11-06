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
Stream_Decoder_ESpeakNGDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::Stream_Decoder_ESpeakNGDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , CBData_ ()
 , sampleRate_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ESpeakNGDecoder_T::Stream_Decoder_ESpeakNGDecoder_T"));

  CBData_.task = this;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_ESpeakNGDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::~Stream_Decoder_ESpeakNGDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ESpeakNGDecoder_T::~Stream_Decoder_ESpeakNGDecoder_T"));

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
Stream_Decoder_ESpeakNGDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ESpeakNGDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    sampleRate_ = 0;
  } // end IF

  int options_i = 0;
  int result = espeak_Initialize (AUDIO_OUTPUT_RETRIEVAL,//AUDIO_OUTPUT_SYNCHRONOUS,
                                  STREAM_DEC_ESPEAK_NG_DECODE_BUFFER_LENGTH_MS,
                                  configuration_in.voiceDirectory.c_str (), // espeak-ng-data directory
                                  options_i);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to espeak_Initialize(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  sampleRate_ = static_cast<unsigned int> (result);

  if (!configuration_in.voice.empty ())
  {
    espeak_VOICE voice_s;
    ACE_OS::memset (&voice_s, 0, sizeof (espeak_VOICE));
    //voice_s.name = ACE_TEXT_ALWAYS_CHAR ("default");
    voice_s.languages = configuration_in.voice.c_str ();
    voice_s.gender = 2; // 0=none 1=male, 2=female
    //voice_s.variant = 0;
    result = espeak_SetVoiceByProperties (&voice_s);
    if (unlikely (result != EE_OK))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to espeak_SetVoiceByProperties(\"%s\"): %d, aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.voice.c_str ()),
                  result));
      sampleRate_ = 0; 
      return false;
    } // end IF
  } // end IF

  espeak_SetSynthCallback (libacestream_espeak_ng_synth_callback);

  CBData_.allocator = allocator_in;

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
Stream_Decoder_ESpeakNGDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ESpeakNGDecoder_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (!message_inout->cont ());

  unsigned int flags_i = espeakCHARS_AUTO;
  int result =
    espeak_Synth (message_inout->rd_ptr (),
                  message_inout->length (),
                  0, POS_CHARACTER,
                  0,
                  flags_i,
                  NULL,
                  &CBData_);
  if (unlikely (result != EE_OK))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to espeak_Synth(): %d, aborting\n"),
                inherited::mod_->name (),
                result));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF
  message_inout->release (); message_inout = NULL;

  return;

error:
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
Stream_Decoder_ESpeakNGDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ESpeakNGDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (inherited::sessionData_->getR ());
      // ACE_ASSERT (session_data_r.formats.empty ());
      MediaType media_type;
      // *NOTE*: flite generates PCM mono signed 16 bits at 8000[/16000]Hz
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
      struct tWAVEFORMATEX waveformatex_s;
      ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
      waveformatex_s.wFormatTag = WAVE_FORMAT_PCM;
      waveformatex_s.nChannels = 1;
      waveformatex_s.nSamplesPerSec = sampleRate_;
      waveformatex_s.wBitsPerSample = 16;
      waveformatex_s.nBlockAlign =
        (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
      waveformatex_s.nAvgBytesPerSec =
        (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
      // waveformatex_s.cbSize = 0;
      Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                media_type_2);
      ACE_OS::memset (&media_type, 0, sizeof (MediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_2;
      media_type_2.format = SND_PCM_FORMAT_S16;
      media_type_2.subFormat = SND_PCM_SUBFORMAT_STD;
      media_type_2.channels = 1;
      media_type_2.rate = sampleRate_;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (media_type_2,
                                STREAM_MEDIATYPE_AUDIO,
                                media_type);
      session_data_r.formats.push_back (media_type);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
#endif // ACE_WIN32 || ACE_WIN64
      break;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
//error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
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
