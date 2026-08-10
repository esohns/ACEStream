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

#include "webm/webm_parser.h"

#include "ace/Log_Msg.h"

#include "common_math_tools.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::Stream_Decoder_LibWebM_2_Demuxer_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , buffer_ ()
 , condition_ (inherited::lock_, NULL, NULL)
 , finished_ (false)
 , maxBufferCapacity_ (STREAM_DEC_WEBM_MAX_BUFFER_SIZE)
 , readPosition_ (0)
 , totalPosition_ (0)
 , currentElementId_ (webm::Id::kTagDefault)
 , currentElementSize_ (0)
 , lastTrackNumber_ (0)
 , lastTrackType_ (STREAM_MEDIATYPE_INVALID)
 , lastCodecId_ ()
 , trackNumberToMessageMediaType_ ()
 , trackNumbersToSkip_ ()
 , audioTrackNumber_ (-1)
 , videoTrackNumber_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::Stream_Decoder_LibWebM_2_Demuxer_T"));

  buffer_.reserve (maxBufferCapacity_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::~Stream_Decoder_LibWebM_2_Demuxer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::~Stream_Decoder_LibWebM_2_Demuxer_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::initialize"));

  if (inherited::isInitialized_)
  {
    buffer_.clear ();
    finished_ = false;
    readPosition_ = 0;
    totalPosition_ = 0;

    currentElementId_ = webm::Id::kTagDefault;
    currentElementSize_ = 0;
    lastTrackNumber_ = 0;
    lastTrackType_ = STREAM_MEDIATYPE_INVALID;
    lastCodecId_.clear ();
    lastCodecPrivateData_.clear ();

    trackNumberToMessageMediaType_.clear ();
    trackNumbersToSkip_.clear ();

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
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::handleDataMessage"));

  // initialize return value(s)
  passMessageDownstream_out = false;

  static ACE_Time_Value sleep_timeout_10ms (std::chrono::milliseconds (10));
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
      while (!finished_ &&
             (buffer_.size () + available_bytes_i > maxBufferCapacity_))
      {
        absolute_timeout = COMMON_TIME_NOW + sleep_timeout_10ms;
        condition_.wait (&absolute_timeout);
      } // end WHILE
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
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::handleSessionMessage"));

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
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::Read (std::size_t bytesToRead_in,
                                                     std::uint8_t* buffer_in,
                                                     std::uint64_t* readBytes_out)
{
  // sanity check(s)
  ACE_ASSERT (bytesToRead_in >= 0 && buffer_in && readBytes_out);

  // initialize return value(s)
  *readBytes_out = 0;
  //webm::Status result (webm::Status::kEndOfFile);

  std::size_t bytes_available_i;
  std::size_t bytes_to_copy_i;

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, webm::Status (webm::Status::kWouldBlock));
    // sanity check(s)
    bytes_available_i = buffer_.size () - readPosition_;
    if (unlikely (!bytes_available_i))
      return webm::Status (webm::Status::kWouldBlock);

    bytes_to_copy_i = std::min (bytesToRead_in, bytes_available_i);
    ACE_OS::memcpy (buffer_in, buffer_.data () + readPosition_, bytes_to_copy_i);

    readPosition_ += bytes_to_copy_i;
    totalPosition_ += bytes_to_copy_i;
  } // end lock scope
  *readBytes_out = bytes_to_copy_i;

  return webm::Status (bytes_to_copy_i == bytesToRead_in ? webm::Status::kOkCompleted
                                                         : webm::Status::kOkPartial);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::Skip (std::uint64_t bytesToSkip_in,
                                                     std::uint64_t* skippedBytes_out)
{
  // sanity check(s)
  ACE_ASSERT (bytesToSkip_in >= 0 && skippedBytes_out);

  // initialize return value(s)
  *skippedBytes_out = 0;

  std::size_t bytes_available_i, to_skip_amount_i;

  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, webm::Status (webm::Status::kWouldBlock));
    // sanity check(s)
    bytes_available_i = buffer_.size () - readPosition_;
    if (unlikely (!bytes_available_i))
      return webm::Status (webm::Status::kWouldBlock);

    to_skip_amount_i =
      std::min (static_cast<std::size_t> (bytesToSkip_in), bytes_available_i);
    readPosition_ += to_skip_amount_i;
    totalPosition_ += to_skip_amount_i;
  } // end lock scope
  *skippedBytes_out = to_skip_amount_i;

  return webm::Status (to_skip_amount_i == bytesToSkip_in ? webm::Status::kOkCompleted
                                                          : webm::Status::kOkPartial);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
std::uint64_t
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::Position () const
{
  std::uint64_t result;
  
  { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, 0);
    result = totalPosition_;
  } // end lock scope

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnElementBegin (const webm::ElementMetadata& metadata_in,
                                                               webm::Action* action_out)
{
  // sanity check(s)
  ACE_ASSERT (action_out);

  currentElementId_ = metadata_in.id;
  currentElementSize_ = metadata_in.size;

  switch (metadata_in.id)
  {
    case webm::Id::kTrackEntry:
    {
      *action_out = webm::Action::kRead;
      break;
    }
    case webm::Id::kTrackNumber:
    case webm::Id::kTrackType:
    case webm::Id::kCodecId:
    case webm::Id::kCodecPrivate:
    {
      *action_out = webm::Action::kRead;
      break;
    }
    case webm::Id::kSimpleBlock:
    case webm::Id::kBlock:
    {
      *action_out = webm::Action::kRead;
      break;
    }
    default:
    {
      *action_out = webm::Action::kRead;
      break;
    }
  } // end SWITCH

  return webm::Status (webm::Status::kOkCompleted);
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename MediaType>
//webm::Status
//Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
//                                   TimePolicyType,
//                                   ConfigurationType,
//                                   ControlMessageType,
//                                   DataMessageType,
//                                   SessionMessageType,
//                                   MediaType>::OnEbml (const webm::ElementMetadata& metadata_in,
//                                                       webm::Reader* reader_in)
//{
//  // sanity check(s)
//  ACE_ASSERT (reader_in);
//
//  std::uint64_t bytes_actually_read = 0;
//  webm::Status status_s;
//
//  switch (currentElementId_)
//  {
//    case webm::Id::kCodecId:
//    {
//      std::vector<char> codec_buf (metadata_in.size + 1, '\0');
//      reader_in->Read (metadata_in.size,
//                       reinterpret_cast<std::uint8_t*> (codec_buf.data ()),
//                       &bytes_actually_read);
//      lastCodecId_ = std::string (codec_buf.data ());
//      return webm::Status (webm::Status::kOkCompleted);
//    }
//    case webm::Id::kCodecPrivate:
//    {
//      lastCodecPrivateData_.resize (metadata_in.size);
//      reader_in->Read (metadata_in.size,
//                       lastCodecPrivateData_.data (),
//                       &bytes_actually_read);
//      return webm::Status(webm::Status::kOkCompleted);
//    }
//    case webm::Id::kTrackNumber:
//    {
//      std::vector<std::uint8_t> val_buf (metadata_in.size);
//      reader_in->Read (metadata_in.size,
//                       val_buf.data (),
//                       &bytes_actually_read);
//
//      lastTrackNumber_ = 0;
//      for (size_t i = 0; i < bytes_actually_read; ++i)
//        lastTrackNumber_ = (lastTrackNumber_ << 8) | val_buf[i];
//      return webm::Status (webm::Status::kOkCompleted);
//    }
//    case webm::Id::kTrackType:
//    {
//      std::vector<std::uint8_t> val_buf (metadata_in.size);
//      reader_in->Read (metadata_in.size,
//                       val_buf.data (),
//                       &bytes_actually_read);
//           
//      std::uint64_t type_i = 0;
//      for (size_t i = 0; i < bytes_actually_read; ++i)
//        type_i = (type_i << 8) | val_buf[i];
//      switch (type_i)
//      {
//        case 2: // Audio
//        {
//          lastTrackType_ = STREAM_MEDIATYPE_AUDIO;
//          break;
//        }
//        default:
//        {
//          ACE_DEBUG ((LM_DEBUG,
//                      ACE_TEXT ("%s: ignoring track (current#: %Q) type (was: %Q), continuing\n"),
//                      inherited::mod_->name (),
//                      lastTrackNumber_,
//                      type_i));
//          break;
//        }
//      } // end SWITCH
//      return webm::Status(webm::Status::kOkCompleted);
//    }
//    case webm::Id::kSimpleBlock:
//    case webm::Id::kBlock:
//    {
//      std::vector<std::uint8_t> block_container_a (metadata_in.size);
//      status_s = reader_in->Read (metadata_in.size,
//                                  block_container_a.data (),
//                                  &bytes_actually_read);
//      if (status_s.code == webm::Status::kWouldBlock)
//        return status_s;
//
//      size_t parse_offset_i = 0;
//
//      std::uint64_t track_number_i = 0;
//      size_t vint_length_i =
//        parseVariableLengthInteger (block_container_a.data () + parse_offset_i, 
//                                    block_container_a.size () - parse_offset_i,
//                                    &track_number_i);
//      if (unlikely (!vint_length_i))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to parse Track# VINT, aborting\n"),
//                    inherited::mod_->name ()));
//        return webm::Status (webm::Status::kInvalidElementValue);
//      } // end IF
//      parse_offset_i += vint_length_i;
//
//      if (std::find (trackNumbersToSkip_.begin (), trackNumbersToSkip_.end (), track_number_i) != trackNumbersToSkip_.end () ||
//                     track_number_i != audioTrackNumber_) // *TODO*
//        return webm::Status (webm::Status::kOkCompleted); // --> skip this frame
//
//      // sanity check(s)
//      if (unlikely (parse_offset_i + 2 > block_container_a.size ()))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: cannot parse Relative Timecode, aborting\n"),
//                    inherited::mod_->name ()));
//        return webm::Status (webm::Status::kInvalidElementValue);
//      } // end IF
//      std::int16_t relative_timecode_i =
//        (block_container_a[parse_offset_i] << 8) | block_container_a[parse_offset_i + 1];
//      parse_offset_i += 2;
//
//      // sanity check(s)
//      if (parse_offset_i + 1 > block_container_a.size ())
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: cannot parse Flag Mask, aborting\n"),
//                    inherited::mod_->name ()));
//        return webm::Status (webm::Status::kInvalidElementValue);
//      } // end IF
//      std::uint8_t flags_i = block_container_a[parse_offset_i];
//      parse_offset_i += 1;
//
//      bool is_keyframe_b = (currentElementId_ == webm::Id::kSimpleBlock) && ((flags_i & 0x80) != 0);
//      std::uint8_t lacing_type_i =
//        (flags_i & 0x06) >> 1; // 00 = No lacing, 01 = Xiph, 11 = EBML, 10 = Fixed-size
//      // sanity check(s)
//      if (unlikely (lacing_type_i))
//      {
//        // WebM files typically do not use lacing for video streams (VP9/AV1), 
//        // but audio files (Opus) occasionally do. For raw, unlaced data:
//        ACE_DEBUG ((LM_WARNING,
//                    ACE_TEXT ("%s: block lacing (was: 0x%X is unsupported ATM, continuing\n"),
//                    inherited::mod_->name (),
//                    lacing_type_i));
//      } // end IF
//
//      size_t raw_frame_size_i = block_container_a.size () - parse_offset_i;
//      std::uint8_t* raw_frame_data_p = block_container_a.data () + parse_offset_i;
//      DataMessageType* message_p =
//        inherited::allocateMessage (raw_frame_size_i + inherited::configuration_->allocatorConfiguration->paddingBytes,
//                                    NULL);
//      if (unlikely (!message_p))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%B), aborting\n"),
//                    inherited::mod_->name (),
//                    raw_frame_size_i + inherited::configuration_->allocatorConfiguration->paddingBytes));
//        return webm::Status (webm::Status::kNotEnoughMemory);
//      } // end IF
//      message_p->size (raw_frame_size_i);
//      int result = message_p->copy (reinterpret_cast<char*> (raw_frame_data_p),
//                                    raw_frame_size_i);
//      if (unlikely (result == -1))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%B): \"%m\", aborting\n"),
//                    inherited::mod_->name (),
//                    raw_frame_size_i));
//        message_p->release (); message_p = NULL;
//        return webm::Status (webm::Status::kNotEnoughMemory);
//      } // end IF
//      message_p->setMediaType (trackNumberToMessageMediaType_[track_number_i]);
//
//      result = inherited::put_next (message_p, NULL);
//      if (unlikely (result == -1))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
//                    inherited::mod_->name ()));
//        message_p->release (); message_p = NULL;
//        return webm::Status (webm::Status::kNotEnoughMemory);
//      } // end IF
//      message_p = NULL;
//
//      return webm::Status (webm::Status::kOkCompleted);
//    }
//    default:
//      break;
//  } // end SWITCH
//
//  // skip selected elements not explicitly parsed
//  reader_in->Skip (metadata_in.size, &bytes_actually_read);
//
//  return webm::Status(webm::Status::kOkCompleted);
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnTrackEntry (const webm::ElementMetadata& metadata_in,
                                                             const webm::TrackEntry& track_in)
{
  lastTrackNumber_ = track_in.track_number.value ();
  switch (track_in.track_type.value ())
  {
    case webm::TrackType::kAudio:
    {
      lastTrackType_ = STREAM_MEDIATYPE_AUDIO;
      break;
    }
    case webm::TrackType::kVideo:
    {
      lastTrackType_ = STREAM_MEDIATYPE_VIDEO;
      break;
    }
    default:
    {
      lastTrackType_ = STREAM_MEDIATYPE_INVALID;
      break;
    }
  } // end SWITCH
  lastCodecId_ = track_in.codec_id.value ();
  lastCodecPrivateData_ = track_in.codec_private.value ();

  switch (lastTrackType_)
  {
    case STREAM_MEDIATYPE_AUDIO:
    {
      trackNumberToMessageMediaType_.insert (std::make_pair (lastTrackNumber_, STREAM_MEDIATYPE_AUDIO));
      if (audioTrackNumber_ == -1)
      {
        audioTrackNumber_ = lastTrackNumber_;

        if (lastCodecId_ == ACE_TEXT_ALWAYS_CHAR ("A_VORBIS"))
        { // sanity check(s)
          ACE_ASSERT (!lastCodecPrivateData_.empty ());
          std::uint8_t num_packets_minus_one_i = lastCodecPrivateData_[0];
          if (unlikely (num_packets_minus_one_i != 2))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: parse error for Vorbis codec private data (packet count - 1 was: %d, expected: 2), continuing\n"),
                        inherited::mod_->name (),
                        num_packets_minus_one_i));
            break;
          } // end IF

          size_t offset_i = 1;
          std::uint32_t size1_i = 0, size2_i = 0;

          offset_i +=
            parseXiphLacingSize (lastCodecPrivateData_.data () + offset_i, lastCodecPrivateData_.size () - offset_i, &size1_i);
          offset_i +=
            parseXiphLacingSize (lastCodecPrivateData_.data () + offset_i, lastCodecPrivateData_.size () - offset_i, &size2_i);
          if (unlikely (offset_i + size1_i + size2_i > lastCodecPrivateData_.size ()))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: parse error for Vorbis codec private data (invalid packet sizes), continuing\n"),
                        inherited::mod_->name ()));
            break;
          } // end IF
          std::uint32_t size3_i =
            static_cast<std::uint32_t> (lastCodecPrivateData_.size () - (offset_i + size1_i + size2_i));
          if (offset_i + size1_i + size2_i + size3_i != lastCodecPrivateData_.size ())
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: header lengths do not match codec private data size (was: %B, required: %B), continuing\n"),
                        inherited::mod_->name (),
                        offset_i + size1_i + size2_i + size3_i,
                        lastCodecPrivateData_.size ()));
            break;
          } // end IF

          // *NOTE*: the actual splitting is done by the Vorbis decoder module downstream...
          //         --> pass the raw blob downstream
          ACE_ASSERT (inherited::configuration_);
          ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
          ACE_ASSERT (inherited::sessionData_);

          struct Stream_MediaFramework_SessionData_CodecConfiguration codec_configuration_s;
          typename SessionMessageType::DATA_T::DATA_T& session_data_r =
            const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
          codec_configuration_s.size = static_cast<ACE_UINT32> (lastCodecPrivateData_.size ());
          ACE_NEW_NORETURN (codec_configuration_s.data,
                            ACE_UINT8[lastCodecPrivateData_.size () + inherited::configuration_->allocatorConfiguration->paddingBytes]);
          ACE_ASSERT (codec_configuration_s.data);
          ACE_OS::memset (codec_configuration_s.data, 0, lastCodecPrivateData_.size () + inherited::configuration_->allocatorConfiguration->paddingBytes);
          ACE_OS::memcpy (codec_configuration_s.data, lastCodecPrivateData_.data (), lastCodecPrivateData_.size ());
          session_data_r.codecConfiguration.insert (std::make_pair (86021, // AV_CODEC_ID_VORBIS
                                                                    codec_configuration_s));
        } // end IF
        else if (lastCodecId_ == ACE_TEXT_ALWAYS_CHAR ("A_OPUS"))
        {
          struct Stream_MediaFramework_SessionData_CodecConfiguration codec_configuration_s;
          typename SessionMessageType::DATA_T::DATA_T& session_data_r =
            const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
          codec_configuration_s.size = static_cast<ACE_UINT32> (lastCodecPrivateData_.size ());
          ACE_NEW_NORETURN (codec_configuration_s.data,
                            ACE_UINT8[lastCodecPrivateData_.size () + inherited::configuration_->allocatorConfiguration->paddingBytes]);
          ACE_ASSERT (codec_configuration_s.data);
          ACE_OS::memset (codec_configuration_s.data, 0, lastCodecPrivateData_.size () + inherited::configuration_->allocatorConfiguration->paddingBytes);
          ACE_OS::memcpy (codec_configuration_s.data, lastCodecPrivateData_.data (), lastCodecPrivateData_.size ());
          session_data_r.codecConfiguration.insert (std::make_pair (86076, // AV_CODEC_ID_OPUS
                                                                    codec_configuration_s));
        } // end ELSE IF
        else
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: skipping codec private data for codec (was: \"%s\"), continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (lastCodecId_.c_str ())));
          break;
        } // end ELSE

        break;
      } // end IF
      trackNumbersToSkip_.push_back (lastTrackNumber_);
      break;
    }
    case STREAM_MEDIATYPE_VIDEO:
    {
      trackNumberToMessageMediaType_.insert (std::make_pair (lastTrackNumber_, STREAM_MEDIATYPE_VIDEO));
      if (videoTrackNumber_ == -1)
      {
        videoTrackNumber_ = lastTrackNumber_;
        break;
      } // end IF
      trackNumbersToSkip_.push_back (lastTrackNumber_);
      break;
    }
    default:
      break;
  } // end SWITCH

  return webm::Status(webm::Status::kOkCompleted);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnSimpleBlockBegin (const webm::ElementMetadata& metadata_in,
                                                                   const webm::SimpleBlock& block_in,
                                                                   webm::Action* action_out)
{
  lastTrackNumber_ = block_in.track_number;
  if (std::find (trackNumbersToSkip_.begin (), trackNumbersToSkip_.end (), lastTrackNumber_) != trackNumbersToSkip_.end () ||
                 (lastTrackNumber_ != audioTrackNumber_                                                                    &&
                  lastTrackNumber_ != videoTrackNumber_))
    *action_out = webm::Action::kSkip; // --> skip this block
  else
    *action_out = webm::Action::kRead; // parse block frame(s)

  return webm::Status (webm::Status::kOkCompleted);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnFrame (const webm::FrameMetadata& metadata_in,
                                                        webm::Reader* reader_in,
                                                        std::uint64_t* bytesRemaining_inout)
{
  // sanity check(s)
  ACE_ASSERT (reader_in && bytesRemaining_inout/* && *bytesRemaining_inout == metadata_in.size*/);

  std::uint64_t bytes_actually_read_this_time_i = 0, bytes_actually_read_i = 0;
  webm::Status status_s;
  int result;
  size_t offset_i = 0, bytes_to_read_i = metadata_in.size;

  DataMessageType* message_p =
    inherited::allocateMessage (metadata_in.size + inherited::configuration_->allocatorConfiguration->paddingBytes,
                                NULL);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                metadata_in.size + inherited::configuration_->allocatorConfiguration->paddingBytes));
    return webm::Status (webm::Status::kNotEnoughMemory);
  } // end IF
  message_p->size (metadata_in.size);
  message_p->setMediaType (trackNumberToMessageMediaType_[lastTrackNumber_]);

  do
  { bytes_actually_read_this_time_i = 0;
    status_s =
      reader_in->Read (bytes_to_read_i,
                       reinterpret_cast<uint8_t*> (message_p->wr_ptr () + offset_i),
                       &bytes_actually_read_this_time_i);
    if (likely (bytes_actually_read_this_time_i))
    {
      bytes_actually_read_i += bytes_actually_read_this_time_i;
      offset_i += bytes_actually_read_this_time_i;
      bytes_to_read_i -= bytes_actually_read_this_time_i;
      *bytesRemaining_inout -= bytes_actually_read_this_time_i;
    } // end IF
    if (unlikely (bytes_actually_read_i < metadata_in.size))
    {
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, webm::Status (webm::Status::kEndOfFile));
        if (unlikely (finished_))
        {
          message_p->wr_ptr (bytes_actually_read_i);

          result = inherited::put_next (message_p, NULL);
          if (unlikely (result == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_p->release (); message_p = NULL;
          } // end IF
          return webm::Status (webm::Status::kEndOfFile);
        } // end IF
      } // end lock scope

      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: failed to webm::Reader::Read(%Q), retrying\n"),
      //            inherited::mod_->name (),
      //            metadata_in.size));
      continue;
    } // end IF
  } while (*bytesRemaining_inout);
  ACE_ASSERT (*bytesRemaining_inout == 0);
  message_p->wr_ptr (metadata_in.size);

  result = inherited::put_next (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    return webm::Status (webm::Status::kEndOfFile);
  } // end IF
  message_p = NULL;

  return webm::Status (webm::Status::kOkCompleted);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnClusterEnd (const webm::ElementMetadata& metadata_in,
                                                             const webm::Cluster& cluster_in)
{
  purge ();

  return webm::Status (webm::Status::kOkCompleted);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
webm::Status
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::OnSegmentEnd (const webm::ElementMetadata& metadata_in)
{
  purge ();

  return webm::Status (webm::Status::kOkCompleted);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::svc"));

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
  DataMessageType* message_p = NULL;
  std::vector<long> track_numbers_to_skip_a;
  //static ACE_Time_Value backoff_timeout (STREAM_MESSAGE_ALLOCATION_SOURCE_BACKOFF_TIMEOUT_S, 0);
  static ACE_Time_Value sleep_timeout_10ms (std::chrono::milliseconds (10));
  //static ACE_Time_Value sleep_timeout_50ms (std::chrono::milliseconds (50));
  size_t codec_private_length_i;
  struct Stream_MediaFramework_SessionData_CodecConfiguration codec_configuration_s;
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  webm::WebmParser parser;
  webm::Status status_s;

  while (true)
  {
    { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
      if (unlikely (finished_))
      {
        result = 0;
        break; // stream has shut down; leave
      } // end IF
    } // end lock scope

    status_s = parser.Feed (this, this);
    if (status_s.code == webm::Status::kWouldBlock)
    {
      // buffer full ? --> purge some data
      bool do_purge_b = false;
      { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
        // *NOTE*: there is currently no std::abs() for size_t... :-(
        if (Common_Math_Tools::almost_equal_percentage (static_cast<int64_t> (buffer_.size ()), static_cast<int64_t> (maxBufferCapacity_), 0.1f) &&
            (buffer_.size () == readPosition_))
          do_purge_b = true;
      } // end lock scope
      if (unlikely (do_purge_b))
        purge ();
      ACE_OS::sleep (sleep_timeout_10ms);
      continue; 
    } // end IF
    else if (status_s.completed_ok ())
    {
      result = 0;
      break; // EOS
    } // end ELSE IF
    else if (status_s.ok ())
      continue;
    else if (status_s.is_parsing_error ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to parse stream (code was: %d), aborting\n"),
                  inherited::mod_->name (),
                  status_s.code));
      goto error;
    } // end ELSE IF
    // *TODO*: what happened ?
  } // end WHILE

  goto done;

error:
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
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::purge ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibWebM_2_Demuxer_T::purge"));

  { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
    // sanity check(s)
    if (!readPosition_)
      return;

    buffer_.erase (buffer_.begin (), buffer_.begin () + readPosition_);
    readPosition_ = 0;
    buffer_.shrink_to_fit ();

    condition_.broadcast ();
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
size_t
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::parseVariableLengthInteger (const std::uint8_t* buffer_in,
                                                                           size_t bufferSize_in,
                                                                           std::uint64_t* value_out)
{
  // sanity check(s)
  ACE_ASSERT (buffer_in && bufferSize_in && value_out);
  std::uint8_t first_byte = buffer_in[0];
  if (unlikely (first_byte == 0))
    return 0; // Invalid VINT indicator

  // count leading zeros to find total byte length
  size_t length_i = 1;
  while ((first_byte & (0x80 >> (length_i - 1))) == 0 && length_i <= 8)
    length_i++;
  // sanity check(s)
  if (length_i > bufferSize_in)
    return 0; // Not enough data in buffer

  // Clear the leading marker bit from the first byte
  std::uint64_t result = first_byte & ((0x80 >> (length_i - 1)) - 1);
  // Read remaining bytes
  for (size_t i = 1; i < length_i; ++i)
    result = (result << 8) | buffer_in[i];

  *value_out = result;

  return length_i;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
size_t
Stream_Decoder_LibWebM_2_Demuxer_T<ACE_SYNCH_USE,
                                   TimePolicyType,
                                   ConfigurationType,
                                   ControlMessageType,
                                   DataMessageType,
                                   SessionMessageType,
                                   MediaType>::parseXiphLacingSize (const std::uint8_t* buffer_in,
                                                                    size_t bufferSize_in,
                                                                    std::uint32_t* packetSize_out)
{
  // sanity check(s)
  ACE_ASSERT (buffer_in && bufferSize_in && packetSize_out);

  size_t index_i = 0;
  std::uint32_t aggregated_size_i = 0;
        
  while (index_i < bufferSize_in)
  {
    aggregated_size_i += buffer_in[index_i];
    if (buffer_in[index_i] != 255)
    {
      index_i++;
      break;
    } // end IF
    index_i++;
  } // end WHILE

  *packetSize_out = aggregated_size_i;

  return index_i;
}
