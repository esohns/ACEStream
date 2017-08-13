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

//#include "stream_dev_defines.h"
//#include "stream_dev_tools.h"

#include "stream_lib_defines.h"

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
                                               UserDataType>::Stream_MediaFramework_MediaFoundation_Source_T ()
 : inherited ()
 , sessionData_ (NULL)
 , isFirst_ (false)
 , baseTimeStamp_ (0)
 , mediaSession_ (NULL)
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

  inherited::queue_.waitForIdleState ();

  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
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
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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

    if (mediaSession_)
    {
      result = mediaSession_->Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      mediaSession_->Release ();
      mediaSession_ = NULL;
    } // end IF

    inherited::isInitialized_ = false;
  } // end IF

  //mediaType_ = &configuration_->mediaType;

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
    //QITABENT (OWN_TYPE_T, IMFSourceReaderCallback),
    QITABENT (OWN_TYPE_T, IMFSampleGrabberSinkCallback2),
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
  if (count == 0);
  //delete this;

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
//  //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
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
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
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
//                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
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
                                     UserDataType>::OnProcessSample (const struct _GUID& majorMediaType_in,
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
                                     UserDataType>::OnProcessSampleEx (const struct _GUID& majorMediaType_in,
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

  MessageType* message_p = NULL;
  int result = -1;
  HRESULT result_2 = E_FAIL;

  if (isFirst_)
  {
    isFirst_ = false;
    baseTimeStamp_ = timeStamp_in;
  } // end IF
  timeStamp_in -= baseTimeStamp_;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_->streamConfiguration);

  // *TODO*: remove type inference
  message_p =
    allocateMessage (configuration_->streamConfiguration->bufferSize);
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::allocateMessage(%d) failed: \"%m\", aborting\n"),
                configuration_->streamConfiguration->bufferSize));
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
        inherited::name ()));
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
    presentationClock_->Release ();
    presentationClock_ = NULL;
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
      ACE_ASSERT (!sessionData_);
      // *TODO*: remove type inference
      ACE_ASSERT (inherited::configuration_->streamConfiguration);

      sessionData_ =
        &const_cast<SessionDataType&> (message_inout->get ());
      sessionData_->increase ();
      typename SessionDataType::DATA_T& session_data_r =
        const_cast<typename SessionDataType::DATA_T&> (sessionData_->get ());

      bool COM_initialized = false;
      bool is_running = false;

      HRESULT result_2 = CoInitializeEx (NULL,
                                         (COINIT_MULTITHREADED    |
                                          COINIT_DISABLE_OLE1DDE  |
                                          COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!session_data_r.direct3DDevice);
      ACE_ASSERT (!session_data_r.resetToken);

      //if (inherited::configuration_->direct3DDevice)
      //{
      //  inherited::configuration_->direct3DDevice->AddRef ();
      //  Direct3DDevice_ = inherited::configuration_->direct3DDevice;

      //  // sanity check(s)
      //  ACE_ASSERT (session_data_r.resetToken);
      //} // end IF
      // *TODO*: remove type inferences
      IDirect3DDeviceManager9* direct3D_manager_p = NULL;
      struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters;
      if (!Stream_Module_Device_Tools::getDirect3DDevice (inherited::configuration_->window,
                                                          session_data_r.format,
                                                          session_data_r.direct3DDevice,
                                                          d3d_presentation_parameters,
                                                          direct3D_manager_p,
                                                          session_data_r.resetToken))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::getDirect3DDevice(), aborting\n")));
        goto error;
      } // end IF
      ACE_ASSERT (session_data_r.direct3DDevice);
      ACE_ASSERT (session_data_r.resetToken);

      // sanity check(s)
      ACE_ASSERT (!mediaSession_);

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
      WCHAR* symbolic_link_p = NULL;
      UINT32 symbolic_link_size = 0;
      TOPOID node_id = 0;
      if (!initialize_MediaFoundation (inherited::configuration_->window,
                                       session_data_r.format,
                                       inherited::configuration_->mediaSource,
                                       symbolic_link_p,
                                       symbolic_link_size,
                                       direct3D_manager_p,
                                       this,
                                       node_id,
                                       session_data_r.rendererNodeId,
                                       mediaSession_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_MediaFoundation(), returning\n")));
        goto error;
      } // end IF
      CoTaskMemFree (symbolic_link_p);
      ACE_ASSERT (mediaSession_);

//do_run:
      // start displaying video data (asynchronous mode)
      //DWORD actual_stream_index = 0;
      //DWORD stream_flags = 0;
      //LONGLONG timestamp = 0;
      //IMFSample* sample_p = NULL;
      //result_2 = sourceReader_->ReadSample (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                      0,
      //                                      NULL,
      //                                      NULL,
      //                                      NULL,
      //                                      NULL);
      //if (FAILED (result_2))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFSourceReader::ReadSample(): \"%s\", returning\n"),
      //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      //  goto error;
      //} // end IF
      //is_running = true;

      break;

error:
      if (mediaSession_)
      {
        result = mediaSession_->Shutdown ();
        if (FAILED (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      session_data_r.aborted = true;

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
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

//continue_:
      finalize_MediaFoundation ();

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      if (mediaSession_)
      {
        result = mediaSession_->Shutdown ();
        if (FAILED (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      inherited::shutdown ();

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
DataMessageType*
Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionDataContainerType,
                                     SessionDataType,
                                     MediaType,
                                     UserDataType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::configuration_->streamConfiguration);

  // initialize return value(s)
  MessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (inherited::configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try {
      // *TODO*: remove type inference
      message_p =
        static_cast<MessageType*> (inherited::configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !inherited::configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      MessageType (requestedSize_in));
  if (!message_p)
  {
    if (inherited::configuration_->streamConfiguration->messageAllocator)
    {
      if (inherited::configuration_->streamConfiguration->messageAllocator->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate MessageType(%u): \"%m\", aborting\n"),
                    requestedSize_in));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate MessageType(%u): \"%m\", aborting\n"),
                  requestedSize_in));
  } // end IF

  return message_p;
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
                                     UserDataType>::initialize_MediaFoundation (const HWND windowHandle_in,
                                                                                const IMFMediaType* IMFMediaType_in,
                                                                                IMFMediaSource*& IMFMediaSource_inout,
                                                                                WCHAR*& symbolicLink_out,
                                                                                UINT32& symbolicLinkSize_out,
                                                                                const IDirect3DDeviceManager9* IDirect3DDeviceManager_in,
                                                                                const IMFSampleGrabberSinkCallback2* IMFSampleGrabberSinkCallback2_in,
                                                                                TOPOID& sampleGrabberSinkNodeId_out,
                                                                                TOPOID& rendererNodeId_out,
                                                                                IMFMediaSession*& IMFMediaSession_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Source_T::initialize_MediaFoundation"));

  // initialize return value(s)
  if (symbolicLinkSize_out)
  {
    // sanity check(s)
    ACE_ASSERT (symbolicLink_out);

    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
    symbolicLinkSize_out = 0;
  } // end IF
  sampleGrabberSinkNodeId_out = 0;
  rendererNodeId_out = 0;

  HRESULT result = E_FAIL;
  bool release_media_session = false;
  bool release_media_source = false;
  std::string device_name;
  IMFMediaSource* media_source_p = IMFMediaSource_inout;
  if (!media_source_p)
  {
    if (!Stream_Module_Device_Tools::getMediaSource (device_name,
                                                     IMFMediaSource_inout,
                                                     symbolicLink_out,
                                                     symbolicLinkSize_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (device_name.c_str ())));
      return false;
    } // end IF
    media_source_p = IMFMediaSource_inout;
    release_media_source = true;
  } // end IF
  ACE_ASSERT (media_source_p);

  IMFTopology* topology_p = NULL;
  //if (!Stream_Module_Device_Tools::loadTargetRendererTopology (IMFMediaType_in,
  //                                                             IMFSourceReaderCallback2_in,
  //                                                             windowHandle_in,
  //                                                             IMFTopology_out))
  if (!Stream_Module_Device_Tools::loadRendererTopology (device_name,
                                                         IMFMediaType_in,
                                                         IMFSampleGrabberSinkCallback2_in,
                                                         windowHandle_in,
                                                         sampleGrabberSinkNodeId_out,
                                                         rendererNodeId_out,
                                                         topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererTopology(), aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (topology_p);

  if (!IMFMediaSession_inout)
  {
    IMFAttributes* attributes_p = NULL;
    result = MFCreateAttributes (&attributes_p, 4);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
    ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
    //ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    result = MFCreateMediaSession (attributes_p,
                                   &IMFMediaSession_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      attributes_p->Release ();

      goto error;
    } // end IF
    attributes_p->Release ();
    release_media_session = true;
  } // end IF
  ACE_ASSERT (IMFMediaSession_inout);

  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE);// |
                          //MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                          //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
  result = IMFMediaSession_inout->SetTopology (topology_flags,
                                               topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  topology_p->Release ();

  return true;

error:
  if (topology_p)
    topology_p->Release ();
  if (release_media_session)
  {
    IMFMediaSession_inout->Release ();
    IMFMediaSession_inout = NULL;
  } // end IF
  if (release_media_source)
  {
    IMFMediaSource_inout->Release ();
    IMFMediaSource_inout = NULL;
  } // end IF
  if (symbolicLinkSize_out)
  {
    // sanity check(s)
    ACE_ASSERT (symbolicLink_out);

    CoTaskMemFree (symbolicLink_out);
    symbolicLink_out = NULL;
    symbolicLinkSize_out = 0;
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
