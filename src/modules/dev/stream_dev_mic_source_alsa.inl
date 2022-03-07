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

#include "common_configuration.h"

#include "common_timer_manager_common.h"

#include "common_task_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_alsa_tools.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"

static void
stream_dev_mic_source_alsa_async_callback (snd_async_handler_t* handler_in)
{
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

    frames_to_read = message_block_p->space () / data_p->frameSize;
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
                                          NULL);
    if (unlikely (result == -1))
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
                              1); // silent
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_recover(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
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
                             StatisticHandlerType>::Stream_Dev_Mic_Source_ALSA_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , CBData_ ()
 , handle_ (NULL)
 , handler_ (NULL)
 , frameSize_ (0)
 , isPassive_ (false)
#if defined(_DEBUG)
 , output_ (NULL)
#endif // _DEBUG
 , pollFds_ (NULL)
 , pollFdCount_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::Stream_Dev_Mic_Source_ALSA_T"));

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
  if (output_)
  {
    result = snd_output_close (output_);
    if (unlikely (result < 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_output_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF
#endif // _DEBUG

  if (unlikely (!isPassive_ &&
                handle_))
  {
    result = snd_pcm_close (handle_);
    if (unlikely (result < 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_close(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF

  if (unlikely (pollFds_))
    delete [] pollFds_;
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
    if (unlikely (output_))
    {
      result = snd_output_close (output_);
      if (unlikely (result < 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to snd_output_close(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_strerror (result))));
      output_ = NULL;
    } // end IF
#endif // _DEBUG

    if (unlikely (!isPassive_ &&
                  handle_))
    {
      result = snd_pcm_close (handle_);
      if (unlikely (result < 0))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to snd_pcm_close(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_strerror (result))));
    } // end IF
    handle_ = NULL;
    isPassive_ = false;
    frameSize_ = 0;

    if (unlikely (pollFds_))
    {
      delete [] pollFds_; pollFds_ = NULL;
    } // end IF
    pollFdCount_ = 0;
  } // end IF

#if defined(_DEBUG)
  if (configuration_in.debug)
  {
    result =
        snd_output_stdio_open (&output_,
                               ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEFAULT_LOG_FILE),
                               ACE_TEXT_ALWAYS_CHAR ("w"));
    if (unlikely (result < 0))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_output_stdio_open(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF
#endif // _DEBUG

  ACE_ASSERT (configuration_in.ALSAConfiguration);
  handle_ = configuration_in.ALSAConfiguration->handle;
  isPassive_ = (handle_ != NULL);

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
      const struct Stream_MediaFramework_ALSA_MediaType* media_type_p =
        &media_type_r;

      if (!isPassive_)
      { ACE_ASSERT (!handle_);
        result =
            snd_pcm_open (&handle_,
                          inherited::configuration_->deviceIdentifier.identifier.c_str (),
                          SND_PCM_STREAM_CAPTURE,
                          inherited::configuration_->ALSAConfiguration->mode);
        if (unlikely (result < 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_open(\"%s\",%d) for capture: \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier.c_str ()),
                      inherited::configuration_->ALSAConfiguration->mode,
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: opened ALSA device (capture)...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_pcm_name (handle_))));

        if (!inherited::configuration_->ALSAConfiguration->format)
          inherited::configuration_->ALSAConfiguration->format =
            const_cast<struct Stream_MediaFramework_ALSA_MediaType*> (media_type_p);
        else
          media_type_p = inherited::configuration_->ALSAConfiguration->format;
        if (unlikely (!Stream_MediaFramework_ALSA_Tools::setFormat (handle_,
                                                                    *inherited::configuration_->ALSAConfiguration)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_ALSA_Tools::setFormat(), aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_))));
          goto error;
        } // end IF
      } // end IF
      ACE_ASSERT (handle_);

#if defined (_DEBUG)
      if (output_)
      {
        snd_pcm_dump (handle_, output_);
        snd_pcm_dump_setup (handle_, output_);
        snd_pcm_status_t* status_p = NULL;
        snd_pcm_status_malloc (&status_p);
        ACE_ASSERT (status_p);
        snd_pcm_status (handle_, status_p);
        snd_pcm_status_dump (status_p, output_);
        snd_pcm_status_free (status_p); status_p = NULL;
      } // end IF
      Stream_MediaFramework_ALSA_Tools::dump (handle_, true);
#endif // _DEBUG

//      result = snd_pcm_nonblock (handle_,
//                                 1);
//      if (unlikely (result < 0))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s/%s: failed to snd_pcm_nonblock(\"%s\"): \"%s\", aborting\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (snd_pcm_name (handle_)),
//                    ACE_TEXT (snd_strerror (result))));
//        goto error;
//      } // end IF

      frameSize_ =
        (snd_pcm_format_width (media_type_r.format) / 8) *
        media_type_r.channels;

      if (inherited::configuration_->ALSAConfiguration->asynch)
      {
        ACE_ASSERT (inherited::configuration_->messageAllocator);
        CBData_.allocator = inherited::configuration_->messageAllocator;
        ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
        CBData_.allocatorConfiguration =
          inherited::configuration_->allocatorConfiguration;
        //  CBData_.areas = areas;
        //      CBData_.format = media_type_r;
        CBData_.frameSize = frameSize_;
        CBData_.queue = inherited::msg_queue_;
        CBData_.statistic = &session_data_r.statistic;

        result =
            snd_async_add_pcm_handler (&handler_,
                                       handle_,
                                       stream_dev_mic_source_alsa_async_callback,
                                       &CBData_);
        if (unlikely (result < 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to snd_async_add_pcm_handler(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        signal = snd_async_handler_get_signo (handler_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s/%s: registered asynch PCM handler (signal: %d: \"%S\")...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_pcm_name (handle_)),
                    signal, signal));
      } // end IF
      else
      { ACE_ASSERT (!pollFds_);
        result = snd_pcm_poll_descriptors_count (handle_);
        if (unlikely (result <= 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to snd_pcm_poll_descriptors_count(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        pollFdCount_ = static_cast<unsigned int> (result);
        ACE_NEW_NORETURN (pollFds_,
                          struct pollfd[pollFdCount_]);
        if (unlikely (!pollFds_))
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory (%u byte(s)): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      sizeof (struct pollfd) * pollFdCount_));
          goto error;
        } // end IF
        result = snd_pcm_poll_descriptors (handle_,
                                           pollFds_,
                                           pollFdCount_);
        if (unlikely (static_cast<unsigned int> (result) != pollFdCount_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to snd_pcm_poll_descriptors(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      ACE_TEXT (snd_strerror (result))));
          delete [] pollFds_; pollFds_ = NULL; pollFdCount_ = 0;
          goto error;
        } // end IF
      } // end ELSE

      result =  snd_pcm_start (handle_);
      if (unlikely (result < 0))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s/%s: failed to snd_pcm_start(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_pcm_name (handle_)),
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
      stop_device = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s/%s: started capture device...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (handle_))));

      break;

error:
      if (stop_device)
      {
        result =  snd_pcm_drop (handle_);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to snd_pcm_drop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
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

      if (likely (handle_))
      {
        result = snd_pcm_drop (handle_);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to snd_pcm_drop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s/%s: stopped capture device...\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_))));

        if (!isPassive_)
        {
          result = snd_pcm_hw_free (handle_);
          if (unlikely (result < 0))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s/%s: failed to snd_pcm_hw_free(): \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (snd_pcm_name (handle_)),
                        ACE_TEXT (snd_strerror (result))));
        } // end IF
      } // end IF

      if (inherited::configuration_->ALSAConfiguration->asynch)
      {
        result = snd_async_del_handler (handler_);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: failed to snd_async_del_handler(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s/%s: deregistered asynch PCM handler...\n"),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      inherited::mod_->name ()));
      } // end IF
      else
      {
        if (likely (pollFds_))
        {
          delete [] pollFds_; pollFds_ = NULL;
        } // end IF
        pollFdCount_ = 0;
      } // end ELSE

      if (likely (handle_))
      {
        if (!isPassive_)
        {
          result = snd_pcm_close (handle_);
          if (unlikely (result < 0))
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s/%s: failed to snd_pcm_close(): \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (snd_pcm_name (handle_)),
                        ACE_TEXT (snd_strerror (result))));
        } // end IF
        handle_ = NULL;
      } // end IF

#if defined(_DEBUG)
      if (likely (output_))
      {
        result = snd_output_flush (output_);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_output_flush(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_strerror (result))));
        result = snd_output_close (output_);
        if (unlikely (result < 0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_output_close(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_strerror (result))));
        output_ = NULL;
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
int
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
                             StatisticHandlerType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->ALSAConfiguration);
  if (inherited::configuration_->ALSAConfiguration->asynch)
    return inherited::svc ();
  ACE_ASSERT (inherited::sessionData_);

  if (unlikely (!Common_Task_Tools::setThreadPriority (0,
                                                       std::numeric_limits<int>::min ())))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_Task_Tools::setThreadPriority(), aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: worker thread (id: %t, group: %d) starting\n"),
              inherited::mod_->name (),
              inherited::grp_id_));

  int error = 0;
  bool has_finished = false;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value no_wait = COMMON_TIME_NOW;
  bool release_lock = false;
  int result = -1;
  int result_2 = -1;
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  bool stop_processing = false;
  typename inherited::ISTREAM_T* stream_p =
    const_cast<typename inherited::ISTREAM_T*> (inherited::getP ());
  ACE_Sig_Set sig_set (true); // fill --> block all signals
  unsigned short revents_i;
  snd_pcm_sframes_t available_frames, frames_read = 0;
  snd_pcm_uframes_t frames_to_read;

  do
  {
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p,
                                &no_wait);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Linux: 11 | Win32: 10035
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        break;
      } // end IF
      goto continue_;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
        //         not have been set at this stage

        // signal the controller ?
        if (!has_finished)
        {
          has_finished = true;
          // enqueue(/process) STREAM_SESSION_END
          inherited::finished (false); // recurse upstream ?
        } // end IF

        if (inherited::thr_count_ > 1)
        {
          result_2 = inherited::putq (message_block_p, NULL);
          if (result_2 == -1)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_block_p->release (); message_block_p = NULL;
          } // end IF
          message_block_p = NULL;
        } // end IF
        else
        {
          message_block_p->release (); message_block_p = NULL;
        } // end ELSE

        // has STREAM_SESSION_END been processed ?
        if (!inherited::sessionEndProcessed_)
          continue; // process STREAM_SESSION_END

        result = 0;

        goto done; // STREAM_SESSION_END has been processed
      }
      default:
      {
        // sanity check(s)
        ACE_ASSERT (stream_p);

        // grab lock if processing is 'non-concurrent'
        if (!inherited::configuration_->hasReentrantSynchronousSubDownstream)
          release_lock = stream_p->lock (true);

        inherited::handleMessage (message_block_p,
                                  stop_processing);
        message_block_p = NULL;

        if (release_lock)
          stream_p->unlock (false);

          // finished ?
        if (stop_processing)
        {
          // *IMPORTANT NOTE*: message_block_p has already been released() !

          if (!has_finished)
          {
            has_finished = true;
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?
          } // end IF

          continue;
        } // end IF

        break;
      }
    } // end SWITCH

continue_:
    // session aborted ?
    // sanity check(s)
    // *TODO*: remove type inferences
    ACE_ASSERT (session_data_r.lock);
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock, result);
      if (session_data_r.aborted &&
          !has_finished)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session %u aborted\n"),
                    inherited::mod_->name (),
                    session_data_r.sessionId));
        has_finished = true;
        // enqueue(/process) STREAM_SESSION_END
        inherited::finished (false); // recurse upstream ?
      } // end IF
    } // end lock scope

    // sanity check(s)
    // processed SESSION_BEGIN ?
    if (!pollFds_)
      continue;

#if defined (_GNU_SOURCE)
    result_2 = TEMP_FAILURE_RETRY (ppoll (pollFds_, pollFdCount_, NULL, sig_set));
#else
    result_2 = ppoll (pollFds_, pollFdCount_, NULL, sig_set);
#endif // _GNU_SOURCE
    if (unlikely (result_2 <= 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ppoll(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      break;
    } // end IF
    revents_i = 0;
    result_2 = snd_pcm_poll_descriptors_revents (handle_,
                                                 pollFds_,
                                                 pollFdCount_,
                                                 &revents_i);
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to snd_pcm_poll_descriptors_revents(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (handle_)),
                  ACE_TEXT (snd_strerror (result_2))));
      break;
    } // end IF
    if (unlikely (revents_i & POLLERR))
    {
      bool continue_b = true;
      switch (snd_pcm_state (handle_))
      {
        case SND_PCM_STATE_PREPARED:
        case SND_PCM_STATE_RUNNING:
          break;
        case SND_PCM_STATE_XRUN:
          goto recover_xrun;
        case SND_PCM_STATE_SUSPENDED:
        {
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("%s/%s: suspended, recovering\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_))));
          result_2 = snd_pcm_recover (handle_,
                                      -ESTRPIPE,
                                      1); // silent
          if (unlikely (result_2 < 0))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s/%s: failed to snd_pcm_recover(): \"%s\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (snd_pcm_name (handle_)),
                        ACE_TEXT (snd_strerror (result_2))));
            continue_b = false;
          } // end IF
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s/%s: invalid/unknown device state (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (snd_pcm_name (handle_)),
                      snd_pcm_state (handle_)));
          continue_b = false;
          break;
        }
      } // end SWITCH
      if (unlikely (!continue_b))
        break;
      continue;
    } // end IF
    if (unlikely (!(revents_i & POLLIN)))
      continue;

    ACE_ASSERT (!message_block_p);
    message_block_p =
      inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u) failed: \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->allocatorConfiguration->defaultBufferSize));
      break;
    } // end IF

    available_frames = snd_pcm_avail_update (handle_);
    if (unlikely (available_frames < 0))
    {
      // overrun ? --> recover
      if (available_frames == -EPIPE)
        goto recover_xrun;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to snd_pcm_avail_update(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (handle_)),
                  ACE_TEXT (snd_strerror (result))));
      message_block_p->release (); message_block_p = NULL;
      break;
    } // end IF
    else if (unlikely (available_frames == 0))
    {
      message_block_p->release (); message_block_p = NULL;
      continue;
    } // end ELSE IF

    frames_to_read =
      std::min (message_block_p->space () / frameSize_, static_cast<size_t> (available_frames));
    frames_read = snd_pcm_readi (handle_,
                                 message_block_p->wr_ptr (),
                                 frames_to_read);
    if (unlikely (frames_read <= 0))
    {
      // overrun ? --> recover
      if (frames_read == -EPIPE)
        goto recover_xrun;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to snd_pcm_readi(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (handle_)),
                  ACE_TEXT (snd_strerror (frames_read))));
      message_block_p->release (); message_block_p = NULL;
      break;
    } // end IF
    message_block_p->wr_ptr (static_cast<unsigned int> (frames_read) * frameSize_);
    session_data_r.statistic.capturedFrames += frames_read;

    result_2 = inherited::put_next (message_block_p, NULL);
    if (unlikely (result_2 == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p = NULL;
      break;
    } // end IF

    continue;
recover_xrun:
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s/%s: buffer overrun, recovering\n"),
                inherited::mod_->name (),
                ACE_TEXT (snd_pcm_name (handle_))));

    //        result = snd_pcm_prepare (handle_p);
    result_2 = snd_pcm_recover (handle_,
                                -EPIPE,
                                1); // silent
    if (unlikely (result_2 < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to snd_pcm_recover(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (snd_pcm_name (handle_)),
                  ACE_TEXT (snd_strerror (result_2))));
      if (message_block_p)
      {
        message_block_p->release (); message_block_p = NULL;
      } // end IF
      break;
    } // end IF
    if (message_block_p)
    {
      message_block_p->release (); message_block_p = NULL;
    } // end IF
  } while (true);
  result = -1;

done:
  if (unlikely (!Common_Task_Tools::setThreadPriority (0,
                                                       0)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_Task_Tools::setThreadPriority(), aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF

  return result;
}
