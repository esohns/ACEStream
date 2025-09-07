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

#include "avrt.h"
#include "mmdeviceapi.h"

#include "ace/Log_Msg.h"

#include "common_os_tools.h"

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"

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
          typename MediaType>
Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                               MediaType>::Stream_Dev_Mic_Source_WASAPI_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , audioClient_ (NULL)
 , audioCaptureClient_ (NULL)
 , event_ (NULL)
 , frameSize_ (0)
 , task_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::Stream_Dev_Mic_Source_WASAPI_T"));

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
          typename MediaType>
Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                               MediaType>::~Stream_Dev_Mic_Source_WASAPI_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::~Stream_Dev_Mic_Source_WASAPI_T"));

  if (event_)
    CloseHandle (event_);
  if (audioCaptureClient_)
    audioCaptureClient_->Release ();
  if (audioClient_)
    audioClient_->Release ();
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
          typename MediaType>
bool
Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    if (task_)
    {
      if (!AvRevertMmThreadCharacteristics (task_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to AvRevertMmThreadCharacteristics(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (GetLastError (), false, false).c_str ())));
      task_ = NULL;
    } // end IF
    if (event_)
    {
      if (!CloseHandle (event_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CloseHandle(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      event_ = NULL;
    } // end IF
    if (audioCaptureClient_)
    {
      audioCaptureClient_->Release (); audioCaptureClient_ = NULL;
    } // end IF
    if (audioClient_)
    {
      audioClient_->Release (); audioClient_ = NULL;
    } // end IF
    frameSize_ = 0;
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename MediaType>
void
Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  typename TimerManagerType::INTERFACE_T* itimer_manager_p =
    (inherited::configuration_->timerManager ? inherited::configuration_->timerManager
                                             : inherited::TIMER_MANAGER_SINGLETON_T::instance ());
  ACE_ASSERT (itimer_manager_p);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);

      if (inherited::configuration_->statisticCollectionInterval != ACE_Time_Value::zero)
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
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scheduled statistic collecting timer (id: %d) for interval %#T\n"),
                    inherited::mod_->name (),
                    inherited::timerId_,
                    &inherited::configuration_->statisticCollectionInterval));
      } // end IF

      // determine media type
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      frameSize_ = audio_info_p->nChannels * (audio_info_p->wBitsPerSample / 8);
      ACE_ASSERT (frameSize_ == audio_info_p->nBlockAlign);

      REFERENCE_TIME requested_duration_i = 0;
      enum _AUDCLNT_SHAREMODE share_mode_e =
        STREAM_LIB_WASAPI_CAPTURE_DEFAULT_SHAREMODE;
      IMMDevice* device_p = NULL;
      UINT32 number_of_buffer_frames_i = 0;
      DWORD stream_flags_i =
        (AUDCLNT_STREAMFLAGS_EVENTCALLBACK      |
         //AUDCLNT_STREAMFLAGS_NOPERSIST         |
         /////////////////////////////////
         AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED |
         AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED);
      HANDLE task_h = NULL;
      DWORD task_index_i = 0;
      struct tWAVEFORMATEX* audio_info_2 = NULL;

      if (InlineIsEqualGUID (inherited::configuration_->deviceIdentifier.identifier._guid, GUID_NULL))
      {
        IMMDeviceEnumerator* enumerator_p = NULL;
        result_2 =
          CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL, CLSCTX_ALL,
                            IID_PPV_ARGS (&enumerator_p));
        ACE_ASSERT (SUCCEEDED (result_2) && enumerator_p);
        result_2 = enumerator_p->GetDefaultAudioEndpoint (eCapture,
                                                          eMultimedia,
                                                          &device_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMMDeviceEnumerator::GetDefaultAudioEndpoint(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
          enumerator_p->Release (); enumerator_p = NULL;
          goto error;
        } // end IF
        ACE_ASSERT (device_p);
        enumerator_p->Release (); enumerator_p = NULL;
        goto continue_;
      } // end IF
      device_p =
        Stream_MediaFramework_DirectSound_Tools::getDevice (inherited::configuration_->deviceIdentifier.identifier._guid);
      if (unlikely (!device_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to retrieve device handle (id was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_OS_Tools::GUIDToString (inherited::configuration_->deviceIdentifier.identifier._guid).c_str ())));
        goto error;
      } // end IF
continue_:
      ACE_ASSERT (device_p);

      ACE_ASSERT (!audioClient_);
      result_2 = device_p->Activate (__uuidof (IAudioClient), CLSCTX_ALL,
                                     NULL, (void**)&audioClient_);
      ACE_ASSERT (SUCCEEDED (result_2) && audioClient_);

      // sanity check(s)
      result_2 =
        audioClient_->IsFormatSupported (share_mode_e,
                                         audio_info_p,
                                         ((share_mode_e == AUDCLNT_SHAREMODE_EXCLUSIVE) ? NULL : &audio_info_2));
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IAudioClient::IsFormatSupported(%d,%s), aborting\n"),
                    inherited::mod_->name (),
                    share_mode_e,
                    ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ())));
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      else if (unlikely ((result_2 == S_FALSE) && audio_info_2))
      {
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: closest format matched is: \"%s\" (was: \"%s\"), continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_2, true).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ())));
        struct _AMMediaType media_type_2;
        ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
        if (unlikely (!Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx (*audio_info_2,
                                                                                 media_type_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::fromWaveFormatEx(), aborting\n"),
                      inherited::mod_->name ()));
          device_p->Release (); device_p = NULL;
          CoTaskMemFree (audio_info_2); audio_info_2 = NULL;
          goto error;
        } // end IF
        MediaType media_type_3;
        ACE_OS::memset (&media_type_3, 0, sizeof (MediaType));
        inherited2::getMediaType (media_type_2,
                                  STREAM_MEDIATYPE_AUDIO,
                                  media_type_3);
        session_data_r.formats.push_back (media_type_3);
        Stream_MediaFramework_DirectShow_Tools::free (media_type_2);
        audio_info_p = audio_info_2;
      } // end ELSE IF

      result_2 = audioClient_->GetDevicePeriod (&requested_duration_i, NULL);
      ACE_ASSERT (SUCCEEDED (result_2) && requested_duration_i);
retry:
      result_2 =
        audioClient_->Initialize (share_mode_e,//AUDCLNT_SHAREMODE_SHARED,
                                  stream_flags_i,
                                  requested_duration_i,
                                  ((share_mode_e == AUDCLNT_SHAREMODE_EXCLUSIVE) ? requested_duration_i 
                                                                                 : 0),
                                  audio_info_p,
                                  NULL);
      if (unlikely (FAILED (result_2))) // AUDCLNT_E_UNSUPPORTED_FORMAT: 0x88890008
      {
        if (unlikely (result_2 == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED))
        {
          result_2 = audioClient_->GetBufferSize (&number_of_buffer_frames_i);
          ACE_ASSERT (SUCCEEDED (result_2) && number_of_buffer_frames_i);
          audioClient_->Release (); audioClient_ = NULL;
          requested_duration_i =
            (REFERENCE_TIME)((10000.0 * 1000 / audio_info_p->nSamplesPerSec * number_of_buffer_frames_i) + 0.5);
          result_2 = device_p->Activate (__uuidof (IAudioClient), CLSCTX_ALL,
                                         NULL, (void**)&audioClient_);
          ACE_ASSERT (SUCCEEDED (result_2) && audioClient_);
          goto retry;
        } // end IF
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IAudioClient::Initialize(%d,%d,%q,%q,%s): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    share_mode_e,
                    stream_flags_i,
                    requested_duration_i,
                    ((share_mode_e == AUDCLNT_SHAREMODE_EXCLUSIVE) ? requested_duration_i : 0),
                    ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        if (audio_info_2)
        {
          CoTaskMemFree (audio_info_2); audio_info_2 = NULL;
        } // end IF
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened device \"%s\": format: %s...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToString (inherited::configuration_->deviceIdentifier.identifier._guid).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ())));
      if (audio_info_2)
      {
        CoTaskMemFree (audio_info_2); audio_info_2 = NULL;
      } // end IF
      device_p->Release (); device_p = NULL;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      ACE_ASSERT (!event_);
      event_ = CreateEvent (NULL,  // lpEventAttributes
                            FALSE, // bManualReset
                            FALSE, // bInitialState
                            NULL); // lpName
      if (unlikely (!event_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CreateEvent(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result_2 = audioClient_->SetEventHandle (event_);
      if (unlikely (!SUCCEEDED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IAudioClient::SetEventHandle(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
        goto error;
      } // end IF

      ACE_ASSERT (!audioCaptureClient_);
      result_2 = audioClient_->GetService (IID_PPV_ARGS (&audioCaptureClient_));
      ACE_ASSERT (SUCCEEDED (result_2) && audioCaptureClient_);

      ACE_ASSERT (!task_);
      task_ =
        AvSetMmThreadCharacteristics (TEXT (STREAM_LIB_WASAPI_CAPTURE_DEFAULT_TASKNAME),
                                      &task_index_i);
      if (unlikely (!task_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to AvSetMmThreadCharacteristics(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      result_2 = audioClient_->Start ();
      ACE_ASSERT (SUCCEEDED (result_2));

      break;

error:
      if (audioClient_)
        audioClient_->Stop ();
      if (event_)
      {
        CloseHandle (event_); event_ = NULL;
      } // end IF
      if (audioCaptureClient_)
      {
        audioCaptureClient_->Release (); audioCaptureClient_ = NULL;
      } // end IF
      if (audioClient_)
      {
        audioClient_->Release (); audioClient_ = NULL;
      } // end IF
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
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

      if (likely (audioClient_))
      {
        result_2 = audioClient_->Stop ();
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IAudioClient::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
      } // end IF

      if (likely (task_))
      {
        if (!AvRevertMmThreadCharacteristics (task_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to AvRevertMmThreadCharacteristics(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (GetLastError (), false, false).c_str ())));
        task_ = NULL;
      } // end IF

      if (likely (event_))
      {
        if (!CloseHandle (event_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseHandle(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (GetLastError (), false, false).c_str ())));
        event_ = NULL;
      } // end IF
      if (likely (audioCaptureClient_))
      {
        audioCaptureClient_->Release (); audioCaptureClient_ = NULL;
      } // end IF
      if (likely (audioClient_))
      {
        audioClient_->Release (); audioClient_ = NULL;
      } // end IF

      inherited::sessionEndProcessed_ = true;
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename MediaType>
bool
Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                               MediaType>::collect (StatisticContainerType& data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::collect"));

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
//Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
//                                   SessionMessageType,
//                                   ProtocolMessageType,
//                                   ConfigurationType,
//                                   StreamStateType,
//                                   SessionDataType,
//                                   SessionDataContainerType,
//                                   StatisticContainerType>::report () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::report"));
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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType,
          typename MediaType>
int
Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                               MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Mic_Source_WASAPI_T::svc"));

  // sanity check(s)
  ACE_ASSERT (inherited::allocator_);
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  if (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0A00) // _WIN32_WINNT_WIN10
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

  int                            error                    = 0;
  bool                           has_finished             = false;
  ACE_Message_Block*             message_block_p          = NULL;
  DataMessageType*               message_p                = NULL;
  bool                           release_lock             = false;
  int                            result                   = -1;
  int                            result_2                 = 0;
  typename SessionMessageType::DATA_T* session_data_container_p =
    inherited::sessionData_;
  typename SessionMessageType::DATA_T::DATA_T* session_data_p =
    &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  bool                           stop_processing          = false;
  ACE_Time_Value                 no_wait                  = ACE_OS::gettimeofday ();
  DWORD                          result_3                 = 0;
  HRESULT                        result_4                 = E_FAIL;
  UINT32                         packet_length_i          = 0;
  UINT32                         num_frames_available_i   = 0;
  BYTE*                          data_p                   = 0;
  DWORD                          flags_i                  = 0;
  size_t                         bytes_to_read_i          = 0;
  bool                           session_aborted_b        = false;

  do
  {
    message_block_p = NULL;
    result = inherited::getq (message_block_p,
                              &no_wait);
    if (likely (result == -1))
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
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: worker thread (id: %t) failed to ACE_Task::putq(): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
            message_block_p->release (); message_block_p = NULL;
            break;
          } // end IF
        } // end IF
        else
        {
          message_block_p->release (); message_block_p = NULL;
        } // end ELSE

        // has STREAM_SESSION_END been processed ?
        { ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, inherited::lock_, -1);
          if (!inherited::sessionEndProcessed_)
            continue; // continue processing until STREAM_SESSION_END
        } // end lock scope

        // --> STREAM_SESSION_END has been processed, leave

        result = 0;

        goto done;
      }
      default:
      {
        // grab stream lock if processing is 'concurrent'
        if (unlikely (!inherited::configuration_->hasReentrantSynchronousSubDownstream))
        { ACE_ASSERT (streamLock_);
          try {
            release_lock = streamLock_->lock (true,  // block ?
                                              true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::lock(true,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        inherited::handleMessage (message_block_p,
                                  stop_processing);

        // *IMPORTANT NOTE*: as the session data may change when this stream is
        //                   (un-)link()ed (e.g. inbound network data
        //                   processing), the handle may have to be updated
        if (unlikely (inherited::sessionData_ &&
                      (session_data_container_p != inherited::sessionData_)))
          session_data_p = &const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

        if (unlikely (release_lock))
        { ACE_ASSERT (streamLock_);
          try {
            streamLock_->unlock (false, // unlock ?
                                 true); // forward upstream (if any) ?
          } catch (...) {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: caught exception in Stream_ILock_T::unlock(false,true), continuing\n"),
                        inherited::mod_->name ()));
          }
        } // end IF

        // finished ?
        if (unlikely (stop_processing))
        {
          // *IMPORTANT NOTE*: message_block_p has already been released()

          if (!has_finished)
          {
            has_finished = true;
            // enqueue(/process) STREAM_SESSION_END
            inherited::finished (false); // recurse upstream ?
          } // end IF

          continue; // continue processing until STREAM_SESSION_END
        } // end IF

        // iff STREAM_SESSION_END has been processed: flush data ?
        // --> flush all (session-)data; process remaining control messages only
        // *TODO*: stop_processing is set when STREAM_SESSION_END is processed;
        //         this section is currently not being reached
        // *TODO*: an alternative strategy could be to 'lock the queue' (i.e.
        //         modify put()), and 'filter-sort' the remaining messages when
        //         enqueueing the session end message; there would be no need
        //         to 'flush'
        if (unlikely (sessionEndProcessed_ && result))
        {
          result_2 = queue_.flush (true); // flush session messages ?
          if (result_2 == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to Stream_IMessageQueue::flush(true): \"%m\", aborting\n"),
                        inherited::mod_->name ()));
          else if (result_2)
            ACE_DEBUG ((LM_DEBUG,
                        ACE_TEXT ("%s: session has ended, flushed %u message(s)\n"),
                        inherited::mod_->name (),
                        result_2));
        } // end IF

        break;
      }
    } // end SWITCH
    // sanity check(s)
    if (unlikely (result_2 == -1)) // error (see above)
      break;

continue_:
    // session aborted ?
    // *TODO*: remove type inferences
    { ACE_ASSERT (session_data_p->lock);
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_p->lock, result);
      session_aborted_b = session_data_p->aborted;
    } // end lock scope
    if (unlikely (session_aborted_b))
    {
      if (!has_finished)
      {
        // *TODO*: remove type inferences
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: session (id was: %u) aborted\n"),
                    inherited::mod_->name (),
                    session_data_p->sessionId));

        has_finished = true;
        // enqueue(/process) STREAM_SESSION_END
        inherited::finished (false); // recurse upstream ?
      } // end IF

      continue; // continue processing until STREAM_SESSION_END
    } // end IF

    // step1: wait for the next buffer
    result_3 = WaitForSingleObject (event_, INFINITE);
    if (unlikely (result_3 != WAIT_OBJECT_0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to WaitForSingleObject(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_3, false, false).c_str ())));
      goto error;
    } // end IF

    // step2: grab the next buffer(s) from the capture device
    result_4 = audioCaptureClient_->GetNextPacketSize (&packet_length_i);
    if (unlikely (!SUCCEEDED (result_4)))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: failed to IAudioCaptureClient::GetNextPacketSize(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_4, false, false).c_str ())));
    while (packet_length_i)
    {
      result_4 = audioCaptureClient_->GetBuffer (&data_p,
                                                 &num_frames_available_i,
                                                 &flags_i,
                                                 NULL,
                                                 NULL);
      ACE_ASSERT (SUCCEEDED (result_4) && data_p);

      bytes_to_read_i = frameSize_ * num_frames_available_i;
      try {
        message_p =
            static_cast<DataMessageType*> (inherited::allocator_->malloc (bytes_to_read_i));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%Q), continuing\n"),
                    inherited::mod_->name (),
                    bytes_to_read_i));
        message_p = NULL;
      }
      if (unlikely (!message_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_IAllocator::malloc(%Q), aborting\n"),
                    inherited::mod_->name (),
                    bytes_to_read_i));
        goto error;
      } // end IF

      result_2 = message_p->copy (reinterpret_cast<char*> (data_p),
                                  bytes_to_read_i);
      if (unlikely (result_2 == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Block::copy(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        message_block_p->release (); message_block_p = NULL;
        goto error;
      } // end IF

      result_4 = audioCaptureClient_->ReleaseBuffer (num_frames_available_i);
      ACE_ASSERT (SUCCEEDED (result_4));

      message_p->initialize (session_data_p->sessionId,
                             NULL);

      result_2 = inherited::put_next (message_p, NULL);
      if (unlikely (result_2 == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        message_p->release (); message_p = NULL;
        goto error;
      } // end IF
      message_p = NULL;

      result_4 = audioCaptureClient_->GetNextPacketSize (&packet_length_i);
      if (unlikely (FAILED (result_4)))
      { // AUDCLNT_E_RESOURCES_INVALIDATED: 0x88890026
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IAudioCaptureClient::GetNextPacketSize(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_4, false, false).c_str ())));
        goto error;
      } // end IF
    } // end WHILE

    continue;

error:
    { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *session_data_p->lock, result);
      session_data_p->aborted = true;
    } // end lock scope
  } while (true);
  result = -1;

done:
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: %sthread (id: %t) leaving\n"),
              inherited::mod_->name (),
              (inherited::configuration_->concurrency == STREAM_HEADMODULECONCURRENCY_ACTIVE ? ACE_TEXT ("worker ")
                                                                                             : ACE_TEXT (""))));

  return result;
}
