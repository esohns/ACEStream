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

#include "ace/FILE_IO.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"

#include "stream_common.h"
#include "stream_macros.h"

void
Stream_Tools::crunch (ACE_Message_Block*& messageBlock_inout,
                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::crunch"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_inout);

  // allocate a new message ?
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  size_t total_length = messageBlock_inout->total_length ();
  if (total_length > messageBlock_inout->capacity ())
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("required %d byte(s) (available: %d): allocating a new message\n"),
                total_length,
                messageBlock_inout->capacity ()));

    if (allocator_in)
    {
allocate:
      try {
        message_block_p =
          static_cast<ACE_Message_Block*> (allocator_in->malloc (total_length));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                    total_length));

        // clean up
        messageBlock_inout->release ();
        messageBlock_inout = NULL;

        return;
      }

      // keep retrying ?
      if (!message_block_p && !allocator_in->block ())
        goto allocate;
    } // end IF
    else
      ACE_NEW_NORETURN (message_block_p,
                        ACE_Message_Block (total_length,
                                           ACE_Message_Block::MB_DATA,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL,
                                           ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                                           ACE_Time_Value::zero,
                                           ACE_Time_Value::max_time,
                                           NULL,
                                           NULL));
    if (!message_block_p)
    {
      if (allocator_in)
      {
        if (allocator_in->block ())
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));
      } // end IF
      else
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", aborting\n")));

      // clean up
      messageBlock_inout->release ();
      messageBlock_inout = NULL;

      return;
    } // end IF
  } // end IF
  else
  {
    result = messageBlock_inout->crunch ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", returning\n")));

      // clean up
      messageBlock_inout->release ();
      messageBlock_inout = NULL;

      return;
    } // end IF

    for (message_block_p = messageBlock_inout->cont ();
         message_block_p;
         message_block_p = message_block_p->cont ())
    {
      ACE_ASSERT (message_block_p->length () <= messageBlock_inout->space ());
      result = messageBlock_inout->copy (message_block_p->rd_ptr (),
                                         message_block_p->length ());
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

        // clean up
        messageBlock_inout->release ();
        messageBlock_inout = NULL;

        return;
      } // end IF
    } // end FOR

    return;
  } // end ELSE

  for (ACE_Message_Block* message_block_2 = messageBlock_inout;
       message_block_2;
       message_block_2 = message_block_2->cont ())
  {
    ACE_ASSERT (message_block_2->length () <= message_block_p->space ());
    result = message_block_p->copy (message_block_2->rd_ptr (),
                                    message_block_2->length ());
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));

      // clean up
      message_block_p->release ();
      messageBlock_inout->release ();
      messageBlock_inout = NULL;

      return;
    } // end IF
  } // end FOR

  messageBlock_inout->release ();
  messageBlock_inout = message_block_p;
}

void
Stream_Tools::dump (const ACE_Message_Block* messageBlock_in,
                    const std::string& filename_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);
  ACE_ASSERT (Common_File_Tools::isValidFilename (filename_in));

  int open_flags = (O_CREAT |
                    O_TRUNC |
                    O_WRONLY);
  ACE_FILE_IO file_stream;
  if (!Common_File_Tools::open (filename_in,
                                open_flags,
                                file_stream))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_File_Tools::open(\"%s\"), returning\n"),
                ACE_TEXT (filename_in.c_str ())));
    return;
  } // end IF

  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  size_t bytes_transferred = std::numeric_limits<unsigned int>::max ();
#else
  size_t bytes_transferred = -1;
#endif
  ssize_t bytes_written =
    file_stream.send_n (messageBlock_in,
                        NULL,                // timeout
                        &bytes_transferred); // bytes transferred
  if (bytes_written != static_cast<ssize_t> (messageBlock_in->total_length ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_File_IO::send_n(): \"%m\" [wrote %d/%d bytes], aborting\n"),
                bytes_transferred,
                messageBlock_in->total_length ()));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("wrote %d byte(s) to \"%s\"\n"),
              bytes_transferred,
              ACE_TEXT (filename_in.c_str ())));

error:
  result = file_stream.close ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_File_IO::close(): \"%m\", continuing\n")));
}

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
  case STREAM_MESSAGE_CONTROL:
    result = ACE_TEXT_ALWAYS_CHAR ("MESSAGE_CONTROL"); break;
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
