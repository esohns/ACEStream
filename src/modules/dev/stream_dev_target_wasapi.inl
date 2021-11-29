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

#include "common_log_tools.h"

#include "common_timer_manager_common.h"

#include "stream_defines.h"
#include "stream_macros.h"

#include "stream_lib_directsound_tools.h"

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
                           MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_WASAPI_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  ACE_Message_Block* message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF
  ACE_ASSERT (inherited::msg_queue_);
  int result = inherited::msg_queue_->enqueue_tail (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n"),
                inherited::mod_->name ()));

    // clean up
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

  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::isInitialized_);
  ACE_ASSERT (inherited::sessionData_);

  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_ASSERT (!session_data_r.formats.empty ());
      struct _AMMediaType media_type_s, media_type_2;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      ACE_OS::memset (&media_type_2, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);
      frameSize_ = audio_info_p->nChannels * (audio_info_p->wBitsPerSample / 8);
      ACE_ASSERT (frameSize_ == audio_info_p->nBlockAlign);
      ACE_ASSERT (inherited::configuration_->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::GUID);
      struct _GUID device_identifier_s =
        ((inherited::configuration_->audioOutput >= 0) ? Stream_MediaFramework_DirectSound_Tools::waveDeviceIdToDirectSoundGUID (static_cast<ULONG> (inherited::configuration_->audioOutput),
                                                                                                                                 false) // render
                                                       : inherited::configuration_->deviceIdentifier.identifier._guid);

      REFERENCE_TIME requested_duration_i = 0;
      IMMDeviceEnumerator* enumerator_p = NULL;
      HRESULT result_2 =
        CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          IID_PPV_ARGS (&enumerator_p));
      ACE_ASSERT (SUCCEEDED (result_2));
      IMMDeviceCollection* devices_p = NULL;
      UINT num_devices_i = 0;
      enum _AUDCLNT_SHAREMODE share_mode_e =
        STREAM_DEV_WASAPI_RENDER_DEFAULT_SHAREMODE;
      IMMDevice* device_p = NULL;
      struct _GUID GUID_s = GUID_NULL;
      IPropertyStore* property_store_p = NULL;
      struct tagPROPVARIANT property_s;
      DWORD stream_flags_i =
        (AUDCLNT_STREAMFLAGS_EVENTCALLBACK |
         //AUDCLNT_STREAMFLAGS_NOPERSIST         |
         /////////////////////////////////
         AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED);
      HANDLE task_h = NULL;
      struct tWAVEFORMATEX* audio_info_2 = NULL;
      if (InlineIsEqualGUID (device_identifier_s, GUID_NULL))
      {
        result_2 =
          enumerator_p->GetDefaultAudioEndpoint (eRender, eConsole, &device_p);
        ACE_ASSERT (SUCCEEDED (result_2) && device_p);
        goto continue_;
      } // end IF
      result_2 = enumerator_p->EnumAudioEndpoints (eRender,
                                                   DEVICE_STATEMASK_ALL,
                                                   &devices_p);
      ACE_ASSERT (SUCCEEDED (result_2));
      enumerator_p->Release (); enumerator_p = NULL;
      result_2 = devices_p->GetCount (&num_devices_i);
      ACE_ASSERT (SUCCEEDED (result_2));
      PropVariantInit (&property_s);
      for (UINT i = 0;
           i < num_devices_i;
           ++i)
      { ACE_ASSERT (!device_p);
        result_2 = devices_p->Item (i,
                                    &device_p);
        ACE_ASSERT (SUCCEEDED (result_2));
        result_2 = device_p->OpenPropertyStore (STGM_READ,
                                                &property_store_p);
        ACE_ASSERT (SUCCEEDED (result_2));
        result_2 = property_store_p->GetValue (PKEY_AudioEndpoint_GUID,
                                               &property_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        property_store_p->Release (); property_store_p = NULL;
        ACE_ASSERT (property_s.vt == VT_LPWSTR);
        GUID_s =
          Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (property_s.pwszVal)));
        PropVariantClear (&property_s);
        if (InlineIsEqualGUID (GUID_s, device_identifier_s))
          break;
        device_p->Release (); device_p = NULL;
      } // end FOR
      devices_p->Release (); devices_p = NULL;
      if (!device_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to retrieve render device handle (id was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::GUIDToString (device_identifier_s).c_str ())));
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
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*audio_info_p).c_str ())));
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      if ((result_2 == S_FALSE) && audio_info_2)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: format (was: %s) not supported; closest match: \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*audio_info_p).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*audio_info_2).c_str ())));
        CoTaskMemFree (audio_info_2); audio_info_2 = NULL;
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      if (audio_info_2)
      {
        CoTaskMemFree (audio_info_2); audio_info_2 = NULL;
      } // end IF
      result_2 = audioClient_->GetDevicePeriod (NULL, &requested_duration_i);
      ACE_ASSERT (SUCCEEDED (result_2));
retry:
      result_2 = audioClient_->Initialize (share_mode_e,
                                           stream_flags_i,
                                           requested_duration_i,
                                           ((share_mode_e == AUDCLNT_SHAREMODE_EXCLUSIVE) ? requested_duration_i : 0),
                                           audio_info_p,
                                           NULL);
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
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*audio_info_p).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        device_p->Release (); device_p = NULL;
        goto error;
      } // end IF
      device_p->Release (); device_p = NULL;
      result_2 = audioClient_->GetBufferSize (&bufferSize_);
      ACE_ASSERT (SUCCEEDED (result_2) && bufferSize_);

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
      ACE_ASSERT (SUCCEEDED (result_2));

      result_2 = audioClient_->Start ();
      ACE_ASSERT (SUCCEEDED (result_2));

      inherited::threadCount_ = 1;
      inherited::start (NULL);
      inherited::threadCount_ = 0;

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (inherited::thr_count_ > 0)
      {
        Common_ITask* itask_p = this;
        itask_p->stop (true,   // wait ?
                       false); // high priority ?
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: joined renderer thread\n"),
                    inherited::mod_->name ()));
      } // end IF

      HRESULT result_2 = E_FAIL;
      if (likely (audioClient_))
      {
        result_2 = audioClient_->Stop ();
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IAudioClient::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
      } // end IF

      if (likely (audioRenderClient_))
      {
        audioRenderClient_->Release (); audioRenderClient_ = NULL;
      } // end IF
      if (likely (audioClient_))
      {
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
  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  int error = 0;
  DWORD task_index_i = 0;
  DWORD result_3 = 0;
  HRESULT result_4 = E_FAIL;
  UINT32 packet_length_i = 0;
  UINT32 num_frames_available_i = 0, num_frames_written_i = 0;
  BYTE* data_p = 0;
  //DWORD flags_i = 0;
  size_t bytes_to_write_i = 0;

  ACE_ASSERT (!task_);
  task_ = AvSetMmThreadCharacteristics (TEXT ("Pro Audio"), &task_index_i);
  if (!task_)
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
    message_block_p = NULL;
    result_2 = inherited::getq (message_block_p, NULL);
    if (unlikely (result_2 == -1))
    {
      error = ACE_OS::last_error ();
      if (error != EWOULDBLOCK) // Win32: 10035
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Task::getq(): \"%m\", aborting\n"),
                    inherited::mod_->name ()));
      break;
    } // end IF
    ACE_ASSERT (message_block_p);

    switch (message_block_p->msg_type ())
    {
      case ACE_Message_Block::MB_STOP:
      {
        message_block_p->release (); message_block_p = NULL;
        goto done;
      }
      case STREAM_MESSAGE_DATA:
      case STREAM_MESSAGE_OBJECT:
      {
        // step1: wait for the next buffer
next_buffer:
        result_3 = WaitForSingleObject (event_, INFINITE);
        if (unlikely (result_3 != WAIT_OBJECT_0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to WaitForSingleObject(), aborting\n"),
                      inherited::mod_->name ()));
          result = -1;
          goto done;
        } // end IF

        // step2: grab the next buffer(s) from the render device
        result_4 = audioRenderClient_->GetBuffer (bufferSize_,
                                                  &data_p);
        ACE_ASSERT (SUCCEEDED (result_4) && data_p);
        bytes_to_write_i = frameSize_ * bufferSize_;
        bytes_to_write_i =
          std::min (message_block_p->length (), bytes_to_write_i);
        ACE_ASSERT ((bytes_to_write_i % frameSize_) == 0);
        ACE_OS::memcpy (data_p, message_block_p->rd_ptr (),
                        bytes_to_write_i);
        result_4 = audioRenderClient_->ReleaseBuffer (bytes_to_write_i / frameSize_,
                                                      /*flags_i*/0);
        ACE_ASSERT (SUCCEEDED (result_4));

        message_block_p->rd_ptr (bytes_to_write_i);
        if (message_block_p->length ())
          goto next_buffer;

        // *WARNING*: control falls through
      }
      default:
      {
        message_block_p->release (); message_block_p = NULL;
        break;
      }
    } // end SWITCH
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
