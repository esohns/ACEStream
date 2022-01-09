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

#if defined (_GNU_SOURCE)
#include "unistd.h"
#endif // _GNU_SOURCE

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
 , isShuttingDown_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::Stream_MessageQueueBase_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
int
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::dequeue_head (ACE_Message_Block*& message_out,
                                                         ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::dequeue_head"));

  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock (), -1);

  if (unlikely (inherited::state_ == ACE_Message_Queue_Base::DEACTIVATED))
  {
    errno = ESHUTDOWN;
    return -1;
  } // end IF

  if (unlikely (inherited::wait_not_empty_cond (timeout_in) == -1))
    return -1;

  int result = inherited::dequeue_head_i (message_out);
  if (unlikely (result == -1))
    return -1;
  ACE_ASSERT (message_out);
  if (unlikely (message_out->msg_type () == ACE_Message_Block::MB_STOP))
    isShuttingDown_ = true;

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
int
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::dequeue_head_i (ACE_Message_Block*& message_out,
                                                           ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::dequeue_head_i"));

//  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock (), -1);

  if (unlikely (inherited::state_ == ACE_Message_Queue_Base::DEACTIVATED))
  {
    errno = ESHUTDOWN;
    return -1;
  } // end IF

  if (unlikely (inherited::wait_not_empty_cond (timeout_in) == -1))
    return -1;

  message_out = this->head_;
  this->head_ = this->head_->next ();

  if (this->head_ == 0)
    this->tail_ = 0;
  else
    // The prev pointer of first message block must point to 0...
    this->head_->prev (0);

  size_t mb_bytes = 0;
  size_t mb_length = 0;
  message_out->total_size_and_length (mb_bytes,
                                      mb_length);
  // Subtract off all of the bytes associated with this message.
  this->cur_bytes_ -= mb_bytes;
  this->cur_length_ -= mb_length;
  --this->cur_count_;

  if (this->cur_count_ == 0 && this->head_ == this->tail_)
    this->head_ = this->tail_ = 0;

  // Make sure that the prev and next fields are 0!
  message_out->prev (0);
  message_out->next (0);

#if defined (ACE_HAS_MONITOR_POINTS) && (ACE_HAS_MONITOR_POINTS == 1)
  this->monitor_->receive (this->cur_length_);
#endif

  // Only signal enqueueing threads if we've fallen below the low
  // water mark.
  if (this->cur_bytes_ <= this->low_water_mark_
       && this->signal_enqueue_waiters () == -1)
    return -1;

  if (unlikely (message_out->msg_type () == ACE_Message_Block::MB_STOP))
    isShuttingDown_ = true;

  return ACE_Utils::truncate_cast<int> (this->cur_count_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
int
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::enqueue_head_i (ACE_Message_Block* messageBlock_in,
                                                           ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::enqueue_head_i"));

  int queue_count = 0;
  {
//    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, inherited::lock_, -1);

    if (inherited::state_ == ACE_Message_Queue_Base::DEACTIVATED)
    {
      errno = ESHUTDOWN;
      return -1;
    }

    if (inherited::wait_not_full_cond (timeout_in) == -1)
      return -1;

    queue_count = inherited::enqueue_head_i (messageBlock_in);
    if (queue_count == -1)
      return -1;

#if defined (ACE_HAS_MONITOR_POINTS) && (ACE_HAS_MONITOR_POINTS == 1)
    inherited::monitor_->receive (inherited::cur_length_);
#endif
  }

  if (inherited::notification_strategy_)
    inherited::notification_strategy_->notify ();
  return queue_count;
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
  int result = -1;
  size_t count = 0;
  bool has_waited = false;
  OWN_TYPE_T* this_p = const_cast<OWN_TYPE_T*> (this);

  do
  {
    count = this_p->message_count ();
    if (!count || this_p->deactivated ())
      break;
    has_waited = true;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("waiting for %u message(s)...\n"),
                count));

#if defined (_GNU_SOURCE)
    result = TEMP_FAILURE_RETRY (ACE_OS::sleep (one_second));
#else
    result = ACE_OS::sleep (one_second);
#endif // _GNU_SOURCE
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &one_second));
  } while (true);

  if (has_waited)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("waiting for message(s)...DONE\n")));
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
