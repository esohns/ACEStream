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

#include <vfwmsgs.h>

#include <ace/Log_Msg.h>

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename FilterType>
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType,
                               FilterType>::Stream_Vis_Target_DirectShow_T ()
 : inherited ()
 , closeWindow_ (false)
 , IVideoWindow_ (NULL)
 , window_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::Stream_Vis_Target_DirectShow_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename FilterType>
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType,
                               FilterType>::~Stream_Vis_Target_DirectShow_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::~Stream_Vis_Target_DirectShow_T"));

  HRESULT result = E_FAIL;

  IVideoWindow* video_window_p =
    (IVideoWindow_ ? IVideoWindow_
                   : (inherited::configuration_ ? inherited::configuration_->windowController
                                                : NULL));
  if (video_window_p)
  {
    result = video_window_p->put_Owner (NULL);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IVideoWindow::put_Owner(NULL) \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    result = video_window_p->put_MessageDrain (NULL);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IVideoWindow::put_MessageDrain(NULL) \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF

  if (IVideoWindow_)
    IVideoWindow_->Release ();

  if (window_ && closeWindow_)
    ShowWindow (window_, FALSE);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename FilterType>
void
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType,
                               FilterType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::toggle"));

  // sanity check(s)
  ACE_ASSERT (IVideoWindow_);

  LONG fullscreen_mode = 0;
  HRESULT result = IVideoWindow_->get_FullScreenMode (&fullscreen_mode);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::get_FullScreenMode(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF

  fullscreen_mode = (fullscreen_mode == OATRUE ? OAFALSE : OATRUE);
  if (fullscreen_mode)
  { // --> switch to fullscreen
    result =
      IVideoWindow_->put_MessageDrain (GetAncestor (window_, GA_ROOTOWNER));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF

    result = IVideoWindow_->put_FullScreenMode (OATRUE);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(%d): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  OATRUE,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF
  } // end IF
  else
  { // --> switch to window
    result = IVideoWindow_->put_FullScreenMode (OAFALSE);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(%d): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  OAFALSE,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF

    result = IVideoWindow_->put_MessageDrain (window_);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF

    result = IVideoWindow_->SetWindowForeground (OATRUE);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::SetWindowForeground(OATRUE): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF
  } // end ELSE
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename SessionDataType,
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename FilterType>
void
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType,
                               FilterType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      // flush all data
      unsigned int result_3 =
        inherited::queue_.flush (false); // flush session data ?

      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (session_data_container_r.get ());

      bool is_running = false;
      bool remove_from_ROT = false;
#if defined (_DEBUG)
      std::string log_file_name;
#endif
      //struct _AllocatorProperties allocator_properties;
      IAMBufferNegotiation* buffer_negotiation_p = NULL;
      IVideoWindow* video_window_p = NULL;
      ULONG reference_count = 0;

      //ACE_OS::memset (&allocator_properties, 0, sizeof (allocator_properties));
      //// *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
      ////         if this is -1/0 (why ?)
      ////allocator_properties.cbAlign = -1;  // <-- use default
      //allocator_properties.cbAlign = 1;
      //allocator_properties.cbBuffer = inherited::configuration_->bufferSize;
      ////allocator_properties.cbPrefix = -1; // <-- use default
      //allocator_properties.cbPrefix = 0;
      //allocator_properties.cBuffers =
      //  MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;

      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      if (!window_)
      {
        //ACE_ASSERT (inherited::configuration_->format);
        //ACE_ASSERT (inherited::configuration_->format->formattype == FORMAT_VideoInfo);
        //ACE_ASSERT (inherited::configuration_->format->cbFormat == sizeof (struct tagVIDEOINFOHEADER));
        //struct tagVIDEOINFOHEADER* video_info_header_p =
        //  reinterpret_cast<struct tagVIDEOINFOHEADER*> (inherited::configuration_->format->pbFormat);

        DWORD window_style = (WS_OVERLAPPED     |
                              WS_CAPTION        |
                              (WS_CLIPSIBLINGS  |
                                WS_CLIPCHILDREN) |
                              WS_SYSMENU        |
                              //WS_THICKFRAME     |
                              WS_MINIMIZEBOX    |
                              WS_VISIBLE/*
                              WS_MAXIMIZEBOX*/);
        DWORD window_style_ex = (WS_EX_APPWINDOW |
                                  WS_EX_WINDOWEDGE);
        unsigned int height, width;
        height =
          (inherited::configuration_->area.bottom -
           inherited::configuration_->area.top);
        width =
          (inherited::configuration_->area.right -
           inherited::configuration_->area.left);
        window_ =
          CreateWindowEx (window_style_ex,                                 // dwExStyle
                          ACE_TEXT_ALWAYS_CHAR ("EDIT"),                   // lpClassName
                          ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()), // lpWindowName
                          window_style,                                    // dwStyle
                          CW_USEDEFAULT,                                   // x
                          CW_USEDEFAULT,                                   // y
                          width,                                           // nWidth
                          height,                                          // nHeight
                          //parent_window_handle,                          // hWndParent
                          NULL,
                          NULL,                                            // hMenu
                          GetModuleHandle (NULL),                          // hInstance
                          NULL);                                           // lpParam
        if (!window_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CreateWindow(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: opened window (size: %ux%u, handle: 0x%@)...\n"),
                    inherited::mod_->name (),
                    width, height,
                    window_));
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_);

      if (!inherited::IGraphBuilder_)
      {
        // sanity check(s)
        // *TODO*: remove type inferences
        ACE_ASSERT (inherited::configuration_->filterConfiguration);

        if (!inherited::loadGraph (GUID_NULL,
                                   *inherited::configuration_->filterConfiguration,
                                   *inherited::configuration_->format,
                                   window_,
                                   inherited::IGraphBuilder_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Misc_DirectShow_Target_T::loadGraph(), aborting\n")));
          goto error;
        } // end IF
        ACE_ASSERT (inherited::IGraphBuilder_);
      } // end IF
      ACE_ASSERT (inherited::IGraphBuilder_);
#if defined (_DEBUG)
      log_file_name =
        Common_File_Tools::getLogDirectory (std::string (),
                                            0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += MODULE_DEV_DIRECTSHOW_LOGFILE_NAME;
      Stream_Module_Device_DirectShow_Tools::debug (inherited::IGraphBuilder_,
                                                    log_file_name);
#endif

      if (IVideoWindow_)
      {
        reference_count = IVideoWindow_->AddRef ();
        video_window_p = IVideoWindow_;
        goto continue_;
      } // end IF

      if (session_data_r.windowController)
      {
        reference_count = session_data_r.windowController->AddRef ();
        IVideoWindow_ = session_data_r.windowController;
        reference_count = session_data_r.windowController->AddRef ();
        video_window_p = session_data_r.windowController;
        goto continue_;
      } // end IF

      if (!initialize_DirectShow (inherited::IGraphBuilder_,
                                  *inherited::configuration_->format,
                                  window_,
                                  inherited::configuration_->fullScreen,
                                  IVideoWindow_,
                                  inherited::configuration_->area))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_DirectShow(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (IVideoWindow_);
      reference_count = IVideoWindow_->AddRef ();
      video_window_p = IVideoWindow_;
      ACE_ASSERT (window_);
continue_:
      ACE_ASSERT (video_window_p);

      // sanity check(s)

      // retrieve interfaces for media control and the event sink
      if (!inherited::IMediaControl_)
      {
        result_2 =
          inherited::IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&(inherited::IMediaControl_)));
        if (FAILED (result_2))
          goto error;
      } // end IF
      if (!inherited::IMediaEventEx_)
      {
        result_2 =
          inherited::IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&(inherited::IMediaEventEx_)));
        if (FAILED (result_2))
          goto error;
      } // end IF
      ACE_ASSERT (inherited::IMediaControl_);
      ACE_ASSERT (inherited::IMediaEventEx_);

      // (re-)start forwarding data
      enum _FilterState graph_state;
      result_2 =
        inherited::IMediaControl_->GetState (INFINITE,
                                             (OAFilterState*)&graph_state);
      if (FAILED (result_2)) // VFW_S_STATE_INTERMEDIATE: 0x00040237
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMediaControl::GetState(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2, true).c_str ())));
        goto error;
      } // end IF
      switch (graph_state)
      {
        case State_Paused:
        case State_Stopped:
        {
          result_2 = inherited::IMediaControl_->Run ();
          if (FAILED (result_2)) // VFW_E_SIZENOTSET: 0x80040212
                                 // E_OUTOFMEMORY   : 0x8007000E
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IMediaControl::Run(): \"%s\", aborting\n"),
                        ACE_TEXT (Common_Tools::error2String (result_2, true).c_str ())));
            goto error;
          } // end IF
          else if (result_2 == S_FALSE)
          {
            // *TODO*: for reaons yet unknown, this blocks...
            //enum _FilterState graph_state;
            //result_2 =
            //  inherited::IMediaControl_->GetState (INFINITE,
            //                                       (OAFilterState*)&graph_state);
            //if (FAILED (result_2)) // VFW_S_STATE_INTERMEDIATE: 0x00040237
            //{
            //  ACE_DEBUG ((LM_ERROR,
            //              ACE_TEXT ("failed to IMediaControl::GetState(): \"%s\", aborting\n"),
            //              ACE_TEXT (Common_Tools::error2String (result_2, true).c_str ())));
            //  goto error;
            //} // end IF
            //ACE_ASSERT (graph_state == State_Running);
          } // end ELSE IF
          is_running = true;

          break;
        }
        case State_Running:
        {
          is_running = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown state (was: %d), aborting\n"),
                      graph_state));
          goto error;
        }
      } // end SWITCH

      // register graph in the ROT (so graphedt.exe can see it)
      if (!ROTID_)
        if (!Stream_Module_Device_DirectShow_Tools::addToROT (IGraphBuilder_,
                                                              ROTID_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::addToROT(), aborting\n")));
          goto error;
        } // end IF
      ACE_ASSERT (ROTID_);
      remove_from_ROT = true;

      break;

error:
      // deregister graph from the ROT ?
      if (remove_from_ROT)
      { ACE_ASSERT (ROTID_);
        if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      ROTID_));
        ROTID_ = 0;
      } // end IF
      if (is_running)
      { ACE_ASSERT (inherited::IMediaControl_);
        result_2 = inherited::IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2, true).c_str ())));
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (window_ && closeWindow_)
      {
        ShowWindow (window_, FALSE);

        closeWindow_ = false;
      } // end IF
      window_ = NULL;

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", returning\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      // step1: dispatch all data to DirectShow
      inherited::queue_.waitForIdleState ();
      // step2: *TODO*: wait for DirectShow

      // *IMPORTANT NOTE*: "Reset the owner to NULL before releasing the Filter
      //                   Graph Manager. Otherwise, messages will continue to
      //                   be sent to this window and errors will likely occur
      //                   when the application is terminated. ..."
      if (IVideoWindow_)
      {
        result_2 =
          IVideoWindow_->put_FullScreenMode (OAFALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IVideoWindow::put_FullScreenMode(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        result_2 = IVideoWindow_->put_Visible (OAFALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_Visible(OAFALSE): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        result_2 = IVideoWindow_->put_MessageDrain (NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(NULL): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        result_2 = IVideoWindow_->put_Owner (NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(NULL): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        IVideoWindow_->Release ();
        IVideoWindow_ = NULL;
      } // end IF

      // deregister graph from the ROT ?
      if (ROTID_)
      {
        if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      ROTID_));
        ROTID_ = 0;
      } // end IF

//continue_2:
      if (inherited::IMediaEventEx_)
      {
        result_2 = inherited::IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        inherited::IMediaEventEx_->Release ();
        inherited::IMediaEventEx_ = NULL;
      } // end IF

      // stop DirectShow streaming thread ?
      if (inherited::push_)
        inherited::stop (false,  // wait for completion ?
                         false); // N/A

      if (inherited::IMediaControl_)
      {
        // stop previewing video data (blocks)
        result_2 = inherited::IMediaControl_->Stop ();
        if (FAILED (result_2)) // VFW_E_NO_ALLOCATOR: 0x8004020A
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2, true).c_str ())));
        inherited::IMediaControl_->Release ();
        inherited::IMediaControl_ = NULL;
      } // end IF

      if (inherited::IGraphBuilder_)
      {
        inherited::IGraphBuilder_->Release ();
        inherited::IGraphBuilder_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (window_ && closeWindow_)
      {
        ShowWindow (window_, FALSE);

        closeWindow_ = false;
      } // end IF
      window_ = NULL;

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
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename FilterType>
bool
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType,
                               FilterType>::initialize (const ConfigurationType& configuration_in,
                                                        Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize"));

  HRESULT result = E_FAIL;

  if (inherited::isInitialized_)
  {
    if (IVideoWindow_)
    {
      result = IVideoWindow_->put_MessageDrain (NULL);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      result = IVideoWindow_->put_Owner (NULL);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      IVideoWindow_->Release ();
      IVideoWindow_ = NULL;
    } // end IF

    if (window_ && closeWindow_)
    {
      ShowWindow (window_, FALSE);

      closeWindow_ = false;
    } // end IF
    window_ = NULL;
  } // end IF

  // *TODO*: remove type inferences
  if (configuration_in.windowController)
  {
    ULONG reference_count = configuration_in.windowController->AddRef ();
    IVideoWindow_ = configuration_in.windowController;
  } // end IF
  window_ = configuration_in.window;

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
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename FilterType>
bool
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType,
                               FilterType>::initialize_DirectShow (IGraphBuilder* IGraphBuilder_in,
                                                                   const struct _AMMediaType& mediaType_in,
                                                                   HWND& windowHandle_inout,
                                                                   bool fullScreen_in,
                                                                   IVideoWindow*& IVideoWindow_out,
                                                                   struct tagRECT& windowArea_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize_DirectShow"));

  HRESULT result = E_FAIL;
  //IBaseFilter* ibase_filter_p = NULL;
  IMediaEventEx* imedia_event_ex_p = NULL;
  long window_message = WM_USER;
  LONG_PTR instance_data_p = NULL;
  BOOL result_2 = FALSE;

  // initialize return value(s)
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release ();
    IVideoWindow_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  // retrieve interfaces for media control and the video window 
  result =
    IGraphBuilder_in->QueryInterface (IID_PPV_ARGS (&IVideoWindow_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IVideoWindow): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (IVideoWindow_out);
  //result =
  //  IGraphBuilder_in->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO,
  //                                      &ibase_filter_p);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(%s): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //ACE_ASSERT (ibase_filter_p);
  //result =
  //  ibase_filter_p->QueryInterface (IID_PPV_ARGS (&IVideoWindow_out));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVideoWindow): \"%s\", aborting\n"),
  //              ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF
  //ibase_filter_p->Release ();
  //ibase_filter_p = NULL;
  ACE_ASSERT (IVideoWindow_out);

  if (!windowHandle_inout)
  {
    ACE_ASSERT (mediaType_in.formattype == FORMAT_VideoInfo);
    ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    struct tagVIDEOINFOHEADER* video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);

    DWORD window_style = (WS_OVERLAPPED     |
                          WS_CAPTION        |
                          (WS_CLIPSIBLINGS  |
                            WS_CLIPCHILDREN) |
                          WS_SYSMENU        |
                          //WS_THICKFRAME     |
                          WS_MINIMIZEBOX    |
                          WS_VISIBLE/*
                          WS_MAXIMIZEBOX*/);
    DWORD window_style_ex = (WS_EX_APPWINDOW |
                              WS_EX_WINDOWEDGE);
    windowHandle_inout =
      CreateWindowEx (window_style_ex,                         // dwExStyle
                      ACE_TEXT_ALWAYS_CHAR ("EDIT"),           // lpClassName
                      ACE_TEXT_ALWAYS_CHAR ("EDIT"),           // lpWindowName
                      window_style,                            // dwStyle
                      CW_USEDEFAULT,                           // x
                      CW_USEDEFAULT,                           // y
                      video_info_header_p->bmiHeader.biWidth,  // nWidth
                      video_info_header_p->bmiHeader.biHeight, // nHeight
                      //parent_window_handle,          // hWndParent
                      NULL,
                      NULL,                                    // hMenu
                      GetModuleHandle (NULL),                  // hInstance
                      NULL);                                   // lpParam
    if (!windowHandle_inout)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CreateWindow(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
      goto error;
    } // end IF

    result_2 = GetClientRect (windowHandle_inout, &windowArea_inout);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to GetClientRect(0x%@): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Tools::error2String (GetLastError ()).c_str ())));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (windowHandle_inout);

  result =
    IGraphBuilder_in->QueryInterface (IID_PPV_ARGS (&imedia_event_ex_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::QueryInterface(IID_IMediaEventEx): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (imedia_event_ex_p);
  result =
    imedia_event_ex_p->SetNotifyWindow ((OAHWND)windowHandle_inout,
                                        window_message,
                                        instance_data_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaEventEx::SetNotifyWindow(0x%@): \"%s\", aborting\n"),
                windowHandle_inout,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("window (handle: 0x%@) will receive DirectShow messages (id: 0x%x)...\n"),
              windowHandle_inout,
              window_message));
  imedia_event_ex_p->Release ();
  imedia_event_ex_p = NULL;

  result = IVideoWindow_out->put_Owner ((OAHWND)windowHandle_inout);
  // *NOTE*: "...For the Filter Graph Manager's implementation, if the graph
  //         does not contain a video renderer filter, all methods return
  //         E_NOINTERFACE..."
  if (FAILED (result)) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_Owner(0x%@): \"%s\", continuing\n"),
                windowHandle_inout,
                ACE_TEXT (Common_Tools::error2String (result, true).c_str ())));

  result = IVideoWindow_out->put_WindowStyle (WS_CHILD | WS_CLIPSIBLINGS);
  if (FAILED (result)) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_WindowStyle(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  result =
    IVideoWindow_out->SetWindowPosition (0,
                                         0,
                                         (windowArea_inout.right -
                                          windowArea_inout.left),
                                         (windowArea_inout.bottom -
                                          windowArea_inout.top));
  if (FAILED (result)) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(0,0,%d,%d): \"%s\", continuing\n"),
                (windowArea_inout.right - windowArea_inout.left),
                (windowArea_inout.bottom - windowArea_inout.top),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  // redirect mouse and keyboard events to the main gtk window
  result = IVideoWindow_out->put_MessageDrain ((OAHWND)windowHandle_inout);
  if (FAILED (result)) // E_NOINTERFACE      : 0x80004002
                       // VFW_E_NOT_CONNECTED: 0x80040209
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_MessageDrain(0x%@): \"%s\", continuing\n"),
                windowHandle_inout,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  result = IVideoWindow_out->put_Visible (OATRUE);
  if (FAILED (result)) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_Visible(OATRUE): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  result =
    IVideoWindow_out->put_FullScreenMode (fullScreen_in ? OATRUE : OAFALSE);
  if (FAILED (result)) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_FullScreenMode(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  // *TODO*: forward WM_MOVE messages to the video window via NotifyOwnerMessage
  //         (see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd407298(v=vs.85).aspx)

  return true;

error:
  //if (ibase_filter_p)
  //  ibase_filter_p->Release ();
  if (imedia_event_ex_p)
    imedia_event_ex_p->Release ();
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release ();
    IVideoWindow_out = NULL;
  } // end IF

  return false;
}
