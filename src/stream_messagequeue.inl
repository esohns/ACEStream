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
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
Stream_MessageQueue_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      SessionMessageType>::Stream_MessageQueue_T (size_t maxMessages_in,
                                                                  ACE_Notification_Strategy* notificationInterface_in)
 : inherited (maxMessages_in,
              notificationInterface_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue_T::Stream_MessageQueue_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename SessionMessageType>
unsigned int
Stream_MessageQueue_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      SessionMessageType>::flush (bool flushSessionMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue_T::flush"));

  int result = 0;
  ACE_Message_Block* temp_p = NULL;
  size_t bytes = 0;
  size_t length = 0;
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, 0);
    message_block_p = inherited::head_;
    while (message_block_p)
    {
      switch (message_block_p->msg_type ())
      {
        case STREAM_MESSAGE_SESSION:
        {
          if (likely (!flushSessionMessages_in))
            break;
          // *WARNING*: control falls through here
        }
        case STREAM_MESSAGE_DATA:
        case STREAM_MESSAGE_OBJECT:
        {
          // remove this block
          if (message_block_p == inherited::head_)
            inherited::head_ = message_block_p->next ();
          else
          {
            message_block_p->prev ()->next (message_block_p->next ());
            if (message_block_p->prev ()->next ())
              message_block_p->prev ()->next ()->prev (message_block_p->prev ());
          } // end ELSE
          if (unlikely (message_block_p == inherited::tail_))
          { // *TODO*: just use message_block_p->prev () ?
            temp_p = inherited::head_ ? inherited::head_ : NULL;
            if (temp_p)
              while (temp_p->next ())
                temp_p = temp_p->next ();
            inherited::tail_ = temp_p;
          } // end IF

          // clean up
          bytes = 0; length = 0;
          message_block_p->total_size_and_length (bytes,
                                                  length);
          inherited::cur_bytes_ -= bytes;
          inherited::cur_length_ -= length;
          --inherited::cur_count_;

          temp_p = message_block_p;
          message_block_p = message_block_p->next ();
          temp_p->release (); temp_p = NULL;

          ++result;

          continue;
        }
        case STREAM_MESSAGE_CONTROL:
        default:
          break;
      } // end SWITCH

      message_block_p = message_block_p->next ();
    } // end WHILE

    // signal waiters ?
    if (unlikely (result &&
                  (inherited::cur_count_ < inherited::low_water_mark_)))
    {
      result_2 = inherited::signal_enqueue_waiters ();
      if (unlikely (result_2 == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::signal_enqueue_waiters(): \"%m\", continuing\n")));
    } // end IF
  } // end lock scope

  return static_cast<unsigned int> (result);
}
