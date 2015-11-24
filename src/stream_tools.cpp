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

#include "stream_tools.h"

#include <sstream>

#include "ace/OS.h"
#include "ace/Log_Msg.h"

#include "stream_common.h"
#include "stream_macros.h"

std::string
Stream_Tools::timeStamp2LocalString (const ACE_Time_Value& timeStamp_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::timeStamp2LocalString"));

  // initialize return value(s)
  std::string result;

  //ACE_Date_Time time_local(timestamp_in);
  tm time_local;
  // init structure
  time_local.tm_sec = -1;
  time_local.tm_min = -1;
  time_local.tm_hour = -1;
  time_local.tm_mday = -1;
  time_local.tm_mon = -1;
  time_local.tm_year = -1;
  time_local.tm_wday = -1;
  time_local.tm_yday = -1;
  time_local.tm_isdst = -1; // expect localtime !!!
  // *PORTABILITY*: this isn't entirely portable so do an ugly hack
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  time_local.tm_gmtoff = 0;
  time_local.tm_zone = NULL;
#endif

  // step1: compute UTC representation
  time_t time_seconds = timeStamp_in.sec ();
  // *PORTABILITY*: man page says we should call this before...
  ACE_OS::tzset ();
  if (!ACE_OS::localtime_r (&time_seconds,
                            &time_local))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::localtime_r(): \"%m\", aborting\n")));
    return result;
  } // end IF

  // step2: create string
  // *TODO*: rewrite this in C++
  char time_string[BUFSIZ];
  if (ACE_OS::strftime (time_string,
                        sizeof (time_string),
                        ACE_TEXT_ALWAYS_CHAR (STREAM_TOOLS_STRFTIME_FORMAT),
                        &time_local) != STREAM_TOOLS_STRFTIME_SIZE)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::strftime(): \"%m\", aborting\n")));
    return result;
  } // end IF
  result = time_string;

  // OK: append any usecs
  if (timeStamp_in.usec ())
  {
    std::ostringstream converter;
    converter << timeStamp_in.usec ();
    result += ACE_TEXT_ALWAYS_CHAR (".");
    result += converter.str ();
  } // end IF

  return result;
}

std::string
Stream_Tools::messageType2String (Stream_MessageType messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::messageType2String"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (messageType_in)
  {
    case STREAM_MESSAGE_SESSION:
      result = ACE_TEXT_ALWAYS_CHAR ("MESSAGE_SESSION"); break;
    case STREAM_MESSAGE_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("MESSAGE_DATA"); break;
    case STREAM_MESSAGE_OBJECT:
      result = ACE_TEXT_ALWAYS_CHAR ("MESSAGE_OBJECT"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (type: \"%d\"), aborting\n"),
                  messageType_in));
      break;
    }
  } // end SWITCH

  return result;
}
