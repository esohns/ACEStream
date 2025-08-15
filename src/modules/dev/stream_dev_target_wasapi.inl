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
#include "Shlwapi.h"

#include "ace/Log_Msg.h"

#include "common_os_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_directsound_tools.h"
#include "stream_lib_guids.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::Stream_Dev_Target_WASAPI_T (ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , audioClient_ (NULL)
 , audioRenderClient_ (NULL)
 , bufferSize_ (0)
 , event_ (NULL)
 , frameSize_ (0)
 , queue_ (STREAM_QUEUE_MAX_SLOTS, // max # slots
           NULL)                   // notification handle
 , task_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::Stream_Dev_Target_WASAPI_T"));

  inherited::msg_queue (&queue_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::~Stream_Dev_Target_WASAPI_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::~Stream_Dev_Target_WASAPI_T"));

  if (event_)
    CloseHandle (event_);
  if (audioRenderClient_)
    audioRenderClient_->Release ();
  if (audioClient_)
    audioClient_->Release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
bool
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::initialize (const ConfigurationType& configuration_in,
                                                   Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    int result = queue_.activate ();
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue::activate(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      return false;
    } // end IF

    task_ = NULL;
    if (event_)
    {
      if (!CloseHandle (event_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CloseHandle(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      event_ = NULL;
    } // end IF
    if (audioRenderClient_)
    {
      audioRenderClient_->Release (); audioRenderClient_ = NULL;
    } // end IF
    if (audioClient_)
    {
      audioClient_->Release (); audioClient_ = NULL;
    } // end IF
    bufferSize_ = 0;
    frameSize_ = 0;
  } // end IF

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u data messages\n"),
                    inherited::mod_->name (),
                    result));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting\n"),
                    inherited::mod_->name ()));
      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::msg_queue_);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  
  message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = inherited::msg_queue_->enqueue_tail (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    message_block_p->release ();
    return;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  HRESULT result_2 = E_FAIL;
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  bool high_priority_b = false;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      unsigned int result = queue_.flush (false); // flush all data messages
      if (unlikely (result == static_cast<unsigned int> (-1)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MessageQueue_T::flush(false): \"%m\", returning\n"),
                    inherited::mod_->name ()));
        return;
      } // end IF
      else if (result > 0)
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting: flushed %u data messages\n"),
                    inherited::mod_->name (),
                    result));
      else
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: aborting\n"),
                    inherited::mod_->name ()));

      high_priority_b = true;
      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
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
      struct _GUID device_identifier_s = GUID_NULL;
      switch (inherited::configuration_->deviceIdentifier.identifierDiscriminator)
      {
        case Stream_Device_Identifier::ID:
        {
          device_identifier_s =
            Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (static_cast<ULONG> (inherited::configuration_->deviceIdentifier.identifier._id),
                                                                                    false); // render
          break;
        }
        case Stream_Device_Identifier::GUID:
        {
          device_identifier_s =
            inherited::configuration_->deviceIdentifier.identifier._guid;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: invalid/unknown device identifier type (was: %d), aborting\n"),
                      inherited::mod_->name (),
                      inherited::configuration_->deviceIdentifier.identifierDiscriminator));
          goto error;
        }
      } // end SWITCH

      REFERENCE_TIME requested_duration_i = 0;
      enum _AUDCLNT_SHAREMODE share_mode_e =
        STREAM_LIB_WASAPI_RENDER_DEFAULT_SHAREMODE;
      IMMDevice* device_p = NULL;
      DWORD stream_flags_i =
        (AUDCLNT_STREAMFLAGS_EVENTCALLBACK           |
         //AUDCLNT_STREAMFLAGS_NOPERSIST             |
         /////////////////////////////////
         AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED      |
         AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED);
      struct _GUID GUID_s = GUID_NULL;
      //HANDLE task_h = NULL;
      //DWORD task_index_i = 0;
      struct tWAVEFORMATEX* audio_info_2 = NULL;
      IAudioSessionControl* audio_session_control_p = NULL;

      if (InlineIsEqualGUID (device_identifier_s, GUID_NULL))
      {
        IMMDeviceEnumerator* enumerator_p = NULL;
        result_2 =
          CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL, CLSCTX_ALL,
                            IID_PPV_ARGS (&enumerator_p));
        ACE_ASSERT (SUCCEEDED (result_2) && enumerator_p);
        result_2 =
          enumerator_p->GetDefaultAudioEndpoint (eRender, eMultimedia, &device_p);
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
        Stream_MediaFramework_DirectSound_Tools::getDevice (device_identifier_s);
      if (unlikely (!device_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to retrieve device handle (id was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_OS_Tools::GUIDToString (device_identifier_s).c_str ())));
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
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: format (was: %s) not supported; closest match: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_2, true).c_str ())));
        CoTaskMemFree (audio_info_2); audio_info_2 = NULL;
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      ACE_ASSERT (!audio_info_2);

      result_2 = audioClient_->GetDevicePeriod (&requested_duration_i, NULL);
      ACE_ASSERT (SUCCEEDED (result_2) && requested_duration_i);
      //requested_duration_i =
      //  (REFERENCE_TIME)((10000.0 * 1000 / audio_info_p->nSamplesPerSec * 160) + 0.5);
      GUID_s = CLSID_ACEStream_MediaFramework_WASAPI_AudioSession;
retry:
      result_2 =
        audioClient_->Initialize (share_mode_e,
                                  stream_flags_i,
                                  requested_duration_i,
                                  ((share_mode_e == AUDCLNT_SHAREMODE_EXCLUSIVE) ? requested_duration_i 
                                                                                 : 0),
                                  audio_info_p,
                                  &GUID_s);
      if (unlikely (FAILED (result_2))) // AUDCLNT_E_UNSUPPORTED_FORMAT: 0x88890008
      {
        if (unlikely (result_2 == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED))
        {
          result_2 = audioClient_->GetBufferSize (&bufferSize_);
          ACE_ASSERT (SUCCEEDED (result_2) && bufferSize_);
          audioClient_->Release (); audioClient_ = NULL;
          requested_duration_i =
            (REFERENCE_TIME)((10000.0 * 1000 / audio_info_p->nSamplesPerSec * bufferSize_) + 0.5);
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
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened device \"%s\": format: %s...\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::directSoundGUIDToString (device_identifier_s).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectSound_Tools::toString (*audio_info_p, true).c_str ())));

      device_p->Release (); device_p = NULL;
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      result_2 = audioClient_->GetBufferSize (&bufferSize_);
      ACE_ASSERT (SUCCEEDED (result_2) && bufferSize_);
      result_2 =
        audioClient_->GetService (IID_PPV_ARGS (&audio_session_control_p));
      ACE_ASSERT (SUCCEEDED (result_2) && audio_session_control_p);
      result_2 =
        audio_session_control_p->RegisterAudioSessionNotification (this);
      ACE_ASSERT (SUCCEEDED (result_2));
      audio_session_control_p->Release (); audio_session_control_p = NULL;

      ACE_ASSERT (!event_);
      event_ = CreateEvent (NULL,  // lpEventAttributes
                            FALSE, // bManualReset
                            FALSE, // bInitialState
                            NULL); // lpName
      if (!event_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CreateEvent(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result_2 = audioClient_->SetEventHandle (event_);
      ACE_ASSERT (SUCCEEDED (result_2));

      ACE_ASSERT (!audioRenderClient_);
      result_2 = audioClient_->GetService (IID_PPV_ARGS (&audioRenderClient_));
      ACE_ASSERT (SUCCEEDED (result_2) && audioRenderClient_);

      result_2 = audioClient_->Start ();
      ACE_ASSERT (SUCCEEDED (result_2));

      inherited::threadCount_ = 1;
      inherited::start (NULL);
      inherited::threadCount_ = 0;

      break;

error:
      if (audioClient_)
        audioClient_->Stop ();
      if (event_)
      {
        CloseHandle (event_); event_ = NULL;
      } // end IF
      if (audioRenderClient_)
      {
        audioRenderClient_->Release (); audioRenderClient_ = NULL;
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
end:
      if (likely (inherited::thr_count_ > 0))
      {
        if (high_priority_b || !inherited::configuration_->waitForDataOnEnd)
          queue_.flush (false); // flush all data messages
        Common_ITask* itask_p = this;
        itask_p->stop (true,             // wait ?
                       high_priority_b); // high priority ?
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

      if (likely (event_))
      {
        if (!CloseHandle (event_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseHandle(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (GetLastError (), false, false).c_str ())));
        event_ = NULL;
      } // end IF
      if (likely (audioRenderClient_))
      {
        audioRenderClient_->Release (); audioRenderClient_ = NULL;
      } // end IF
      if (likely (audioClient_))
      {
        IAudioSessionControl* audio_session_control_p = NULL;
        result_2 =
          audioClient_->GetService (IID_PPV_ARGS (&audio_session_control_p));
        if (FAILED (result_2))
        {
          if (result_2 == AUDCLNT_E_NOT_INITIALIZED)
            goto continue_2;
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IAudioClient::GetService(IID_IAudioSessionControl): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
          goto continue_2;
        } // end IF
        ACE_ASSERT (audio_session_control_p);
        result_2 =
          audio_session_control_p->UnregisterAudioSessionNotification (this);
        // *NOTE*: needs to be the same thread that called RegisterAudioSessionNotification() ?
        if (FAILED (result_2)) // E_NOTFOUND: 0x80070490
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IAudioSessionControl::UnregisterAudioSessionNotification(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        audio_session_control_p->Release (); audio_session_control_p = NULL;
continue_2:
        audioClient_->Release (); audioClient_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::QueryInterface (REFIID IID_in,
                                                       void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IUnknown),
    QITABENT (OWN_TYPE_T, IAudioSessionEvents),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnDisplayNameChanged (LPCWSTR NewDisplayName,
                                                             LPCGUID EventContext)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnDisplayNameChanged"));

  ACE_UNUSED_ARG (NewDisplayName);
  ACE_UNUSED_ARG (EventContext);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnIconPathChanged (LPCWSTR NewIconPath,
                                                          LPCGUID EventContext)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnIconPathChanged"));

  ACE_UNUSED_ARG (NewIconPath);
  ACE_UNUSED_ARG (EventContext);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnSimpleVolumeChanged (float NewVolume,
                                                              BOOL NewMute,
                                                              LPCGUID EventContext)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnSimpleVolumeChanged"));

  ACE_UNUSED_ARG (NewVolume);
  ACE_UNUSED_ARG (NewMute);
  ACE_UNUSED_ARG (EventContext);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnChannelVolumeChanged (DWORD ChannelCount,
                                                               float NewChannelVolumeArray[],
                                                               DWORD ChangedChannel,
                                                               LPCGUID EventContext)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnChannelVolumeChanged"));

  ACE_UNUSED_ARG (ChannelCount);
  ACE_UNUSED_ARG (NewChannelVolumeArray);
  ACE_UNUSED_ARG (ChangedChannel);
  ACE_UNUSED_ARG (EventContext);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnGroupingParamChanged (LPCGUID NewGroupingParam,
                                                               LPCGUID EventContext)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnGroupingParamChanged"));

  ACE_UNUSED_ARG (NewGroupingParam);
  ACE_UNUSED_ARG (EventContext);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnStateChanged (AudioSessionState NewState)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnStateChanged"));

  ACE_UNUSED_ARG (NewState);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
HRESULT
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::OnSessionDisconnected (AudioSessionDisconnectReason DisconnectReason)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::OnSessionDisconnected"));

  ACE_UNUSED_ARG (DisconnectReason);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
void
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::stop (bool waitForCompletion_in,
                                             bool highPriority_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::stop"));

  ACE_UNUSED_ARG (waitForCompletion_in);
  ACE_UNUSED_ARG (highPriority_in);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  ACE_NEW_NORETURN (message_block_p,
                    ACE_Message_Block (0,                                  // size
                                       ACE_Message_Block::MB_STOP,         // type
                                       NULL,                               // continuation
                                       NULL,                               // data
                                       NULL,                               // buffer allocator
                                       NULL,                               // locking strategy
                                       ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
                                       ACE_Time_Value::zero,               // execution time
                                       ACE_Time_Value::max_time,           // deadline time
                                       NULL,                               // data block allocator
                                       NULL));                             // message allocator
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate ACE_Message_Block: \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  result = this->putq (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task_Base::putq(): \"%m\", continuing\n"),
                inherited::mod_->name ()));
    message_block_p->release (); message_block_p = NULL;
  } // end IF

  if (likely (waitForCompletion_in))
    inherited::wait ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
ACE_Message_Block*
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::get ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::get"));

  ACE_Message_Block* message_block_p = NULL;
  int result = inherited::getq (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    int error = ACE_OS::last_error ();
    if (error != EWOULDBLOCK) // Win32: 10035
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    return NULL;
  } // end IF
  ACE_ASSERT (message_block_p);

  switch (message_block_p->msg_type ())
  {
    case ACE_Message_Block::MB_STOP:
    {
      message_block_p->release (); message_block_p = NULL;
      result = queue_.deactivate ();
      if (unlikely (result == -1))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue::deactivate(): \"%m\", continuing\n"),
                    inherited::mod_->name ()));
      break;
    }
    case STREAM_MESSAGE_DATA:
    case STREAM_MESSAGE_OBJECT:
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message type (was: %d), continuing\n"),
                  inherited::mod_->name (),
                  message_block_p->msg_type ()));
      message_block_p->release (); message_block_p = NULL;
      break;
    }
  } // end SWITCH

  return message_block_p;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
int
Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                           TimePolicyType,
                           ConfigurationType,
                           ControlMessageType,
                           DataMessageType,
                           SessionMessageType,
                           SessionControlType,
                           SessionEventType,
                           UserDataType,
                           MediaType>::svc (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::svc"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (ID: %t) starting\n"),
              inherited::mod_->name ()));

  int result = 0;
  ACE_Message_Block* message_block_p = NULL;
  DWORD task_index_i = 0;
  DWORD result_2 = 0;
  HRESULT result_3 = E_FAIL;
  UINT32 num_frames_available_i = 0;
  BYTE* data_p = 0;
  size_t bytes_to_write_i = 0;
  size_t offset_i = 0;
  UINT32 buffer_size_i = 0; // (in #frames)
  // *NOTE*: supply the soundcard with data as fast as possible; even if it is
  //         less than the available buffer
  //bool buffer_written_b = false;

  ACE_ASSERT (!task_);
  task_ =
    AvSetMmThreadCharacteristics (TEXT (STREAM_LIB_WASAPI_RENDER_DEFAULT_TASKNAME),
                                  &task_index_i);
  if (unlikely (!task_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to AvSetMmThreadCharacteristics(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    result = -1;
    goto done;
  } // end IF

  // process sample data
  do
  {
    // step1: wait for the next buffer
next_buffer:
    result_2 = WaitForSingleObject (event_, INFINITE);
    if (unlikely (result_2 != WAIT_OBJECT_0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to WaitForSingleObject(), aborting\n"),
                  inherited::mod_->name ()));
      result = -1;
      goto done;
    } // end IF

    // step2: grab the next buffer(s) from the render device
    buffer_size_i = bufferSize_;
retry:
    result_3 = audioRenderClient_->GetBuffer (buffer_size_i,
                                              &data_p);
    //AUDCLNT_E_BUFFER_TOO_LARGE: 0x88890006
    if (unlikely (FAILED (result_3)))
    {
      if (result_3 == AUDCLNT_E_BUFFER_TOO_LARGE)
      {
        // retry with a smaller buffer
        buffer_size_i /= 2;
        // *TODO*: do not write less data than IAudioClient::GetStreamLatency()
        buffer_size_i += static_cast<UINT32> (frameSize_ - (buffer_size_i % frameSize_));
        ACE_ASSERT ((buffer_size_i % frameSize_) == 0);
        goto retry;
      } // end IF
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IAudioRenderClient::GetBuffer(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_3, false, false).c_str ())));
      notify (STREAM_SESSION_MESSAGE_ABORT);
      result = -1;
      goto done;
    } // end IF
    ACE_ASSERT (data_p);
    num_frames_available_i = buffer_size_i;
    offset_i = 0;

    if (message_block_p)
      goto continue_;

next_buffer_2:
    message_block_p = get ();
    if (unlikely (!message_block_p))
    {
      result_3 =
        audioRenderClient_->ReleaseBuffer (buffer_size_i - num_frames_available_i,
                                           /*flags_i*/0);
      ACE_ASSERT (SUCCEEDED (result_3));
      goto done;
    } // end IF
    //buffer_written_b = false;

continue_:
    bytes_to_write_i = std::min (message_block_p->length (),
                                 (num_frames_available_i * frameSize_));
    //ACE_ASSERT ((bytes_to_write_i % frameSize_) == 0);
    ACE_OS::memcpy (data_p + offset_i, message_block_p->rd_ptr (),
                    bytes_to_write_i);
    num_frames_available_i -=
      static_cast<UINT32> (bytes_to_write_i / frameSize_);
    offset_i += bytes_to_write_i;

    message_block_p->rd_ptr (bytes_to_write_i);
    if (!message_block_p->length ())
    {
      //buffer_written_b = true;
      if (unlikely (message_block_p->cont ()))
      {
        ACE_Message_Block* message_block_2 = message_block_p->cont ();
        message_block_p->cont (NULL);
        message_block_p->release (); message_block_p = message_block_2;
      } // end IF
      else
      {
        message_block_p->release (); message_block_p = NULL;
      } // end ELSE
    } // end IF

    if (!num_frames_available_i /* || buffer_written_b*/)
    {
      result_3 =
        audioRenderClient_->ReleaseBuffer (buffer_size_i - num_frames_available_i,
                                           /*flags_i*/0);
      ACE_ASSERT (SUCCEEDED (result_3));
      goto next_buffer;
    } // end IF
    if (!message_block_p)
      goto next_buffer_2;
    goto continue_;
  } while (true);
  result = -1;

done:
  if (likely (task_))
  {
    AvRevertMmThreadCharacteristics (task_);
    task_ = NULL;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: renderer thread (ID: %t) leaving\n"),
              inherited::mod_->name ()));

  return result;
}
