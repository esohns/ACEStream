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

#include <ace/Log_Msg.h>

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
                     SessionMessageType>::Stream_SessionBase_T ()
 : condition_ (lock_)
 , lock_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::Stream_SessionBase_T"));

}

template <typename SessionIdType,
          typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType>
Stream_SessionBase_T<SessionIdType,
                     SessionDataType,
                     SessionEventType,
                     MessageType,
                     SessionMessageType>::~Stream_SessionBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::~Stream_SessionBase_T"));

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

  // *TODO*: support waiting for specific session events
  ACE_UNUSED_ARG (waitForSessionEnd_in);

  int result = condition_.wait (timeout_in);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ETIME) // Linux: 62
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::wait(): \"%m\", continuing\n")));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("ACE_Condition::wait(): \"%m\", continuing\n")));
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

  int result = condition_.broadcast ();
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

  int result = condition_.broadcast ();
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
                     SessionMessageType>::notify (SessionIdType sessionId_in,
                                                  const SessionEventType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
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
                     SessionMessageType>::notify (SessionIdType sessionId_in,
                                                  const MessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (message_in);
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
                     SessionMessageType>::notify (SessionIdType sessionId_in,
                                                  const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionBase_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionMessage_in);
}
