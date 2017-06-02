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

#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "stream_macros.h"

#include "stream_dec_common.h"
#include "stream_dec_defines.h"

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_ZIPDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::Stream_Decoder_ZIPDecoder_T ()
 : inherited ()
 , sessionData_ (NULL)
 , buffer_ (NULL)
 , crunchMessages_ (STREAM_DECODER_DEFAULT_CRUNCH_MESSAGES)
 , stream_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::Stream_Decoder_ZIPDecoder_T"));

  ACE_OS::memset (&stream_, 0, sizeof (struct z_stream_s));
}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_ZIPDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::~Stream_Decoder_ZIPDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::~Stream_Decoder_ZIPDecoder_T"));

  // clean up any unprocessed (chained) buffer(s)
  if (buffer_)
    buffer_->release ();
}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_ZIPDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::initialize"));

  if (inherited::isInitialized_)
  {
    sessionData_ = NULL;

    if (buffer_)
      buffer_->release ();
    buffer_ = NULL;
    crunchMessages_ = STREAM_DECODER_DEFAULT_CRUNCH_MESSAGES;
  } // end IF

  // *TODO*: remove type dependencies
  crunchMessages_ = configuration_in.crunchMessages;

  stream_.zalloc = Z_NULL;
  stream_.zfree = Z_NULL;
  stream_.opaque = (voidpf)NULL;

  return inherited::initialize (configuration_in,
                                allocator_in);
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataContainerType>
//const ConfigurationType&
//Stream_Decoder_ZIPDecoder_T<SessionMessageType,
//                            MessageType,
//                            ConfigurationType,
//                            SessionDataContainerType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::get"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
//}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_ZIPDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::mod_);
  ACE_ASSERT (sessionData_);

  switch (sessionData_->format)
  {
    case STREAM_COMPRESSION_FORMAT_GZIP:
    case STREAM_COMPRESSION_FORMAT_ZLIB:
    {
      passMessageDownstream_out = false;
      break;
    }
    default:
    {
      // not compressed / encoding not supported
      // --> pass data along unmodified
      return;
    }
  } // end SWITCH

  ACE_Message_Block* message_block_p = NULL;

  // form a chain of buffers
  if (buffer_)
  {
    message_block_p = buffer_->cont ();
    if (message_block_p)
    {
      while (message_block_p->cont ()) // skip to end
        message_block_p = message_block_p->cont ();
    } // end IF
    else
      message_block_p = buffer_;
    message_block_p->cont (message_inout); // chain the buffer
  } // end IF
  else
    buffer_ = message_inout;
  message_inout = NULL;

  int result = -1;
  DataMessageType* message_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  bool finalize_stream = false;
  bool complete = false;
  ACE_Message_Block* buffer_p = buffer_;
  //unsigned int read_bytes = 0;
  unsigned int written_bytes = 0;

  //// "crunch" messages for easier decompression ?
  //if (crunchMessages_ &&
  //    buffer_->cont ())
  //{
  //  // sanity check(s)
  //  ACE_ASSERT (buffer_->total_length () <= STREAM_DECODER_BUFFER_SIZE);

  //  // step1: get a new message buffer
  //  message_p = allocateMessage (STREAM_DECODER_BUFFER_SIZE);
  //  if (!message_p)
  //  {
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to allocate message(%u), returning\n"),
  //                STREAM_DECODER_BUFFER_SIZE));
  //    goto error;
  //  } // end IF

  //  // step2: copy available data
  //  for (message_block_p = buffer_;
  //       message_block_p;
  //       message_block_p = message_block_p->cont ())
  //  {
  //    // sanity check(s)
  //    ACE_ASSERT (message_block_p->length () <= message_p->space ());

  //    result = message_p->copy (message_block_p->rd_ptr (),
  //                              message_block_p->length ());
  //    if (result == -1)
  //    {
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

  //      // clean up
  //      message_p->release ();

  //      goto error;
  //    } // end IF
  //  } // end FOR

  //  buffer_->release ();
  //  buffer_ = message_p;
  //} // end IF

  // initialize stream
  stream_.next_in =
    reinterpret_cast<unsigned char*> (buffer_->rd_ptr ());
  stream_.avail_in = (uInt)buffer_->length ();
  int window_bits =
    ((sessionData_->format == STREAM_COMPRESSION_FORMAT_ZLIB) ? 0 // use zlib header setting
                                                              : STREAM_DECODER_DEFAULT_ZLIB_WINDOWBITS);
  // *NOTE*: see also: http://www.zlib.net/manual.html#Basic
  if (sessionData_->format == STREAM_COMPRESSION_FORMAT_GZIP)
    window_bits += STREAM_DECODER_ZLIB_WINDOWBITS_GZIP_OFFSET;
  result = inflateInit2 (&stream_, window_bits);
  if (result != Z_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to inflateInit2(%d) (result was: %d): \"%s\", returning\n"),
                inherited::mod_->name (),
                window_bits,
                result,
                (stream_.msg ? ACE_TEXT (stream_.msg) : ACE_TEXT (""))));
    goto error;
  } // end IF
  finalize_stream = true;

  // read header ?
  struct gz_header_s header_s;
  if (sessionData_->format == STREAM_COMPRESSION_FORMAT_GZIP)
  {
    ACE_OS::memset (&header_s, 0, sizeof (struct gz_header_s));
    result = inflateGetHeader (&stream_, &header_s);
    if (result != Z_OK)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to inflateGetHeader() (result was: %d): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  result,
                  (stream_.msg ? ACE_TEXT (stream_.msg) : ACE_TEXT (""))));
      goto error;
    } // end IF
  } // end IF

  // step1: allocate a new message
  message_p = inherited::allocateMessage (STREAM_DECODER_BUFFER_SIZE);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to allocate message(%u), returning\n"),
                inherited::mod_->name (),
                STREAM_DECODER_BUFFER_SIZE));
    goto error;
  } // end IF
  message_block_p = message_p;

  // step2: decompress the buffer (chain)
  ACE_ASSERT (message_block_p);
  stream_.next_out =
    reinterpret_cast<unsigned char*> (message_block_p->wr_ptr ());
  stream_.avail_out = (uInt)message_block_p->size ();
  for (;;)
  {
    result = inflate (&stream_, Z_NO_FLUSH);
    if ((result != Z_OK) &&
        (result != Z_STREAM_END))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to inflate() (result was: %d): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  result,
                  (stream_.msg ? ACE_TEXT (stream_.msg) : ACE_TEXT (""))));
      goto error;
    } // end IF
    //buffer_p->rd_ptr (stream_.total_in - read_bytes);
    //read_bytes = stream_.total_in;
    message_block_p->wr_ptr (stream_.total_out - written_bytes);
    written_bytes = stream_.total_out;

    if (result == Z_STREAM_END)
    {
      // set next head buffer
      // *TODO*: (depending on processing upstream, i.e. 'interleaved' buffers)
      //         it may be necessary to adjust read/writer pointers here
      if (stream_.total_in < buffer_->total_length ())
      {
        message_block_2 = buffer_p->duplicate ();
        if (!message_block_2)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to MessageType::duplicate(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end IF
      else
        message_block_2 = buffer_p->cont ();
      buffer_p->cont (NULL);
      buffer_->release ();
      buffer_ = message_block_2;

      complete = true;
      break; // done
    } // end IF

    // set next input ?
    if (!stream_.avail_in)
    {
      buffer_p = buffer_p->cont ();
      if (!buffer_p)
        break; // no more data, cannot proceed

      stream_.next_in =
        reinterpret_cast<unsigned char*> (buffer_p->rd_ptr ());
      stream_.avail_in = (uInt)buffer_p->length ();
    } // end IF

    // set next output ?
    if (!stream_.avail_out)
    {
      message_block_2 = message_block_p;
      message_block_p = inherited::allocateMessage (STREAM_DECODER_BUFFER_SIZE);
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to allocate message(%u), returning\n"),
                    inherited::mod_->name (),
                    STREAM_DECODER_BUFFER_SIZE));
        goto error;
      } // end IF
      message_block_2->cont (message_block_p);

      stream_.next_out =
        reinterpret_cast<unsigned char*> (message_block_p->wr_ptr ());
      stream_.avail_out = (uInt)message_block_p->size ();
    } // end IF
  } // end FOR

  // finalize stream
  result = inflateEnd (&stream_);
  if (result != Z_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to inflateEnd() (result was: %d): \"%s\", returning\n"),
                inherited::mod_->name (),
                result,
                (stream_.msg ? ACE_TEXT (stream_.msg) : ACE_TEXT (""))));
    goto error;
  } // end IF
  finalize_stream = false;

  if (!complete)
  {
    // clean up
    message_p->release ();
    message_p = NULL;

    goto continue_; // not enough data, cannot proceed
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: inflated %d --> %d byte(s)...\n"),
              inherited::mod_->name (),
              stream_.total_in,
              message_p->total_length ()));

  result = inherited::put_next (message_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

continue_:
  return;

error:
  if (finalize_stream)
  {
    result = inflateEnd (&stream_);
    if (result != Z_OK)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to inflateEnd() (result was: %d): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  result,
                  (stream_.msg ? ACE_TEXT (stream_.msg) : ACE_TEXT (""))));
  } // end IF
  if (message_p)
    message_p->release ();
  buffer_->release ();
  buffer_ = NULL;
}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_ZIPDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  const SessionDataContainerType& session_data_container_r =
    message_inout->get ();
  typename SessionDataContainerType::DATA_T& session_data_r =
    const_cast<typename SessionDataContainerType::DATA_T&> (session_data_container_r.get ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      sessionData_ = &session_data_r;
      break;
    }
    default:
      break;
  } // end SWITCH
}

//template <typename SynchStrategyType,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataContainerType>
//DataMessageType*
//Stream_Decoder_ZIPDecoder_T<SynchStrategyType,
//                            TimePolicyType,
//                            ConfigurationType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            SessionDataContainerType>::allocateMessage (unsigned int requestedSize_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_ZIPDecoder_T::allocateMessage"));

//  // initialize return value(s)
//  DataMessageType* message_p = NULL;

//  if (allocator_)
//  {
//allocate:
//    try {
//      message_p =
//        static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in));
//    } catch (...) {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
//                  requestedSize_in));
//      return NULL;
//    }

//    // keep retrying ?
//    if (!message_p && !allocator_->block ())
//      goto allocate;
//  } // end IF
//  else
//    ACE_NEW_NORETURN (message_p,
//                      DataMessageType (requestedSize_in));
//  if (!message_p)
//  {
//    if (allocator_)
//    {
//      if (allocator_->block ())
//        ACE_DEBUG ((LM_CRITICAL,
//                    ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
//    } // end IF
//    else
//      ACE_DEBUG ((LM_CRITICAL,
//                  ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
//  } // end IF

//  return message_p;
//}
