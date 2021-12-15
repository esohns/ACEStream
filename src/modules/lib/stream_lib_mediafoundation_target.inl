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

#undef NANOSECONDS
#include "reftime.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_defines.h"
#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_lib_defines.h"
#include "stream_lib_mediafoundation_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::Stream_MediaFramework_MediaFoundation_Target_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , inherited3 ()
 , inherited4 ()
 , baseTimeStamp_ (0)
 , condition_ (inherited::lock_)
 , delayStart_ (false)
 , ignoreNextStop_ (false)
 , isFirst_ (true)
 , manageMediaSession_ (true)
 , mediaSession_ (NULL)
 , topologyIsReady_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::Stream_MediaFramework_MediaFoundation_Target_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::~Stream_MediaFramework_MediaFoundation_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::~Stream_MediaFramework_MediaFoundation_Target_T"));

  if (mediaSession_)
  {
    if (manageMediaSession_)
      Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::initialize"));

  HRESULT result = E_FAIL;
  int result_2 = -1;

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (likely (first_run))
  {
    first_run = false;
    COM_initialized = Common_Tools::initializeCOM ();
  } // end IF

  if (inherited::isInitialized_)
  {
    baseTimeStamp_ = 0;
    delayStart_ = false;
    ignoreNextStop_ = false;
    isFirst_ = true;
    if (mediaSession_)
    {
      if (manageMediaSession_)
        Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
      mediaSession_->Release (); mediaSession_ = NULL;
    } // end IF
    manageMediaSession_ = true;
    topologyIsReady_ = false;
  } // end IF

  // *TODO*: remove type inferences
  manageMediaSession_ = configuration_in.manageMediaSession;
  if (configuration_in.session)
  {
    ULONG reference_count = configuration_in.session->AddRef ();
    mediaSession_ = configuration_in.session;
  } // end IF

  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.mediaFoundationConfiguration);
  if (!inherited4::initialize (*configuration_in.mediaFoundationConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  if (!inherited::initialize (configuration_in,
                              allocator_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBaseSynch_T::initialize(), aborting\n"),
                inherited::mod_->name ()));
    goto error;
  } // end IF

  if (COM_initialized) Common_Tools::finalizeCOM ();

  return true;

error:
  if (mediaSession_)
  {
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF

  if (COM_initialized) Common_Tools::finalizeCOM ();

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited4::configuration_);
  ACE_ASSERT (inherited4::configuration_->mediaType);
  ACE_ASSERT (inherited4::mediaStream_);
  if (unlikely (inherited4::state_ != inherited4::STATE_STARTED))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: media source not running (state was: %d), returning\n"),
                inherited::mod_->name (),
                inherited4::state_));
    return;
  } // end IF

  HRESULT result = E_FAIL;
  //if (unlikely (inherited4::buffering_))
  //{
  //  inherited4::buffering_ = false;

  //  // send MEBufferingStopped
  //  struct tagPROPVARIANT property_s;
  //  PropVariantInit (&property_s);
  //  PropVariantClear (&property_s);
  //  property_s.vt = VT_EMPTY;
  //  result = inherited4::eventQueue_->QueueEventParamVar (MEBufferingStopped,
  //                                                        GUID_NULL,
  //                                                        S_OK,
  //                                                        &property_s);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  PropVariantClear (&property_s);
  //} // end IF

  // send MEMediaSample
  // step0: allocate sample
  IMFSample* sample_p = NULL;
  result = MFCreateSample (&sample_p);
  if (unlikely (FAILED (result) || !sample_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFCreateSample(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true, false).c_str ())));
    return;
  } // end IF

  // step1: set attributes
  if (isFirst_)
  {
    isFirst_ = false;
    result = sample_p->SetUINT32 (MFSampleExtension_Discontinuity,
                                  TRUE);
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  //result = sample_p->SetUINT32 (MFSampleExtension_CleanPoint,
  //                              TRUE);
  //ACE_ASSERT (SUCCEEDED (result));
  //MFSampleExtension_ForwardedDecodeUnits
  //MFSampleExtension_ForwardedDecodeUnitType
  IUnknown* unknown_p = NULL;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited4::lock_);
    if (!inherited4::tokens_.empty ())
    {
      unknown_p = inherited4::tokens_.front ();
      inherited4::tokens_.pop_front ();
    } // end IF
  } // end lock scope
  result = sample_p->SetUnknown (MFSampleExtension_Token,
                                 unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  if (unknown_p)
  {
    unknown_p->Release (); unknown_p = NULL;
  } // end IF

  // step2: set buffer / parameters
  IMFMediaBuffer* buffer_p = NULL;
  size_t total_length_i = message_inout->total_length ();
  result = MFCreateMemoryBuffer (total_length_i,
                                 &buffer_p);
  if (unlikely (FAILED (result) || !buffer_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFCreateMemoryBuffer(%u): \"%s\", returning\n"),
                inherited::mod_->name (),
                total_length_i,
                ACE_TEXT (Common_Error_Tools::errorToString (result, true, false).c_str ())));
    sample_p->Release (); sample_p = NULL;
    return;
  } // end IF
  BYTE* data_p = NULL;
  DWORD max_length_i = 0, current_length_i = 0;
  result = buffer_p->Lock (&data_p,
                           &max_length_i,
                           &current_length_i);
  ACE_ASSERT (SUCCEEDED (result) && data_p && (max_length_i == total_length_i) && !current_length_i);
  ACE_Message_Block* message_block_p = message_inout;
  unsigned int offset_i = 0;
  do
  {
    ACE_OS::memcpy (data_p + offset_i, message_block_p->rd_ptr (),
                    message_block_p->length ());
    offset_i += message_block_p->length ();
    message_block_p = message_block_p->cont ();
  } while (message_block_p);
  result = buffer_p->SetCurrentLength (total_length_i);
  ACE_ASSERT (SUCCEEDED (result));
  result = buffer_p->Unlock ();
  ACE_ASSERT (SUCCEEDED (result));
  result = sample_p->AddBuffer (buffer_p);
  ACE_ASSERT (SUCCEEDED (result));
  buffer_p->Release (); buffer_p = NULL;

  result = sample_p->SetSampleTime (baseTimeStamp_);
  ACE_ASSERT (SUCCEEDED (result));
  UINT32 bytes_per_second_i = 0;
  result =
    inherited4::configuration_->mediaType->GetUINT32 (MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                                      &bytes_per_second_i);
  ACE_ASSERT (SUCCEEDED (result) && bytes_per_second_i);
  LONGLONG duration_i =
    (LONGLONG)(total_length_i * MILLISECONDS_TO_100NS_UNITS(1000) / (double)bytes_per_second_i);
  result = sample_p->SetSampleDuration (duration_i);
  ACE_ASSERT (SUCCEEDED (result));
  baseTimeStamp_ += duration_i;

  // step3: send event
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  property_s.vt = VT_UNKNOWN;
  property_s.punkVal = sample_p;
  result = inherited4::mediaStream_->QueueEvent (MEMediaSample,
                                                 GUID_NULL,
                                                 S_OK,
                                                 &property_s);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFEventQueue::QueueEventParamUnk(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true, false).c_str ())));
    PropVariantClear (&property_s);
    return;
  } // end IF
  PropVariantClear (&property_s);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::handleSessionMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  int result = -1;
  HRESULT result_2 = E_FAIL;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      bool COM_initialized = Common_Tools::initializeCOM ();
      bool is_running = false;
      ULONG reference_count = 0;
      TOPOID node_id = 0;
      struct _GUID GUID_s = GUID_NULL;
      struct tagPROPVARIANT property_s;
      ACE_Time_Value deadline =
        ACE_Time_Value (STREAM_LIB_MEDIAFOUNDATION_MEDIASESSION_READY_TIMEOUT_S, 0);
      int error = 0;

      if (!mediaSession_)
      {
        if (session_data_r.session)
        {
          reference_count = session_data_r.session->AddRef ();
          mediaSession_ = session_data_r.session;
          goto continue_;
        } // end IF

        if (!initializeMediaSession (session_data_r.formats.back (),
                                     NULL,
                                     node_id,
                                     mediaSession_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Target_T::initializeMediaSession(), aboring\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end IF
continue_:
      ACE_ASSERT (mediaSession_);

      if (manageMediaSession_)
      {
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::lock_);
          result_2 = mediaSession_->BeginGetEvent (this, NULL);
          if (FAILED (result_2))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
            goto error;
          } // end IF

          // wait for MF_TOPOSTATUS_READY event
          deadline = COMMON_TIME_NOW + deadline;
          result = condition_.wait (&deadline);
          if (unlikely (result == -1))
          { error = ACE_OS::last_error ();
            if (error != ETIME) // 137: timed out
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("%s: failed to ACE_Condition::wait(%#T): \"%m\", aborting\n"),
                          inherited::mod_->name (),
                          &deadline));
            goto continue_2;
          } // end IF

continue_2:
          if (!topologyIsReady_)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: topology not ready%s, aborting\n"),
                        inherited::mod_->name (),
                        (error == ETIME) ? ACE_TEXT (" (timed out)") : ACE_TEXT ("")));
            goto error;
          } // end IF

          if (unlikely (delayStart_))
            goto continue_3;

          PropVariantInit (&property_s);
          //property_s.vt = VT_EMPTY;
          result_2 = mediaSession_->Start (&GUID_s,      // time format
                                           &property_s); // start position
          if (FAILED (result_2))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to IMFMediaSession::Start(): \"%s\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
            PropVariantClear (&property_s);
            goto error;
          } // end IF
          PropVariantClear (&property_s);
          is_running = true;
        } // end lock scope
      } // end IF

continue_3:
      if (COM_initialized) Common_Tools::finalizeCOM ();

      break;

error:
      if (mediaSession_)
      {
        if (manageMediaSession_)
        {
          Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
          finalizeMediaSession ();
        } // end IF
        mediaSession_->Release (); mediaSession_ = NULL;
      } // end IF

      if (COM_initialized) Common_Tools::finalizeCOM ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      bool COM_initialized = Common_Tools::initializeCOM ();

      if (mediaSession_)
      {
        if (manageMediaSession_)
        {
          Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
          finalizeMediaSession ();
        } // end IF
        mediaSession_->Release (); mediaSession_ = NULL;
      } // end IF

      if (COM_initialized) Common_Tools::finalizeCOM ();

      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::configuration_);
      ACE_ASSERT (inherited::configuration_->mediaFoundationConfiguration);

      const SessionDataContainerType& session_data_container_r =
        message_inout->getR ();
      const SessionDataType& session_data_r =
        session_data_container_r.getR ();

      // step1: update media source
      if (inherited::configuration_->mediaFoundationConfiguration->mediaType)
      {
        inherited::configuration_->mediaFoundationConfiguration->mediaType->Release (); inherited::configuration_->mediaFoundationConfiguration->mediaType = NULL;
      } // end IF
      inherited2::getMediaType (session_data_r.formats.back (),
                                inherited::configuration_->mediaFoundationConfiguration->mediaType);
      ACE_ASSERT (inherited::configuration_->mediaFoundationConfiguration->mediaType);
      if (!inherited4::initialize (*inherited::configuration_->mediaFoundationConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // step2: update topology
      if (!updateMediaSession (session_data_r.formats.back ()))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Target_T::updateMediaSession(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      break;

error_2:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::QueryInterface (const IID& IID_in,
                                                                           void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IMFAsyncCallback),
    { 0 },
  };

  HRESULT result = QISearch (this,
                             query_interface_table,
                             IID_in,
                             interface_out);
  if (result == E_NOINTERFACE)
    result = inherited4::QueryInterface (IID_in,
                                         interface_out);

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::Invoke"));

  HRESULT result = E_FAIL;
  IMFMediaEvent* media_event_p = NULL;
  MediaEventType event_type = MEUnknown;
  HRESULT status = E_FAIL;
  struct tagPROPVARIANT value;
  bool stop_b = false;
  bool request_event_b = true;
  PropVariantInit (&value);

  // sanity check(s)
  ACE_ASSERT (result_in);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (mediaSession_);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->EndGetEvent (result_in, &media_event_p);
  if (FAILED (result) || !media_event_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::EndGetEvent(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stop_b = true;
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = media_event_p->GetType (&event_type);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetStatus (&status);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_event_p->GetValue (&value);
  ACE_ASSERT (SUCCEEDED (result));
  switch (event_type)
  {
    case MEEndOfPresentation:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MEEndOfPresentation\n"),
                  inherited::mod_->name ()));
      break;
    }
    case MEError:
    { // MF_E_INVALID_TIMESTAMP : 0xc00d36c0
      // MF_E_STREAMSINK_REMOVED: 0xc00d4a38
      // MF_E_STREAM_ERROR      : 0xc00da7fb
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received MEError: \"%s\"\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      request_event_b = false;
      stop_b = true;
      break;
    }
    case MESessionClosed:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionClosed, shutting down\n"),
                  inherited::mod_->name ()));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      request_event_b = false;
      break;
    }
    case MESessionEnded:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionEnded, closing sesion\n"),
                  inherited::mod_->name ()));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Close ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      break;
    }
    case MESessionCapabilitiesChanged:
    {
      UINT32 session_capabilities_i = 0, session_capabilities_delta_i = 0;
      result = media_event_p->GetUINT32 (MF_EVENT_SESSIONCAPS,
                                         &session_capabilities_i);
      ACE_ASSERT (SUCCEEDED (result));
      result = media_event_p->GetUINT32 (MF_EVENT_SESSIONCAPS_DELTA,
                                         &session_capabilities_delta_i);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionCapabilitiesChanged: %u/%u\n"),
                  inherited::mod_->name (),
                  session_capabilities_i, session_capabilities_delta_i));
      break;
    }
    case MESessionNotifyPresentationTime:
    {
      UINT64 presentation_time_start_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_START_PRESENTATION_TIME,
                                         &presentation_time_start_i);
      ACE_ASSERT (SUCCEEDED (result));
      UINT64 presentation_time_offset_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_PRESENTATION_TIME_OFFSET,
                                         &presentation_time_offset_i);
      ACE_ASSERT (SUCCEEDED (result));
      UINT64 presentation_time_at_output_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_START_PRESENTATION_TIME_AT_OUTPUT,
                                         &presentation_time_at_output_i);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionNotifyPresentationTime: %Q/%Q/%Q\n"),
                  inherited::mod_->name (),
                  presentation_time_start_i, presentation_time_offset_i, presentation_time_at_output_i));
      break;
    }
    case MESessionStarted:
    { // status MF_E_INVALIDREQUEST    : 0xC00D36B2
      // status MF_E_ATTRIBUTENOTFOUND : 0xC00D36E6
      // status MF_E_STREAMSINK_REMOVED: 0xc00d4a38
      UINT64 presentation_time_offset_i = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_PRESENTATION_TIME_OFFSET,
                                         &presentation_time_offset_i);
      //ACE_ASSERT (SUCCEEDED (result)); // MF_E_ATTRIBUTENOTFOUND: 0xc00d36e6
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStarted: \"%s\", presentation offset: %q\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ()),
                  presentation_time_offset_i));
      break;
    }
    case MESessionStopped:
    { // status MF_E_INVALIDREQUEST: 0xC00D36B2
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionStopped, closing sesion\n"),
                  inherited::mod_->name ()));
      if (ignoreNextStop_)
      {
        ignoreNextStop_ = false;
        break;
      } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = mediaSession_->Close ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      break;
    }
    case MESessionTopologySet:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologySet (status was: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      break;
    }
    case MESessionTopologyStatus:
    {
      UINT32 attribute_value = 0;
      MF_TOPOSTATUS topology_status = MF_TOPOSTATUS_INVALID;
      result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                         &attribute_value);
      ACE_ASSERT (SUCCEEDED (result));
      topology_status = static_cast<MF_TOPOSTATUS> (attribute_value);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MESessionTopologyStatus: \"%s\" (status was: \"%s\")\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
      // start media session ?
      if (topology_status == MF_TOPOSTATUS_READY)
      {
        { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, inherited::lock_, E_FAIL);
          if (SUCCEEDED (status))
            topologyIsReady_ = true;
          else
            stop_b = true;
          int result_2 = condition_.broadcast ();
          if (unlikely (result_2 == -1))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to ACE_Condition::broadcast(): \"%m\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (topology_status).c_str ())));
            stop_b = true;
            goto error;
          } // end IF
        } // end lock scope
      } // end IF
      break;
    }
    case MEExtendedType:
    {
      struct _GUID GUID_s = GUID_NULL;
      result = media_event_p->GetExtendedType (&GUID_s);
      ACE_ASSERT (SUCCEEDED (result));
      // MF_MEEXT_SAR_AUDIO_ENDPOINT_CHANGED: {02E7187D-0087-437E-A27F-CF5ADCCD3112}
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received extended media session event (type was: %s)\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
      break;
    }
    case MEStreamSinkFormatInvalidated:
    {
      //IMFMediaSink* media_sink_p = NULL;
      //// *TODO*: {3EA99C15-A893-4B46-B221-5FAE05C36152}
      //struct _GUID GUID_s =
      //  Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{3EA99C15-A893-4B46-B221-5FAE05C36152}"));
      //result = media_event_p->GetUnknown (GUID_s,
      //                                    IID_PPV_ARGS (&media_sink_p));
      //ACE_ASSERT (SUCCEEDED (result) && media_sink_p);
      TOPOID node_id = 0;
      result = media_event_p->GetUINT64 (MF_EVENT_OUTPUT_NODE,
                                         &node_id);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MEStreamSinkFormatInvalidated (id: %q)\n"),
                  inherited::mod_->name (),
                  node_id));
      //media_sink_p->Release (); media_sink_p = NULL;
      break;
    }
    case MEEndOfPresentationSegment:
    { // *TODO*: {9C86CC50-68CE-4CFF-AA1E-9A5A40D5B4E0}
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: received MEEndOfPresentationSegment\n"),
                  inherited::mod_->name ()));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: received unknown/invalid media session event (type was: %d), continuing\n"),
                  inherited::mod_->name (),
                  event_type));
      break;
    }
  } // end SWITCH
  PropVariantClear (&value);
  media_event_p->Release (); media_event_p = NULL;

  if (unlikely (!request_event_b))
    goto continue_;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = mediaSession_->BeginGetEvent (this, NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::BeginGetEvent(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stop_b = true;
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

continue_:
  if (unlikely (stop_b))
    this->notify (STREAM_SESSION_MESSAGE_ABORT);

  return S_OK;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
error:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (unlikely (stop_b))
    this->notify (STREAM_SESSION_MESSAGE_ABORT);

  if (media_event_p)
    media_event_p->Release ();
  PropVariantClear (&value);

  return E_FAIL;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataType,
                                               SessionDataContainerType,
                                               MediaType>::initializeMediaSession (IMFMediaType* mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                                                   IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                                                   IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                                                   TOPOID& sampleGrabberSinkNodeId_out,
                                                                                   IMFMediaSession*& IMFMediaSession_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::initializeMediaSession"));

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (IMFMediaSession_inout);

  TOPOID node_id = 0;
  IMFTopologyNode* topology_node_p = NULL;
  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);//    |
                          //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                          //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
  IMFTopology* topology_p = NULL;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  HRESULT result =
    const_cast<IMFMediaSession*> (IMFMediaSession_inout)->GetFullTopology (flags,
                                                                           0,
                                                                           &topology_p);
  if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);

  if (!Stream_MediaFramework_MediaFoundation_Tools::addGrabber (//mediaType_in,
                                                                sampleGrabberSinkCallback_in,
                                                                topology_p,
                                                                node_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addGrabber(), aborting\n")));
    goto error;
  } // end IF
  result = topology_p->GetNodeByID (node_id,
                                    &topology_node_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (topology_node_p);
  topology_node_p->Release (); topology_node_p = NULL;

  result = IMFMediaSession_inout->SetTopology (topology_flags,
                                               topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  // debug info
#if defined (_DEBUG)
  Stream_MediaFramework_MediaFoundation_Tools::dump (topology_p);
#endif // _DEBUG
  topology_p->Release (); topology_p = NULL;

  return true;

error:
  if (topology_p)
    topology_p->Release ();

  return false;
}
