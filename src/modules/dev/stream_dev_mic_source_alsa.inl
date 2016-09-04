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
stream_dev_mic_source_alsa_async_callback (snd_async_handler_t* handler_in)
{
  // sanity check(s)
  ACE_ASSERT (handler_in);

  Stream_Module_Device_ALSA_Capture_AsynchCBData* data_p =
      reinterpret_cast<Stream_Module_Device_ALSA_Capture_AsynchCBData*> (snd_async_handler_get_callback_private (handler_in));
  snd_pcm_t* handle_p = snd_async_handler_get_pcm (handler_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (handle_p);

  snd_pcm_sframes_t available_frames, frames_read = 0;
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  snd_pcm_uframes_t frames_to_read;

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

    if (!message_block_p)
    {
      if (data_p->allocator)
      {
        try {
          message_block_p =
              static_cast<ACE_Message_Block*> (data_p->allocator->malloc (data_p->bufferSize));
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                      data_p->bufferSize));
          message_block_p = NULL;
        }
      } // end IF
      else
        ACE_NEW_NORETURN (message_block_p,
                          ACE_Message_Block (data_p->bufferSize));
      if (!message_block_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory, aborting\n")));
        goto error;
      } // end IF
    } // end IF

    frames_to_read = message_block_p->size () / data_p->sampleSize;
    frames_to_read =
        (frames_to_read > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                            : frames_to_read);
    frames_read = snd_pcm_readi (handle_p,
                                 message_block_p->wr_ptr (),
                                 frames_to_read);
    if (frames_read < 0)
    {
      // overrun ? --> recover
      if (frames_read == -EPIPE)
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_readi(): \"%s\", returning\n"),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    message_block_p->wr_ptr (frames_read * data_p->sampleSize);

    result = data_p->queue->enqueue (message_block_p,
                                     NULL);
    if (result < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::enqueue(): \"%m\", returning\n")));
      goto error;
    } // end IF
    message_block_p = NULL;

    continue;

recover:
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("buffer overrun, recovering\n")));

    //        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (handle_p,
                              -EPIPE,
                              1);
    if (result < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_prepare(): \"%s\", returning\n"),
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
          typename StatisticContainerType>
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
                             StatisticContainerType>::Stream_Dev_Mic_Source_ALSA_T (ACE_SYNCH_MUTEX_T* lock_in,
                                                                                    bool autoStart_in)
 : inherited (lock_in,      // lock handle
              autoStart_in, // auto-start ?
              true)         // generate session messages ?
 , asynchCBData_ ()
 , asynchHandler_ (NULL)
 , debugOutput_ (NULL)
 , deviceHandle_ (NULL)
 , isPassive_ (false)
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
          typename StatisticContainerType>
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
                             StatisticContainerType>::~Stream_Dev_Mic_Source_ALSA_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::~Stream_Dev_Mic_Source_ALSA_T"));

  int result = -1;

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
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType>
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
                             StatisticContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::initialize"));

  int result = -1;
  bool result_2 = false;

  if (inherited::isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

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

#if defined (_DEBUG)
  result =
      snd_output_stdio_open (&debugOutput_,
                             ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_LOG_FILE),
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

  result_2 = inherited::initialize (configuration_in);
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

//template <typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_ALSA_T<SessionMessageType,
//                           ProtocolMessageType,
//                           ConfigurationType,
//                           StreamStateType,
//                           SessionDataType,
//                           SessionDataContainerType,
//                           StatisticContainerType>::handleDataMessage (ProtocolMessageType*& message_inout,
//                                                                       bool& passMessageDownstream_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::handleDataMessage"));

//  // sanity check(s)
//  ACE_ASSERT (message_inout);
//  ACE_ASSERT (isInitialized_);
//}

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
          typename StatisticContainerType>
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
                             StatisticContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::handleSessionMessage"));

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

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());

      bool stop_device = false;
      int signal = 0;

      deviceHandle_ = inherited::configuration_->captureDeviceHandle;
      if (deviceHandle_)
        isPassive_ = true;
      else
      {
        // *TODO*: remove type inference
        //  int mode = MODULE_DEV_MIC_ALSA_DEFAULT_MODE;
        int mode = 0;
        //    snd_spcm_init();
        result = snd_pcm_open (&deviceHandle_,
                               inherited::configuration_->device.c_str (),
                               SND_PCM_STREAM_CAPTURE, mode);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_open(\"%s\"): \"%s\", aborting\n"),
                      ACE_TEXT (inherited::configuration_->device.c_str ()),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: opened ALSA device (capture) \"%s\"...\n"),
                    ACE_TEXT (inherited::mod_->name ()),
                    ACE_TEXT (inherited::configuration_->device.c_str ())));

        // *TODO*: remove type inference
        ACE_ASSERT (inherited::configuration_->format);
        if (!Stream_Module_Device_Tools::setFormat (deviceHandle_,
                                                    *inherited::configuration_->format))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(): \"%m\", aborting\n")));
          goto error;
        } // end IF
      } // end ELSE
      ACE_ASSERT (deviceHandle_);

#if defined (_DEBUG)
      ACE_ASSERT (debugOutput_);
      result = snd_pcm_dump (deviceHandle_,
                             debugOutput_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_dump(\"%s\"): \"%s\", continuing\n"),
                    ACE_TEXT (inherited::configuration_->device.c_str ()),
                    ACE_TEXT (snd_strerror (result))));
      result = snd_pcm_dump_setup (deviceHandle_,
                                   debugOutput_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_pcm_dump_setup(\"%s\"): \"%s\", continuing\n"),
                    ACE_TEXT (inherited::configuration_->device.c_str ()),
                    ACE_TEXT (snd_strerror (result))));
//      result = snd_pcm_dump_hw_setup (deviceHandle_,
//                                      debugOutput_);
//      if (result < 0)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to snd_pcm_dump_hw_setup(\"%s\"): \"%s\", continuing\n"),
//                    ACE_TEXT (inherited::configuration_->device.c_str ()),
//                    ACE_TEXT (snd_strerror (result))));
//      result = snd_pcm_dump_sw_setup (deviceHandle_,
//                                      debugOutput_);
//      if (result < 0)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to snd_pcm_dump_sw_setup(\"%s\"): \"%s\", continuing\n"),
//                    ACE_TEXT (inherited::configuration_->device.c_str ()),
//                    ACE_TEXT (snd_strerror (result))));
#endif

      asynchCBData_.allocator = inherited::configuration_->messageAllocator;
    //  asynchCBData_.areas = areas;
      asynchCBData_.bufferSize = inherited::configuration_->bufferSize;
      asynchCBData_.queue = inherited::msg_queue ();
      asynchCBData_.sampleSize =
          (snd_pcm_format_width (inherited::configuration_->format->format) / 8) *
          inherited::configuration_->format->channels;
      result =
          snd_async_add_pcm_handler (&asynchHandler_,
                                     deviceHandle_,
                                     stream_dev_mic_source_alsa_async_callback,
                                     &asynchCBData_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to snd_async_add_pcm_handler(): \"%s\", aborting\n"),
                    ACE_TEXT (snd_strerror (result))));
        goto error;
      } // end IF
      signal = snd_async_handler_get_signo (asynchHandler_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\": registered asynch PCM handler (signal: %d: \"%S\")...\n"),
                  ACE_TEXT (inherited::mod_->name ()),
                  ACE_TEXT (snd_pcm_name (deviceHandle_)),
                  signal,
                  signal));

      // *TODO*: remove type inference
//      if (inherited::configuration_->format)
//        if (!Stream_Module_Device_Tools::setCaptureFormat (deviceHandle_,
//                                                           *inherited::configuration_->format))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(): \"%m\", aborting\n")));
//          goto error;
//        } // end IF

      session_data_r.format = *inherited::configuration_->format;

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

      if (deviceHandle_)
      {
        result =  snd_pcm_start (deviceHandle_);
        if (result < 0)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_start(): \"%s\", aborting\n"),
                      ACE_TEXT (snd_strerror (result))));
          goto error;
        } // end IF
        stop_device = true;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: \"%s\": started capture device...\n"),
                  ACE_TEXT (inherited::mod_->name ()),
                  ACE_TEXT (snd_pcm_name (deviceHandle_))));

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
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: \"%s\": stopped capture device...\n"),
                      ACE_TEXT (inherited::mod_->name ()),
                      ACE_TEXT (snd_pcm_name (deviceHandle_))));

        result = snd_pcm_hw_free (deviceHandle_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_pcm_hw_free(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
      } // end IF

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

      if (debugOutput_)
      {
        result = snd_output_close (debugOutput_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to snd_output_close(): \"%s\", continuing\n"),
                      ACE_TEXT (snd_strerror (result))));
        debugOutput_ = NULL;
      } // end IF

      inherited::shutdown ();

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
          typename StatisticContainerType>
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
                             StatisticContainerType>::collect (StatisticContainerType& data_out)
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

//template <ACE_SYNCH_DECL,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//void
//Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::report"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP;
//  ACE_NOTREACHED (return;)
//}

//template <ACE_SYNCH_DECL,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename ConfigurationType,
//          typename StreamControlType,
//          typename StreamNotificationType,
//          typename StreamStateType,
//          typename SessionDataType,
//          typename SessionDataContainerType,
//          typename StatisticContainerType>
//int
//Stream_Dev_Mic_Source_ALSA_T<ACE_SYNCH_USE,
//                             ControlMessageType,
//                             DataMessageType,
//                             SessionMessageType,
//                             ConfigurationType,
//                             StreamControlType,
//                             StreamNotificationType,
//                             StreamStateType,
//                             SessionDataType,
//                             SessionDataContainerType,
//                             StatisticContainerType>::svc (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_ALSA_T::svc"));

//  // sanity check(s)
//  ACE_ASSERT (inherited::mod_);
//  ACE_ASSERT (inherited::sessionData_);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("\"%s\": worker thread (ID: %t) starting...\n"),
//              inherited::mod_->name ()));

//  int error = 0;
//  bool has_finished = false;
//  ACE_Message_Block* message_block_p = NULL;
//  ACE_Time_Value no_wait = COMMON_TIME_NOW;
//  bool release_lock = false;
//  int result = -1;
//  int result_2 = -1;
//  const SessionDataType& session_data_r = inherited::sessionData_->get ();
//  bool stop_processing = false;

//  struct v4l2_buffer buffer;
//  ACE_OS::memset (&buffer, 0, sizeof (struct v4l2_buffer));
//  buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
//  buffer.memory = V4L2_MEMORY_USERPTR;
//  struct v4l2_event event;
//  ACE_OS::memset (&event, 0, sizeof (struct v4l2_event));
//  Stream_Module_Device_BufferMapIterator_t iterator;
////  unsigned int queued, done = 0;

//  // step1: start processing data
//  do
//  {
//    message_block_p = NULL;
//    result_2 = inherited::getq (message_block_p,
//                                &no_wait);
//    if (result_2 == -1)
//    {
//      error = ACE_OS::last_error ();
//      if (error != EWOULDBLOCK) // Win32: 10035
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));

//        if (!has_finished)
//        {
//          has_finished = true;
//          // enqueue(/process) STREAM_SESSION_END
//          inherited::finished ();
//        } // end IF

//        break;
//      } // end IF

//      goto continue_;
//    } // end IF
//    ACE_ASSERT (message_block_p);

//    switch (message_block_p->msg_type ())
//    {
//      case ACE_Message_Block::MB_STOP:
//      {
//        // clean up
//        message_block_p->release ();
//        message_block_p = NULL;

//        // *NOTE*: when close()d manually (i.e. user abort), 'finished' will
//        //         not have been set at this stage

//        // signal the controller ?
//        if (!has_finished)
//        {
//          has_finished = true;
//          // enqueue(/process) STREAM_SESSION_END
//          inherited::finished ();

//          // has STREAM_SESSION_END been processed ? --> done
//          if (!inherited::thr_count_ && !inherited::runSvcOnStart_) goto done;

//          continue; // process STREAM_SESSION_END
//        } // end IF

//done:
//        result = 0;

//        goto done_2; // STREAM_SESSION_END has been processed
//      }
//      default:
//      {
//        // sanity check(s)
//        ACE_ASSERT (inherited::configuration_);
//        ACE_ASSERT (inherited::configuration_->ilock);

//        // grab lock if processing is 'non-concurrent'
//        if (!inherited::concurrent_)
//          release_lock = inherited::configuration_->ilock->lock (true);

//        inherited::handleMessage (message_block_p,
//                                  stop_processing);

//        if (release_lock) inherited::configuration_->ilock->unlock (false);

//        // finished ?
//        if (stop_processing)
//        {
//          // *IMPORTANT NOTE*: message_block_p has already been released() !

//          if (!has_finished)
//          {
//            has_finished = true;
//            // enqueue(/process) STREAM_SESSION_END
//            inherited::finished ();
//          } // end IF

//          continue;
//        } // end IF

//        break;
//      }
//    } // end SWITCH

//continue_:
//    // session aborted ?
//    // sanity check(s)
//    // *TODO*: remove type inferences
//    ACE_ASSERT (session_data_r.lock);
//    {
//      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_r.lock, result);

//      if (session_data_r.aborted &&
//          !has_finished)
//      {
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("session aborted\n")));

//        has_finished = true;
//        // enqueue(/process) STREAM_SESSION_END
//        inherited::finished ();
//      } // end IF
//    } // end lock scope

//#if defined (_DEBUG)
//    // log device status to kernel log ?
//    if (debug_)
//    {
//      result = v4l2_ioctl (captureFileDescriptor_,
//                           VIDIOC_LOG_STATUS);
//      if (result == -1)
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
//                    captureFileDescriptor_, ACE_TEXT ("VIDIOC_LOG_STATUS")));
//    } // end IF
//#endif

////    // dequeue pending events
////    result = v4l2_ioctl (captureFileDescriptor_,
////                         VIDIOC_DQEVENT,
////                         &event);
////    if (result == -1)
////    {
////      ACE_DEBUG ((LM_ERROR,
////                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
////                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
////    } // end IF
////    else
////    {
////      for (unsigned int i = 0;
////           i < event.pending;
////           ++i)
////      {
////        result = v4l2_ioctl (captureFileDescriptor_,
////                             VIDIOC_DQEVENT,
////                             &event);
////        if (result == -1)
////          ACE_DEBUG ((LM_ERROR,
////                      ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", continuing\n"),
////                      captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQEVENT")));
////      } // end FOR
////    } // end ELSE

////    queued =
////        Stream_Module_Device_Tools::queued (captureFileDescriptor_,
////                                            inherited::configuration_->buffers,
////                                            done);
////    ACE_DEBUG ((LM_DEBUG,
////                ACE_TEXT ("#queued/done buffers: %u/%u...\n"),
////                queued, done));

//    // *NOTE*: blocks until:
//    //         - a buffer is available
//    //         - a frame has been written by the device
//    result_2 = v4l2_ioctl (captureFileDescriptor_,
//                           VIDIOC_DQBUF,
//                           &buffer);
//    if (result_2 == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to v4l2_ioctl(%d,%s): \"%m\", aborting\n"),
//                  captureFileDescriptor_, ACE_TEXT ("VIDIOC_DQBUF")));
//      break;
//    } // end IF
//    if (buffer.flags & V4L2_BUF_FLAG_ERROR)
//      ACE_DEBUG ((LM_WARNING,
//                  ACE_TEXT ("%s: streaming error (fd: %d, index: %d), continuing\n"),
//                  inherited::mod_->name (),
//                  captureFileDescriptor_, buffer.index));

//    iterator = bufferMap_.find (buffer.index);
//    ACE_ASSERT (iterator != bufferMap_.end ());
//    message_block_p = (*iterator).second;
//    message_block_p->reset ();
//    message_block_p->wr_ptr (buffer.bytesused);

//    result_2 = inherited::put_next (message_block_p, NULL);
//    if (result_2 == -1)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE_Task::put_next(): \"%m\", aborting\n")));

//      // clean up
//      message_block_p->release ();

//      break;
//    } // end IF
//  } while (true);
//  result = -1;

//done_2:
//  return result;
//}
