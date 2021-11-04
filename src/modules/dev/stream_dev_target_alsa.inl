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

// #include "common_file_tools.h"
// #include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_lib_alsa_tools.h"

#include "stream_dev_defines.h"

static void
stream_dev_target_alsa_async_callback (snd_async_handler_t* handler_in)
{
  // never wait for the queue
  static ACE_Time_Value no_wait_2 = ACE_OS::gettimeofday ();

  // sanity check(s)
  ACE_ASSERT(handler_in);
  struct Stream_Device_ALSA_Playback_AsynchCBData* data_p =
      reinterpret_cast<struct Stream_Device_ALSA_Playback_AsynchCBData*> (snd_async_handler_get_callback_private (handler_in));
  ACE_ASSERT (data_p);
  snd_pcm_t* handle_p = snd_async_handler_get_pcm (handler_in);
  ACE_ASSERT (handle_p);

  snd_pcm_sframes_t available_frames, frames_written = 0;
  int result = -1;
  snd_pcm_uframes_t frames_to_write = 0;

  do
  {
    available_frames = snd_pcm_avail_update (handle_p);
    if (unlikely (available_frames < 0))
    {
      // overrun ? --> recover
      if (likely (available_frames == -EPIPE))
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    if (available_frames == 0)
      return;

    if (!data_p->currentBuffer)
    {
      result = data_p->queue->dequeue (data_p->currentBuffer,
                                       &no_wait_2);
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        if (likely (error == ETIMEDOUT))
          return;
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
    if (unlikely (frames_written < 0))
    {
      // overrun ? --> recover
      if (likely (frames_written == -EPIPE))
        goto recover;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_writei(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    data_p->currentBuffer->rd_ptr (frames_written * data_p->sampleSize);

    if (!data_p->currentBuffer->length ())
    {
      data_p->currentBuffer->release (); data_p->currentBuffer = NULL;
    } // end IF

    continue;

recover:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("buffer underrun, recovering\n")));

    //        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (handle_p,
                              -EPIPE,
                              1);
    if (unlikely (result < 0))
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
    data_p->currentBuffer->release (); data_p->currentBuffer = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::Stream_Dev_Target_ALSA_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , asynchCBData_ ()
 , asynchHandler_ (NULL)
#if defined (_DEBUG)
 , debugOutput_ (NULL)
#endif // _DEBUG
 , deviceHandle_ (NULL)
 , isPassive_ (false)
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
 , sampleSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::Stream_Dev_Target_ALSA_T"));

  inherited::msg_queue (&queue_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::~Stream_Dev_Target_ALSA_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::~Stream_Dev_Target_ALSA_T"));

  int result = -1;

  if (inherited::configuration_ &&
      inherited::configuration_->ALSAConfiguration &&
      inherited::configuration_->ALSAConfiguration->asynch)
  {
    result = queue_.deactivate ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n")));
    result = queue_.flush ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", continuing\n")));

    if (asynchCBData_.currentBuffer)
      asynchCBData_.currentBuffer->release ();
  } // end IF

#if defined (_DEBUG)
  if (debugOutput_)
  {
    result = snd_output_close (debugOutput_);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
  } // end IF
#endif // _DEBUG

  if (!isPassive_ &&
      deviceHandle_)
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
          typename SessionDataType>
bool
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  { ACE_ASSERT(inherited::configuration_);
    ACE_ASSERT(inherited::configuration_->ALSAConfiguration);
    if (inherited::configuration_->ALSAConfiguration->asynch)
    {
      result = queue_.deactivate ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::deactivate(): \"%m\", aborting\n")));
        return false;
      } // end IF
      result = queue_.flush ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue::flush(): \"%m\", aborting\n")));
        return false;
      } // end IF

      if (asynchCBData_.currentBuffer)
      {
        asynchCBData_.currentBuffer->release (); asynchCBData_.currentBuffer = NULL;
      } // end IF
    } // end IF

#if defined (_DEBUG)
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

    if (!isPassive_ &&
        deviceHandle_)
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

  ACE_ASSERT (configuration_in.ALSAConfiguration);
  if (configuration_in.ALSAConfiguration->asynch)
  {
    result = queue_.activate ();
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
                             ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEFAULT_LOG_FILE),
                             ACE_TEXT_ALWAYS_CHAR ("w"));
  if (result < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_output_stdio_open(): \"%s\", continuing\n"),
                ACE_TEXT (snd_strerror (result))));
#endif // _DEBUG

  deviceHandle_ = configuration_in.ALSAConfiguration->handle;
  if (deviceHandle_)
    isPassive_ = true;

  return inherited::initialize (configuration_in,
                                allocator_in);

//error:
#if defined (_DEBUG)
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

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType>
void
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::handleDataMessage"));

  // sanity check(s)
  if (unlikely (!deviceHandle_))
    return;
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->ALSAConfiguration);

  int result = -1;
  ACE_Message_Block* message_block_p = message_inout;
  if (likely (inherited::configuration_->ALSAConfiguration->asynch))
  {
    message_block_p = message_inout->duplicate ();
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to DataMessageType::duplicate(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      return;
    } // end IF

    ACE_ASSERT (inherited::msg_queue_);
    result = inherited::msg_queue_->enqueue (message_block_p, NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue(): \"%m\", returning\n"),
                  inherited::mod_->name ()));
      message_block_p->release (); message_block_p =NULL;
      return;
    } // end IF
    message_block_p = NULL;

    return;
  } // end IF

  snd_pcm_sframes_t available_frames = 0, frames_written = 0;
  snd_pcm_uframes_t frames_to_write = 0;
  unsigned int offset = 0;
  unsigned int bytes_to_write = message_block_p->length ();

  do
  {
    available_frames = snd_pcm_avail_update (deviceHandle_);
    if (unlikely (available_frames < 0))
    {
      // overrun ? --> recover
      if (likely (available_frames == -EPIPE))
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
    if (unlikely (available_frames == 0))
    {
      result = snd_pcm_wait (deviceHandle_, -1);
      if (unlikely (result < 0))
      {
        // underrun ? --> recover
        if (likely (result == -EPIPE))
          goto recover;

        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_wait(): \"%s\", returning\n"),
                    ACE_TEXT (snd_strerror (result))));
        return;
      } // end IF

      continue;
    } // end IF

    frames_to_write = bytes_to_write / sampleSize_;
    frames_to_write =
        (frames_to_write > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                             : frames_to_write);
    frames_written = snd_pcm_writei (deviceHandle_,
                                     message_block_p->rd_ptr () + offset,
                                     frames_to_write);
    if (unlikely (frames_written < 0))
    {
      // overrun ? --> recover
      if (likely (frames_written == -EPIPE))
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_writei(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      return;
    } // end IF
    bytes_to_write -= (frames_written * sampleSize_);
    offset += (frames_written * sampleSize_);

    if (unlikely (bytes_to_write == 0))
    {
      message_block_p = message_block_p->cont ();
      if (likely (!message_block_p))
        break;
      bytes_to_write = message_block_p->length ();
      offset = 0;
    } // end IF

    continue;

recover:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("buffer underrun, recovering\n")));

//     result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (deviceHandle_,
                              -EPIPE,
                              1);
    if (unlikely (result < 0))
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
          typename SessionDataType>
void
Stream_Dev_Target_ALSA_T<ACE_SYNCH_USE,
                         TimePolicyType,
                         ConfigurationType,
                         ControlMessageType,
                         DataMessageType,
                         SessionMessageType,
                         SessionDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_ALSA_T::handleSessionMessage"));

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
      struct Stream_MediaFramework_ALSA_MediaType media_type_r =
        session_data_r.formats.back ();
      sampleSize_ =
          (snd_pcm_format_width (media_type_r.format) / 8) * media_type_r.channels;

      bool stop_device = false;
      size_t initial_buffer_size = 0;
      void* buffer_p = NULL;
      if (!isPassive_)
      { ACE_ASSERT (!deviceHandle_);
//        std::string device_name_string =
//            Stream_Device_Tools::getDeviceName (SND_PCM_STREAM_PLAYBACK);
//        if (device_name_string.empty ())
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s: failed to Stream_Device_Tools::getDeviceName(SND_PCM_STREAM_PLAYBACK), aborting\n"),
//                      inherited::mod_->name ()));
//          goto error;
//        } // end IF

        int mode = 0;
        // *TODO*: remove type inference
//         if (inherited::configuration_->ALSAConfiguration->asynch)
//           mode |= SND_PCM_ASYNC;
        //    snd_spcm_init();
        std::string device_identifier_string =
          inherited::configuration_->deviceIdentifier.identifier;
open:
        result = snd_pcm_open (&deviceHandle_,
                               device_identifier_string.c_str (),
                               SND_PCM_STREAM_PLAYBACK,
                               mode);
        if (result < 0)
        {
          if ((result == EBUSY) &&
              (device_identifier_string == inherited::configuration_->deviceIdentifier.identifier) &&
              (device_identifier_string != ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEVICE_DEFAULT)))
          {
            ACE_DEBUG ((LM_WARNING,
                        ACE_TEXT ("%s: failed to snd_pcm_open(\"%s\",%d) for playback: \"%s\", falling back\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (device_identifier_string.c_str ()),
                        mode,
                        ACE_TEXT (snd_strerror (result))));
            device_identifier_string =
              ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_DEVICE_DEFAULT);
            goto open;
          } // end IF
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_open(\"%s\",%d) for playback: \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (device_identifier_string.c_str ()),
                      mode,
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: opened ALSA device (playback) \"%s\"...\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (device_identifier_string.c_str ())));

        ACE_ASSERT (!inherited::configuration_->ALSAConfiguration->format);
        inherited::configuration_->ALSAConfiguration->format = &media_type_r;
        if (!Stream_MediaFramework_ALSA_Tools::setFormat (deviceHandle_,
                                                          *inherited::configuration_->ALSAConfiguration))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_ALSA_Tools::setFormat(\"%s\"), aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (device_identifier_string.c_str ())));
          inherited::configuration_->ALSAConfiguration->format = NULL;
          goto error;
        } // end IF
        inherited::configuration_->ALSAConfiguration->format = NULL;
      } // end IF
      ACE_ASSERT (deviceHandle_);

#if defined (_DEBUG)
      ACE_ASSERT (debugOutput_);
      result = snd_pcm_dump (deviceHandle_,
                             debugOutput_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to snd_pcm_dump(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT (inherited::mod_->name ()),
                    ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier.c_str ()),
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
#endif // _DEBUG

      if (inherited::configuration_->ALSAConfiguration->asynch)
      {
        //  asynchCBData_.areas = areas;
        asynchCBData_.queue = inherited::msg_queue_;
        asynchCBData_.sampleSize = sampleSize_;
        result =
            snd_async_add_pcm_handler (&asynchHandler_,
                                       deviceHandle_,
                                       stream_dev_target_alsa_async_callback,
                                       &asynchCBData_);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_async_add_pcm_handler(): \"%s\", aborting\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: \"%s\": registered asynch PCM handler...\n"),
                    ACE_TEXT (inherited::mod_->name ()),
                    ACE_TEXT (snd_pcm_name (deviceHandle_))));
      } // end IF

      // *NOTE*: apparently, ALSA needs some initial data
      //         --> write one periods' worth of silence
      initial_buffer_size =
        inherited::configuration_->ALSAConfiguration->periodSize * asynchCBData_.sampleSize;
      buffer_p = malloc (initial_buffer_size);
      ACE_ASSERT (buffer_p);
      result =
          snd_pcm_format_set_silence (media_type_r.format,
                                      buffer_p,
                                      inherited::configuration_->ALSAConfiguration->periodSize);
      ACE_ASSERT (result >= 0);
      result = snd_pcm_writei (deviceHandle_,
                               buffer_p,
                               inherited::configuration_->ALSAConfiguration->periodSize);
      ACE_ASSERT (result >= 0);
      free (buffer_p); buffer_p = NULL;

      result =  snd_pcm_start (deviceHandle_);
      if (result < 0)
      {
        if (result == -EPIPE)
        {
          stop_device = true;

          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("buffer underrun, recovering\n")));

//           result = snd_pcm_prepare (handle_p);
          result = snd_pcm_recover (deviceHandle_,
                                    -EPIPE,
                                    1);
          if (result < 0)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to snd_pcm_recover(): \"%s\", aborting\n"),
                        ACE_TEXT (inherited::mod_->name ()),
                        ACE_TEXT (snd_strerror (result))));
            goto error;
          } // end IF
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_start(): \"%s\", aborting\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
      } // end IF
      stop_device = true;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\": started playback device...\n"),
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
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_strerror (result))));
      } // end IF

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (deviceHandle_)
      {
        result = snd_pcm_drain (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to snd_pcm_drain(): \"%s\", continuing\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: \"%s\": stopped playback device...\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_pcm_name (deviceHandle_))));
      } // end IF

      if (inherited::configuration_->ALSAConfiguration->asynch &&
          asynchHandler_)
      {
        result = snd_async_del_handler (asynchHandler_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_async_del_handler(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: deregistered asynch PCM handler...\n"),
                      ACE_TEXT (inherited::mod_->name ())));
      } // end IF

      if (!isPassive_ &&
          deviceHandle_)
      {
        result = snd_pcm_hw_free (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_hw_free(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));

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

      if (inherited::configuration_->ALSAConfiguration->asynch)
      {
        result = inherited::msg_queue_->deactivate ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
        result = inherited::msg_queue_->flush ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ACE_Message_Queue::flush(): \"%m\", continuing\n"),
                      inherited::mod_->name ()));
      } // end IF
      else
      {
        if (inherited::thr_count_)
          inherited::stop (false,  // wait ?
                           false); // high priority ?
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
