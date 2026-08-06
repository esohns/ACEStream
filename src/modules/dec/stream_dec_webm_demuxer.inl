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

#include "common_image_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::Stream_Decoder_WebM_Demuxer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , context_ (NULL)
 , CBData_ ()
 , queue_ (0,    // max # slots; 0 --> unlimited
           NULL) // notification handle
 , trackIndexToMessageMediaType_ ()
 , audioTrackIndex_ (-1)
 , videoTrackIndex_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::Stream_Decoder_WebM_Demuxer_T"));

  CBData_.queue = &queue_;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::~Stream_Decoder_WebM_Demuxer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::~Stream_Decoder_WebM_Demuxer_T"));

  if (CBData_.buffer)
    CBData_.buffer->release ();

  if (context_)
    nestegg_destroy (context_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (context_)
    {
      nestegg_destroy (context_); context_ = NULL;
    } // end IF

    if (CBData_.buffer)
    {
      CBData_.buffer->release (); CBData_.buffer = NULL;
    } // end IF

    queue_.activate ();
    queue_.flush (true);

    trackIndexToMessageMediaType_.clear ();

    audioTrackIndex_ = -1;
    videoTrackIndex_ = -1;
  } // end IF
  ACE_ASSERT (!context_);

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
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  int result = queue_.enqueue (message_inout,
                               NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (unlikely (error != ESHUTDOWN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    goto error;
  } // end IF
  message_inout = NULL;

  return;

error:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      goto end;

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      int result = inherited::activate ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_T::activate(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      goto continue_;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      stop ();

      if (likely (inherited::thr_count_))
        inherited::wait (false); // wait for message queue ?

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
int
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (!context_);

  int result;
  DataMessageType* message_p = NULL;
  unsigned int tracks_i = 0, track_i = -1, chunks_i = 0;
  int track_type_i, track_codec_i;
  std::vector<unsigned int> track_ids_to_skip_a;
  nestegg_packet* packet_p = NULL;
  unsigned char* chunk_data_p;
  size_t chunk_size_i;
  static ACE_Time_Value backoff_timeout (STREAM_MESSAGE_ALLOCATION_SOURCE_BACKOFF_TIMEOUT_S, 0);
  nestegg_io io = { acestream_webm_demuxer_io_read_cb,
                    acestream_webm_demuxer_io_seek_cb,
                    acestream_webm_demuxer_io_tell_cb,
                    &CBData_ };
  int retries_i = 0;
  
retry:
  result = nestegg_init (&context_,
                         io,
#if defined (_DEBUG)
                         acestream_webm_demuxer_log_cb,
#else
                         NULL,
#endif // _DEBUG
                         -1); // max offset --> ignore
  if (unlikely (result < 0 || !context_))
  { --retries_i;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: nestegg_init() failed, %s\n"),
                inherited::mod_->name (),
                retries_i ? ACE_TEXT ("retrying") : ACE_TEXT ("aborting")));
    if (retries_i)
    {
      CBData_.position = 0;
      goto retry;
    } // end IF
    goto error;
  } // end IF
  ACE_ASSERT (context_);

  result = nestegg_track_count (context_, &tracks_i);
  ACE_ASSERT (!result);
  for (unsigned int i = 0; i < tracks_i; i++)
  {
    track_type_i = nestegg_track_type (context_, i);
    ACE_ASSERT (track_type_i != -1);
    track_codec_i = nestegg_track_codec_id (context_, i);
    ACE_ASSERT (track_codec_i != -1);

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: pre-processing track %u: type: %d; codec %d...\n"),
                inherited::mod_->name (),
                i,
                track_type_i, track_codec_i));

    switch (track_type_i)
    {
      case NESTEGG_TRACK_VIDEO:
      {
        trackIndexToMessageMediaType_[i] = STREAM_MEDIATYPE_VIDEO;
        if (videoTrackIndex_ == -1)
          videoTrackIndex_ = i;

        break;
      }
      case NESTEGG_TRACK_AUDIO:
      {
        trackIndexToMessageMediaType_[i] = STREAM_MEDIATYPE_AUDIO;
        if (audioTrackIndex_ == -1)
          audioTrackIndex_ = i;

        break;
      }
      default:
      {
        track_ids_to_skip_a.push_back (i);

        break;
      }
    } // end SWITCH
  } // end FOR

  do
  { ACE_ASSERT (!packet_p);
    result = nestegg_read_packet (context_, &packet_p);
    if (unlikely (result <= 0))
    {
      if (likely (result == 0))
        goto done; // EOF reached
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to nestegg_read_packet(), aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } // end IF
    ACE_ASSERT (packet_p);

    result = nestegg_packet_track (packet_p, &track_i);
    ACE_ASSERT (!result);
    if (std::find (track_ids_to_skip_a.begin (), track_ids_to_skip_a.end (), track_i) != track_ids_to_skip_a.end () ||
                   track_i != audioTrackIndex_)
    {
      nestegg_free_packet (packet_p); packet_p = NULL;
      continue;
    } // end IF

    result = nestegg_packet_count (packet_p, &chunks_i);
    ACE_ASSERT (!result);
    for (unsigned int i = 0; i < chunks_i; ++i)
    {
      chunk_data_p = NULL; chunk_size_i = 0;
      result = nestegg_packet_data (packet_p, i, &chunk_data_p, &chunk_size_i);
      ACE_ASSERT (!result && chunk_data_p && chunk_size_i);

      message_p =
        inherited::allocateMessage (chunk_size_i + inherited::configuration_->allocatorConfiguration->paddingBytes,
                                    &backoff_timeout);
      if (unlikely (!message_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                    inherited::mod_->name (),
                    chunk_size_i + inherited::configuration_->allocatorConfiguration->paddingBytes));
        nestegg_free_packet (packet_p); packet_p = NULL;
        goto error;
      } // end IF
      message_p->size (chunk_size_i);

      result = message_p->copy (reinterpret_cast<char*> (chunk_data_p),
                                chunk_size_i);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%B): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    chunk_size_i));
        nestegg_free_packet (packet_p); packet_p = NULL;
        message_p->release (); message_p = NULL;
        goto error;
      } // end IF
      message_p->setMediaType (trackIndexToMessageMediaType_[track_i]);

      result = inherited::put_next (message_p, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        nestegg_free_packet (packet_p); packet_p = NULL;
        message_p->release (); message_p = NULL;
        goto error;
      } // end IF
      message_p = NULL;
    } // end FOR
    nestegg_free_packet (packet_p); packet_p = NULL;
  } while (true);
  result = -1;

done:
  nestegg_destroy (context_); context_ = NULL;

  return result;

error:
  if (context_)
  {
    nestegg_destroy (context_); context_ = NULL;
  } // end IF

  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

  return -1;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_WebM_Demuxer_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              MediaType>::stop ()
{
  COMMON_TRACE (ACE_TEXT ("Stream_Decoder_WebM_Demuxer_T::stop"));

  // sanity check(s)
  if (unlikely (queue_.deactivated ()))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: queue inactive; cannot stop, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  ACE_Message_Block* message_block_p = NULL;
  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  int result = queue_.enqueue (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}
