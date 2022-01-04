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
#include "ace/OS.h"

#include "stream_macros.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

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
 , context_ (NULL)
 , context2_ (NULL)
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
    if (unlikely (context2_))
    {
      DS_FinishStream (context2_); context2_ = NULL;
    } // end IF
    if (likely (context_))
    {
      DS_FreeModel (context_); context_ = NULL;
    } // end IF
  } // end IF

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

//  result = DS_SetModelBeamWidth (context_, 0);
//  if (unlikely (result))
//  { char* string_p = DS_ErrorCodeToErrorMessage (result);
//    ACE_ASSERT (string_p);
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to DS_SetModelBeamWidth(): \"%s\", aborting\n"),
//                inherited::mod_->name (),
//                ACE_TEXT (string_p)));
//    DS_FreeString (string_p);
//    return false;
//  } // end IF

  if (!configuration_in.scorerFile.empty ())
  {
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

  for (std::vector<std::pair<std::string, float> >::const_iterator iterator = configuration_in.hotWords.begin ();
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

  int result = -1;
  ACE_Message_Block* message_block_p = message_inout;
  const char* last_p = NULL, *prev_p = NULL, *partial_p = NULL;
  while (message_block_p)
  {
    DS_FeedAudioContent (context2_,
                         reinterpret_cast<short int*> (message_block_p->rd_ptr ()),
                         message_block_p->length () / 2);

    partial_p = DS_IntermediateDecode (context2_);
    if (!last_p || ACE_OS::strcmp (last_p, partial_p))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (partial_p)));
      last_p = partial_p;
    } // end IF
    else
    {
      DS_FreeString (const_cast<char*> (partial_p));
    } // end ELSE
    if (prev_p && prev_p != last_p)
    {
      DS_FreeString (const_cast<char*> (prev_p));
    } // end IF

    message_block_p = message_block_p->cont ();
  } // end WHILE

  if (last_p)
  {
    DS_FreeString (const_cast<char*> (last_p));
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
      ACE_ASSERT (false); // *TODO*
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
      if (unlikely (media_type_s.rate != DS_GetModelSampleRate (context_)))
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
        {
          ACE_ASSERT (false); // *TODO*: send downstream
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
