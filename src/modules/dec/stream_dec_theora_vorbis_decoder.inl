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
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::Stream_Decoder_TheoraVorbisDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , isFirstInput_ (true)
 , isFirstTheoraOutput_ (true)
 , isFirstVorbisOutput_ (true)
 , messageData_ (NULL)
 , sessionId_ (0)
 , sync_ ()
 , page_ ()
 , streams_ ()
 , theoraInfo_ ()
 , theoraComment_ ()
 , theoraContext_ (NULL)
 , theoraSetupInfo_ (NULL)
 , vorbisBlock_ ()
 , vorbisComment_ ()
 , vorbisInfo_ ()
 , vorbisState_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::Stream_Decoder_TheoraVorbisDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::~Stream_Decoder_TheoraVorbisDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::~Stream_Decoder_TheoraVorbisDecoder_T"));

  if (unlikely (messageData_))
    messageData_->decrease ();

  // OGG bits
  for (OggStreamMapIterator_t iterator = streams_.begin ();
       iterator != streams_.end ();
       ++iterator)
    ogg_stream_clear (&(*iterator).second.state);
  ogg_sync_clear (&sync_);

  // Vorbis bits
  vorbis_block_clear (&vorbisBlock_);
  vorbis_comment_clear (&vorbisComment_);
  vorbis_info_clear (&vorbisInfo_);
  vorbis_dsp_clear (&vorbisState_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::initialize (const ConfigurationType& configuration_in,
                                                             Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    isFirstInput_ = true;
    isFirstTheoraOutput_ = true;
    isFirstVorbisOutput_ = true;
    if (unlikely (messageData_))
    {
      messageData_->decrease (); messageData_ = NULL;
    } // end IF
    sessionId_ = 0;

    ogg_sync_clear (&sync_);
    for (OggStreamMapIterator_t iterator = streams_.begin ();
         iterator != streams_.end (); ++iterator)
      ogg_stream_clear (&(*iterator).second.state);
    streams_.clear ();

    th_comment_clear (&theoraComment_);
    th_info_clear (&theoraInfo_);
    if (theoraSetupInfo_)
    {
      th_setup_free (theoraSetupInfo_); theoraSetupInfo_ = NULL;
    } // end IF
    if (theoraContext_)
    {
      th_decode_free (theoraContext_); theoraContext_ = NULL;
    } // end IF

    vorbis_block_clear (&vorbisBlock_);
    vorbis_comment_clear (&vorbisComment_);
    vorbis_info_clear (&vorbisInfo_);
    vorbis_dsp_clear (&vorbisState_);
  } // end IF

  ogg_sync_init (&sync_);

  th_comment_init (&theoraComment_);
  th_info_init (&theoraInfo_);

  vorbis_comment_init (&vorbisComment_);
  vorbis_info_init (&vorbisInfo_);

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
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  ACE_Message_Block* message_block_p = message_inout;
  DataMessageType* message_p = NULL;
  int result, serial_number_i;
  char* buffer_p = NULL;
  OggStreamMapIterator_t iterator;

  if (unlikely (isFirstInput_))
  { isFirstInput_ = false;
    // step1: retain the initial message data
    ACE_ASSERT (!messageData_);
    messageData_ =
      &const_cast<typename DataMessageType::DATA_T&> (message_inout->getR ());
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
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: failed to ogg_sync_pageout(), continuing\n"),
                    inherited::mod_->name ()));
        continue;
      } // end IF
      serial_number_i = ogg_page_serialno (&page_);
      iterator = streams_.find (serial_number_i);
      if (unlikely (iterator == streams_.end ()))
      {
        OggStream_t ogg_stream;
        ACE_OS::memset (&ogg_stream, 0, sizeof (OggStream_t));
        ogg_stream.type = STREAM_MEDIATYPE_INVALID;
        result = ogg_stream_init (&ogg_stream.state,
                                  serial_number_i);
        ACE_ASSERT (result == 0);
        streams_.insert (std::make_pair (serial_number_i, ogg_stream));
        iterator = streams_.find (serial_number_i);
      } // end IF
      ACE_ASSERT (iterator != streams_.end ());

      result = ogg_stream_pagein (&((*iterator).second.state), &page_);
      ACE_ASSERT (result == 0);
     
      do
      {
        ogg_packet packet_s;
        result = ogg_stream_packetout (&((*iterator).second.state), &packet_s);
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

        if (unlikely ((*iterator).second.type == STREAM_MEDIATYPE_INVALID))
        {
          if (th_decode_headerin (&theoraInfo_,
                                  &theoraComment_,
                                  &theoraSetupInfo_,
                                  &packet_s) >= 0)
          {
            (*iterator).second.type = STREAM_MEDIATYPE_VIDEO;
            (*iterator).second.packetNumber++;
            continue;
          } // end IF
          else if (vorbis_synthesis_headerin (&vorbisInfo_,
                                              &vorbisComment_,
                                              &packet_s) == 0)
          {
            (*iterator).second.type = STREAM_MEDIATYPE_AUDIO;
            (*iterator).second.packetNumber++;
            continue;
          } // end ELSE IF
          else
          {
            ACE_DEBUG ((LM_WARNING,
                        ACE_TEXT ("%s: unknown stream type (maybe Opus ?), continuing\n"),
                        inherited::mod_->name ()));
            continue;
          } // end ELSE
        } // end IF

        switch ((*iterator).second.type)
        {
          case STREAM_MEDIATYPE_AUDIO:
          {
            if (unlikely ((*iterator).second.packetNumber < 3))
            {
              result = vorbis_synthesis_headerin (&vorbisInfo_,
                                                  &vorbisComment_,
                                                  &packet_s);
              if (unlikely (result < 0))
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to vorbis_synthesis_headerin(): %d, aborting\n"),
                            inherited::mod_->name (),
                            result));
                goto error;
              } // end IF
              (*iterator).second.packetNumber++;
              if (unlikely ((*iterator).second.packetNumber == 3))
              {
                ACE_DEBUG ((LM_DEBUG,
                            ACE_TEXT ("%s: initialized: %d channel(s) @ %d Hz\n"),
                            inherited::mod_->name (),
                            vorbisInfo_.channels, vorbisInfo_.rate));

                result = vorbis_synthesis_init (&vorbisState_,
                                                &vorbisInfo_);
                if (unlikely (result < 0))
                {
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: failed to vorbis_synthesis_init(), aborting\n"),
                              inherited::mod_->name ()));
                  goto error;
                } // end IF

                result = vorbis_block_init (&vorbisState_,
                                            &vorbisBlock_);
                if (unlikely (result < 0))
                {
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: failed to vorbis_block_init(), aborting\n"),
                              inherited::mod_->name ()));
                  goto error;
                } // end IF
                (*iterator).second.initialized = true;
              } // end IF
            } // end IF
            else if (unlikely (!processVorbisPacket (packet_s)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to processVorbisPacket(), aborting\n"),
                          inherited::mod_->name ()));
              goto error;
            } // end ELSE IF

            break;
          }
          case STREAM_MEDIATYPE_VIDEO:
          {
            if (unlikely ((*iterator).second.packetNumber < 3))
            {
              result = th_decode_headerin (&theoraInfo_,
                                           &theoraComment_,
                                           &theoraSetupInfo_,
                                           &packet_s);
              if (unlikely (result < 0))
              {
                ACE_DEBUG ((LM_ERROR,
                            ACE_TEXT ("%s: failed to th_decode_headerin(): %d, aborting\n"),
                            inherited::mod_->name (),
                            result));
                goto error;
              } // end IF
              (*iterator).second.packetNumber++;
              if (unlikely ((*iterator).second.packetNumber == 3))
              { ACE_ASSERT (theoraSetupInfo_);
                theoraContext_ = th_decode_alloc (&theoraInfo_,
                                                  theoraSetupInfo_);
                if (unlikely (!theoraContext_))
                {
                  ACE_DEBUG ((LM_ERROR,
                              ACE_TEXT ("%s: failed to th_decode_alloc(), aborting\n"),
                              inherited::mod_->name ()));
                  goto error;
                } // end IF
                ACE_DEBUG ((LM_DEBUG,
                            ACE_TEXT ("%s: initialized theora\n"),
                            inherited::mod_->name ()));
                th_setup_free (theoraSetupInfo_); theoraSetupInfo_ = NULL;
                (*iterator).second.initialized = true;
              } // end IF
            } // end IF
            else if (unlikely (!processTheoraPacket (packet_s)))
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to processTheoraPacket(), aborting\n"),
                          inherited::mod_->name ()));
              goto error;
            } // end ELSE IF

            break;
          }
          default:
          {
            ACE_DEBUG ((LM_WARNING,
                        ACE_TEXT ("%s: invalid/unknown stream type (was: %d), continuing\n"),
                        inherited::mod_->name (),
                        (*iterator).second.type));
            break;
          }
        } // end SWITCH
      } while (true);
    } while (true);
    buffer_p = NULL;
continue_:
    message_block_p = message_block_p->cont ();
  } while (message_block_p);

continue_2:
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
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::handleSessionMessage"));

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

      if (messageData_)
      {
        messageData_->decrease (); messageData_ = NULL;
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
          typename MediaType>
bool
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::processVorbisPacket (ogg_packet& packet_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::processVorbisPacket"));

  int result = vorbis_synthesis (&vorbisBlock_, &packet_in);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to vorbis_synthesis(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  result = vorbis_synthesis_blockin (&vorbisState_, &vorbisBlock_);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to vorbis_synthesis_blockin(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  float** pcm_p;
  int samples_i;
  float* data_p;
  DataMessageType* message_p = NULL;
  do
  { pcm_p = NULL;
    samples_i = vorbis_synthesis_pcmout (&vorbisState_, &pcm_p);
    if (samples_i <= 0)
      break;
    ACE_ASSERT (pcm_p);

    ACE_ASSERT (!message_p);
    message_p = inherited::allocateMessage (samples_i * vorbisInfo_.channels * sizeof (float),
                                            NULL);
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%B), aborting\n"),
                  inherited::mod_->name (),
                  samples_i * vorbisInfo_.channels * sizeof (float)));
      return false;
    } // end IF
    message_p->setMediaType (STREAM_MEDIATYPE_AUDIO);

    // need to interleave the samples :-(
    data_p = reinterpret_cast<float*> (message_p->wr_ptr ());
    for (int i = 0; i < samples_i; i++)
      for (int j = 0; j < vorbisInfo_.channels; j++)
        data_p[i * vorbisInfo_.channels + j] = pcm_p[j][i];
    message_p->wr_ptr (samples_i * vorbisInfo_.channels * sizeof (float));

    result = vorbis_synthesis_read (&vorbisState_, samples_i);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to vorbis_synthesis_read(), aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      return false;
    } // end IF

    if (unlikely (isFirstVorbisOutput_))
    { isFirstVorbisOutput_ = false;
      ACE_ASSERT (messageData_);
      typename DataMessageType::DATA_T* message_data_p = messageData_;
      message_data_p->increase ();
      message_p->initialize (message_data_p,
                             sessionId_,
                             NULL);
      ACE_ASSERT (!message_data_p);
    } // end IF

    result = inherited::put_next (message_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_p->release (); message_p = NULL;
      return false;
    } // end IF
    message_p = NULL;
  } while (true);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_TheoraVorbisDecoder_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     MediaType>::processTheoraPacket (ogg_packet& packet_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoder_T::processTheoraPacket"));

  ACE_ASSERT (theoraContext_);

  ogg_int64_t granulepos;
  int result = th_decode_packetin (theoraContext_,
                                   &packet_in,
                                   &granulepos);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to th_decode_packetin(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  th_ycbcr_buffer yuv;
  result = th_decode_ycbcr_out (theoraContext_, yuv);
  if (unlikely (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to th_decode_ycbcr_out(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // Note: Use yuv[0].width/height instead of th_info if dealing with cropped frame logic
  // 1. Get visible dimensions vs. internal buffer dimensions
  int visible_w = theoraInfo_.pic_width;
  int visible_h = theoraInfo_.pic_height;

  // Determine chroma downsampling ratios based on pixel format
  int x_dec = 1;
  int y_dec = 1;
  switch (theoraInfo_.pixel_fmt)
  {
    case TH_PF_420:
    {
      x_dec = 2; // Chroma width is half of luma
      y_dec = 2; // Chroma height is half of luma
      break;
    }
    case TH_PF_422:
    {
      x_dec = 2; // Chroma width is half
      y_dec = 1; // Chroma height is full
      break;
    }
    case TH_PF_444:
    {
      x_dec = 1; // Chroma width is full
      y_dec = 1; // Chroma height is full
      break;
    }
    case TH_PF_RSVD:
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown pixel format (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  theoraInfo_.pixel_fmt));
      return false;
    }
  } // end SWITCH

  int chroma_w = visible_w / x_dec;
  int chroma_h = visible_h / y_dec;

  // 2. Calculate continuous packed memory sizing
  size_t y_size = (size_t)visible_w * visible_h;
  size_t chroma_size = (size_t)chroma_w * chroma_h;
  size_t buffer_size_i = y_size + (2 * chroma_size); // Y + Cb + Cr

  DataMessageType* message_p = inherited::allocateMessage (buffer_size_i,
                                                           NULL);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%B), aborting\n"),
                inherited::mod_->name (),
                buffer_size_i));
    return false;
  } // end IF
  message_p->setMediaType (STREAM_MEDIATYPE_VIDEO);

  char* src_y =
    reinterpret_cast<char*> (yuv[0].data) + (theoraInfo_.pic_y * yuv[0].stride) + theoraInfo_.pic_x;
  char* src_cb =
    reinterpret_cast<char*> (yuv[1].data) + ((theoraInfo_.pic_y / y_dec) * yuv[1].stride) + (theoraInfo_.pic_x / x_dec);
  char* src_cr =
    reinterpret_cast<char*> (yuv[2].data) + ((theoraInfo_.pic_y / y_dec) * yuv[2].stride) + (theoraInfo_.pic_x / x_dec);
  // copy the Y (Luminance) Plane row-by-row to safely drop stride padding
  for (int i = 0; i < visible_h; i++)
  {
    result = message_p->copy (src_y, visible_w);
    ACE_ASSERT (result == 0);
    src_y += yuv[0].stride; // Step over the stride padding to the next row start
  } // end FOR

  // copy the Cb (Chroma Blue) Plane row-by-row
  for (int i = 0; i < chroma_h; i++)
  {
    result = message_p->copy (src_cb, chroma_w);
    ACE_ASSERT (result == 0);
    src_cb += yuv[1].stride;
  } // end FOR

  // copy the Cr (Chroma Red) Plane row-by-row
  for (int i = 0; i < chroma_h; i++)
  {
    result = message_p->copy (src_cr, chroma_w);
    ACE_ASSERT (result == 0);
    src_cr += yuv[2].stride;
  } // end FOR

  if (unlikely (isFirstTheoraOutput_))
  { isFirstTheoraOutput_ = false;
    ACE_ASSERT (messageData_);
    typename DataMessageType::DATA_T* message_data_p = messageData_;
    message_data_p->increase ();
    message_p->initialize (message_data_p,
                           sessionId_,
                           NULL);
    ACE_ASSERT (!message_data_p);
  } // end IF

  result = inherited::put_next (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    return false;
  } // end IF

  return true;
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
Stream_Decoder_TheoraVorbisDecoderH_T<ACE_SYNCH_USE,
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
                                      MediaType>::Stream_Decoder_TheoraVorbisDecoderH_T (typename inherited::ISTREAM_T* stream_in)
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoderH_T::Stream_Decoder_TheoraVorbisDecoderH_T"));

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
Stream_Decoder_TheoraVorbisDecoderH_T<ACE_SYNCH_USE,
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
                                      MediaType>::~Stream_Decoder_TheoraVorbisDecoderH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoderH_T::~Stream_Decoder_TheoraVorbisDecoderH_T"));

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
Stream_Decoder_TheoraVorbisDecoderH_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoderH_T::initialize"));

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
Stream_Decoder_TheoraVorbisDecoderH_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_TheoraVorbisDecoderH_T::svc"));

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
