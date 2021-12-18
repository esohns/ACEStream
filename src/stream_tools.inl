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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicy>
ACE_Message_Block*
Stream_Tools::get (ACE_UINT64 bytes_in,
                   ACE_Message_Queue<ACE_SYNCH_USE,
                                     TimePolicy>* queue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Tools::get"));

  ACE_Message_Block* result_p = NULL;

  // sanity check(s)
  ACE_ASSERT (bytes_in);
  ACE_ASSERT (queue_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL, *message_block_2 = NULL, *message_block_3 = NULL;
  ACE_UINT64 remaining_bytes_i = bytes_in, total_length_i = 0;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  ACE_UINT64 skipped_bytes_i = 0;

  while (remaining_bytes_i)
  {
    result = queue_in->dequeue_head (message_block_p,
                                     &no_wait);
    if (unlikely (result == -1))
    { int error = ACE_OS::last_error ();
      if (unlikely (error != EWOULDBLOCK))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::dequeue_head(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (message_block_p);
    total_length_i = message_block_p->total_length ();
    if (total_length_i <= remaining_bytes_i)
    {
      if (!result_p)
        result_p = message_block_p;
      else
      { // append
        message_block_2 = result_p;
        while (message_block_2->cont ())
          message_block_2 = message_block_2->cont ();
        message_block_2->cont (message_block_p);
      } // end ELSE
      message_block_p = NULL;
      remaining_bytes_i -= total_length_i;
      continue;
    } // end IF

    // total_length_i > remaining_bytes_i --> crop message_block_p
    message_block_2 = message_block_p;
    while (skipped_bytes_i < remaining_bytes_i)
    {
      skipped_bytes_i += message_block_2->length ();
      message_block_3 = message_block_2;
      message_block_2 = message_block_2->cont ();
    } // end WHILE

    // skipped_bytes_i >= remaining_bytes_i
    if (skipped_bytes_i == remaining_bytes_i)
    {
      message_block_3->cont (NULL);
      if (!result_p)
        result_p = message_block_p;
      else
      { // append
        message_block_3 = result_p;
        while (message_block_3->cont ())
          message_block_3 = message_block_3->cont ();
        message_block_3->cont (message_block_p);
      } // end ELSE

      // requeue message_block_2
      result = queue_in->enqueue_head (message_block_2);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::enqueue_head(): \"%m\", aborting\n")));
        message_block_2->release ();
        goto error;
      } // end IF

      break;
    } // end IF

    // skipped_bytes_i > remaining_bytes_i

    message_block_2 = message_block_3->duplicate ();
    if (unlikely (!message_block_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", aborting\n")));
      goto error;
    } // end IF
    message_block_3->cont (NULL);
    message_block_3->length (remaining_bytes_i);

    if (!result_p)
      result_p = message_block_p;
    else
    { // append
      message_block_3 = result_p;
      while (message_block_3->cont ())
        message_block_3 = message_block_3->cont ();
      message_block_3->cont (message_block_p);
    } // end ELSE

    // requeue message_block_2
    message_block_2->rd_ptr (remaining_bytes_i);
    result = queue_in->enqueue_head (message_block_2);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::enqueue_head(): \"%m\", aborting\n")));
      message_block_2->release ();
      goto error;
    } // end IF

    break;
  } // end WHILE

  return result_p;

error:
  while (result_p)
  {
    message_block_p = result_p;
    if (message_block_p->cont ())
    {
      while (message_block_p->cont ())
      {
        message_block_2 = message_block_p;
        message_block_p = message_block_p->cont ();
      } // end WHILE
      message_block_2->cont (NULL);
    } // end IF
    else
      result_p = NULL;
    result = queue_in->enqueue_head (message_block_p);
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::enqueue_head(): \"%m\", continuing\n")));
  } // end WHILE

  return NULL;
}
