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

#include "ace/Time_Value.h"

#include "stream_macros.h"

template <typename SessionMessageType>
Stream_MessageQueue_T<SessionMessageType>::Stream_MessageQueue_T (unsigned int maxMessages_in)
 : inherited (maxMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue_T::Stream_MessageQueue_T"));

}

template <typename SessionMessageType>
Stream_MessageQueue_T<SessionMessageType>::~Stream_MessageQueue_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue_T::~Stream_MessageQueue_T"));

}

template <typename SessionMessageType>
unsigned int
Stream_MessageQueue_T<SessionMessageType>::flush (bool flushSessionMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue_T::flush"));

  int result = 0;

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited::lock_);

  ACE_Message_Block* temp_p = NULL;
  ACE_Message_Block* message_block_p = inherited::head_;
  size_t bytes = 0;
  size_t length = 0;
  int result_2 = -1;
  while (message_block_p)
  {
    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_USER:
      {
        // *NOTE*: currently, all of these are 'session' messages
        SessionMessageType* session_message_p =
          dynamic_cast<SessionMessageType*> (message_block_p);
        if (session_message_p &&
            !flushSessionMessages_in)
          break;
      }
      case ACE_Message_Block::MB_DATA:
      {
        // remove this block
        if (message_block_p == inherited::head_)
          inherited::head_ = message_block_p->next ();
        else
          message_block_p->prev ()->next (message_block_p->next ());
        if (message_block_p == inherited::tail_)
          inherited::tail_ = message_block_p->prev ();

        temp_p = message_block_p;
        message_block_p = message_block_p->next ();

        // clean up
        temp_p->total_size_and_length (bytes,
                                       length);
        inherited::cur_bytes_ -= bytes;
        inherited::cur_length_ -= length;
        --inherited::cur_count_;
        temp_p->release ();

        ++result;

        continue;
      }
      default:
      {
        //ACE_DEBUG ((LM_DEBUG,
        //            ACE_TEXT ("retaining message (type was: %d)\n"),
        //            message_block_p->msg_type ()));
        break;
      }
    } // end SWITCH

    message_block_p = message_block_p->next ();
  } // end WHILE

  // signal waiters ?
  if (result &&
      (inherited::cur_bytes_ <= inherited::low_water_mark_))
  {
    result_2 = inherited::signal_enqueue_waiters ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::signal_enqueue_waiters(): \"%m\", continuing\n")));
  } // end IF

  return result;
}

template <typename SessionMessageType>
void
Stream_MessageQueue_T<SessionMessageType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue_T::waitForIdleState"));

  // *TODO*: find a better way to do this
  ACE_Time_Value one_second (1, 0);

  do
  {
    if (const_cast<Stream_MessageQueue_T*> (this)->message_count () > 0)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting...\n")));

      ACE_OS::sleep (one_second);

      continue;
    } // end IF

    // OK: queue is empty (AT THE MOMENT !)
    break;
  } while (true);
}
