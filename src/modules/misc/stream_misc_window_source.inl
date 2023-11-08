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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "amvideo.h"
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include "uuids.h"
#endif // UUIDS_H
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#include "common_timer_manager_common.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename MediaType>
Stream_Module_Window_Source_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType,
                              TimerManagerType,
 #if defined (ACE_WIN32) || defined (ACE_WIN64)
                              MediaType>::Stream_Module_Window_Source_T (ISTREAM_T* stream_in)
#else
                              MediaType>::Stream_Module_Window_Source_T (typename inherited::ISTREAM_T* stream_in)
#endif // ACE_WIN32 || ACE_WIN64
 : inherited (stream_in) // stream handle
 , handler_ (this,
             false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Window_Source_T::Stream_Module_Window_Source_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename MediaType>
bool
Stream_Module_Window_Source_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType,
                              TimerManagerType,
                              MediaType>::initialize (const ConfigurationType& configuration_in,
                                                      Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Window_Source_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    long timer_id = handler_.get_2 ();
    if (unlikely (timer_id != -1))
    {
      ACE_ASSERT (inherited::configuration_);
      typename TimerManagerType::INTERFACE_T* itimer_manager_p =
        (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                 : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
      ACE_ASSERT (itimer_manager_p);

      const void* act_p = NULL;
      int result = itimer_manager_p->cancel_timer (timer_id,
                                                   &act_p);
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    timer_id));
      handler_.set (-1);
    } // end IF
  } // end IF

  if (unlikely (!configuration_in.window))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no window provided, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename MediaType>
void
Stream_Module_Window_Source_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType,
                              TimerManagerType,
                              MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Window_Source_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      inherited::change (STREAM_STATE_SESSION_STOPPING);
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    { ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

      ACE_Time_Value interval;
      long timer_id = -1;
      unsigned int frames_per_second_i = 0;
      suseconds_t frame_time_us = 0;

      // schedule regular statistic collection ?
      // *NOTE*: the runtime-statistic module is responsible for regular
      //         reporting, the head module merely collects information
      // *TODO*: remove type inference
      if (inherited::configuration_->statisticReportingInterval != ACE_Time_Value::zero)
      { ACE_ASSERT (inherited::timerId_ == -1);
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        ACE_Time_Value interval (0, STREAM_DEFAULT_STATISTIC_COLLECTION_INTERVAL_MS * 1000);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_), // event handler
                                            NULL,                            // asynchronous completion token
                                            COMMON_TIME_NOW + interval,      // first wakeup time
                                            interval);                       // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", returning\n"),
                      inherited::mod_->name (),
                      &interval));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    inherited::timerId_,
                    &interval));
      } // end IF

      // get media type / frame size
      ACE_ASSERT (!session_data_r.formats.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      ACE_ASSERT (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo));
      ACE_ASSERT (media_type_s.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
      struct tagVIDEOINFOHEADER* video_info_header_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_s.pbFormat);
      ACE_ASSERT (video_info_header_p->AvgTimePerFrame);
      frames_per_second_i = 10000000 / static_cast<int> (video_info_header_p->AvgTimePerFrame);

      ACE_ASSERT (inherited::configuration_->allocatorConfiguration->defaultBufferSize >= video_info_header_p->bmiHeader.biSizeImage);
#else
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      frames_per_second_i = media_type_s.framesPerSecond;

      ACE_ASSERT (inherited::configuration_->allocatorConfiguration->defaultBufferSize >= media_type_s.sizeImage);
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (frames_per_second_i);

      // determine interval size from average frames per second
      frame_time_us = static_cast<suseconds_t> (1000000 / static_cast<double> (frames_per_second_i));

      // start sample generator timer
      interval.set (0, frame_time_us);
      timer_id =
        itimer_manager_p->schedule_timer (&handler_,       // event handler handle
                                          NULL,            // asynchronous completion token
                                          COMMON_TIME_NOW, // first wakeup time
                                          interval);       // interval
      if (unlikely (timer_id == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      handler_.set (timer_id);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: scheduled generator timer (id: %d) for interval %#T\n"),
                  inherited::mod_->name (),
                  timer_id,
                  &interval));

      break;

error:
      inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message (see above: 2566)
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (unlikely (inherited::sessionEndProcessed_))
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      long timer_id = handler_.get_2 ();
      if (likely (timer_id != -1))
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (timer_id,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      timer_id));
        handler_.set (-1);
      } // end IF

      if (inherited::timerId_ != -1)
      {
        typename TimerManagerType::INTERFACE_T* itimer_manager_p =
            (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                                     : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
        ACE_ASSERT (itimer_manager_p);
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer() (id was: %d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
//        if (unlikely (act_p))
//        {
//          delete act_p; act_p = NULL;
//        } // end IF
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      { Common_ITask* itask_p = this; // *TODO*: is the no other way ?
        itask_p->stop (false,  // wait for completion ?
                       false); // high priority ?
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename MediaType>
void
Stream_Module_Window_Source_T<ACE_SYNCH_USE,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              ConfigurationType,
                              StreamControlType,
                              StreamNotificationType,
                              StreamStateType,
                              SessionDataType,
                              SessionDataContainerType,
                              StatisticContainerType,
                              TimerManagerType,
                              MediaType>::handle (const void* act_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Window_Source_T::handle"));

  ACE_UNUSED_ARG (act_in);

  // sanity check(s)
  ACE_ASSERT (inherited::allocator_);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  ACE_Message_Block* message_block_p = NULL;
  int result = -1;

  // step1: allocate buffer
  try {
    message_block_p =
      static_cast<ACE_Message_Block*> (inherited::allocator_->malloc (inherited::configuration_->allocatorConfiguration->defaultBufferSize));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    message_block_p = NULL;
  }
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  // step2: fill buffer
#if defined (ACE_WIN32) || defined (ACE_WIN64)

#else
  message_block_p->wr_ptr (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
#endif // ACE_WIN32 || ACE_WIN64

  // step3: push data downstream
  result = this->put (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::put(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  return;

error:
  if (message_block_p)
    message_block_p->release ();

  this->notify (STREAM_SESSION_MESSAGE_ABORT);
}
