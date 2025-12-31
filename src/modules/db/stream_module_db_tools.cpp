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

#include "stream_module_db_tools.h"

#if defined (SOCI_SUPPORT)
#if defined (MYSQL_SUPPORT)
#include "soci/mysql/soci-mysql.h"
#endif // MYSQL_SUPPORT
#endif // SOCI_SUPPORT

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

void
Stream_Module_DataBase_Tools::initialize()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_DataBase_Tools::initialize"));

#if defined (SOCI_SUPPORT)
#if defined (MYSQL_SUPPORT)
  soci::register_factory_mysql ();
#endif // MYSQL_SUPPORT
#endif // SOCI_SUPPORT
}

std::string
Stream_Module_DataBase_Tools::timestampToDatabaseString (const ACE_Time_Value& timestamp_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_DataBase_Tools::timestampToDatabaseString"));

  // initialize return value(s)
  std::string result;

  //ACE_Date_Time time_local (timestamp_in);
//   struct tm time_local;
//   time_local.tm_sec = -1;
//   time_local.tm_min = -1;
//   time_local.tm_hour = -1;
//   time_local.tm_mday = -1;
//   time_local.tm_mon = -1;
//   time_local.tm_year = -1;
//   time_local.tm_wday = -1;
//   time_local.tm_yday = -1;
//   time_local.tm_isdst = -1; // expect localtime !!!
//   // *PORTABILITY*: this isn't entirely portable so do an ugly hack
// #if defined (ACE_WIN32) || defined (ACE_WIN64)
// #else
//   time_local.tm_gmtoff = 0;
//   time_local.tm_zone = NULL;
// #endif // ACE_WIN32 || ACE_WIN64

  // step1: compute UTC representation
  time_t time_seconds = timestamp_in.sec ();
  // *PORTABILITY*: the man page suggests calling this first...
  ACE_OS::tzset ();
  // if (unlikely (!ACE_OS::localtime_r (&time_seconds,
  //                                     &time_local)))
  // {
  //   ACE_DEBUG ((LM_ERROR,
  //               ACE_TEXT ("failed to ACE_OS::localtime_r(): \"%m\", aborting\n")));
  //   return result;
  // } // end IF

  // step2: create string
  char buffer_a[BUFSIZ];
  // if (unlikely (ACE_OS::strftime (buffer_a,
  //                                 sizeof (char[BUFSIZ]),
  //                                 ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_DB_TOOLS_STRFTIME_FORMAT),
  //                                 &time_local) != STREAM_MODULE_DB_TOOLS_STRFTIME_SIZE))
  if (unlikely (std::strftime (buffer_a,
                               sizeof (char[BUFSIZ]),
                               ACE_TEXT_ALWAYS_CHAR (STREAM_MODULE_DB_TOOLS_STRFTIME_FORMAT),
                               std::localtime (&time_seconds)) != STREAM_MODULE_DB_TOOLS_STRFTIME_SIZE))
  {
    // ACE_DEBUG ((LM_ERROR,
    //             ACE_TEXT ("failed to ACE_OS::strftime(): \"%m\", aborting\n")));
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to std::strftime(): \"%m\", aborting\n")));
    return result;
  } // end IF
  result = buffer_a;

  // OK: append any usecs
  if (likely (timestamp_in.usec ()))
  {
    std::ostringstream converter;
    converter << timestamp_in.usec ();
    result += '.';
    result += converter.str ();
  } // end IF

  return result;
}
