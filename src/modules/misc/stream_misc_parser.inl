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

#include "common_parser_defines.h"

#include "stream_defines.h"
#include "stream_macros.h"

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
                          UserDataType>::Stream_Module_CppParser_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , configuration_ (NULL)
 , fragment_ (NULL)
 , offset_ (0)
 , parser_ (dynamic_cast<ParserInterfaceType*> (this), // parser
            &scanner_)                                 // scanner
// , argument_ ()
 , scanner_ ()
 , scannerState_ ()
 , isFirst_ (true)
 , buffer_ (NULL)
 , streamBuffer_ ()
 , stream_ (&streamBuffer_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_CppParser_T::Stream_Module_CppParser_T"));

  scanner_.setP (this);

  scanner_.set_debug (COMMON_PARSER_DEFAULT_LEX_TRACE ? 1 : 0);
#if defined (YYDEBUG)
  parser_.set_debug_level (COMMON_PARSER_DEFAULT_YACC_TRACE ? 1 : 0);
//  yydebug = (COMMON_PARSER_DEFAULT_YACC_TRACE ? 1 : 0);
//  yysetdebug (COMMON_PARSER_DEFAULT_YACC_TRACE ? 1 : 0);
#endif // YYDEBUG
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

  scanner_.set_debug (configuration_->debugScanner ? 1 : 0);
#if defined (YYDEBUG)
  parser_.set_debug_level (configuration_->debugParser ? 1 : 0);
//  yydebug = (configuration_->debugParser ? 1 : 0);
//  yysetdebug (configuration_->debugParser ? 1 : 0);
#endif // YYDEBUG

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

  ACE_ASSERT (false); // *TODO*
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
  ACE_ASSERT (configuration_);
  ACE_ASSERT (fragment_);

  ACE_Message_Block* message_block_p = fragment_;
  if (!fragment_->cont ())
  {
    // sanity check(s)
    if (!configuration_->block)
      return false; // not enough data, cannot proceed

    waitBuffer (); // <-- wait for data
    if (unlikely (!fragment_->cont ()))
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
  ACE_ASSERT (fragment_->capacity () - fragment_->length () >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
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
  bool done = false;
  SessionMessageType* session_message_p = NULL;
  bool is_data = false;

  // *IMPORTANT NOTE*: 'this' is the parser thread currently blocked in yylex()

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->block);
  if (!inherited::msg_queue_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: message queue not set - cannot wait, returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  // 1. wait for data
  do
  {
    result = inherited::msg_queue_->dequeue_head (message_block_p, NULL);
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
      case STREAM_MESSAGE_DATA:
      case STREAM_MESSAGE_OBJECT:
        is_data = true;
        break;
      case STREAM_MESSAGE_SESSION:
      {
        session_message_p = static_cast<SessionMessageType*> (message_block_p);
        if (session_message_p->type () == STREAM_SESSION_MESSAGE_END)
          done = true; // session has finished --> abort
        break;
      }
      default:
        break;
    } // end SWITCH
    if (is_data)
      break;

    // requeue message
    result = inherited::msg_queue_->enqueue_tail (message_block_p, NULL);
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

  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);

  // sanity check(s)
  ACE_ASSERT (fragment_);

  // create/initialize a new buffer state
  streamBuffer_.set (fragment_->rd_ptr (),
                     fragment_->length () + COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
//  stream_.rdbuf (&streamBuffer_);
  scanner_.switch_streams (&stream_, NULL);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("parsing fragment (%u byte(s))\n"),
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
          typename ParserDriverType,
          typename UserDataType>
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
                       UserDataType>::Stream_Module_Parser_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , headFragment_ (NULL)
 , parserQueue_ (STREAM_QUEUE_MAX_SLOTS, NULL)
 , resetQueue_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::Stream_Module_Parser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType,
          typename UserDataType>
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
                       UserDataType>::~Stream_Module_Parser_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::~Stream_Module_Parser_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType,
          typename UserDataType>
bool
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
                       UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                  Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (headFragment_)
    {
      headFragment_->release (); headFragment_ = NULL;
    } // end IF
    if (parserQueue_.deactivated ())
    {
      result = parserQueue_.activate ();
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue_T::activate(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return false;
      } // end IF
    } // end IF
    parserQueue_.flush ();
    resetQueue_ = false;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);
  if (!configuration_in.parserConfiguration->messageQueue)
  {
    resetQueue_ = true;
    const_cast<ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
      &parserQueue_;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: updated parser configuration, continuing\n"),
                inherited::mod_->name ()));
  } // end IF
  else
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: parser uses external queue, continuing\n"),
                inherited::mod_->name ()));
  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize parser: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
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
          typename ParserDriverType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
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

  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;

  // *IMPORTANT NOTE*: send 'step data' session message so downstream modules know
  //                   that some data has arrived
  if (likely (session_data_container_p))
  {
    session_data_container_p->increase ();

    typename SessionMessageType::DATA_T::DATA_T& session_data_r =
      const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());
    session_data_r.bytes += message_inout->total_length ();
  } // end IF
  if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP_DATA,
                                               session_data_container_p,
                                               NULL,
                                               false))) // expedited ?
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                inherited::mod_->name (),
                STREAM_SESSION_MESSAGE_STEP_DATA));

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT (message_inout->capacity () - message_inout->length () >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  result = parserQueue_.enqueue_tail (message_inout,
                                      NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (unlikely (error != ESHUTDOWN))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
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
          typename ParserDriverType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
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
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      goto end;

      break;
    }
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
end:
      stop ();

      if (likely (inherited::thr_count_))
        inherited::wait (false); // wait for message queue ?

      if (headFragment_)
      {
        headFragment_->release (); headFragment_ = NULL;
      } // end IF

      if (resetQueue_)
      {
        resetQueue_ = false;
        inherited::configuration_->parserConfiguration->messageQueue = NULL;
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
          typename ParserDriverType,
          typename UserDataType>
int
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
                       UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Parser_T::svc"));

  // sanity check(s)
  //ACE_ASSERT (state_);

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  int error = 0;
  DataMessageType* message_p = NULL;
  //typename DataMessageType::DATA_T* message_data_container_p = NULL;
  //typename DataMessageType::DATA_T::DATA_T* message_data_p = NULL;
//  bool do_scan_end = false;
//  const typename SessionMessageType::DATA_T::DATA_T* session_data_p = NULL;
  bool result_2 = false;

  do
  {
    message_block_p = NULL;
    result = parserQueue_.dequeue_head (message_block_p,
                                        NULL);
    if (result == -1)
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Message_Queue::dequeue_head(): \"%m\", aborting\n"),
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
      {
        // initialize message ?
        message_p = static_cast<DataMessageType*> (message_block_p);

        //{ ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (headFragment_)
          { // enqueue at the end
            ACE_Message_Block* message_block_2 = headFragment_;
            while (message_block_2->cont ())
              message_block_2 = message_block_2->cont ();
            message_block_2->cont (message_p);
          } // end IF
          else
            headFragment_ = message_p;

          // free 0-length fragments from the head
          ACE_Message_Block* message_block_2 = headFragment_;
          while (message_block_2 &&
                 !message_block_2->length ())
            message_block_2 = message_block_2->cont ();
          if (!message_block_2)
          { // there is no data at all
            headFragment_->release (); headFragment_ = NULL;
            continue; // wait for more data
          } // end IF
          ACE_ASSERT (message_block_2 && message_block_2->length ());
          if (message_block_2 != headFragment_)
          {
            ACE_Message_Block* message_block_3 = headFragment_;
            while (message_block_3->cont () != message_block_2)
              message_block_3 = message_block_3->cont ();
            ACE_ASSERT (message_block_3 && (message_block_3->cont () == message_block_2));
            message_block_3->cont (NULL);
            headFragment_->release (); headFragment_ = NULL;
            headFragment_ = static_cast<DataMessageType*> (message_block_2);
          } // end IF
          ACE_ASSERT (headFragment_ && headFragment_->length ());
        //} // end lock scope
continue_:
        // parse next data fragment(s)
        try {
          result_2 = inherited2::parse (headFragment_);
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Common_IParser_T::parse(), continuing\n"),
                      inherited::mod_->name ()));
          result_2 = false;
        }
        if (unlikely (!result_2))
        {
          // *NOTE*: most probable reason: connection
          //         has been closed --> session end
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_IParser_T::parse(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        // *NOTE*: handle cases where all available data has been parsed
        if (inherited2::finished_)
        {
          inherited2::finished_ = false;
          break;
        } // end IF
        if (headFragment_ &&
            headFragment_->length ())
          goto continue_;

        break;

error:
        if (headFragment_)
        {
          headFragment_->release (); headFragment_ = NULL;
        } // end IF
        result = -1;

        goto error_2;
      }
    } // end SWITCH
  } while (true);
  result = -1;

done:
  return result;

error_2:
  inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ParserDriverType,
          typename UserDataType>
void
Stream_Module_Parser_T<ACE_SYNCH_USE,
                       TimePolicyType,
                       ConfigurationType,
                       ControlMessageType,
                       DataMessageType,
                       SessionMessageType,
                       ParserDriverType,
                       UserDataType>::stop ()
{
  COMMON_TRACE (ACE_TEXT ("Stream_Module_Parser_T::stop"));

  // sanity check(s)
  ACE_ASSERT (!parserQueue_.deactivated ());

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

  int result = parserQueue_.enqueue_tail (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::Stream_Module_ParserH_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in,                               // stream handle
              false,                                   // auto-start ? (active mode only)
              STREAM_HEADMODULECONCURRENCY_CONCURRENT, // concurrency mode
              true)
 , inherited2 ()
 , headFragment_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::Stream_Module_ParserH_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::~Stream_Module_ParserH_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::~Stream_Module_ParserH_T"));

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
bool
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::initialize"));

  if (inherited::isInitialized_)
  {
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);
  ACE_ASSERT (!configuration_in.parserConfiguration->messageQueue);

  const_cast<ConfigurationType&> (configuration_in).parserConfiguration->messageQueue =
    &(inherited::queue_);

  if (!inherited2::initialize (*configuration_in.parserConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize parser: \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
void
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::handleDataMessage"));

  DataMessageType* message_p = NULL;
  int result = -1;
  bool release_inbound_message = true; // message_inout

  // initialize return value(s)
  passMessageDownstream_out = false;

  // append the "\0\0"-sequence, as required by flex
  ACE_ASSERT ((message_inout->capacity () - message_inout->length ()) >= COMMON_PARSER_FLEX_BUFFER_BOUNDARY_SIZE);
  *(message_inout->wr_ptr ()) = YY_END_OF_BUFFER_CHAR;
  *(message_inout->wr_ptr () + 1) = YY_END_OF_BUFFER_CHAR;
  // *NOTE*: DO NOT adjust the write pointer --> length() must stay as it was

  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    if (!headFragment_)
      headFragment_ = message_inout;
    else
    {
      for (message_p = headFragment_;
        message_p->cont ();
        message_p = dynamic_cast<DataMessageType*> (message_p->cont ()));
      message_p->cont (message_inout);

      //// just signal the parser (see below for an explanation)
      //result = condition_.broadcast ();
      //if (result == -1)
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%s\", continuing\n"),
      //              inherited::mod_->name ()));
    } // end ELSE

    message_p = headFragment_;
  } // end lock scope
  ACE_ASSERT (message_p);
  message_inout = NULL;
  release_inbound_message = false;

  { // *NOTE*: protect scanner/parser state
    //ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);

    // OK: parse the message (fragment)

    //  ACE_DEBUG ((LM_DEBUG,
    //              ACE_TEXT ("parsing message (ID:%u,%u byte(s))...\n"),
    //              message_p->id (),
    //              message_p->length ()));

    if (!inherited2::parse (message_p))
    { // *NOTE*: most probable reason: connection
      //         has been closed --> session end
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: failed to ParserDriverType::parse() (message id was: %d), returning\n"),
                  inherited::mod_->name (),
                  message_p->id ()));
      goto error;
    } // end IF
    // the message fragment has been parsed successfully

    if (!inherited2::hasFinished ())
      goto continue_; // --> wait for more data to arrive
  } // end lock scope

  // *NOTE*: the message has been parsed successfully
  //         --> pass the data (chain) downstream
  {//ACE_Guard<ACE_SYNCH_MUTEX> aGuard (lock_);
    //// *NOTE*: new data fragments may have arrived by now
    ////         --> set the next head fragment ?
    //message_2 = dynamic_cast<DataMessageType*> (message_p->cont ());
    //if (message_2)
    //  message_p->cont (NULL);

    result = inherited::put_next (headFragment_, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task_T::put_next(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      headFragment_->release (); headFragment_ = NULL;
      goto error;
    } // end IF
    headFragment_ = NULL;
  } // end lock scope

continue_:
error:
  if (release_inbound_message)
  { ACE_ASSERT (message_inout);
    message_inout->release (); message_inout = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
void
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

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
      stop ();
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
void
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::stop ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::stop"));

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

  int result = inherited::queue_.enqueue (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename SessionControlType,
          typename SessionEventType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename UserDataType,
          typename ParserDriverType>
int
Stream_Module_ParserH_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        ConfigurationType,
                        SessionControlType,
                        SessionEventType,
                        StreamStateType,
                        SessionDataType,
                        SessionDataContainerType,
                        StatisticContainerType,
                        TimerManagerType,
                        UserDataType,
                        ParserDriverType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_ParserH_T::put"));

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
    {
      typename SessionMessageType::DATA_T* session_data_container_p =
        inherited::sessionData_;

      // *IMPORTANT NOTE*: send 'step data' session message so downstream modules know
      //                   that some data has arrived
      if (likely (session_data_container_p))
      {
        session_data_container_p->increase ();

        typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (session_data_container_p->getR ());
        session_data_r.bytes += messageBlock_in->total_length ();
      } // end IF
      if (unlikely (!inherited::putSessionMessage (STREAM_SESSION_MESSAGE_STEP_DATA,
                                                   session_data_container_p,
                                                   NULL,
                                                   false))) // expedited ?
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_TaskBase_T::putSessionMessage(%d), continuing\n"),
                    inherited::mod_->name (),
                    STREAM_SESSION_MESSAGE_STEP_DATA));
      break;
    }
    default:
      break;
  } // end SWITCH

  return inherited::put (messageBlock_in,
                         timeValue_in);
}
