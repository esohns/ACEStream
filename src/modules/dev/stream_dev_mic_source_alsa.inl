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
#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"

#include "common_configuration.h"
#include "common_file_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_lib_alsa_tools.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"

static void
stream_dev_mic_source_alsa_async_callback (snd_async_handler_t* handler_in)
{
  // never wait for the queue
  static ACE_Time_Value no_wait = ACE_OS::gettimeofday ();

  // sanity check(s)
  ACE_ASSERT (handler_in);
  struct Stream_Device_ALSA_Capture_AsynchCBData* data_p =
      static_cast<struct Stream_Device_ALSA_Capture_AsynchCBData*> (snd_async_handler_get_callback_private (handler_in));
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->allocatorConfiguration);
  ACE_ASSERT (data_p->queue);
  ACE_ASSERT (data_p->statistic);
  snd_pcm_t* handle_p = snd_async_handler_get_pcm (handler_in);
  ACE_ASSERT (handle_p);

  snd_pcm_sframes_t available_frames, frames_read = 0;
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  snd_pcm_uframes_t frames_to_read;

  do
  {
    available_frames = snd_pcm_avail_update (handle_p);
    if (unlikely (available_frames < 0))
    {
      // overrun ? --> recover
      if (available_frames == -EPIPE)
        goto recover;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    if (unlikely (available_frames == 0))
      break;

    if (!message_block_p)
    {
      if (likely (data_p->allocator))
      {
        try {
          message_block_p =
              static_cast<ACE_Message_Block*> (data_p->allocator->malloc (data_p->allocatorConfiguration->defaultBufferSize));
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                      ACE_TEXT (snd_pcm_name (handle_p)),
                      data_p->allocatorConfiguration->defaultBufferSize));
          message_block_p = NULL;
        }
      } // end IF
      else
        ACE_NEW_NORETURN (message_block_p,
                          ACE_Message_Block (data_p->allocatorConfiguration->defaultBufferSize));
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                    ACE_TEXT (snd_pcm_name (handle_p))));
        goto error;
      } // end IF
    } // end IF

    frames_to_read = message_block_p->size () / data_p->frameSize;
    frames_to_read =
      (frames_to_read > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                          : frames_to_read);
    frames_read = snd_pcm_readi (handle_p,
                                 message_block_p->wr_ptr (),
                                 frames_to_read);
    if (unlikely (frames_read < 0))
    {
      // overrun ? --> recover
      if (frames_read == -EPIPE)
        goto recover;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_readi(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (frames_read))));
      goto error;
    } // end IF
    message_block_p->wr_ptr (static_cast<unsigned int> (frames_read) * data_p->frameSize);
    data_p->statistic->capturedFrames += frames_read;

    result = data_p->queue->enqueue_tail (message_block_p,
                                          &no_wait);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p))));
      goto error;
    } // end IF
    message_block_p = NULL;

    continue;

recover:
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: buffer overrun, recovering\n"),
                ACE_TEXT (snd_pcm_name (handle_p))));

//        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (handle_p,
                              -EPIPE,
                              1);
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_recover(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
  } while (true);

error:
  if (message_block_p)
    message_block_p->release ();
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
          typename StatisticHandlerType>
Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
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
                             StatisticHandlerType>::Stream_Dev_Mic_Source_ALSA_T (ISTREAM_T* stream_in,
                                                                                  bool autoStart_in,
                                                                                  enum Stream_HeadModuleConcurrency concurrency_in)
 : inherited (stream_in,
              autoStart_in,
              concurrency_in,
              true)
 , asynchCBData_ ()
 , asynchHandler_ (NULL)
#if defined(_DEBUG)
 , debugOutput_ (NULL)
#endif // _DEBUG
 , deviceHandle_ (NULL)
 , isPassive_ (false)
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::Stream_Dev_Mic_Source_ALSA_T"));

  inherited::msg_queue (&queue_);
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
          typename StatisticHandlerType>
Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
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
                             StatisticHandlerType>::~Stream_Dev_Mic_Source_ALSA_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::~Stream_Dev_Mic_Source_ALSA_T"));

  int result = -1;

#if defined(_DEBUG)
  if (debugOutput_)
  {
    result = snd_output_close (debugOutput_);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF
#endif // _DEBUG

  if (!isPassive_)
    if (deviceHandle_)
    {
      result = snd_pcm_close (deviceHandle_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                    ACE_TEXT (snd_strerror (result))));
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
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
                             StatisticHandlerType>::initialize (const ConfigurationType& configuration_in,
                                                                Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
#if defined(_DEBUG)
    if (debugOutput_)
    {
      result = snd_output_close (debugOutput_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                    ACE_TEXT (snd_strerror (result))));
      debugOutput_ = NULL;
    } // end IF
#endif // _DEBUG

    if (!isPassive_)
      if (deviceHandle_)
      {
        result = snd_pcm_close (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
        deviceHandle_ = NULL;
      } // end IF
    isPassive_ = false;
  } // end IF

#if defined(_DEBUG)
  if (configuration_in.debug)
  {
    result =
        snd_output_stdio_open (&debugOutput_,
                               ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEFAULT_LOG_FILE),
                               ACE_TEXT_ALWAYS_CHAR ("w"));
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_output_stdio_open(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF
#endif // _DEBUG

  ACE_ASSERT (configuration_in.ALSAConfiguration);
  deviceHandle_ = configuration_in.ALSAConfiguration->handle;
  if (deviceHandle_)
    isPassive_ = true;

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
          typename StatisticHandlerType>
void
Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
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
                             StatisticHandlerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->ALSAConfiguration);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    { // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
      const struct Stream_MediaFramework_ALSA_MediaType& media_type_r =
          session_data_r.formats.back ();

      bool stop_device = false;
      int signal = 0;

      if (!isPassive_)
      { ACE_ASSERT (!deviceHandle_);
        int mode = STREAM_LIB_ALSA_CAPTURE_DEFAULT_MODE;
//         if (inherited::configuration_->ALSAConfiguration->asynch)
//           mode |= SND_PCM_ASYNC;
        result =
            snd_pcm_open (&deviceHandle_,
                          inherited::configuration_->deviceIdentifier.identifier.c_str (),
                          SND_PCM_STREAM_CAPTURE,
                          mode);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_open(\"%s\",%d) for capture: \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier.c_str ()),
                      mode,
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: opened ALSA device (capture) \"%s\"...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier.c_str ())));

        ACE_ASSERT (!inherited::configuration_->ALSAConfiguration->format);
        inherited::configuration_->ALSAConfiguration->format =
          &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (media_type_r);
        if (!Stream_MediaFramework_ALSA_Tools::setFormat (deviceHandle_,
                                                          *inherited::configuration_->ALSAConfiguration))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_ALSA_Tools::setFormat(), aborting\n"),
                      inherited::mod_->name ()));
          inherited::configuration_->ALSAConfiguration->format = NULL;
          goto error;
        } // end IF
        inherited::configuration_->ALSAConfiguration->format = NULL;
      } // end IF
      ACE_ASSERT (deviceHandle_);

#if defined(_DEBUG)
      if (debugOutput_)
      {
        result = snd_pcm_dump (deviceHandle_,
                               debugOutput_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_dump(\"%s\"): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier.c_str ()),
                      ACE_TEXT (snd_strerror (result))));
        result = snd_pcm_dump_setup (deviceHandle_,
                                     debugOutput_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_dump_setup(\"%s\"): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier.c_str ()),
                      ACE_TEXT (snd_strerror (result))));
  //      result = snd_pcm_dump_hw_setup (deviceHandle_,
  //                                      debugOutput_);
  //      if (result < 0)
  //        ACE_DEBUG ((LM_ERROR,
  //                    ACE_TEXT ("failed to snd_pcm_dump_hw_setup(\"%s\"): \"%s\", continuing\n"),
  //                    ACE_TEXT (inherited::configuration_->deviceIdentifier.c_str ()),
  //                    ACE_TEXT (snd_strerror (result))));
  //      result = snd_pcm_dump_sw_setup (deviceHandle_,
  //                                      debugOutput_);
  //      if (result < 0)
  //        ACE_DEBUG ((LM_ERROR,
  //                    ACE_TEXT ("failed to snd_pcm_dump_sw_setup(\"%s\"): \"%s\", continuing\n"),
  //                    ACE_TEXT (inherited::configuration_->deviceIdentifier.c_str ()),
  //                    ACE_TEXT (snd_strerror (result))));
      } // end IF
#endif // _DEBUG

      ACE_ASSERT (inherited::configuration_->messageAllocator);
      asynchCBData_.allocator = inherited::configuration_->messageAllocator;
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
      asynchCBData_.allocatorConfiguration =
        inherited::configuration_->allocatorConfiguration;
//  asynchCBData_.areas = areas;
//      asynchCBData_.format = media_type_r;
      asynchCBData_.frameSize =
        (snd_pcm_format_width (media_type_r.format) / 8) *
          media_type_r.channels;
      asynchCBData_.queue = inherited::msg_queue ();
      asynchCBData_.statistic = &session_data_r.statistic;

      result =
          snd_async_add_pcm_handler (&asynchHandler_,
                                     deviceHandle_,
                                     stream_dev_mic_source_alsa_async_callback,
                                     &asynchCBData_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to snd_async_add_pcm_handler(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
      signal = snd_async_handler_get_signo (asynchHandler_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\": registered asynch PCM handler (signal: %d: \"%S\")...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (deviceHandle_)),
                  signal,
                  signal));

      ACE_ASSERT (deviceHandle_);
      result =  snd_pcm_start (deviceHandle_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to snd_pcm_start(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
      stop_device = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\": started capture device...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (deviceHandle_))));

      break;

error:
      if (stop_device)
      {
        result =  snd_pcm_drop (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_drop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_strerror (result))));
      } // end IF

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

      if (deviceHandle_)
      {
        result = snd_pcm_drop (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_drop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: \"%s\": stopped capture device...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (deviceHandle_))));

        if (!isPassive_)
        {
          result = snd_pcm_hw_free (deviceHandle_);
          if (result < 0)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to snd_pcm_hw_free(): \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (snd_strerror (result))));
        } // end IF
      } // end IF

      result = snd_async_del_handler (asynchHandler_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to snd_async_del_handler(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_strerror (result))));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: deregistered asynch PCM handler...\n"),
                    inherited::mod_->name ()));

      if (deviceHandle_)
      {
        if (!isPassive_)
        {
          result = snd_pcm_close (deviceHandle_);
          if (result < 0)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to snd_pcm_close(): \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (snd_strerror (result))));
        } // end IF
        deviceHandle_ = NULL;
      } // end IF

#if defined(_DEBUG)
      if (debugOutput_)
      {
        result = snd_output_close (debugOutput_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_output_close(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_strerror (result))));
        debugOutput_ = NULL;
      } // end IF
#endif // _DEBUG

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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename StatisticHandlerType>
bool
Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
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
                             StatisticHandlerType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::collect"));

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
