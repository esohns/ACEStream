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

#include "common_file_tools.h"

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
Stream_Decoder_SAPIDecoder_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::Stream_Decoder_SAPIDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , format_ ()
 , stream_ (NULL)
 , tempFilePath_ ()
 , voice_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPIDecoder_T::Stream_Decoder_SAPIDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Decoder_SAPIDecoder_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::~Stream_Decoder_SAPIDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPIDecoder_T::~Stream_Decoder_SAPIDecoder_T"));

  if (stream_)
  {
    stream_->Close ();
    stream_->Release ();
  } // end IF
  if (voice_)
    voice_->Release ();
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
Stream_Decoder_SAPIDecoder_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPIDecoder_T::initialize"));

  HRESULT result;

  if (inherited::isInitialized_)
  {
    if (likely (stream_))
    {
      result = stream_->Close ();
      ACE_ASSERT (SUCCEEDED (result));
      stream_->Release (); stream_ = NULL;
    } // end IF
    if (likely (voice_))
    {
      voice_->Release (); voice_ = NULL;
    } // end IF
  } // end IF

  result = CoCreateInstance (CLSID_SpVoice, NULL,
                             CLSCTX_INPROC_SERVER,
                             IID_PPV_ARGS (&voice_));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_OS_Tools::GUIDToString (CLSID_SpVoice).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    return false;
  } // end IF

  result = format_.AssignFormat (SPSF_22kHz16BitMono);
  ACE_ASSERT (SUCCEEDED (result));

  tempFilePath_ =
    Common_File_Tools::getTempFilename (ACE_TEXT_ALWAYS_CHAR ("acestream_sapi_tts_temp_"), true);
  result = SPBindToFile (tempFilePath_.c_str (),
                         SPFM_CREATE_ALWAYS,
                         &stream_,
                         &format_.FormatId (),
                         format_.WaveFormatExPtr (),
                         SPFEI_ALL_EVENTS);
  ACE_ASSERT (SUCCEEDED (result) && stream_);

  result = voice_->SetOutput (stream_, TRUE);
  ACE_ASSERT (SUCCEEDED (result));

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
Stream_Decoder_SAPIDecoder_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPIDecoder_T::handleDataMessage"));

  passMessageDownstream_out = false;
  std::wstring text_string = ACE_TEXT_ALWAYS_WCHAR (message_inout->rd_ptr ());
  message_inout->release (); message_inout = NULL;

  // sanity check(s)
  ACE_ASSERT (voice_);

  ACE_Message_Block* message_block_p = NULL;
  uint8_t* data_p = NULL, *data_2 = NULL;
  ACE_UINT64 file_size_i, offset_i = 0;
  int result_2;

  // step1: speak text --> WAV file
  HRESULT result = voice_->Speak (text_string.c_str (),
                                  SPF_DEFAULT,
                                  NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ISpVoice::Speak(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  result = stream_->Close ();
  ACE_ASSERT (SUCCEEDED (result));

  // step2: (sort of-) parse/skip over WAV header
  if (unlikely (!Common_File_Tools::load (tempFilePath_,
                                          data_p,
                                          file_size_i,
                                          0)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_File_Tools::load(\"%s\"): \"%m\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (tempFilePath_.c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (data_p);
  data_2 = data_p;

#define DESC_SIZE 4
#define COPY_SIZE 1024
  const char riff_hdr[DESC_SIZE + 1] = {'R', 'I', 'F', 'F', '\0'};
  const char wave_hdr[DESC_SIZE + 1] = {'W', 'A', 'V', 'E', '\0'};
  const char data_hdr[DESC_SIZE + 1] = {'d', 'a', 't', 'a', '\0'};

  if (ACE_OS::strncmp ((char*)data_2, riff_hdr, DESC_SIZE))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid RIFF header(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  data_2 += DESC_SIZE; offset_i += DESC_SIZE;

  data_2 += DESC_SIZE; offset_i += DESC_SIZE; // skip over file size

  if (ACE_OS::strncmp ((char*)data_2, wave_hdr, DESC_SIZE))
  {
    ACE_DEBUG ((LM_ERROR, ACE_TEXT ("%s: invalid WAVE header(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  data_2 += DESC_SIZE; offset_i += DESC_SIZE;

  // step2: allocate message block
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->messageAllocator);
  message_block_p =
    static_cast<ACE_Message_Block*> (inherited::configuration_->messageAllocator->malloc (inherited::configuration_->allocatorConfiguration->defaultBufferSize));
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IAllocator::malloc(%u): \"%m\", aborting\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    goto error;
  } // end IF

  // step2: copy data into message buffer
  while (offset_i < file_size_i)
  {
    bool is_data_chunk;
    int32_t chunk_size;

    is_data_chunk = ACE_OS::strncmp ((char *)data_2, data_hdr, DESC_SIZE) == 0;
    data_2 += DESC_SIZE; offset_i += DESC_SIZE;
#define big_endian_int(buffer) (int32_t)*buffer | ((int32_t)*(buffer + 1) << 8) | ((int32_t)*(buffer + 2) << 16) | ((int32_t)*(buffer + 3) << 24)
    chunk_size = big_endian_int (data_2);
    data_2 += DESC_SIZE; offset_i += DESC_SIZE;

    if (is_data_chunk)
    {
      int32_t num_blocks = chunk_size / COPY_SIZE;
      int32_t trailing_bytes = chunk_size % COPY_SIZE;
      for (int32_t i = 0; i < num_blocks; i++)
      {
        if (unlikely (message_block_p->space () < COPY_SIZE))
        {
          // step3: push data downstream
          result = inherited::put_next (message_block_p, NULL);
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            goto error;
          } // end IF

          message_block_p =
            static_cast<ACE_Message_Block*> (inherited::configuration_->messageAllocator->malloc (inherited::configuration_->allocatorConfiguration->defaultBufferSize));
          if (unlikely (!message_block_p))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to Stream_IAllocator::malloc(%u): \"%m\", aborting\n"),
                        inherited::mod_->name (),
                        inherited::configuration_->allocatorConfiguration->defaultBufferSize));
            goto error;
          } // end IF
          ACE_ASSERT (message_block_p->space () >= COPY_SIZE);
        } // end IF

        result_2 = message_block_p->copy ((char*)data_2, COPY_SIZE);
        if (unlikely (result_2 == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%u): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      COPY_SIZE));
          goto error;
        } // end IF
        data_2 += COPY_SIZE; offset_i += COPY_SIZE;
      } // end FOR

      if (!trailing_bytes)
        continue;
      if (unlikely (message_block_p->space () < trailing_bytes))
      {
        // step3: push data downstream
        result = inherited::put_next (message_block_p, NULL);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF

        message_block_p =
          static_cast<ACE_Message_Block*> (inherited::configuration_->messageAllocator->malloc (inherited::configuration_->allocatorConfiguration->defaultBufferSize));
        if (unlikely (!message_block_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_IAllocator::malloc(%u): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->allocatorConfiguration->defaultBufferSize));
          goto error;
        } // end IF
        ACE_ASSERT (message_block_p->space () >= trailing_bytes);
      } // end IF

      result_2 = message_block_p->copy ((char*)data_2, trailing_bytes);
      if (unlikely (result_2 == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%u): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    trailing_bytes));
        goto error;
      } // end IF
      data_2 += trailing_bytes; offset_i += trailing_bytes;
    } // end IF
    else
    {
      /* skip over all other chunks */
      data_2 += chunk_size; offset_i += chunk_size;
    } // end ELSE
  } // end WHILE

  // step3: push data downstream
  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  result = stream_->BindToFile (ACE_TEXT_ALWAYS_WCHAR (tempFilePath_.c_str ()),
                                SPFM_CREATE_ALWAYS,
                                &format_.FormatId (),
                                format_.WaveFormatExPtr (),
                                SPFEI_ALL_EVENTS);
  ACE_ASSERT (SUCCEEDED (result));

  return;

error:
  if (data_p)
    delete [] data_p;
  if (message_block_p)
    message_block_p->release ();

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
Stream_Decoder_SAPIDecoder_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataContainerType,
                             MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_SAPIDecoder_T::handleSessionMessage"));

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
      struct _AMMediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
      struct tWAVEFORMATEX waveformatex_s;
      ACE_OS::memset (&waveformatex_s, 0, sizeof (struct tWAVEFORMATEX));
      waveformatex_s.wFormatTag = WAVE_FORMAT_PCM;
      waveformatex_s.nChannels = 1;
      waveformatex_s.nSamplesPerSec = 22000;
      waveformatex_s.wBitsPerSample = 16;
      waveformatex_s.nBlockAlign =
        (waveformatex_s.nChannels * (waveformatex_s.wBitsPerSample / 8));
      waveformatex_s.nAvgBytesPerSec =
        (waveformatex_s.nSamplesPerSec * waveformatex_s.nBlockAlign);
      // waveformatex_s.cbSize = 0;
      Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (waveformatex_s,
                                                                media_type_2);
      ACE_OS::memset (&media_type, 0, sizeof (MediaType));
      inherited2::getMediaType (media_type_2,
                                STREAM_MEDIATYPE_AUDIO,
                                media_type);
      session_data_r.formats.push_back (media_type);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);

      break;

//error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
//    case STREAM_SESSION_MESSAGE_RESIZE:
//    {
//      break;
//    }
    case STREAM_SESSION_MESSAGE_END:
    { ACE_ASSERT (inherited::configuration_);

      break;
    }
    default:
      break;
  } // end SWITCH
}
