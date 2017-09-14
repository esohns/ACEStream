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

#include "stream_misc_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                          UserDataType>::Stream_Module_CppParser_T (ISTREAM_T* stream_in,
#else
                          UserDataType>::Stream_Module_CppParser_T (typename inherited::ISTREAM_T* stream_in,
                                                                    bool traceScanning_in,
                                                                    bool traceParsing_in)
#endif
 : inherited (stream_in)
 , configuration_ (NULL)
 , fragment_ (NULL)
 , offset_ (0)
 , trace_ (traceParsing_in)
 , parser_ (dynamic_cast<ParserInterfaceType*> (this), // parser
            &scanner_)                                 // scanner
// , argument_ ()
 , scanner_ ()
 , scannerState_ ()
 , blockInParse_ (false)
 , isFirst_ (true)
 , buffer_ (NULL)
 , streamBuffer_ ()
 , stream_ (&streamBuffer_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::Stream_Module_CppParser_T"));

  scanner_.setP (this);

  scanner_.set_debug (traceScanning_in ? 1 : 0);
  parser_.set_debug_level (traceParsing_in ? 1 : 0);
#if YYDEBUG
//  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::~Stream_Module_CppParser_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::~Stream_Module_CppParser_T"));

  // finalize lex scanner
  if (buffer_)
    scanner_.yy_delete_buffer (buffer_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::initialize"));

  if (inherited::isInitialized_)
  {
    configuration_ = NULL;
    fragment_ = NULL;
    offset_ = 0;
    trace_ = STREAM_MISC_PARSER_DEFAULT_YACC_TRACE;

    blockInParse_ = false;
    isFirst_ = true;

    if (buffer_)
    {
      scanner_.yy_delete_buffer (buffer_);
      buffer_ = NULL;
    } // end IF
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);
  configuration_ = configuration_in.parserConfiguration;
  trace_ = configuration_->debugParser;

  blockInParse_ = configuration_->block;

  scanner_.set_debug (configuration_->debugScanner ? 1 : 0);
#if defined (YYDEBUG)
  parser_.set_debug_level (configuration_->debugParser ? 1 : 0);
//  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  try {
    message_inout->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IDumpState::dump_state(), continuing\n"),
                ACE_TEXT (inherited::mod_->name ())));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
      break;
    case STREAM_SESSION_MESSAGE_END:
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
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::parse (ACE_Message_Block* data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::parse"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (data_in);

  // retain a handle to the 'current' fragment
  fragment_ = data_in;
  offset_ = 0;

  int result = -1;
  bool do_scan_end = false;
  if (!begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IScannerBase::begin(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  do_scan_end = true;

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;

//    bittorrent_set_column (1, state_);
//    bittorrent_set_lineno (1, state_);
  } // end IF

  // parse data fragment
  try {
    result = parser_.parse ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in ParserType::parse(), continuing\n"),
                inherited::mod_->name ()));
    result = 1;
  }
  switch (result)
  {
    case 0:
      break; // done/need more data
    case 1:
    default:
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to parse PDU (result was: %d), aborting\n"),
                  inherited::mod_->name (),
                  result));
      goto error;
    }
  } // end SWITCH

  // finalize buffer/scanner
  end ();
  do_scan_end = false;

  goto continue_;

error:
  if (do_scan_end)
    end ();
  fragment_ = NULL;

continue_:
  return (result == 0);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::error (const std::string& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::error"));

//  std::ostringstream converter;
//  converter << location_in;

  // *NOTE*: the output format has been "adjusted" to fit in with bison error-reporting
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("(@%d.%d-%d.%d): %s\n"),
//              location_in.begin.line, location_in.begin.column,
//              location_in.end.line, location_in.end.column,
//              ACE_TEXT (message_in.c_str ())));
  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("failed to parse \"%s\" (@%s): \"%s\"\n"),
              ACE_TEXT ("%s: failed to Common_IParser_T::parse(): \"%s\"\n"),
              inherited::mod_->name (),
//              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
//              converter.str ().c_str (),
              message_in.c_str ()));

  // dump message
  ACE_Message_Block* message_block_p = fragment_;
  while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
  ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (message_block_p);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IDumpState::dump_state(), continuing\n"),
                inherited::mod_->name ()));
  }

  //std::clog << location_in << ": " << message_in << std::endl;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::switchBuffer (bool unlink_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::switchBuffer"));

  // sanity check(s)
  ACE_ASSERT (fragment_);

  ACE_Message_Block* message_block_p = fragment_;
  if (!fragment_->cont ())
  {
    // sanity check(s)
    if (!blockInParse_)
      return false; // not enough data, cannot proceed

    waitBuffer (); // <-- wait for data
    if (!fragment_->cont ())
    {
      // *NOTE*: most probable reason: received session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: no data after Common_IScannerBase::waitBuffer(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end IF
  fragment_ = fragment_->cont ();
  offset_ = 0;

  // unlink ?
  if (unlink_in)
    message_block_p->cont (NULL);

  // switch to the next fragment

  // clean state
  end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (fragment_->capacity () - fragment_->length () >= STREAM_MISC_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(fragment_->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(fragment_->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!begin ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IScannerBase::begin(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::waitBuffer ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Queue_Base* message_queue_p = inherited::msg_queue ();
  bool done = false;
  SessionMessageType* session_message_p = NULL;
  enum Stream_SessionMessageType session_message_type =
      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (blockInParse_);
  if (!message_queue_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: message queue not set - cannot wait, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // 1. wait for data
  do
  {
    result = message_queue_p->dequeue_head (message_block_p,
                                            NULL);
    if (result == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ESHUTDOWN)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue::dequeue_head(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
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
          session_message_type = session_message_p->type ();
          if (session_message_type == STREAM_SESSION_MESSAGE_END)
            done = true; // session has finished --> abort
        } // end IF
        break;
      }
      default:
        break;
    } // end SWITCH
    if (is_data) break;

    // requeue message
    result = message_queue_p->enqueue_tail (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF

    message_block_p = NULL;
  } while (!done);

  // 2. append data ?
  if (message_block_p)
  {
    // sanity check(s)
    ACE_ASSERT (fragment_);

    ACE_Message_Block* message_block_2 = fragment_;
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
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::begin (const char* buffer_in,
                                                unsigned int bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::begin"));

//  static int counter = 1;

  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);

  // sanity check(s)
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  streamBuffer_.set (fragment_->rd_ptr (),
                     fragment_->length () + STREAM_MISC_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
//  stream_.rdbuf (&streamBuffer_);
  scanner_.switch_streams (&stream_, NULL);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing fragment #%d --> %d byte(s)\n"),
//              counter++,
//              fragment_->length ()));

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_CppParser_T<ACE_SYNCH_USE,
                          TimePolicyType,
                          ConfigurationType,
                          ControlMessageType,
                          DataMessageType,
                          SessionMessageType,
                          ScannerType,
                          ScannerStateType,
                          ParserType,
                          ParserConfigurationType,
                          ParserInterfaceType,
                          ParserArgumentType,
                          UserDataType>::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::end"));

  // sanity check(s)
  if (!buffer_)
    return;

  // clean state
  scanner_.yy_delete_buffer (buffer_);
  buffer_ = NULL;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                       UserDataType>::Stream_Module_Parser_T (ISTREAM_T* stream_in,
#else
                       UserDataType>::Stream_Module_Parser_T (typename inherited::ISTREAM_T* stream_in,
                                                              bool traceScanning_in,
                                                              bool traceParsing_in)
#endif
 : inherited (stream_in)
 , configuration_ (NULL)
 , fragment_ (NULL)
 , isFirst_ (true)
 , offset_ (0)
 , trace_ (traceParsing_in)
// , parser_ (static_cast<ParserInterfaceType*> (this), // parser
//            NULL)                                     // scanner
// , argument_ ()
 , buffer_ (NULL)
 , state_ (NULL)
 , scannerState_ ()
 , useYYScanBuffer_ (STREAN_MISC_PARSER_FLEX_USE_YY_SCAN_BUFFER)
 , blockInParse_ (false)
 , streamBuffer_ ()
 , stream_ (&streamBuffer_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::Stream_Module_Parser_T"));

  ACE_UNUSED_ARG (traceScanning_in);

//  parser_.set_debug_level (traceParsing_in ? 1 : 0);
#if YYDEBUG
//  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::~Stream_Module_Parser_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::~Stream_Module_Parser_T"));

  // finalize lex scanner
  if (buffer_)
  {
    ACE_ASSERT (state_);
    try {
      this->destroy (state_,
                     buffer_);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::destroy(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF

  if (state_)
  {
    try {
      this->finalize (state_);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::finalize(): \"%m\", continuing\n"),
                  inherited::mod_->name ()));
    }
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::initialize"));

  if (inherited::isInitialized_)
  {
    configuration_ = NULL;
    fragment_ = NULL;
    isFirst_ = true;
    offset_ = 0;
    trace_ = STREAM_MISC_PARSER_DEFAULT_YACC_TRACE;

    blockInParse_ = false;

    if (buffer_)
    {
      ACE_ASSERT (state_);
      try {
        this->destroy (state_,
                       buffer_);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::destroy(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      }
      buffer_ = NULL;
    } // end IF

    if (state_)
    {
      try {
        this->finalize (state_);
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::finalize(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      }
      state_ = NULL;
    } // end IF
    useYYScanBuffer_ = STREAN_MISC_PARSER_FLEX_USE_YY_SCAN_BUFFER;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  configuration_ = configuration_in.parserConfiguration;
  trace_ = configuration_->debugParser;

  blockInParse_ = configuration_->block;

  bool result = false;
  try {
    result = this->initialize (state_,
                               &scannerState_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::initialize(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
  }
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_ILexScanner_T::initialize(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  ACE_ASSERT (state_);
//  parser_.set (state_);

  ACE_ASSERT (inherited::msg_queue_);
  result = inherited::msg_queue_->activate ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // trace ?
  try {
    this->debug (state_,
                 configuration_->debugScanner);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::debug(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
  }
#if YYDEBUG
  parser_.set_debug_level (trace_ ? 1 : 0);
  //  yydebug = (trace_ ? 1 : 0);
//  yysetdebug (trace_ ? 1 : 0);
#endif

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::handleDataMessage"));

  int result = -1;

  // initialize return value(s)
  // *NOTE*: the default behavior is to pass all messages along
  //         --> in this case, the individual frames are extracted and passed
  //             as such
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_inout->capacity () - message_inout->length () >= STREAM_MISC_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  result = inherited::msg_queue_->enqueue_tail (message_inout,
                                                NULL);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

    // clean up
    message_inout->release ();
    message_inout = NULL;

    return;
  } // end IF
  message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      result = inherited::activate ();
      if (result == -1)
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
      if (inherited::thr_count_)
        inherited::stop (false, // wait for completion ?
                         true); // locked access ?
      else
      {
        ACE_ASSERT (inherited::msg_queue_);
        result = inherited::msg_queue_->deactivate ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end ELSE

      if (buffer_)
      {
        ACE_ASSERT (state_);
        try {
          this->destroy (state_,
                         buffer_);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Common_ILexScanner_T::destroy(): \"%m\", continuing\n")));
        }
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
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::parse (ACE_Message_Block* data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::parse"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (state_);
  ACE_ASSERT (data_in);

  // retain a handle to the 'current' fragment
  fragment_ = dynamic_cast<DataMessageType*> (data_in);
  offset_ = 0;

  // sanity check(s)
  ACE_ASSERT (fragment_);

  int result = -1;
  bool do_scan_end = false;
  if (!begin (NULL, 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IScannerBase::begin(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF
  do_scan_end = true;

  // initialize scanner ?
  if (isFirst_)
  {
    isFirst_ = false;

//    bittorrent_set_column (1, state_);
//    bittorrent_set_lineno (1, state_);
  } // end IF

  // parse data fragment
  try {
//    result = parser_.parse (fragment_);
    this->lex ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::lex(), continuing\n"),
//                ACE_TEXT ("%s: caught exception in Common_IParser_T::parse(), continuing\n"),
                inherited::mod_->name ()));
    result = 1;
  }
  switch (result)
  {
    case 0:
      break; // done/need more data
    case 1:
    default:
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to parse PDU (result was: %d), aborting\n"),
                  inherited::mod_->name (),
                  result));
      goto error;
    }
  } // end SWITCH

  // finalize buffer/scanner
  end ();
  do_scan_end = false;

  goto continue_;

error:
  if (do_scan_end)
    end ();
  fragment_ = NULL;

continue_:
  return (result == 0);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::error (const std::string& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::error"));

  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("%s: failed to Common_IParser_T::parse(): \"%s\"\n"),
              ACE_TEXT ("%s: failed to Common_ILexScanner_T::lex(): \"%s\"\n"),
              inherited::mod_->name (),
//              std::string (fragment_->rd_ptr (), fragment_->length ()).c_str (),
              message_in.c_str ()));

  // dump message
  ACE_Message_Block* message_block_p = fragment_;
  while (message_block_p->prev ()) message_block_p = message_block_p->prev ();
  ACE_ASSERT (message_block_p);
  Common_IDumpState* idump_state_p =
    dynamic_cast<Common_IDumpState*> (message_block_p);
  ACE_ASSERT (idump_state_p);
  try {
    idump_state_p->dump_state ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_IDumpState::dump_state(), continuing\n"),
                inherited::mod_->name ()));
  }
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::switchBuffer (bool unlink_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::switchBuffer"));

  ACE_UNUSED_ARG (unlink_in);

  // sanity check(s)
  ACE_ASSERT (state_);

  ACE_Message_Block* message_block_p = fragment_;
  ACE_Message_Block* message_block_2 = NULL;

  // retrieve trailing chunk
  if (!fragment_)
    goto continue_;

  do
  {
    message_block_2 = message_block_p->cont ();
    if (message_block_2)
      message_block_p = message_block_2;
    else
      break;
  } while (true);
  ACE_ASSERT (!message_block_p->cont ());

continue_:
  waitBuffer (); // <-- wait for data

  message_block_2 = message_block_p ? message_block_p->cont ()
                                    : fragment_;
  if (!message_block_2)
  {
    // *NOTE*: most probable reason: received session end
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: no data after Common_IScannerBase::waitBuffer(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  // switch to the next fragment

  // clean state
  end ();

  // initialize next buffer

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_block_2->capacity () - message_block_2->length () >= STREAM_MISC_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_block_2->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_block_2->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  if (!begin (message_block_2->rd_ptr (),
              message_block_2->length ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_IScannerBase::begin(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::waitBuffer ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::waitBuffer"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  bool done = false;
  SessionMessageType* session_message_p = NULL;
//  enum Stream_SessionMessageType session_message_type =
//      STREAM_SESSION_MESSAGE_INVALID;
  bool is_data = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);
//  ACE_ASSERT (blockInParse_);

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
                    ACE_TEXT ("%s: failed to ACE_Message_Queue::dequeue_head(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
      return;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_DATA:
      case ACE_Message_Block::MB_PROTO:
        is_data = true;
        break;
      case ACE_Message_Block::MB_STOP:
        done = true; // session has finished --> abort
        break;
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
              bool stop_processing = false;
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
    if (is_data)
      break;

    // requeue message ?
    if (message_block_p)
    {
      result = inherited::msg_queue_->enqueue_tail (message_block_p, NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
      message_block_p = NULL;
    } // end IF
  } while (!done);
  if (!message_block_p)
    return;

  // 2. append data ?
  if (message_block_p)
  {
    // sanity check(s)
    ACE_ASSERT (fragment_);

    ACE_Message_Block* message_block_2 = fragment_;
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
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
bool
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::begin (const char* buffer_in,
                                             unsigned int bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::begin"));

//  static int counter = 1;

  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);

  // sanity check(s)
  ACE_ASSERT (!buffer_);
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  try {
    buffer_ = this->create (state_,
                            fragment_->rd_ptr (),
                            fragment_->length ());
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::create(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
  }
  if (!buffer_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_ILexScanner_T::create(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
//  else
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: parsing fragment #%d --> %d byte(s)\n"),
//                inherited::mod_->name (),
//                counter++,
//                fragment_->length ()));

  return (buffer_ != NULL);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::end"));

  // sanity check(s)
  if (!buffer_)
    return;

  // clean buffer
  try {
    this->destroy (state_,
                   buffer_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::destroy(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
  }
  buffer_ = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ScannerStateType,
          typename ParserType,
          typename ParserConfigurationType,
          typename ParserInterfaceType,
          typename ParserArgumentType,
          typename UserDataType>
int
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ScannerStateType,
                       ParserType,
                       ParserConfigurationType,
                       ParserInterfaceType,
                       ParserArgumentType,
                       UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::svc"));

  // sanity check(s)
  ACE_ASSERT (state_);

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  int error = 0;
  typename DataMessageType::DATA_T* message_data_container_p = NULL;
  typename DataMessageType::DATA_T::DATA_T* message_data_p = NULL;
  bool do_scan_end = false;
  const typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
  bool result_2 = false;

  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              NULL);
    if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage

        message_block_p->release ();

        result = 0;

        goto done;
      }
      default:
      { ACE_ASSERT (!fragment_);
        fragment_ = dynamic_cast<DataMessageType*> (message_block_p);
        ACE_ASSERT (fragment_);

        if (fragment_->isInitialized ())
          goto continue_;

        // sanity check(s)
        ACE_ASSERT (inherited::sessionData_);

        session_data_p = &inherited::sessionData_->getR ();

        ACE_NEW_NORETURN (message_data_p,
                          typename DataMessageType::DATA_T::DATA_T ());
        if (!message_data_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_NEW_NORETURN (message_data_container_p,
                          typename DataMessageType::DATA_T (message_data_p));
        if (!message_data_container_p)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        message_data_p = NULL;
        fragment_->initialize (message_data_container_p,
                               session_data_p->sessionId,
                               NULL);
        message_data_container_p = NULL;

continue_:
        if (!begin (fragment_->rd_ptr (),
                    fragment_->length ()))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ILexScanner_T::begin(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        do_scan_end = true;

        // initialize scanner ?
        if (isFirst_)
        {
          isFirst_ = false;

//          /* column is only valid if an input buffer exists. */
//          ARDrone_MAVLink_Scanner_set_column (1, inherited::state_);
//          ARDrone_MAVLink_Scanner_set_lineno (1, inherited::state_);
        } // end IF

        // parse data fragment
        try {
          result_2 = this->lex ();
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Common_ILexScanner_T::lex(), continuing\n"),
                      inherited::mod_->name ()));
          result_2 = false;
        }
        if (!result_2)
        {
          // *NOTE*: most probable reason: connection
          //         has been closed --> session end
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: failed to Common_ILexScanner_T::lex(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF

        // clean up
        end ();
        do_scan_end = false;

        // more data ?
        if (fragment_)
          goto continue_;

        break;

error:
        if (message_data_p)
          delete message_data_p;
        if (message_data_container_p)
          message_data_container_p->decrease ();
        if (message_block_p)
          message_block_p->release ();
        if (do_scan_end)
          end ();

        goto done;
      }
    } // end SWITCH
  } while (true);
  result = -1;

done:
  return result;
}
