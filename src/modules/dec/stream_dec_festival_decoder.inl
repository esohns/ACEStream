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

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_FestivalDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::Stream_Decoder_FestivalDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FestivalDecoder_T::Stream_Decoder_FestivalDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_FestivalDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::~Stream_Decoder_FestivalDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FestivalDecoder_T::~Stream_Decoder_FestivalDecoder_T"));

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
Stream_Decoder_FestivalDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FestivalDecoder_T::initialize"));

  if (inherited::isInitialized_)
  { ACE_ASSERT (inherited::configuration_);
    if (inherited::configuration_->manageFestival)
      festival_tidy_up ();
  } // end IF

  if (configuration_in.manageFestival)
    festival_initialize (1, FESTIVAL_HEAP_SIZE);

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
Stream_Decoder_FestivalDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FestivalDecoder_T::handleDataMessage"));

  passMessageDownstream_out = false;

  EST_String string (message_inout->rd_ptr ());
  EST_Wave wave;

  int result = festival_text_to_wave (string,
                                      wave);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to festival_text_to_wave(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF
  message_inout->release (); message_inout = NULL;

  // step1: allocate message block
  ACE_ASSERT (inherited::configuration_->messageAllocator);
  ACE_Message_Block* message_block_p =
    static_cast<ACE_Message_Block*> (inherited::configuration_->messageAllocator->malloc (wave.num_samples () * sizeof (short)));
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IAllocator::malloc(%d): \"%m\", aborting\n"),
                inherited::mod_->name (),
                wave.num_samples () * sizeof (short)));
    goto error;
  } // end IF

  // step2: copy data into message buffer
  wave.copy_channel (0,
                     reinterpret_cast<short*> (message_block_p->wr_ptr ()),
                     0,
                     EST_ALL);
  message_block_p->wr_ptr (wave.num_samples () * sizeof (short));

  // step3: push data downstream
  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
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
Stream_Decoder_FestivalDecoder_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionDataContainerType,
                                 MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_FestivalDecoder_T::handleSessionMessage"));

  int result = -1;

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
      // *NOTE*: festival generates PCM mono signed 16 bits at 16000Hz
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
      Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                media_type_2);
      ACE_OS::memset (&media_type, 0, sizeof (MediaType));
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_2;
      media_type_2.format = SND_PCM_FORMAT_S16;
      media_type_2.subFormat = SND_PCM_SUBFORMAT_STD;
      media_type_2.channels = 1;
      media_type_2.rate = 16000;
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
    { ACE_ASSERT (inherited::configuration_);
      if (inherited::configuration_->manageFestival)
        festival_tidy_up ();

      break;
    }
    default:
      break;
  } // end SWITCH
}
