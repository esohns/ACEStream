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

#include "common_string_tools.h"

#include "stream_macros.h"

#include "stream_html_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename ParserContextType>
Stream_Module_HTMLParser_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                           ParserContextType>::Stream_Module_HTMLParser_T (ISTREAM_T* stream_in)
#else
                           ParserContextType>::Stream_Module_HTMLParser_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , headFragment_ (NULL)
 , parserContext_ ()
 , SAXHandler_ ()
 , mode_ (STREAM_MODULE_HTMLPARSER_DEFAULT_MODE)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::Stream_Module_HTMLParser_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename ParserContextType>
Stream_Module_HTMLParser_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType,
                           ParserContextType>::~Stream_Module_HTMLParser_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::~Stream_Module_HTMLParser_T"));

  if (inherited::isInitialized_)
  {
    if (parserContext_.parserContext)
    {
      if (parserContext_.parserContext->myDoc)
        xmlFreeDoc (parserContext_.parserContext->myDoc);

      htmlFreeParserCtxt (parserContext_.parserContext);
    } // end IF

    xmlCleanupParser ();
  } // end IF

  if (headFragment_)
    headFragment_->release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename ParserContextType>
void
Stream_Module_HTMLParser_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType,
                           ParserContextType>::handleDataMessage (DataMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::handleDataMessage"));

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  xmlParserErrors parse_errors = XML_ERR_OK;
  xmlErrorPtr error_p = NULL;

  // send the whole document downstream
  passMessageDownstream_out = false;

  if (!headFragment_)
    headFragment_ = message_inout;
  else
  { // append fragment
    message_block_p = headFragment_;
    while (message_block_p->cont ())
      message_block_p = message_block_p->cont ();
    message_block_p->cont (message_inout);
  } // end ELSE

  // sanity check(s)
  ACE_ASSERT (parserContext_.parserContext);

  do
  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("parsing HTML (message ID: %d, %d byte(s))...\n"),
//                message_p->id (),
//                message_p->length ()));

    result = htmlParseChunk (parserContext_.parserContext, // context
                             message_inout->rd_ptr (),     // chunk
                             message_inout->length (),     // size
                             0);                           // terminate ?
    if (result)
    {
      parse_errors = static_cast<xmlParserErrors> (result);
      error_p = xmlGetLastError ();
      ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p->level),
                  ACE_TEXT ("failed to htmlParseChunk() (result was: %d): \"%s\", continuing\n"),
                  parse_errors,
                  ACE_TEXT (error_p->message)));
      xmlCtxtResetLastError (parserContext_.parserContext);
    } // end IF
    message_block_p = message_block_p->cont ();
    if (!message_block_p)
      break; // done --> more fragments to arrive ?
  } while (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename ParserContextType>
void
Stream_Module_HTMLParser_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType,
                           ParserContextType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!parserContext_.sessionData);

      // *TODO*: remove type inference
      parserContext_.sessionData =
        &const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (parserContext_.sessionData);

      // *TODO*: remove type inference
      parserContext_.sessionData =
        &const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      // sanity check(s)
      ACE_ASSERT (headFragment_);
      
      const typename DataMessageType::DATA_T& data_container_r =
        headFragment_->getR ();
      typename DataMessageType::DATA_T::DATA_T& data_r =
        const_cast<typename DataMessageType::DATA_T::DATA_T&> (data_container_r.getR ());
      int result = -1;
      xmlParserErrors parse_errors = XML_ERR_OK;
      xmlErrorPtr error_p = NULL;

      // *IMPORTANT NOTE*: no more data will arrive for this document
      result = htmlParseChunk (parserContext_.parserContext,
                               ACE_TEXT_ALWAYS_CHAR (""),
                               0,
                               1); // terminate
      if (result)
      {
        parse_errors = static_cast<xmlParserErrors> (result);
        error_p = xmlGetLastError ();
        ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p->level),
                    ACE_TEXT ("failed to htmlParseChunk() (result was: %d): \"%s\", continuing\n"),
                    parse_errors,
                    ACE_TEXT (error_p->message)));
        xmlCtxtResetLastError (parserContext_.parserContext);
      } // end IF
      if (!parserContext_.parserContext->wellFormed)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: HTML document not well-formed, continuing\n"),
                    inherited::mod_->name ()));
      error_p = xmlGetLastError ();
      if (error_p->code)
        ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p->level),
                    ACE_TEXT ("%s: HTML document had errors (last error was: \"%s\"), continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (error_p->message)));

  //    ACE_DEBUG ((LM_DEBUG,
  //                ACE_TEXT ("parsing HTML...DONE\n")));

      // *TODO*: remove type inference
      ACE_ASSERT (!data_r.HTMLDocument);
      data_r.HTMLDocument = parserContext_.parserContext->myDoc;
      parserContext_.parserContext->myDoc = NULL;

      // reset parser
      // *TODO*: apparently, there currently is now way to 'correctly' reset the
      //         parser that would allow reuse (htmlNewInputStream() internal...)
      //         after xmlClearParserCtxt()/htmlCtxtReset() --> recreate
      //xmlClearParserCtxt (parserContext_.parserContext);
      //xmlParserInputBufferPtr buffer_p =
      //  xmlAllocParserInputBuffer (XML_CHAR_ENCODING_NONE);
      //if (!buffer_p)
      //{
      //  ACE_DEBUG ((LM_CRITICAL,
      //              ACE_TEXT ("failed to xmlAllocParserInputBuffer(), returning\n")));
      //  goto error;
      //} // end IF
      //htmlParserInputPtr stream_p =
      //  htmlNewInputStream (parserContext_.parserContext);
      //if (!stream_p)
      //{
      //  ACE_DEBUG ((LM_CRITICAL,
      //              ACE_TEXT ("failed to htmlNewInputStream(), returning\n")));
      //  goto error;
      //} // end IF
      //stream_p->buf = buffer_p;
      //xmlBufResetInput (buffer_p->buffer, stream_p);
      //inputPush (parserContext_.parserContext, stream_p);
      htmlFreeParserCtxt (parserContext_.parserContext);
      parserContext_.parserContext = NULL;
      if (!resetParser ())
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_HTMLParser_T::resetParser(), continuing\n"),
                    inherited::mod_->name ()));
      //if (parserContext_.parserContext)
      //  htmlCtxtReset (parserContext_.parserContext);

      result = inherited::put_next (headFragment_, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
        headFragment_->release ();
      } // end IF
      headFragment_ = NULL;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      parserContext_.sessionData = NULL;

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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename ParserContextType>
bool
Stream_Module_HTMLParser_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType,
                           ParserContextType>::initialize (const ConfigurationType& configuration_in,
                                                           Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::initialize"));

  static bool is_first_run = true;
  if (is_first_run)
  {
    LIBXML_TEST_VERSION
    is_first_run = false;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (configuration_in.parserConfiguration);

  if (inherited::isInitialized_)
  {
    if (parserContext_.parserContext)
    {
      if (parserContext_.parserContext->myDoc)
      {
        xmlFreeDoc (parserContext_.parserContext->myDoc); parserContext_.parserContext->myDoc = NULL;
      } // end IF

      htmlFreeParserCtxt (parserContext_.parserContext); parserContext_.parserContext = NULL;
    } // end IF

    xmlCleanupParser ();
  } // end IF

  xmlInitParser ();
//  xmlKeepBlanksDefault (1);
//  xmlLineNumbersDefault (1);
//  xmlSubstituteEntitiesDefault (1);
  if (configuration_in.parserConfiguration->debugParser)
    xmlPedanticParserDefault (1);

  // sanity check(s)
  ACE_ASSERT (inherited::mod_);

  // *TODO*: remove type inferences
  mode_ = configuration_in.mode;
  if (mode_ == STREAM_MODULE_HTMLPARSER_MODE_SAX)
  {
    //htmlDefaultSAXHandlerInit ();
    xmlSAX2InitHtmlDefaultSAXHandler (&SAXHandler_);
    if (!initializeSAXParser ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to initialize SAX parser: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF
  } // end IF

  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::stream_html_parser_sax_default_error_cb);

  if (!resetParser ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_HTMLParser_T::resetParser(), aborting\n"),
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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename ParserContextType>
bool
Stream_Module_HTMLParser_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionDataContainerType,
                           SessionDataType,
                           ParserContextType>::resetParser ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_HTMLParser_T::resetParser"));

  // sanity check(s)
  ACE_ASSERT (!parserContext_.parserContext);

  parserContext_.parserContext =
    htmlCreatePushParserCtxt (((mode_ == STREAM_MODULE_HTMLPARSER_MODE_SAX) ? &SAXHandler_
                                                                            : NULL), // SAX handler
                              ((mode_ == STREAM_MODULE_HTMLPARSER_MODE_SAX) ? &parserContext_
                                                                            : NULL), // user data (SAX)
                              NULL,                                                  // chunk
                              0,                                                     // size
                              NULL,                                                  // filename
                              XML_CHAR_ENCODING_NONE);                               // encoding
  if (!parserContext_.parserContext)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to htmlCreatePushParserCtxt(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  int parser_options =
    (HTML_PARSE_RECOVER   | // relaxed parsing
     HTML_PARSE_NODEFDTD  | // do not default a doctype if not found
//           HTML_PARSE_NOERROR   |   // suppress error reports
//           HTML_PARSE_NOWARNING |   // suppress warning reports
     HTML_PARSE_PEDANTIC  | // pedantic error reporting
     HTML_PARSE_NOBLANKS  | // remove blank nodes
//       HTML_PARSE_NONET      |     // forbid network access
     HTML_PARSE_NOIMPLIED | // do not add implied html/body... elements
     HTML_PARSE_COMPACT   | // compact small text nodes
//           HTML_PARSE_IGNORE_ENC);  // ignore internal document encoding hint
     XML_PARSE_HUGE);       // parse large documents
  int result = htmlCtxtUseOptions (parserContext_.parserContext,
                                   parser_options);
  if (result)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: failed to htmlCtxtUseOptions(%d) (result was: %d), continuing\n"),
                inherited::mod_->name (),
                parser_options, result));

  xmlSetGenericErrorFunc (parserContext_.parserContext,
                          &::stream_html_parser_sax_default_error_cb);
  xmlSetStructuredErrorFunc (parserContext_.parserContext,
                             &::stream_html_parser_sax_default_structured_error_cb);

  return true;
}
