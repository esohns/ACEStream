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

#include <algorithm>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "stream_misc_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::Stream_Module_Delay_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , availableTokens_ (0)
 , condition_ (inherited::lock_)
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::Stream_Module_Delay_T"));

  // set group id for worker thread(s)
  // *TODO*: pass this in from outside
  inherited::grp_id (STREAM_MODULE_TASK_GROUP_ID);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: aborting: flushed %u data messages\n"),
                  inherited::mod_->name (),
                  result));
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
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::handleDataMessage"));

  passMessageDownstream_out = false;

  int result = queue_.enqueue_tail (message_inout, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::enqueue_tail(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_inout->release (); message_inout = NULL;
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);

  Common_ITimerCBBase* itimer_p =
      (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                               : COMMON_TIMERMANAGER_SINGLETON::instance ());
  ACE_ASSERT (itimer_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u data messages\n"),
                    inherited::mod_->name (),
                    result));
      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ACE_UINT64 average_bytes_per_second_i = 0;
      ACE_Time_Value interval =
        inherited::configuration_->delayConfiguration->interval;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      struct tWAVEFORMATEX* waveformatex_p = NULL;
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      struct Stream_MediaFramework_V4L_MediaType media_type_2;
#endif // ACE_WIN32 || ACE_WIN64

      if (inherited::configuration_->delayConfiguration->mode != STREAM_MISCELLANEOUS_DELAY_MODE_INVALID)
        goto continue_;

      // *TODO*: this assumes audio input; move this block to a template specialization
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_INVALID, // N/A
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);
      average_bytes_per_second_i = waveformatex_p->nAvgBytesPerSec;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_INVALID, // N/A
                                media_type_s);
      average_bytes_per_second_i =
        media_type_s.rate                                *
        (snd_pcm_format_width (media_type_s.format) / 8) *
        media_type_s.channels;
#endif // ACE_WIN32 || ACE_WIN64
      availableTokens_ =
        static_cast<ACE_UINT64> (static_cast<float> (average_bytes_per_second_i) * static_cast<float> (STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US) / 1000000.0F);
      inherited::configuration_->delayConfiguration->averageTokensPerInterval =
        availableTokens_;
      interval = ACE_Time_Value (0, STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US);
      inherited::configuration_->delayConfiguration->mode =
        STREAM_MISCELLANEOUS_DELAY_MODE_BYTES;

continue_:
      switch (inherited::configuration_->delayConfiguration->mode)
      {
        case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
        {
          // ACE_FALLTHROUGH;
        }
        case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
        {
          // ACE_FALLTHROUGH;
        }
        case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
        case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES:
        { ACE_ASSERT (inherited::configuration_->delayConfiguration->averageTokensPerInterval);
          availableTokens_ =
            inherited::configuration_->delayConfiguration->averageTokensPerInterval;

          if (unlikely (interval == ACE_Time_Value::zero))
          {
            float frame_rate_f;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
            ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
            inherited2::getMediaType (session_data_r.formats.back (),
                                      STREAM_MEDIATYPE_VIDEO,
                                      media_type_s);
            frame_rate_f =
              static_cast<float> (Stream_MediaFramework_DirectShow_Tools::toFramerate (media_type_s));
            Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
            inherited2::getMediaType (session_data_r.formats.back (),
                                      STREAM_MEDIATYPE_VIDEO,
                                      media_type_2);
            frame_rate_f =
              media_type_2.frameRate.numerator / static_cast<float> (media_type_2.frameRate.denominator);
#endif // ACE_WIN32 || ACE_WIN64
            ACE_ASSERT (frame_rate_f > 0.0f);

            ACE_DEBUG ((LM_WARNING,
                        ACE_TEXT ("%s: deducing scheduler interval from (video) frame rate (was: %.2f)...\n"),
                        inherited::mod_->name (),
                        frame_rate_f));

            interval =
              ACE_Time_Value (0, static_cast<suseconds_t> ((1.0f / frame_rate_f) * 1000000.0f));
          } // end IF

          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->delayConfiguration->mode));
          goto error;
        }
      } // end SWITCH

      // schedule the delay interval timer
      resetTimeoutHandlerId_ =
        itimer_p->schedule_timer (&resetTimeoutHandler_, // event handler handle
                                  NULL,                  // asynchronous completion token
                                  interval,              // delay
                                  interval);             // interval
      if (unlikely (resetTimeoutHandlerId_ == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimerCBBase::schedule_timer(%#T): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    &interval));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: scheduled interval timer (%#T, id: %d)\n"),
                  inherited::mod_->name (),
                  &interval,
                  resetTimeoutHandlerId_));

      // *NOTE*: this prevents a race condition in svc()
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        inherited::threadCount_ = 1;
        bool lock_activate_was_b = inherited::TASK_BASE_T::TASK_BASE_T::lockActivate_;
        inherited::lockActivate_ = false;
        if (unlikely (!inherited::start (NULL)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Task_Base_T::start(), aborting\n"),
                      inherited::mod_->name ()));
          inherited::lockActivate_ = lock_activate_was_b;
          inherited::threadCount_ = 0;
          goto error;
        } // end IF
        inherited::lockActivate_ = lock_activate_was_b;
        inherited::threadCount_ = 0;
        ACE_ASSERT (!inherited::threadIds_.empty ());
      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      // *NOTE*: this is done in svc() to allow ordered shutdown
      //if (likely (resetTimeoutHandlerId_ != -1))
      //{
      //  const void* act_p = NULL;
      //  int result = itimer_p->cancel_timer (resetTimeoutHandlerId_,
      //                                       &act_p);
      //  if (unlikely (result <= 0))
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("%s: failed to Common_ITimerCBBase::cancel_timer(%d): \"%m\", continuing\n"),
      //                inherited::mod_->name (),
      //                resetTimeoutHandlerId_));
      //  else
      //    ACE_DEBUG ((LM_DEBUG,
      //                ACE_TEXT ("%s: cancelled interval timer (id was: %d)\n"),
      //                inherited::mod_->name (),
      //                resetTimeoutHandlerId_));
      //  resetTimeoutHandlerId_ = -1;
      //} // end IF

      stop (true,   // wait ?
            false); // high priority ?

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
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::reset"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);

  int result = -1;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    availableTokens_ = // --> do NOT "catch up"
      inherited::configuration_->delayConfiguration->averageTokensPerInterval;

    result = condition_.broadcast ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
  } // end lock scope
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::stop (bool waitForCompletion_in,
                                           bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::stop"));

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

  int result = (highPriority_in ? queue_.enqueue_head (message_block_p, NULL) :
                                  queue_.enqueue_tail (message_block_p, NULL));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue_T::%s(): \"%m\", continuing\n"),
                inherited::mod_->name (),
                (highPriority_in ? ACE_TEXT ("enqueue_head") : ACE_TEXT ("enqueue_tail"))));
    message_block_p->release ();
    message_block_p = NULL;
  } // end IF  
  message_block_p = NULL;

  if (waitForCompletion_in)
  {
    Common_ITask* itask_p = this;
    itask_p->wait (true); // wait for message queue(s) ?
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
int
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::svc"));

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
  //static ACE_Time_Value no_wait = ACE_OS::gettimeofday ();

  do
  {
    result = queue_.dequeue_head (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      error = ACE_OS::last_error ();
      //if (error == ETIME)
      //  goto continue_;

      if (unlikely (error != ESHUTDOWN))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: worker thread %t failed to ACE_Message_Queue_T::dequeue_head(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        return -1;
      } // end IF
      result = 0; // OK, queue has been deactivate()d
      break;
    } // end IF

    ACE_ASSERT (message_block_p);
    if (unlikely (message_block_p->msg_type () == ACE_Message_Block::MB_STOP))
    {
      if (unlikely (inherited::thr_count_ > 1))
      {
        result = queue_.enqueue_tail (message_block_p, NULL);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue_T::enqueue_tail(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          message_block_p->release ();
          return -1;
        } // end IF
        message_block_p = NULL;
      } // end IF
      // clean up ?
      if (message_block_p)
      {
        message_block_p->release (); message_block_p = NULL;
      } // end IF
      break; // done
    } // end IF

    dispatch (message_block_p);
    message_block_p = NULL;
  } while (true);

  if (likely (resetTimeoutHandlerId_ != -1))
  {
    Common_ITimerCBBase* itimer_p =
        (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                 : COMMON_TIMERMANAGER_SINGLETON::instance ());
    ACE_ASSERT (itimer_p);

    const void* act_p = NULL;
    result = itimer_p->cancel_timer (resetTimeoutHandlerId_,
                                     &act_p);
    if (unlikely (result <= 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_ITimerCBBase::cancel_timer(%d): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  resetTimeoutHandlerId_));
    else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: cancelled interval timer (id was: %d)\n"),
                  inherited::mod_->name (),
                  resetTimeoutHandlerId_));
    resetTimeoutHandlerId_ = -1;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::dispatch (ACE_Message_Block* messageBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::dispatch"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);
  ACE_ASSERT (messageBlock_in);

  int result = -1;
  ACE_UINT64 available_tokens_i = 0;
  size_t total_length_i = 0;

continue_:
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    while (!availableTokens_)
    {
      result = condition_.wait (NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::wait(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        messageBlock_in->release ();
        return;
      } // end IF
    } // end WHILE

    switch (inherited::configuration_->delayConfiguration->mode)
    {
      case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
      case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES:
      {
        total_length_i = messageBlock_in->total_length ();
        available_tokens_i = std::min (total_length_i, availableTokens_);
        availableTokens_ -= available_tokens_i;
        break;
      }
      case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
      case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
      {
        available_tokens_i = 1;
        --availableTokens_;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), returning\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->delayConfiguration->mode));
        messageBlock_in->release ();
        return;
      }
    } // end SWITCH
  } // end lock scope

  switch (inherited::configuration_->delayConfiguration->mode)
  {
    case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
    case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES:
    {
      ACE_Message_Block* message_block_2 = messageBlock_in;
      if (available_tokens_i < total_length_i)
      {
        message_block_2 = Stream_Tools::get (available_tokens_i,
                                             messageBlock_in,
                                             messageBlock_in);
        if (unlikely (!message_block_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Tools::get(%u), returning\n"),
                      inherited::mod_->name (),
                      available_tokens_i));
          messageBlock_in->release ();
          return;
        } // end IF
      } // end IF

      result = inherited::put_next (message_block_2, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        messageBlock_in->release ();
        message_block_2->release ();
        return;
      } // end IF

      if (available_tokens_i < total_length_i)
        goto continue_;

      break;
    }
    case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
    case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
    {
      result = inherited::put_next (messageBlock_in, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        messageBlock_in->release ();
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->delayConfiguration->mode));
      messageBlock_in->release ();
      return;
    }
  } // end SWITCH
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
Stream_Module_Delay_2<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::Stream_Module_Delay_2 (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , availableTokens_ (0)
 , condition_ (inherited::lock_)
 , resetTimeoutHandler_ (this)
 , resetTimeoutHandlerId_ (-1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_2::Stream_Module_Delay_2"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
bool
Stream_Module_Delay_2<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                 Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_2::initialize"));

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_2<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_2::handleDataMessage"));

     // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);

  passMessageDownstream_out = false;

  int result = -1;
  ACE_UINT64 available_tokens_i = 0;
  size_t total_length_i = 0;
  ACE_Message_Block* message_block_p = message_inout;
  message_inout = NULL;

//continue_:
{ ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
  while (!availableTokens_)
  {
    result = condition_.wait (NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::wait(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      message_block_p->release ();
      return;
    } // end IF
  } // end WHILE

  switch (inherited::configuration_->delayConfiguration->mode)
  {
    case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
    {
      total_length_i = message_block_p->total_length ();
      available_tokens_i = std::min (total_length_i, availableTokens_);
      availableTokens_ -= available_tokens_i;
      break;
    }
    case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
    case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
    {
      available_tokens_i = 1;
      --availableTokens_;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->delayConfiguration->mode));
      message_block_p->release ();
      return;
    }
  } // end SWITCH
} // end lock scope

  switch (inherited::configuration_->delayConfiguration->mode)
  {
    case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
    {
      ACE_Message_Block* message_block_2 = message_block_p;
      if (available_tokens_i < total_length_i)
      {
        message_block_2 = Stream_Tools::get (available_tokens_i,
                                             message_block_p,
                                             message_block_p);
        if (!message_block_2)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Tools::get(%u), returning\n"),
                      inherited::mod_->name (),
                      available_tokens_i));
          message_block_p->release ();
          return;
        } // end IF
      } // end IF

      result = inherited::put_next (message_block_2, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        message_block_2->release ();
        message_block_p->release ();
        return;
      } // end IF

      if (available_tokens_i < total_length_i)
      {
        result = inherited::ungetq (message_block_p, NULL);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Task::ungetq(): \"%m\", returning\n"),
                      inherited::mod_->name ()));
          message_block_p->release ();
          return;
        } // end IF
      } // end IF

      break;
    }
    case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
    case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
    {
      result = inherited::put_next (message_block_p, NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        message_block_p->release ();
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), returning\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->delayConfiguration->mode));
      message_block_p->release ();
      return;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_2<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_2::handleSessionMessage"));

     // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

     // sanity check(s)
  ACE_ASSERT (message_inout);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);

  Common_ITimerCBBase* itimer_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                               : COMMON_TIMERMANAGER_SINGLETON::instance ());
  ACE_ASSERT (itimer_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ACE_UINT64 average_bytes_per_second_i = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      struct tWAVEFORMATEX* waveformatex_p = NULL;
#else
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64

      if (inherited::configuration_->delayConfiguration->mode != STREAM_MISCELLANEOUS_DELAY_MODE_INVALID)
        goto continue_;

   // *TODO*: move this to a template specialization
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_INVALID, // N/A
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_WaveFormatEx));
      waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (waveformatex_p);
      average_bytes_per_second_i = waveformatex_p->nAvgBytesPerSec;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#else
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_INVALID, // N/A
                                media_type_s);
      average_bytes_per_second_i = media_type_s.rate                                *
                                   (snd_pcm_format_width (media_type_s.format) / 8) *
                                   media_type_s.channels;
#endif // ACE_WIN32 || ACE_WIN64
      availableTokens_ =
        static_cast<ACE_UINT64> (static_cast<float> (average_bytes_per_second_i) * static_cast<float> (STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US) / 1000000.0F);
      inherited::configuration_->delayConfiguration->averageTokensPerInterval =
        availableTokens_;
      inherited::configuration_->delayConfiguration->interval =
        ACE_Time_Value (0, STREAM_MISC_DEFAULT_DELAY_AUDIO_INTERVAL_US);
      inherited::configuration_->delayConfiguration->mode =
        STREAM_MISCELLANEOUS_DELAY_MODE_BYTES;

continue_:
      switch (inherited::configuration_->delayConfiguration->mode)
      {
        case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
        {
          // ACE_FALLTHROUGH;
        }
        case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
        {
          // ACE_FALLTHROUGH;
        }
        case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
        case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER_BYTES:
        { ACE_ASSERT (inherited::configuration_->delayConfiguration->averageTokensPerInterval);
          availableTokens_ =
            inherited::configuration_->delayConfiguration->averageTokensPerInterval;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown delay mode (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->delayConfiguration->mode));
          goto error;
        }
      } // end SWITCH

         // schedule the delay interval timer
      resetTimeoutHandlerId_ =
        itimer_p->schedule_timer (&resetTimeoutHandler_,                                    // event handler handle
                                  NULL,                                                     // asynchronous completion token
                                  inherited::configuration_->delayConfiguration->interval,  // delay
                                  inherited::configuration_->delayConfiguration->interval); // interval
      if (unlikely (resetTimeoutHandlerId_ == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimerCBBase::schedule_timer(%#T): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    &inherited::configuration_->delayConfiguration->interval));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: scheduled interval timer (%#T, id: %d)\n"),
                  inherited::mod_->name (),
                  &inherited::configuration_->delayConfiguration->interval,
                  resetTimeoutHandlerId_));

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (likely (resetTimeoutHandlerId_ != -1))
      {
        const void* act_p = NULL;
        int result = itimer_p->cancel_timer (resetTimeoutHandlerId_,
                                             &act_p);
        if (unlikely (result <= 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimerCBBase::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      resetTimeoutHandlerId_));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: cancelled interval timer (id was: %d)\n"),
                      inherited::mod_->name (),
                      resetTimeoutHandlerId_));
        resetTimeoutHandlerId_ = -1;
      } // end IF

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
          typename MediaType,
          typename UserDataType>
void
Stream_Module_Delay_2<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_2::reset"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);

  int result = -1;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
    availableTokens_ += // --> "catch up"
      inherited::configuration_->delayConfiguration->averageTokensPerInterval;

    result = condition_.broadcast ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
  } // end lock scope
}
