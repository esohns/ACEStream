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

#include "ace/Message_Block.h"
#include "ace/Time_Value.h"

#include "stream_common.h"
#include "stream_iallocator.h"
#include "stream_imessagequeue.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_HeadReaderTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::Stream_HeadReaderTask_T (NOTIFY_T* notify_in,
                                                                    Stream_IMessageQueue* messageQueue_in,
                                                                    bool queueIncomingMessages_in)
 : inherited ()
 , enqueue_ (queueIncomingMessages_in)
 , notify_ (notify_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadReaderTask_T::Stream_HeadReaderTask_T"));

  // sanity check(s)
  ACE_ASSERT (notify_);

  if (unlikely (messageQueue_in))
  {
    MESSAGE_QUEUE_T* message_queue_p =
      dynamic_cast<MESSAGE_QUEUE_T*> (messageQueue_in);
    if (unlikely (!message_queue_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<ACE_Message_Queue>(%@) failed, continuing\n"),
                  inherited::mod_->name (),
                  messageQueue_in));
    } // end IF
    inherited::msg_queue (message_queue_p);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
int
Stream_HeadReaderTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadReaderTask_T::put"));

  ACE_UNUSED_ARG (timeValue_in);

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
      break;
    case STREAM_MESSAGE_SESSION:
    {
      // sanity check(s)
      ACE_ASSERT (notify_);

      SessionMessageType* session_message_p =
        static_cast<SessionMessageType*> (messageBlock_in);
      try {
        notify_->notify (session_message_p->type (),
                         false); // recurse upstream ?
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_INotify_T::notify(), aborting\n"),
                    inherited::mod_->name ()));
      }
      break;
    }
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message (type was: %d), aborting\n"),
                  inherited::mod_->name (),
                  messageBlock_in->msg_type ()));
      messageBlock_in->release ();
      return -1;
    }
  } // end SWITCH

  if (inherited::next_)
    return inherited::put_next (messageBlock_in, timeValue_in);
  if (enqueue_)
    return inherited::put (messageBlock_in, timeValue_in);

  messageBlock_in->release ();
  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_HeadWriterTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::Stream_HeadWriterTask_T (NOTIFY_T* notify_in)
 : inherited ()
 , isLinked_ (false)
 , notify_ (notify_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadWriterTask_T::Stream_HeadWriterTask_T"));

  // sanity check(s)
  ACE_ASSERT (notify_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_HeadWriterTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::~Stream_HeadWriterTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadWriterTask_T::~Stream_HeadWriterTask_T"));

  if (unlikely (sessionData_))
    sessionData_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
int
Stream_HeadWriterTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadWriterTask_T::put"));

  ACE_UNUSED_ARG (timeValue_in);

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_SESSION:
    {
      SessionMessageType* session_message_p =
        static_cast<SessionMessageType*> (messageBlock_in);
      switch (session_message_p->type ())
      {
        case STREAM_SESSION_MESSAGE_LINK:
        { ACE_ASSERT (!sessionData_);
          sessionData_ =
            &const_cast<typename SessionMessageType::DATA_T&> (session_message_p->getR ());
          sessionData_->increase ();
          isLinked_ = true;
          break;
        }
        case STREAM_SESSION_MESSAGE_END:
        {
          // *TODO*: merge session data every time ?
          if (isLinked_)
          { ACE_ASSERT (sessionData_);
            session_message_p->initialize (session_message_p->sessionId (),
                                           STREAM_SESSION_MESSAGE_END,
                                           sessionData_,
                                           &const_cast<typename SessionMessageType::USER_DATA_T&> (session_message_p->getR_2 ()));
            sessionData_->decrease (); sessionData_ = NULL;
            isLinked_ = false;
          } // end IF
          break;
        }
        default:
        {
          // *TODO*: merge session data every time ?
          if (isLinked_)
            session_message_p->initialize (session_message_p->sessionId (),
                                           session_message_p->type (),
                                           sessionData_,
                                           &const_cast<typename SessionMessageType::USER_DATA_T&> (session_message_p->getR_2 ()));
          break;
        }
      } // end SWITCH
      break;
    }
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message (type was: %d), aborting\n"),
                  inherited::mod_->name (),
                  messageBlock_in->msg_type ()));
      messageBlock_in->release ();
      return -1;
    }
  } // end SWITCH

  return inherited::put (messageBlock_in, timeValue_in);
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_TailWriterTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::Stream_TailWriterTask_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TailWriterTask_T::Stream_TailWriterTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
int
Stream_TailWriterTask_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                                ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TailWriterTask_T::put"));

  bool reply = false;

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
    case STREAM_MESSAGE_SESSION:
    {
      reply = true;
      break;
    }
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message (type was: %d), aborting\n"),
                  inherited::mod_->name (),
                  messageBlock_in->msg_type ()));
      messageBlock_in->release ();
      return -1;
    }
  } // end SWITCH

  if (unlikely (reply))
    return inherited::reply (messageBlock_in, timeValue_in);

  return inherited::put (messageBlock_in, timeValue_in);
}
