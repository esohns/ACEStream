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
SAXDefaultErrorCallback (void* context_in,
                         const char* message_in,
                         ...)
{
  STREAM_TRACE (ACE_TEXT ("::SAXDefaultErrorCallback"));

  ACE_TCHAR buffer[BUFSIZ];
  va_list arguments;

  va_start (arguments, message_in);
  int length = ACE_OS::vsnprintf (buffer,
                                  sizeof (buffer),
//                                  sizeof (buffer) / sizeof (buffer[0]),
                                  message_in, arguments);
  va_end (arguments);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("SAXDefaultErrorCallback: %s\n"),
              buffer));
}

void
SAXDefaultStructuredErrorCallback (void* userData_in,
                                   xmlErrorPtr error_in)
{
  STREAM_TRACE (ACE_TEXT ("::SAXDefaultStructuredErrorCallback"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("SAXDefaultStructuredErrorCallback: %s\n"),
              ACE_TEXT (error_in->message)));
}
