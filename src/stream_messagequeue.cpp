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

#include "ace/Synch.h" // *TODO*: remove this ASAP (ACE bug)
#include "stream_messagequeue.h"

#include "ace/Time_Value.h"

#include "stream_macros.h"
#include "stream_message_base.h"

Stream_MessageQueue::Stream_MessageQueue (unsigned int maxMessages_in)
 : inherited (maxMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue::Stream_MessageQueue"));

}

Stream_MessageQueue::~Stream_MessageQueue ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue::~Stream_MessageQueue"));

}

unsigned int
Stream_MessageQueue::flushData ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue::flushData"));

  int result = 0;

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (inherited::lock_);

  for (ACE_Message_Block* message_block_p = inherited::head_;
       message_block_p;
       message_block_p = message_block_p->next ())
  {
    if (message_block_p->msg_type () > STREAM_MESSAGE_MAP_2)
    {
      // remove this block
      if (message_block_p == inherited::head_)
        inherited::head_ = message_block_p->next ();
      else
        message_block_p->prev ()->next (message_block_p->next ());
      if (message_block_p == inherited::tail_)
        inherited::tail_ = message_block_p->prev ();

      // clean up
      message_block_p->release ();

      ++result;
    } // end IF
  } // end FOR

  return result;
}

void
Stream_MessageQueue::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueue::waitForIdleState"));

  // *TODO*: find a better way to do this
  ACE_Time_Value one_second (1, 0);

  do
  {
    if (const_cast<Stream_MessageQueue*> (this)->message_count () > 0)
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
