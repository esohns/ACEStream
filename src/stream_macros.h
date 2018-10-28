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

#ifndef STREAM_MACROS_H
#define STREAM_MACROS_H

// tracing //

#define STREAM_TRACE_IMPL(X) ACE_Trace ____ (ACE_TEXT (X), __LINE__, ACE_TEXT (__FILE__))

// by default tracing is turned off
#if !defined (STREAM_NTRACE)
#  define STREAM_NTRACE 1
#endif /* STREAM_NTRACE */

#if (STREAM_NTRACE == 1)
#  define STREAM_TRACE(X)
#else
#  if !defined (STREAM_HAS_TRACE)
#    define STREAM_HAS_TRACE
#  endif /* STREAM_HAS_TRACE */
#  define STREAM_TRACE(X) STREAM_TRACE_IMPL(X)
#  include "ace/Trace.h"
#endif /* STREAM_NTRACE */

// application //

#define STREAM_CHECK_VERSION(major,minor,micro)                                                                    \
  ((ACEStream_VERSION_MAJOR > major)                                                                            || \
   ((ACEStream_VERSION_MAJOR == major) && (ACEStream_VERSION_MINOR > minor))                                    || \
   ((ACEStream_VERSION_MAJOR == major) && (ACEStream_VERSION_MINOR == minor) && (ACEStream_VERSION_MICRO >= micro)))

#define STREAM_MAKE_VERSION_STRING_VARIABLE(program,version,variable) std::string variable; do {                              \
  variable = program; variable += ACE_TEXT_ALWAYS_CHAR (" ");                                                               \
  variable += version; variable += ACE_TEXT_ALWAYS_CHAR (" compiled on ");                                                  \
  variable += ACE_TEXT_ALWAYS_CHAR (COMPILATION_DATE_TIME);                                                                   \
  variable += ACE_TEXT_ALWAYS_CHAR (" host platform "); variable += Common_Tools::compilerPlatformName ();                  \
  variable += ACE_TEXT_ALWAYS_CHAR (" with "); variable += Common_Tools::compilerName ();                                   \
  variable += ACE_TEXT_ALWAYS_CHAR (" "); variable += Common_Tools::compilerVersion ();                                     \
  variable += ACE_TEXT_ALWAYS_CHAR (" against ACE "); variable += Common_Tools::compiledVersion_ACE ();                     \
  variable += ACE_TEXT_ALWAYS_CHAR (" , Common "); variable += ACE_TEXT_ALWAYS_CHAR (Common_PACKAGE_VERSION_FULL);       \
  variable += ACE_TEXT_ALWAYS_CHAR (" , ACEStream "); variable += ACE_TEXT_ALWAYS_CHAR (ACEStream_PACKAGE_VERSION_FULL); \
} while (0)

#endif
