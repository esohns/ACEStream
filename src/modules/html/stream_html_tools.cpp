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

#include "stream_html_tools.h"

#if defined (LIBXML2_SUPPORT)
#include "libxml/xpathInternals.h"
#endif // LIBXML2_SUPPORT

#include "ace/Log_Msg.h"

#include "stream_macros.h"

#if defined (LIBXML2_SUPPORT)
ACE_Log_Priority
Stream_HTML_Tools::errorLevelToLogPriority (xmlErrorLevel errorLevel_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HTML_Tools::errorLevelToLogPriority"));

  switch (errorLevel_in)
  {
    case XML_ERR_NONE:
      return LM_DEBUG;
    case XML_ERR_WARNING:
      return LM_WARNING;
    case XML_ERR_ERROR:
      break;
    case XML_ERR_FATAL:
      return LM_CRITICAL;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown error level (was: %d), aborting\n"),
                  errorLevel_in));
      break;
    }
  } // end SWITCH

  return LM_ERROR;
}

xmlXPathObject*
Stream_HTML_Tools::query (xmlDoc* document_in,
                          const Stream_HTML_XPathNameSpaces_t& namespaces_in,
                          const std::string& query_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HTML_Tools::query"));

  // sanity check(s)
  ACE_ASSERT (document_in);

  // step1: create a query context
  xmlXPathContextPtr xpath_context_p = xmlXPathNewContext (document_in);
  if (unlikely (!xpath_context_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathNewContext(); \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // step2: register given namespaces
  int result = -1;
  for (Stream_HTML_XPathNameSpacesConstIterator_t iterator = namespaces_in.begin ();
       iterator != namespaces_in.end ();
       ++iterator)
  {
    result = xmlXPathRegisterNs (xpath_context_p,
                                 BAD_CAST ((*iterator).first.c_str ()),
                                 BAD_CAST ((*iterator).second.c_str ()));
    if (unlikely (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to xmlXPathRegisterNs(\"%s\" --> \"%s\"), continuing\n"),
                  ACE_TEXT ((*iterator).first.c_str ()),
                  ACE_TEXT ((*iterator).second.c_str ())));
  } // end FOR

  // step3: perform query
  xmlXPathObject* result_p =
    xmlXPathEvalExpression (BAD_CAST (query_in.c_str ()),
                            xpath_context_p);
  if (unlikely (!result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to xmlXPathEvalExpression(\"%s\"); \"%m\", aborting\n"),
                ACE_TEXT (query_in.c_str ())));
    xmlXPathFreeContext (xpath_context_p);
    return NULL;
  } // end IF

  // step4: clean up
  xmlXPathFreeContext (xpath_context_p); xpath_context_p = NULL;

  return result_p;
}
#endif // LIBXML2_SUPPORT
