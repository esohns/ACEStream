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

#include "stream_module_htmlparser.h"

#include "stream_html_defines.h"

//Stream_HTML_Export const char libacestream_default_html_parser_module_name_string[] =
const char libacestream_default_html_parser_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (MODULE_HTML_PARSER_DEFAULT_NAME_STRING);

//void
//SAXDefaultStartDocument (void* userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::SAXDefaultStartDocument"));
//
//  ACE_UNUSED_ARG (userData_in);
//
//  // *TODO*
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}
//void
//SAXDefaultEndDocument (void* userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::SAXDefaultEndDocument"));
//
//  ACE_UNUSED_ARG (userData_in);
//
//  // *TODO*
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}
//void
//SAXDefaultCharacters (void* userData_in,
//                      const xmlChar* string_in,
//                      int length_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::SAXDefaultCharacters"));
//
//  ACE_UNUSED_ARG (userData_in);
//  ACE_UNUSED_ARG (string_in);
//  ACE_UNUSED_ARG (length_in);
//
//  // *TODO*
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}
//void
//SAXDefaultStartElement (void* userData_in,
//                        const xmlChar* name_in,
//                        const xmlChar** attributes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::SAXDefaultStartElement"));
//
//  ACE_UNUSED_ARG (userData_in);
//  ACE_UNUSED_ARG (name_in);
//  ACE_UNUSED_ARG (attributes_in);
//
//  // *TODO*
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}
//void
//SAXDefaultEndElement (void* userData_in,
//                      const xmlChar* name_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::SAXDefaultEndElement"));
//
//  ACE_UNUSED_ARG (userData_in);
//  ACE_UNUSED_ARG (name_in);
//
//  // *TODO*
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//
//  ACE_NOTREACHED (return;)
//}
//xmlEntityPtr
//SAXDefaultGetEntity (void* userData_in,
//                     const xmlChar* name_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::SAXDefaultGetEntity"));
//
//  ACE_UNUSED_ARG (userData_in);
//  ACE_UNUSED_ARG (name_in);
//
//  // *TODO*
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (NULL);
//
//  ACE_NOTREACHED (return NULL;)
//}

////////////////////////////////////////////////////////////////////////////////

void
stream_html_parser_sax_default_error_cb (void* userData_in,
                                         const char* message_in,
                                         ...)
{
  STREAM_TRACE (ACE_TEXT ("::stream_html_parser_sax_default_error_cb"));

  ACE_UNUSED_ARG (userData_in);

  ACE_TCHAR buffer_a[BUFSIZ];
  va_list arguments_a;

  va_start (arguments_a, message_in);
  int length = ACE_OS::vsnprintf (buffer_a,
                                  sizeof (ACE_TCHAR[BUFSIZ]),
//                                  sizeof (buffer) / sizeof (buffer[0]),
                                  message_in, arguments_a);
  ACE_UNUSED_ARG (length);
  va_end (arguments_a);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("stream_html_parser_sax_default_error_cb: %s"),
              buffer_a));
}

void
stream_html_parser_sax_default_structured_error_cb (void* userData_in,
                                                    const xmlError* error_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_html_parser_sax_default_structured_error_cb"));

  ACE_UNUSED_ARG (userData_in);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("stream_html_parser_sax_default_structured_error_cb: %s"),
              ACE_TEXT (error_in->message)));
}
