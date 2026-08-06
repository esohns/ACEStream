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

#include "stream_dec_webm_demuxer.h"

#include "ace/Time_Value.h"

#include "common_time_common.h"

#include "stream_dec_defines.h"

const char libacestream_default_dec_webm_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_WEBM_DEMUXER_DEFAULT_NAME_STRING);

//////////////////////////////////////////

int64_t
acestream_webm_demuxer_io_read_cb (void* buffer_in,
                                   size_t bufferSize_in,
                                   void* userData_in)
{
  // sanity check(s)
  struct ACEStream_WebM_Demuxer_CBData* cb_data_p =
    static_cast<struct ACEStream_WebM_Demuxer_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (buffer_in);
  ACE_ASSERT (bufferSize_in);

  int bytes_read = 0;
  int result, error = 0, bytes_to_read, available_space_i = static_cast<int> (bufferSize_in), offset = 0;
  ACE_Message_Block* message_block_p = NULL, *head_p = cb_data_p->buffer;
  //static ACE_Time_Value no_wait = COMMON_TIME_NOW;

  if (cb_data_p->buffer)
  {
    message_block_p = cb_data_p->buffer;
    cb_data_p->buffer = NULL;
    goto continue_;
  } // end IF

deque:
  // sanity check(s)
  ACE_ASSERT (cb_data_p->queue);
  ACE_ASSERT (!message_block_p);

  result = cb_data_p->queue->dequeue (message_block_p,
                                      NULL/*&no_wait*/);
  if (unlikely (result == -1))
  { ACE_ASSERT (!bytes_read); // *TODO*
    bytes_read = -1;

    error = ACE_OS::last_error ();
    if (likely (error == EWOULDBLOCK))
      error = EAGAIN;
    else if (error == ESHUTDOWN)
      bytes_read = 0; // send EOF (again :()
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::dequeue(): \"%m\", aborting\n")));
    goto continue_2;
  } // end IF
  ACE_ASSERT (message_block_p);
  head_p = message_block_p;

  switch (message_block_p->msg_type ())
  {
    case ACE_Message_Block::MB_STOP:
    { ACE_ASSERT (bytes_read == 0);
      cb_data_p->queue->deactivate ();
      message_block_p->release ();
      goto continue_2;
    }
    default:
      break;
  } // end SWITCH

continue_:
  ACE_ASSERT (available_space_i > 0);
  ACE_ASSERT (message_block_p);

  bytes_to_read =
    std::min (available_space_i, static_cast<int> (message_block_p->length ()));
  if (unlikely (bytes_to_read == 0))
  { ACE_ASSERT (!message_block_p->length ());
    if (likely (message_block_p->cont ()))
    {
      ACE_Message_Block* message_block_2 = message_block_p;
      message_block_p = message_block_p->cont ();
      message_block_2->cont (NULL);
      message_block_2->release ();
      goto continue_;
    } // end IF
    message_block_p->release (); message_block_p = NULL;
    goto set_buffer;
  } // end IF

  ACE_OS::memcpy (reinterpret_cast<char*> (buffer_in) + offset, message_block_p->rd_ptr (), bytes_to_read);
  offset += bytes_to_read;
  bytes_read += bytes_to_read;
  available_space_i -= bytes_to_read;
  message_block_p->rd_ptr (bytes_to_read);

  if (available_space_i > 0)
  { ACE_ASSERT (!message_block_p->length ());
    if (unlikely (message_block_p->cont ()))
    {
      ACE_Message_Block* message_block_2 = message_block_p;
      message_block_p = message_block_p->cont ();
      message_block_2->cont (NULL);
      message_block_2->release ();
      goto continue_;
    } // end IF

    message_block_p->release (); message_block_p = NULL;
    goto deque;
    //goto set_buffer;
  } // end IF
  // --> no more space in buffer

  ACE_ASSERT (message_block_p);
  if (!message_block_p->length ())
  {
    if (unlikely (message_block_p->cont ()))
    {
      ACE_Message_Block* message_block_2 = message_block_p;
      message_block_p = message_block_p->cont ();
      message_block_2->cont (NULL);
      message_block_2->release ();
    } // end IF
  } // end IF

set_buffer:
  cb_data_p->buffer = message_block_p;

continue_2:
  if (bytes_read == 0)
    return 0; // signal end of stream
  else if (bytes_read < 0)
    return -1; // pass error tp nestegg

  cb_data_p->position += bytes_read;

  ACE_ASSERT (bytes_read == bufferSize_in);
  return bytes_read;
}

int
acestream_webm_demuxer_io_seek_cb (int64_t offset_in,
                                   int whence_in,
                                   void* userData_in)
{
  ACE_UNUSED_ARG (offset_in);
  ACE_UNUSED_ARG (whence_in);
  ACE_UNUSED_ARG (userData_in);

  return -1;
}

int64_t
acestream_webm_demuxer_io_tell_cb (void* userData_in)
{
  // sanity check(s)
  struct ACEStream_WebM_Demuxer_CBData* cb_data_p =
    static_cast<struct ACEStream_WebM_Demuxer_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

  return cb_data_p->position;
}

//////////////////////////////////////////

void
acestream_webm_demuxer_log_cb (nestegg* context_in,
                               unsigned int severity_in,
                               char const* format_in,
                               ...)
{
  char buffer_a[BUFSIZ];
  va_list arguments_a;

  va_start (arguments_a,
            format_in);
  int length = ACE_OS::vsnprintf (buffer_a,
                                  sizeof (char[BUFSIZ]),
                                  format_in,
                                  arguments_a);
  ACE_UNUSED_ARG (length);
  va_end (arguments_a);

  enum ACE_Log_Priority log_priority_e = LM_DEBUG;
  switch (severity_in)
  {
    case NESTEGG_LOG_CRITICAL:
      log_priority_e = LM_CRITICAL; break;
    case NESTEGG_LOG_ERROR:
      log_priority_e = LM_ERROR; break;
    case NESTEGG_LOG_WARNING:
      log_priority_e = LM_WARNING; break;
    case NESTEGG_LOG_INFO:
      log_priority_e = LM_INFO; break;
    case NESTEGG_LOG_DEBUG:
      log_priority_e = LM_DEBUG; break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown nestegg log severity (was: %d), continuing\n"),
                  severity_in));
      break;
    }
  } // end SWITCH
  ACE_DEBUG ((log_priority_e,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (buffer_a)));
}
