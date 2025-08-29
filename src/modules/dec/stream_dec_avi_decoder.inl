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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mmiscapi.h"
#include "aviriff.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_image_defines.h"

#include "stream_macros.h"

#include "stream_dec_defines.h"

#include "stream_dec_riff_scanner.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::Stream_Decoder_AVIDecoder_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 (COMMON_PARSER_DEFAULT_LEX_TRACE,
               COMMON_PARSER_DEFAULT_YACC_TRACE)
 , buffer_ (NULL)
 , frameSize_ (0)
 , headerParsed_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::Stream_Decoder_AVIDecoder_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::~Stream_Decoder_AVIDecoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::~Stream_Decoder_AVIDecoder_T"));

  // clean up any unprocessed (chained) buffer(s)
  if (buffer_)
    buffer_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::initialize"));

  // sanity check(s)
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      buffer_->release (); buffer_ = NULL;
    } // end IF
    frameSize_ = 0;
    headerParsed_ = false;
  } // end IF

  // *TODO*: remove type inference
  inherited2::initialize (frameSize_,
                          configuration_in.parserConfiguration->extractHeaderOnly,
#if defined (_DEBUG)
                          configuration_in.parserConfiguration->debugScanner,
                          configuration_in.parserConfiguration->debugParser,
#else
                          false,
                          false,
#endif // _DEBUG
                          inherited::msg_queue (),
                          configuration_in.parserConfiguration->useYYScanBuffer);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::handleDataMessage"));

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

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

  // OK: parse this message
  if (!inherited2::parse (buffer_))
  {
    if (inherited2::finished_) // received session end ?
      goto done;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Decoder_AVIParserDriver::parse() (message ID: %d), returning\n"),
                message_inout->id ()));
    goto done;
  } // end IF

done:
  return;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  const typename SessionMessageType::DATA_T& session_data_container_r =
    message_inout->getR ();
  SessionDataContainerType& session_data_r =
    const_cast<SessionDataContainerType&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // *TODO*: remove type inference
      frameSize_ = session_data_r.frameSize;
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
          typename SessionDataContainerType>
bool
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::frame (const struct RIFF_chunk_meta& chunk_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::frame"));

  int result = -1;
  ACE_Message_Block *message_block_p, *message_block_2 = NULL;
  ACE_Message_Block* message_block_3 = NULL; // new buffer top
  ACE_UINT64 skipped_bytes = 0;
  Stream_Decoder_RIFFChunksIterator_t iterator;
  ACE_UINT32 chunk_size_i =
    chunk_in.size % 2 ? chunk_in.size + 1 : chunk_in.size;

  if (headerParsed_)
    goto dispatch;

  headerParsed_ = true;

  // find offset of the first (frame) data chunk
  ACE_ASSERT (!inherited2::chunks_.empty ());
  struct RIFF_chunk_meta temp;
  temp.riff_list_identifier = FOURCC ('m', 'o', 'v', 'i');
  //  iterator = driver_.chunks_.find (temp);
  iterator =
    std::find (inherited2::chunks_.begin (), inherited2::chunks_.end (), temp);
  ACE_ASSERT (iterator != inherited2::chunks_.end ());

  // discard all header buffers up until the first frame
  //  4 + 4 + 4 + // RIFF
  //  4 + 4 + 4 + // hdrl
  //  4 + (4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + (4 * 4)) + // avih
  //  (struct _avimainheader) 4 + 4 + 4 + // strl 4 + (4 + 4 + 4 + 4 + 4 + 2 + 2
  //  + 4 + 4 + 4 + 4 + 4 + 4 + 4 + 4 + (4 * 2)) + // strh (struct
  //  _avistreamheader) 4 + (4 + 4 + 4 + 2 + 2 + 4 + 4 + 4 + 4 + 4 + 4 + (4 *
  //  1)) + // strf (struct tagBITMAPINFO)
  //  ...
  Stream_Tools::skip ((*iterator).offset + 4 + 4 + 4, // up to and including 'movi' header
                      buffer_,
                      false); // release skipped bytes, if possible

dispatch:
  // *NOTE*: AVI header has been parsed
  ACE_UINT64 bytes_to_skip = 0;

  // sanity check(s)
  ACE_ASSERT (buffer_);
  size_t buffered_bytes = buffer_->total_length ();
  ACE_ASSERT (buffered_bytes >= chunk_size_i + 4 + 4);

  message_block_p = NULL;
  if (buffered_bytes == chunk_size_i + 4 + 4)
  {
    message_block_p = buffer_;
    buffer_ = NULL;
    goto dispatch_2;
  } // end IF

  // --> (buffered_bytes > frame size + 4 + 4)
  message_block_p = buffer_;
  skipped_bytes = 0;

  message_block_2 = message_block_p;
  do
  {
    skipped_bytes += message_block_2->length ();
    if (skipped_bytes >= chunk_size_i + 4 + 4)
      break;
    message_block_2 = message_block_2->cont ();
    ACE_ASSERT (message_block_2);
  } while (true);
  if (skipped_bytes > chunk_size_i + 4 + 4)
  {
    message_block_3 = message_block_2->duplicate ();
    if (unlikely (!message_block_3))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to MessageType::duplicate(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF

    bytes_to_skip =
      message_block_2->length () - (skipped_bytes - (chunk_size_i + 4 + 4));
    message_block_3->rd_ptr (bytes_to_skip);
    message_block_2->length (bytes_to_skip);
    if (message_block_2->cont ())
      message_block_2->cont ()->release ();
    message_block_2->cont (NULL);
    buffer_ = message_block_3;
  } // end IF

dispatch_2:
  ACE_ASSERT (message_block_p->total_length () == chunk_size_i + 4 + 4);

  // remove AVI frame header (00db, ...)
  Stream_Tools::skip (4 + 4,
                      message_block_p,
                      false); // release skipped bytes, if possible
  ACE_ASSERT (message_block_p->total_length () == chunk_size_i);

  result = inherited::put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
    return false;
  } // end IF

  if (likely (buffer_))
  {
    RIFF_Scanner_reset_hold_char (inherited2::scannerState_);
    inherited2::switchBuffer (buffer_);
    return true;  
  } // end IF

  inherited2::fragment_ = NULL;
  inherited2::wait ();
  if (!inherited2::fragment_)
    return false; // done

  buffer_ = inherited2::fragment_;
  RIFF_Scanner_reset_hold_char (inherited2::scannerState_);
  inherited2::switchBuffer (buffer_);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_AVIDecoder_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::betweenFrameChunk (const struct RIFF_chunk_meta& chunk_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::betweenFrameChunk"));

  // skip over chunk bytes and resume
  Stream_Tools::skip (4 + 4 + chunk_in.size,
                      buffer_,
                      false); // release skipped bytes, if possible

  RIFF_Scanner_reset_hold_char (inherited2::scannerState_);
  inherited2::switchBuffer (buffer_);

  return true;
}
