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

#ifndef STREAM_HTML_TOOLS_H
#define STREAM_HTML_TOOLS_H

#if defined (LIBXML2_SUPPORT)
#include "libxml/xmlerror.h"
#include "libxml/xpath.h"

#include <string>
#endif // LIBXML2_SUPPORT

#include "ace/Global_Macros.h"
#include "ace/Log_Priority.h"

#if defined (LIBXML2_SUPPORT)
#include "stream_html_common.h"
#endif // LIBXML2_SUPPORT

class Stream_HTML_Tools
{
 public:
#if defined (LIBXML2_SUPPORT)
  static ACE_Log_Priority errorLevelToLogPriority (xmlErrorLevel); // error level

  static xmlXPathObject* query (xmlDoc*,                              // document handle
                                const Stream_HTML_XPathNameSpaces_t&, // namespaces
                                const std::string&);                  // query
#endif // LIBXML2_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_HTML_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HTML_Tools (const Stream_HTML_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HTML_Tools& operator= (const Stream_HTML_Tools&))
};

#endif
