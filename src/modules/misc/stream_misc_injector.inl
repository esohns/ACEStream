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
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
Stream_Module_Injector_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                         UserDataType>::Stream_Module_Injector_T (ISTREAM_T* stream_in)
#else
                         UserDataType>::Stream_Module_Injector_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Injector_T::Stream_Module_Injector_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
Stream_Module_Injector_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Injector_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      inherited::threadCount_ = 1;
      inherited::start (NULL);
      inherited::threadCount_ = 0;

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      if (!inherited::configuration_->queue ||
          !inherited::thr_count_)
        break; // nothing to do

      int result = -1;
      ACE_Message_Block* message_block_p = NULL;

      // enqueue a control message
      ACE_NEW_NORETURN (message_block_p,
                        ACE_Message_Block (0,                                  // size
                                           ACE_Message_Block::MB_STOP,         // type
                                           NULL,                               // continuation
                                           NULL,                               // data
                                           NULL,                               // buffer allocator
                                           NULL,                               // locking strategy
                                           ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                           ACE_Time_Value::zero,               // execution time
                                           ACE_Time_Value::max_time,           // deadline time
                                           NULL,                               // data block allocator
                                           NULL));                             // message allocator
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF

      result = inherited::configuration_->queue->enqueue_tail (message_block_p,
                                                               NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        message_block_p->release (); message_block_p = NULL;
        return;
      } // end IF
      inherited::TASK_BASE_T::wait ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
int
Stream_Module_Injector_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Injector_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     NULL);
#else
  Common_Error_Tools::setThreadName (inherited::threadName_,
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;
  int error = -1;
  bool stop_processing = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->queue);

  do
  {
    result =
      inherited::configuration_->queue->dequeue_head (message_block_p,
                                                      NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Message_Queue_Base::dequeue_head(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF

    ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      // clean up ?
      if (message_block_p)
      {
        message_block_p->release (); message_block_p = NULL;
      } // end IF
      break; // done
    } // end IF

    // inject message into stream
    result = inherited::put_next (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: worker thread %t failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      return -1;
    } // end IF

    message_block_p = NULL;
  } while (true);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}
