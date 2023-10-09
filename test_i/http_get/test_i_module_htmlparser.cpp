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
#include "stdafx.h"

#include "test_i_module_htmlparser.h"

#include <regex>
#include <sstream>
#include <string>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

void
test_i_libxml2_sax_error_cb (void* userData_in,
                             const char* message_in,
                             ...)
{
  //STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_error_cb"));

  int result = -1;

  // sanity check(s)
  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);
  ACE_ASSERT (data_p);

  ACE_TCHAR buffer[BUFSIZ];
  va_list arguments;

  va_start (arguments, message_in);
  result = ACE_OS::vsnprintf (buffer,
                              sizeof (buffer),
//                            sizeof (buffer) / sizeof (buffer[0]),
                              message_in, arguments);
  ACE_UNUSED_ARG (result);
  va_end (arguments);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("errorCallback[%s:%d:%d] (%d,%d): %s"),
              data_p->parserContext->lastError.file, data_p->parserContext->lastError.line, data_p->parserContext->lastError.int2,
              data_p->parserContext->lastError.domain, data_p->parserContext->lastError.code,
              buffer));
}

void
test_i_libxml2_sax_structured_error_cb (void* userData_in,
                                        xmlErrorPtr error_in)
{
  //STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_structured_error_cb"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("structured error: %s\n"),
              ACE_TEXT (error_in->message)));
}

//////////////////////////////////////////

Test_I_Stream_HTMLParser::Test_I_Stream_HTMLParser (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::Test_I_Stream_HTMLParser"));

}

void
Test_I_Stream_HTMLParser::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!inherited::parserContext_.sessionData);

      // *TODO*: remove type inferences
      struct Test_I_Stream_SessionData& session_data_r =
        const_cast<struct Test_I_Stream_SessionData&> (inherited::sessionData_->getR ());

//      if (parserContext_)
//        htmlCtxtReset (parserContext_);

      session_data_r.parserContext = &(inherited::parserContext_);
      inherited::parserContext_.sessionData = &session_data_r;

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      // *IMPORTANT NOTE*: no more data will arrive for this document
      if (inherited::parserContext_.parserContext)
      {
        int result = htmlParseChunk (inherited::parserContext_.parserContext,
                                     ACE_TEXT_ALWAYS_CHAR (""),
                                     0,
                                     1); // terminate
        xmlErrorPtr error_p = xmlGetLastError ();
        if (result)
        {
          xmlParserErrors parse_errors = static_cast<xmlParserErrors> (result);
          ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p ? error_p->level : XML_ERR_ERROR),
                      ACE_TEXT ("%s: failed to htmlParseChunk() (result was: %d): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      result,
                      error_p ? ACE_TEXT (error_p->message) : ACE_TEXT ("")));
        } // end IF
        if (!inherited::parserContext_.parserContext->wellFormed)
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s: document not well-formed, continuing\n"),
                      inherited::mod_->name ()));
        if (error_p &&
            error_p->code)
          ACE_DEBUG ((Stream_HTML_Tools::errorLevelToLogPriority (error_p->level),
                      ACE_TEXT ("%s: document had errors (last error was: %d: \"%s\"), continuing\n"),
                      inherited::mod_->name (),
                      error_p->code, ACE_TEXT (error_p->message)));
        xmlCtxtResetLastError (inherited::parserContext_.parserContext);

        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("parsing HTML...DONE\n")));

        htmlCtxtReset (inherited::parserContext_.parserContext);
      } // end IF

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      inherited::parserContext_.sessionData = NULL;

      break;
    }
    default:
      break;
  } // end SWITCH
}

bool
Test_I_Stream_HTMLParser::initialize (const struct Test_I_HTTPGet_ModuleHandlerConfiguration& configuration_in,
                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.mode == STREAM_MODULE_HTMLPARSER_MODE_SAX);

//  initGenericErrorDefaultFunc ((xmlGenericErrorFunc*)&::errorCallback);
//  xmlSetGenericErrorFunc (inherited::parserContext_, &::errorCallback);
//  xmlSetStructuredErrorFunc (inherited::parserContext_, &::structuredErrorCallback);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

bool
Test_I_Stream_HTMLParser::initializeSAXParser ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_HTMLParser::initializeSAXParser"));

  // set necessary SAX parser callbacks
  // *IMPORTANT NOTE*: the default SAX callbacks expect xmlParserCtxtPtr as user
  //                   data. This implementation uses Test_I_SAXParserContext*.
  //                   --> the default callbacks will crash on invocation, so
  //                       disable them here...
  inherited::SAXHandler_.cdataBlock = NULL;
  inherited::SAXHandler_.comment = NULL;
  inherited::SAXHandler_.internalSubset = NULL;
  inherited::SAXHandler_.startDocument = NULL;
  inherited::SAXHandler_.endDocument = NULL;

//  inherited::SAXHandler_.getEntity = getEntity;
//  inherited::SAXHandler_.startDocument = test_i_libxml2_sax_start_document_cb;
//  inherited::SAXHandler_.endDocument = test_i_libxml2_sax_start_document_cb;
  inherited::SAXHandler_.startElement = test_i_libxml2_sax_start_element_cb;
  inherited::SAXHandler_.endElement = test_i_libxml2_sax_end_element_cb;
  inherited::SAXHandler_.characters = test_i_libxml2_sax_characters_cb;
  ///////////////////////////////////////
  inherited::SAXHandler_.warning = test_i_libxml2_sax_error_cb;
  inherited::SAXHandler_.error = test_i_libxml2_sax_error_cb;
  inherited::SAXHandler_.fatalError = test_i_libxml2_sax_error_cb;

  ACE_ASSERT (inherited::SAXHandler_.initialized);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void
test_i_libxml2_sax_start_document_cb (void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_start_document_cb"));

  ACE_UNUSED_ARG (userData_in);
}

void
test_i_libxml2_sax_end_document_cb (void* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_end_document_cb"));

  ACE_UNUSED_ARG (userData_in);
}

void
test_i_libxml2_sax_characters_cb (void* userData_in,
                                  const xmlChar* string_in,
                                  int length_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_characters_cb"));

  ACE_UNUSED_ARG (length_in);

  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  switch (data_p->state)
  {
    case SAXPARSER_STATE_READ_DATE_2:
    {
      // parse date string
      std::string date_string = reinterpret_cast<const char*> (string_in);
      std::string regex_string =
        ACE_TEXT_ALWAYS_CHAR ("^(?:[[:space:]]+)(?:[[:alpha:]]+, )([[:digit:]]{1,2})\\.([[:digit:]]{1,2})\\.([[:digit:]]{2,4})(?:[[:space:]]+)$");
      std::regex regex (regex_string);
      std::smatch match_results;
      if (!std::regex_match (date_string,
                             match_results,
                             regex,
                             std::regex_constants::match_default))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid date string (was: \"%s\"), returning\n"),
                    ACE_TEXT (date_string.c_str ())));
        return;
      } // end IF
//      ACE_ASSERT (match_results.ready () && !match_results.empty ());
      ACE_ASSERT (!match_results.empty ());

      ACE_ASSERT (match_results[1].matched);
      ACE_ASSERT (match_results[2].matched);
      ACE_ASSERT (match_results[3].matched);

      std::stringstream converter;
      long value;
      struct tm tm_time;
      ACE_OS::memset (&tm_time, 0, sizeof (struct tm));
      converter << match_results[1].str ();
      converter >> value;
      tm_time.tm_mday = value;
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter << match_results[2].str ();
      converter >> value;
      tm_time.tm_mon = value - 1; // months are 0-11
      converter.clear ();
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter << match_results[3].str ();
      converter >> value;
      tm_time.tm_year = value - 1900; // years are since 1900

      time_t time_seconds = ACE_OS::mktime (&tm_time);
      data_p->timeStamp.set (time_seconds, 0);

      data_p->state = SAXPARSER_STATE_IN_BODY;

      break;
    }
    case SAXPARSER_STATE_READ_ITEMS_3:
    {
      data_p->dataItem.description = (char*)string_in;
      data_p->dataItem.description = Common_String_Tools::strip (data_p->dataItem.description);

      data_p->state = SAXPARSER_STATE_IN_BODY;

      break;
    }
    default:
      break;
  } // end SWITCH
}

void
test_i_libxml2_sax_start_element_cb (void* userData_in,
                                     const xmlChar* name_in,
                                     const xmlChar** attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_start_element_cb"));

  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  const xmlChar** attributes_p = attributes_in;

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_INVALID);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF

  // ------------------------------- header ------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_IN_HEAD;

    return;
  } // end IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_IN_BODY;

    return;
  } // end IF

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("section"))))
  {
    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("archive__day"))))
        {
          data_p->state = SAXPARSER_STATE_READ_DATE_1;
          break;
        } // end IF
      } // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end IF

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("h2"))))
  {
    if (data_p->state != SAXPARSER_STATE_READ_DATE_1)
      return;

    data_p->state = SAXPARSER_STATE_READ_DATE_2;
    return;
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("ul"))))
  {
    while (NULL != attributes_p && NULL != attributes_p[0])
    {
      if (xmlStrEqual (attributes_p[0],
                       BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
      {
        if (xmlStrEqual (attributes_p[1],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("archive__list"))))
        {
          data_p->state = SAXPARSER_STATE_READ_ITEMS_1;
          break;
        } // end IF
      }   // end IF

      attributes_p = &attributes_p[2];
    } // end WHILE
  } // end ELSE IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("a"))))
  {
    if (data_p->state == SAXPARSER_STATE_READ_ITEMS_1)
    {
      // save link URL
      while (NULL != attributes_p && NULL != attributes_p[0])
      {
        if (xmlStrEqual (attributes_p[0],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("href"))))
        {
          data_p->dataItem.URI = (char*)attributes_p[1];
          break;
        } // end IF

        attributes_p = &attributes_p[2];
      } // end WHILE

      data_p->state = SAXPARSER_STATE_READ_ITEMS_2;
    } // end IF
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("span"))))
  {
    if (data_p->state == SAXPARSER_STATE_READ_ITEMS_2)
    {
      while (NULL != attributes_p && NULL != attributes_p[0])
      {
        if (xmlStrEqual (attributes_p[0],
                         BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("class"))))
        {
          if (xmlStrEqual (attributes_p[1],
                           BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("a-article-teaser__title-text"))))
          {
            data_p->state = SAXPARSER_STATE_READ_ITEMS_3;
            break;
          } // end IF
        } // end IF

        attributes_p = &attributes_p[2];
      } // end WHILE
    } // end IF
  } // end ELSE IF
}

void
test_i_libxml2_sax_end_element_cb (void* userData_in,
                                   const xmlChar* name_in)
{
  STREAM_TRACE (ACE_TEXT ("::test_i_libxml2_sax_end_element_cb"));

  struct Test_I_SAXParserContext* data_p =
      static_cast<struct Test_I_SAXParserContext*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("html"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HTML);

    data_p->state = SAXPARSER_STATE_INVALID;

    return;
  } // end IF

  // ------------------------------- header ------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("head"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_HEAD);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF

  // -------------------------------- body -------------------------------------
  if (xmlStrEqual (name_in,
                   BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("body"))))
  {
    // sanity check(s)
    ACE_ASSERT (data_p->state == SAXPARSER_STATE_IN_BODY);

    data_p->state = SAXPARSER_STATE_IN_HTML;

    return;
  } // end IF
  else if (xmlStrEqual (name_in,
                        BAD_CAST (ACE_TEXT_ALWAYS_CHAR ("span"))))
  {
    if (data_p->state != SAXPARSER_STATE_READ_ITEMS_3)
      return; // nothing to do

    // sanity check(s)
    ACE_ASSERT (data_p->sessionData);

    Test_I_PageDataIterator_t iterator =
        data_p->sessionData->data.pageData.find (data_p->timeStamp);
    if (iterator == data_p->sessionData->data.pageData.end ())
    {
      Test_I_DataItems_t data_items;
      data_items.push_back (data_p->dataItem);
      data_p->sessionData->data.pageData[data_p->timeStamp] = data_items;
    } // end IF
    else
      (*iterator).second.push_back (data_p->dataItem);

    data_p->dataItem.description.clear ();

    data_p->state = SAXPARSER_STATE_READ_ITEMS_1;
  } // end IF
}
