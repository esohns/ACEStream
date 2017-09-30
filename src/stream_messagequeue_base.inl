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
void
Stream_MessageQueueBase_T<ACE_SYNCH_USE,
                          TimePolicyType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageQueueBase_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("# currently queued objects: %d\n"),
              const_cast<OWN_TYPE_T*> (this)->message_count ()));
}
