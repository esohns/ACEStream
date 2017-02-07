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
#include "stream_dec_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::Stream_Decoder_H264_NAL_Decoder_T ()
 : inherited ()
 , buffer_ (NULL)
 , isFirst_ (true)
 , scannerState_ (NULL)
 //, scannerTables_ (scannerTables_in)
 , bufferState_ (NULL)
 , useYYScanBuffer_ (STREAM_DECODER_FLEX_DEFAULT_USE_YY_SCAN_BUFFER)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::Stream_Decoder_H264_NAL_Decoder_T"));

  int result = -1;

  // step1: initialize flex state
  result = Stream_Decoder_H264_NAL_Bisector_lex_init_extra (this,
                                                            &scannerState_);
  if (result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_init_extra: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (scannerState_);

//  // step2: load tables ?
//  FILE* file_p = NULL;
//  if (!scannerTables_.empty ())
//  {
//    std::string filename_string;
//    std::string package_name;
//#ifdef HAVE_CONFIG_H
//    package_name = ACE_TEXT_ALWAYS_CHAR (LIBACENETWORK_PACKAGE_NAME);
//#else
//#error "package name not available, set HAVE_CONFIG_H"
//#endif
//#if defined (DEBUG_DEBUGGER)
//  filename_string = Common_File_Tools::getWorkingDirectory ();
//  filename_string += ACE_DIRECTORY_SEPARATOR_STR;
//  filename_string += ACE_TEXT_ALWAYS_CHAR ("../../src/protocol/http");
//#else
//    filename_string =
//        Common_File_Tools::getConfigurationDataDirectory (package_name,
//                                                          true);
//#endif // #ifdef DEBUG_DEBUGGER
//    filename_string += ACE_DIRECTORY_SEPARATOR_STR;
//    filename_string +=
//        ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_DIRECTORY);
//    filename_string += ACE_DIRECTORY_SEPARATOR_STR;
//    filename_string += scannerTables_;
//    file_p = ACE_OS::fopen (filename_string.c_str (),
//                            ACE_TEXT_ALWAYS_CHAR ("rb"));
//    if (!file_p)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::fopen(\"%s\"): \"%m\", returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));
//      return;
//    } // end IF
//    result = HTTP_Scanner_tables_fload (file_p,
//                                        scannerState_);
//    if (result)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to yy_tables_fload(\"%s\"): \"%m\", returning\n"),
//                  ACE_TEXT (filename_string.c_str ())));

//      // clean up
//      result = ACE_OS::fclose (file_p);
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_OS::fclose(\"%s\"): \"%m\", continuing\n"),
//                    ACE_TEXT (filename_string.c_str ())));

//      return;
//    } // end IF
//    result = ACE_OS::fclose (file_p);
//    if (result == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_OS::fclose(\"%s\"): \"%m\", continuing\n"),
//                  ACE_TEXT (filename_string.c_str ())));

//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("loaded \"%s\"...\n"),
//                ACE_TEXT (scannerTables_.c_str ())));
//  } // end IF

  // trace ?
  Stream_Decoder_H264_NAL_Bisector_set_debug (COMMON_PARSER_DEFAULT_LEX_TRACE,
                                              scannerState_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::~Stream_Decoder_H264_NAL_Decoder_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::~Stream_Decoder_H264_NAL_Decoder_T"));

  if (buffer_)
    buffer_->release ();

//  int result = -1;

//  // finalize lex scanner
//  if (!scannerTables_.empty ())
//  {
//    result = HTTP_Scanner_tables_destroy (scannerState_);
//    if (result)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to yy_tables_destroy(): \"%m\", continuing\n")));
//  } // end IF

  if (Stream_Decoder_H264_NAL_Bisector_lex_destroy (scannerState_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to yylex_destroy(): \"%m\", continuing\n")));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                         Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->activate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::activate(): \"%m\", continuing\n")));

    if (buffer_)
    {
      buffer_->release ();
      buffer_ = NULL;
    } // end IF
    isFirst_ = true;

    //if (!scannerTables_.empty ())
    //{
    //  result = HTTP_Scanner_tables_destroy (scannerState_);
    //  if (result)
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to yy_tables_destroy(): \"%m\", continuing\n")));
    //  scannerTables_ = scannerTables_in;
    //} // end IF

    if (bufferState_)
    { ACE_ASSERT (scannerState_);
      Stream_Decoder_H264_NAL_Bisector__delete_buffer (bufferState_,
                                                       scannerState_);
      bufferState_ = NULL;
    } // end IF
    //if (scannerState_)
    //{
    //  if (HTTP_Scanner_lex_destroy (scannerState_))
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to yylex_destroy: \"%m\", continuing\n")));
    //  scannerState_ = NULL;
    //} // end IF

    useYYScanBuffer_ = STREAM_DECODER_FLEX_DEFAULT_USE_YY_SCAN_BUFFER;
  } // end IF

  useYYScanBuffer_ = configuration_in.useYYScanBuffer;

  Stream_Decoder_H264_NAL_Bisector_set_debug ((configuration_in.debugScanner ? 1 : 0),
                                              scannerState_);

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
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = NULL;
  bool do_scan_end = false;
  unsigned int offset = (isFirst_ ? 0 : 3);

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // sanity check(s)
  ACE_ASSERT (!buffer_);

  buffer_ = message_inout;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (buffer_->capacity () - buffer_->length () >=
              STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(buffer_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(buffer_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  message_block_p = buffer_;

scan:
  if (!scan_begin (message_block_p->rd_ptr () + offset,
                   message_block_p->length () - offset))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Decoder_H264_NAL_Decoder_T::scan_begin(), aborting\n")));
    goto error;
  } // end IF
  do_scan_end = true;

  // initialize scanner ?
  if (isFirst_)
  { /* column is only valid if an input buffer exists. */
    Stream_Decoder_H264_NAL_Bisector_set_column (1, scannerState_);
    Stream_Decoder_H264_NAL_Bisector_set_lineno (1, scannerState_);
  } // end IF

  // parse data fragment
  try {
    result = Stream_Decoder_H264_NAL_Bisector_lex (scannerState_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in yylex(), continuing\n")));
    result = 1;
  }
  switch (result)
  {
    case -1:
    {
      // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to bisect H264 NAL units (result was: %d), aborting\n"),
                  result));
      goto error;
    }
    default:
    {
      // frame the NAL unit
      unsigned int remaining_bytes = result + offset;
      unsigned int length = 0;
      unsigned int trailing_bytes = 0;
      message_block_p = buffer_;

      if (isFirst_)
      {
        isFirst_ = false;

        while (remaining_bytes)
        {
          length = message_block_p->length ();
          if (length > remaining_bytes)
          {
            trailing_bytes = length - remaining_bytes;
            length = remaining_bytes;
          } // end IF
          remaining_bytes -= length;
          message_block_p->rd_ptr (length);
          if (remaining_bytes)
          {
            message_block_p = message_block_p->cont ();
            ACE_ASSERT (message_block_p);
          } // end IF
        } // end WHILE

        // clean up
        scan_end ();
        do_scan_end = false;

        if (trailing_bytes)
        {
          offset = 3;
          goto scan;
        } // end IF

        break;
      } // end IF

      while (remaining_bytes)
      {
        length = message_block_p->length ();
        if (length > remaining_bytes)
        {
          trailing_bytes = length - remaining_bytes;
          length = remaining_bytes;
        } // end IF
        remaining_bytes -= length;
        if (remaining_bytes)
        {
          message_block_p = message_block_p->cont ();
          ACE_ASSERT (message_block_p);
        } // end IF
      } // end WHILE

      if (trailing_bytes)
      {
        message_block_2 = message_block_p->duplicate ();
        if (!message_block_2)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF

        message_block_p->wr_ptr (message_block_p->rd_ptr () +
                                 (message_block_p->length () - trailing_bytes));
        length = message_block_p->length ();
        ACE_ASSERT (length < 150000);
        message_block_2->rd_ptr (message_block_p->length ());
      } // end IF
      else
        message_block_2 = NULL;

      result = inherited::put_next (buffer_, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      buffer_ = message_block_2;

      // clean up
      scan_end ();
      do_scan_end = false;

      if (trailing_bytes)
      {
        message_block_p = buffer_;
        remaining_bytes = 3;
        do
        {
          length = message_block_p->length ();
          if (length > remaining_bytes)
            break;
          remaining_bytes -= length;
          if (remaining_bytes)
          {
            message_block_p = message_block_p->cont ();
            if (!message_block_p)
              break;
          } // end IF
        } while (true);

        if (!message_block_p)
          break;
        else
          offset = remaining_bytes;
        goto scan;
      } // end IF

      break;
    }
  } // end SWITCH

  return;

error:
  if (do_scan_end)
    scan_end ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::handleSessionMessage"));

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
      goto continue_;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;

continue_:
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        buffer_->release ();
        buffer_ = NULL;
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
          typename SessionDataContainerType>
void
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::error (const std::string& message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTP_ParserDriver_T::error"));

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("\": \"%s\"...\n"),
              ACE_TEXT (message_in.c_str ())));
//   ACE_DEBUG((LM_ERROR,
//              ACE_TEXT("failed to parse \"%s\": \"%s\"...\n"),
//              std::string(fragment_->rd_ptr(), fragment_->length()).c_str(),
//              message_in.c_str()));

//   std::clog << message_in << std::endl;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::switchBuffer (bool unlink_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::switchBuffer"));

  ACE_UNUSED_ARG (unlink_in);

  // sanity check(s)
  ACE_ASSERT (buffer_);
  ACE_ASSERT (scannerState_);

  // retrieve trailing chunk
  ACE_Message_Block* message_block_p = buffer_;
  ACE_Message_Block* message_block_2 = NULL;
  do
  {
    message_block_2 = message_block_p->cont ();
    if (message_block_2)
      message_block_p = message_block_2;
    else
      break;
  } while (true);

  ACE_ASSERT (!message_block_p->cont ());
  waitBuffer (); // <-- wait for data
  message_block_2 = message_block_p->cont ();
  if (!message_block_2)
  {
    // *NOTE*: most probable reason: received session end
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no data after Stream_Decoder_H264_NAL_Decoder_T::waitBuffer(), aborting\n")));
    return false;
  } // end IF

  // switch to the next fragment

  // clean state
  scan_end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_block_2->capacity () - message_block_2->length () >=
              STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_block_2->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_block_2->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!scan_begin (message_block_2->rd_ptr (),
                   message_block_2->length ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Decoder_H264_NAL_Decoder_T::scan_begin(), aborting\n")));
    return false;
  } // end IF

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("switched input buffers...\n")));

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::waitBuffer ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool done = false;
  SessionMessageType* session_message_p = NULL;
  enum Stream_SessionMessageType session_message_type =
      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data = false;
  bool stop_processing = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  //// sanity check(s)
  //ACE_ASSERT (blockInParse_);

  // 1. wait for data
  do
  {
    result = inherited::msg_queue_->dequeue_head (message_block_p,
                                                  NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::dequeue_head(): \"%m\", returning\n")));
      return;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_DATA:
      case ACE_Message_Block::MB_PROTO:
        is_data = true; break;
      case ACE_Message_Block::MB_USER:
      {
        session_message_p = dynamic_cast<SessionMessageType*> (message_block_p);
        if (session_message_p)
        {
          switch (session_message_p->type ())
          {
            case STREAM_SESSION_MESSAGE_END:
            {
              done = true; // session has finished --> abort

              break;
            }
            default:
            {
              inherited::handleMessage (session_message_p,
                                        stop_processing);
              if (stop_processing)
                done = true; // session has finished (error) --> abort
              message_block_p = NULL;
              break;
            }
          } // end SWITCH
        } // end IF
        break;
      }
      default:
        break;
    } // end SWITCH
    if (is_data) break;

    // requeue message ?
    if (message_block_p)
    {
      result = inherited::msg_queue_->enqueue_tail (message_block_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n")));
        return;
      } // end IF
      message_block_p = NULL;
    } // end IF
  } while (!done);

  // 2. append data ?
  if (message_block_p)
  {
    // sanity check(s)
    ACE_ASSERT (buffer_);

    ACE_Message_Block* message_block_2 = buffer_;
    for (;
         message_block_2->cont ();
         message_block_2 = message_block_2->cont ());
    message_block_2->cont (message_block_p);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
bool
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::scan_begin (const char* data_in,
                                                                         unsigned int length_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::scan_begin"));

  // sanity check(s)
  ACE_ASSERT (!bufferState_);
  ACE_ASSERT (scannerState_);
  ACE_ASSERT (data_in);

  // create/initialize a new buffer state
  if (useYYScanBuffer_)
  {
    bufferState_ =
      Stream_Decoder_H264_NAL_Bisector__scan_buffer (const_cast<char*> (data_in),
                                                     length_in + STREAM_DECODER_FLEX_BUFFER_BOUNDARY_SIZE,
                                                     scannerState_);
  } // end IF
  else
  {
    bufferState_ =
      Stream_Decoder_H264_NAL_Bisector__scan_bytes (data_in,
                                                    length_in,
                                                    scannerState_);
  } // end ELSE
  if (!bufferState_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to yy_scan_buffer/bytes(0x%@, %u): \"%m\", aborting\n"),
                inherited::mod_->name (),
                data_in,
                length_in));
    return false;
  } // end IF
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing fragment #%d --> %d byte(s)...\n"),
//              counter++,
//              fragment_->length ()));

//  // *WARNING*: contrary (!) to the documentation, still need to switch_buffers()...
//  HTTP_Scanner__switch_to_buffer (bufferState_,
//                                  scannerState_);

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType>
void
Stream_Decoder_H264_NAL_Decoder_T<ACE_SYNCH_USE,
                                  TimePolicyType,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  SessionDataContainerType>::scan_end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_H264_NAL_Decoder_T::scan_end"));

  // sanity check(s)
  ACE_ASSERT (bufferState_);

  // clean state
  Stream_Decoder_H264_NAL_Bisector__delete_buffer (bufferState_,
                                                   scannerState_);
  bufferState_ = NULL;
}
