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

#include "pipewire/core.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_configuration.h"

#include "common_timer_manager_common.h"

#include "common_task_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_pipewire_tools.h"

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
          typename TimerManagerType>
Stream_Dev_Mic_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::Stream_Dev_Mic_Source_Pipewire_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , CBData_ ()
 , context_ (NULL)
 , isPipewireMainLoopThread_ (false)
 , loop_ (NULL)
 , PODBuffer_ ()
 , registryEvents_ ()
 , registryListener_ ()
 , streamEvents_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_Pipewire_T::Stream_Dev_Mic_Source_Pipewire_T"));

  ACE_OS::memset (&CBData_, 0, sizeof (struct Stream_Device_Pipewire_Capture_CBData));
  CBData_.clientEvents.version = PW_VERSION_CLIENT_EVENTS;
  CBData_.clientEvents.info = acestream_dev_mic_pw_on_client_event_info_cb;
  CBData_.deviceEvents.version = PW_VERSION_DEVICE_EVENTS;
  CBData_.deviceEvents.info = acestream_dev_mic_pw_on_device_event_info_cb;
  CBData_.deviceEvents.param = acestream_dev_mic_pw_on_device_event_param_cb;
  CBData_.queue = inherited::msg_queue_;
  ACE_ASSERT (CBData_.queue);

  // ACE_OS::memset (PODBuffer_, 0, sizeof (uint8_t[STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE]));

  ACE_OS::memset (&registryEvents_, 0, sizeof (struct pw_registry_events));
  registryEvents_.version = PW_VERSION_REGISTRY_EVENTS;
  registryEvents_.global = acestream_dev_mic_pw_on_registry_event_global_cb;

  ACE_OS::memset (&registryListener_, 0, sizeof (struct spa_hook));

  ACE_OS::memset (&streamEvents_, 0, sizeof (struct pw_stream_events));
  streamEvents_.version = PW_VERSION_STREAM_EVENTS;
  streamEvents_.param_changed = acestream_dev_mic_pw_on_stream_param_changed_cb;
  streamEvents_.process = acestream_dev_mic_pw_on_process_cb;
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
          typename TimerManagerType>
Stream_Dev_Mic_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::~Stream_Dev_Mic_Source_Pipewire_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_Pipewire_T::~Stream_Dev_Mic_Source_Pipewire_T"));

  // *NOTE*: this doesn't work as the pipewire loop needs to destroy the proxies
  if (CBData_.stream)
  {
    pw_stream_disconnect (CBData_.stream); pw_stream_destroy (CBData_.stream);
  } // end IF
  if (CBData_.client)
    pw_proxy_destroy (CBData_.client);
  if (CBData_.device)
    pw_proxy_destroy (CBData_.device);
  if (CBData_.node)
    pw_proxy_destroy (CBData_.node);
  if (CBData_.registry)
    pw_proxy_destroy ((struct pw_proxy*)CBData_.registry);
  if (CBData_.core)
    pw_core_disconnect (CBData_.core);
  if (context_)
    pw_context_destroy (context_);

  if (loop_)
    pw_main_loop_destroy (loop_);
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
          typename TimerManagerType>
bool
Stream_Dev_Mic_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::initialize (const ConfigurationType& configuration_in,
                                                                Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_Pipewire_T::initialize"));

  if (inherited::isInitialized_)
  {
    // *NOTE*: this doesn't work as the pipewire loop needs to destroy the
    //         proxies
    if (unlikely (CBData_.stream))
    {
      pw_stream_disconnect (CBData_.stream); pw_stream_destroy (CBData_.stream); CBData_.stream = NULL;
    } // end IF
    if (unlikely (CBData_.client))
    {
      pw_proxy_destroy (CBData_.client); CBData_.client = NULL;
    } // end IF
    if (unlikely (CBData_.device))
    {
      pw_proxy_destroy (CBData_.device); CBData_.device = NULL;
    } // end IF
    if (unlikely (CBData_.node))
    {
      pw_proxy_destroy (CBData_.node); CBData_.node = NULL;
    } // end IF
    if (unlikely (CBData_.registry))
    {
      pw_proxy_destroy ((struct pw_proxy*)CBData_.registry); CBData_.registry = NULL;
    } // end IF
    if (unlikely (CBData_.core))
    {
      pw_core_disconnect (CBData_.core); CBData_.core = NULL;
    } // end IF
    if (unlikely (context_))
    {
      pw_context_destroy (context_); context_ = NULL;
    } // end IF

    if (unlikely (loop_))
    {
      pw_main_loop_destroy (loop_); loop_ = NULL;
    } // end IF

    CBData_.nodeName.clear ();
    isPipewireMainLoopThread_ = false;
  } // end IF

  // source- node name
  CBData_.nodeName = configuration_in.nodeName;

  ACE_ASSERT (!loop_);
  loop_ = pw_main_loop_new (NULL);
  if (unlikely (!loop_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to pw_main_loop_new(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  CBData_.loop = loop_;

  ACE_ASSERT (!context_);
  struct pw_loop* loop_p = pw_main_loop_get_loop (loop_);
  ACE_ASSERT (loop_p);
  context_ = pw_context_new (loop_p, NULL, 0);
  if (unlikely (!context_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to pw_context_new(), aborting\n"),
                inherited::mod_->name ()));
    pw_main_loop_destroy (loop_); loop_ = NULL;
    return false;
  } // end IF
  CBData_.core = pw_context_connect (context_, NULL, 0);
  if (unlikely (!CBData_.core))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to pw_context_connect(), aborting\n"),
                inherited::mod_->name ()));
    pw_context_destroy (context_); context_ = NULL;
    pw_main_loop_destroy (loop_); loop_ = NULL;
    return false;
  } // end IF

  CBData_.registry = pw_core_get_registry (CBData_.core,
                                           PW_VERSION_REGISTRY,
                                           0);
  ACE_ASSERT (CBData_.registry);
  pw_registry_add_listener (CBData_.registry,
                            &registryListener_,
                            &registryEvents_,
                            &CBData_);

  // setup stream
  struct pw_properties* properties_p =
    pw_properties_new (PW_KEY_MEDIA_TYPE, ACE_TEXT_ALWAYS_CHAR ("Audio"),
                       PW_KEY_MEDIA_CATEGORY, ACE_TEXT_ALWAYS_CHAR ("Capture"),
                       PW_KEY_MEDIA_ROLE, ACE_TEXT_ALWAYS_CHAR ("Music"),
                       NULL);
  if (unlikely (!properties_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to pw_properties_new(), aborting\n"),
                inherited::mod_->name ()));
    pw_main_loop_destroy (loop_); loop_ = NULL;
    return false;
  } // end IF
  // "Stereo Mix" ?
  pw_properties_set (properties_p,
                     PW_KEY_STREAM_CAPTURE_SINK, ACE_TEXT_ALWAYS_CHAR ("true"));

  ACE_ASSERT (!CBData_.stream);
  CBData_.stream = pw_stream_new_simple (loop_p,
                                         ACE_TEXT_ALWAYS_CHAR ("audio-capture"),
                                         properties_p,
                                         &streamEvents_,
                                         &CBData_);
  if (unlikely (!CBData_.stream))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to pw_stream_new_simple(), aborting\n"),
                inherited::mod_->name ()));
    pw_properties_free (properties_p);
    pw_main_loop_destroy (loop_); loop_ = NULL;
    return false;
  } // end IF

  CBData_.allocator = configuration_in.messageAllocator;
  CBData_.allocatorConfiguration = configuration_in.allocatorConfiguration;
  CBData_.statistic = &(inherited::statistic_);

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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
void
Stream_Dev_Mic_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_Pipewire_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    { // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      typename SessionMessageType::DATA_T::DATA_T& session_data_r =
          const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct Stream_MediaFramework_ALSA_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);

      CBData_.frameSize =
        (snd_pcm_format_width (media_type_s.format) / 8) * media_type_s.channels;

      struct spa_pod_builder POD_builder_s =
        SPA_POD_BUILDER_INIT (PODBuffer_, sizeof (uint8_t[STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE]));
      const struct spa_pod* parameters_a[1];
      struct spa_audio_info_raw audio_info_raw_s;
      ACE_OS::memset (&audio_info_raw_s, 0, sizeof (struct spa_audio_info_raw));
      audio_info_raw_s.channels = media_type_s.channels;
      audio_info_raw_s.format =
        Stream_MediaFramework_ALSA_Tools::ALSAFormatToPipewireFormat (media_type_s.format);
      audio_info_raw_s.position[0] = SPA_AUDIO_CHANNEL_FL;
      audio_info_raw_s.position[1] = SPA_AUDIO_CHANNEL_FR;
      audio_info_raw_s.rate = media_type_s.rate;
      parameters_a[0] = spa_format_audio_raw_build (&POD_builder_s,
                                                    SPA_PARAM_EnumFormat,
                                                    &audio_info_raw_s);
      ACE_ASSERT (parameters_a[0]);
      ACE_ASSERT (CBData_.stream);
      enum pw_stream_flags stream_flags_e =
          static_cast<enum pw_stream_flags> (PW_STREAM_FLAG_AUTOCONNECT |
                                             PW_STREAM_FLAG_MAP_BUFFERS /*|
                                             PW_STREAM_FLAG_RT_PROCESS*/);
      result = pw_stream_connect (CBData_.stream,
                                  PW_DIRECTION_INPUT,
                                  PW_ID_ANY,
                                  stream_flags_e,
                                  parameters_a, 1);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to pw_stream_connect(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      // *NOTE*: this prevents a race condition in svc()
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        inherited::threadCount_ = 1;
        bool lock_activate_was_b =
          inherited::TASK_BASE_T::TASK_BASE_T::lockActivate_;
        inherited::lockActivate_ = false;

        result = inherited::TASK_BASE_T::open (NULL);
        if (unlikely (result == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_Task_Base_T::open(), aborting\n"),
                      inherited::mod_->name ()));
          inherited::lockActivate_ = lock_activate_was_b;
          inherited::threadCount_ = 0;
          goto error;
        } // end IF
        inherited::lockActivate_ = lock_activate_was_b;
        inherited::threadCount_ = 0;
        ACE_ASSERT (inherited::threadIds_.size () == 2);
      } // end lock scope

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      // clean up
      if (likely (CBData_.stream))
      {
        Stream_MediaFramework_Pipewire_Tools::finalizeStream (pw_main_loop_get_loop (loop_),
                                                              CBData_.stream);
        CBData_.stream = NULL;
      } // end IF
      if (likely (CBData_.client))
      {
        Stream_MediaFramework_Pipewire_Tools::freeProxy (pw_main_loop_get_loop (loop_),
                                                         CBData_.client);
        CBData_.client = NULL;
      } // end IF
      if (likely (CBData_.device))
      {
        Stream_MediaFramework_Pipewire_Tools::freeProxy (pw_main_loop_get_loop (loop_),
                                                         CBData_.device);
        CBData_.device = NULL;
      } // end IF
      if (likely (CBData_.node))
      {
        Stream_MediaFramework_Pipewire_Tools::freeProxy (pw_main_loop_get_loop (loop_),
                                                         CBData_.node);
        CBData_.node = NULL;
      } // end IF
      if (likely (CBData_.registry))
      {
        Stream_MediaFramework_Pipewire_Tools::freeProxy (pw_main_loop_get_loop (loop_),
                                                         (struct pw_proxy*)CBData_.registry);
        CBData_.registry = NULL;
      } // end IF
      if (likely (CBData_.core))
      {
        pw_core_disconnect (CBData_.core); CBData_.core = NULL;
      } // end IF
      if (likely (context_))
      {
        pw_context_destroy (context_); context_ = NULL;
      } // end IF

      if (likely (loop_))
        pw_main_loop_quit (loop_);
      while (inherited::thr_count_ > 1)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: waiting for pipewire loop thread...\n"),
                    inherited::mod_->name ()));
        ACE_OS::sleep (ACE_Time_Value (1, 0));
      } // end WHILE

      if (likely (loop_))
      {
        pw_main_loop_destroy (loop_); loop_ = NULL;
      } // end IF

      if (inherited::configuration_->concurrency != STREAM_HEADMODULECONCURRENCY_CONCURRENT)
      { Common_ITask* itask_p = this;
        itask_p->stop (false,  // wait ?
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType>
bool
Stream_Dev_Mic_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_Pipewire_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  // step1: collect data
//  long average_frame_size = 0;

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
          typename TimerManagerType>
int
Stream_Dev_Mic_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_Pipewire_T::svc"));

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

  int result = -1;

  // *NOTE*: there should (!) never be a race here, as the second thread is
  //         started by the first (see above)
  if (isPipewireMainLoopThread_)
  { ACE_ASSERT (loop_);
    result = pw_main_loop_run (loop_);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to pw_main_loop_run(), aborting\n"),
                  inherited::mod_->name ()));
      this->notify (STREAM_SESSION_MESSAGE_ABORT);
    } // end IF

    goto done;
  } // end IF
  isPipewireMainLoopThread_ = true;

//////////////////////////////////////////

  result = inherited::svc ();

//////////////////////////////////////////

done:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: (%s): worker thread (id: %t, group: %d) leaving\n"),
              inherited::mod_->name (),
              ACE_TEXT (inherited::threadName_.c_str ()),
              inherited::grp_id_));

  return result;
}
