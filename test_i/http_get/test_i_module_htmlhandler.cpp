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

#include "test_i_module_htmlhandler.h"

#include <iostream>

#include "ace/Date_Time.h"

#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

#include "stream_macros.h"

Test_I_Stream_Module_HTMLHandler::Test_I_Stream_Module_HTMLHandler ()
 : inherited ()
 , configuration_ ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLHandler::Test_I_Stream_Module_HTMLHandler"));

}

Test_I_Stream_Module_HTMLHandler::~Test_I_Stream_Module_HTMLHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLHandler::~Test_I_Stream_Module_HTMLHandler"));

}

void
Test_I_Stream_Module_HTMLHandler::handleDataMessage (Test_I_Stream_Message*& message_inout,
                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLHandler::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);
  ACE_UNUSED_ARG (message_inout);
}

void
Test_I_Stream_Module_HTMLHandler::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLHandler::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  // *TODO*: remove type inferences
  const Test_I_Stream_SessionData_t& session_data_container_r =
      message_inout->get ();
  const Test_I_Stream_SessionData& session_data_r =
      session_data_container_r.get ();

  switch (message_inout->type ())
  {
    case STREAM_SESSION_END:
    {
      if (!session_data_r.parserContext)
        return;

      // sanity check(s)
      ACE_ASSERT (session_data_r.parserContext);
      ACE_ASSERT (session_data_r.parserContext->parserContext);

      if (session_data_r.parserContext->parserContext->myDoc)
      {
        result =
            htmlParseChunk(session_data_r.parserContext->parserContext, // context
                           ACE_TEXT_ALWAYS_CHAR (""),                   // chunk
                           0,                                           // size
                           1);                                          // terminate ?
        if (result)
        {
          xmlParserErrors error = static_cast<xmlParserErrors> (result);
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("failed to htmlParseChunk() (error was: %d), continuing\n"),
                      error));
        } // end IF

        if (!session_data_r.parserContext->parserContext->wellFormed)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("HTML document not well formed, continuing\n")));
        if (session_data_r.parserContext->parserContext->errNo)
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("HTML document had errors ((last) error was: %d), continuing\n"),
                      session_data_r.parserContext->parserContext->errNo));

        // extract headlines from HTML
//        xmlXPathContextPtr xpath_context_p = xmlXPathNewContext (document_);
        xmlXPathContextPtr xpath_context_p =
            xmlXPathNewContext (session_data_r.parserContext->parserContext->myDoc);
        if (!xpath_context_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to xmlXPathNewContext(); \"%m\", returning\n")));
          return;
        } // end IF
        xmlChar* query_string_p =
            BAD_CAST (ACE_TEXT_ALWAYS_CHAR (HTMLHANDLER_XPATH_QUERY_STRING));
        xmlXPathObjectPtr xpath_object_p =
            xmlXPathEvalExpression (query_string_p,
                                    xpath_context_p);
        if (!xpath_object_p)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to xmlXPathEvalExpression(\"%s\"); \"%m\", returning\n"),
                      ACE_TEXT (query_string_p)));

          // clean up
          xmlXPathFreeContext (xpath_context_p);

          return;
        } // end IF
        ACE_ASSERT (xpath_object_p->nodesetval);

        for (int i = 0;
             i < xpath_object_p->nodesetval->nodeNr;
             i++)
        {
          ACE_ASSERT (xpath_object_p->nodesetval->nodeTab);
          ACE_ASSERT (xpath_object_p->nodesetval->nodeTab[i]);
          ACE_ASSERT (xpath_object_p->nodesetval->nodeTab[i]->children);
          std::cout << ACE_TEXT_ALWAYS_CHAR (xpath_object_p->nodesetval->nodeTab[i]->children[0].content)
                    << std::endl;
        } // end FOR

        // clean up
        xmlXPathFreeObject (xpath_object_p);
        xmlXPathFreeContext (xpath_context_p);
      } // end IF
      else
      {
        ACE_Date_Time date;
        date.update ();
        for (Test_I_DataConstIterator_t iterator = session_data_r.data.begin ();
             iterator != session_data_r.data.end ();
             ++iterator)
        {
          date.update ((*iterator).first);
          std::cout << date.day () << '.' << date.month () << '.' << date.year ()
                    << std::endl;

          for (Test_I_DataItemsIterator_t iterator2 = (*iterator).second.begin ();
               iterator2 != (*iterator).second.end ();
               ++iterator2)
            std::cout << *iterator2 << std::endl;
        } // end FOR
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

const Test_I_Stream_ModuleHandlerConfiguration&
Test_I_Stream_Module_HTMLHandler::get () const
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLHandler::get"));

  return configuration_;
}
bool
Test_I_Stream_Module_HTMLHandler::initialize (const Test_I_Stream_ModuleHandlerConfiguration& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Module_HTMLHandler::initialize"));

  configuration_ = configuration_in;

  return true;
}
