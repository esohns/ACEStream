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

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_misc_defines.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::Stream_Misc_MediaFoundation_Source_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , isFirst_ (false)
 , isInitialized_ (false)
 , baseTimeStamp_ (0)
 , mediaSource_ (NULL)
 , referenceCount_ (0)
 , topology_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::Stream_Misc_MediaFoundation_Source_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::~Stream_Misc_MediaFoundation_Source_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::~Stream_Misc_MediaFoundation_Source_T"));

  inherited::queue_.waitForIdleState ();

  if (topology_)
    topology_->Release ();
  if (mediaSource_)
    mediaSource_->Release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
bool
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::initialize"));

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

  if (isInitialized_)
  {
    //ACE_DEBUG ((LM_WARNING,
    //            ACE_TEXT ("re-initializing...\n")));

    inherited::queue_.waitForIdleState ();

    configuration_ = NULL;
    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    referenceCount_ = 0;

    if (topology_)
    {
      topology_->Release ();
      topology_ = NULL;
    } // end IF
    if (mediaSource_)
    {
      mediaSource_->Release ();
      mediaSource_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  //mediaType_ = &configuration_->mediaType;

  isInitialized_ = true;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
const ConfigurationType&
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::QueryInterface (const IID& IID_in,
                                                                 void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::QueryInterface"));

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
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
ULONG
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::AddRef ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::AddRef"));

  return InterlockedIncrement (&referenceCount_);
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
ULONG
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::Release"));

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
//Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::OnEvent (DWORD streamIndex_in,
//                                                          IMFMediaEvent* mediaEvent_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnEvent"));
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
//Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::OnFlush (DWORD streamIndex_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnFlush"));
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
//Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
//                                     MessageType,
//                                     ConfigurationType,
//                                     SessionDataType,
//                                     MediaType>::OnReadSample (HRESULT result_in,
//                                                               DWORD streamIndex_in,
//                                                               DWORD streamFlags_in,
//                                                               LONGLONG timeStamp_in,
//                                                               IMFSample* sample_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnReadSample"));
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
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnClockStart (MFTIME systemClockTime_in,
                                                               LONGLONG clockStartOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnClockStart"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (clockStartOffset_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnClockStop (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnClockStop"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnClockPause (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnClockPause"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnClockRestart (MFTIME systemClockTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnClockRestart"));

  ACE_UNUSED_ARG (systemClockTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnClockSetRate (MFTIME systemClockTime_in,
                                                                 float playbackRate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnClockSetRate"));

  ACE_UNUSED_ARG (systemClockTime_in);
  ACE_UNUSED_ARG (playbackRate_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_OK);
  ACE_NOTREACHED (return S_OK;)
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnProcessSample (const struct _GUID& majorMediaType_in,
                                                                  DWORD flags_in,
                                                                  LONGLONG timeStamp_in,
                                                                  LONGLONG duration_in,
                                                                  const BYTE* buffer_in,
                                                                  DWORD bufferSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnProcessSample"));

  ACE_UNUSED_ARG (majorMediaType_in);
  ACE_UNUSED_ARG (flags_in);
  ACE_UNUSED_ARG (duration_in);

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
                ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::allocateMessage(%d) failed: \"%m\", aborting\n"),
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
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnSetPresentationClock (IMFPresentationClock* presentationClock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnSetPresentationClock"));

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
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
HRESULT
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::OnShutdown ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::OnShutdown"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::handleDataMessage (MessageType*& message_inout,
                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::handleDataMessage"));

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

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::handleSessionMessage"));

  int result = -1;
  IRunningObjectTable* ROT_p = NULL;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (isInitialized_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);
      // *TODO*: remove type inference
      ACE_ASSERT (configuration_->streamConfiguration);

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
      ACE_ASSERT (!mediaSource_);
      ACE_ASSERT (!topology_);

      // *TODO*: remove type inferences
      IDirect3DDeviceManager9* direct3D_manager_p = NULL;
      struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters;
      if (!Stream_Module_Device_Tools::getDirect3DDevice (configuration_->window,
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
      //if (inherited::configuration_->direct3DDevice)
      //{
      //  inherited::configuration_->direct3DDevice->AddRef ();
      //  Direct3DDevice_ = inherited::configuration_->direct3DDevice;

      //  // sanity check(s)
      //  ACE_ASSERT (session_data_r.resetToken);
      //} // end IF

      WCHAR* symbolic_link_p = NULL;
      UINT32 symbolic_link_size = 0;
      TOPOID node_id = 0;
      if (!initialize_MediaFoundation (configuration_->window,
                                       session_data_r.format,
                                       configuration_->mediaSource,
                                       symbolic_link_p,
                                       symbolic_link_size,
                                       direct3D_manager_p,
                                       this,
                                       node_id,
                                       session_data_r.rendererNodeId,
                                       topology_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to initialize_MediaFoundation(), returning\n")));
        goto error;
      } // end IF
      CoTaskMemFree (symbolic_link_p);
      ACE_ASSERT (topology_);

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
      if (COM_initialized)
        CoUninitialize ();

      session_data_r.aborted = true;

      break;
    }
    case STREAM_SESSION_END:
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
      finalize_MediaFoundation (); // stop 'streaming thread'

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      if (topology_)
      {
        topology_->Release ();
        topology_ = NULL;
      } // end IF

      if (mediaSource_)
      {
        mediaSource_->Release ();
        mediaSource_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
MessageType*
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::allocateMessage (unsigned int requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->streamConfiguration);

  // initialize return value(s)
  MessageType* message_p = NULL;

  // *TODO*: remove type inference
  if (configuration_->streamConfiguration->messageAllocator)
  {
allocate:
    try
    {
      // *TODO*: remove type inference
      message_p =
        static_cast<MessageType*> (configuration_->streamConfiguration->messageAllocator->malloc (requestedSize_in));
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                  requestedSize_in));
      message_p = NULL;
    }

    // keep retrying ?
    if (!message_p &&
        !configuration_->streamConfiguration->messageAllocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      MessageType (requestedSize_in));
  if (!message_p)
  {
    if (configuration_->streamConfiguration->messageAllocator)
    {
      if (configuration_->streamConfiguration->messageAllocator->block ())
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

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
bool
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::initialize_MediaFoundation (const HWND windowHandle_in,
                                                                             const IMFMediaType* IMFMediaType_in,
                                                                             IMFMediaSource*& IMFMediaSource_inout,
                                                                             WCHAR*& symbolicLink_out,
                                                                             UINT32& symbolicLinkSize_out,
                                                                             const IDirect3DDeviceManager9* IDirect3DDeviceManager_in,
                                                                             const IMFSampleGrabberSinkCallback* IMFSampleGrabberSinkCallback_in,
                                                                             TOPOID& sampleGrabberSinkNodeId_out,
                                                                             TOPOID& rendererNodeId_out,
                                                                             IMFTopology*& IMFTopology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::initialize_MediaFoundation"));

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
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
  } // end IF

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

  //if (!Stream_Module_Device_Tools::loadTargetRendererTopology (IMFMediaType_in,
  //                                                             IMFSourceReaderCallback_in,
  //                                                             windowHandle_in,
  //                                                             IMFTopology_out))
  if (!Stream_Module_Device_Tools::loadRendererTopology (device_name,
                                                         IMFMediaType_in,
                                                         IMFSampleGrabberSinkCallback_in,
                                                         windowHandle_in,
                                                         sampleGrabberSinkNodeId_out,
                                                         rendererNodeId_out,
                                                         IMFTopology_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::loadRendererTopology(), aborting\n")));
    goto error;
  } // end IF

  return true;

error:
  if (IMFTopology_out)
  {
    IMFTopology_out->Release ();
    IMFTopology_out = NULL;
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
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename MediaType>
void
Stream_Misc_MediaFoundation_Source_T<SessionMessageType,
                                     MessageType,
                                     ConfigurationType,
                                     SessionDataType,
                                     MediaType>::finalize_MediaFoundation ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_MediaFoundation_Source_T::finalize_MediaFoundation"));

  inherited::shutdown ();
}
