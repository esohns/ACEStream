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

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      UserDataType>::Stream_Module_Delay_T (ISTREAM_T* stream_in)
#else
                      UserDataType>::Stream_Module_Delay_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::Stream_Module_Delay_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  if (unlikely (resetTimeoutHandlerId_ == -1))
    return; // no delay

  int result = inherited::msg_queue_->enqueue_tail (message_inout);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_inout->release ();
  } // end IF

  passMessageDownstream_out = false;
  message_inout = NULL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::configuration_);

  Common_ITimerBase* itimer_p =
      (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                               : COMMON_TIMERMANAGER_SINGLETON::instance ());
  ACE_ASSERT (itimer_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      inherited::msg_queue_->activate ();

      // schedule the second-granularity timer
      ACE_Time_Value one_second (1, 0); // one-second interval
      resetTimeoutHandlerId_ =
        itimer_p->schedule_timer (&resetTimeoutHandler_,                              // event handler handle
                                  NULL,                                               // asynchronous completion token
                                  COMMON_TIME_NOW + inherited::configuration_->delay, // first wakeup time
                                  inherited::configuration_->delay);                  // interval
      if (unlikely (resetTimeoutHandlerId_ == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimerBase::schedule_timer(%#T): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    &inherited::configuration_->delay));
        goto error;
      } // end IF
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: scheduled interval timer (%#T, id: %d)\n"),
                  inherited::mod_->name (),
                  &inherited::configuration_->delay,
                  resetTimeoutHandlerId_));
#endif // _DEBUG

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (resetTimeoutHandlerId_ != -1)
      {
        const void* act_p = NULL;
        int result = itimer_p->cancel_timer (resetTimeoutHandlerId_,
                                             &act_p);
        if (unlikely (result <= 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimerBase::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      resetTimeoutHandlerId_));
        resetTimeoutHandlerId_ = -1;
      } // end IF

      inherited::msg_queue_->deactivate ();

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
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      UserDataType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::reset"));

  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;

  int result = inherited::msg_queue_->dequeue_head (message_block_p,
                                                    &no_wait);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != EWOULDBLOCK)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::dequeue_head(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
    return;
  } // end IF
  ACE_ASSERT (message_block_p);

  result = inherited::put_next (message_block_p,
                                NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
    return;
  } // end IF
}
