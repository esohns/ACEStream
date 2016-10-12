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

#include <ace/Message_Block.h>
#include <ace/Time_Value.h>

#include "stream_common.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_HeadTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::Stream_HeadTask_T ()
 : inherited ()
 , isLinked_ (false)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadTask_T::Stream_HeadTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
Stream_HeadTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::~Stream_HeadTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadTask_T::~Stream_HeadTask_T"));

  if (sessionData_)
    sessionData_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
int
Stream_HeadTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionIdType,
                  SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                          ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadTask_T::put"));

  ACE_UNUSED_ARG (timeout_in);

  switch (messageBlock_in->msg_type ())
  {
    case ACE_Message_Block::MB_FLUSH:
      break;
    case ACE_Message_Block::MB_USER:
    { // *NOTE*: currently, all of these are 'session' messages
      SessionMessageType* session_message_p =
        dynamic_cast<SessionMessageType*> (messageBlock_in);
      if (!session_message_p)
      {
        if (inherited::mod_)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: dynamic_cast<SessionMessageType>(%@) failed (type was: %d), aborting\n"),
                      inherited::mod_->name (),
                      messageBlock_in,
                      messageBlock_in->msg_type ()));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("dynamic_cast<SessionMessageType>(%@) failed (type was: %d), aborting\n"),
                      messageBlock_in,
                      messageBlock_in->msg_type ()));

        // clean up
        messageBlock_in->release ();

        break;
      } // end IF

      Stream_SessionMessageType message_type = session_message_p->type ();
      switch (message_type)
      {
        case STREAM_SESSION_MESSAGE_LINK:
        {
          isLinked_ = true;

          // sanity check(s)
          ACE_ASSERT (!sessionData_);

          sessionData_ =
            &const_cast<typename SessionMessageType::DATA_T&> (session_message_p->get ());
          sessionData_->increase ();

          break;
        }
        case STREAM_SESSION_MESSAGE_END:
        {
          // *TODO*: merge session data every time ?
          if (isLinked_)
            session_message_p->initialize (STREAM_SESSION_MESSAGE_END,
                                           sessionData_,
                                           &const_cast<typename SessionMessageType::USER_DATA_T&> (session_message_p->data ()));

          // clean up
          isLinked_ = false;

          if (sessionData_)
          {
            sessionData_->decrease ();
            sessionData_ = NULL;
          } // end IF

          break;
        }
        default:
        {
          if (isLinked_)
            session_message_p->initialize (message_type,
                                           sessionData_,
                                           &const_cast<typename SessionMessageType::USER_DATA_T&> (session_message_p->data ()));

          break;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      if (inherited::mod_)
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: received an unknown message (type was: %d), continuing\n"),
                    inherited::mod_->name (),
                    messageBlock_in->msg_type ()));
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("received an unknown message (type was: %d), continuing\n"),
                    messageBlock_in->msg_type ()));
      break;
    }
  } // end SWITCH

  return 0;
}
