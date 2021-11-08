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
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_HeadTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionEventType>::Stream_HeadTask_T (Stream_IMessageQueue* messageQueue_in)
 : inherited ()
 , isLinked_ (false)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadTask_T::Stream_HeadTask_T"));

  if (messageQueue_in)
  {
    MESSAGE_QUEUE_T* message_queue_p =
      dynamic_cast<MESSAGE_QUEUE_T*> (messageQueue_in);
    if (unlikely (!message_queue_p))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: dynamic_cast<ACE_Message_Queue>(%@) failed, continuing\n"),
                  inherited::mod_->name (),
                  messageQueue_in));
    else
      inherited::msg_queue (message_queue_p);
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_HeadTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
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
          typename SessionEventType>
int
Stream_HeadTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                          ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadTask_T::put"));

  ACE_UNUSED_ARG (timeValue_in);

  int result = 0;
  bool enqueue_message = true;

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
    {
      enqueue_message = false;
      break;
    }
    case STREAM_MESSAGE_SESSION:
    {
      enqueue_message = false;

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
      enqueue_message = false;
      result = -1;
      break;
    }
  } // end SWITCH

  if (likely (enqueue_message))
    return inherited::put (messageBlock_in, timeValue_in);

  messageBlock_in->release ();

  return result;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_TailTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionEventType>::Stream_TailTask_T (Stream_IAllocator* allocator_in)
 : inherited ()
 , allocator_ (allocator_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TailTask_T::Stream_TailTask_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
Stream_TailTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionEventType>::~Stream_TailTask_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_TailTask_T::~Stream_TailTask_T"));

  if (sessionData_)
    sessionData_->decrease ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
int
Stream_TailTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionEventType>::put (ACE_Message_Block* messageBlock_in,
                                          ACE_Time_Value* timeValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TailTask_T::put"));

  ACE_UNUSED_ARG (timeValue_in);

  int result = 0;
  bool enqueue_message = true;

  // sanity check(s)
  ACE_ASSERT (messageBlock_in);

  switch (messageBlock_in->msg_type ())
  {
    case STREAM_MESSAGE_CONTROL:
    {
      enqueue_message = false;
      break;
    }
    case STREAM_MESSAGE_SESSION:
    {
      enqueue_message = false;

      SessionMessageType* session_message_p =
        static_cast<SessionMessageType*> (messageBlock_in);
      switch (session_message_p->type ())
      {
        case STREAM_SESSION_MESSAGE_BEGIN:
        { ACE_ASSERT (!sessionData_);
          sessionData_ =
            &const_cast<typename SessionMessageType::DATA_T&> (session_message_p->getR ());
          sessionData_->increase ();
          break;
        }
        case STREAM_SESSION_MESSAGE_END:
        { ACE_ASSERT (sessionData_);
          const typename SessionMessageType::DATA_T::DATA_T& session_data_r =
            sessionData_->getR ();
          if (unlikely (!reply (STREAM_CONTROL_END,
                                session_data_r)))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to Stream_TailTask_T::reply(%d), aborting\n"),
                        inherited::mod_->name (),
                        STREAM_CONTROL_END));
            result = -1;
            break;
          } // end IF
          sessionData_->decrease (); sessionData_ = NULL;
          break;
        }
        default:
          break;
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
      enqueue_message = false;
      result = -1;
      break;
    }
  } // end SWITCH

  if (likely (enqueue_message))
    return inherited::put (messageBlock_in, timeValue_in);

  messageBlock_in->release ();

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
bool
Stream_TailTask_T<ACE_SYNCH_USE,
                  TimePolicyType,
                  ConfigurationType,
                  ControlMessageType,
                  DataMessageType,
                  SessionMessageType,
                  SessionEventType>::reply (typename ControlMessageType::CONTROL_T controlType_in,
                                            const typename SessionMessageType::DATA_T::DATA_T& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_TailTask_T::reply"));

  // initialize return value(s)
  ACE_Message_Block* message_block_p = NULL;

  // *TODO*: remove type inference
  if (likely (allocator_))
  {
retry:
    try {
      // *TODO*: remove type inference
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<ACE_Message_Block*> (allocator_->calloc ()),
                               ControlMessageType (controlType_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::calloc(), aborting\n"),
                  inherited::mod_->name ()));
      return false;
    }

    // keep retrying ?
    if (unlikely (!message_block_p &&
                  !allocator_->block ()))
      goto retry;
  } // end IF
  else
    ACE_NEW_NORETURN (message_block_p,
                      ControlMessageType (controlType_in));
  if (unlikely (!message_block_p))
  {
    if (likely (allocator_))
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate control message: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate control message: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    return false;
  } // end IF
  int result =
    message_block_p->size (sizeof (typename SessionMessageType::DATA_T::DATA_T));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::size(%d): \"%m\", aborting\n"),
                inherited::mod_->name (),
                sizeof (typename SessionMessageType::DATA_T::DATA_T)));
    message_block_p->release (); message_block_p = NULL;
    return false;
  } // end IF
  result =
    message_block_p->copy (reinterpret_cast<const char*> (&sessionData_in),
                           sizeof (typename SessionMessageType::DATA_T::DATA_T));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                inherited::mod_->name (),
                sizeof (typename SessionMessageType::DATA_T::DATA_T)));
    message_block_p->release (); message_block_p = NULL;
    return false;
  } // end IF

  // forward message
  result = inherited::reply (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::reply(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
    return false;
  } // end IF

  return true;
}
