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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::Stream_MediaFramework_MediaFoundation_Source_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , inherited2 ()
 , condition_ (inherited::lock_)
 , manageMediaSession_ (true)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
 , presentationClock_ (NULL)
 , referenceCount_ (1)
 , topologyIsReady_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::Stream_MediaFramework_MediaFoundation_Source_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::~Stream_MediaFramework_MediaFoundation_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::~Stream_MediaFramework_MediaFoundation_Source_T"));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (mediaSession_)
  {
    if (manageMediaSession_)
      Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (presentationClock_)
    presentationClock_->Release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
bool
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::initialize (const ConfigurationType& configuration_in,
                                                                          Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::initialize"));

  HRESULT result = E_FAIL;

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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (mediaSession_)
    {
      if (manageMediaSession_)
        Stream_MediaFramework_MediaFoundation_Tools::shutdown (mediaSession_);
      mediaSession_->Release (); mediaSession_ = NULL;
    } // end IF
    manageMediaSession_ = true;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    if (presentationClock_)
    {
      presentationClock_->Release (); presentationClock_ = NULL;
    } // end IF
    referenceCount_ = 1;
    topologyIsReady_ = false;
  } // end IF

  // *TODO*: remove type inferences
  manageMediaSession_ = configuration_in.manageMediaSession;
  if (configuration_in.session)
  {
    ULONG reference_count = configuration_in.session->AddRef ();
    mediaSession_ = configuration_in.session;
  } // end IF

  if (COM_initialized) Common_Tools::finalizeCOM ();

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::QueryInterface (const IID& IID_in,
                                                                              void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    QITABENT (OWN_TYPE_T, IMFSampleGrabberSinkCallback2),
#else
    QITABENT (OWN_TYPE_T, IMFSampleGrabberSinkCallback),
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    QITABENT (OWN_TYPE_T, IMFAsyncCallback),
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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
ULONG
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0)
  //  delete this;

  return count;
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::OnEvent (DWORD streamIndex_in,
//                                                          IMFMediaEvent* mediaEvent_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnEvent"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//  ACE_UNUSED_ARG (mediaEvent_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_OK);
//  ACE_NOTREACHED (return S_OK;)
//}
// 
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::OnFlush (DWORD streamIndex_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnFlush"));
//
//  ACE_UNUSED_ARG (streamIndex_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_OK);
//  ACE_NOTREACHED (return S_OK;)
//}
// 
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::OnReadSample (HRESULT result_in,
//                                                               DWORD streamIndex_in,
//                                                               DWORD streamFlags_in,
//                                                               LONGLONG timeStamp_in,
//                                                               IMFSample* sample_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnReadSample"));
//
//  int result = -1;
//  ACE_Message_Block* message_block_p = NULL;
//  result = inherited::getq (message_block_p, NULL);
//  if (result == -1)
//  {
//    int error = ACE_OS::last_error ();
//    if (error != ESHUTDOWN)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
//                  inherited::name ()));
//
//    // clean up
//    message_block_p->release ();
//
//    return E_FAIL;
//  } // end IF
//  ACE_ASSERT (message_block_p);
//
//  MessageType* message_p = NULL;
//  try
//  {
//    message_p = dynamic_cast<MessageType*> (message_block_p);
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in dynamic_cast<MessageType*>(0x%@), continuing\n"),
//                message_block_p));
//    message_p = NULL;
//  }
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to dynamic_cast<MessageType*>(0x%@), aborting\n"),
//                message_block_p));
//
//    // clean up
//    message_block_p->release ();
//
//    return E_FAIL;
//  } // end IF
//  ACE_ASSERT (message_p);
//
//  typename MessageType::DATA_T& data_r =
//    const_cast<typename MessageType::DATA_T&> (message_p->get ());
//  ACE_ASSERT (!data_r.sample);
//
//  //DWORD total_length = 0;
//  //HRESULT result_2 = sample_in->GetTotalLength (&total_length);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFSample::GetTotalLength(): \"%m\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  return result_2;
//  //} // end IF
//  DWORD buffer_count = 0;
//  HRESULT result_2 = sample_in->GetBufferCount (&buffer_count);
//  ACE_ASSERT (buffer_count == 1);
//  result_2 = sample_in->GetBufferByIndex (0,
//                                          &data_r.sample);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSample::GetBufferByIndex(0): \"%m\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//
//    // clean up
//    message_p->release ();
//
//    return result_2;
//  } // end IF
//  ACE_ASSERT (data_r.sample);
//  data_r.sampleTime = timeStamp_in;
//
//  BYTE* buffer_p = NULL;
//  DWORD maximum_length, current_length;
//  result_2 = data_r.sample->Lock (&buffer_p,
//                                  &maximum_length,
//                                  &current_length);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSample::Lock(): \"%m\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//
//    // clean up
//    data_r.sample->Release ();
//    message_p->release ();
//
//    return result_2;
//  } // end IF
//  ACE_ASSERT (buffer_p);
//
//  message_p->base (reinterpret_cast<char*> (buffer_p),
//                    current_length,
//                    ACE_Message_Block::DONT_DELETE);
//  message_p->wr_ptr (current_length);
//
//  return S_OK;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnClockStart (MFTIME systemClockTime_in,
                                                                            LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnClockStart"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (clockStartOffset_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnClockStop"));

  ACE_UNUSED_ARG (systemClockTime_in);

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnClockPause"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnClockRestart"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnClockSetRate (MFTIME systemClockTime_in,
                                                                              float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnClockSetRate"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (playbackRate_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnProcessSample (REFGUID majorMediaType_in,
                                                                               DWORD flags_in,
                                                                               LONGLONG timeStamp_in,
                                                                               LONGLONG duration_in,
                                                                               const BYTE* buffer_in,
                                                                               DWORD bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnProcessSample"));

  IMFAttributes* attributes_p = NULL;
  return OnProcessSampleEx (majorMediaType_in,
                            flags_in,
                            timeStamp_in,
                            duration_in,
                            buffer_in,
                            bufferSize_in,
                            attributes_p);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnProcessSampleEx (REFGUID majorMediaType_in,
                                                                                 DWORD flags_in,
                                                                                 LONGLONG timeStamp_in,
                                                                                 LONGLONG duration_in,
                                                                                 const BYTE* buffer_in,
                                                                                 DWORD bufferSize_in,
                                                                                 IMFAttributes* attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnProcessSampleEx"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (duration_in);
  ACE_UNUSED_ARG (attributes_in);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);

  DataMessageType* message_p = NULL;
  int result = -1;

  message_p =
    inherited::allocateMessage (std::max (static_cast<DWORD> (inherited::configuration_->allocatorConfiguration->defaultBufferSize),
                                          bufferSize_in));
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%u), aborting\n"),
                inherited::mod_->name (),
                std::max (static_cast<DWORD> (inherited::configuration_->allocatorConfiguration->defaultBufferSize), bufferSize_in)));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (message_p);

  result = message_p->copy (reinterpret_cast<char*> (const_cast<BYTE*> (buffer_in)),
                            bufferSize_in);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%u): \"%m\", aborting\n"),
                inherited::mod_->name (),
                bufferSize_in));
    message_p->release (); message_p = NULL;
    return E_FAIL;
  } // end IF

  result = inherited::put_next (message_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n"),
                inherited::mod_->name ()));
    message_p->release (); message_p = NULL;
    return E_FAIL;
  } // end IF

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnSetPresentationClock"));

  // sanity check(s)
  if (presentationClock_)
  {
    presentationClock_->Release (); presentationClock_ = NULL;
  } // end IF

  ULONG reference_count = 0;
  if (presentationClock_in)
    reference_count = presentationClock_in->AddRef ();
  presentationClock_ = presentationClock_in;

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnShutdown"));

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
HRESULT
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::Invoke (IMFAsyncResult* result_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::Invoke"));

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
                  ACE_TEXT ("%s: received MESessionCapabilitiesChanged (now/delta): 0x%x/0x%x\n"),
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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
void
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (message_inout);

  message_inout->release (); message_inout = NULL;
  passMessageDownstream_out = false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
void
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::handleSessionMessage"));

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
      IMFMediaType* media_type_p = NULL;
      MediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));

      bool is_running = false;
      bool COM_initialized = Common_Tools::initializeCOM ();
      ULONG reference_count = 0;
      IMFTopology* topology_p = NULL;
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

        IMFMediaSource* media_source_p = NULL;
        if (!initializeMediaSession (inherited::configuration_->deviceIdentifier,
                                     NULL,
                                     session_data_r.formats.back (),
                                     media_source_p,
                                     NULL,
                                     this,
                                     node_id,
                                     session_data_r.rendererNodeId,
                                     mediaSession_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Source_T::initializeMediaSession(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        media_source_p->Release (); media_source_p = NULL;
      } // end IF
continue_:
      ACE_ASSERT (mediaSession_);

      if (!Stream_MediaFramework_MediaFoundation_Tools::getTopology (mediaSession_,
                                                                     topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getTopology(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (topology_p);
      if (!Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                                node_id))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                         node_id,
                                                                         media_type_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(%q), aborting\n"),
                    inherited::mod_->name (),
                    node_id));
        goto error;
      } // end IF
      ACE_ASSERT (media_type_p);
      topology_p->Release (); topology_p = NULL;

      inherited2::getMediaType (media_type_p,
                                media_type_2);
      session_data_r.formats.push_back (media_type_2);
      media_type_p->Release (); media_type_p = NULL;

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
          {
            error = ACE_OS::last_error ();
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

          //if (unlikely (delayStart_))
          //  goto continue_3;

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

//continue_3:
      if (COM_initialized) Common_Tools::finalizeCOM ();

      break;

error:
      if (media_type_p)
        media_type_p->Release ();
      if (topology_p)
        topology_p->Release ();
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
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
bool
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::initializeMediaSession (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                                                      HWND windowHandle_in,
                                                                                      const IMFMediaType* IMFMediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                                                      IMFMediaSourceEx*& IMFMediaSource_inout,
#else
                                                                                      IMFMediaSource*& IMFMediaSource_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                                                                      IDirect3DDeviceManager9* IDirect3DDeviceManager_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                                                      IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback_in,
#else
                                                                                      IMFSampleGrabberSinkCallback* IMFSampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                                                      TOPOID& sampleGrabberSinkNodeId_out,
                                                                                      TOPOID& rendererNodeId_out//,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                                      ,IMFMediaSession*& IMFMediaSession_inout)
#else
                                                                                      )
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::initializeMediaSession"));

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;

  HRESULT result = E_FAIL;
  bool release_media_session = false;
  bool release_media_source = false;
  if (!IMFMediaSource_inout)
  { ACE_ASSERT (deviceIdentifier_in.identifierDiscriminator == Stream_Device_Identifier::GUID);
    if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (deviceIdentifier_in.identifier._guid,
                                                                      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                      IMFMediaSource_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(\"%s\"), aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in.identifier._guid).c_str ())));
      return false;
    } // end IF
    release_media_source = true;
  } // end IF
  ACE_ASSERT (IMFMediaSource_inout);

  IMFTopology* topology_p = NULL;
  //if (!Stream_Device_Tools::loadRendererTopology (device_name,
  //                                                IMFMediaType_in,
  //                                                IMFSampleGrabberSinkCallback_in,
  //                                                windowHandle_in,
  //                                                sampleGrabberSinkNodeId_out,
  //                                                rendererNodeId_out,
  //                                                topology_p))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererTopology(), aborting\n")));
  //  goto error;
  //} // end IF
  ACE_ASSERT (topology_p);

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (!IMFMediaSession_inout)
  {
    IMFAttributes* attributes_p = NULL;
    result = MFCreateAttributes (&attributes_p, 4);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to MFCreateAttributes(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
    ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
    //ACE_ASSERT (SUCCEEDED (result));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
    result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
    result = MFCreateMediaSession (attributes_p,
                                   &IMFMediaSession_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      attributes_p->Release (); attributes_p = NULL;
      goto error;
    } // end IF
    release_media_session = true;
    attributes_p->Release (); attributes_p = NULL;
  } // end IF

  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                          //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                          //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
  result = IMFMediaSession_inout->SetTopology (topology_flags,
                                               topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  topology_p->Release (); topology_p = NULL;

  return true;

error:
  if (topology_p)
    topology_p->Release ();
  if (release_media_session)
  {
    IMFMediaSession_inout->Release (); IMFMediaSession_inout = NULL;
  } // end IF
  if (release_media_source)
  {
    IMFMediaSource_inout->Release (); IMFMediaSource_inout = NULL;
  } // end IF

  return false;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          typename UserDataType>
void
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType>::finalizeMediaSession ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::finalizeMediaSession"));

}
