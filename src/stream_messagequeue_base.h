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

#ifndef STREAM_MESSAGEQUEUE_BASE_H
#define STREAM_MESSAGEQUEUE_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Queue_T.h"

#include "common_idumpstate.h"

#include "stream_imessagequeue.h"

// forward declarations
class ACE_Notification_Strategy;

template <ACE_SYNCH_DECL,
          typename TimePolicyType>
class Stream_MessageQueueBase_T
 : public ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType>,
   public Stream_IMessageQueue,
   public Common_IDumpState
{
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> inherited;

 public:
  Stream_MessageQueueBase_T (unsigned int,                       // maximum # of queued messages
                             ACE_Notification_Strategy* = NULL); // notification callback handle
  inline virtual ~Stream_MessageQueueBase_T () {};

  // implement Stream_IMessageQueue
  inline virtual unsigned int flush (bool = false) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) };
  inline virtual void waitForIdleState () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // convenient types
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> MESSAGE_QUEUE_T;
  typedef ACE_Message_Queue_Iterator<ACE_SYNCH_USE,
                                     TimePolicyType> MESSAGE_QUEUE_ITERATOR_T;

  // *IMPORTANT NOTE*: override so that the queue considers the # of enqueued
  //                   messages (instead of the amount of enqueued bytes) to
  //                   determine its' water mark
  inline virtual bool is_full_i (void) { return (inherited::cur_count_ >= inherited::high_water_mark_); };

 private:
  // convenient types
  typedef Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                                    TimePolicyType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageQueueBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageQueueBase_T (const Stream_MessageQueueBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageQueueBase_T& operator= (const Stream_MessageQueueBase_T&))
};

// include template definition
#include "stream_messagequeue_base.inl"

#endif
