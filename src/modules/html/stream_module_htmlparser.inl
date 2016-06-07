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

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::Stream_Module_HTMLParser_T ()
 : inherited ()
 , configuration_ (NULL)
 , parserContext_ ()
 , SAXHandler_ ()
 , initialized_ (false)
 , mode_ (STREAM_MODULE_HTMLPARSER_DEFAULT_MODE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::Stream_Module_HTMLParser_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::~Stream_Module_HTMLParser_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::~Stream_Module_HTMLParser_T"));

  // clean up
  if (initialized_)
  {
    if (parserContext_.parserContext)
    {
      if (parserContext_.parserContext->myDoc)
        xmlFreeDoc (parserContext_.parserContext->myDoc);

      htmlFreeParserCtxt (parserContext_.parserContext);
    } // end IF

    xmlCleanupParser ();
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
void
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::handleDataMessage (MessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  MessageType* message_p = message_inout;
  xmlParserErrors error;
  bool complete = false;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (parserContext_.parserContext);

  do
  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("parsing HTML (message ID: %d, %d byte(s))...\n"),
//                message_p->getID (),
//                message_p->length ()));

    result = htmlParseChunk (parserContext_.parserContext, // context
                             message_p->rd_ptr (),         // chunk
                             message_p->length (),         // size
                             0);                           // terminate ?
    if (result)
    {
      error = static_cast<xmlParserErrors> (result);
//      ACE_DEBUG ((LM_WARNING,
//                  ACE_TEXT ("failed to htmlParseChunk() ((last) error was: %d), continuing\n"),
//                  error));
      xmlCtxtResetLastError (parserContext_.parserContext);
    } // end IF
    message_block_p = message_p->cont ();
    if (!message_block_p)
      break; // done --> more fragments to arrive

    message_p = dynamic_cast<MessageType*> (message_block_p);
    if (!message_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to dynamic_cast<MessageType*>(%@), returning\n"),
                  message_block_p));
      return;
    } // end IF

    // *TODO*: this depends on current (upstream) HTTP parser behavior
    //         --> remove ASAP
    if ((message_p->length () == 0) &&
        (message_p->cont () == NULL))
    {
      complete = true;
      break; // done
    } // end IF
  } while (true);

  if (complete)
  {
    result = htmlParseChunk (parserContext_.parserContext,
                             ACE_TEXT_ALWAYS_CHAR (""),
                             0,
                             1);
    if (result)
    {
      error = static_cast<xmlParserErrors> (result);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to htmlParseChunk() (status was: %d), continuing\n"),
                  error));
    } // end IF

    if (!parserContext_.parserContext->wellFormed)
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("HTML document not well-formed, continuing\n")));
//    if (parserContext_.parserContext->errNo)
//      ACE_DEBUG ((LM_WARNING,
//                  ACE_TEXT ("HTML document had errors (last error was: \"%s\"), continuing\n"),
//                  ACE_TEXT (ACE_OS::strerror (parserContext_.parserContext->errNo))));

//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("parsing HTML...DONE\n")));

    const typename MessageType::DATA_T& data_container_r =
        message_inout->get ();
    typename MessageType::DATA_T::DATA_T& data_r =
        const_cast<typename MessageType::DATA_T::DATA_T&> (data_container_r.get ());
    ACE_ASSERT (!data_r.HTMLDocument);
    data_r.HTMLDocument = parserContext_.parserContext->myDoc;
//    data_r.HTMLDocument = xmlCopyDoc (parserContext_.parserContext->myDoc);
//    if (!data_r.HTMLDocument)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to xmlCopyDoc(): \"%m\", continuing\n")));
    parserContext_.parserContext->myDoc = NULL;
    xmlClearParserCtxt (parserContext_.parserContext);
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
void
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *TODO*: remove type inferences
  const typename SessionMessageType::DATA_T& session_data_container_r =
    message_inout->get ();
  const SessionDataType& session_data_r =
    session_data_container_r.get ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
//      if (parserContext_)
//        htmlCtxtReset (parserContext_);

      // *TODO*: the upstream session data may not be the same as the downstream
      //         one...
      const_cast<SessionDataType&> (session_data_r).parserContext =
          &parserContext_;
      parserContext_.sessionData =
          &const_cast<SessionDataType&> (session_data_r);

      break;
    }
    case STREAM_SESSION_STEP:
    {
      if (parserContext_.parserContext)
        htmlCtxtReset (parserContext_.parserContext);

      break;
    }
    case STREAM_SESSION_END:
    {
      // *TODO*: the upstream session data may not be the same as the downstream
      //         one...
      const_cast<SessionDataType&> (session_data_r).parserContext =
          &parserContext_;
      parserContext_.sessionData =
          &const_cast<SessionDataType&> (session_data_r);

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
const ModuleHandlerConfigurationType&
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
bool
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::initialize (const ModuleHandlerConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::initialize"));

  static bool is_first_run = true;

  if (is_first_run)
  {
    LIBXML_TEST_VERSION
    is_first_run = false;
  } // end IF

  if (initialized_)
  {
    if (parserContext_.parserContext)
    {
      if (parserContext_.parserContext->myDoc)
      {
        xmlFreeDoc (parserContext_.parserContext->myDoc);
        parserContext_.parserContext->myDoc = NULL;
      } // end IF

      htmlFreeParserCtxt (parserContext_.parserContext);
      parserContext_.parserContext = NULL;
    } // end IF

    xmlCleanupParser ();

    initialized_ = false;
  } // end IF

  configuration_ =
    &const_cast<ModuleHandlerConfigurationType&> (configuration_in);

  xmlInitParser ();
//  xmlKeepBlanksDefault (1);
//  xmlLineNumbersDefault (1);
//  xmlSubstituteEntitiesDefault (1);
  if (configuration_in.traceParsing)
    xmlPedanticParserDefault (1);

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  // *TODO*: remove type inferences
  mode_ = configuration_->mode;
  if (mode_ == STREAM_MODULE_HTMLPARSER_SAX)
  {
    htmlDefaultSAXHandlerInit ();
    xmlSAX2InitHtmlDefaultSAXHandler (&SAXHandler_);
    if (!initializeSAXParser ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initializeSAXParser(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end IF

  parserContext_.parserContext =
      htmlCreatePushParserCtxt (((mode_ == STREAM_MODULE_HTMLPARSER_SAX) ? &SAXHandler_
                                                                         : NULL), // SAX handler
                                ((mode_ == STREAM_MODULE_HTMLPARSER_SAX) ? &parserContext_
                                                                         : NULL), // user data (SAX)
                                NULL,                                             // chunk
                                0,                                                // size
                                NULL,                                             // filename
                                XML_CHAR_ENCODING_NONE);                          // encoding
  if (!parserContext_.parserContext)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to htmlCreatePushParserCtxt(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  int parser_options =
      (HTML_PARSE_RECOVER   |     // Relaxed parsing
       HTML_PARSE_NODEFDTD  |     // do not default a doctype if not found
//           HTML_PARSE_NOERROR   |   // suppress error reports
//           HTML_PARSE_NOWARNING |   // suppress warning reports
       HTML_PARSE_PEDANTIC  |     // pedantic error reporting
//           HTML_PARSE_NOBLANKS  |   // remove blank nodes
       HTML_PARSE_NONET     |     // Forbid network access
       HTML_PARSE_NOIMPLIED); //| // Do not add implied html/body... elements
//           HTML_PARSE_COMPACT   |   // compact small text nodes
//           HTML_PARSE_IGNORE_ENC);  // ignore internal document encoding hint
  int result = htmlCtxtUseOptions (parserContext_.parserContext,
                                   parser_options);
//  if (result)
  if (result != parser_options)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to htmlCtxtUseOptions(%d) (result was: %d), continuing\n"),
                inherited::mod_->name (),
                parser_options, result));

  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::SAXDefaultErrorCallback);
  xmlSetGenericErrorFunc (parserContext_.parserContext,
                          &::SAXDefaultErrorCallback);
  xmlSetStructuredErrorFunc (parserContext_.parserContext,
                             &::SAXDefaultStructuredErrorCallback);

  initialized_ = true;

  return true;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ModuleHandlerConfigurationType,
          typename SessionDataType,
          typename ParserContextType>
bool
Stream_Module_HTMLParser_T<SessionMessageType,
                           MessageType,
                           ModuleHandlerConfigurationType,
                           SessionDataType,
                           ParserContextType>::initializeSAXParser ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::initializeSAXParser"));

  // *TODO*
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);

  ACE_NOTREACHED (return false;)
}
