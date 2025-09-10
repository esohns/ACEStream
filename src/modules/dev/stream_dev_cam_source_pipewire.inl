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

#include "spa/param/video/raw-utils.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "common_configuration.h"

#include "common_timer_manager_common.h"

#include "common_task_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_tools.h"

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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::Stream_Dev_Cam_Source_Pipewire_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , CBData_ ()
 , events_ ()
 , isPipewireMainLoopThread_ (false)
 , loop_ (NULL)
 , PODBuffer_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::Stream_Dev_Cam_Source_Pipewire_T"));

  CBData_.queue = inherited::msg_queue_;
  ACE_ASSERT (CBData_.queue);

  events_.version = PW_VERSION_STREAM_EVENTS;
  events_.param_changed = acestream_dev_cam_pw_on_stream_param_changed_cb;
  events_.process = acestream_dev_cam_pw_on_process_cb;

  // ACE_OS::memset (PODBuffer_, 0, sizeof (uint8_t[STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE]));
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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::~Stream_Dev_Cam_Source_Pipewire_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::~Stream_Dev_Cam_Source_Pipewire_T"));

  if (likely (CBData_.stream))
    pw_stream_destroy (CBData_.stream);
  if (likely (loop_))
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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (unlikely (CBData_.stream))
    {
      pw_stream_destroy (CBData_.stream); CBData_.stream = NULL;
    } // end IF

    if (likely (loop_))
    {
      pw_main_loop_destroy (loop_); loop_ = NULL;
    } // end IF

    isPipewireMainLoopThread_ = false;
  } // end IF

  ACE_ASSERT (!loop_);
  loop_ = pw_main_loop_new (NULL);
  if (unlikely (!loop_))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to pw_main_loop_new(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF

  struct pw_properties* properties_p =
      pw_properties_new (PW_KEY_MEDIA_TYPE, ACE_TEXT_ALWAYS_CHAR ("Video"),
                         PW_KEY_MEDIA_CATEGORY, ACE_TEXT_ALWAYS_CHAR ("Capture"),
                         PW_KEY_MEDIA_ROLE, ACE_TEXT_ALWAYS_CHAR ("Camera"),
                         NULL);
  if (unlikely (!properties_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to pw_properties_new(), aborting\n"),
                inherited::mod_->name ()));
    pw_main_loop_destroy (loop_); loop_ = NULL;
    return false;
  } // end IF

  ACE_ASSERT (!CBData_.stream);
  CBData_.stream = pw_stream_new_simple (pw_main_loop_get_loop (loop_),
                                         ACE_TEXT_ALWAYS_CHAR ("video-capture"),
                                         properties_p,
                                         &events_,
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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 ConfigurationType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 StreamStateType,
                                 StatisticContainerType,
                                 SessionManagerType,
                                 TimerManagerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  const typename SessionMessageType::DATA_T::DATA_T* session_data_p =
      &inherited::sessionData_->getR ();
  message_inout->initialize (session_data_p->sessionId,
                             NULL);
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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::handleSessionMessage"));

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
      struct Stream_MediaFramework_V4L_MediaType media_type_s;
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);

      struct spa_pod_builder POD_builder_s =
        SPA_POD_BUILDER_INIT (PODBuffer_, sizeof (uint8_t[STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE]));
      const struct spa_pod* parameters_a[1];
      struct spa_video_info_raw video_info_raw_s;
      ACE_OS::memset (&video_info_raw_s, 0, sizeof (struct spa_video_info_raw));
      video_info_raw_s.format =
        Stream_MediaFramework_Tools::v4lFormatToPipewireFormat (media_type_s.format.pixelformat);
      video_info_raw_s.size.width = media_type_s.format.width;
      video_info_raw_s.size.height = media_type_s.format.height;
      video_info_raw_s.framerate.num = media_type_s.frameRate.numerator;
      video_info_raw_s.framerate.denom = media_type_s.frameRate.denominator;
      video_info_raw_s.pixel_aspect_ratio.num = 1;
      video_info_raw_s.pixel_aspect_ratio.denom = 1;
      video_info_raw_s.chroma_site = SPA_VIDEO_CHROMA_SITE_NONE;
      video_info_raw_s.color_range = SPA_VIDEO_COLOR_RANGE_0_255;
      video_info_raw_s.color_matrix = SPA_VIDEO_COLOR_MATRIX_RGB;
      video_info_raw_s.transfer_function = SPA_VIDEO_TRANSFER_GAMMA10;
      parameters_a[0] = spa_format_video_raw_build (&POD_builder_s,
                                                    SPA_PARAM_EnumFormat,
                                                    &video_info_raw_s);
      // parameters_a[0] = spa_pod_builder_add_object (&POD_builder_s,
      //                 SPA_TYPE_OBJECT_Format, SPA_PARAM_EnumFormat,
      //                 SPA_FORMAT_mediaType,       SPA_POD_Id(SPA_MEDIA_TYPE_video),
      //                 SPA_FORMAT_mediaSubtype,    SPA_POD_Id(SPA_MEDIA_SUBTYPE_raw),
      //                 SPA_FORMAT_VIDEO_format,    SPA_POD_CHOICE_ENUM_Id(7,
      //                                                 SPA_VIDEO_FORMAT_RGB,
      //                                                 SPA_VIDEO_FORMAT_RGB,
      //                                                 SPA_VIDEO_FORMAT_RGBA,
      //                                                 SPA_VIDEO_FORMAT_RGBx,
      //                                                 SPA_VIDEO_FORMAT_BGRx,
      //                                                 SPA_VIDEO_FORMAT_YUY2,
      //                                                 SPA_VIDEO_FORMAT_I420),
      //                 SPA_FORMAT_VIDEO_size,      SPA_POD_CHOICE_RANGE_Rectangle(
      //                                                 &SPA_RECTANGLE(media_type_s.format.width, media_type_s.format.height),
      //                                                 &SPA_RECTANGLE(1, 1),
      //                                                 &SPA_RECTANGLE(4096, 4096)),
      //                 SPA_FORMAT_VIDEO_framerate, SPA_POD_CHOICE_RANGE_Fraction(
      //                                                 &SPA_FRACTION(media_type_s.frameRate.numerator, media_type_s.frameRate.denominator),
      //                                                 &SPA_FRACTION(0, 1),
      //                                                 &SPA_FRACTION(1000, 1)));
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
        bool lock_activate_was_b = inherited::TASK_BASE_T::TASK_BASE_T::lockActivate_;
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

      ACE_ASSERT (loop_);
      pw_main_loop_quit (loop_);
      // *TODO*: wait for thread

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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::collect"));

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
Stream_Dev_Cam_Source_Pipewire_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Cam_Source_Pipewire_T::svc"));

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

    pw_stream_destroy (CBData_.stream); CBData_.stream = NULL;

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
