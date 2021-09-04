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

///////////////////////////////////////////////////////////////////////////
// Define OLE Automation constants
///////////////////////////////////////////////////////////////////////////
#ifndef OATRUE
#define OATRUE (-1)
#endif // OATRUE
#ifndef OAFALSE
#define OAFALSE (0)
#endif // OAFALSE
//#include <DShow.h>
#include <Mferror.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <vfwmsgs.h>
#include <WinUser.h>

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_log_tools.h"

#include "common_ui_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_defines.h"

#include "stream_lib_tools.h"

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
    if (unlikely (FAILED (result)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(NULL) \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

    result = IVideoWindow_->put_MessageDrain (NULL);
    if (unlikely (FAILED (result)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(NULL) \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

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
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::get_FullScreenMode(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    is_fullscreen = (fullscreen_mode == OATRUE);
  } // end IF
  else
  {
    BOOL fullscreen_b = FALSE;
get_mode:
    result = IMFVideoDisplayControl_->GetFullscreen (&fullscreen_b);
    if (unlikely (FAILED (result))) // 0xC00D36B2: MF_E_INVALIDREQUEST
    { // *TODO*: remove tight loop here
      if (result == MF_E_INVALIDREQUEST) // <-- (still) in transition
        goto get_mode;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::GetFullscreen(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    is_fullscreen = fullscreen_b;
  } // end ELSE

  if (is_fullscreen)
  { // --> switch to window
    if (IVideoWindow_)
    {
      result = IVideoWindow_->put_FullScreenMode (OAFALSE);
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(OAFALSE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      result = IVideoWindow_->put_MessageDrain ((OAHWND)window_);
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      result = IVideoWindow_->SetWindowForeground (OATRUE);
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::SetWindowForeground(OATRUE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
    } // end IF
    else
    {
      struct tagRECT area_s;
      BOOL result = GetClientRect (window_, &area_s);
      ACE_ASSERT (result);
      UINT uFlags = (SWP_ASYNCWINDOWPOS |
                     SWP_NOACTIVATE     |
                     SWP_NOMOVE         |
                     SWP_NOSIZE);
      if (unlikely (!SetWindowPos (window_,
                                   HWND_NOTOPMOST,
                                   area_s.left, area_s.top,
                                   area_s.right - area_s.left, area_s.bottom - area_s.top,
                                   uFlags)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SetWindowPos(%@): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    window_,
                    ACE_TEXT (Common_Error_Tools::errorToString (GetLastError ()).c_str ())));
        return;
      } // end IF

      result = IMFVideoDisplayControl_->SetFullscreen (FALSE);
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(FALSE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      result = IVideoWindow_->put_FullScreenMode (OATRUE);
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(OATRUE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
    } // end IF
    else
    {
      struct tagRECT area_s;
      BOOL result = GetClientRect (window_, &area_s);
      ACE_ASSERT (result);
      UINT uFlags = (SWP_ASYNCWINDOWPOS |
                     SWP_NOACTIVATE     |
                     SWP_NOMOVE         |
                     SWP_NOSIZE);
      if (unlikely (!SetWindowPos (window_,
                                   HWND_TOPMOST,
                                   area_s.left, area_s.top,
                                   area_s.right - area_s.left, area_s.bottom - area_s.top,
                                   uFlags)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SetWindowPos(%@): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    window_,
                    ACE_TEXT (Common_Error_Tools::errorToString (GetLastError ()).c_str ())));
        return;
      } // end IF

      result = IMFVideoDisplayControl_->SetFullscreen (TRUE);
      if (unlikely (FAILED (result)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(TRUE): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                               FilterType>::handleDataMessage (DataMessageType*& message_inout,
                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::handleDataMessage"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  // forward message to the directshow filter graph ?
  if (likely (!InlineIsEqualGUID (inherited::configuration_->filterCLSID, GUID_NULL)))
    inherited::handleDataMessage (message_inout,
                                  passMessageDownstream_out);
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
      ACE_ASSERT (inherited::msg_queue_);
      unsigned int result_3 =
        inherited::msg_queue_->flush (); // flush session data ?
      ACE_UNUSED_ARG (result_3);
      break;
    }
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      //// step1: initialize COM
      //// sanity check(s)
      //Stream_MediaFramework_Tools::initialize (STREAM_MEDIAFRAMEWORK_DIRECTSHOW);
      //Stream_Module_Device_DirectShow_Tools::initialize (true);

      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      const SessionDataType& session_data_r = inherited::sessionData_->getR ();
      unsigned int height, width;
      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
      bool is_running = false;
      bool remove_from_ROT = false;
      bool add_to_ROT = false;
#if defined (_DEBUG)
      std::string log_file_name;
#endif // _DEBUG

      // step2: assemble display format
      struct tagRECT area_s;
      ACE_OS::memset (&area_s, 0, sizeof (struct tagRECT));
      if (window_)
      {
        BOOL result = GetClientRect (window_, &area_s);
        ACE_ASSERT (result);
        height = area_s.bottom - area_s.top;
        width = area_s.right - area_s.left;
      } // end IF
      else
      {
        height = STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_HEIGHT;
        width = STREAM_DEV_CAM_DEFAULT_CAPTURE_SIZE_WIDTH;
        
      } // end ELSE

      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited::getMediaType (session_data_r.formats.back (),
                               media_type_s);
      ACE_ASSERT (media_type_s.pbFormat);
      if (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo))
      {
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_s.pbFormat);
        video_info_header_p->bmiHeader.biWidth = width;
        video_info_header_p->bmiHeader.biHeight = height;
        video_info_header_p->bmiHeader.biSizeImage =
          Stream_MediaFramework_Tools::frameSize (media_type_s);
        // *NOTE*: empty --> use entire video
        result_2 = SetRectEmpty (&video_info_header_p->rcSource);
        ACE_ASSERT (SUCCEEDED (result_2));
        // *NOTE*: empty --> fill entire buffer
        result_2 = SetRectEmpty (&video_info_header_p->rcTarget);
        ACE_ASSERT (SUCCEEDED (result_2));
        video_info_header_p->dwBitRate =
          (video_info_header_p->bmiHeader.biSizeImage *
           8 *
           (10000000 / static_cast<DWORD> (video_info_header_p->AvgTimePerFrame)));
        media_type_s.lSampleSize =
          video_info_header_p->bmiHeader.biSizeImage;
      } // end IF
      else if (InlineIsEqualGUID (media_type_s.formattype, FORMAT_VideoInfo2))
      {
        video_info_header2_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_s.pbFormat);
        video_info_header2_p->bmiHeader.biWidth = width;
        video_info_header2_p->bmiHeader.biHeight = height;
        video_info_header2_p->bmiHeader.biSizeImage =
          Stream_MediaFramework_Tools::frameSize (media_type_s);
        media_type_s.lSampleSize =
          video_info_header2_p->bmiHeader.biSizeImage;
      } // end ELSE IF
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown media format type (was: \"%s\"), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (media_type_s.formattype).c_str ())));
        Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
        goto error;
      } // end ELSE

      // step3: set up filter graph ?
      if (!inherited::IGraphBuilder_)
      {
        // sanity check(s)
        // *TODO*: remove type inferences
        ACE_ASSERT (inherited::configuration_->filterConfiguration);

        if (unlikely (!inherited::loadGraph (inherited::configuration_->filterCLSID,
                                             *inherited::configuration_->filterConfiguration,
                                             media_type_s,
                                             window_,
                                             inherited::IGraphBuilder_)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Target_T::loadGraph(), aborting\n"),
                      inherited::mod_->name ()));
          Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
          goto error;
        } // end IF
        ACE_ASSERT (inherited::IGraphBuilder_);
        add_to_ROT = true;
      } // end IF
      ACE_ASSERT (inherited::IGraphBuilder_);
#if defined (_DEBUG)
      log_file_name =
        Common_Log_Tools::getLogDirectory (ACE_TEXT_ALWAYS_CHAR (""),
                                           0);
      log_file_name += ACE_DIRECTORY_SEPARATOR_STR;
      log_file_name += STREAM_LIB_DIRECTSHOW_LOGFILE_NAME;
      Stream_MediaFramework_DirectShow_Tools::debug (inherited::IGraphBuilder_,
                                                     log_file_name);
#endif // _DEBUG

      if (unlikely (!initialize_DirectShow (inherited::IGraphBuilder_,
                                            media_type_s,
                                            window_,
                                            inherited::configuration_->fullScreen,
                                            area_s,
                                            IVideoWindow_,
                                            IMFVideoDisplayControl_)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_DirectShow(), aborting\n"),
                    inherited::mod_->name ()));
        Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
        goto error;
      } // end IF
      ACE_ASSERT (IVideoWindow_ || IMFVideoDisplayControl_);
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      // update configuration
      if (IVideoWindow_)
      {
        if (inherited::configuration_->windowController)
          inherited::configuration_->windowController->Release ();
        IVideoWindow_->AddRef ();
        inherited::configuration_->windowController = IVideoWindow_;
      }
      if (IMFVideoDisplayControl_)
      {
        if (inherited::configuration_->windowController2)
          inherited::configuration_->windowController2->Release ();
        IMFVideoDisplayControl_->AddRef ();
        inherited::configuration_->windowController2 = IMFVideoDisplayControl_;
      }
      ACE_ASSERT (window_);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@\n"),
                  inherited::mod_->name (),
                  window_));
#endif // _DEBUG

      // retrieve interfaces for media control and the event sink
      if (!inherited::IMediaControl_)
      {
        result_2 =
          inherited::IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&(inherited::IMediaControl_)));
        if (unlikely (FAILED (result_2)))
          goto error;
      } // end IF
      if (!inherited::IMediaEventEx_)
      {
        result_2 =
          inherited::IGraphBuilder_->QueryInterface (IID_PPV_ARGS (&(inherited::IMediaEventEx_)));
        if (unlikely (FAILED (result_2)))
          goto error;
      } // end IF
      ACE_ASSERT (inherited::IMediaControl_);
      ACE_ASSERT (inherited::IMediaEventEx_);

      // (re-)start forwarding data
      OAFilterState graph_state = 0;
      result_2 =
        inherited::IMediaControl_->GetState (INFINITE,
                                             &graph_state);
      if (unlikely (FAILED (result_2))) // VFW_S_STATE_INTERMEDIATE: 0x00040237
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaControl::GetState(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        goto error;
      } // end IF
      switch (static_cast<enum _FilterState> (graph_state))
      {
        case State_Paused:
        case State_Stopped:
        {
          result_2 = inherited::IMediaControl_->Run ();
          if (unlikely (FAILED (result_2))) // VFW_E_SIZENOTSET: 0x80040212
                                            // E_OUTOFMEMORY   : 0x8007000E
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to IMediaControl::Run(): \"%s\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
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
            //              ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
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

      // register graph in the ROT (so graphedt.exe can see it) ?
      if (add_to_ROT)
      { ACE_ASSERT (!inherited::ROTID_);
        if (unlikely (!Stream_MediaFramework_DirectShow_Tools::addToROT (IGraphBuilder_,
                                                                         inherited::ROTID_)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::addToROT(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_ASSERT (inherited::ROTID_);
        remove_from_ROT = true;
#if defined (_DEBUG)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: registered filter graph in running object table (id: %u)\n"),
                    inherited::mod_->name (),
                    inherited::ROTID_));
#endif // _DEBUG
      } // end IF

      break;

error:
      // deregister graph from the ROT ?
      if (remove_from_ROT)
      { ACE_ASSERT (inherited::ROTID_);
        if (unlikely (!Stream_MediaFramework_DirectShow_Tools::removeFromROT (inherited::ROTID_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      inherited::mod_->name (),
                      inherited::ROTID_));
        inherited::ROTID_ = 0;
      } // end IF
      if (is_running)
      { ACE_ASSERT (inherited::IMediaControl_);
        result_2 = inherited::IMediaControl_->Stop ();
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (window_ && closeWindow_)
      {
        if (unlikely (!DestroyWindow (window_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to DestroyWindow(%@): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      window_,
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
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
      Stream_MediaFramework_DirectShow_Graph_t filter_graph_layout;
      struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry graph_entry;
      Stream_MediaFramework_DirectShow_GraphConfiguration_t filter_graph_configuration;
      IBaseFilter* filter_p = NULL;
      IPin* pin_p = NULL;
      enum _FilterState filter_state = State_Stopped;
      const SessionDataType& session_data_r = inherited::sessionData_->getR ();

      result_2 =
        inherited::IMediaControl_->GetState (INFINITE,
                                             reinterpret_cast<OAFilterState*> (&filter_state));
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMediaControl::GetState(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        goto error_2;
      } // end IF
      if ((filter_state == State_Paused) ||
          (filter_state == State_Running))
      {
        was_running = (filter_state == State_Running);

        result_2 = IMediaControl_->Stop ();
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
          goto error_2;
        } // end IF
      } // end IF

      Stream_MediaFramework_DirectShow_Tools::get (inherited::IGraphBuilder_,
                                                   (inherited::push_ ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L
                                                                     : STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L),
                                                   filter_graph_layout);
      for (Stream_MediaFramework_DirectShow_GraphConstIterator_t iterator = filter_graph_layout.begin ();
           iterator != filter_graph_layout.end ();
           ++iterator)
      {
        graph_entry.filterName = *iterator;
        filter_graph_configuration.push_back (graph_entry);
      } // end FOR
      result_2 =
        inherited::IGraphBuilder_->FindFilterByName ((inherited::push_ ? STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L
                                                                       : STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L),
                                                     &filter_p);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    (inherited::push_ ? ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)
                                      : ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        goto error_2;
      } // end IF
      pin_p = Stream_MediaFramework_DirectShow_Tools::pin (filter_p,
                                                           PINDIR_OUTPUT);
      if (unlikely (!pin_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::pin(\"%s\",PINDIR_OUTPUT): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    (inherited::push_ ? ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)
                                      : ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        filter_p->Release (); filter_p = NULL;
        goto error_2;
      } // end IF
      filter_p->Release (); filter_p = NULL;
      result_2 =
        pin_p->ConnectionMediaType (filter_graph_configuration.front ().mediaType);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IPin::ConnectionMediaType(\"%s\"/\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    (inherited::push_ ? ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE_L)
                                      : ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE_L)),
                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        pin_p->Release (); pin_p = NULL;
        goto error_2;
      } // end IF
      pin_p->Release (); pin_p = NULL;
      if (!Stream_MediaFramework_DirectShow_Tools::disconnect (inherited::IGraphBuilder_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // update the source filter input media format and reconnect
      ACE_ASSERT (!session_data_r.formats.empty ());
      ACE_ASSERT (false); // *TODO*
      //Stream_MediaFramework_DirectShow_Tools::resize (Stream_MediaFramework_DirectShow_Tools::toResolution (session_data_r.formats.back ()),
      //                                                *filter_graph_configuration.front ().mediaType);
      if (unlikely (!Stream_MediaFramework_DirectShow_Tools::connect (inherited::IGraphBuilder_,
                                                                      filter_graph_configuration)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::connect(), aborting\n"),
                    inherited::mod_->name ()));
        goto error_2;
      } // end IF

      // restart the filter graph ?
      if (was_running)
      {
        result_2 = inherited::IMediaControl_->Run ();
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Run(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
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
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", returning\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      // step1: dispatch all data to DirectShow
      inherited::idle ();
      // step2: *TODO*: wait for DirectShow

      // *IMPORTANT NOTE*: "Reset the owner to NULL before releasing the Filter
      //                   Graph Manager. Otherwise, messages will continue to
      //                   be sent to this window and errors will likely occur
      //                   when the application is terminated. ..."
      if (IVideoWindow_)
      {
        result_2 = IVideoWindow_->put_FullScreenMode (OAFALSE);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

        result_2 = IVideoWindow_->put_Visible (OAFALSE);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_Visible(OAFALSE): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));

        result_2 = IVideoWindow_->put_MessageDrain (NULL);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(NULL): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));

        result_2 = IVideoWindow_->put_Owner (NULL);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(NULL): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));

        IVideoWindow_->Release (); IVideoWindow_ = NULL;
      } // end IF
      else if (IMFVideoDisplayControl_)
      {
        result_2 = IMFVideoDisplayControl_->SetFullscreen (FALSE);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(FALSE): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

        IMFVideoDisplayControl_->Release (); IMFVideoDisplayControl_ = NULL;
      } // end ELSEIF

      // deregister graph from the ROT ?
      if (inherited::ROTID_)
      {
        if (unlikely (!Stream_MediaFramework_DirectShow_Tools::removeFromROT (inherited::ROTID_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::removeFromROT(%d), continuing\n"),
                      inherited::mod_->name (),
                      inherited::ROTID_));
        inherited::ROTID_ = 0;
      } // end IF

//continue_2:
      if (inherited::IMediaEventEx_)
      {
        result_2 = inherited::IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
        if (unlikely (FAILED (result_2)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        inherited::IMediaEventEx_->Release (); inherited::IMediaEventEx_ = NULL;
      } // end IF

      if (inherited::IMediaControl_)
      {
        // stop DirectShow streaming thread ?
        if (inherited::push_)
          inherited::stop (true,   // wait ?
                           false); // high priority ?

        // stop previewing video data (blocks)
        result_2 = inherited::IMediaControl_->Stop ();
        if (unlikely (FAILED (result_2))) // VFW_E_NO_ALLOCATOR: 0x8004020A
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMediaControl::Stop(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        inherited::IMediaControl_->Release (); inherited::IMediaControl_ = NULL;
      } // end IF

      if (inherited::IGraphBuilder_)
      {
        inherited::IGraphBuilder_->Release (); inherited::IGraphBuilder_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (window_ && closeWindow_)
      {
        if (unlikely (!DestroyWindow (window_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to DestroyWindow(%@): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      window_,
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
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
      if (unlikely (FAILED (result)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

      result = IVideoWindow_->put_Owner (NULL);
      if (unlikely (FAILED (result)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

      IVideoWindow_->Release (); IVideoWindow_ = NULL;
    } // end IF
    if (IMFVideoDisplayControl_)
    {
      IMFVideoDisplayControl_->Release (); IMFVideoDisplayControl_ = NULL;
    } // end IF

    if (window_ && closeWindow_)
    {
      ShowWindow (window_, FALSE);
      closeWindow_ = false;
    } // end IF
    window_ = NULL;
  } // end IF

  // sanity check(s)
  //ACE_ASSERT (configuration_in.window);

  inherited::getWindowType (configuration_in.window,
                            window_);

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
  //struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  struct Common_UI_DisplayDevice display_device_s;
  MONITORINFOEX monitor_info_ex_s;
  unsigned int delta_x, delta_y;

  // initialize return value(s)
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release (); IVideoWindow_out = NULL;
  } // end IF
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release (); IMFVideoDisplayControl_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IGraphBuilder_in);
  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo));
  ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));

  video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
  ACE_ASSERT (video_info_header_p);

  if (windowHandle_inout)
    goto continue_;
  // retrieve display device 'geometry' data (i.e. monitor coordinates)
  ACE_ASSERT (inherited::configuration_->deviceIdentifier.identifierDiscriminator == Stream_Device_Identifier::STRING);
  display_device_s =
    Common_UI_Tools::getDisplay (ACE_TEXT_ALWAYS_CHAR (inherited::configuration_->deviceIdentifier.identifier._string));
  ACE_ASSERT (display_device_s.handle);
  ACE_OS::memset (&monitor_info_ex_s, 0, sizeof (MONITORINFOEX));
  monitor_info_ex_s.cbSize = sizeof (MONITORINFOEX);
  if (unlikely (!GetMonitorInfo (display_device_s.handle,
                                 reinterpret_cast<struct tagMONITORINFO*> (&monitor_info_ex_s))))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to GetMonitorInfo(\"%s\"): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->deviceIdentifier.identifier._string),
                ACE_TEXT (Common_Error_Tools::errorToString (GetLastError ()).c_str ())));
    goto error;
  } // end IF
  // *NOTE*: center new windows on the display device
  ACE_ASSERT ((monitor_info_ex_s.rcWork.right - monitor_info_ex_s.rcWork.left) >= video_info_header_p->bmiHeader.biWidth);
  // *NOTE*: disregard the task bars' height
  //ACE_ASSERT ((monitor_info_ex_s.rcWork.bottom - monitor_info_ex_s.rcWork.top) >= video_info_header_p->bmiHeader.biHeight);
  ACE_ASSERT ((monitor_info_ex_s.rcWork.bottom - 0) >= video_info_header_p->bmiHeader.biHeight);
  delta_x =
    (fullScreen_in ? 0
                   : (static_cast<unsigned int> ((monitor_info_ex_s.rcWork.right - monitor_info_ex_s.rcWork.left) - video_info_header_p->bmiHeader.biWidth) / 2));
  delta_y =
    (fullScreen_in ? 0
                   : (static_cast<unsigned int> ((monitor_info_ex_s.rcWork.bottom - monitor_info_ex_s.rcWork.top) - video_info_header_p->bmiHeader.biHeight) / 2));

  DWORD window_style = (WS_CAPTION     |
                        WS_MAXIMIZEBOX |
                        WS_MINIMIZEBOX |
                        //WS_OVERLAPPED     |
                        WS_SIZEBOX     |
                        WS_SYSMENU     |
                        WS_VISIBLE);
  DWORD window_style_ex = (WS_EX_APPWINDOW     |
                           WS_EX_RIGHTSCROLLBAR// |
                           /*WS_EX_WINDOWEDGE*/);
  windowHandle_inout =
    CreateWindowEx (window_style_ex,                                  // dwExStyle
#if defined (UNICODE)
                    ACE_TEXT_ALWAYS_WCHAR ("EDIT"),                   // lpClassName
                    ACE_TEXT_ALWAYS_WCHAR (inherited::mod_->name ()), // lpWindowName
#else
                    ACE_TEXT_ALWAYS_CHAR ("EDIT"),                    // lpClassName
                    ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()),  // lpWindowName
#endif // UNICODE
                    window_style,                                     // dwStyle
                    //CW_USEDEFAULT, CW_USEDEFAULT,                    // x,y
                    delta_x, delta_y,                                 // x,y
                    video_info_header_p->bmiHeader.biWidth,           // width
                    video_info_header_p->bmiHeader.biHeight,          // height
                    //parent_window_handle,                           // hWndParent
                    NULL,                                             // hWndParent
                    NULL,                                             // hMenu
                    GetModuleHandle (NULL),                           // hInstance
                    NULL);                                            // lpParam
  if (unlikely (!windowHandle_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CreateWindow(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: opened display window (size: %dx%d, handle: 0x%@)\n"),
              inherited::mod_->name (),
              video_info_header_p->bmiHeader.biWidth, video_info_header_p->bmiHeader.biHeight,
              windowHandle_inout));

  result_2 = GetClientRect (windowHandle_inout, &windowArea_inout);
  if (unlikely (!result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to GetClientRect(0x%@): \"%s\", aborting\n"),
                inherited::mod_->name (),
                windowHandle_inout,
                ACE_TEXT (Common_Error_Tools::errorToString (GetLastError ()).c_str ())));
    goto error;
  } // end IF
continue_:
  ACE_ASSERT (windowHandle_inout);

  result =
    IGraphBuilder_in->QueryInterface (IID_PPV_ARGS (&imedia_event_ex_p));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IMediaEventEx): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (imedia_event_ex_p);
  result =
    imedia_event_ex_p->SetNotifyWindow (reinterpret_cast<OAHWND> (windowHandle_inout),
                                        window_message,
                                        instance_data_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMediaEventEx::SetNotifyWindow(0x%@): \"%s\", aborting\n"),
                inherited::mod_->name (),
                windowHandle_inout,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window handle 0x%@ will receive DirectShow messages (id: 0x%x)\n"),
              inherited::mod_->name (),
              windowHandle_inout,
              window_message));
  imedia_event_ex_p->Release (); imedia_event_ex_p = NULL;

  // retrieve video window control and configure output
  result =
    IGraphBuilder_in->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO,
                                        &ibase_filter_p);
  if (unlikely (FAILED (result)))
  {
    result =
      IGraphBuilder_in->FindFilterByName (STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_NULL,
                                          &ibase_filter_p);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IGraphBuilder::FindFilterByName(%s): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    // --> replace null renderer with video renderer
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: replacing null renderer with video renderer\n"),
                inherited::mod_->name ()));
    Stream_MediaFramework_DirectShow_Tools::remove (IGraphBuilder_in,
                                                    ibase_filter_p);
    ibase_filter_p->Release (); ibase_filter_p = NULL;
    result =
      CoCreateInstance (STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&ibase_filter_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CoCreateInstance(%s): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Tools::GUIDToString (STREAM_LIB_DIRECTSHOW_FILTER_CLSID_VIDEO_RENDER).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (ibase_filter_p);
    Stream_MediaFramework_DirectShow_Tools::append (IGraphBuilder_in,
                                                    ibase_filter_p,
                                                    STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO);
  } // end IF
  ACE_ASSERT (ibase_filter_p);
  result = ibase_filter_p->GetClassID (&GUID_s);
  ACE_ASSERT (SUCCEEDED (result));
  if (InlineIsEqualGUID (CLSID_EnhancedVideoRenderer, GUID_s))
  { 
    if (unlikely (!Stream_MediaFramework_DirectShow_Tools::getVideoWindow (IGraphBuilder_in,
                                                                           STREAM_LIB_DIRECTSHOW_FILTER_NAME_RENDER_VIDEO,
                                                                           IMFVideoDisplayControl_out)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::getVideoWindow(), aborting\n"),
                  inherited::mod_->name ()));
      goto error;
    } //  end IF
    ACE_ASSERT (IMFVideoDisplayControl_out);

    result =
      IMFVideoDisplayControl_out->SetVideoWindow (windowHandle_inout);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetVideoWindow(%@): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    result =
      IMFVideoDisplayControl_out->SetAspectRatioMode (MFVideoARMode_PreservePicture);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetAspectRatioMode(MFVideoARMode_PreservePicture): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    struct tagRECT destination_rectangle_s;
    result = SetRectEmpty (&destination_rectangle_s);
    ACE_ASSERT (SUCCEEDED (result));
    destination_rectangle_s.right =
      (windowArea_inout.right - windowArea_inout.left);
    destination_rectangle_s.bottom =
      (windowArea_inout.bottom - windowArea_inout.top);
    result =
      IMFVideoDisplayControl_out->SetVideoPosition (NULL,                      // <-- default: entire video
                                                    &destination_rectangle_s); // <-- entire window
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetVideoPosition(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  windowHandle_inout,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = IMFVideoDisplayControl_out->SetFullscreen (fullScreen_in);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFVideoDisplayControl::SetFullscreen(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF
  else if (InlineIsEqualGUID (CLSID_VideoMixingRenderer, GUID_s) ||
           InlineIsEqualGUID (CLSID_VideoRendererDefault, GUID_s))
  { // set up windowless mode ?
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0501) // _WIN32_WINNT_WINXP
    //ACE_ASSERT (false); // *TODO*
    //ACE_NOTSUP_RETURN (false);
    //ACE_NOTREACHED (return false;);
#elif COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    IVMRFilterConfig* ivmr_filter_config_p = NULL;
    result =
      ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_filter_config_p));
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRFilterConfig): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (ivmr_filter_config_p);
    result = ivmr_filter_config_p->SetRenderingMode (VMRMode_Windowless);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVMRFilterConfig::SetRenderingMode(VMRMode_Windowless): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      ivmr_filter_config_p->Release (); ivmr_filter_config_p = NULL;
      goto error;
    } // end IF
    ivmr_filter_config_p->Release (); ivmr_filter_config_p = NULL;
    IVMRWindowlessControl* ivmr_windowless_control_p = NULL;
    result =
      ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_windowless_control_p));
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRWindowlessControl): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (ivmr_windowless_control_p);
    result = ivmr_windowless_control_p->SetVideoClippingWindow (windowHandle_inout);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVMRWindowlessControl::SetVideoClippingWindow(%@): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  windowHandle_inout,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      ivmr_windowless_control_p->Release (); ivmr_windowless_control_p = NULL;
      goto error;
    } // end IF
    ivmr_windowless_control_p->Release (); ivmr_windowless_control_p = NULL;
  } // end ELSE IF
  else if (InlineIsEqualGUID (CLSID_VideoMixingRenderer9, GUID_s))
  {
    IVMRFilterConfig9* ivmr_filter_config_p = NULL;
    result =
      ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_filter_config_p));
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRFilterConfig9): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (ivmr_filter_config_p);
    result = ivmr_filter_config_p->SetRenderingMode (VMR9Mode_Windowless);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVMRFilterConfig::SetRenderingMode(VMR9Mode_Windowless): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      ivmr_filter_config_p->Release (); ivmr_filter_config_p = NULL;
      goto error;
    } // end IF
    ivmr_filter_config_p->Release (); ivmr_filter_config_p = NULL;
    IVMRWindowlessControl9* ivmr_windowless_control_p = NULL;
    result =
      ibase_filter_p->QueryInterface (IID_PPV_ARGS (&ivmr_windowless_control_p));
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVMRWindowlessControl9): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (ivmr_windowless_control_p);
    result = ivmr_windowless_control_p->SetVideoClippingWindow (windowHandle_inout);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVMRWindowlessControl9::SetVideoClippingWindow(%@): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
                  windowHandle_inout,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      ivmr_windowless_control_p->Release (); ivmr_windowless_control_p = NULL;
      goto error;
    } // end IF
    ivmr_windowless_control_p->Release (); ivmr_windowless_control_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0501)
  } // end ELSE IF
  else if (InlineIsEqualGUID (CLSID_VideoRenderer, GUID_s)) {}
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown renderer filter (was: %s), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
    goto error;
  } // end ELSE

  result =
    IGraphBuilder_in->QueryInterface (IID_PPV_ARGS (&IVideoWindow_out));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IGraphBuilder::QueryInterface(IID_IVideoWindow): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
//result =
//  ibase_filter_p->QueryInterface (IID_PPV_ARGS (&IVideoWindow_out));
//if (unlikely (FAILED (result)))
//{
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("%s: failed to IBaseFilter::QueryInterface(IID_IVideoWindow): \"%s\", aborting\n"),
//              ACE_TEXT (Stream_Module_Device_Tools::name (ibase_filter_p).c_str ()),
//              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//  goto error;
//} // end IF
  ACE_ASSERT (IVideoWindow_out);

  result =
    IVideoWindow_out->put_Owner (reinterpret_cast<OAHWND> (windowHandle_inout));
  // *NOTE*: "...For the Filter Graph Manager's implementation, if the graph
  //         does not contain a video renderer filter, all methods return
  //         E_NOINTERFACE..."
  if (unlikely (FAILED (result))) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::put_Owner(0x%@): \"%s\", continuing\n"),
                inherited::mod_->name (),
                windowHandle_inout,
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

  result = IVideoWindow_out->put_WindowStyle (WS_CHILD | WS_CLIPSIBLINGS);
  if (unlikely (FAILED (result))) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::put_WindowStyle(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  result =
    IVideoWindow_out->SetWindowPosition (windowArea_inout.left,
                                          windowArea_inout.top,
                                          (windowArea_inout.right -
                                          windowArea_inout.left),
                                          (windowArea_inout.bottom -
                                          windowArea_inout.top));
  if (unlikely (FAILED (result))) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
                inherited::mod_->name (),
                windowArea_inout.left, windowArea_inout.top,
                (windowArea_inout.right - windowArea_inout.left),
                (windowArea_inout.bottom - windowArea_inout.top),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  // redirect mouse and keyboard events to the main gtk window
  result =
    IVideoWindow_out->put_MessageDrain (reinterpret_cast<OAHWND> (windowHandle_inout));
  if (unlikely (FAILED (result))) // E_NOINTERFACE      : 0x80004002
                                  // VFW_E_NOT_CONNECTED: 0x80040209
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::put_MessageDrain(0x%@): \"%s\", continuing\n"),
                inherited::mod_->name (),
                windowHandle_inout,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  result = IVideoWindow_out->put_Visible (OATRUE);
  if (unlikely (FAILED (result))) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::put_Visible(OATRUE): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  result =
    IVideoWindow_out->put_FullScreenMode (fullScreen_in ? OATRUE : OAFALSE);
  if (unlikely (FAILED (result))) // E_NOINTERFACE: 0x80004002
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IVideoWindow::put_FullScreenMode(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  if (!fullScreen_in)
  {
    result = IVideoWindow_out->SetWindowForeground (OATRUE);
    if (unlikely (FAILED (result))) // E_NOINTERFACE: 0x80004002
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IVideoWindow::SetWindowForeground(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF

  // *TODO*: forward WM_MOVE messages to the video window via NotifyOwnerMessage
  //         (see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd407298(v=vs.85).aspx)


  return true;

error:
  if (ibase_filter_p)
    ibase_filter_p->Release ();
  if (imedia_event_ex_p)
    imedia_event_ex_p->Release ();
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release (); IVideoWindow_out = NULL;
  } // end IF
  if (IMFVideoDisplayControl_out)
  {
    IMFVideoDisplayControl_out->Release (); IMFVideoDisplayControl_out = NULL;
  } // end IF

  return false;
}
