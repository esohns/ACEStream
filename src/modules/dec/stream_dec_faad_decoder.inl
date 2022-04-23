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

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_FAAD_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      SessionDataContainerType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      MediaType>::Stream_Decoder_FAAD_T (ISTREAM_T* stream_in)
#else
                      MediaType>::Stream_Decoder_FAAD_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , configuration_ ()
 , context_ (NULL)
 , sampleSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FAAD_T::Stream_Decoder_FAAD_T"));

  ACE_OS::memset (&configuration_, 0, sizeof (struct NeAACDecConfiguration));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_FAAD_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      SessionDataContainerType,
                      MediaType>::~Stream_Decoder_FAAD_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FAAD_T::~Stream_Decoder_FAAD_T"));

  if (unlikely (context_))
    NeAACDecClose (context_);
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
Stream_Decoder_FAAD_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      SessionDataContainerType,
                      MediaType>::initialize (const ConfigurationType& configuration_in,
                                              Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FAAD_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (likely (context_))
    {
      NeAACDecClose (context_); context_ = NULL;
    } // end IF
  } // end IF

  context_ = NeAACDecOpen ();
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to NeAACDecOpen(), aborting\n"),
                inherited::mod_->name ()));
    return false;
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
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Decoder_FAAD_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FAAD_T::handleDataMessage"));

  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (context_);

  ACE_Message_Block* message_block_p = message_inout;
  void* result_p = NULL;
  struct NeAACDecFrameInfo frame_info_s;
  DataMessageType* message_p = NULL;
  int result = -1;
  void* data_p = NULL;

  static bool is_first = true;
  if (unlikely (is_first))
  {
    is_first = false;

    unsigned long sample_rate = 0;
    unsigned char channels = 0;
    long result_2 =
      NeAACDecInit (context_,
                    reinterpret_cast<unsigned char*> (message_block_p->rd_ptr ()),
                    message_block_p->length (),
                    &sample_rate,
                    &channels);
    if (unlikely (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to NeAACDecInit(), aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: initialized faad context: %u channels @ %u samples/s\n"),
                inherited::mod_->name (),
                channels,
                sample_rate));
  } // end IF

  while (message_block_p)
  {
    message_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to allocate message(%u), aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      goto error;
    } // end IF

    ACE_OS::memset (&frame_info_s, 0, sizeof (struct NeAACDecFrameInfo));
    data_p = message_p->wr_ptr ();
    result_p =
      NeAACDecDecode2 (context_,
                       &frame_info_s,
                       reinterpret_cast<unsigned char*> (message_block_p->rd_ptr ()),
                       message_block_p->length (),
                       &data_p,
                       message_p->size ());
    if (unlikely (!result_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to NeAACDecDecode2(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (NeAACDecGetErrorMessage (frame_info_s.error))));
      message_p->release (); message_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (frame_info_s.bytesconsumed == message_block_p->length ());
    ACE_ASSERT (frame_info_s.samples);
    message_p->wr_ptr (frame_info_s.samples * sampleSize_);

    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      goto error;
    } // end IF

continue_:
    message_block_p = message_block_p->cont ();
  } // end WHILE

  message_inout->release (); message_inout = NULL;

  return;

error:
  message_inout->release (); message_inout = NULL;

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
Stream_Decoder_FAAD_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   SessionDataContainerType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FAAD_T::handleSessionMessage"));

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
      unsigned long sample_rate = 0;
      unsigned char channels = 0;
      unsigned char faad_format = 0;
      long result = 0;
      unsigned char result_2 = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      struct tWAVEFORMATEX* waveformatex_p =
        Stream_MediaFramework_DirectShow_Tools::toWaveFormatEx (media_type_s);
      ACE_ASSERT (waveformatex_p);
      sample_rate = waveformatex_p->sampleRate;
      channels = waveformatex_p->channels;
      sampleSize_ = (waveformatex_p->wBitsPerSample / 8);
      CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      sample_rate = media_type_s.rate;
      channels = media_type_s.channels;
      sampleSize_ = (snd_pcm_format_width (media_type_s.format) / 8);

      switch (media_type_s.format)
      {
        case SND_PCM_FORMAT_S16:
        case SND_PCM_FORMAT_U16:
          faad_format = FAAD_FMT_16BIT;
          break;
        case SND_PCM_FORMAT_S24_LE:
        case SND_PCM_FORMAT_S24_BE:
        case SND_PCM_FORMAT_U24_LE:
        case SND_PCM_FORMAT_U24_BE:
          faad_format = FAAD_FMT_24BIT;
          break;
        case SND_PCM_FORMAT_S32_LE:
        case SND_PCM_FORMAT_S32_BE:
        case SND_PCM_FORMAT_U32_LE:
        case SND_PCM_FORMAT_U32_BE:
          faad_format = FAAD_FMT_32BIT;
          break;
        case SND_PCM_FORMAT_FLOAT_LE:
        case SND_PCM_FORMAT_FLOAT_BE:
          faad_format = FAAD_FMT_FLOAT;
          break;
        case SND_PCM_FORMAT_FLOAT64_LE:
        case SND_PCM_FORMAT_FLOAT64_BE:
          faad_format = FAAD_FMT_DOUBLE;
          break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown format (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      media_type_s.format));
          goto error;
        }
      } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (sampleSize_);

      configuration_.defObjectType = LC;
      configuration_.defSampleRate = sample_rate;
      configuration_.outputFormat = faad_format;
      configuration_.downMatrix = 0;
      configuration_.useOldADTSFormat = 0;
      configuration_.dontUpSampleImplicitSBR = 0;

      result_2 = NeAACDecSetConfiguration (context_,
                                           &configuration_);
      if (unlikely (result_2))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: failed to NeAACDecSetConfiguration(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (NeAACDecGetErrorMessage (result_2))));
//        goto error;
      } // end IF

//      result = NeAACDecInit (context_,
//                             NULL,
//                             0,
//                             &sample_rate,
//                             &channels);
//      if (unlikely (result))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to NeAACDecInit(), aborting\n"),
//                    inherited::mod_->name ()));
//        goto error;
//      } // end IF

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
