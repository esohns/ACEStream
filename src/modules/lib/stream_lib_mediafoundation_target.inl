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

//#include "stream_dev_mediafoundation_tools.h"

#include "stream_lib_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::Stream_MediaFramework_MediaFoundation_Target_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , sessionData_ (NULL)
 , isFirst_ (false)
 , baseTimeStamp_ (0)
 , mediaSession_ (NULL)
 , presentationClock_ (NULL)
 , referenceCount_ (0)
 , sampleGrabberSinkNodeId_ (0)
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
          typename SessionDataContainerType>
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::~Stream_MediaFramework_MediaFoundation_Target_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::~Stream_MediaFramework_MediaFoundation_Target_T"));

  inherited::queue_.waitForIdleState ();

  if (presentationClock_)
    presentationClock_->Release ();
  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    mediaSession_->Release ();
    mediaSession_ = NULL;
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                            Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::initialize"));

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
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;
  } // end IF

  if (inherited::isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    inherited::queue_.waitForIdleState ();

    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    referenceCount_ = 0;

    if (presentationClock_)
    {
      presentationClock_->Release ();
      presentationClock_ = NULL;
    } // end IF
    if (mediaSession_)
    {
      HRESULT result = mediaSession_->Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      mediaSession_->Release ();
      mediaSession_ = NULL;
    } // end IF
    sampleGrabberSinkNodeId_ = 0;
  } // end IF

  //mediaType_ = &configuration_->mediaType;

  return inherited::initialize (configuration_in,
                                allocator_in);
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType>
//const ConfigurationType&
//Stream_MediaFramework_MediaFoundation_Target_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     SessionDataContainerType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::get"));
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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::QueryInterface (const IID& IID_in,
                                                                                void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    //QITABENT (OWN_TYPE_T, IMFSourceReaderCallback),
    QITABENT (OWN_TYPE_T, IMFSampleGrabberSinkCallback),
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
          typename SessionDataType,
          typename SessionDataContainerType>
ULONG
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
ULONG
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0)
  //delete this;

  return count;
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_Target_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType>::OnEvent (DWORD streamIndex_in,
//                                                                IMFMediaEvent* mediaEvent_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnEvent"));
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
//          typename SessionDataType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_Target_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType>::OnFlush (DWORD streamIndex_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnFlush"));
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
//          typename SessionDataType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_Target_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType>::OnReadSample (HRESULT result_in,
//                                                               DWORD streamIndex_in,
//                                                               DWORD streamFlags_in,
//                                                               LONGLONG timeStamp_in,
//                                                               IMFSample* sample_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnReadSample"));
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
//  DataMessageType* message_p = NULL;
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
//  //              ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
//                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
//                ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnClockStart (MFTIME systemClockTime_in,
                                                                              LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnClockStart"));

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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnClockStop"));

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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnClockPause"));

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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnClockRestart"));

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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnClockSetRate (MFTIME systemClockTime_in,
                                                                                float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnClockSetRate"));

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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnProcessSample (const struct _GUID& majorMediaType_in,
                                                                                 DWORD flags_in,
                                                                                 LONGLONG timeStamp_in,
                                                                                 LONGLONG duration_in,
                                                                                 const BYTE* buffer_in,
                                                                                 DWORD bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnProcessSample"));

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
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnProcessSampleEx (const struct _GUID& majorMediaType_in,
                                                                                   DWORD flags_in,
                                                                                   LONGLONG timeStamp_in,
                                                                                   LONGLONG duration_in,
                                                                                   const BYTE* buffer_in,
                                                                                   DWORD bufferSize_in,
                                                                                   IMFAttributes* attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnProcessSampleEx"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (timeStamp_in);
  ACE_UNUSED_ARG (duration_in);
  ACE_UNUSED_ARG (buffer_in);
  ACE_UNUSED_ARG (bufferSize_in);
  ACE_UNUSED_ARG (attributes_in);

  DataMessageType* message_p = NULL;
  int result = -1;
  //HRESULT result_2 = E_FAIL;

  if (isFirst_)
  {
    isFirst_ = false;
    baseTimeStamp_ = timeStamp_in;
  } // end IF
  timeStamp_in -= baseTimeStamp_;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_->allocatorConfiguration);

  // *TODO*: remove type inference
  message_p =
    inherited::allocateMessage (configuration_->allocatorConfiguration->defaultBufferSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_TaskBase_T::allocateMessage(%d), aborting\n"),
                inherited::mod_->name (),
                configuration_->allocatorConfiguration->defaultBufferSize));
    goto error;
  } // end IF
  ACE_ASSERT (message_p);
  ACE_ASSERT (message_p->capacity () >= bufferSize_in);

  // *TODO*: copy this data into the message buffer ?
  message_p->base (reinterpret_cast<char*> (const_cast<BYTE*> (buffer_in)),
                                            bufferSize_in,
                                            ACE_Message_Block::DONT_DELETE);
  message_p->wr_ptr (bufferSize_in);

  result = inherited::putq (message_p, NULL);
  if (result == -1)
  {
    int error = ACE_OS::last_error ();
    if (error != ESHUTDOWN)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
    goto error;
  } // end IF

  return S_OK;

error:
  if (message_p)
    message_p->release ();

  return E_FAIL;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnSetPresentationClock"));

  // sanity check(s)
  if (presentationClock_)
  {
    presentationClock_->Release ();
    presentationClock_ = NULL;
  } // end IF

  ULONG reference_count = presentationClock_in->AddRef ();
  ACE_UNUSED_ARG (reference_count);
  presentationClock_ = presentationClock_in;

  return S_OK;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::OnShutdown"));

  return S_OK;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                   bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::handleDataMessage"));

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  ACE_Message_Block* message_block_p = message_inout->duplicate ();
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::duplicate(): \"%m\", returning\n")));
    return;
  } // end IF
  int result = inherited::queue_.enqueue_tail (message_block_p, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue::enqueue_tail(): \"%m\", returning\n")));

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
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::handleSessionMessage"));

  //int result = -1;

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
      ACE_ASSERT (!sessionData_);

      sessionData_ =
        &const_cast<SessionDataContainerType&> (message_inout->getR ());
      sessionData_->increase ();
      typename SessionDataContainerType::DATA_T& session_data_r =
        const_cast<typename SessionDataContainerType::DATA_T&> (sessionData_->getR ());

      bool COM_initialized = false;

      HRESULT result_2 = CoInitializeEx (NULL,
                                         (COINIT_MULTITHREADED    |
                                          COINIT_DISABLE_OLE1DDE  |
                                          COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!mediaSession_);
      ACE_ASSERT (session_data_r.inputFormat);
      ACE_ASSERT (!(inherited::configuration_->session &&
                    session_data_r.session));

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
      } // end IF

      //WCHAR* symbolic_link_p = NULL;
      //UINT32 symbolic_link_size = 0;
      if (!initialize_MediaFoundation (*session_data_r.inputFormat,
                                       this,
                                       sampleGrabberSinkNodeId_,
                                       mediaSession_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_MediaFoundation(), returning\n")));
        goto error;
      } // end IF
      ACE_ASSERT (mediaSession_);
      ACE_ASSERT (sampleGrabberSinkNodeId_);

      break;

error:
      if (mediaSession_)
      {
        result_2 = mediaSession_->Shutdown ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF

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
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

//continue_:
      finalize_MediaFoundation (); // stop 'streaming thread'

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      if (mediaSession_)
      {
        result_2 = mediaSession_->Shutdown ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      sampleGrabberSinkNodeId_ = 0;

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
          typename SessionDataContainerType>
bool
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::initialize_MediaFoundation (const struct _AMMediaType& mediaType_in,
                                                                                            const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                                                            TOPOID& sampleGrabberSinkNodeId_out,
                                                                                            IMFMediaSession*& IMFMediaSession_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::initialize_MediaFoundation"));

  // initialize return value(s)
  sampleGrabberSinkNodeId_out = 0;

  // sanity check(s)
  ACE_ASSERT (IMFMediaSession_inout);

  IMFMediaType* media_type_p = NULL;
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
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);

  result = MFCreateMediaType (&media_type_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = MFInitMediaTypeFromAMMediaType (media_type_p,
                                           &mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFInitMediaTypeFromAMMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  if (!Stream_Module_Device_MediaFoundation_Tools::addGrabber (media_type_p,
                                                               IMFSampleGrabberSinkCallback2_in,
                                                               topology_p,
                                                               node_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_MediaFoundation_Tools::addGrabber(), aborting\n")));
    goto error;
  } // end IF
  result = topology_p->GetNodeByID (node_id,
                                    &topology_node_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (topology_node_p);
  topology_node_p->Release ();
  topology_node_p = NULL;

  result = IMFMediaSession_inout->SetTopology (topology_flags,
                                               topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  // debug info
#if defined (_DEBUG)
  Stream_Module_Device_MediaFoundation_Tools::dump (topology_p);
#endif
  topology_p->Release ();
  topology_p = NULL;

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_p)
    topology_p->Release ();

  return false;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataType,
                                     SessionDataContainerType>::finalize_MediaFoundation ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Target_T::finalize_MediaFoundation"));

}
