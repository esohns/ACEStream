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

#include "stream_macros.h"

#include "stream_dec_defines.h"

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_AVIDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::Stream_Decoder_AVIDecoder_T ()
 : inherited ()
 , driver_ (STREAM_DECODER_DEFAULT_LEX_TRACE,
            STREAM_DECODER_DEFAULT_YACC_TRACE)
 , allocator_ (NULL)
 , buffer_ (NULL)
 , crunchMessages_ (STREAM_DECODER_DEFAULT_CRUNCH_MESSAGES)
 , sessionData_ (NULL)
 , debugScanner_ (STREAM_DECODER_DEFAULT_LEX_TRACE)
 , debugParser_ (STREAM_DECODER_DEFAULT_YACC_TRACE)
 , isDriverInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::Stream_Decoder_AVIDecoder_T"));

}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_AVIDecoder_T<SynchStrategyType,
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

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_AVIDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.streamConfiguration);

  if (inherited::isInitialized_)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("re-initializing...\n")));

    //allocator_ = NULL;
    if (buffer_)
      buffer_->release ();
    buffer_ = NULL;
    crunchMessages_ = STREAM_DECODER_DEFAULT_CRUNCH_MESSAGES;
    sessionData_ = NULL;

    debugScanner_ = STREAM_DECODER_DEFAULT_LEX_TRACE;
    debugParser_ = STREAM_DECODER_DEFAULT_YACC_TRACE;
    isDriverInitialized_ = false;

    inherited::isInitialized_ = false;
  } // end IF

  allocator_ = configuration_in.streamConfiguration->messageAllocator;
  crunchMessages_ = configuration_in.crunchMessages;

  debugScanner_ = configuration_in.traceScanning;
  debugParser_ = configuration_in.traceParsing;

  return inherited::initialize (configuration_in);
}
//template <typename SynchStrategyType,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ModuleHandlerConfigurationType,
//          typename SessionDataContainerType>
//const ConfigurationType&
//Stream_Decoder_AVIDecoder_T<SynchStrategyType,
//                            TimePolicyType,
//                            ConfigurationType,
//                            ControlMessageType,
//                            DataMessageType,
//                            SessionMessageType,
//                            ModuleHandlerConfigurationType,
//                            SessionDataContainerType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::get"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (ConfigurationType ());
//
//  ACE_NOTREACHED (return ConfigurationType ();)
//}

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_AVIDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (sessionData_);

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

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

  if (sessionData_->frameSize)
  {
begin:
    // AVI header has been parsed
    // --> wait for more data ?
    unsigned int buffered_bytes = buffer_->total_length ();
    if (buffered_bytes < sessionData_->frameSize)
      return; // done

    // forward any accumulated frame(s)
    message_block_p = buffer_;



    ACE_ASSERT (buffer_->total_length () == sessionData_->frameSize);
    result = inherited::put_next (buffer_, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

      // clean up
      message_block_p->release ();
      buffer_->release ();
      buffer_ = NULL;

      return;
    } // end IF
    message_block_p->rd_ptr (buffered_bytes - sessionData_->frameSize);
    buffer_ = message_block_p;

    return;
  } // end IF

  // "crunch" messages for easier parsing ?
  if (crunchMessages_ &&
      buffer_->cont ())
  {
    //     message->dump_state();

    // step1: get a new message buffer
    DataMessageType* message_p = allocateMessage (STREAM_DECODER_BUFFER_SIZE);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to allocate message(%u), returning\n"),
                  STREAM_DECODER_BUFFER_SIZE));
      goto error;
    } // end IF

    // step2: copy available data
    for (ACE_Message_Block* message_block_p = buffer_;
         message_block_p;
         message_block_p = message_block_p->cont ())
    {
      ACE_ASSERT (message_block_p->length () <= message_p->space ());
      result = message_p->copy (message_block_p->rd_ptr (),
                                message_block_p->length ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

        // clean up
        message_p->release ();

        goto error;
      } // end IF
    } // end FOR

      // clean up
    buffer_->release ();
    buffer_ = message_p;
  } // end IF

  // initialize driver ?
  if (!isDriverInitialized_)
  {
    // *TODO*: remove type inference
    driver_.initialize (sessionData_->frameSize,
                        true,                    // parse header only
                        debugScanner_,
                        debugParser_,
                        inherited::msg_queue (),
                        crunchMessages_);
    isDriverInitialized_ = true;
  } // end IF

  // OK: parse this message

  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
  //              message_p->id (),
  //              message_p->length ()));

  if (!driver_.parse (buffer_))
  { // *NOTE*: most probable cause: need more data
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("failed to HTTP_ParserDriver::parse() (message ID: %d), returning\n"),
    //                message_p->id ()));
    goto done;
  } // end IF

  // *NOTE*: the (chained) fragment(s) has/have been parsed, the read pointer
  //         has been advanced to the first chunk data
  // *TODO*: the current parser cannot parse only the header
  //         --> make it left-recursive
  ACE_ASSERT (sessionData_->frameSize);

  goto begin;

done:
  return;

error:
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
Stream_Decoder_AVIDecoder_T<SynchStrategyType,
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
    message_inout->get ();
  SessionDataContainerType& session_data_r =
    const_cast<SessionDataContainerType&> (session_data_container_r.get ());
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

template <typename SynchStrategyType,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
DataMessageType*
Stream_Decoder_AVIDecoder_T<SynchStrategyType,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionDataContainerType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_AVIDecoder_T::allocateMessage"));

  // initialize return value(s)
  DataMessageType* message_p = NULL;

  if (allocator_)
  {
allocate:
    try {
      message_p =
        static_cast<DataMessageType*> (allocator_->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  requestedSize_in));
      return NULL;
    }

    // keep retrying ?
    if (!message_p && !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      DataMessageType (requestedSize_in));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
  } // end IF

  return message_p;
}
