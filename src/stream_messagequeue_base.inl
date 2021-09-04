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
#include "ace/OS.h"
#include "ace/Time_Value.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::Stream_MessageQueueBase_T (unsigned int maxMessages_in,
                                                                      ACE_Notification_Strategy* notificationInterface_in)
 : inherited (maxMessages_in,           // high water mark
              maxMessages_in,           // low water mark
              notificationInterface_in) // notification strategy
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::Stream_MessageQueueBase_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
unsigned int
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::flush (bool flushSessionMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::flush"));

  ACE_UNUSED_ARG (flushSessionMessages_in);

  int result = inherited::flush ();
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", returning\n")));
    return 0;
  } // end IF

  return static_cast<unsigned int> (result);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
void
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::waitForIdleState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::waitForIdleState"));

  ACE_Time_Value one_second (1, 0);
  size_t number_of_messages = 0;
  int result = -1;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  do
  {
    number_of_messages = this_p->message_count ();
    if (unlikely (number_of_messages > 0))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("waiting (count: %u message(s))...\n"),
                  number_of_messages));

      result = ACE_OS::sleep (one_second);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                    &one_second));

      continue;
    } // end IF

    // OK: queue is empty ATM
    break;
  } while (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
void
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("%d queued message(s) in %u byte(s)\n"),
              const_cast<OWN_TYPE_T*> (this)->message_count (),
              const_cast<OWN_TYPE_T*> (this)->message_bytes ()));
}
