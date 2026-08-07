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

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::Stream_Decoder_LibWebM_Demuxer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , absoluteHeadOffset_ (0)
 , buffer_ ()
 , condition_ (inherited::lock_, NULL, NULL)
 , finished_ (false)
 , isFirstSegment_ (true)
 , maxBufferCapacity_ (STREAM_DEC_WEBM_MAX_BUFFER_SIZE)
 , trackNumberToMessageMediaType_ ()
 , audioTrackNumber_ (-1)
 , videoTrackNumber_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_Demuxer_T::Stream_Decoder_LibWebM_Demuxer_T"));

  buffer_.reserve (maxBufferCapacity_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::~Stream_Decoder_LibWebM_Demuxer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_Demuxer_T::~Stream_Decoder_LibWebM_Demuxer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::initialize (const ConfigurationType& configuration_in,
                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_Demuxer_T::initialize"));

  if (inherited::isInitialized_)
  {
    absoluteHeadOffset_ = 0;
    buffer_.clear ();
    finished_ = false;
    isFirstSegment_ = true;

    trackNumberToMessageMediaType_.clear ();

    audioTrackNumber_ = -1;
    videoTrackNumber_ = -1;
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
          typename MediaType>
void
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_Demuxer_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  static ACE_Time_Value timeout (std::chrono::milliseconds (20));
  ACE_Time_Value absolute_timeout;
  size_t available_bytes_i;
  uint8_t* data_p;
  ACE_Message_Block* message_block_p = message_inout;

next:
  { ACE_ASSERT (message_block_p);
    available_bytes_i = message_block_p->length ();
    if (unlikely (!available_bytes_i))
      goto continue_;
    data_p = reinterpret_cast<uint8_t*> (message_block_p->rd_ptr ());

    { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
      //while (!finished_ &&
      //       (buffer_.size () + available_bytes_i > maxBufferCapacity_))
      //{
      //  absolute_timeout = COMMON_TIME_NOW + timeout;
      //  condition_.wait (&absolute_timeout);
      //} // end WHILE
      if (unlikely (finished_))
        goto clean;
      buffer_.insert (buffer_.end (), data_p, data_p + available_bytes_i);
      condition_.broadcast ();
    } // end lock scope

continue_:
    message_block_p = message_block_p->cont ();
    if (unlikely (message_block_p))
      goto next;
  } // end lock scope

clean:
  message_inout->release (); message_inout = NULL;

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
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_Demuxer_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      goto end;
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
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        finished_ = true;
        condition_.broadcast ();
      } // end lock scope

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
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::Read (long long position_in,
                                                   long length_in,
                                                   unsigned char* buffer_in)
{
  // sanity check(s)
  ACE_ASSERT (position_in >= 0 && length_in >= 0);

  static ACE_Time_Value sleep_timeout_10ms (std::chrono::milliseconds (10));
  //ACE_Time_Value absolute_timeout;
  size_t relative_offset_i;

  //{ ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    // sanity check(s)
    if (unlikely (position_in < absoluteHeadOffset_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: cannot satisfy request for data @%q, %d byte(s), aborting\n"),
                  inherited::mod_->name (),
                  position_in,
                  length_in));
      return -1;
    } // end IF

    //if (position_in + length_in > absoluteHeadOffset_ + static_cast<long long> (buffer_.size ()))
    //{
    //  if (finished_)
    //    return -1; // EOS
    //  return 1; // E_BUFFER_NOT_FULL
    //} // end IF
    while (position_in + length_in > absoluteHeadOffset_ + static_cast<long long> (buffer_.size ()))
    {
      if (unlikely (finished_))
        return -1;
      //absolute_timeout = COMMON_TIME_NOW + sleep_timeout_10ms;
      //condition_.wait (&absolute_timeout);
      ACE_OS::sleep (sleep_timeout_10ms);
    } // end WHILE

    relative_offset_i = static_cast<size_t> (position_in - absoluteHeadOffset_);
    ACE_OS::memcpy (buffer_in, &buffer_[relative_offset_i], length_in);
  //} // end lock scope

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::Length (long long* total_in,
                                                     long long* available_in)
{
  ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, 0);

  *available_in = absoluteHeadOffset_ + buffer_.size ();
  *total_in = -1; // -1 to denote a live streaming source

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_Demuxer_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);

  int result = -1;
  long long result_2;
  long result_3;
  DataMessageType* message_p = NULL;
  std::vector<long> track_numbers_to_skip_a;
  //static ACE_Time_Value backoff_timeout (STREAM_MESSAGE_ALLOCATION_SOURCE_BACKOFF_TIMEOUT_S, 0);
  //static ACE_Time_Value sleep_timeout_10ms (std::chrono::milliseconds (10));
  //static ACE_Time_Value sleep_timeout_50ms (std::chrono::milliseconds (50));
  long long position_i = 0;
  //mkvparser::EBMLHeader ebml_header;
  mkvparser::Segment* segment_p = NULL;
  const mkvparser::Tracks* tracks_p = NULL; 
  const mkvparser::Cluster* cluster_p = NULL;
  const mkvparser::BlockEntry* block_entry_p = NULL;
  unsigned long tracks_i = 0;
  long long track_i;
  const mkvparser::Track* track_p = NULL; 
  int frames_i;
  size_t codec_private_length_i;
  struct Stream_MediaFramework_SessionData_CodecConfiguration codec_configuration_s;
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  //result_2 = ebml_header.Parse (this, position_i);
  //while (result_2)
  //{
  //  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
  //    if (unlikely (finished_))
  //    {
  //      result = 0;
  //      goto clean;
  //    } // end IF
  //  } // end lock scope

  //  //ACE_OS::sleep (sleep_timeout_10ms);
  //  result_2 = ebml_header.Parse (this, position_i);
  //} // end WHILE
  //if (unlikely (result_2 < 0))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to mkvparser::EBMLHeader(): %q, aborting\n"),
  //              inherited::mod_->name (),
  //              result_2));
  //  goto error;
  //} // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: successfully parsed EBML header...\n"),
  //            inherited::mod_->name ()));

  //{ ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
  //  if (unlikely (finished_))
  //  {
  //    result = 0;
  //    goto clean;
  //  } // end IF
  //} // end lock scope

  ACE_ASSERT (!segment_p);
  result_2 = mkvparser::Segment::CreateInstance (this,
                                                 position_i,
                                                 segment_p);
  while (result_2 > 0 || !segment_p)
  {
    { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
      if (unlikely (finished_))
      {
        result = 0;
        goto clean;
      } // end IF
    } // end lock scope

    //ACE_OS::sleep (sleep_timeout_50ms);
    result_2 = mkvparser::Segment::CreateInstance (this,
                                                   position_i,
                                                   segment_p);
  } // end WHILE
  if (unlikely (result_2 < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to mkvparser::Segment::CreateInstance(): %q, aborting\n"),
                inherited::mod_->name (),
                result_2));
    goto error;
  } // end IF
  ACE_ASSERT (segment_p);

#if defined (LIBWEBM_MODIFIED_API)
  // If your libwebm version allows directly overriding the size bounds:
  segment_p->m_size = -1;
#endif
  result_3 = segment_p->Load ();
  while (result_3 && !segment_p->GetCount ())
  {
    { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
      if (unlikely (finished_))
      {
        result = 0;
        goto clean;
      } // end IF
    } // end lock scope

    //ACE_OS::sleep (sleep_timeout_50ms);
    result_3 = segment_p->Load ();
  } // end WHILE
  //if (unlikely (result_3 < 0))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to mkvparser::Segment::Load(): %d, aborting\n"),
  //              inherited::mod_->name (),
  //              result_3));
  //  goto error;
  //} // end IF

  //result_2 = segment_p->ParseHeaders ();
  //while (result_2 > 0)
  //{
  //  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
  //    if (unlikely (finished_))
  //    {
  //      result = 0;
  //      goto clean;
  //    } // end IF
  //  } // end lock scope

  //  ACE_OS::sleep (sleep_timeout_50ms);
  //  result_2 = segment_p->ParseHeaders ();
  //} // end WHILE
  //if (unlikely (result_2 < 0))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to mkvparser::Segment::ParseHeaders(): %q, aborting\n"),
  //              inherited::mod_->name (),
  //              result_2));
  //  goto error;
  //} // end IF

  tracks_p = segment_p->GetTracks ();
  ACE_ASSERT (tracks_p);
  tracks_i = tracks_p->GetTracksCount ();
  for (unsigned long i = 0; i < tracks_i; ++i)
  {
    track_p = tracks_p->GetTrackByIndex (i);
    ACE_ASSERT (track_p);
    track_i = track_p->GetNumber ();
    switch (track_p->GetType ())
    {
      case mkvparser::Track::Type::kAudio:
      {
        const mkvparser::AudioTrack* audio_track_p =
          static_cast<const mkvparser::AudioTrack*> (track_p);
        ACE_ASSERT (audio_track_p);
        const unsigned char* codec_private_p =
          audio_track_p->GetCodecPrivate (codec_private_length_i);
        if (codec_private_length_i && codec_private_p)
        {
          codec_configuration_s.size = static_cast<ACE_UINT32> (codec_private_length_i);
          ACE_NEW_NORETURN (codec_configuration_s.data,
                            ACE_UINT8[codec_private_length_i + inherited::configuration_->allocatorConfiguration->paddingBytes]);
          ACE_ASSERT (codec_configuration_s.data);
          ACE_OS::memset (codec_configuration_s.data, 0, codec_private_length_i + inherited::configuration_->allocatorConfiguration->paddingBytes);
          ACE_OS::memcpy (codec_configuration_s.data, codec_private_p, codec_private_length_i);
          const char* codec_id_p = audio_track_p->GetCodecId ();
          ACE_ASSERT (codec_id_p && std::string (codec_id_p) == ACE_TEXT_ALWAYS_CHAR ("A_VORBIS"));
          session_data_r.codecConfiguration.insert (std::make_pair (86021, // AV_CODEC_ID_VORBIS
                                                                    codec_configuration_s));
        } // end IF

        trackNumberToMessageMediaType_.insert (std::make_pair (track_i, STREAM_MEDIATYPE_AUDIO));
        if (audioTrackNumber_ == -1)
        {
          audioTrackNumber_ = track_i;
          break;
        } // end IF
        track_numbers_to_skip_a.push_back (track_i);
        break;
      }
      case mkvparser::Track::Type::kVideo:
      {
        trackNumberToMessageMediaType_.insert (std::make_pair (track_i, STREAM_MEDIATYPE_VIDEO));
        if (videoTrackNumber_ == -1)
        {
          videoTrackNumber_ = track_i;
          break;
        } // end IF
        track_numbers_to_skip_a.push_back (track_i);
        break;
      }
      default:
      {
        track_numbers_to_skip_a.push_back (track_i);
        break;
      }
    } // end SWITCH
  } // end FOR

  cluster_p = segment_p->GetFirst ();
  ACE_ASSERT (cluster_p);
  while (cluster_p && !cluster_p->EOS ())
  {
    { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
      if (unlikely (finished_))
      {
        result = 0;
        break;
      } // end IF
    } // end lock scope

    result_3 = cluster_p->GetFirst (block_entry_p);
    if (unlikely (result_3 || !block_entry_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to mkvparser::Cluster::GetFirst(): %d, aborting\n"),
                  inherited::mod_->name (),
                  result_3));
      goto error;
    } // end IF

    while (block_entry_p && !block_entry_p->EOS ())
    {
      const mkvparser::Block* block_p = block_entry_p->GetBlock ();
      ACE_ASSERT (block_p);
      track_i = block_p->GetTrackNumber ();
      ACE_ASSERT (track_i);
      if (std::find (track_numbers_to_skip_a.begin (), track_numbers_to_skip_a.end (), track_i) != track_numbers_to_skip_a.end () ||
                     track_i != audioTrackNumber_) // *TODO*
        goto continue_;

      frames_i = block_p->GetFrameCount ();
      for (int i = 0; i < frames_i; ++i)
      {
        const mkvparser::Block::Frame& frame_r = block_p->GetFrame (i);
        message_p =
          inherited::allocateMessage (frame_r.len + inherited::configuration_->allocatorConfiguration->paddingBytes,
                                      NULL);
        if (unlikely (!message_p))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                      inherited::mod_->name (),
                      frame_r.len + inherited::configuration_->allocatorConfiguration->paddingBytes));
          goto error;
        } // end IF
        message_p->size (frame_r.len);

        frame_r.Read (this,
                      reinterpret_cast<unsigned char*> (message_p->wr_ptr ()));
        message_p->wr_ptr (frame_r.len);
        message_p->setMediaType (trackNumberToMessageMediaType_[track_i]);

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

        if (frame_r.pos + frame_r.len > position_i)
          position_i = frame_r.pos + frame_r.len;
      } // end FOR
continue_:
      cluster_p-> GetNext (block_entry_p, block_entry_p);
    } // end WHILE

    const mkvparser::Cluster* cluster_2 = NULL;
    long long parse_pos = 0;
    long parse_len = 0;
    result_3 =
      segment_p->ParseNext (cluster_p, cluster_2, parse_pos, parse_len);
    while (result_3 && !cluster_2)
    {
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        if (unlikely (finished_))
        {
          result = 0;
          goto clean;
        } // end IF
      } // end lock scope

      //ACE_OS::sleep (sleep_timeout_10ms);
      result_3 =
        segment_p->ParseNext (cluster_p, cluster_2, parse_pos, parse_len);
    } // end WHILE
    if (unlikely (result_3 < 0 || !cluster_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to mkvparser::Segment::ParseNext(): %d, aborting\n"),
                  inherited::mod_->name (),
                  result_3));
      goto error;
    } // end IF
    cluster_p = cluster_2;
    //cluster_p = segment_p->GetNext (cluster_p);
    //ACE_ASSERT (cluster_p);

    if (position_i > 0)
      purge (position_i);
  } // end WHILE

clean:
  if (segment_p)
  {
    delete segment_p; segment_p = NULL;
  } // end IF

  goto done;

error:
  result = -1;
  if (segment_p)
    delete segment_p;

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
    finished_ = true;
    condition_.broadcast ();
  } // end lock scope

  // *WARNING*: could deadlock if all upstream modules are synchronous !
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

done:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Decoder_LibWebM_Demuxer_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 MediaType>::purge (long long absoluteCurrentPosition_in)
{
  ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);

  // sanity check(s)
  if (absoluteCurrentPosition_in <= absoluteHeadOffset_)
    return;

  size_t bytes_to_purge_i =
    static_cast<size_t> (absoluteCurrentPosition_in - absoluteHeadOffset_);
  if (bytes_to_purge_i >= buffer_.size ())
  {
    absoluteHeadOffset_ += buffer_.size ();
    buffer_.clear ();
  } // end IF
  else
  {
    buffer_.erase (buffer_.begin (), buffer_.begin () + bytes_to_purge_i);
    absoluteHeadOffset_ = absoluteCurrentPosition_in;
  } // end ELSE

  condition_.broadcast ();
}
