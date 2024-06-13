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
#else
#if defined (FFMPEG_SUPPORT)
#ifdef __cplusplus
extern "C"
{
#include "libavutil/imgutils.h"
}
#endif // __cplusplus
#endif // FFMPEG_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "common_ui_gtk_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
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
                              MediaType>::Stream_Module_Window_Source_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , handler_ (this,
             false)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , captureContext_ (NULL)
 , captureBitmap_ (NULL)
 , sourceContext_ (NULL)
 , resolution_ ()
#else
#endif // ACE_WIN32 || ACE_WIN64
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
                              MediaType>::~Stream_Module_Window_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Window_Source_T::~Stream_Module_Window_Source_T"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (captureContext_)
    DeleteDC (captureContext_);
  if (captureBitmap_)
    DeleteObject (captureBitmap_);
  if (inherited::configuration_ && sourceContext_)
    ReleaseDC (inherited::configuration_->window, sourceContext_);
#else
  if (window_)
    g_object_unref (window_);
#endif // ACE_WIN32 || ACE_WIN64
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
  { ACE_ASSERT (inherited::configuration_);
    long timer_id = handler_.get_2 ();
    if (unlikely (timer_id != -1))
    {
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

#if defined (ACE_WIN32) || defined (ACE_WIN64)
    DeleteDC (captureContext_); captureContext_ = NULL;
    DeleteObject (captureBitmap_); captureBitmap_ = NULL;
    ReleaseDC (inherited::configuration_->window, sourceContext_); sourceContext_ = NULL;
#else
    g_object_unref (window_); window_ = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF

  if (unlikely (!configuration_in.window))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no window provided, aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  window_ = Common_UI_GTK_Tools::get (configuration_in.window);
  if (!window_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_UI_GTK_Tools::get (%d), aborting\n"),
                inherited::mod_->name (),
                configuration_in.window));
    return false;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      struct Stream_MediaFramework_FFMPEG_VideoMediaType media_type_s;
#endif // ACE_WIN32 || ACE_WIN64

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
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(%#T): \"%m\", aborting\n"),
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
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
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

      ACE_ASSERT (inherited::configuration_->window);
      sourceContext_ = GetDC (inherited::configuration_->window);
      ACE_ASSERT (sourceContext_);
      captureContext_ = CreateCompatibleDC (sourceContext_);
      ACE_ASSERT (captureContext_);
      captureBitmap_ = CreateCompatibleBitmap (sourceContext_,
                                               video_info_header_p->bmiHeader.biWidth,
                                               video_info_header_p->bmiHeader.biHeight);
      ACE_ASSERT (captureBitmap_);
      if (unlikely (!SelectObject (captureContext_, captureBitmap_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SelectObject(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
        goto error;
      } // end IF
      resolution_.cx = video_info_header_p->bmiHeader.biWidth;
      resolution_.cy = video_info_header_p->bmiHeader.biHeight;
      ACE_OS::memset (&bitmapInfo_, 0, sizeof (BITMAPINFO));
      bitmapInfo_.bmiHeader = video_info_header_p->bmiHeader;
#else
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      frames_per_second_i = media_type_s.frameRate.num;

#if defined (FFMPEG_SUPPORT)
      frameSize_ = av_image_get_buffer_size (media_type_s.format,
                                             media_type_s.resolution.width,
                                             media_type_s.resolution.height,
                                             1); // *TODO*: linesize alignment
#else
      ACE_ASSERT (false); // *TODO*
#endif // FFMPEG_SUPPORT
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration->defaultBufferSize >= frameSize_);
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
  if (unlikely (!inherited::sessionData_))
    return;

  ACE_Message_Block* message_block_p = NULL;
  DataMessageType* message_p = NULL;
  const SessionDataType& session_data_r = inherited::sessionData_->getR ();
  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  GdkPixbuf* pixel_buffer_p = NULL;
  guchar* data_p = NULL;
  guint length_i = 0;
#endif // ACE_WIN32 || ACE_WIN64

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
  message_p = static_cast<DataMessageType*> (message_block_p);
  message_p->initialize (session_data_r.sessionId,
                         NULL);

  // step2: fill buffer
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!BitBlt (captureContext_, 0, 0, resolution_.cx, resolution_.cy, sourceContext_, 0, 0, SRCCOPY | CAPTUREBLT))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to BitBlt(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
    goto error;
  } // end IF
  result =
    GetDIBits (captureContext_, captureBitmap_, 0, resolution_.cy, message_block_p->wr_ptr (), &bitmapInfo_, DIB_RGB_COLORS);
  if (result != resolution_.cy)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to GetDIBits(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
    goto error;
  } // end IF
  message_block_p->wr_ptr (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
#else
  pixel_buffer_p = Common_UI_GTK_Tools::get (window_);
  if (unlikely (!pixel_buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_UI_GTK_Tools::get(%@), aborting\n"),
                inherited::mod_->name (),
                window_));
    goto error;
  } // end IF
  data_p =
    gdk_pixbuf_get_pixels_with_length (pixel_buffer_p,
                                       &length_i);
  ACE_ASSERT (data_p && length_i == frameSize_);
  result = message_block_p->copy (reinterpret_cast<char*> (data_p),
                                  frameSize_);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%u): \"%m\", aborting\n"),
                inherited::mod_->name (),
                frameSize_));
    g_object_unref (pixel_buffer_p); pixel_buffer_p = NULL;
    goto error;
  } // end IF
  g_object_unref (pixel_buffer_p); pixel_buffer_p = NULL;
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
