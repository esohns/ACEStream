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

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

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
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
                               MediaType>::Stream_Dev_Mic_Source_WaveIn_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , bufferHeaders_ ()
 , CBData_ ()
 , handle_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::Stream_Dev_Mic_Source_WaveIn_T"));

  ACE_OS::memset (&bufferHeaders_, 0, sizeof (struct wavehdr_tag[STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS]));
  ACE_OS::memset (&CBData_, 0, sizeof (struct stream_dev_wavein_cbdata));
  CBData_.task = this;
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
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
                               MediaType>::~Stream_Dev_Mic_Source_WaveIn_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::~Stream_Dev_Mic_Source_WaveIn_T"));

  if (unlikely (handle_))
  {
    MMRESULT result = waveInClose (handle_);
    char error_msg_a[BUFSIZ];
    if (unlikely (result != MMSYSERR_NOERROR))
    { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to waveInClose(%@): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (error_msg_a)));
    } // end IF
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
          typename TimerManagerType,
          typename MediaType>
bool
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (unlikely (handle_))
    {
      MMRESULT result = waveInClose (handle_);
      char error_msg_a[BUFSIZ];
      if (unlikely (result != MMSYSERR_NOERROR))
      { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveInClose(%@): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (error_msg_a)));
      } // end IF
      handle_ = NULL;
    } // end IF
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
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->messageAllocator);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::sessionData_);
  typename DataMessageType::DATA_T& data_r =
    const_cast<typename DataMessageType::DATA_T&> (message_inout->getR ());
  ACE_ASSERT (data_r.index < STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS);
  ACE_ASSERT (CBData_.buffers[data_r.index] == message_inout);

  MMRESULT result = waveInUnprepareHeader (handle_,
                                           &bufferHeaders_[data_r.index],
                                           sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveInUnprepareHeader(%u): \"%s\", returning\n"),
                inherited::mod_->name (),
                data_r.index,
                ACE_TEXT (error_msg_a)));
    return;
  } // end IF

  CBData_.buffers[data_r.index] = NULL;
  DataMessageType* message_p = NULL;
  try {
    message_p =
      static_cast<DataMessageType*> (inherited::configuration_->messageAllocator->malloc (inherited::configuration_->allocatorConfiguration->defaultBufferSize));
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), returning\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    return;
  }
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory (was: %u bytes), returning\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    return;
  } // end IF
  typename DataMessageType::DATA_T& data_2 =
    const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
  data_2.index = data_r.index;
  CBData_.buffers[data_r.index] = message_p;
  data_r.index = -1;

  bufferHeaders_[data_2.index].lpData = message_p->wr_ptr ();
  ACE_ASSERT (bufferHeaders_[data_2.index].dwBufferLength == inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  ACE_ASSERT (bufferHeaders_[data_2.index].dwUser == data_2.index);

  result = waveInPrepareHeader (handle_,
                                &bufferHeaders_[data_2.index],
                                sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveInPrepareHeader(%d): \"%s\", returning\n"),
                inherited::mod_->name (),
                data_2.index,
                ACE_TEXT (error_msg_a)));
    return;
  } // end IF

  result = waveInAddBuffer (handle_,
                            &bufferHeaders_[data_2.index],
                            sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { char error_msg_a[BUFSIZ];
    waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveInAddBuffer(%d): \"%s\", returning\n"),
                inherited::mod_->name (),
                data_2.index,
                ACE_TEXT (error_msg_a)));
    return;
  } // end IF

  ++CBData_.inFlightBuffers;

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  message_inout->initialize (session_data_r.sessionId,
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename TimerManagerType,
          typename MediaType>
void
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::handleSessionMessage"));

  MMRESULT result = MMSYSERR_ERROR;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);
  char         error_msg_a[BUFSIZ];
  ACE_OS::memset (&error_msg_a, 0, sizeof (char[BUFSIZ]));

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
      ACE_ASSERT (inherited::sessionData_);

      if (inherited::configuration_->statisticCollectionInterval !=
          ACE_Time_Value::zero)
      {
        // schedule regular statistic collection
        ACE_ASSERT (inherited::timerId_ == -1);
        inherited::timerId_ =
          itimer_manager_p->schedule_timer (&(inherited::statisticHandler_),                                          // event handler handle
                                            NULL,                                                                     // asynchronous completion token
                                            COMMON_TIME_NOW + inherited::configuration_->statisticCollectionInterval, // first wakeup time
                                            inherited::configuration_->statisticCollectionInterval);                  // interval
        if (unlikely (inherited::timerId_ == -1))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::schedule_timer(): \"%m\", aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
//        ACE_DEBUG ((LM_DEBUG,
//                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
//                    inherited::mod_->name (),
//                    inherited::timerId_,
//                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      UINT device_id_i = -1;
      Stream_Device_Tools::id (inherited::configuration_->deviceIdentifier,
                               device_id_i);

      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      //ACE_ASSERT (media_type_s.subtype == MEDIASUBTYPE_PCM);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      ACE_ASSERT (audio_info_p);

      DWORD flags_u = CALLBACK_FUNCTION |
                      //WAVE_ALLOWSYNC    |
                      //WAVE_MAPPED_DEFAULT_COMMUNICATION_DEVICE |
                      WAVE_FORMAT_DIRECT;
                      //WAVE_MAPPED;
      result = waveInOpen (&handle_,
                           //WAVE_MAPPER,
                           device_id_i,
                           audio_info_p,
                           reinterpret_cast<DWORD_PTR> (stream_dev_wavein_data_cb),
                           reinterpret_cast<DWORD_PTR> (&CBData_),
                           flags_u);
      if (unlikely (result != MMSYSERR_NOERROR))
      { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveInOpen(%u): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    device_id_i,
                    ACE_TEXT (error_msg_a)));
        goto error;
      } // end IF
      ACE_ASSERT (handle_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened device (id: %u, format: %s)...\n"),
                  inherited::mod_->name (),
                  device_id_i,
                  ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      // prepare buffer blocks / add to input queue
      if (!allocateBuffers (inherited::configuration_->messageAllocator,
                            inherited::configuration_->allocatorConfiguration->defaultBufferSize))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to allocate buffers (%u), aborting\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->allocatorConfiguration->defaultBufferSize));
        goto error;
      } // end IF

      for (unsigned int i = 0;
           i < STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS;
           ++i)
      {
        result = waveInPrepareHeader (handle_,
                                      &bufferHeaders_[i],
                                      sizeof (struct wavehdr_tag));
        if (unlikely (result != MMSYSERR_NOERROR))
        { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveInPrepareHeader(%u): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      i,
                      ACE_TEXT (error_msg_a)));
          goto error;
        } // end IF

        // add buffers to the input queue
        result = waveInAddBuffer (handle_,
                                  &bufferHeaders_[i],
                                  sizeof (struct wavehdr_tag));
        if (unlikely (result != MMSYSERR_NOERROR))
        { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveInAddBuffer(%u): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      i,
                      ACE_TEXT (error_msg_a)));
          goto error;
        } // end IF

        ++CBData_.inFlightBuffers;
      } // end FOR

      result = waveInStart (handle_);
      if (unlikely (result != MMSYSERR_NOERROR))
      { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveInStart(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (error_msg_a)));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: started audio capture (id: %u, handle: %@, %u buffer(s))\n"),
                  inherited::mod_->name (),
                  device_id_i,
                  &handle_,
                  STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS));

      break;

error:
      if (handle_)
        waveInReset (handle_);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // *NOTE*: only process the first 'session end' message
      { ACE_GUARD (ACE_Thread_Mutex, aGuard, inherited::lock_);
        if (inherited::sessionEndProcessed_)
          break; // done
        inherited::sessionEndProcessed_ = true;
      } // end lock scope

      if (likely (inherited::timerId_ != -1))
      {
        const void* act_p = NULL;
        int result_2 = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                       &act_p);
        if (unlikely (result_2 == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      if (likely (handle_))
      {
        result = waveInReset (handle_);
        if (unlikely (result != MMSYSERR_NOERROR))
        { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveInReset(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (error_msg_a)));
        } // end IF
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: stopped audio capture (handle: %@)\n"),
                      inherited::mod_->name (),
                      &handle_));
      } // end IF

      while (CBData_.inFlightBuffers)
        ACE_OS::sleep (ACE_Time_Value (1, 0));
      unsigned int flushed_buffers_i = inherited::queue_.flush (false);
      ACE_ASSERT (flushed_buffers_i == STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS);

      DataMessageType* message_p = NULL;
      for (unsigned int i = 0;
           i < STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS;
           ++i)
      {
        if (likely (handle_))
        {
          result = waveInUnprepareHeader (handle_,
                                          &bufferHeaders_[i],
                                          sizeof (struct wavehdr_tag));
          if (unlikely (result != MMSYSERR_NOERROR))
          { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to waveInUnprepareHeader(%u): \"%s\", continuing\n"),
                        inherited::mod_->name (),
                        i,
                        ACE_TEXT (error_msg_a)));
          } // end IF
        } // end IF

        CBData_.buffers[i] = NULL;
      } // end FOR

      if (likely (handle_))
      {
        result = waveInClose (handle_);
        if (unlikely (result != MMSYSERR_NOERROR))
        { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveInClose(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (error_msg_a)));
        } // end IF
        else
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: closed device...\n"),
                      inherited::mod_->name ()));
        handle_ = NULL;
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
bool
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
                               MediaType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::collect"));

  // sanity check(s)
  ACE_ASSERT (inherited::isInitialized_);

  // step0: initialize container
  data_out.timeStamp = COMMON_TIME_NOW;

  //// step2: send the information downstream
  //if (!inherited::putStatisticMessage (data_out)) // data container
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_HeadModuleTaskBase_T::putStatisticMessage(), aborting\n")));
  //              inherited::mod_->name ()));
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
//Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::report"));
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
//          typename StatisticContainerType,
//          typename TimerManagerType,
//          typename MediaType>
//void
//Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
//                               ControlMessageType,
//                               DataMessageType,
//                               SessionMessageType,
//                               ConfigurationType,
//                               StreamControlType,
//                               StreamNotificationType,
//                               StreamStateType,
//                               SessionDataType,
//                               SessionDataContainerType,
//                               StatisticContainerType,
//                               TimerManagerType,
//                               MediaType>::set (const unsigned int index_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::set"));
//
//  // sanity check(s)
//  ACE_ASSERT (index_in < STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS);
//
//  if (CBData_.buffers[index_in]) // *NOTE*: stream may have already finished
//    CBData_.buffers[index_in]->reset ();
//
//  MMRESULT result = waveInAddBuffer (handle_,
//                                     &bufferHeaders_[index_in],
//                                     sizeof (struct wavehdr_tag));
//  if (unlikely (result != MMSYSERR_NOERROR))
//  { char error_msg_a[BUFSIZ];
//    waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to waveInAddBuffer(%d): \"%s\", returning\n"),
//                inherited::mod_->name (),
//                index_in,
//                ACE_TEXT (error_msg_a)));
//  } // end IF
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
          typename StatisticContainerType,
          typename TimerManagerType,
          typename MediaType>
bool
Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
                               MediaType>::allocateBuffers (Stream_IAllocator* allocator_in,
                                                            unsigned int bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::allocateBuffers"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);

  DataMessageType* message_p = NULL;

  for (unsigned int i = 0;
       i < STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS;
       ++i)
  {
    try {
      message_p =
        static_cast<DataMessageType*> (allocator_in->malloc (bufferSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  inherited::mod_->name (),
                  bufferSize_in));
      return false;
    }
    if (unlikely (!message_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate memory (was: %u bytes), aborting\n"),
                  inherited::mod_->name (),
                  bufferSize_in));
      return false;
    } // end IF
    CBData_.buffers[i] = message_p;

    bufferHeaders_[i].lpData = CBData_.buffers[i]->wr_ptr ();
    bufferHeaders_[i].dwBufferLength = bufferSize_in;
    bufferHeaders_[i].dwUser = i;

    message_p = static_cast<DataMessageType*> (CBData_.buffers[i]);
    ACE_ASSERT (message_p);
    typename DataMessageType::DATA_T& data_r =
      const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
    data_r.index = i;
  } // end FOR

  return true;
}
