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

#include <d3d9types.h>
#include <mferror.h>
#include <mfidl.h>

#include <ace/Log_Msg.h>
#include <ace/OS.h>

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::Stream_Vis_Target_MediaFoundation_T ()
 : inherited ()
 , device_ (NULL)
 , mediaSession_ (NULL)
 , streamSink_ (NULL)
 //, videoSampleAllocator_ (NULL)
 , videoDisplayControl_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::Stream_Vis_Target_MediaFoundation_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::~Stream_Vis_Target_MediaFoundation_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::~Stream_Vis_Target_MediaFoundation_T"));

  //if (videoSampleAllocator_)
  //  videoSampleAllocator_->Release ();
  if (videoDisplayControl_)
    videoDisplayControl_->Release ();
  if (streamSink_)
    streamSink_->Release ();
  if (mediaSession_)
  {
    HRESULT result = mediaSession_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    mediaSession_->Release ();
  } // end IF
  if (device_)
    device_->Release ();
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
Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);

  //HRESULT result = E_FAIL;
  //typename const MessageType::DATA_T& message_data_r = message_inout->get ();

  //// sanity check(s)
  //ACE_ASSERT (streamSink_);
  ////ACE_ASSERT (videoSampleAllocator_);
  //ACE_ASSERT (message_data_r.sample);

  //// *NOTE*: EVR does not accept plain IMFSamples; only 'video samples'
  //IMFSample* sample_p = NULL;
  //result = videoSampleAllocator_->AllocateSample (&sample_p);
  //if (FAILED (result)) // MF_E_SAMPLEALLOCATOR_EMPTY: 0xC00D4A3EL
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFVideoSampleAllocator::AllocateSample(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return;
  //} // end IF
  //ACE_ASSERT (sample_p);

  //result = sample_p->SetSampleTime (message_data_r.sampleTime);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::SetSampleTime(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  sample_p->Release ();

  //  return;
  //} // end IF

  //DWORD count = 0;
  //result = message_data_r.sample->GetBufferCount (&count);
  //ACE_ASSERT (SUCCEEDED (result));
  //ACE_ASSERT (count == 1);
  //IMFMediaBuffer* media_buffer_p = NULL;
  //result = message_data_r.sample->GetBufferByIndex (0, &media_buffer_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::GetBufferByIndex(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  sample_p->Release ();

  //  return;
  //} // end IF
  //ACE_ASSERT (media_buffer_p);
  //result = sample_p->AddBuffer (media_buffer_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::AddBuffer(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  media_buffer_p->Release ();
  //  sample_p->Release ();

  //  return;
  //} // end IF
  //media_buffer_p->Release ();

  //result = streamSink_->ProcessSample (sample_p);
  //result = streamSink_->ProcessSample (message_data_r.sample);
  //if (FAILED (result)) // E_NOINTERFACE
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFStreamSink::ProcessSample(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  //sample_p->Release ();

  //  return;
  //} // end IF
  //sample_p->Release ();
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
Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (session_data_container_r.get ());

      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!device_);
      ACE_ASSERT (!streamSink_);
      ACE_ASSERT (!mediaSession_);
      ACE_ASSERT (!videoDisplayControl_);
      ACE_ASSERT (session_data_r.format);
      ACE_ASSERT (session_data_r.session);

      ULONG reference_count = 0;
      if (session_data_r.direct3DDevice)
      {
        reference_count = session_data_r.direct3DDevice->AddRef ();
        device_ = session_data_r.direct3DDevice;
      } // end IF
      else
      {
        struct _D3DPRESENT_PARAMETERS_ presentation_parameters;
        ACE_OS::memset (&presentation_parameters,
                        0,
                        sizeof (struct _D3DPRESENT_PARAMETERS_));
        IDirect3DDeviceManager9* direct3D_manager_p = NULL;
        UINT reset_token = 0;
        if (!Stream_Module_Device_Tools::getDirect3DDevice (configuration_->window,
                                                            session_data_r.format,
                                                            device_,
                                                            presentation_parameters,
                                                            direct3D_manager_p,
                                                            reset_token))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_Direct3D(), aborting\n")));
          goto error;
        } // end IF
        ACE_ASSERT (direct3D_manager_p);
        direct3D_manager_p->Release ();
      } // end ELSE

      if (session_data_r.session)
      {
        reference_count = session_data_r.session->AddRef ();
        mediaSession_ = session_data_r.session;
      } // end IF

      IMFMediaSink* media_sink_p = NULL;
      if (!initialize_MediaFoundation (configuration_->window,
                                       configuration_->area,
                                       //media_type_p,
                                       configuration_->rendererNodeId,
                                       media_sink_p,
                                       videoDisplayControl_,
                                       //videoSampleAllocator_,
                                       mediaSession_))
      {
        ACE_DEBUG ((LM_ERROR,
                   ACE_TEXT ("failed to initialize_MediaFoundation(), aborting\n")));
        goto error;
      } // end IF
      ACE_ASSERT (media_sink_p);
      ACE_ASSERT (videoDisplayControl_);
      ACE_ASSERT (mediaSession_);

      DWORD count = 0;
      result = media_sink_p->GetStreamSinkCount (&count);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSink::GetStreamSinkCount(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (count > 0);
      result = media_sink_p->GetStreamSinkByIndex (0, &streamSink_);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSink::GetStreamSinkByIndex(0): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (streamSink_);
      media_sink_p->Release ();
      media_sink_p = NULL;

      //IMediaTypeHandler* media_type_handler_p = NULL;
      //result = streamSink_->GetMediaTypeHandler (&media_type_handler_p);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFStreamSink::GetMediaTypeHandler(): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      //  goto error;
      //} // end IF
      //IMFMediaType* media_type_p = NULL;
      //result = MFCreateMediaType (&media_type_p);
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //  // clean up
      //  media_type_handler_p->Release ();

      //  goto error;
      //} // end IF
      //result = media_type_p->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
      //ACE_ASSERT (SUCCEEDED (result));
      //result = media_type_p->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB24);
      //ACE_ASSERT (SUCCEEDED (result));
      //result = media_type_p->SetUINT32 (MF_MT_INTERLACE_MODE,
      //  MFVideoInterlace_Progressive);
      //ACE_ASSERT (SUCCEEDED (result));
      //result = media_type_p->SetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
      //ACE_ASSERT (SUCCEEDED (result));
      //result =
      //  MFSetAttributeRatio (media_type_p, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
      //ACE_ASSERT (SUCCEEDED (result));
      //result =
      //  Stream_Module_Device_Tools::copyAttribute (session_data_r.format,
      //                                             media_type_p,
      //                                             MF_MT_FRAME_SIZE);
      //ACE_ASSERT (SUCCEEDED (result));
      //result =
      //  Stream_Module_Device_Tools::copyAttribute (session_data_r.format,
      //                                             media_type_p,
      //                                             MF_MT_FRAME_RATE);
      //result = media_type_handler_p->SetCurrentMediaType (media_type_p);
      //if (FAILED (result)) // MF_E_INVALIDMEDIATYPE: 0xC00D36B4L
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaTypeHandler::SetCurrentMediaType(\"%s\"): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //  // clean up
      //  media_type_p->Release ();
      //  media_type_handler_p->Release ();

      //  goto error;
      //} // end IF
      //media_type_p->Release ();
      //media_type_p = NULL;
      //media_type_handler_p->Release ();
      //media_type_handler_p = NULL;

      //result = streamSink_->QueryInterface (IID_PPV_ARGS (&get_service_p));
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFStreamSink::QueryInterface(IID_IMFGetService): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      //  goto error;
      //} // end IF
      //result =
      //  get_service_p->GetService (MR_VIDEO_ACCELERATION_SERVICE,
      //                             IID_PPV_ARGS (&IMFVideoSampleAllocator_out));
      //if (FAILED (result))
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFGetService::GetService(MR_VIDEO_ACCELERATION_SERVICE): \"%s\", aborting\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      //  goto error;
      //} // end IF
      //get_service_p->Release ();
      //get_service_p = NULL;

      //IMFVideoDisplayControl* video_display_control_p = NULL;
      if (configuration_->windowController)
      {
        if (videoDisplayControl_)
        {
          videoDisplayControl_->Release ();
          videoDisplayControl_ = NULL;
        } // end IF
        reference_count = configuration_->windowController->AddRef ();
        videoDisplayControl_ = configuration_->windowController;
        //video_display_control_p = videoDisplayControl_;
      } // end IF
      //else
      //  video_display_control_p = videoDisplayControl_;
      //ACE_ASSERT (video_display_control_p);

      goto continue_;

error:
      if (media_sink_p)
        media_sink_p->Release ();
      if (videoDisplayControl_)
      {
        videoDisplayControl_->Release ();
        videoDisplayControl_ = NULL;
      } // end IF
      //if (videoSampleAllocator_)
      //{
      //  videoSampleAllocator_->Release ();
      //  videoSampleAllocator_ = NULL;
      //} // end IF
      if (streamSink_)
      {
        streamSink_->Release ();
        streamSink_ = NULL;
      } // end IF
      if (mediaSession_)
      {
        // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
        //result_2 = mediaSession_->Shutdown ();
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF
      if (device_)
      {
        device_->Release ();
        device_ = NULL;
      } // end IF

      session_data_r.aborted = true;

continue_:
      if (COM_initialized)
        CoUninitialize ();

      break;
    }
    case STREAM_SESSION_END:
    {
      result_2 = CoInitializeEx (NULL,
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

      //if (videoSampleAllocator_)
      //{
      //  result_2 = videoSampleAllocator_->UninitializeSampleAllocator ();
      //  if (FAILED (result_2))
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("failed to IMFVideoSampleAllocator::UninitializeSampleAllocator(): \"%s\", continuing\n"),
      //                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      //  videoSampleAllocator_->Release ();
      //  videoSampleAllocator_ = NULL;
      //} // end IF
      if (videoDisplayControl_)
      {
        videoDisplayControl_->Release ();
        videoDisplayControl_ = NULL;
      } // end IF
      if (streamSink_)
      {
        streamSink_->Release ();
        streamSink_ = NULL;
      } // end IF
      if (mediaSession_)
      {
        // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
        //result_2 = mediaSession_->Shutdown ();
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        mediaSession_->Release ();
        mediaSession_ = NULL;
      } // end IF
      if (device_)
      {
        device_->Release ();
        device_ = NULL;
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
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::initialize"));

  if (inherited::isInitialized_)
  {
    //if (videoSampleAllocator_)
    //{
    //  videoSampleAllocator_->Release ();
    //  videoSampleAllocator_ = NULL;
    //} // end IF
    if (videoDisplayControl_)
    {
      videoDisplayControl_->Release ();
      videoDisplayControl_ = NULL;
    } // end IF
    if (streamSink_)
    {
      streamSink_->Release ();
      streamSink_ = NULL;
    } // end IF
    if (mediaSession_)
    {
      HRESULT result = mediaSession_->Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      mediaSession_->Release ();
      mediaSession_ = NULL;
    } // end IF
    if (device_)
    {
      device_->Release ();
      device_ = NULL;
    } // end IF

    inherited::isInitialized_ = false;
  } // end IF

  return inherited::initialize (configuration_in);
}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType>
//const ConfigurationType&
//Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
//                                    MessageType,
//                                    ConfigurationType,
//                                    SessionDataType,
//                                    SessionDataContainerType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::get"));
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
bool
Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize_MediaFoundation (const HWND windowHandle_in,
                                                                                           const struct tagRECT& windowArea_in,
                                                                                           TOPOID rendererNodeId_in,
                                                                                           //const IMFMediaType* mediaType_in,
                                                                                           IMFMediaSink*& IMFMediaSink_out,
                                                                                           IMFVideoDisplayControl*& IMFVideoDisplayControl_out,
                                                                                           //IMFVideoSampleAllocator*& IMFVideoSampleAllocator_out,
                                                                                           IMFMediaSession* IMFMediaSession_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::initialize_MediaFoundation"));

  HRESULT result = E_FAIL;
  //IMFVideoRenderer* video_renderer_p = NULL;

  // initialize return value(s)
  ACE_ASSERT (!IMFMediaSink_out);
  ACE_ASSERT (!IMFVideoDisplayControl_out);
  //ACE_ASSERT (!IMFVideoSampleAllocator_out);
  ACE_ASSERT (IMFMediaSession_in);

  bool set_topology = false;
  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  IMFTopology* topology_p = NULL;
  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
  //         --> (try to) wait for the next MESessionTopologySet event
  // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
  //         still fails with MF_E_INVALIDREQUEST)
  do
  {
    result =
      const_cast<IMFMediaSession*> (IMFMediaSession_in)->GetFullTopology (flags,
                                                                          0,
                                                                          &topology_p);
  } while (result == MF_E_INVALIDREQUEST);
  if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_p);

  IMFTopologyNode* topology_node_p = NULL;
  TOPOID node_id = 0;
  IMFStreamSink* stream_sink_p = NULL;
  IMFVideoPresenter* video_presenter_p = NULL;
  HWND window_handle = NULL;
  if (topology_p)
  {
    IUnknown* unknown_p = NULL;
    if (rendererNodeId_in)
    {
      result = topology_p->GetNodeByID (rendererNodeId_in,
                                        &topology_node_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                    rendererNodeId_in,
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        goto error;
      } // end IF
    } // end IF
    else
    {
      window_handle = CreateWindow (ACE_TEXT ("EDIT"),
                                    0,
                                    WS_OVERLAPPEDWINDOW,
                                    //WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    640,
                                    480,
                                    0,
                                    0,
                                    GetModuleHandle (NULL),
                                    0);
      if (!window_handle)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CreateWindow(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
        goto error;
      } // end IF
      ShowWindow (window_handle, TRUE);
      //if (!Stream_Module_Device_Tools::addRenderer (windowHandle_in,
      if (!Stream_Module_Device_Tools::addRenderer (window_handle,
                                                    topology_p,
                                                    node_id))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::addRenderer(), aborting\n")));
        goto error;
      } // end IF
      set_topology = true;
      result = topology_p->GetNodeByID (node_id,
                                        &topology_node_p);
      ACE_ASSERT (SUCCEEDED (result));
    } // end ELSE
    ACE_ASSERT (topology_node_p);
    unknown_p = NULL;
    result = topology_node_p->GetObject (&unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    topology_node_p->Release ();
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFStreamSink): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      unknown_p->Release ();

      goto error;
    } // end IF
    unknown_p->Release ();
    result = stream_sink_p->GetMediaSink (&IMFMediaSink_out);
    ACE_ASSERT (SUCCEEDED (result));
    stream_sink_p->Release ();
  
    goto continue_;
  } // end IF

  result = MFCreateTopology (&topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  TOPOID topology_id = 0;
  result = topology_p->GetTopologyID (&topology_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("created topology (id was: %q)...\n"),
              topology_id));

  IMFActivate* activate_p = NULL;
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  result = activate_p->ActivateObject (IID_PPV_ARGS (&IMFMediaSink_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    activate_p->Release ();

    goto error;
  } // end IF
  activate_p->Release ();

  //result = MFCreateVideoRenderer (IID_PPV_ARGS (&IMFMediaSink_out));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoRenderer(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  IMFPresentationClock* presentation_clock_p = NULL;
  result = MFCreatePresentationClock (&presentation_clock_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreatePresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFPresentationTimeSource* presentation_time_source_p = NULL;
  result = MFCreateSystemTimeSource (&presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSystemTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  result = presentation_clock_p->SetTimeSource (presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::SetTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_time_source_p->Release ();
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  presentation_time_source_p->Release ();
  result = IMFMediaSink_out->SetPresentationClock (presentation_clock_p);
  if (FAILED (result)) // MF_E_NOT_INITIALIZED: 0xC00D36B6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::SetPresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  result = presentation_clock_p->Start (0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::Start(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_clock_p->Release ();

    goto error;
  } // end IF
  presentation_clock_p->Release ();

  //IMFTransform* transform_p = NULL;
  //result =
  //  MFCreateVideoMixer (NULL,                         // owner
  //                      IID_IDirect3DDevice9,         // device
  //                      IID_PPV_ARGS (&transform_p)); // return value: interface handle
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                  &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  stream_sink_p = NULL;
  result = IMFMediaSink_out->GetStreamSinkByIndex (0,
                                                    &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release ();
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_p->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    topology_node_p->Release ();

    goto error;
  } // end IF
  node_id = 0;
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              node_id));
  topology_node_p->Release ();
  // *TODO*: connect renderer node to a stream source
  set_topology = true;

continue_:
  ACE_ASSERT (IMFMediaSink_out);
  ACE_ASSERT (topology_p);

  if (set_topology)
  {
    DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE    |
                            MFSESSION_SETTOPOLOGY_NORESOLUTION);// |
                            //MFSESSION_SETTOPOLOGY_CLEAR_CURRENT);
    result = IMFMediaSession_in->SetTopology (topology_flags,
                                              topology_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    // debug info
    Stream_Module_Device_Tools::dump (topology_p);
  } // end IF

  result =
    MFCreateVideoPresenter (NULL,                               // owner
                            IID_IDirect3DDevice9,               // device
                            IID_PPV_ARGS (&video_presenter_p)); // return value: interface handle
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  IMFGetService* get_service_p = NULL;
  result = video_presenter_p->QueryInterface (IID_PPV_ARGS (&get_service_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoPresenter::QueryInterface(IID_IMFGetService): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    video_presenter_p->Release ();

    goto error;
  } // end IF
  video_presenter_p->Release ();
  result =
    get_service_p->GetService (MR_VIDEO_RENDER_SERVICE,
                               IID_PPV_ARGS (&IMFVideoDisplayControl_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFGetService::GetService(MR_VIDEO_RENDER_SERVICE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    get_service_p->Release ();

    goto error;
  } // end IF
  get_service_p->Release ();
  ACE_ASSERT (IMFVideoDisplayControl_out);

  //result = IMFVideoDisplayControl_out->SetVideoWindow (windowHandle_in);
  result = IMFVideoDisplayControl_out->SetVideoWindow (window_handle);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoWindow(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //struct MFVideoNormalizedRect source_rectangle = {0, 0, 1, 1};
  struct tagRECT destination_rectangle;
  ACE_OS::memset (&destination_rectangle, 0, sizeof (struct tagRECT));
  destination_rectangle.right = windowArea_in.right - windowArea_in.left;
  destination_rectangle.bottom = windowArea_in.bottom - windowArea_in.top;
  result =
    IMFVideoDisplayControl_out->SetVideoPosition (NULL,//&source_rectangle,
                                                  &destination_rectangle);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoPosition(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  DWORD rendering_preferences =
    (MFVideoRenderPrefs_DoNotRenderBorder     |
     MFVideoRenderPrefs_DoNotClipToDevice     |
     MFVideoRenderPrefs_AllowOutputThrottling |
     MFVideoRenderPrefs_AllowBatching         |
     MFVideoRenderPrefs_AllowScaling          |
     MFVideoRenderPrefs_DoNotRepaintOnStop);
  result =
    IMFVideoDisplayControl_out->SetRenderingPrefs (rendering_preferences);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoDisplayControl::SetRenderingPrefs(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  //result = IMFMediaSink_out->QueryInterface (IID_PPV_ARGS (&video_renderer_p));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFMediaSink::QueryInterface(IID_IMFVideoRenderer): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //result = video_renderer_p->InitializeRenderer (NULL,//transform_p,
  //                                               video_presenter_p);
  //if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFVideoRenderer::InitializeRenderer(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //video_renderer_p->Release ();

  //result =
  //  IMFVideoSampleAllocator_out->InitializeSampleAllocator (MODULE_VIS_DEFAULT_VIDEO_SAMPLES,
  //                                                          const_cast<IMFMediaType*> (mediaType_in));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFVideoSampleAllocator::InitializeSampleAllocator(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  topology_p->Release ();
  topology_p = NULL;

  goto continue_2;

error:
  if (topology_p)
    topology_p->Release ();
  //if (video_renderer_p)
  //  video_renderer_p->Release ();
  //if (IMFVideoSampleAllocator_out)
  //{
  //  IMFVideoSampleAllocator_out->Release ();
  //  IMFVideoSampleAllocator_out = NULL;
  //} // end IF
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release ();
    IMFVideoDisplayControl_out = NULL;
  } // end IF
  if (IMFMediaSink_out)
  {
    IMFMediaSink_out->Release ();
    IMFMediaSink_out = NULL;
  } // end IF

  return false;

continue_2:

  goto continue_3;

//error_2:
  return false;

continue_3:
  return true;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_MediaFoundation_2<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::Stream_Vis_Target_MediaFoundation_2 ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::Stream_Vis_Target_MediaFoundation_2"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_MediaFoundation_2<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::~Stream_Vis_Target_MediaFoundation_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::~Stream_Vis_Target_MediaFoundation_2"));

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
Stream_Vis_Target_MediaFoundation_2<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);

  //HRESULT result = E_FAIL;
  //typename const MessageType::DATA_T& message_data_r = message_inout->get ();

  //// sanity check(s)
  //ACE_ASSERT (streamSink_);
  ////ACE_ASSERT (videoSampleAllocator_);
  //ACE_ASSERT (message_data_r.sample);

  //// *NOTE*: EVR does not accept plain IMFSamples; only 'video samples'
  //IMFSample* sample_p = NULL;
  //result = videoSampleAllocator_->AllocateSample (&sample_p);
  //if (FAILED (result)) // MF_E_SAMPLEALLOCATOR_EMPTY: 0xC00D4A3EL
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFVideoSampleAllocator::AllocateSample(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return;
  //} // end IF
  //ACE_ASSERT (sample_p);

  //result = sample_p->SetSampleTime (message_data_r.sampleTime);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::SetSampleTime(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  sample_p->Release ();

  //  return;
  //} // end IF

  //DWORD count = 0;
  //result = message_data_r.sample->GetBufferCount (&count);
  //ACE_ASSERT (SUCCEEDED (result));
  //ACE_ASSERT (count == 1);
  //IMFMediaBuffer* media_buffer_p = NULL;
  //result = message_data_r.sample->GetBufferByIndex (0, &media_buffer_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::GetBufferByIndex(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  sample_p->Release ();

  //  return;
  //} // end IF
  //ACE_ASSERT (media_buffer_p);
  //result = sample_p->AddBuffer (media_buffer_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFSample::AddBuffer(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  media_buffer_p->Release ();
  //  sample_p->Release ();

  //  return;
  //} // end IF
  //media_buffer_p->Release ();

  //result = streamSink_->ProcessSample (sample_p);
  //result = streamSink_->ProcessSample (message_data_r.sample);
  //if (FAILED (result)) // E_NOINTERFACE
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFStreamSink::ProcessSample(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  // clean up
  //  //sample_p->Release ();

  //  return;
  //} // end IF
  //sample_p->Release ();
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
Stream_Vis_Target_MediaFoundation_2<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                     bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::handleSessionMessage"));

  //int result = -1;
  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  switch (message_inout->type ())
  {
  case STREAM_SESSION_MESSAGE_BEGIN:
  {
    const SessionDataContainerType& session_data_container_r =
      message_inout->get ();
    SessionDataType& session_data_r =
      const_cast<SessionDataType&> (session_data_container_r.get ());

    result_2 = CoInitializeEx (NULL,
                               (COINIT_MULTITHREADED     |
                                COINIT_DISABLE_OLE1DDE   |
                                COINIT_SPEED_OVER_MEMORY));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    COM_initialized = true;

    goto continue_;

  error:
    session_data_r.aborted = true;

  continue_:
    if (COM_initialized)
      CoUninitialize ();

    break;
  }
  case STREAM_SESSION_MESSAGE_END:
  {
    result_2 = CoInitializeEx (NULL,
                               (COINIT_MULTITHREADED     |
                                COINIT_DISABLE_OLE1DDE   |
                                COINIT_SPEED_OVER_MEMORY));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      break;
    } // end IF
    COM_initialized = true;

    if (COM_initialized)
      CoUninitialize ();

    break;
  }
  default:
    break;
  } // end SWITCH
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename SessionDataContainerType>
//bool
//Stream_Vis_Target_MediaFoundation_2<SessionMessageType,
//                                    MessageType,
//                                    ConfigurationType,
//                                    SessionDataType,
//                                    SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::initialize"));
//
//  return inherited::initialize (configuration_in);
//}
//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType>
//const ConfigurationType&
//Stream_Vis_Target_MediaFoundation_2<SessionMessageType,
//                                    MessageType,
//                                    ConfigurationType,
//                                    SessionDataType,
//                                    SessionDataContainerType>::get () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::get"));
//
//  // sanity check(s)
//  ACE_ASSERT (configuration_);
//
//  return *configuration_;
//}

//template <typename SessionMessageType,
//          typename MessageType,
//          typename ConfigurationType,
//          typename SessionDataType,
//          typename SessionDataContainerType>
//HRESULT
//Stream_Vis_Target_MediaFoundation_2<SessionMessageType,
//                                    MessageType,
//                                    ConfigurationType,
//                                    SessionDataType,
//                                    SessionDataContainerType>::OnProcessSampleEx (const struct _GUID& majorMediaType_in,
//                                                                                  DWORD flags_in,
//                                                                                  LONGLONG timeStamp_in,
//                                                                                  LONGLONG duration_in,
//                                                                                  const BYTE* buffer_in,
//                                                                                  DWORD bufferSize_in,
//                                                                                  IMFAttributes* attributes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_2::OnProcessSampleEx"));
//
//  ACE_UNUSED_ARG (majorMediaType_in);
//  ACE_UNUSED_ARG (flags_in);
//  ACE_UNUSED_ARG (timeStamp_in);
//  ACE_UNUSED_ARG (duration_in);
//  ACE_UNUSED_ARG (buffer_in);
//  ACE_UNUSED_ARG (bufferSize_in);
//  ACE_UNUSED_ARG (attributes_in);
//
//  ProtocolMessageType* message_p = NULL;
//  int result = -1;
//  HRESULT result_2 = E_FAIL;
//
//  if (isFirst_)
//  {
//    isFirst_ = false;
//    baseTimeStamp_ = timeStamp_in;
//  } // end IF
//  timeStamp_in -= baseTimeStamp_;
//
//  // sanity check(s)
//  ACE_ASSERT (inherited::configuration_);
//  // *TODO*: remove type inference
//  ACE_ASSERT (inherited::configuration_->streamConfiguration);
//
//  // *TODO*: remove type inference
//  message_p =
//    inherited::allocateMessage (inherited::configuration_->streamConfiguration->bufferSize);
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("Stream_HeadModuleTaskBase_T::allocateMessage(%d) failed: \"%m\", aborting\n"),
//                inherited::configuration_->streamConfiguration->bufferSize));
//    goto error;
//  } // end IF
//  ACE_ASSERT (message_p);
//  ACE_ASSERT (message_p->capacity () >= bufferSize_in);
//
//  // *TODO*: copy this data into the message buffer ?
//  message_p->base (reinterpret_cast<char*> (const_cast<BYTE*> (buffer_in)),
//                   bufferSize_in,
//                   ACE_Message_Block::DONT_DELETE);
//  message_p->wr_ptr (bufferSize_in);
//
//  result = inherited::putq (message_p, NULL);
//  if (result == -1)
//  {
//    int error = ACE_OS::last_error ();
//    if (error != ESHUTDOWN)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", aborting\n"),
//                  inherited::name ()));
//    goto error;
//  } // end IF
//
//  return S_OK;
//
//error:
//  if (message_p)
//    message_p->release ();
//
//  return E_FAIL;
//}
