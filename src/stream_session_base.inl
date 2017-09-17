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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::Stream_SessionBase_T (ACE_SYNCH_MUTEX* lock_in)
 : inSession_ (false)
 , lock_ (lock_in)
 /////////////////////////////////////////
 , condition_ (*lock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::Stream_SessionBase_T"));

  // sanity check(s)
  ACE_ASSERT (lock_in);
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::wait (bool waitForSessionEnd_in,
                                                const ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::wait"));

  int result = -1;

  if (( waitForSessionEnd_in && !inSession_) ||
      (!waitForSessionEnd_in &&  inSession_))
    return; // nothing to do

  // sanity check(s)
  ACE_ASSERT (lock_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *lock_);
    result = condition_.wait (timeout_in);
  } // end lock scope
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ETIME) // Linux: 62
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::wait(): \"%m\", continuing\n")));
  } // end IF
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::startCB ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::startCB"));

  int result = -1;

  result = condition_.broadcast ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", returning\n")));
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::endCB ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::endCB"));

  int result = -1;

  result = condition_.broadcast ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", returning\n")));
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::start (SessionIdType sessionId_in,
                                                 const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

  // sanity check(s)
  ACE_ASSERT (lock_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *lock_);
    inSession_ = true;
  } // end lock scope

  try {
    startCB ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_ISessionCB::startCB(), continuing\n")));
  }
}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::end (SessionIdType sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  try {
    endCB ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_ISessionCB::endCB(), continuing\n")));
  }

  // sanity check(s)
  ACE_ASSERT (lock_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *lock_);
    inSession_ = false;
  } // end lock scope
}
