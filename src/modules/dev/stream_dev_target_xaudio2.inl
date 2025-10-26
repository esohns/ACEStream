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

#include "common_os_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::Stream_Dev_Target_XAudio2_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in) // stream handle
 , inherited2 ()
 , bufferEndEvent_ (NULL)
 , engineHandle_ (NULL)
 , masterVoice_ (NULL)
 , sourceVoice_ (NULL)
 , streamEndEvent_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::Stream_Dev_Target_XAudio2_T"));

  bufferEndEvent_ = CreateEvent (NULL, FALSE, FALSE, NULL);
  ACE_ASSERT (bufferEndEvent_);
  streamEndEvent_ = CreateEvent (NULL, FALSE, FALSE, NULL);
  ACE_ASSERT (streamEndEvent_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::~Stream_Dev_Target_XAudio2_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::~Stream_Dev_Target_XAudio2_T"));

  if (sourceVoice_)
    sourceVoice_->DestroyVoice ();
  if (masterVoice_)
    masterVoice_->DestroyVoice ();
  if (engineHandle_)
    engineHandle_->Release ();
  if (bufferEndEvent_)
    CloseHandle (bufferEndEvent_);
  if (streamEndEvent_)
    CloseHandle (streamEndEvent_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::initialize (const ConfigurationType& configuration_in,
                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::initialize"));

  if (unlikely (inherited::isInitialized_))
  {
    if (sourceVoice_)
    {
      sourceVoice_->DestroyVoice (); sourceVoice_ = NULL;
    } // end IF
    if (masterVoice_)
    {
      masterVoice_->DestroyVoice (); masterVoice_ = NULL;
    } // end IF
    if (engineHandle_)
    {
      engineHandle_->Release (); engineHandle_ = NULL;
    } // end IF
  } // end IF

  HRESULT result = XAudio2Create (&engineHandle_,
                                  0,
                                  XAUDIO2_DEFAULT_PROCESSOR);
  if (unlikely (FAILED (result) || !engineHandle_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XAudio2Create(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    return false;
  } // end IF

  struct XAUDIO2_DEBUG_CONFIGURATION xaudio2_debug_configuration_s;
  ACE_OS::memset (&xaudio2_debug_configuration_s, 0, sizeof (struct XAUDIO2_DEBUG_CONFIGURATION));
#if defined (_DEBUG)
  xaudio2_debug_configuration_s.TraceMask =
    XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS | XAUDIO2_LOG_INFO | XAUDIO2_LOG_DETAIL;
  xaudio2_debug_configuration_s.TraceMask |= XAUDIO2_LOG_STREAMING;
  //xaudio2_debug_configuration_s.BreakMask = XAUDIO2_LOG_ERRORS;
  xaudio2_debug_configuration_s.LogThreadID = TRUE;
  xaudio2_debug_configuration_s.LogFileline = TRUE;
  xaudio2_debug_configuration_s.LogFunctionName = TRUE;
#endif // _DEBUG
  engineHandle_->SetDebugConfiguration (&xaudio2_debug_configuration_s,
                                        NULL);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleControlMessage (ControlMessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::handleControlMessage"));

  switch (message_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_ABORT:
    {
      if (likely (sourceVoice_))
      {
        HRESULT result = sourceVoice_->FlushSourceBuffers ();
        if (unlikely (FAILED (result)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IXAudio2SourceVoice::FlushSourceBuffers(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
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
          typename MediaType>
void
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (sourceVoice_);

  ACE_Message_Block* message_block_p = message_inout->duplicate ();
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::duplicate(): \"%m\", returning\n"),
                inherited::mod_->name ()));
    return;
  } // end IF

  struct XAUDIO2_BUFFER xaudio2_buffer_s;
  ACE_OS::memset (&xaudio2_buffer_s, 0, sizeof (struct XAUDIO2_BUFFER));
  xaudio2_buffer_s.AudioBytes = message_inout->length ();
  ACE_ASSERT (xaudio2_buffer_s.AudioBytes <= XAUDIO2_MAX_BUFFER_BYTES);
  xaudio2_buffer_s.pAudioData = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());
  xaudio2_buffer_s.pContext = message_block_p;

  static bool is_first_b = true;
  HRESULT result;
  DWORD wait_result;

  struct XAUDIO2_VOICE_STATE voice_state_s;
  ACE_OS::memset (&voice_state_s, 0, sizeof (struct XAUDIO2_VOICE_STATE));
  sourceVoice_->GetState (&voice_state_s);
  if (unlikely (voice_state_s.BuffersQueued >= XAUDIO2_MAX_QUEUED_BUFFERS))
  {
retry:
    // wait until a buffer has been consumed
    wait_result = WaitForSingleObject (bufferEndEvent_,
                                       INFINITE);
    if (unlikely (wait_result != WAIT_OBJECT_0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to wait for buffer end event: \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      message_block_p->release ();
      goto error;
    } // end IF
  } // end IF

  result = sourceVoice_->SubmitSourceBuffer (&xaudio2_buffer_s,
                                             NULL);
  if (unlikely (FAILED (result)))
  {
    if (result == E_OUTOFMEMORY)
      goto retry;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IXAudio2SourceVoice::SubmitSourceBuffer(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    message_block_p->release ();
    goto error;
  } // end IF

  if (unlikely (is_first_b))
  {
    is_first_b = false;
    result = sourceVoice_->Start (0, XAUDIO2_COMMIT_NOW);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IXAudio2SourceVoice::Start(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
      goto error;
    } // end IF
  } // end IF

  return;

error:
  notify (STREAM_SESSION_MESSAGE_ABORT);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);

  HRESULT result;
  typename SessionMessageType::DATA_T::DATA_T& session_data_r =
    const_cast<typename SessionMessageType::DATA_T::DATA_T&> (inherited::sessionData_->getR ());
  bool high_priority_b = false;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      if (likely (sourceVoice_))
      {
        result = sourceVoice_->FlushSourceBuffers ();
        if (unlikely (FAILED (result)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IXAudio2SourceVoice::FlushSourceBuffers(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
      } // end IF

      high_priority_b = true;
      goto end;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (engineHandle_);

      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_AUDIO,
                                media_type_s);
      ACE_ASSERT (media_type_s.majortype == MEDIATYPE_Audio);
      ACE_ASSERT (media_type_s.formattype == FORMAT_WaveFormatEx);
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (media_type_s.pbFormat);

      result = engineHandle_->CreateMasteringVoice (&masterVoice_,
                                                    audio_info_p->nChannels,
                                                    audio_info_p->nSamplesPerSec,
                                                    0,
                                                    0,
                                                    NULL);
      if (unlikely (FAILED (result) || !masterVoice_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IXAudio2::CreateMasteringVoice(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
        goto error;
      } // end IF

      result = engineHandle_->CreateSourceVoice (&sourceVoice_,
                                                 audio_info_p,
                                                 0,
                                                 XAUDIO2_DEFAULT_FREQ_RATIO,
                                                 this,
                                                 NULL,
                                                 NULL);
      if (unlikely (FAILED (result) || !sourceVoice_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IXAudio2::CreateSourceVoice(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
        goto error;
      } // end IF

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      if (likely (sourceVoice_))
      { // enqueue final buffer ?
        //struct XAUDIO2_BUFFER xaudio2_buffer_s;
        //ACE_OS::memset (&xaudio2_buffer_s, 0, sizeof (struct XAUDIO2_BUFFER));
        //xaudio2_buffer_s.Flags = XAUDIO2_END_OF_STREAM;

        //result = sourceVoice_->SubmitSourceBuffer (&xaudio2_buffer_s);
        //if (unlikely (FAILED (result)))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("%s: failed to IXAudio2SourceVoice::SubmitSourceBuffer(): \"%s\", continuing\n"),
        //              inherited::mod_->name (),
        //              ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
        result = sourceVoice_->Discontinuity ();
        if (unlikely (FAILED (result)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IXAudio2SourceVoice::Discontinuity(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
      } // end IF

      if (likely (!high_priority_b))
      { ACE_ASSERT (streamEndEvent_);
        DWORD result_2 = WaitForSingleObjectEx (streamEndEvent_,
                                                INFINITE,
                                                TRUE);
        if (unlikely (result_2 != WAIT_OBJECT_0))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to WaitForSingleObjectEx(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (GetLastError (), false, false).c_str ())));
      } // end IF

      if (likely (sourceVoice_))
      {
        result = sourceVoice_->Stop (high_priority_b ? 0 : XAUDIO2_PLAY_TAILS,
                                     XAUDIO2_COMMIT_NOW);
        if (unlikely (FAILED (result)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IXAudio2SourceVoice::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));

        sourceVoice_->DestroyVoice (); sourceVoice_ = NULL;
      } // end IF

      if (likely (masterVoice_))
      {
        masterVoice_->DestroyVoice (); masterVoice_ = NULL;
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
          typename MediaType>
void
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::OnBufferEnd (void* pBufferContext)
{
  //STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::OnBufferEnd"));

  // sanity check(s)
  ACE_Message_Block* message_block_p =
    static_cast<ACE_Message_Block*> (pBufferContext);
  ACE_ASSERT (message_block_p);

  message_block_p->release ();

  SetEvent (bufferEndEvent_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Dev_Target_XAudio2_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            MediaType>::OnVoiceError (void* pBufferContext,
                                                      HRESULT Error)
{
  //STREAM_TRACE (ACE_TEXT ("Stream_Dev_Target_XAudio2_T::OnVoiceError"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: XAudio2 error (context was: %@): \"%s\", continuing\n"),
              inherited::mod_->name (),
              pBufferContext,
              ACE_TEXT (Common_Error_Tools::errorToString (Error, false, false).c_str ())));
}
