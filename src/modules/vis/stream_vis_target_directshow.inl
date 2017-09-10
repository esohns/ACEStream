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

#include <control.h>
#include <evr.h>
#include <Mferror.h>
#include <vfwmsgs.h>

#include "ace/Log_Msg.h"

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
                               FilterType>::Stream_Vis_Target_DirectShow_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , closeWindow_ (false)
 , IMFVideoDisplayControl_ (NULL)
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

  if (IVideoWindow_)
  {
    result = IVideoWindow_->put_Owner (NULL);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(NULL) \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    result = IVideoWindow_->put_MessageDrain (NULL);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(NULL) \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    IVideoWindow_->Release ();
  } // end IF
  if (IMFVideoDisplayControl_)
    IMFVideoDisplayControl_->Release ();

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
  ACE_ASSERT (IVideoWindow_ || IMFVideoDisplayControl_);

  bool is_fullscreen = false;
  HRESULT result = E_FAIL;
  if (IVideoWindow_)
  {
    LONG fullscreen_mode = 0;
    result = IVideoWindow_->get_FullScreenMode (&fullscreen_mode);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::get_FullScreenMode(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    is_fullscreen = (fullscreen_mode == OATRUE);
  } // end IF
  else
  {
    BOOL fullscreen_b = FALSE;
get_mode:
    result = IMFVideoDisplayControl_->GetFullscreen (&fullscreen_b);
    if (FAILED (result)) // 0xC00D36B2: MF_E_INVALIDREQUEST
    { // *TODO*: remove tight loop here
      if (result == MF_E_INVALIDREQUEST) // <-- (still) in transition
        goto get_mode;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::GetFullscreen(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    is_fullscreen = fullscreen_b;
  } // end ELSE

  if (is_fullscreen)
  { // --> switch to window
    if (IVideoWindow_)
    {
      result = IVideoWindow_->put_FullScreenMode (OAFALSE);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(OAFALSE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      result = IVideoWindow_->put_MessageDrain ((OAHWND)window_);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      result = IVideoWindow_->SetWindowForeground (OATRUE);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::SetWindowForeground(OATRUE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
    } // end IF
    else
    {
      UINT uFlags = (SWP_ASYNCWINDOWPOS |
                     SWP_NOACTIVATE     |
                     SWP_NOMOVE         |
                     SWP_NOSIZE);
      if (!SetWindowPos (window_,
                         HWND_NOTOPMOST,
                         inherited::configuration_->area.left,
                         inherited::configuration_->area.top,
                         inherited::configuration_->area.right - inherited::configuration_->area.left,
                         inherited::configuration_->area.bottom - inherited::configuration_->area.top,
                         uFlags))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SetWindowPos(%@): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    window_,
                    ACE_TEXT (Common_Tools::errorToString (GetLastError ()).c_str ())));
        return;
      } // end IF

      result = IMFVideoDisplayControl_->SetFullscreen (FALSE);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(FALSE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
    } // end ELSE
  } // end IF
  else
  { // --> switch to fullscreen
    if (IVideoWindow_)
    {
      result =
        IVideoWindow_->put_MessageDrain ((OAHWND)GetAncestor (window_,
                                                              GA_ROOTOWNER));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      result = IVideoWindow_->put_FullScreenMode (OATRUE);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(OATRUE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
    } // end IF
    else
    {
      UINT uFlags = (SWP_ASYNCWINDOWPOS |
                     SWP_NOACTIVATE     |
                     SWP_NOMOVE         |
                     SWP_NOSIZE);
      if (!SetWindowPos (window_,
                         HWND_TOPMOST,
                         inherited::configuration_->area.left,
                         inherited::configuration_->area.top,
                         inherited::configuration_->area.right - inherited::configuration_->area.left,
                         inherited::configuration_->area.bottom - inherited::configuration_->area.top,
                         uFlags))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SetWindowPos(%@): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    window_,
                    ACE_TEXT (Common_Tools::errorToString (GetLastError ()).c_str ())));
        return;
      } // end IF

      result = IMFVideoDisplayControl_->SetFullscreen (TRUE);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(TRUE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
    } // end ELSE
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
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      const SessionDataType& session_data_r = inherited::sessionData_->get ();

      unsigned int height, width;
      height =
        (inherited::configuration_->area.bottom -
         inherited::configuration_->area.top);
      width =
        (inherited::configuration_->area.right -
         inherited::configuration_->area.left);
      bool is_running = false;
      bool remove_from_ROT = false;
      struct _AMMediaType* media_type_p = NULL;
      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
#if defined (_DEBUG)
      std::string log_file_name;
#endif

      // step1: create display window, if none was specified
      if (!window_)
      {
        // retrieve display device 'geometry' data (i.e. monitor coordinates)
        // *TODO*: remove type inference
        ACE_ASSERT (!inherited::configuration_->device.empty ());
        HMONITOR monitor_h = NULL;
        if (!Stream_Module_Device_Tools::getDisplayDevice (inherited::configuration_->device,
                                                           monitor_h))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Module_Device_Tools::getDisplayDevice(\"%s\"): \"%s\", returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->device.c_str ()),
                      ACE_TEXT (Common_Tools::errorToString (GetLastError ()).c_str ())));
          goto error;
        } // end IF
        ACE_ASSERT (monitor_h);
        MONITORINFOEX monitor_info_ex_s;
        unsigned int delta_x, delta_y;
        ACE_OS::memset (&monitor_info_ex_s, 0, sizeof (MONITORINFOEX));
        monitor_info_ex_s.cbSize = sizeof (MONITORINFOEX);
        if (!GetMonitorInfo (monitor_h,
                             reinterpret_cast<struct tagMONITORINFO*> (&monitor_info_ex_s)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to GetMonitorInfo(\"%s\"): \"%s\", returning\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (inherited::configuration_->device.c_str ()),
                      ACE_TEXT (Common_Tools::errorToString (GetLastError ()).c_str ())));
          goto error;
        } // end IF
        // *NOTE*: center the new window on the display device
        ACE_ASSERT ((monitor_info_ex_s.rcWork.right -
                     monitor_info_ex_s.rcWork.left) >= static_cast<LONG> (width));
        ACE_ASSERT ((monitor_info_ex_s.rcWork.bottom -
                     0) >= static_cast<LONG> (height));
        // *NOTE*: disregard the task bar
                     //monitor_info_ex_s.rcWork.top) >= static_cast<LONG> (height));
        delta_x =
          (((static_cast<unsigned int> (monitor_info_ex_s.rcWork.right) -
             static_cast<unsigned int> (monitor_info_ex_s.rcWork.left)) - width) / 2);
        delta_y =
          (((static_cast<unsigned int> (monitor_info_ex_s.rcWork.bottom) -
             static_cast<unsigned int> (monitor_info_ex_s.rcWork.top)) - height) / 2);

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
        window_ =
          CreateWindowEx (window_style_ex,                                 // dwExStyle
                          ACE_TEXT_ALWAYS_CHAR ("EDIT"),                   // lpClassName
                          ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()), // lpWindowName
                          window_style,                                    // dwStyle
                          //CW_USEDEFAULT, CW_USEDEFAULT,                    // x,y
                          delta_x, delta_y,                                // x,y
                          width, height,                                   // width, height
                          //parent_window_handle,                          // hWndParent
                          NULL,                                            // hWndParent
                          NULL,                                            // hMenu
                          GetModuleHandle (NULL),                          // hInstance
                          NULL);                                           // lpParam
        if (!window_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindow(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: opened display window (size: %ux%u, handle: 0x%@)\n"),
                    inherited::mod_->name (),
                    width, height,
                    window_));
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_);

      // step2: assemble display format
      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      ACE_ASSERT (session_data_r.format);
      if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*session_data_r.format,
                                                                 media_type_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (media_type_p);
      ACE_ASSERT (media_type_p->pbFormat);
      if (media_type_p->formattype == FORMAT_VideoInfo)
      {
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_p->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
      } // end IF
      else if (media_type_p->formattype == FORMAT_VideoInfo2)
      {
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_p->pbFormat);
        video_info_header2_p->bmiHeader.biWidth = width;
        video_info_header2_p->bmiHeader.biHeight = height;
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::GUIDToString (media_type_p->formattype).c_str ())));
        goto error;
      } // end ELSE

      // step3: set up DirectShow display pipeline
      if (!inherited::IGraphBuilder_)
      {
        // sanity check(s)
        // *TODO*: remove type inferences
        ACE_ASSERT (inherited::configuration_->filterConfiguration);

        if (!inherited::loadGraph (GUID_NULL,
                                   *inherited::configuration_->filterConfiguration,
                                   *media_type_p,
                                   window_,
                                   inherited::IGraphBuilder_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Misc_DirectShow_Target_T::loadGraph(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_ASSERT (inherited::IGraphBuilder_);
      } // end IF
      ACE_ASSERT (inherited::IGraphBuilder_);
#if defined (_DEBUG)
      log_file_name = Common_File_Tools::getLogDirectory (std::string (), 0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += MODULE_LIB_DIRECTSHOW_LOGFILE_NAME;
      Stream_Module_Device_DirectShow_Tools::debug (inherited::IGraphBuilder_,
                                                    log_file_name);
#endif

      if (!initialize_DirectShow (inherited::IGraphBuilder_,
                                  *media_type_p,
                                  window_,
                                  inherited::configuration_->fullScreen,
                                  inherited::configuration_->area,
                                  IVideoWindow_,
                                  IMFVideoDisplayControl_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_DirectShow(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (IVideoWindow_ || IMFVideoDisplayControl_);
      ACE_ASSERT (window_);
      Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);

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
                    ACE_TEXT ("%s: failed to IMediaControl::GetState(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
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
                        ACE_TEXT ("%s: failed to IMediaControl::Run(): \"%s\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
            goto error;
          } // end IF
          else if (result_2 == S_FALSE)
          {
            // *TODO*: for reaons yet unknown, this blocks
            //enum _FilterState graph_state;
            //result_2 =
            //  inherited::IMediaControl_->GetState (INFINITE,
            //                                       (OAFilterState*)&graph_state);
            //if (FAILED (result_2)) // VFW_S_STATE_INTERMEDIATE: 0x00040237
            //{
            //  ACE_DEBUG ((LM_ERROR,
            //              ACE_TEXT ("%s: failed to IMediaControl::GetState(): \"%s\", aborting\n"),
            //              inherited::mod_->name (),
            //              ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
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
                      ACE_TEXT ("%s: invalid/unknown state (was: %d), aborting\n"),
                      inherited::mod_->name (),
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
                      ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::addToROT(), aborting\n"),
                      inherited::mod_->name ()));
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
                      ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      inherited::mod_->name (),
                      ROTID_));
        ROTID_ = 0;
      } // end IF
      if (is_running)
      { ACE_ASSERT (inherited::IMediaControl_);
        result_2 = inherited::IMediaControl_->Stop ();
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
      } // end IF

      if (media_type_p)
        Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);

      if (COM_initialized)
        CoUninitialize ();

      if (window_ && closeWindow_)
      {
        if (!DestroyWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to DestroyWindow(%@): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      window_,
                      ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
        closeWindow_ = false;
        window_ = NULL;
      } // end IF

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_RESIZE:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      if (!inherited::IMediaControl_ ||
          !inherited::IGraphBuilder_)
        break;

      // stop/disconnect the filter graph
      bool was_running = false;
      Stream_Module_Device_DirectShow_Graph_t filter_graph_layout;
      struct Stream_Module_Device_DirectShow_GraphConfigurationEntry graph_entry;
      Stream_Module_Device_DirectShow_GraphConfiguration_t filter_graph_configuration;
      IBaseFilter* filter_p = NULL;
      IPin* pin_p = NULL;
      enum _FilterState filter_state = State_Stopped;
      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
      unsigned int width, height;
      const SessionDataType& session_data_r = inherited::sessionData_->get ();

      result_2 =
        inherited::IMediaControl_->GetState (INFINITE,
                                             reinterpret_cast<OAFilterState*> (&filter_state));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaControl::GetState(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
        goto error_2;
      } // end IF
      if ((filter_state == State_Paused) ||
          (filter_state == State_Running))
      {
        was_running = (filter_state == State_Running);

        result_2 = IMediaControl_->Stop ();
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
          goto error_2;
        } // end IF
      } // end IF

      Stream_Module_Device_DirectShow_Tools::get (inherited::IGraphBuilder_,
                                                  (inherited::push_ ? MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L
                                                                    : MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L),
                                                  filter_graph_layout);
      for (Stream_Module_Device_DirectShow_GraphConstIterator_t iterator = filter_graph_layout.begin ();
           iterator != filter_graph_layout.end ();
           ++iterator)
      {
        graph_entry.filterName = *iterator;
        filter_graph_configuration.push_back (graph_entry);
      } // end FOR
      result_2 =
        inherited::IGraphBuilder_->FindFilterByName ((inherited::push_ ? MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L
                                                                       : MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L),
                                                     &filter_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    (inherited::push_ ? ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)
                                      : ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        goto error_2;
      } // end IF
      pin_p = Stream_Module_Device_DirectShow_Tools::pin (filter_p,
                                                          PINDIR_OUTPUT);
      if (!pin_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::pin(\"%s\",PINDIR_OUTPUT): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    (inherited::push_ ? ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)
                                      : ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

        filter_p->Release ();

        goto error_2;
      } // end IF
      filter_p->Release ();
      filter_p = NULL;
      result_2 =
        pin_p->ConnectionMediaType (filter_graph_configuration.front ().mediaType);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IPin::ConnectionMediaType(\"%s\"/\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    (inherited::push_ ? ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)
                                      : ACE_TEXT_WCHAR_TO_TCHAR (MODULE_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)),
                    ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

        pin_p->Release ();

        goto error_2;
      } // end IF
      pin_p->Release ();
      pin_p = NULL;
      if (!Stream_Module_Device_DirectShow_Tools::disconnect (inherited::IGraphBuilder_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::disconnect(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // update the source filter input media format and reconnect
      ACE_ASSERT (session_data_r.format);
      ACE_ASSERT (session_data_r.format->pbFormat);
      if (session_data_r.format->formattype == FORMAT_VideoInfo)
      {
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.format->pbFormat);
        width = video_info_header_p->bmiHeader.biWidth;
        height = video_info_header_p->bmiHeader.biHeight;
      } // end IF
      else if (session_data_r.format->formattype == FORMAT_VideoInfo2)
      {
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (session_data_r.format->pbFormat);
        width = video_info_header2_p->bmiHeader.biWidth;
        height = video_info_header2_p->bmiHeader.biHeight;
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::GUIDToString (session_data_r.format->formattype).c_str ())));
        goto error_2;
      } // end ELSE
      if (filter_graph_configuration.front ().mediaType->formattype == FORMAT_VideoInfo)
      { 
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (filter_graph_configuration.front ().mediaType->pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight =
          ((video_info_header_p->bmiHeader.biHeight < 0) ? -static_cast<LONG> (height) : height);
        video_info_header_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header_p->bmiHeader);
      } // end IF
      else if (filter_graph_configuration.front ().mediaType->formattype == FORMAT_VideoInfo2)
      {
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (filter_graph_configuration.front ().mediaType->pbFormat);
        video_info_header2_p->bmiHeader.biWidth = width;
        video_info_header2_p->bmiHeader.biHeight =
          ((video_info_header2_p->bmiHeader.biHeight < 0) ? -static_cast<LONG> (height) : height);
        video_info_header2_p->bmiHeader.biSizeImage =
          DIBSIZE (video_info_header2_p->bmiHeader);
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::GUIDToString (filter_graph_configuration.front ().mediaType->formattype).c_str ())));
        goto error_2;
      } // end ELSE
      if (!Stream_Module_Device_DirectShow_Tools::connect (inherited::IGraphBuilder_,
                                                           filter_graph_configuration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::connect(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // restart the filter graph ?
      if (was_running)
      {
        result_2 = inherited::IMediaControl_->Run ();
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Run(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
          goto error_2;
        } // end IF
      } // end IF

      break;

error_2:
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
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
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
        result_2 = IVideoWindow_->put_FullScreenMode (OAFALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

        result_2 = IVideoWindow_->put_Visible (OAFALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_Visible(OAFALSE): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));

        result_2 = IVideoWindow_->put_MessageDrain (NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(NULL): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));

        result_2 = IVideoWindow_->put_Owner (NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(NULL): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));

        IVideoWindow_->Release ();
        IVideoWindow_ = NULL;
      } // end IF
      else if (IMFVideoDisplayControl_)
      {
        result_2 = IMFVideoDisplayControl_->SetFullscreen (FALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(FALSE): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

        IMFVideoDisplayControl_->Release ();
        IMFVideoDisplayControl_ = NULL;
      } // end ELSEIF

      // deregister graph from the ROT ?
      if (ROTID_)
      {
        if (!Stream_Module_Device_DirectShow_Tools::removeFromROT (ROTID_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      inherited::mod_->name (),
                      ROTID_));
        ROTID_ = 0;
      } // end IF

//continue_2:
      if (inherited::IMediaEventEx_)
      {
        result_2 = inherited::IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2).c_str ())));
        inherited::IMediaEventEx_->Release ();
        inherited::IMediaEventEx_ = NULL;
      } // end IF

      if (inherited::IMediaControl_)
      {
        // stop DirectShow streaming thread ?
        if (inherited::push_)
          inherited::stop (false,  // wait for completion ?
                           false); // N/A

        // stop previewing video data (blocks)
        result_2 = inherited::IMediaControl_->Stop ();
        if (FAILED (result_2)) // VFW_E_NO_ALLOCATOR: 0x8004020A
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Tools::errorToString (result_2, true).c_str ())));
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
        if (!DestroyWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to DestroyWindow(%@): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      window_,
                      ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
        window_ = NULL;
        closeWindow_ = false;
      } // end IF

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
                    ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      result = IVideoWindow_->put_Owner (NULL);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

      IVideoWindow_->Release ();
      IVideoWindow_ = NULL;
    } // end IF
    if (IMFVideoDisplayControl_)
    {
      IMFVideoDisplayControl_->Release ();
      IMFVideoDisplayControl_ = NULL;
    } // end IF

    if (window_ && closeWindow_)
    {
      ShowWindow (window_, FALSE);

      closeWindow_ = false;
    } // end IF
    window_ = NULL;
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
                                                                   struct tagRECT& windowArea_inout,
                                                                   IVideoWindow*& IVideoWindow_out,
                                                                   IMFVideoDisplayControl*& IMFVideoDisplayControl_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize_DirectShow"));

  HRESULT result = E_FAIL;
  IBaseFilter* ibase_filter_p = NULL;
  IMediaEventEx* imedia_event_ex_p = NULL;
  long window_message = WM_USER;
  LONG_PTR instance_data_p = NULL;
  BOOL result_2 = FALSE;
  struct _GUID GUID_s = GUID_NULL;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;

  // initialize return value(s)
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release ();
    IVideoWindow_out = NULL;
  } // end IF
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release ();
    IMFVideoDisplayControl_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);

  ACE_ASSERT (mediaType_in.formattype == FORMAT_VideoInfo);
  ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
  video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
  if (!windowHandle_inout)
  {
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
                  ACE_TEXT ("%s: failed to CreateWindow(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (::GetLastError ()).c_str ())));
      goto error;
    } // end IF

    result_2 = GetClientRect (windowHandle_inout, &windowArea_inout);
    if (!result_2)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to GetClientRect(0x%@): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Tools::errorToString (GetLastError ()).c_str ())));
      goto error;
    } // end IF
  } // end IF
  ACE_ASSERT (windowHandle_inout);

  result =
    IGraphBuilder_in->QueryInterface (IID_PPV_ARGS (&imedia_event_ex_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaEventEx): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (imedia_event_ex_p);
  result =
    imedia_event_ex_p->SetNotifyWindow (reinterpret_cast<OAHWND> (windowHandle_inout),
                                        window_message,
                                        instance_data_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(0x%@): \"%s\", aborting\n"),
                inherited::mod_->name (),
                windowHandle_inout,
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window (handle: 0x%@) will receive DirectShow messages (id: 0x%x)\n"),
              inherited::mod_->name (),
              windowHandle_inout,
              window_message));
  imedia_event_ex_p->Release ();
  imedia_event_ex_p = NULL;

  // retrieve video window control and configure output
  result =
    IGraphBuilder_in->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO,
                                        &ibase_filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (ibase_filter_p);
  result = ibase_filter_p->GetClassID (&GUID_s);
  ACE_ASSERT (SUCCEEDED (result));
  if (InlineIsEqualGUID (CLSID_EnhancedVideoRenderer, GUID_s))
  { 
    if (!Stream_Module_Device_DirectShow_Tools::getVideoWindow (IGraphBuilder_in,
                                                                MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO,
                                                                IMFVideoDisplayControl_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::getVideoWindow(), aborting\n")));
      goto error;
    } //  end IF
    ACE_ASSERT (IMFVideoDisplayControl_out);

    result =
      IMFVideoDisplayControl_out->SetVideoWindow (windowHandle_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetVideoWindow(%@): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result =
      IMFVideoDisplayControl_out->SetAspectRatioMode (MFVideoARMode_PreservePicture);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetAspectRatioMode(MFVideoARMode_PreservePicture): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result =
      IMFVideoDisplayControl_out->SetVideoPosition (NULL,               // <-- default: entire video
                                                    &windowArea_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetVideoPosition(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = IMFVideoDisplayControl_out->SetFullscreen (fullScreen_in);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF
  else
  { // set up windowless mode ?
#if (_WIN32_WINNT < _WIN32_WINNT_WINXP)
#elif (_WIN32_WINNT < _WIN32_WINNT_VISTA)
    if (InlineIsEqualGUID (CLSID_VideoMixingRenderer, GUID_s) ||
        InlineIsEqualGUID (CLSID_VideoRendererDefault, GUID_s))
    {
      IVMRFilterConfig* ivmr_filter_config_p = NULL;
      result =
        ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_filter_config_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRFilterConfig): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (ivmr_filter_config_p);
      result = ivmr_filter_config_p->SetRenderingMode (VMRMode_Windowless);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVMRFilterConfig::SetRenderingMode(VMRMode_Windowless): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        ivmr_filter_config_p->Release ();
        goto error;
      } // end IF
      ivmr_filter_config_p->Release ();
      IVMRWindowlessControl* ivmr_windowless_control_p = NULL;
      result =
        ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_windowless_control_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRWindowlessControl): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (ivmr_windowless_control_p);
      result = ivmr_windowless_control_p->SetVideoClippingWindow (windowHandle_inout);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVMRWindowlessControl::SetVideoClippingWindow(%@): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    windowHandle_inout,
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        ivmr_windowless_control_p->Release ();
        goto error;
      } // end IF
      ivmr_windowless_control_p->Release ();
    } // end IF
    else if (InlineIsEqualGUID (CLSID_VideoMixingRenderer9, GUID_s))
    {
      IVMRFilterConfig9* ivmr_filter_config_p = NULL;
      result =
        ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_filter_config_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRFilterConfig9): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (ivmr_filter_config_p);
      result = ivmr_filter_config_p->SetRenderingMode (VMR9Mode_Windowless);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVMRFilterConfig::SetRenderingMode(VMR9Mode_Windowless): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        ivmr_filter_config_p->Release ();
        goto error;
      } // end IF
      ivmr_filter_config_p->Release ();
      IVMRWindowlessControl9* ivmr_windowless_control_p = NULL;
      result =
        ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_windowless_control_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRWindowlessControl9): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (ivmr_windowless_control_p);
      result = ivmr_windowless_control_p->SetVideoClippingWindow (windowHandle_inout);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVMRWindowlessControl9::SetVideoClippingWindow(%@): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                    windowHandle_inout,
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
        ivmr_windowless_control_p->Release ();
        goto error;
      } // end IF
      ivmr_windowless_control_p->Release ();
    } // end ELSEIF
#else
    result =
      IGraphBuilder_in->QueryInterface (IID_PPV_ARGS (&IVideoWindow_out));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IVideoWindow): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
//result =
//  ibase_filter_p->QueryInterface (IID_PPV_ARGS (&IVideoWindow_out));
//if (FAILED (result))
//{
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVideoWindow): \"%s\", aborting\n"),
//              ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
//              ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
//  goto error;
//} // end IF
    ACE_ASSERT (IVideoWindow_out);

    result =
      IVideoWindow_out->put_Owner (reinterpret_cast<OAHWND> (windowHandle_inout));
    // *NOTE*: "...For the Filter Graph Manager's implementation, if the graph
    //         does not contain a video renderer filter, all methods return
    //         E_NOINTERFACE..."
    if (FAILED (result)) // E_NOINTERFACE: 0x80004002
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(0x%@): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Tools::errorToString (result, true).c_str ())));

    result = IVideoWindow_out->put_WindowStyle (WS_CHILD | WS_CLIPSIBLINGS);
    if (FAILED (result)) // E_NOINTERFACE: 0x80004002
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_WindowStyle(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    result =
      IVideoWindow_out->SetWindowPosition (windowArea_inout.left,
                                           windowArea_inout.top,
                                           (windowArea_inout.right -
                                            windowArea_inout.left),
                                           (windowArea_inout.bottom -
                                            windowArea_inout.top));
    if (FAILED (result)) // E_NOINTERFACE: 0x80004002
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  windowArea_inout.left, windowArea_inout.top,
                  (windowArea_inout.right - windowArea_inout.left),
                  (windowArea_inout.bottom - windowArea_inout.top),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    // redirect mouse and keyboard events to the main gtk window
    result =
      IVideoWindow_out->put_MessageDrain (reinterpret_cast<OAHWND> (windowHandle_inout));
    if (FAILED (result)) // E_NOINTERFACE      : 0x80004002
                         // VFW_E_NOT_CONNECTED: 0x80040209
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(0x%@): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    result = IVideoWindow_out->put_Visible (OATRUE);
    if (FAILED (result)) // E_NOINTERFACE: 0x80004002
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_Visible(OATRUE): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    result =
      IVideoWindow_out->put_FullScreenMode (fullScreen_in ? OATRUE : OAFALSE);
    if (FAILED (result)) // E_NOINTERFACE: 0x80004002
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::errorToString (result).c_str ())));

    if (!fullScreen_in)
    {
      result = IVideoWindow_out->SetWindowForeground (OATRUE);
      if (FAILED (result)) // E_NOINTERFACE: 0x80004002
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::SetWindowForeground(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::errorToString (result).c_str ())));
    } // end IF

    // *TODO*: forward WM_MOVE messages to the video window via NotifyOwnerMessage
    //         (see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd407298(v=vs.85).aspx)
  } // end ELSE
#endif

  return true;

error:
  if (ibase_filter_p)
    ibase_filter_p->Release ();
  if (imedia_event_ex_p)
    imedia_event_ex_p->Release ();
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release ();
    IVideoWindow_out = NULL;
  } // end IF
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release ();
    IMFVideoDisplayControl_out = NULL;
  } // end IF

  return false;
}
