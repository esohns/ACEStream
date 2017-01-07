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
          typename PinConfigurationType>
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType>::Stream_Vis_Target_DirectShow_T ()
 : inherited ()
 , inherited2 ()
 , IVideoWindow_ (NULL)
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
          typename PinConfigurationType>
Stream_Vis_Target_DirectShow_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               SessionDataType,
                               FilterConfigurationType,
                               PinConfigurationType>::~Stream_Vis_Target_DirectShow_T ()
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
          typename PinConfigurationType>
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
                               PinConfigurationType>::handleDataMessage (DataMessageType*& message_inout,
                                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::handleDataMessage"));

  ACE_UNUSED_ARG (message_inout);
  ACE_UNUSED_ARG (passMessageDownstream_out);
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
          typename PinConfigurationType>
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
                               PinConfigurationType>::handleSessionMessage (SessionMessageType*& message_inout,
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
    case STREAM_SESSION_MESSAGE_BEGIN:
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
      ACE_ASSERT (!IVideoWindow_);

      IVideoWindow* video_window_p = IVideoWindow_;
      if (video_window_p) goto continue_;

      if (session_data_r.windowController)
      {
        ULONG reference_count = session_data_r.windowController->AddRef ();
        IVideoWindow_ = session_data_r.windowController;
      } // end IF
      else
      {
        // sanity check(s)
        ACE_ASSERT (inherited::configuration_->graphBuilder);

        struct tagRECT target_area;
        ACE_OS::memset (&target_area, 0, sizeof (struct tagRECT));
        target_area.bottom =
          (inherited::configuration_->area.y +
           inherited::configuration_->area.height);
        target_area.left = inherited::configuration_->area.x;
        target_area.right =
          (inherited::configuration_->area.x +
           inherited::configuration_->area.width);
        target_area.top = inherited::configuration_->area.y;
        if (!initialize_DirectShow (inherited::configuration_->window,
                                    target_area,
                                    inherited::configuration_->graphBuilder,
                                    IVideoWindow_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_DirectShow(), aborting\n")));
          goto error;
        } // end IF
      } // end IF
continue_:
      ACE_ASSERT (IVideoWindow_);

      goto continue_2;

error:
      session_data_r.aborted = true;

continue_2:
      if (COM_initialized)
        CoUninitialize ();

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
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      // *IMPORTANT NOTE*: "Reset the owner to NULL before releasing the Filter
      //                   Graph Manager. Otherwise, messages will continue to
      //                   be sent to this window and errors will likely occur
      //                   when the application is terminated. ..."
      if (IVideoWindow_)
      {
        result_2 = IVideoWindow_->put_Visible (OAFALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IVideoWindow::put_Visible(OAFALSE) \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        result_2 = IVideoWindow_->put_AutoShow (OAFALSE);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IVideoWindow::put_AutoShow(OAFALSE) \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        result_2 = IVideoWindow_->put_Owner (NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IVideoWindow::put_Owner(NULL) \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        result_2 = IVideoWindow_->put_MessageDrain (NULL);
        if (FAILED (result_2))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IVideoWindow::put_MessageDrain(NULL) \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

        IVideoWindow_->Release ();
        IVideoWindow_ = NULL;
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
          typename FilterConfigurationType,
          typename PinConfigurationType>
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
                               PinConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize"));

  HRESULT result = E_FAIL;

  if (inherited::isInitialized_)
  {
    if (IVideoWindow_)
    {
      result = IVideoWindow_->put_Owner (NULL);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IVideoWindow::put_Owner() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      result = IVideoWindow_->put_MessageDrain (NULL);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IVideoWindow::put_MessageDrain() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      IVideoWindow_->Release ();
      IVideoWindow_ = NULL;
    } // end IF

    inherited::isInitialized_ = false;
  } // end IF

  if (configuration_in.windowController)
  {
    ULONG reference_count = configuration_in.windowController->AddRef ();
    IVideoWindow_ = configuration_in.windowController;
  } // end IF

  // *TODO*: remove type inference
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Misc_DirectShow_Target_T::initialize(), aborting\n")));
    return false;
  } // end IF

  return inherited2::initialize (*configuration_in.filterConfiguration);
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
          typename PinConfigurationType>
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
                               PinConfigurationType>::initialize_DirectShow (const HWND windowHandle_in,
                                                                             const struct tagRECT& windowArea_in,
                                                                             IGraphBuilder* IGraphBuilder_in,
                                                                             IVideoWindow*& IVideoWindow_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_DirectShow_T::initialize_DirectShow"));

  HRESULT result = E_FAIL;

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

  HWND window_handle = windowHandle_in;
  if (!window_handle)
  {
    window_handle = CreateWindow (ACE_TEXT ("EDIT"),
                                  0,
                                  WS_OVERLAPPEDWINDOW,
                                  //WS_CHILD | WS_VISIBLE | ES_READONLY | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  COMMON_UI_WINDOW_DEFAULT_WIDTH,
                                  COMMON_UI_WINDOW_DEFAULT_HEIGHT,
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
  } // end IF
  ACE_ASSERT (window_handle);

  result = IVideoWindow_out->put_Owner ((OAHWND)window_handle);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_Owner(0x%@): \"%s\", continuing\n"),
                window_handle,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  // redirect mouse and keyboard events to the main gtk window
  result = IVideoWindow_out->put_MessageDrain ((OAHWND)window_handle);
  if (FAILED (result)) // VFW_E_NOT_CONNECTED: 0x80040209
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_MessageDrain(0x%@): \"%s\", continuing\n"),
                window_handle,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  result = IVideoWindow_out->put_WindowStyle (WS_CHILD | WS_CLIPCHILDREN);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::put_WindowStyle(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  result =
    IVideoWindow_out->SetWindowPosition (windowArea_in.left,
                                         windowArea_in.top,
                                         windowArea_in.right,
                                         windowArea_in.bottom);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  return true;

error:
  if (IVideoWindow_out)
  {
    IVideoWindow_out->Release ();
    IVideoWindow_out = NULL;
  } // end IF

  return false;
}
