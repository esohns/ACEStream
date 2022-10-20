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
 , hasMessagePump_ (false)
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
  ACE_ASSERT (context_);
  ACE_ASSERT (window_);

  if (unlikely (StretchDIBits (context_,
                               0, 0, resolution_.cx, resolution_.cy,
                               0, 0, resolution_.cx, resolution_.cy,
                               message_inout->rd_ptr (),
                               &header_,
                               DIB_RGB_COLORS,
                               SRCCOPY) == 0))
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
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);

      if (!window_handle_p)
      {
        WNDCLASSEX window_class_ex_s;
        window_class_ex_s.cbSize = sizeof (WNDCLASSEX);
        window_class_ex_s.style = CS_HREDRAW | CS_VREDRAW;
        window_class_ex_s.lpfnWndProc = DefWindowProc;
        window_class_ex_s.cbClsExtra = 0;
        window_class_ex_s.cbWndExtra = 0;
        window_class_ex_s.hInstance = (HINSTANCE)GetModuleHandle (NULL);
        window_class_ex_s.hIcon = NULL;
        window_class_ex_s.hCursor = LoadCursor (NULL, IDC_ARROW);
        window_class_ex_s.hbrBackground = (HBRUSH)COLOR_WINDOW;
        window_class_ex_s.lpszMenuName = NULL;
        ACE_TCHAR szClassName[256] = ACE_TEXT ("SampleWindowClass");
#if defined (UNICODE)
        window_class_ex_s.lpszClassName = ACE_TEXT_ALWAYS_WCHAR (szClassName);
#else
        window_class_ex_s.lpszClassName = szClassName;
#endif // UNICODE
        window_class_ex_s.hIconSm = NULL;
        ATOM atom = RegisterClassEx (&window_class_ex_s);
        if (unlikely (atom == 0))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to RegisterClassEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF

        DWORD window_style_i = (WS_OVERLAPPED     |
                                WS_CAPTION        |
                                (WS_CLIPSIBLINGS  |
                                 WS_CLIPCHILDREN) |
                                WS_SYSMENU        |
                                WS_VISIBLE        |
                                WS_MINIMIZEBOX    |
                                WS_MAXIMIZEBOX    |
                                WS_THICKFRAME); // --> resizeable
        DWORD window_style_ex_i = (WS_EX_APPWINDOW |
                                   WS_EX_WINDOWEDGE);
        window_handle_p =
          CreateWindowEx (window_style_ex_i,                       // dwExStyle
#if defined (UNICODE)
                          ACE_TEXT_ALWAYS_WCHAR (szClassName),                   // lpClassName
                          ACE_TEXT_ALWAYS_WCHAR (inherited::mod_->name ()), // lpWindowName
#else
                          ACE_TEXT_ALWAYS_CHAR (szClassName),               // lpClassName
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

      header_.bmiHeader.biBitCount =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_s);
      header_.bmiHeader.biWidth = resolution_.cx;
      // *IMPORTANT NOTE*: "...StretchDIBits creates a top-down image if the
      //                   sign of the biHeight member of the BITMAPINFOHEADER
      //                   structure for the DIB is negative. ..."
      header_.bmiHeader.biHeight = -resolution_.cy;
      header_.bmiHeader.biPlanes = 1;
      header_.bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
      header_.bmiHeader.biSizeImage = resolution_.cx * resolution_.cy * (header_.bmiHeader.biBitCount / 8);
      header_.bmiHeader.biCompression = BI_RGB;

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      // start message pump
      if (hasMessagePump_)
      {
        inherited::threadCount_ = 1;
        inherited::start (NULL);
        inherited::threadCount_ = 0;
      } // end IF

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
                            STREAM_MEDIATYPE_VIDEO,
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
  hasMessagePump_ = (window_ != ACE_INVALID_HANDLE);

  return inherited::initialize (configuration_in,
                                allocator_in);
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
int
Stream_Vis_Target_GDI_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        ConfigurationType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType,
                        SessionDataType,
                        SessionDataContainerType,
                        MediaType>::svc ()
{
  COMMON_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::svc"));

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0A00) // _WIN32_WINNT_WIN10
  Common_Error_Tools::setThreadName (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
                                     NULL);
#else
  Common_Error_Tools::setThreadName (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
                                     0);
#endif // _WIN32_WINNT_WIN10
#endif // ACE_WIN32 || ACE_WIN64
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: spawned thread (id: %t, group id: %d)\n"),
              ACE_TEXT (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
              STREAM_MODULE_TASK_GROUP_ID));

  // sanity check(s)
  ACE_ASSERT (window_);

  struct tagMSG msg;
  while (GetMessage (&msg, window_, 0, 0) != -1)
  {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
  } // end WHILE

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: spawned thread (id: %t, group id: %d) leaving\n"),
              ACE_TEXT (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
              STREAM_MODULE_TASK_GROUP_ID));

  return 0;
}
