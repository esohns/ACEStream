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

#include "d3d9types.h"
#include "mferror.h"
#include "mfidl.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_vis_defines.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::Stream_Vis_Target_MediaFoundation_T ()
 : inherited ()
 , configuration_ (NULL)
 , isInitialized_ (false)
 , IDirect3DDevice9Ex_ (NULL)
 , IMFMediaSink_ (NULL)
 , IMFStreamSink_ (NULL)
 //, IMFVideoSampleAllocator_ (NULL)
 , IMFVideoDisplayControl_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::Stream_Vis_Target_MediaFoundation_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::~Stream_Vis_Target_MediaFoundation_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::~Stream_Vis_Target_MediaFoundation_T"));

  if (IDirect3DDevice9Ex_)
    IDirect3DDevice9Ex_->Release ();
  //if (IMFVideoSampleAllocator_)
  //  IMFVideoSampleAllocator_->Release ();
  if (IMFVideoDisplayControl_)
    IMFVideoDisplayControl_->Release ();
  if (IMFStreamSink_)
    IMFStreamSink_->Release ();
  if (IMFMediaSink_)
    IMFMediaSink_->Release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleDataMessage (MessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  typename const MessageType::DATA_T& message_data_r = message_inout->get ();

  // sanity check(s)
  ACE_ASSERT (IMFMediaSink_);
  //ACE_ASSERT (IMFVideoSampleAllocator_);
  ACE_ASSERT (message_data_r.sample);

  //// *NOTE*: EVR does not accept plain IMFSamples; only 'video samples'
  //IMFSample* sample_p = NULL;
  //result = IMFVideoSampleAllocator_->AllocateSample (&sample_p);
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

  //result = IMFStreamSink_->ProcessSample (sample_p);
  result = IMFStreamSink_->ProcessSample (message_data_r.sample);
  if (FAILED (result)) // E_NOINTERFACE
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamSink::ProcessSample(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    //sample_p->Release ();

    return;
  } // end IF
  //sample_p->Release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
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
      ACE_ASSERT (!IDirect3DDevice9Ex_);
      ACE_ASSERT (session_data_r.format);

      if (session_data_r.direct3DDevice)
      {
        session_data_r.direct3DDevice->AddRef ();
        IDirect3DDevice9Ex_ = session_data_r.direct3DDevice;
      } // end IF
      else
      {
        IDirect3DDeviceManager9* direct3D_manager_p = NULL;
        UINT reset_token = 0;
        if (!Stream_Module_Device_Tools::getDirect3DDevice (configuration_->window,
                                                            session_data_r.format,
                                                            IDirect3DDevice9Ex_,
                                                            direct3D_manager_p,
                                                            reset_token))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_Direct3D(), aborting\n")));
          goto error;
        } // end IF
        direct3D_manager_p->Release ();
      } // end ELSE

      IMFVideoDisplayControl* video_display_control_p =
        configuration_->windowController;
      if (!video_display_control_p)
      {
        // sanity check(s)
        ACE_ASSERT (session_data_r.format);
        ACE_ASSERT (!IMFVideoDisplayControl_);

        IMFMediaType* media_type_p = NULL;
        result = MFCreateMediaType (&media_type_p);
        ACE_ASSERT (SUCCEEDED (result));
        result = media_type_p->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
        ACE_ASSERT (SUCCEEDED (result));
        result = media_type_p->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        ACE_ASSERT (SUCCEEDED (result));
        result =
          media_type_p->SetUINT32 (MF_MT_INTERLACE_MODE,
                                   MFVideoInterlace_Progressive);
        ACE_ASSERT (SUCCEEDED (result));
        result = media_type_p->SetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT, true);
        ACE_ASSERT (SUCCEEDED (result));
        result =
          MFSetAttributeRatio (media_type_p, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
        ACE_ASSERT (SUCCEEDED (result));
        result =
          Stream_Module_Device_Tools::copyAttribute (session_data_r.format,
                                                     media_type_p,
                                                     MF_MT_FRAME_SIZE);
        ACE_ASSERT (SUCCEEDED (result));
        result =
          Stream_Module_Device_Tools::copyAttribute (session_data_r.format,
                                                     media_type_p,
                                                     MF_MT_FRAME_RATE);
        if (!initialize_MediaFoundation (configuration_->window,
                                         configuration_->area,
                                         media_type_p,
                                         IMFMediaSink_,
                                         IMFStreamSink_,
                                         IMFVideoDisplayControl_))//,
//                                         IMFVideoSampleAllocator_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_MediaFoundation(), aborting\n")));

          // clean up
          media_type_p->Release ();

          goto error;
        } // end IF
        media_type_p->Release ();
        video_display_control_p = IMFVideoDisplayControl_;
      } // end IF
      ACE_ASSERT (video_display_control_p);

      goto continue_;

error:
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

      IMFVideoDisplayControl* video_display_control_p =
        (IMFVideoDisplayControl_ ? IMFVideoDisplayControl_
                                 : configuration_->windowController);

      if (IDirect3DDevice9Ex_)
      {
        IDirect3DDevice9Ex_->Release ();
        IDirect3DDevice9Ex_ = NULL;
      } // end IF
      //if (IMFVideoSampleAllocator_)
      //{
      //  result_2 = IMFVideoSampleAllocator_->UninitializeSampleAllocator ();
      //  if (FAILED (result_2))
      //    ACE_DEBUG ((LM_ERROR,
      //                ACE_TEXT ("failed to IMFVideoSampleAllocator::UninitializeSampleAllocator(): \"%s\", continuing\n"),
      //                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      //  IMFVideoSampleAllocator_->Release ();
      //  IMFVideoSampleAllocator_ = NULL;
      //} // end IF
      if (IMFVideoDisplayControl_)
      {
        IMFVideoDisplayControl_->Release ();
        IMFVideoDisplayControl_ = NULL;
      } // end IF
      if (IMFStreamSink_)
      {
        IMFStreamSink_->Release ();
        IMFStreamSink_ = NULL;
      } // end IF
      if (IMFMediaSink_)
      {
        result_2 = IMFMediaSink_->Shutdown ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaSink::Shutdown(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        IMFMediaSink_->Release ();
        IMFMediaSink_ = NULL;
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
          typename SessionDataContainerType>
bool
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::initialize"));

  //HRESULT result = E_FAIL;

  if (isInitialized_)
  {
    isInitialized_ = false;

    if (IDirect3DDevice9Ex_)
    {
      IDirect3DDevice9Ex_->Release ();
      IDirect3DDevice9Ex_ = NULL;
    } // end IF
    //if (IMFVideoSampleAllocator_)
    //{
    //  IMFVideoSampleAllocator_->Release ();
    //  IMFVideoSampleAllocator_ = NULL;
    //} // end IF
    if (IMFVideoDisplayControl_)
    {
      IMFVideoDisplayControl_->Release ();
      IMFVideoDisplayControl_ = NULL;
    } // end IF
    if (IMFMediaSink_)
    {
      IMFMediaSink_->Release ();
      IMFMediaSink_ = NULL;
    } // end IF
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
const ConfigurationType&
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Vis_Target_MediaFoundation_T<SessionMessageType,
                                    MessageType,
                                    ConfigurationType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize_MediaFoundation (const HWND windowHandle_in,
                                                                                           const struct tagRECT& windowArea_in,
                                                                                           const IMFMediaType* mediaType_in,
                                                                                           IMFMediaSink*& IMFMediaSink_out,
                                                                                           IMFStreamSink*& IMFStreamSink_out,
                                                                                           IMFVideoDisplayControl*& IMFVideoDisplayControl_out)//,
                                                                                           //IMFVideoSampleAllocator*& IMFVideoSampleAllocator_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_MediaFoundation_T::initialize_MediaFoundation"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  ACE_ASSERT (!IMFMediaSink_out);
  ACE_ASSERT (!IMFStreamSink_out);
  ACE_ASSERT (!IMFVideoDisplayControl_out);
  //ACE_ASSERT (!IMFVideoSampleAllocator_out);

  IMFActivate* activate_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  IMFPresentationClock* presentation_clock_p = NULL;
  IMFPresentationTimeSource* presentation_time_source_p = NULL;
  IMFTransform* transform_p = NULL;
  IMFGetService* get_service_p = NULL;
  IMFVideoPresenter* video_presenter_p = NULL;
  IMFVideoRenderer* video_renderer_p = NULL;

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

  result = activate_p->ActivateObject (IID_IMFMediaSink,
                                       (void**)&IMFMediaSink_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  activate_p->Release ();
  activate_p = NULL;

  //result = MFCreateVideoRenderer (IID_IMFMediaSink,
  //                                (void**)&IMFMediaSink_out);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoRenderer(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = MFCreatePresentationClock (&presentation_clock_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreatePresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = MFCreateSystemTimeSource (&presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSystemTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = presentation_clock_p->SetTimeSource (presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::SetTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFMediaSink_out->SetPresentationClock (presentation_clock_p);
  if (FAILED (result)) // MF_E_NOT_INITIALIZED: 0xC00D36B6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::SetPresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  //result = MFCreateVideoMixer (NULL,                  // owner
  //                             IID_IDirect3DDevice9,  // device
  //                             IID_IMFTransform,      // interface id
  //                             (void**)&transform_p); // return value: interface handle
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = MFCreateVideoPresenter (NULL,                        // owner
                                   IID_IDirect3DDevice9,        // device
                                   IID_IMFVideoPresenter,       // interface id
                                   (void**)&video_presenter_p); // return value: interface handle
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = video_presenter_p->QueryInterface (IID_IMFGetService,
                                              (void**)&get_service_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoPresenter::QueryInterface(IID_IMFGetService): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  video_presenter_p->Release ();
  result = get_service_p->GetService (MR_VIDEO_RENDER_SERVICE,
                                      IID_IMFVideoDisplayControl,
                                      (void**)&IMFVideoDisplayControl_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFGetService::GetService(MR_VIDEO_RENDER_SERVICE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  get_service_p->Release ();
  get_service_p = NULL;
  //result = IMFVideoDisplayControl_out->SetVideoWindow (windowHandle_in);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoWindow(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //struct tagRECT destination_rectangle;
  //ACE_OS::memset (&destination_rectangle, 0, sizeof (struct tagRECT));
  //destination_rectangle.right = windowArea_in.right - windowArea_in.left;
  //destination_rectangle.bottom = windowArea_in.bottom - windowArea_in.top;
  //result =
  //  IMFVideoDisplayControl_out->SetVideoPosition (NULL,
  //                                                &destination_rectangle);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoPosition(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

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

  DWORD count = 0;
  result = IMFMediaSink_out->GetStreamSinkCount (&count);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::GetStreamSinkCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (count > 0);
  result = IMFMediaSink_out->GetStreamSinkByIndex (0, &IMFStreamSink_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::GetStreamSinkByIndex(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFStreamSink_out->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamSink::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result =
    media_type_handler_p->SetCurrentMediaType (const_cast<IMFMediaType*> (mediaType_in));
  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE: 0xC00D36B4L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::SetCurrentMediaType(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release ();
  media_type_handler_p = NULL;

  //result = IMFStreamSink_out->QueryInterface (IID_PPV_ARGS (&get_service_p));
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

  result = presentation_clock_p->Start (0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::Start(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (activate_p)
    activate_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();
  if (presentation_clock_p)
    presentation_clock_p->Release ();
  if (presentation_time_source_p)
    presentation_time_source_p->Release ();
  if (transform_p)
    transform_p->Release ();
  if (get_service_p)
    get_service_p->Release ();
  if (video_presenter_p)
    video_presenter_p->Release ();
  if (video_renderer_p)
    video_renderer_p->Release ();
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
  if (IMFStreamSink_out)
  {
    IMFStreamSink_out->Release ();
    IMFStreamSink_out = NULL;
  } // end IF
  if (IMFMediaSink_out)
  {
    IMFMediaSink_out->Release ();
    IMFMediaSink_out = NULL;
  } // end IF

  return false;

continue_:

  goto continue_2;

//error_2:
  return false;

continue_2:
  return true;
}
