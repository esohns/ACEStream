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
          typename MediaType>
Stream_Decoder_VorbisDecoder_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::Stream_Decoder_VorbisDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , isFirstInput_ (true)
 , isFirstOutput_ (true)
 , messageData_ (NULL)
 , sessionId_ (0)
 , sync_ ()
 , page_ ()
 , packetNumber_ (0)
 , serialNumber_ (0)
 , stream_ ()
 , streamInitialized_ (false)
 , block_ ()
 , comment_ ()
 , info_ ()
 , state_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoder_T::Stream_Decoder_VorbisDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_VorbisDecoder_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::~Stream_Decoder_VorbisDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoder_T::~Stream_Decoder_VorbisDecoder_T"));

  if (unlikely (messageData_))
    messageData_->decrease ();

  // OGG bits
  ogg_stream_clear (&stream_);
  ogg_sync_clear (&sync_);

  // Vorbis bits
  vorbis_block_clear (&block_);
  vorbis_comment_clear (&comment_);
  vorbis_info_clear (&info_);
  vorbis_dsp_clear (&state_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_VorbisDecoder_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    isFirstInput_ = true;
    isFirstOutput_ = true;
    if (unlikely (messageData_))
    {
      messageData_->decrease (); messageData_ = NULL;
    } // end IF
    sessionId_ = 0;

    ogg_sync_clear (&sync_);
    packetNumber_ = 0;
    serialNumber_ = 0;
    ogg_stream_clear (&stream_);
    streamInitialized_ = false;

    vorbis_block_clear (&block_);
    vorbis_comment_clear (&comment_);
    vorbis_info_clear (&info_);
    vorbis_dsp_clear (&state_);
  } // end IF

  ogg_sync_init (&sync_);

  vorbis_comment_init (&comment_);
  vorbis_info_init (&info_);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_VorbisDecoder_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoder_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  ACE_Message_Block* message_block_p = message_inout;
  DataMessageType* message_p = NULL;
  int result;
  char* buffer_p = NULL;

  // retain the initial message data
  if (unlikely (isFirstInput_))
  { isFirstInput_ = false;
    ACE_ASSERT (!messageData_);
    messageData_ =
      &const_cast<DataMessageType::DATA_T&> (message_inout->getR ());
    ACE_ASSERT (messageData_);
    messageData_->increase ();
    sessionId_ = message_inout->sessionId ();
  } // end IF

  do
  { ACE_ASSERT (message_block_p);
    if (unlikely (!message_block_p->length ()))
      goto continue_;
    ACE_ASSERT (!buffer_p);
    buffer_p =
      ogg_sync_buffer (&sync_,
                       static_cast<long> (message_block_p->length ()));
    ACE_ASSERT (buffer_p);
    ACE_OS::memcpy (buffer_p, message_block_p->rd_ptr (), message_block_p->length ());
    result = ogg_sync_wrote (&sync_, static_cast<long> (message_block_p->length ()));
    ACE_ASSERT (result == 0);

    do
    {
      result = ogg_sync_pageout (&sync_, &page_);
      if (result == 0)
        break;
      else if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ogg_sync_pageout(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      
      if (unlikely (!streamInitialized_))
      { ACE_ASSERT (!serialNumber_);
        serialNumber_ = ogg_page_serialno (&page_);
        ACE_ASSERT (serialNumber_ != -1);
        result = ogg_stream_init (&stream_,
                                  serialNumber_);
        ACE_ASSERT (result == 0);
        streamInitialized_ = true;
      } // end IF

      if (unlikely (ogg_page_serialno (&page_) != serialNumber_))
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: skipping page from foreign stream...\n"),
                    inherited::mod_->name ()));
        continue;
      } // end IF

      result = ogg_stream_pagein (&stream_, &page_);
      ACE_ASSERT (result == 0);
     
      do
      {
        ogg_packet packet_s;
        result = ogg_stream_packetout (&stream_, &packet_s);
        if (result == 0)
          break;
        else if (result < 0)
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: failed to ogg_stream_packetout(), continuing\n"),
                      inherited::mod_->name ()));
          continue;
          /*goto error*/;
        } // end IF

        if (unlikely (packetNumber_ < 3))
        {
          bool continue_b = false;
          switch (packetNumber_)
          {
            case 0:
            {
              if (!vorbis_synthesis_idheader (&packet_s))
              {
                //ACE_DEBUG ((LM_DEBUG,
                //            ACE_TEXT ("%s: failed to vorbis_synthesis_idheader(), continuing\n"),
                //            inherited::mod_->name ()));
                continue_b = true;
                break;
              } // end IF
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("%s: 1...\n"),
                          inherited::mod_->name ()));

              //packet_s.b_o_s = 1;
              break;
            }
            case 1:
            {
              if (unlikely (packet_s.packet[0] != 0x03))
              {
                ACE_DEBUG ((LM_WARNING,
                            ACE_TEXT ("%s: 2: invalid magic: 0x%02X, restarting\n"),
                            inherited::mod_->name (),
                            packet_s.packet[0]));
                ogg_sync_reset (&sync_);
                //vorbis_synthesis_restart (&state_);
                continue_b = true;
                break;
              } // end IF
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("%s: 2...\n"),
                          inherited::mod_->name ()));
              break;
            }
            default:
            { ACE_ASSERT (packetNumber_ == 2);
              ACE_DEBUG ((LM_DEBUG,
                          ACE_TEXT ("%s: 3...\n"),
                          inherited::mod_->name ()));
              break;
            }
          } // end SWITCH
          if (unlikely (continue_b))
            continue;
          result = vorbis_synthesis_headerin (&info_,
                                              &comment_,
                                              &packet_s);
          if (unlikely (result < 0))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to vorbis_synthesis_headerin(): %d, aborting\n"),
                        inherited::mod_->name (),
                        result));
            goto error;
          } // end IF
          ++packetNumber_;
          if (unlikely (packetNumber_ == 3))
          {
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: initialized: %d channel(s) @ %d Hz\n"),
                        inherited::mod_->name (),
                        info_.channels,
                        info_.rate));

            result = vorbis_synthesis_init (&state_, &info_);
            if (unlikely (result < 0))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to vorbis_synthesis_init(), aborting\n"),
                          inherited::mod_->name ()));
              goto error;
            } // end IF

            result = vorbis_block_init (&state_, &block_);
            if (unlikely (result < 0))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to vorbis_block_init(), aborting\n"),
                          inherited::mod_->name ()));
              goto error;
            } // end IF
          } // end IF
        } // end IF
        else
        {
          result = vorbis_synthesis (&block_, &packet_s);
          if (unlikely (result < 0))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to vorbis_synthesis(), aborting\n"),
                        inherited::mod_->name ()));
            goto error;
          } // end IF
          //++packetNumber_;

          result = vorbis_synthesis_blockin (&state_, &block_);
          if (unlikely (result < 0))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to vorbis_synthesis_blockin(), aborting\n"),
                        inherited::mod_->name ()));
            goto error;
          } // end IF

          float** pcm_p;
          int samples_i;
          float* data_p;
          do
          {
            samples_i = vorbis_synthesis_pcmout (&state_, &pcm_p);
            if (samples_i <= 0)
              break;

            ACE_ASSERT (!message_p);
            message_p = inherited::allocateMessage (samples_i * info_.channels * sizeof (float),
                                                    NULL);
            if (unlikely (!message_p))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%B), aborting\n"),
                          inherited::mod_->name (),
                          samples_i * info_.channels * sizeof (float)));
              goto error;
            } // end IF

            // need to interleave the samples :-(
            data_p = reinterpret_cast<float*> (message_p->wr_ptr ());
            for (int i = 0; i < samples_i; i++)
              for (int j = 0; j < info_.channels; j++)
                data_p[i * info_.channels + j] = pcm_p[j][i];
            message_p->wr_ptr (samples_i * info_.channels * sizeof (float));

            result = vorbis_synthesis_read (&state_, samples_i);
            if (unlikely (result < 0))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to vorbis_synthesis_read(), aborting\n"),
                          inherited::mod_->name ()));
              message_p->release (); message_p = NULL;
              goto error;
            } // end IF

            if (unlikely (isFirstOutput_))
            { isFirstOutput_ = false;
              ACE_ASSERT (messageData_);
              message_p->initialize (messageData_,
                                     sessionId_,
                                     NULL);
              ACE_ASSERT (!messageData_);
            } // end IF

            result = inherited::put_next (message_p, NULL);
            if (unlikely (result == -1))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                          inherited::mod_->name ()));
              message_p->release (); message_p = NULL;
              goto error;
            } // end IF
            message_p = NULL;
          } while (true);
        } // end ELSE
      } while (true);
    } while (true);
    buffer_p = NULL;
continue_:
    message_block_p = message_block_p->cont ();
  } while (message_block_p);
  message_inout->release (); message_inout = NULL;

  return;

error:
  if (message_inout)
  {
    message_inout->release (); message_inout = NULL;
  } // end IF

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_VorbisDecoder_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  int error_i;
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    { // sanity check(s)

      goto continue_2;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_2:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    { // sanity check(s)

      break;
    }
    default:
      break;
  } // end SWITCH
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Decoder_VorbisDecoderH_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                UserDataType,
                                MediaType>::Stream_Decoder_VorbisDecoderH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , sync_ ()
 , page_ ()
 , packetNumber_ (0)
 , serialNumber_ (0)
 , stream_ ()
 , streamInitialized_ (false)
 , block_ ()
 , comment_ ()
 , info_ ()
 , state_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoderH_T::Stream_Decoder_VorbisDecoderH_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Decoder_VorbisDecoderH_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                UserDataType,
                                MediaType>::~Stream_Decoder_VorbisDecoderH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoderH_T::~Stream_Decoder_VorbisDecoderH_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Decoder_VorbisDecoderH_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                UserDataType,
                                MediaType>::initialize (const ConfigurationType& configuration_in,
                                                        Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoderH_T::initialize"));

  if (inherited::isInitialized_)
  {
    ogg_sync_clear (&sync_);
    packetNumber_ = 0;
    serialNumber_ = 0;
    ogg_stream_clear (&stream_);
    streamInitialized_ = false;

    vorbis_block_clear (&block_);
    vorbis_comment_clear (&comment_);
    vorbis_info_clear (&info_);
    vorbis_dsp_clear (&state_);
  } // end IF

  ogg_sync_init (&sync_);

  vorbis_comment_init (&comment_);
  vorbis_info_init (&info_);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
int
Stream_Decoder_VorbisDecoderH_T<ACE_SYNCH_USE,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType,
                                ConfigurationType,
                                StreamControlType,
                                StreamNotificationType,
                                StreamStateType,
                                StatisticContainerType,
                                SessionManagerType,
                                TimerManagerType,
                                UserDataType,
                                MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_VorbisDecoderH_T::svc"));

  int result = -1;
  int result_2 = -1;
  int error = 0;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  int message_type = -1;
  DataMessageType* message_p = NULL;
  bool done_b = false;
  bool stop_processing_b = false;
  std::string file_path_string;
  int encoding_i = 0, channels_i = 0;
  long rate_l = 0;
  int error_i = 0;
  size_t done_u = 0;
  MediaType media_type_s;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&media_type_s, 0, sizeof (MediaType));
  struct _AMMediaType media_type_2;
  struct tWAVEFORMATEX format_s;
  HRESULT result_3 = E_FAIL;
#else
  struct Stream_MediaFramework_ALSA_MediaType media_type_2;
#endif // ACE_WIN32 || ACE_WIN64
  typename SessionMessageType::DATA_T* session_data_container_p = NULL;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

//next:
  file_path_string = inherited::configuration_->fileIdentifier.identifier;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: processing file \"%s\" (%Q byte(s))\n"),
              inherited::mod_->name (),
              ACE_TEXT (file_path_string.c_str ()),
              Common_File_Tools::size (file_path_string)));
  error_i = -1;
  if (unlikely (error_i != 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to  mpg123_open(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (file_path_string.c_str ())));
    inherited::change (STREAM_STATE_SESSION_STOPPING);
    return -1;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
  ACE_OS::memset (&format_s, 0, sizeof (struct tWAVEFORMATEX));
  format_s.wFormatTag = WAVE_FORMAT_PCM;
  format_s.nChannels = channels_i;
  format_s.nSamplesPerSec = rate_l;
  format_s.wBitsPerSample = mpg123_encsize (encoding_i) * 8;
  format_s.nBlockAlign = (format_s.nChannels * (format_s.wBitsPerSample / 8));
  format_s.nAvgBytesPerSec = (format_s.nSamplesPerSec * format_s.nBlockAlign);
  if (!Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (format_s,
                                                                 media_type_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx(), aborting\n"),
                inherited::mod_->name ()));
    inherited::change (STREAM_STATE_SESSION_STOPPING);
    goto continue_;
  } // end IF
#else
  media_type_2.format = SND_PCM_FORMAT_FLOAT;
  media_type_2.channels = channels_i;
  media_type_2.rate = rate_l;
#endif // ACE_WIN32 || ACE_WIN64
  inherited2::getMediaType (media_type_2,
                            STREAM_MEDIATYPE_AUDIO,
                            media_type_s);
  session_data_r.formats.push_back (media_type_s);
  session_data_r.sourceFileName = file_path_string;
  session_data_container_p = inherited::sessionData_->clone ();
  ACE_ASSERT (session_data_container_p);
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP,
                                               session_data_container_p,
                                               inherited::streamState_->userData,
                                               false))) // expedited ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), aborting\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STEP));
    inherited::change (STREAM_STATE_SESSION_STOPPING);
    goto continue_;
  } // end IF

  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p,
                                &no_wait);
    if (result_2 == -1)
    { error = ACE_OS::last_error ();
      if (unlikely (error != EWOULDBLOCK)) // Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));

        if (unlikely (inherited::current () != STREAM_STATE_FINISHED))
        {
          inherited::change (STREAM_STATE_SESSION_STOPPING);
          message_block_p->release (); message_block_p = NULL;
          continue;
        } // end IF

        break;
      } // end IF
    } // end IF

    if (message_block_p)
    {
      message_type = message_block_p->msg_type ();
      switch (message_type)
      {
        case ACE_Message_Block::MB_STOP:
        {
          if (unlikely (inherited::isHighPriorityStop_))
          {
            if (likely (!inherited::abortSent_))
              inherited::control (STREAM_CONTROL_ABORT,
                                  false, // forward upstream ?
                                  true); // expedited ?
          } // end IF

          bool finish_b = false;
          { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
            if (unlikely (!inherited::sessionEndSent_ && !inherited::sessionEndProcessed_))
              finish_b = true;
          } // end lock scope
          if (unlikely (finish_b))
          {
            message_block_p->release (); message_block_p = NULL;
            inherited::finished (); // enqueue SESSION_END and continue
            continue;
          } // end IF

          // *NOTE*: this is racy; the penultimate thread may have left svc() and
          //         not have decremented thr_count_ yet. In this case, the
          //         stop-message might remain in the queue during shutdown (or,
          //         even worse-) during re-initialization...
          // *TODO*: ward against this scenario
          if (unlikely (inherited::thr_count_ > 1))
          {
            result_2 =
              (inherited::isHighPriorityStop_ ? inherited::ungetq (message_block_p, NULL)
                                              : inherited::putq (message_block_p, NULL));
            if (unlikely (result_2 == -1))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task::putq(): \"%m\", aborting\n"),
                          inherited::mod_->name ()));
              message_block_p->release (); message_block_p = NULL;
              done_b = true;
              break;
            } // end IF
          } // end IF
          else
          {
            message_block_p->release (); message_block_p = NULL;
          } // end ELSE
          inherited::isHighPriorityStop_ = false;

          // --> SESSION_END has been processed; leave
          done_b = true;
          result = 0;
          break;
        }
        default:
          break;
      } // end SWITCH
      // sanity check(s)
      if (unlikely (done_b))
        break;

      // process manually
      inherited::handleMessage (message_block_p,
                                stop_processing_b);
      if (unlikely (stop_processing_b)) // <-- SESSION_END has been processed || finished || serious error
      { stop_processing_b = false; // reset, just in case...
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (unlikely (!inherited::sessionEndSent_ && !inherited::sessionEndProcessed_))
          {
            inherited::change (STREAM_STATE_SESSION_STOPPING);
            continue;
          } // end IF
        } // end lock scope
      } // end IF

      continue; // there was a message --> retry until idle
    } // end IF

    // session aborted ?
    if (unlikely (session_data_r.aborted))
    {
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        if (!inherited::sessionEndSent_ && !inherited::sessionEndProcessed_)
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: session (id was: %u) aborted\n"),
                      inherited::mod_->name (),
                      session_data_r.sessionId));
          inherited::change (STREAM_STATE_SESSION_STOPPING);
        } // end IF
      } // end lock scope
      continue;
    } // end IF

    // *TODO*: remove type inference
    message_p =
      inherited::allocateMessage (static_cast<unsigned int> (0),
                                  NULL);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                  inherited::mod_->name (),
                  0));
      inherited::change (STREAM_STATE_SESSION_STOPPING);
      continue;
    } // end IF

    done_u = 0;
    error_i = -1;
    switch (error_i)
    {
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to mpg123_read(\"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (file_path_string.c_str ())));
        message_p->release (); message_p = NULL;
        inherited::change (STREAM_STATE_SESSION_STOPPING);
        continue;
      }
    } // end SWITCH
  } while (true);

continue_:
  error_i = -1;
  if (unlikely (error_i != 0))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mpg123_close(), continuing\n"),
                inherited::mod_->name ()));

  return result;
}
