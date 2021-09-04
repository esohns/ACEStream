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
#include "ace/OS.h"

#include "common_tools.h"
#include "common_file_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_file_defines.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#include "stream_lib_tools.h"

#include "stream_vis_common.h"
#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataType,
                        SessionDataContainerType,
                        MediaType>::Stream_Vis_Target_GDI_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , closeWindow_ (false)
 , context_ (NULL)
 , header_ ()
 , resolution_ ()
 , window_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::Stream_Vis_Target_GDI_T"));

  ACE_OS::memset (&header_, 0, sizeof (struct tagBITMAPINFO));
  ACE_OS::memset (&resolution_, 0, sizeof (Common_Image_Resolution_t));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataType,
                        SessionDataContainerType,
                        MediaType>::~Stream_Vis_Target_GDI_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::~Stream_Vis_Target_GDI_T"));

  if (context_)
  { ACE_ASSERT (window_);
    if (unlikely (!::ReleaseDC (window_, context_)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
  } // end IF

  if (closeWindow_)
  { ACE_ASSERT (window_);
    if (unlikely (!::CloseWindow (window_)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
  } // end IF
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataType,
                        SessionDataContainerType,
                        MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (unlikely (!window_))
    return; // --> nothing to do
  ACE_ASSERT (context_);

  if (unlikely (!::StretchDIBits (context_,
                                  0, 0, resolution_.cx, resolution_.cy,
                                  0, 0, resolution_.cx, resolution_.cy,
                                  message_inout->rd_ptr (),
                                  &header_,
                                  DIB_RGB_COLORS,
                                  SRCCOPY)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to StretchDIBits(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataType,
                        SessionDataContainerType,
                        MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      struct _AMMediaType media_type_s;
      ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
      HWND window_handle_p = window_;

      // sanity check(s)
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.back (),
                                media_type_s);
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);

      if (!window_handle_p)
      {
        //ACE_ASSERT (!direct3DConfiguration_->focusWindow);
        DWORD window_style_i = (WS_OVERLAPPED     |
                                WS_CAPTION        |
                                (WS_CLIPSIBLINGS  |
                                  WS_CLIPCHILDREN) |
                                WS_SYSMENU        |
                                //WS_THICKFRAME     |
                                WS_MINIMIZEBOX    |
                                WS_VISIBLE/*
                                WS_MAXIMIZEBOX*/);
        DWORD window_style_ex_i = (WS_EX_APPWINDOW |
                                    WS_EX_WINDOWEDGE);
        window_handle_p =
          CreateWindowEx (window_style_ex_i,                       // dwExStyle
#if defined (UNICODE)
                          ACE_TEXT_ALWAYS_WCHAR ("EDIT"),                   // lpClassName
                          ACE_TEXT_ALWAYS_WCHAR (inherited::mod_->name ()), // lpWindowName
#else
                          ACE_TEXT_ALWAYS_CHAR ("EDIT"),                    // lpClassName
                          ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()),  // lpWindowName
#endif // UNICODE
                          window_style_i,                          // dwStyle
                          CW_USEDEFAULT,                           // x
                          CW_USEDEFAULT,                           // y
                          resolution_.cx,                          // nWidth
                          resolution_.cy,                          // nHeight
                          NULL,                                    // hWndParent
                          NULL,                                    // hMenu
                          GetModuleHandle (NULL),                  // hInstance
                          NULL);                                   // lpParam
        if (unlikely (!window_handle_p))
        { // ERROR_INVALID_PARAMETER: 87
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        window_ = window_handle_p;
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_handle_p);

      ACE_ASSERT (!context_);
      context_ = ::GetDC (window_);
      ACE_ASSERT (context_);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@ (drawing context: 0x%@\n"),
                  inherited::mod_->name (),
                  window_,
                  context_));
#endif // _DEBUG

      header_.bmiHeader.biBitCount = 32;
      header_.bmiHeader.biWidth = resolution_.cx;
      header_.bmiHeader.biHeight = resolution_.cy;
      header_.bmiHeader.biPlanes = 1;
      header_.bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
      header_.bmiHeader.biSizeImage = resolution_.cx * resolution_.cy * (header_.bmiHeader.biBitCount / 8);
      header_.bmiHeader.biCompression = BI_RGB;

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (context_)
      { ACE_ASSERT (window_);
        if (!::ReleaseDC (window_, context_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        context_ = NULL;
      } // end IF

      if (closeWindow_)
      { ACE_ASSERT (window_);
        closeWindow_ = false;
        if (!::CloseWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        window_ = NULL;
      } // end IF

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (context_)
      { ACE_ASSERT (window_);
        if (!::ReleaseDC (window_, context_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        context_ = NULL;
      } // end IF

      if (closeWindow_)
      { ACE_ASSERT (window_);
        if (unlikely (!::CloseWindow (window_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
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
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataType,
                        SessionDataContainerType,
                        MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::toggle"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  ACE_ASSERT (!session_data_r.formats.empty ());

  struct _AMMediaType media_type_s;
  inherited2::getMediaType (session_data_r.formats.back (),
                            media_type_s);
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::initialize (const ConfigurationType& configuration_in,
                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (context_)
    { ACE_ASSERT (window_);
      if (!::ReleaseDC (window_, context_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      context_ = NULL;
    } // end IF

    if (closeWindow_)
    { ACE_ASSERT (window_);
      closeWindow_ = false;
      if (unlikely (!::CloseWindow (window_)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    } // end IF
    window_ = NULL;
  } // end IF

  // *TODO*: remove type inferences
  window_ = configuration_in.window;

  return inherited::initialize (configuration_in,
                                allocator_in);
}
