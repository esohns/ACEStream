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
#include <strstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

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
Stream_Decoder_DeepSpeechDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                   MediaType>::Stream_Decoder_DeepSpeechDecoder_T (ISTREAM_T* stream_in)
#else
                                   MediaType>::Stream_Decoder_DeepSpeechDecoder_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , bufferedMs_ (0)
 , context_ (NULL)
 , context2_ (NULL)
 , decodedWords_ (0)
 , sampleSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_DeepSpeechDecoder_T::Stream_Decoder_DeepSpeechDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_DeepSpeechDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::~Stream_Decoder_DeepSpeechDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_DeepSpeechDecoder_T::~Stream_Decoder_DeepSpeechDecoder_T"));

  if (unlikely (context2_))
    DS_FinishStream (context2_);
  if (likely (context_))
    DS_FreeModel (context_);
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
Stream_Decoder_DeepSpeechDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_DeepSpeechDecoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    bufferedMs_ = 0;
    if (unlikely (context2_))
    {
      DS_FinishStream (context2_); context2_ = NULL;
    } // end IF
    if (likely (context_))
    {
      DS_FreeModel (context_); context_ = NULL;
    } // end IF
    decodedWords_ = 0;
    sampleSize_ = 0;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: using model file: \"%s\"\n"),
              inherited::mod_->name (),
              ACE_TEXT (configuration_in.modelFile.c_str ())));
  result = DS_CreateModel (configuration_in.modelFile.c_str (),
                           &context_);
  if (unlikely (result || !context_))
  { char* string_p = DS_ErrorCodeToErrorMessage (result);
    ACE_ASSERT (string_p);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to DS_CreateModel(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.modelFile.c_str ()),
                ACE_TEXT (string_p)));
    DS_FreeString (string_p);
    return false;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: default beam width: %u\n"),
//              inherited::mod_->name (),
//              DS_GetModelBeamWidth (context_)));
  if (DS_GetModelBeamWidth (context_) != STREAM_DEC_DEEPSPEECH_DEFAULT_BEAM_WIDTH)
  {
    result = DS_SetModelBeamWidth (context_, STREAM_DEC_DEEPSPEECH_DEFAULT_BEAM_WIDTH);
    if (unlikely (result))
    { char* string_p = DS_ErrorCodeToErrorMessage (result);
      ACE_ASSERT (string_p);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DS_SetModelBeamWidth(%u): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  STREAM_DEC_DEEPSPEECH_DEFAULT_BEAM_WIDTH,
                  ACE_TEXT (string_p)));
      DS_FreeString (string_p);
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set beam width: %u\n"),
                inherited::mod_->name (),
                STREAM_DEC_DEEPSPEECH_DEFAULT_BEAM_WIDTH));
  } // end IF

  if (!configuration_in.scorerFile.empty ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: using scorer file: \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (configuration_in.scorerFile.c_str ())));
    result = DS_EnableExternalScorer (context_,
                                      configuration_in.scorerFile.c_str ());
    if (unlikely (result))
    { char* string_p = DS_ErrorCodeToErrorMessage (result);
      ACE_ASSERT (string_p);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DS_EnableExternalScorer(\"%s\"): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.scorerFile.c_str ()),
                  ACE_TEXT (string_p)));
      DS_FreeString (string_p);
      return false;
    } // end IF
  } // end IF
//  result = DS_SetScorerAlphaBeta (context_,
//                                  0.0F, 0.0F);
//  if (unlikely (result))
//  { char* string_p = DS_ErrorCodeToErrorMessage (result);
//    ACE_ASSERT (string_p);
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to DS_SetScorerAlphaBeta(): \"%s\", aborting\n"),
//                inherited::mod_->name (),
//                ACE_TEXT (string_p)));
//    DS_FreeString (string_p);
//    return false;
//  } // end IF

  for (Stream_Decoder_DeepSpeech_HotWordsConstIterator_t iterator = configuration_in.hotWords.begin ();
       iterator != configuration_in.hotWords.end ();
       ++iterator)
  {
    result = DS_AddHotWord (context_,
                            (*iterator).first.c_str (),
                            (*iterator).second);
    if (unlikely (result))
    { char* string_p = DS_ErrorCodeToErrorMessage (result);
      ACE_ASSERT (string_p);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DS_AddHotWord(\"%s\",%f): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT ((*iterator).first.c_str ()), (*iterator).second,
                  ACE_TEXT (string_p)));
      DS_FreeString (string_p);
      return false;
    } // end IF
  } // end FOR

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
Stream_Decoder_DeepSpeechDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_DeepSpeechDecoder_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (context2_);

  typename DataMessageType::DATA_T& data_r =
    const_cast<typename DataMessageType::DATA_T&> (message_inout->getR ());
  ACE_Message_Block* message_block_p = message_inout;
  const char* partial_p = NULL;
  unsigned int number_of_samples_i = 0;
  int result = -1;

  while (message_block_p)
  {
    number_of_samples_i = message_block_p->length () / sampleSize_;
    DS_FeedAudioContent (context2_,
                         reinterpret_cast<short int*> (message_block_p->rd_ptr ()),
                         number_of_samples_i);
    bufferedMs_ +=
      static_cast<unsigned int> ((number_of_samples_i * 1000.0) / static_cast<double> (DS_GetModelSampleRate (context_)));
    if (bufferedMs_ < STREAM_DEC_DEEPSPEECH_DECODE_BUFFER_LENGTH_MS)
      goto continue_;
    bufferedMs_ -= STREAM_DEC_DEEPSPEECH_DECODE_BUFFER_LENGTH_MS;
    partial_p = DS_IntermediateDecode (context2_);
    decodedWords_ += processWords (partial_p,
                                   data_r.words); // *TODO*: remove type inference
    if (likely (partial_p))
    {
      DS_FreeString (const_cast<char*> (partial_p)); partial_p = NULL;
    } // end IF
    if (decodedWords_ < STREAM_DEC_DEEPSPEECH_RESTREAM_WORD_LIMIT)
      goto continue_;
    partial_p = DS_FinishStream (context2_);
    decodedWords_ += processWords (partial_p,
                                   data_r.words); // *TODO*: remove type inference
    if (likely (partial_p))
    {
      DS_FreeString (const_cast<char*> (partial_p)); partial_p = NULL;
    } // end IF
    context2_ = NULL;
    result = DS_CreateStream (context_, &context2_);
    if (unlikely ((result != DS_ERR_OK) || !context2_))
    { char* string_p = DS_ErrorCodeToErrorMessage (result);
      ACE_ASSERT (string_p);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DS_CreateStream(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (string_p)));
      DS_FreeString (string_p);
      goto error;
    } // end IF
    decodedWords_ = 0;
    bufferedMs_ = 0;
continue_:
    message_block_p = message_block_p->cont ();
  } // end WHILE

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
unsigned int
Stream_Decoder_DeepSpeechDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::processWords (const char* inputString_in,
                                                             Stream_Decoder_DeepSpeech_Result_t& result_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_DeepSpeechDecoder_T::processWords"));

  // sanity check(s)
  if (!inputString_in ||
      !ACE_OS::strlen (inputString_in))
    return 0;

  unsigned int prev_size_i = result_out.size ();
  std::string token_string;
//  std::istringstream string_stream (inputString_in); // *NOTE*: avoid this copy
  std::istrstream string_stream (const_cast<char*> (inputString_in),
                                 ACE_OS::strlen (inputString_in));
#if 0
  while (std::getline (string_stream, token_string, ' '))
    result_out.push_back (token_string);
#else
  // step1: retrieve new words
  Stream_Decoder_DeepSpeech_Result_t result;
  while (std::getline (string_stream, token_string, ' '))
    result.push_back (token_string);

  // step2: crop any trailing/beginning duplicates
  if (!prev_size_i)
  {
    result_out = result;
    return result_out.size ();
  } // end IF
  Stream_Decoder_DeepSpeech_ResultIterator_t start_iterator =
    result_out.begin ();
  Stream_Decoder_DeepSpeech_ResultDifference_t index_i = 0, index_2 = 0;
  Stream_Decoder_DeepSpeech_ResultIterator_t iterator_2;
continue_:
  for (Stream_Decoder_DeepSpeech_ResultIterator_t iterator = start_iterator;
       iterator != result_out.end ();
       ++iterator)
  {
continue_2:
    index_i = std::distance (start_iterator, iterator);
    iterator_2 = std::find (result.begin (), result.end (), *iterator);
    if (iterator_2 == result.end ())
    { // no overlap at this index --> increment start_iterator and continue
      std::advance (start_iterator, 1);
      if (start_iterator == result_out.end ())
        goto continue_3; // done; append
      goto continue_;
    } // end IF
    index_2 = std::distance (result.begin (), iterator_2);
    if (index_i != index_2)
    { // no overlap at this index --> increment start_iterator and continue
      std::advance (start_iterator, 1);
      if (start_iterator == result_out.end ())
        goto continue_3; // done; append
      goto continue_;
    } // end IF
    std::advance (iterator, 1);
    if (iterator == result_out.end ())
    { // found matching trailing sequence: it starts at start_iterator
      result_out.erase (start_iterator, result_out.end ());
continue_3:
      result_out.insert (result_out.end (), result.begin (), result.end ());
      break;
    } // end IF
    goto continue_2;
  } // end FOR

continue_4:
#endif // 0
  return result_out.size () - prev_size_i;
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
Stream_Decoder_DeepSpeechDecoder_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_DeepSpeechDecoder_T::handleSessionMessage"));

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
                                media_type_s);
      struct tWAVEFORMATEX* waveformatex_p =
        Stream_MediaFramework_DirectShow_Tools::toWaveFormatEx (media_type_s);
      ACE_ASSERT (waveformatex_p);
      if (unlikely (Stream_MediaFramework_DirectSound_Tools::isFloat (*waveformatex_p)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample format, aborting\n"),
                    inherited::mod_->name ()));
        CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
        goto error;
      } // end IF
      if (unlikely (waveformatex_p->nSamplesPerSec != static_cast<unsigned int> (DS_GetModelSampleRate (context_))))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample rate (expected %d; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    DS_GetModelSampleRate (context_), waveformatex_p->nSamplesPerSec));
        CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
        goto error;
      } // end IF
      if (unlikely (waveformatex_p->wBitsPerSample != 16))
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
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      if (unlikely (snd_pcm_format_linear (media_type_s.format) == 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample format (expected \"%s\"; was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_pcm_format_name (SND_PCM_FORMAT_S16_LE)), ACE_TEXT (snd_pcm_format_name (media_type_s.format))));
        goto error;
      } // end IF
      if (unlikely (media_type_s.rate != static_cast<unsigned int> (DS_GetModelSampleRate (context_))))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid sample rate (expected %d; was: %u), aborting\n"),
                    inherited::mod_->name (),
                    DS_GetModelSampleRate (context_), media_type_s.rate));
        goto error;
      } // end IF
      if (unlikely (snd_pcm_format_width (media_type_s.format) != 16))
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
      ACE_ASSERT(!context2_);

      result = DS_CreateStream (context_, &context2_);
      if (unlikely (result != DS_ERR_OK))
      { char* string_p = DS_ErrorCodeToErrorMessage (result);
        ACE_ASSERT (string_p);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to DS_CreateStream(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (string_p)));
        DS_FreeString (string_p);
        goto error;
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (likely (context2_))
      {
        char* string_p = DS_FinishStream (context2_);
        if (likely (string_p))
        { // *TODO*: send final results downstream
          DS_FreeString (string_p); string_p = NULL;
        } // end IF
        context2_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
