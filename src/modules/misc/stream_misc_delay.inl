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

#include "ace/Log_Msg.h"

#include "common_time_common.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}

#include "stream_lib_alsa_common.h"
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                      UserDataType>::Stream_Module_Delay_T (ISTREAM_T* stream_in)
#else
                      UserDataType>::Stream_Module_Delay_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in)
 , availableTokens_ (0)
 , condition_ (inherited::lock_)
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
          typename MediaType,
          typename UserDataType>
bool
Stream_Module_Delay_T<ACE_SYNCH_USE,
                      TimePolicyType,
                      ConfigurationType,
                      ControlMessageType,
                      DataMessageType,
                      SessionMessageType,
                      MediaType,
                      UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                 Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Delay_T::initialize"));

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

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->delayConfiguration);

  passMessageDownstream_out = false;

  int result = -1;
  ACE_UINT64 available_tokens_i = 0;
  unsigned int total_length_i = 0;
  ACE_Message_Block* message_block_p = message_inout;
  message_inout = NULL;

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
        message_block_p->release ();
        return;
      } // end IF
    } // end WHILE

    switch (inherited::configuration_->delayConfiguration->mode)
    {
      case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
      {
        total_length_i = message_block_p->total_length ();
        available_tokens_i = std::min (static_cast<ACE_UINT64> (total_length_i),
                                       availableTokens_);
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
        goto continue_;

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
      availableTokens_ = average_bytes_per_second_i;
      inherited::configuration_->delayConfiguration->averageTokensPerInterval =
        average_bytes_per_second_i;
      inherited::configuration_->delayConfiguration->interval =
        ACE_Time_Value (1, 0);
      inherited::configuration_->delayConfiguration->mode =
        STREAM_MISCELLANEOUS_DELAY_MODE_BYTES;

continue_:
      switch (inherited::configuration_->delayConfiguration->mode)
      {
        case STREAM_MISCELLANEOUS_DELAY_MODE_BYTES:
        case STREAM_MISCELLANEOUS_DELAY_MODE_MESSAGES:
        {
          availableTokens_ =
            inherited::configuration_->delayConfiguration->averageTokensPerInterval;
          break;
        }
        case STREAM_MISCELLANEOUS_DELAY_MODE_SCHEDULER:
        {
          inherited::configuration_->delayConfiguration->interval =
            ACE_Time_Value::zero;
          inherited::configuration_->delayConfiguration->interval.msec (static_cast<long> (1000 / inherited::configuration_->delayConfiguration->averageTokensPerInterval));
          inherited::configuration_->delayConfiguration->averageTokensPerInterval = 1;
          availableTokens_ = 1;
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
    availableTokens_ += // --> "catch up"
      inherited::configuration_->delayConfiguration->averageTokensPerInterval;

    result = condition_.broadcast ();
    if (unlikely (result == -1))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
  } // end lock scope
}
