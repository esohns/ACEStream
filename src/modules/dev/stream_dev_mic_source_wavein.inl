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

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_directshow_tools.h"

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
          typename TimerManagerType>
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
                               TimerManagerType>::Stream_Dev_Mic_Source_WaveIn_T (ISTREAM_T* stream_in)
 : inherited (stream_in,                            // stream handle
              false,                                // auto-start ?
              STREAM_HEADMODULECONCURRENCY_PASSIVE, // concurrency
              true)                                 // generate session messages ?
 , bufferHeaders_ ()
 , CBData_ ()
 , closeContext_ (false)
 , context_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::Stream_Dev_Mic_Source_WaveIn_T"));

  ACE_OS::memset (&bufferHeaders_, 0, sizeof (struct wavehdr_tag[STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS]));
  ACE_OS::memset (&CBData_, 0, sizeof (struct libacestream_wave_in_cbdata));
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
          typename TimerManagerType>
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
                               TimerManagerType>::~Stream_Dev_Mic_Source_WaveIn_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::~Stream_Dev_Mic_Source_WaveIn_T"));

  if (unlikely (closeContext_))
  { ACE_ASSERT (context_);
    MMRESULT result = waveInClose (context_);
    if (unlikely (result != MMSYSERR_NOERROR))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to waveInClose(%@): %d, continuing\n"),
                  inherited::mod_->name (),
                  result));
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
          typename TimerManagerType>
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
                               TimerManagerType>::initialize (const ConfigurationType& configuration_in,
                                                              Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (unlikely (closeContext_))
    { ACE_ASSERT (context_);
      MMRESULT result = waveInClose (context_);
      if (unlikely (result != MMSYSERR_NOERROR))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveInClose(%@): %d, continuing\n"),
                    inherited::mod_->name (),
                    result));
      closeContext_ = false;
      context_ = NULL;
    } // end IF
  } // end IF

  //UINT n = waveInGetNumDevs ();
  //printf ("%d\n", n);
  //WAVEINCAPS capabilities_s;
  //ACE_OS::memset (&capabilities_s, 0, sizeof (WAVEINCAPS));
  //MMRESULT result = MMSYSERR_NOERROR;
  //for (int c = 0;
  //     c <= n - 1;
  //     ++c)
  //{
  //  result = waveInGetDevCaps (c, &capabilities_s, sizeof (WAVEINCAPS));
  //  ACE_ASSERT (result == MMSYSERR_NOERROR);
  //  printf ("%d\n", capabilities_s.wMid);
  //  printf ("%d\n", capabilities_s.wPid);
  //  printf ("%s\n", capabilities_s.szPname);
  //  printf ("%d\n", capabilities_s.dwFormats);
  //  printf ("%d\n", capabilities_s.wChannels);
  //};

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
          typename TimerManagerType>
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
                               TimerManagerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                        bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::handleSessionMessage"));

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
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
#if defined (_DEBUG)
      std::string log_file_name;
#endif // _DEBUG

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

      //WAVEINCAPS capabilities_s;
      //struct tWAVEFORMATEX wave_format_ex_s;
      //ACE_OS::memset (&wave_format_ex_s, 0, sizeof (struct tWAVEFORMATEX));
      //MMRESULT result = waveInGetDevCaps (configuration_->audioInput,
      //                                    &capabilities_s,
      //                                    sizeof (WAVEINCAPS));
      //if (unlikely (result != MMSYSERR_NOERROR))
      //{
      //  waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to waveInGetDevCaps(%d): \"%s\", aborting\n"),
      //              inherited::mod_->name (),
      //              configuration_->audioInput,
      //              ACE_TEXT (error_msg_a)));
      //  goto error;
      //} // end IF

      //// attempt 44.1 kHz stereo if device is capable
      //if (capabilities_s.dwFormats & WAVE_FORMAT_4S16)
      //{
      //  wave_format_ex_s.nChannels = 2;          // stereo
      //  wave_format_ex_s.nSamplesPerSec = 44100; // 44.1 kHz (44.1 * 1000)
      //} // end IF
      //else
      //{
      //  wave_format_ex_s.nChannels = capabilities_s.wChannels; // use DevCaps # channels
      //  wave_format_ex_s.nSamplesPerSec = 22050;               // 22.05 kHz (22.05 * 1000)
      //} // end ELSE
      //wave_format_ex_s.wFormatTag = WAVE_FORMAT_PCM;
      //wave_format_ex_s.wBitsPerSample = 16;
      //wave_format_ex_s.nBlockAlign =
      //  wave_format_ex_s.nChannels * wave_format_ex_s.wBitsPerSample / 8;
      //wave_format_ex_s.nAvgBytesPerSec =
      //  wave_format_ex_s.nSamplesPerSec * wave_format_ex_s.nBlockAlign;
      //wave_format_ex_s.cbSize = 0;
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct _AMMediaType& media_type_r = session_data_r.formats.back ();
      ACE_ASSERT (media_type_r.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_r.subtype == MEDIASUBTYPE_PCM);
      ACE_ASSERT (media_type_r.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_r.pbFormat);
      ACE_ASSERT (inherited::configuration_->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::ID);
      result = waveInOpen (&context_,
                           inherited::configuration_->deviceIdentifier.identifier._id,
                           audio_info_p,
                           (DWORD)(VOID*)libacestream_wave_in_data_cb,
                           (DWORD)&CBData_,
                           CALLBACK_FUNCTION);
      if (unlikely (result != MMSYSERR_NOERROR))
      {
        waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveInOpen(%d): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    inherited::configuration_->deviceIdentifier.identifier._id,
                    ACE_TEXT (error_msg_a)));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened capture device (id: %d, handle: %@)\n"),
                  inherited::mod_->name (),
                  inherited::configuration_->deviceIdentifier.identifier._id,
                  &context_));

      CBData_.channels = audio_info_p->nChannels;
      CBData_.sampleRate = audio_info_p->nSamplesPerSec;
      CBData_.sampleSize =
        (CBData_.channels * (audio_info_p->wBitsPerSample / 8));

      // prepare buffer blocks and add to input queue
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
           i < STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS;
           ++i)
      {
        result = waveInPrepareHeader (context_,
                                      &bufferHeaders_[i],
                                      sizeof (struct wavehdr_tag));
        if (unlikely (result != MMSYSERR_NOERROR))
        {
          waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveInPrepareHeader(%d): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      i,
                      ACE_TEXT (error_msg_a)));
          goto error;
        } // end IF
        
         // add buffers to the input queue
         result = waveInAddBuffer (context_,
                                   &bufferHeaders_[i],
                                   sizeof (struct wavehdr_tag));
        if (unlikely (result != MMSYSERR_NOERROR))
        {
          waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to waveInAddBuffer(%d): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      i,
                      ACE_TEXT (error_msg_a)));
          goto error;
        } // end IF
      } // end FOR

      result = waveInStart (context_);
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
                  inherited::configuration_->deviceIdentifier.identifier._id,
                  &context_,
                  STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS));

      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: capture format: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (session_data_r.formats.back (), true).c_str ())));

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

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
        result = itimer_manager_p->cancel_timer (inherited::timerId_,
                                                 &act_p);
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Common_ITimer::cancel_timer(%d): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::timerId_));
        inherited::timerId_ = -1;
      } // end IF

      MMRESULT result = waveInReset (context_);
      if (unlikely (result != MMSYSERR_NOERROR))
      { waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to waveInReset(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (error_msg_a)));
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: stopped audio capture (handle: %@)\n"),
                  inherited::mod_->name (),
                  &context_));

      DataMessageType* message_p = NULL;
      for (unsigned int i = 0;
           i < STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS;
           ++i)
      {
        result = waveInUnprepareHeader (context_,
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

        if (likely (CBData_.buffers[i]))
        {
          message_p = static_cast<DataMessageType*> (CBData_.buffers[i]);
          ACE_ASSERT (message_p);
          typename DataMessageType::DATA_T& data_r =
            const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
          data_r.task = NULL;
          CBData_.buffers[i] = NULL;
        } // end IF
      } // end FOR

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
          typename TimerManagerType>
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
                               TimerManagerType>::collect (StatisticContainerType& data_out)
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
          typename TimerManagerType>
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
                               TimerManagerType>::set (const unsigned int index_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::set"));

  // sanity check(s)
  ACE_ASSERT (index_in < STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS);

  char         error_msg_a[BUFSIZ];

  if (CBData_.buffers[index_in]) // *NOTE*: stream may have already finished
    CBData_.buffers[index_in]->reset ();
  MMRESULT result = waveInAddBuffer (context_,
                                     &bufferHeaders_[index_in],
                                     sizeof (struct wavehdr_tag));
  if (unlikely (result != MMSYSERR_NOERROR))
  { ACE_OS::memset (&error_msg_a, 0, sizeof (char[BUFSIZ]));
    waveInGetErrorText (result, error_msg_a, BUFSIZ - 1);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to waveInAddBuffer(%d): \"%s\", returning\n"),
                inherited::mod_->name (),
                index_in,
                ACE_TEXT (error_msg_a)));
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
          typename TimerManagerType>
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
                               TimerManagerType>::allocateBuffers (Stream_IAllocator* allocator_in,
                                                                   unsigned int bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WaveIn_T::allocateBuffers"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);

  DataMessageType* message_p = NULL;

  for (unsigned int i = 0;
       i < STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS;
       ++i)
  {
    try {
      CBData_.buffers[i] = static_cast<DataMessageType*> (allocator_in->malloc (bufferSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  bufferSize_in));
      CBData_.buffers[i] = NULL;
      return false;
    }
    bufferHeaders_[i].dwBufferLength = bufferSize_in;
    bufferHeaders_[i].lpData = CBData_.buffers[i]->rd_ptr ();

    bufferHeaders_[i].dwUser = i;

    message_p = static_cast<DataMessageType*> (CBData_.buffers[i]);
    ACE_ASSERT (message_p);
    typename DataMessageType::DATA_T& data_r =
      const_cast<typename DataMessageType::DATA_T&> (message_p->getR ());
    data_r.task = this;
    data_r.index = i;
  } // end FOR

  return true;
}
