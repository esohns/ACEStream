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

#include "cmulex/cmu_lex.h"
#include "usenglish/usenglish.h"

#include "cmu_grapheme_lang/cmu_grapheme_lang.h"
#include "cmu_grapheme_lex/cmu_grapheme_lex.h"
//#include "cmu_indic_lang/cmu_indic_lang.h"
//#include "cmu_indic_lex/cmu_indic_lex.h"

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
Stream_Decoder_FliteDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Decoder_FliteDecoder_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Decoder_FliteDecoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , CBData_ ()
 , features_ (NULL)
 , voice_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FliteDecoder_T::Stream_Decoder_FliteDecoder_T"));

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
Stream_Decoder_FliteDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::~Stream_Decoder_FliteDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FliteDecoder_T::~Stream_Decoder_FliteDecoder_T"));

  if (unlikely (features_))
    delete_features (features_);
  if (unlikely (flite_voice_list))
  {
    delete_val (flite_voice_list); flite_voice_list = NULL;
  } // end IF
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
Stream_Decoder_FliteDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FliteDecoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (unlikely (features_))
    {
      delete_features (features_); features_ = NULL;
    }
    voice_ = NULL;
    if (unlikely (flite_voice_list))
    {
      delete_val (flite_voice_list); flite_voice_list = NULL;
    } // end IF
  } // end IF

  if (configuration_in.manageFlite)
  {
    result = flite_init ();
    if (unlikely (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to flite_init(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end IF
  //ACE_ASSERT (!flite_voice_list);
  if (unlikely (!registerVoices (configuration_in.voiceDirectory)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to registerVoices(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.voiceDirectory.c_str ())));
    return false;
  } // end IF
  //ACE_ASSERT (flite_voice_list);
  ACE_ASSERT (!voice_);
  //voice_ = flite_voice_select (configuration_in.voice.c_str ());
  std::string filename_string = configuration_in.voiceDirectory;
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string += configuration_in.voice;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_FLITE_VOICE_FILENAME_EXTENSION_STRING);
  voice_ = flite_voice_load (filename_string.c_str ());
  if (unlikely (!voice_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to flite_voice_load(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (filename_string.c_str ())));
    return false;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: loaded voice \"%s\"\n"),
              inherited::mod_->name (),
              ACE_TEXT (configuration_in.voice.c_str ())));
  ACE_ASSERT (!features_);
  features_ = new_features ();
  if (unlikely (!features_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to new_features(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  result = feat_copy_into (features_,
                           voice_->features);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to feat_copy_into(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  struct cst_audio_streaming_info_struct* audio_streaming_info_p =
    new_audio_streaming_info ();
  if (unlikely (!audio_streaming_info_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to new_audio_streaming_info(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  audio_streaming_info_p->asc = libacestream_flite_audio_stream_chunk_cb;
  audio_streaming_info_p->userdata = &CBData_;
  feat_set (voice_->features,
            ACE_TEXT_ALWAYS_CHAR ("streaming_info"),
            audio_streaming_info_val (audio_streaming_info_p));

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
bool
Stream_Decoder_FliteDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::registerVoices (const std::string& baseDirectory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FliteDecoder_T::registerVoices"));

  // sanity check(s)
  if (unlikely (flite_voice_list))
  {
    delete_val (flite_voice_list); flite_voice_list = NULL;
  } // end IF

  // step1: register languages
  flite_add_lang (ACE_TEXT_ALWAYS_CHAR ("eng"),
                  usenglish_init,
                  cmu_lex_init);
  flite_add_lang (ACE_TEXT_ALWAYS_CHAR ("usenglish"),
                  usenglish_init,
                  cmu_lex_init);
  //flite_add_lang (ACE_TEXT_ALWAYS_CHAR ("cmu_indic_lang"),
  //                cmu_indic_lang_init,
  //                cmu_indic_lex_init);
  flite_add_lang (ACE_TEXT_ALWAYS_CHAR ("cmu_grapheme_lang"),
                  cmu_grapheme_lang_init,
                  cmu_grapheme_lex_init);

  //// step2: register voices
  //struct cst_voice_struct* voice_p =
  //  register_cmu_us_slt (baseDirectory_in.empty () ? NULL
  //                                                 : baseDirectory_in.c_str ());
  //if (unlikely (!voice_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to register_cmu_us_slt(\"%s\"), aborting\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (baseDirectory_in.c_str ())));
  //  return false;
  //} // end IF
  //ACE_ASSERT (voice_val (voice_p));
  //flite_voice_list = cons_val (voice_val (voice_p), flite_voice_list);
  //if (unlikely (!flite_voice_list))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to cons_val(), aborting\n"),
  //              inherited::mod_->name ()));
  //  return false;
  //} // end IF

  //flite_voice_list = val_reverse (flite_voice_list);
  //ACE_ASSERT (flite_voice_list);

  return true;
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
Stream_Decoder_FliteDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FliteDecoder_T::handleDataMessage"));

  passMessageDownstream_out = false;

  float duration_f = flite_text_to_speech (message_inout->rd_ptr (),
                                           voice_,
                                           ACE_TEXT_ALWAYS_CHAR ("stream"));
  if (unlikely (!duration_f))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to flite_text_to_speech(): \"%s\", aborting\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF

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
Stream_Decoder_FliteDecoder_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataContainerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FliteDecoder_T::handleSessionMessage"));

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
      ACE_ASSERT (session_data_r.formats.empty ());
      MediaType media_type;
      // *NOTE*: flite generates PCM mono signed 16 bits at 8000[/16000]Hz
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
      struct tWAVEFORMATEX waveformatex_s;
      ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
      waveformatex_s.wFormatTag = WAVE_FORMAT_PCM;
      waveformatex_s.nChannels = 1;
      waveformatex_s.nSamplesPerSec = 16000;
      waveformatex_s.wBitsPerSample = 16;
      waveformatex_s.nBlockAlign =
        (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
      waveformatex_s.nAvgBytesPerSec =
        (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
      // waveformatex_s.cbSize = 0;
      HRESULT result = CreateAudioMediaType (&waveformatex_s,
                                             &media_type_2,
                                             TRUE); // set format ?
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CreateAudioMediaType(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_OS::memset (&media_type, 0, sizeof (MediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_2;
      media_type_2.format = SND_PCM_FORMAT_S16;
      media_type_2.subFormat = SND_PCM_SUBFORMAT_STD;
      media_type_2.channels = 1;
      media_type_2.rate = 16000;
#endif // ACE_WIN32 || ACE_WIN64
      inherited2::getMediaType (media_type_2,
                                media_type);
      session_data_r.formats.push_back (media_type);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
#endif // ACE_WIN32 || ACE_WIN64
      break;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
error:
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
