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
#include "ace/Module.h"
#include "ace/OS.h"
#include "ace/Stream.h"

#include "common_file_tools.h"

#include "stream_common.h"
#include "stream_iallocator.h"
#include "stream_istreamcontrol.h"
#include "stream_macros.h"

ACE_Message_Block*
Stream_Tools::get (ACE_UINT64 bytes_in,
                   ACE_Message_Block* messageBlock_in,
                   ACE_Message_Block*& messageBlock_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::get"));

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);
//  ACE_ASSERT (!messageBlock_out);
  ACE_UINT64 total_bytes_i = messageBlock_in->total_length ();
  if (bytes_in >= total_bytes_i)
    return messageBlock_in;

  // bytes_in < total_bytes_i

  ACE_UINT64 skipped_bytes_i = 0, bytes_to_skip_i = 0;
  ACE_Message_Block* message_block_p = messageBlock_in, *message_block_2 = NULL;

  while (skipped_bytes_i < bytes_in)
  {
    skipped_bytes_i += message_block_p->length ();
    message_block_2 = message_block_p;
    message_block_p = message_block_p->cont ();
  } // end WHILE

  // skipped_bytes_i >= bytes_in

  if (skipped_bytes_i == bytes_in)
  {
    message_block_2->cont (NULL);
    messageBlock_out = message_block_p;
    return messageBlock_in;
  } // end IF

  // skipped_bytes_i > bytes_in

  bytes_to_skip_i = message_block_2->length () - (skipped_bytes_i - bytes_in);
  messageBlock_out = message_block_2->duplicate ();
  if (unlikely (!messageBlock_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  message_block_2->cont (NULL);
  message_block_2->length (bytes_to_skip_i);
  messageBlock_out->rd_ptr (bytes_to_skip_i);

  return messageBlock_in;
}

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
                ACE_TEXT ("required %u byte(s) (available: %u): allocating a new message\n"),
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
                    ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), returning\n"),
                    total_length));
        messageBlock_inout->release (); messageBlock_inout = NULL;
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
    if (unlikely (!message_block_p))
    {
      if (allocator_in)
      {
        if (allocator_in->block ())
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", returning\n")));
      } // end IF
      else
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate ACE_Message_Block: \"%m\", returning\n")));
      messageBlock_inout->release (); messageBlock_inout = NULL;
      return;
    } // end IF
  } // end IF
  else
  {
    result = messageBlock_inout->crunch ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", returning\n")));
      messageBlock_inout->release (); messageBlock_inout = NULL;
      return;
    } // end IF

    for (message_block_p = messageBlock_inout->cont ();
         message_block_p;
         message_block_p = message_block_p->cont ())
    { ACE_ASSERT (message_block_p->length () <= messageBlock_inout->space ());
      result = messageBlock_inout->copy (message_block_p->rd_ptr (),
                                         message_block_p->length ());
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
        messageBlock_inout->release (); messageBlock_inout = NULL;
        return;
      } // end IF
    } // end FOR
    return;
  } // end ELSE

  for (ACE_Message_Block* message_block_2 = messageBlock_inout;
       message_block_2;
       message_block_2 = message_block_2->cont ())
  { ACE_ASSERT (message_block_2->length () <= message_block_p->space ());
    result = message_block_p->copy (message_block_2->rd_ptr (),
                                    message_block_2->length ());
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
      message_block_p->release ();
      messageBlock_inout->release (); messageBlock_inout = NULL;
      return;
    } // end IF
  } // end FOR

  messageBlock_inout->release (); messageBlock_inout = message_block_p;
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

bool
Stream_Tools::isFirstModule (const Stream_Base_t& stream_in,
                             const Stream_Module_t& module_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::isFirstModule"));

  Stream_Iterator_t iterator (stream_in);
  const Stream_Module_t* module_p = NULL;
  bool is_first_b = true, found_b = false;
  for (;
       !iterator.done ();
       iterator.next (module_p))
  { ACE_ASSERT (module_p);
    if ((module_p == &module_in) ||
        !ACE_OS::strcmp (module_in.name (),
                         module_p->name ()))
    {
      found_b = true;
      break;
    } // end IF
    is_first_b = false;
  } // end FOR

  return found_b && is_first_b;
}

bool
Stream_Tools::has (Stream_IStream_t* stream_in,
                   const std::string& name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::has"));

  // sanity check(s)
  ACE_ASSERT (stream_in);
  ACE_ASSERT (!name_in.empty ());

  return stream_in->find (name_in,
                          false,
                          false) != NULL;
}

std::string
Stream_Tools::timeStampToLocalString (const ACE_Time_Value& timeStamp_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::timeStampToLocalString"));

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
Stream_Tools::messageTypeToString (enum Stream_MessageType messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::messageTypeToString"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (messageType_in)
  {
  case STREAM_MESSAGE_CONTROL:
    result = ACE_TEXT_ALWAYS_CHAR ("CONTROL"); break;
    case STREAM_MESSAGE_SESSION:
      result = ACE_TEXT_ALWAYS_CHAR ("SESSION"); break;
    case STREAM_MESSAGE_DATA:
      result = ACE_TEXT_ALWAYS_CHAR ("DATA"); break;
    case STREAM_MESSAGE_OBJECT:
      result = ACE_TEXT_ALWAYS_CHAR ("OBJECT"); break;
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

std::string
Stream_Tools::generateUniqueName (const std::string& prefix_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::generateUniqueName"));

  std::string result;

  // sanity check(s)
  ACE_ASSERT (prefix_in.size () <= (BUFSIZ - 6 + 1));

  // *NOTE*: see also: man 3 mkstemp
  ACE_TCHAR buffer[BUFSIZ];
  if (unlikely (!prefix_in.empty ()))
    ACE_OS::strcpy (buffer, prefix_in.c_str ());
  ACE_OS::strcpy (buffer + prefix_in.size (), ACE_TEXT ("XXXXXX"));
  ACE_OS::mktemp (buffer);
  if (unlikely (!ACE_OS::strlen (buffer)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::mktemp(): \"%m\", aborting\n")));
    return std::string ();
  } // end IF
  result = buffer;

  return result;
}

std::string
Stream_Tools::sanitizeUniqueName (const std::string& string_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::sanitizeUniqueName"));

  std::string result = string_in;

  std::string::size_type position = string_in.find_last_of ('_');
  if ((string_in.size () > (6 + 1)) && 
      (position == (string_in.size () - (6 + 1))))
    result.erase (position, std::string::npos);

  return result;
}
