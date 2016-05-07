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

#include "d3d9.h"
#include "mfidl.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::Stream_Vis_Target_DirectShow_T ()
 : inherited ()
 , configuration_ (NULL)
 , isInitialized_ (false)
 //, IVideoWindow_ (NULL)
 , IMFVideoRenderer_ (NULL)
 , IMFVideoDisplayControl_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::Stream_Vis_Target_DirectShow_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::~Stream_Vis_Target_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::~Stream_Vis_Target_DirectShow_T"));

  //HRESULT result = E_FAIL;
  //IVideoWindow* video_window_p =
  //  (IVideoWindow_ ? IVideoWindow_
  //                 : (configuration_ ? configuration_->windowController
  //                                   : NULL));
  //if (video_window_p)
  //{
  //  //result = video_window_p->put_Owner (NULL);
  //  //if (FAILED (result_2))
  //  //  ACE_DEBUG ((LM_ERROR,
  //  //              ACE_TEXT ("failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
  //  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //  result = video_window_p->put_MessageDrain (NULL);
  //  if (FAILED (result))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //} // end IF
  //if (IVideoWindow_)
  //{
  //  IVideoWindow_->Release ();
  //  configuration_->windowController = NULL;
  //} // end IF

  if (IMFVideoDisplayControl_)
  {
    IMFVideoDisplayControl_->Release ();
    IMFVideoDisplayControl_ = NULL;
  } // end IF
  if (IMFVideoRenderer_)
  {
    IMFVideoRenderer_->Release ();
    IMFVideoRenderer_ = NULL;
  } // end IF
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::handleDataMessage (MessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  //ACE_ASSERT (!IVideoWindow_);

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

      //IVideoWindow* video_window_p = configuration_->windowController;
      IMFVideoDisplayControl* video_display_control_p =
        configuration_->windowController;
      //if (!video_window_p)
      if (!video_display_control_p)
      {
        // sanity check(s)
        //ACE_ASSERT (!IVideoWindow_);
        ACE_ASSERT (!IMFVideoDisplayControl_);

        if (!initialize_DirectShow (configuration_->window,
                                    configuration_->area,
                                    //configuration_->builder,
                                    //IVideoWindow_))
                                    IMFVideoRenderer_,
                                    IMFVideoDisplayControl_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_DirectShow(), aborting\n")));
          goto error;
        } // end IF
        //video_window_p = IVideoWindow_;
        video_display_control_p = IMFVideoDisplayControl_;
      } // end IF
      //ACE_ASSERT (video_window_p);
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

      //IVideoWindow* video_window_p =
      //  (IVideoWindow_ ? IVideoWindow_
      //                 : configuration_->windowController);
      IMFVideoDisplayControl* video_display_control_p =
        (IMFVideoDisplayControl_ ? IMFVideoDisplayControl_
                                 : configuration_->windowController);

      // *IMPORTANT NOTE*: "Reset the owner to NULL before releasing the Filter
      //                   Graph Manager. Otherwise, messages will continue to
      //                   be sent to this window and errors will likely occur
      //                   when the application is terminated. ..."
      //if (video_window_p)
      if (video_display_control_p)
      {
        //// *TODO*: this call seems to block...
        //result_2 = video_window_p->put_Visible (OAFALSE);
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IVideoWindow::put_Visible() \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        //result_2 = video_window_p->put_AutoShow (OAFALSE);
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IVideoWindow::put_AutoShow() \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        // *TODO*: this call blocks indefinetly
        //         --> needs to be called from somewhere else ?
        //result_2 = video_window_p->put_Owner (NULL);
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        //result_2 = video_window_p->put_MessageDrain (NULL);
        //if (FAILED (result_2))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      } // end IF

      //if (IVideoWindow_)
      //{
      //  IVideoWindow_->Release ();
      //  IVideoWindow_ = NULL;
      //} // end IF
      if (IMFVideoDisplayControl_)
      {
        IMFVideoDisplayControl_->Release ();
        IMFVideoDisplayControl_ = NULL;
      } // end IF
      if (IMFVideoRenderer_)
      {
        IMFVideoRenderer_->Release ();
        IMFVideoRenderer_ = NULL;
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
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize"));

  //HRESULT result = E_FAIL;

  if (isInitialized_)
  {
    isInitialized_ = false;

    //if (IVideoWindow_)
    if (IMFVideoDisplayControl_)
    {
      //result = IVideoWindow_->put_Owner (NULL);
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //result = IVideoWindow_->put_MessageDrain (NULL);
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      //IVideoWindow_->Release ();
      //IVideoWindow_ = NULL;
      IMFVideoDisplayControl_->Release ();
      IMFVideoDisplayControl_ = NULL;
    } // end IF
    if (IMFVideoRenderer_)
    {
      IMFVideoRenderer_->Release ();
      IMFVideoRenderer_ = NULL;
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
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::get"));

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
Stream_Vis_Target_DirectShow_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::initialize_DirectShow (const HWND windowHandle_in,
                                                                                 const struct tagRECT& windowArea_in,
                                                                                 //IGraphBuilder* IGraphBuilder_in,
                                                                                 //IVideoWindow*& IVideoWindow_out)
                                                                                 IMFVideoRenderer*& IMFVideoRenderer_out,
                                                                                 IMFVideoDisplayControl*& IMFVideoDisplayControl_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize_DirectShow"));

  HRESULT result = E_FAIL;

  // initialize return value(s)
  //ACE_ASSERT (!IVideoWindow_out);
  ACE_ASSERT (!IMFVideoRenderer_out);
  ACE_ASSERT (!IMFVideoDisplayControl_out);

  // sanity check(s)
  //ACE_ASSERT (IGraphBuilder_in);

  //// retrieve interfaces for media control and the video window 
  //HRESULT result = IGraphBuilder_in->QueryInterface (IID_IVideoWindow,
  //                                                   (void**)&IVideoWindow_out);
  //if (FAILED (result))
  //  goto error;

  //goto continue_;

  //IMFActivate* activate_p = NULL;
  //HRESULT result = MFCreateVideoRendererActivate (windowHandle_in,
  //                                                &activate_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

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

  //IMFMediaSink* media_sink_p = NULL;
  //result = activate_p->ActivateObject (IID_IMFMediaSink,
  //                                     (void**)&media_sink_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  result = MFCreateVideoRenderer (IID_IMFVideoRenderer,
                                  (void**)&IMFVideoRenderer_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRenderer(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  IMFTransform* transform_p = NULL;
  result = MFCreateVideoMixer (NULL,                  // owner
                               IID_IDirect3DDevice9,  // device
                               IID_IMFTransform,      // interface id
                               (void**)&transform_p); // return value: interface handle
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  IMFVideoPresenter* video_presenter_p = NULL;
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

  IMFGetService* get_service_p = NULL;
  result = video_presenter_p->QueryInterface (IID_IMFGetService,
                                              (void**)&get_service_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoPresenter::QueryInterface(IID_IMFGetService): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
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
  result =
    IMFVideoDisplayControl_out->SetVideoPosition (NULL,
                                                  &const_cast<struct tagRECT&> (windowArea_in));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoPosition(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = IMFVideoDisplayControl_out->SetVideoWindow (windowHandle_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoDisplayControl::SetVideoWindow(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  result = IMFVideoRenderer_out->InitializeRenderer (transform_p,
                                                     video_presenter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFVideoRenderer::InitializeRenderer(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  //ACE_DEBUG ((LM_ERROR,
  //            ACE_TEXT ("failed to IGraphBuilder::QueryInterface(): \"%s\", aborting\n"),
  //            ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  //if (activate_p)
  //  activate_p->Release ();
  if (transform_p)
    transform_p->Release ();
  if (get_service_p)
    get_service_p->Release ();
  if (video_presenter_p)
    video_presenter_p->Release ();
  if (IMFVideoRenderer_out)
  {
    IMFVideoRenderer_out->Release ();
    IMFVideoRenderer_out = NULL;
  } // end IF
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release ();
    IMFVideoDisplayControl_out = NULL;
  } // end IF

  return false;

continue_:
  //ACE_ASSERT (IVideoWindow_out);

  //if (!windowHandle_in)
  //  goto continue_2;

  //result = IVideoWindow_out->put_Owner ((OAHWND)windowHandle_in);
  //if (FAILED (result))
  //  goto error_2;

  // redirect mouse and keyboard events to the main gtk window
  //result = IVideoWindow_out->put_MessageDrain ((OAHWND)windowHandle_in);
  //if (FAILED (result)) // VFW_E_NOT_CONNECTED: 0x80040209
  //  goto error_2;

  //result = IVideoWindow_out->put_WindowStyle (WS_CHILD | WS_CLIPCHILDREN);
  //if (FAILED (result))
  //  goto error_2;
  //result =
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    IVideoWindow_out->SetWindowPosition (windowArea_in.left,
//                                         windowArea_in.top,
//                                         windowArea_in.right,
//                                         windowArea_in.bottom);
//#else
//    IVideoWindow_out->SetWindowPosition (windowArea_in.x,
//                                         windowArea_in.y,
//                                         windowArea_in.width,
//                                         windowArea_in.height);
//#endif
  //if (FAILED (result))
  //  goto error_2;

  goto continue_2;

//error_2:
  //ACE_DEBUG ((LM_ERROR,
  //            ACE_TEXT ("failed to configure IVideoWindow: \"%s\" (0x%x), aborting\n"),
  //            ACE_TEXT (Common_Tools::error2String (result).c_str ()), result));
  return false;

continue_2:
  return true;
}
