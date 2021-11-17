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
 , isFirst_ (false)
 , baseTimeStamp_ (0)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , mediaSession_ (NULL)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
 , referenceCount_ (1)
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
    mediaSession_->Release (); mediaSession_ = NULL;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
                                               UserDataType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::initialize"));

  // initialize COM ?
  HRESULT result = E_FAIL;
  static bool first_run = true;
  bool COM_initialized = false;
  if (first_run)
  {
    first_run = false;

    result = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;
  } // end IF

  if (inherited::isInitialized_)
  {
    baseTimeStamp_ = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    if (mediaSession_)
    {
      mediaSession_->Release (); mediaSession_ = NULL;
    } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    if (presentationClock_)
      presentationClock_->Release ();
    presentationClock_ = NULL;
    referenceCount_ = 1;

    inherited::isInitialized_ = false;
  } // end IF

  return inherited::initialize (configuration_in);;
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename MediaType>
//const ConfigurationType&
//Stream_MediaFramework_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::get"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
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
                                               UserDataType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::OnClockStop"));

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

  DataMessageType* message_p = NULL;
  int result = -1;
  HRESULT result_2 = E_FAIL;

  if (unlikely (isFirst_))
  {
    isFirst_ = false;
    baseTimeStamp_ = timeStamp_in;
  } // end IF
  timeStamp_in -= baseTimeStamp_;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration);
  ACE_ASSERT (inherited::configuration_->allocatorConfiguration->defaultBufferSize);

  message_p =
    inherited::allocateMessage (inherited::configuration_->allocatorConfiguration->defaultBufferSize);
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), aborting\n"),
                inherited::mod_->name (),
                inherited::configuration_->allocatorConfiguration->defaultBufferSize));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (message_p);

  // sanity check(s)
  ACE_ASSERT (message_p->capacity () >= bufferSize_in);

  result = message_p->copy (reinterpret_cast<char*> (const_cast<BYTE*> (buffer_in)),
                            bufferSize_in);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
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

  ULONG reference_count = presentationClock_in->AddRef ();
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

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
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

  int result = -1;
  bool COM_initialized = false;
  HRESULT result_2 = E_FAIL;
  IRunningObjectTable* ROT_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!mediaSession_);

      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited::sessionData_->getR ());
      IMFMediaType* media_type_p = NULL;
      MediaType media_type_2;
      ACE_OS::memset (&media_type_2, 0, sizeof (MediaType));

      bool is_running = false;

      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (SUCCEEDED (result_2)) // RPC_E_CHANGED_MODE : 0x80010106L
        COM_initialized = true;
      else
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));

      ULONG reference_count = 0;
      if (inherited::configuration_->session)
      {
        reference_count = inherited::configuration_->session->AddRef ();
        mediaSession_ = inherited::configuration_->session;
        reference_count = mediaSession_->AddRef ();
        session_data_r.session = mediaSession_;
      } // end IF
      else if (session_data_r.session)
      {
        reference_count = session_data_r.session->AddRef ();
        mediaSession_ = session_data_r.session;
      } // end ELSE IF
      DWORD flags_i = MFSESSION_GETFULLTOPOLOGY_CURRENT;
      IMFTopology* topology_p = NULL;
      TOPOID node_id = 0;
      IMFTopologyNode* topology_node_p = NULL;
      IUnknown* unknown_p = NULL;
      IMFSampleGrabberSinkCallback* sample_grabber_cb_p = NULL;
      if (!mediaSession_)
      {
        IMFMediaSource* media_source_p = NULL;
        if (!initialize_MediaFoundation (NULL,
                                         session_data_r.formats.back (),
                                         media_source_p,
                                         NULL,
                                         this,
                                         node_id,
                                         session_data_r.rendererNodeId,
                                         mediaSession_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to initialize_MediaFoundation(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        media_source_p->Release (); media_source_p = NULL;
      } // end IF
      ACE_ASSERT (mediaSession_);
      result_2 = mediaSession_->GetFullTopology (flags_i,
                                                  0,
                                                  &topology_p);
      if (FAILED (result_2) || !topology_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        goto error;
      } // end IF
      if (!Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (topology_p,
                                                                                node_id))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      result_2 = topology_p->GetNodeByID (node_id,
                                          &topology_node_p);
      if (FAILED (result_2) || !topology_node_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    node_id,
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        goto error;
      } // end IF
      topology_p->Release (); topology_p = NULL;
      result_2 = topology_node_p->GetObject (&unknown_p);
      if (FAILED (result_2) || !unknown_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFTopologyNode::GetObject(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
        goto error;
      } // end IF
      topology_node_p->Release (); topology_node_p = NULL;
      //result_2 =
      //  unknown_p->QueryInterface (IID_PPV_ARGS (&sample_grabber_cb_p));
      //if (FAILED (result_2) || !unknown_p)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("%s: failed to IUnknown::QueryInterface(): \"%s\", aborting\n"),
      //              inherited::mod_->name (),
      //              ACE_TEXT (Common_Error_Tools::errorToString (result_2, true, false).c_str ())));
      //  goto error;
      //} // end IF
      unknown_p->Release (); unknown_p = NULL;

      break;

error:
      if (mediaSession_)
      {
        mediaSession_->Release (); mediaSession_ = NULL;
      } // end IF
      if (topology_p)
        topology_p->Release ();
      if (topology_node_p)
        topology_node_p->Release ();
      if (unknown_p)
        unknown_p->Release ();

      if (COM_initialized)
        CoUninitialize ();

      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      bool COM_initialized = false;
      HRESULT result_2 = CoInitializeEx (NULL,
                                         (COINIT_MULTITHREADED    |
                                          COINIT_DISABLE_OLE1DDE  |
                                          COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      finalize_MediaFoundation ();

      if (mediaSession_)
      {
        mediaSession_->Release (); mediaSession_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

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
                                               UserDataType>::initialize_MediaFoundation (HWND windowHandle_in,
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
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::initialize_MediaFoundation"));

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;

  HRESULT result = E_FAIL;
  bool release_media_session = false;
  bool release_media_source = false;
  std::string device_name;
  if (!IMFMediaSource_inout)
  {
    if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (device_name,
                                                                      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                                                      IMFMediaSource_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (device_name.c_str ())));
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
                  ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
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
                  ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
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
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
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
                                               UserDataType>::finalize_MediaFoundation ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::finalize_MediaFoundation"));

}
