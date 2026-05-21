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

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "common_image_common.h"

#include "stream_defines.h"
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::Stream_Dev_Cam_Source_GStreamer_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , inherited3 ()
 , busWatchId_ (0)
 , CBData_ ()
 , isFirst_ (true)
 , notifyAbort_ (true)
 , pipeline_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::Stream_Dev_Cam_Source_GStreamer_T"));

}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::~Stream_Dev_Cam_Source_GStreamer_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::~Stream_Dev_Cam_Source_GStreamer_T"));

  if (busWatchId_)
  {
    g_source_remove (busWatchId_);
  } // end IF
  if (CBData_.loop)
  {
    g_main_loop_unref (CBData_.loop);
  } // end IF
  if (pipeline_)
  {
    gst_object_unref (GST_OBJECT (pipeline_));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::initialize (const ConfigurationType& configuration_in,
                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::initialize"));

  bool result;

  if (inherited::isInitialized_)
  {
    if (busWatchId_)
    {
      g_source_remove (busWatchId_); busWatchId_ = 0;
    } // end IF
    if (CBData_.loop)
    {
      g_main_loop_unref (CBData_.loop); CBData_.loop = NULL;
    } // end IF
    isFirst_ = true;
    notifyAbort_ = true;
    if (pipeline_)
    {
      gst_object_unref (GST_OBJECT (pipeline_)); pipeline_ = NULL;
    } // end IF
  } // end IF

  result = inherited::initialize (configuration_in,
                                  allocator_in);
  if (unlikely (!result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::initialize(), aborting\n"),
                inherited::mod_->name ()));

  return result;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::handleSessionMessage"));

  int result;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      inherited::isHighPriorityStop_ = true;
      inherited::change (STREAM_STATE_SESSION_STOPPING);

      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
        const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
#else
      ACE_ASSERT (false); // *TODO*: implement for other platforms
#endif // ACE_WIN32 || ACE_WIN64
      MediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));
      bool is_active = false;

      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                            NULL,                                                                     // asynchronous completion token
                                            COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                            inherited::configuration_->statisticCollectionInterval);                  // interval
        if (inherited::timerId_ == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    inherited::timerId_,
                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      // *TODO*: remove type inferences
      if (!initialize_GStreamer (inherited::configuration_->deviceIdentifier,
                                 inherited::configuration_->window))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_GStreamer(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (pipeline_);

      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);
      // *TODO*: set capture format based on session data
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      inherited2::setFormat (MEDIASUBTYPE_RGB24,
                             media_type_s);
#else
      ACE_ASSERT (false); // *TODO*: implement for other platforms
#endif // ACE_WIN32 || ACE_WIN64
      static Common_Image_Resolution_t resolution_s = {640, 480};
      inherited2::setResolution (resolution_s,
                                 media_type_s);
      inherited2::set (media_type_s,
                       STREAM_MEDIATYPE_VIDEO,
                       media_type_2);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      //Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64
      session_data_r.formats.push_back (media_type_2);

      gst_element_set_state (GST_ELEMENT (pipeline_), GST_STATE_PLAYING);

      // start GStreamer processing thread
      inherited::TASK_BASE_T::threadCount_ = 1;
      inherited::TASK_BASE_T::start (NULL);
      is_active = inherited::TASK_BASE_T::isRunning ();
      ACE_ASSERT (is_active);
      inherited::TASK_BASE_T::threadCount_ = 0;

      break;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      //Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
#endif // ACE_WIN32 || ACE_WIN64

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      // *NOTE*: only process the first 'session end' message
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (unlikely (inherited::sessionEndProcessed_))
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope
      notifyAbort_ = false;

      if (pipeline_)
        gst_element_set_state (GST_ELEMENT (pipeline_), GST_STATE_NULL);

      if (CBData_.loop)
        g_main_loop_quit (CBData_.loop);

      if (inherited::timerId_ != -1)
      {
        const void* act_p = NULL;
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      if (likely (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT))
      {
        Common_ITask* itask_p = this; // *TODO*: is there no other way ?
        itask_p->stop (false,                           // wait for completion ?
                       inherited::isHighPriorityStop_); // high priority ?
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data

  //// step2: send the information downstream
  //if (!inherited::putStatisticMessage (data_out)) // data container
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n")));
  //  return false;
  //} // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::initialize_GStreamer (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                                    const struct Common_UI_Window& window_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::initialize_GStreamer"));

  // sanity check(s)
  ACE_ASSERT (!busWatchId_);
  ACE_ASSERT (!CBData_.loop);
  ACE_ASSERT (!pipeline_);

  // make a loop
  CBData_.loop = g_main_loop_new (NULL, FALSE);
  ACE_ASSERT (CBData_.loop);

  // make a pipeline
  pipeline_ =
    GST_PIPELINE (gst_pipeline_new (ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_GSTREAMER_PIPELINE_ID_STRING)));
  ACE_ASSERT (pipeline_);

  // add a message handler
  GstBus* bus_p = gst_pipeline_get_bus (pipeline_);
  ACE_ASSERT (bus_p);
  busWatchId_ = gst_bus_add_watch (bus_p, acestream_dev_gstreamer_bus_cb, &CBData_);
  gst_object_unref (bus_p); bus_p = NULL;

  // set up pipeline elements
  GstElement* source = NULL, *convert = NULL, *sink = NULL;
  source = gst_element_factory_make (ACE_TEXT_ALWAYS_CHAR ("videotestsrc"), ACE_TEXT_ALWAYS_CHAR ("source"));
  convert = gst_element_factory_make (ACE_TEXT_ALWAYS_CHAR ("videoconvert"),   ACE_TEXT_ALWAYS_CHAR ("convert"));
  sink = gst_element_factory_make (ACE_TEXT_ALWAYS_CHAR ("appsink"),  ACE_TEXT_ALWAYS_CHAR ("appsink"));
  if (unlikely (!source || !convert || !sink))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to create filter element, aborting\n"),
                inherited::mod_->name ()));
    g_main_loop_unref (CBData_.loop); CBData_.loop = NULL;
    gst_object_unref (GST_OBJECT (pipeline_)); pipeline_ = NULL;
    return false;
  } // end IF
  // *TODO*: currently, this fails on Win32 (due to conflicting glib-2.0.lib|dll versions ?)
  // *NOTE*: when linking against the gstreamer-provided glib2, GTK misbehaves
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: instantiated appsink element of type \"%s\"\n"),
              inherited::mod_->name (),
              G_OBJECT_TYPE_NAME (sink)));

  // apply some properties to the sink
  // *TODO*: pass in the media type
  GstCaps* caps_p =
    gst_caps_from_string (ACE_TEXT_ALWAYS_CHAR ("video/x-raw,format=BGR,width=640,height=480"));
  ACE_ASSERT (caps_p);
  g_object_set (G_OBJECT (sink),
                ACE_TEXT_ALWAYS_CHAR ("caps"), caps_p,
                ACE_TEXT_ALWAYS_CHAR ("drop"), TRUE,
                ACE_TEXT_ALWAYS_CHAR ("emit-signals"), TRUE,
                ACE_TEXT_ALWAYS_CHAR ("sync"), FALSE,
                NULL);
  gst_caps_unref (caps_p); caps_p = NULL;

  // set up frame-grabbing callback
  g_signal_connect (G_OBJECT (sink),
                    ACE_TEXT_ALWAYS_CHAR ("new-sample"),
                    G_CALLBACK (acestream_dev_gstreamer_new_sample_cb),
                    &CBData_);

  gst_bin_add_many (GST_BIN (pipeline_), source, convert, sink, NULL);
  gst_element_link_many (source, convert, sink, NULL);

  return true;
}

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename UserDataType,
          typename MediaType>
int
Stream_Dev_Cam_Source_GStreamer_T<ACE_SYNCH_USE,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType,
                                  ConfigurationType,
                                  StreamControlType,
                                  StreamNotificationType,
                                  StreamStateType,
                                  StatisticContainerType,
                                  SessionManagerType,
                                  TimerManagerType,
                                  UserDataType,
                                  MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_GStreamer_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  if (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
    Common_Error_Tools::setThreadName (inherited::threadName_,
                                       NULL);
#else
    Common_Error_Tools::setThreadName (inherited::threadName_,
                                       0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT ("")),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? inherited::grp_id_
                                                                                             : -1)));

  int result = -1;

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, -1);
    if (!isFirst_)
      goto continue_;
    isFirst_ = false;
  } // end lock scope

  // *NOTE*: use the calling threads' context (start() blocks)
  result = inherited::svc ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::svc(): \"%m\"\n"),
                inherited::mod_->name ()));

  goto done;

continue_:
  // sanity check(s)
  ACE_ASSERT (CBData_.loop);

  g_main_loop_run (CBData_.loop);

  // *NOTE*: iff the GStreamer processing thread fails, notify the stream
  //         session about the abort
  if (unlikely (notifyAbort_))
    this->notify (STREAM_SESSION_MESSAGE_ABORT);

  result = 0;

done:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t) leaving\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT (""))));

  return result;
}
