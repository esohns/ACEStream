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

#include "common_file_tools.h"
#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

//#include "stream_dec_defines.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

static void
stream_dev_target_alsa_async_callback (snd_async_handler_t* handler_in)
{
  // sanity check(s)
  ACE_ASSERT (handler_in);

  struct Stream_Module_Device_ALSA_Playback_AsynchCBData* data_p =
      reinterpret_cast<struct Stream_Module_Device_ALSA_Playback_AsynchCBData*> (snd_async_handler_get_callback_private (handler_in));
  snd_pcm_t* handle_p = snd_async_handler_get_pcm (handler_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (handle_p);

  snd_pcm_sframes_t available_frames, frames_written = 0;
  int result = -1;
  snd_pcm_uframes_t frames_to_write;

  do
  {
    available_frames = snd_pcm_avail_update (handle_p);
    if (available_frames < 0)
    {
      // overrun ? --> recover
      if (available_frames == -EPIPE)
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    if (available_frames == 0)
      break;

    if (!data_p->currentBuffer)
    {
      result = data_p->queue->dequeue (data_p->currentBuffer,
                                       NULL);
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue(): \"%m\", returning\n")));
        goto error;
      } // end IF
    } // end IF
    ACE_ASSERT (data_p->currentBuffer);

    frames_to_write = data_p->currentBuffer->length () / data_p->sampleSize;
    frames_to_write =
        (frames_to_write > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                             : frames_to_write);
    frames_written = snd_pcm_writei (handle_p,
                                     data_p->currentBuffer->rd_ptr (),
                                     frames_to_write);
    if (frames_written < 0)
    {
      // overrun ? --> recover
      if (frames_written == -EPIPE)
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_writei(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    data_p->currentBuffer->rd_ptr (frames_written * data_p->sampleSize);

    if (data_p->currentBuffer->length () == 0)
    {
      data_p->currentBuffer->release ();
      data_p->currentBuffer = NULL;
    } // end IF

    continue;

recover:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("buffer underrun, recovering\n")));

    //        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (handle_p,
                              -EPIPE,
                              1);
    if (result < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_recover(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
  } while (true);

error:
  if (data_p->currentBuffer)
  {
    data_p->currentBuffer->release ();
    data_p->currentBuffer = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::Stream_Dev_Target_ALSA_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , asynchCBData_ ()
 , asynchHandler_ (NULL)
 , debugOutput_ (NULL)
 , deviceHandle_ (NULL)
 , isPassive_ (false)
 , useALSAAsynch_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::Stream_Dev_Target_ALSA_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::~Stream_Dev_Target_ALSA_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::~Stream_Dev_Target_ALSA_T"));

  int result = -1;

  if (useALSAAsynch_)
  {
    QUEUE_T* message_queue_p = inherited::msg_queue ();
    ACE_ASSERT (message_queue_p);
    result = message_queue_p->deactivate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
    result = message_queue_p->flush ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));

    if (asynchCBData_.currentBuffer)
      asynchCBData_.currentBuffer->release ();
  } // end IF

  if (debugOutput_)
  {
    result = snd_output_close (debugOutput_);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF

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
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
bool
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::initialize"));

  int result = -1;
  bool result_2 = false;
  QUEUE_T* message_queue_p = inherited::msg_queue ();
  ACE_ASSERT (message_queue_p);

  if (inherited::isInitialized_)
  {
    if (useALSAAsynch_)
    {
      result = message_queue_p->deactivate ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", aborting\n")));
        return false;
      } // end IF
      result = message_queue_p->flush ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", aborting\n")));
        return false;
      } // end IF

      if (asynchCBData_.currentBuffer)
      {
        asynchCBData_.currentBuffer->release ();
        asynchCBData_.currentBuffer = NULL;
      } // end IF

      useALSAAsynch_ = false;
    } // end IF

    if (debugOutput_)
    {
      result = snd_output_close (debugOutput_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                    ACE_TEXT (snd_strerror (result))));
      debugOutput_ = NULL;
    } // end IF

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

  useALSAAsynch_ = configuration_in.asynchPlayback;
  if (useALSAAsynch_)
  {
    result = message_queue_p->activate ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::activate(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

#if defined (_DEBUG)
  result =
      snd_output_stdio_open (&debugOutput_,
                             ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_ALSA_DEFAULT_LOG_FILE),
                             ACE_TEXT_ALWAYS_CHAR ("w"));
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_output_stdio_open(): \"%s\", continuing\n"),
                ACE_TEXT (snd_strerror (result))));
#endif

//  if (!Stream_Module_Device_Tools::initializeCapture (deviceHandle_,
//                                                      configuration_in.method,
//                                                      const_cast<ConfigurationType&> (configuration_in).buffers))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeCapture(%d): \"%m\", aborting\n"),
//                captureFileDescriptor_));
//    goto error;
//  } // end IF

  result_2 = inherited::initialize (configuration_in,
                                    allocator_in);
  if (!result_2)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_HeadModuleTaskBase_T::initialize(): \"%m\", aborting\n")));

  return result_2;

//error:
  if (debugOutput_)
  {
    result = snd_output_close (debugOutput_);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    debugOutput_ = NULL;
  } // end IF

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::handleDataMessage"));

  // sanity check(s)
  if (!deviceHandle_)
    return;

  ACE_Message_Block* message_block_p = message_inout;
  if (useALSAAsynch_)
  {
    message_block_p = message_inout->duplicate ();
    if (!message_block_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF

    ACE_Message_Queue_Base* message_queue_p = inherited::msg_queue ();
    ACE_ASSERT (message_queue_p);
    int result = message_queue_p->enqueue (message_block_p, NULL);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", returning\n"),
                  inherited::mod_->name ()));

      // clean up
      message_block_p->release ();

      return;
    } // end IF

    return;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->format);

  snd_pcm_sframes_t available_frames, frames_written = 0;
  int result = -1;
  snd_pcm_uframes_t frames_to_write;
  unsigned int offset = 0;
  unsigned int bytes_to_write = message_block_p->length ();
  unsigned int sample_size =
      (snd_pcm_format_width (inherited::configuration_->format->format) / 8) *
      inherited::configuration_->format->channels;

  do
  {
    available_frames = snd_pcm_avail_update (deviceHandle_);
    if (available_frames < 0)
    {
      // overrun ? --> recover
      if (available_frames == -EPIPE)
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
    if (available_frames == 0)
    {
      result = snd_pcm_wait (deviceHandle_, -1);
      if (result < 0)
      {
        // underrun ? --> recover
        if (result == -EPIPE)
          goto recover;

        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_wait(): \"%s\", returning\n"),
                    ACE_TEXT (snd_strerror (result))));
        return;
      } // end IF

      continue;
    } // end IF

    frames_to_write = bytes_to_write / sample_size;
    frames_to_write =
        (frames_to_write > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                             : frames_to_write);
    frames_written = snd_pcm_writei (deviceHandle_,
                                     message_block_p->rd_ptr () + offset,
                                     frames_to_write);
    if (frames_written < 0)
    {
      // overrun ? --> recover
      if (frames_written == -EPIPE)
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_writei(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
    bytes_to_write -= (frames_written * sample_size);
    offset += (frames_written * sample_size);

    if (bytes_to_write == 0)
    {
      if (!message_block_p->cont ())
        break;

      message_block_p = message_block_p->cont ();
      bytes_to_write = message_block_p->length ();
      offset = 0;
    } // end IF

    continue;

recover:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("buffer underrun, recovering\n")));

    //        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (deviceHandle_,
                              -EPIPE,
                              1);
    if (result < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_recover(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
  } while (true);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionDataType>
void
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionIdType,
                         SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (inherited::configuration_->streamConfiguration);
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!deviceHandle_);

//      SessionDataType& session_data_r =
//          const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      bool stop_device = false;
      size_t initial_buffer_size = 0;
      void* buffer_p = NULL;

      deviceHandle_ = inherited::configuration_->playbackDeviceHandle;
      if (deviceHandle_)
        isPassive_ = true;
      else
      {
//        std::string device_name_string = inherited::configuration_->device;
        std::string device_name_string =
            Stream_Module_Device_Tools::getALSADeviceName (SND_PCM_STREAM_PLAYBACK);
        if (device_name_string.empty ())
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Module_Device_Tools::getALSADeviceName(SND_PCM_STREAM_PLAYBACK), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF

        // *TODO*: remove type inference
//        int mode = MODULE_DEV_TARGET_ALSA_DEFAULT_MODE;
        int mode = 0;
        //    snd_spcm_init();
        result = snd_pcm_open (&deviceHandle_,
                               device_name_string.c_str (),
                               SND_PCM_STREAM_PLAYBACK, mode);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_open(\"%s\") for playback: \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (device_name_string.c_str ()),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: opened ALSA device (playback) \"%s\"...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (device_name_string.c_str ())));
      } // end ELSE
      ACE_ASSERT (deviceHandle_);

      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->format);
      if (!Stream_Module_Device_Tools::setFormat (deviceHandle_,
                                                  *inherited::configuration_->format))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(): \"%m\", aborting\n")));
        goto error;
      } // end IF

#if defined (_DEBUG)
      ACE_ASSERT (debugOutput_);
      result = snd_pcm_dump (deviceHandle_,
                             debugOutput_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_dump(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (inherited::configuration_->deviceName.c_str ()),
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
#endif

      if (useALSAAsynch_)
      {
        //  asynchCBData_.areas = areas;
        asynchCBData_.queue = inherited::msg_queue ();
        asynchCBData_.sampleSize =
            (snd_pcm_format_width (inherited::configuration_->format->format) / 8) *
            inherited::configuration_->format->channels;
        result =
            snd_async_add_pcm_handler (&asynchHandler_,
                                       deviceHandle_,
                                       stream_dev_target_alsa_async_callback,
                                       &asynchCBData_);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_async_add_pcm_handler(): \"%s\", aborting\n"),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: \"%s\": registered asynch PCM handler...\n"),
                    ACE_TEXT (inherited::mod_->name ()),
                    ACE_TEXT (snd_pcm_name (deviceHandle_))));
      } // end IF

//      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
//      {
//        // schedule regular statistic collection
//        ACE_ASSERT (inherited::timerID_ == -1);
//        ACE_Event_Handler* handler_p = &(inherited::statisticCollectionHandler_);
//        inherited::timerID_ =
//            COMMON_TIMERMANAGER_SINGLETON::instance ()->schedule_timer (handler_p,                                                                // event handler
//                                                                        NULL,                                                                     // argument
//                                                                        COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
//                                                                        inherited::configuration_->statisticCollectionInterval);                  // interval
//        if (inherited::timerID_ == -1)
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Common_Timer_Manager::schedule_timer(): \"%m\", aborting\n")));
//          goto error;
//        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("scheduled statistic collecting timer (ID: %d) for interval %#T...\n"),
//                    inherited::timerID_,
//                    &inherited::configuration_->statisticCollectionInterval));
//      } // end IF

//      if (!Stream_Module_Device_Tools::initializeBuffers<DataMessageType> (deviceHandle_,
//                                                                           inherited::configuration_->method,
//                                                                           inherited::configuration_->buffers,
//                                                                           bufferMap_,
//                                                                           inherited::configuration_->streamConfiguration->messageAllocator))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to Stream_Module_Device_Tools::initializeBuffers(%d): \"%m\", aborting\n"),
//                    captureFileDescriptor_));
//        goto error;
//      } // end IF

      if (useALSAAsynch_)
      {
        // *NOTE*: apparently, ALSA needs some initial data
        //         --> write one periods' worth of silence
        initial_buffer_size =
            inherited::configuration_->format->periodSize *
            asynchCBData_.sampleSize;
        buffer_p = malloc (initial_buffer_size);
        ACE_ASSERT (buffer_p);
        result =
            snd_pcm_format_set_silence (inherited::configuration_->format->format,
                                        buffer_p,
                                        inherited::configuration_->format->periodSize);
        ACE_ASSERT (result >= 0);
        result = snd_pcm_writei (deviceHandle_,
                                 buffer_p,
                                 inherited::configuration_->format->periodSize);
        ACE_ASSERT (result >= 0);
        free (buffer_p);
      } // end IF

      if (!useALSAAsynch_)
      {
        result =  snd_pcm_start (deviceHandle_);
        if (result < 0)
        {
          if (result == -EPIPE)
          {
            stop_device = true;

            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("buffer underrun, recovering\n")));

            //        result = snd_pcm_prepare (handle_p);
            result = snd_pcm_recover (deviceHandle_,
                                      -EPIPE,
                                      1);
            if (result < 0)
            {
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to snd_pcm_recover(): \"%s\", aborting\n"),
                          ACE_TEXT (snd_strerror (result))));
              goto error;
            } // end IF
          } // end IF
          else
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to snd_pcm_start(): \"%s\", aborting\n"),
                        ACE_TEXT (snd_strerror (result))));
            goto error;
          } // end IF
        } // end IF
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: \"%s\": started playback device...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (snd_pcm_name (deviceHandle_))));
      } // end IF
      stop_device = true;

//continue_:
      break;

error:
      if (stop_device)
      {
        result =  snd_pcm_drop (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_drop(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
      } // end IF

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
//      if (inherited::timerID_ != -1)
//      {
//        const void* act_p = NULL;
//        result =
//            COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel_timer (inherited::timerID_,
//                                                                      &act_p);
//        if (result == -1)
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", continuing\n"),
//                      inherited::timerID_));
//        inherited::timerID_ = -1;
//      } // end IF

      if (deviceHandle_)
      {
        result = snd_pcm_drop (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_drop(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: \"%s\": stopped playback device...\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_pcm_name (deviceHandle_))));

        result = snd_pcm_hw_free (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_hw_free(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
      } // end IF

      if (useALSAAsynch_)
      {
        result = snd_async_del_handler (asynchHandler_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_async_del_handler(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: \"%s\": deregistered asynch PCM handler...\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_pcm_name (deviceHandle_))));
      } // end IF

      if (!isPassive_)
        if (deviceHandle_)
        {
          result = snd_pcm_close (deviceHandle_);
          if (result < 0)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                        ACE_TEXT (snd_strerror (result))));
          else
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: closed ALSA device...\n"),
                        ACE_TEXT (inherited::mod_->name ())));
          deviceHandle_ = NULL;
        } // end IF

      if (debugOutput_)
      {
        result = snd_output_close (debugOutput_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
        debugOutput_ = NULL;
      } // end IF

      if (useALSAAsynch_)
      {
        QUEUE_T* message_queue_p = inherited::msg_queue ();
        ACE_ASSERT (message_queue_p);
        result = message_queue_p->deactivate ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        result = message_queue_p->flush ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::flush(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end IF
      else
      {
        if (inherited::thr_count_)
          inherited::stop (false); // wait ?
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
