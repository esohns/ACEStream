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

#include "WinUser.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_error_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::Stream_Vis_Target_Win32_Base_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , notify_ (false)
 , resolution_ ()
 , window_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Win32_Base_T::Stream_Vis_Target_Win32_Base_T"));

  ACE_OS::memset (&resolution_, 0, sizeof (Common_Image_Resolution_t));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::~Stream_Vis_Target_Win32_Base_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Win32_Base_T::~Stream_Vis_Target_Win32_Base_T"));

  if (unlikely (window_))
    if (unlikely (!PostMessage (window_, WM_QUIT, 0, 0)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to PostMessage(%@): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  window_,
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));

  if (unlikely (inherited::thr_count_))
    inherited::wait ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
void
Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Win32_Base_T::toggle"));

  // sanity check(s)
  if (!window_)
    return; // nothing to do

  struct tagWINDOWPLACEMENT window_placement_s;
  ACE_OS::memset (&window_placement_s, 0, sizeof (struct tagWINDOWPLACEMENT));
  window_placement_s.length = sizeof (struct tagWINDOWPLACEMENT);
  if (!GetWindowPlacement (window_, &window_placement_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to GetWindowPlacement(0x%@): \"%s\", returning\n"),
                inherited::mod_->name (),
                window_,
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    return;
  } // end IF
  int command_i =
    (window_placement_s.showCmd == SW_MAXIMIZE) ? SW_RESTORE : SW_MAXIMIZE;
  if (!ShowWindow (window_, command_i))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ShowWindow(0x%@,%d): \"%s\", continuing\n"),
                inherited::mod_->name (),
                window_, command_i,
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
bool
Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Win32_Base_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (window_)
    {
      if (unlikely (!PostMessage (window_, WM_QUIT, 0, 0)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to PostMessage(%@): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    window_,
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    } // end IF
    if (inherited::thr_count_)
      inherited::wait ();
  } // end IF

  // *TODO*: remove type inferences
  inherited3::getWindowType (configuration_in.window, window_);

  return inherited::initialize (configuration_in,
                                allocator_in);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
int
Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::svc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Win32_Base_T::svc"));

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

  window_ = createWindow ();
  if (unlikely (!window_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to createWindow(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    return -1;
  } // end IF
  //SetWindowLongPtr (window_, GWLP_USERDATA, (LONG_PTR)&CBData_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window handle: 0x%@\n"),
              inherited::mod_->name (),
              window_));

  notify_ = true;

  struct tagMSG message_s;
  while (GetMessage (&message_s, window_, 0, 0) != -1)
  {
    TranslateMessage (&message_s);
    DispatchMessage (&message_s);
  } // end WHILE

  if (unlikely (notify_))
    notify (STREAM_SESSION_MESSAGE_ABORT);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: spawned thread (id: %t, group id: %d) leaving\n"),
              ACE_TEXT (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
              STREAM_MODULE_TASK_GROUP_ID));

  return 0;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
HWND
Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               MediaType>::createWindow ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Win32_Base_T::createWindow"));

  WNDCLASSEX window_class_ex_s;
  window_class_ex_s.cbSize = sizeof (WNDCLASSEX);
  window_class_ex_s.style = CS_HREDRAW | CS_VREDRAW;
  window_class_ex_s.lpfnWndProc = DefWindowProc;
  //window_class_ex_s.lpfnWndProc =
  //  libacestream_vis_target_win32_base_window_proc_cb;
  window_class_ex_s.cbClsExtra = 0;
  window_class_ex_s.cbWndExtra = 0;
  window_class_ex_s.hInstance = (HINSTANCE)GetModuleHandle (NULL);
  window_class_ex_s.hIcon = NULL;
  //window_class_ex_s.hCursor = LoadCursor (NULL, IDC_ARROW);
  window_class_ex_s.hCursor = NULL;
  window_class_ex_s.hbrBackground = (HBRUSH)COLOR_WINDOW;
  window_class_ex_s.lpszMenuName = NULL;
  ACE_TCHAR szClassName[256] = ACE_TEXT ("ACEStream Visualization WindowClass");
#if defined (UNICODE)
  window_class_ex_s.lpszClassName = ACE_TEXT_ALWAYS_WCHAR (szClassName);
#else
  window_class_ex_s.lpszClassName = ACE_TEXT_ALWAYS_CHAR (szClassName);
#endif // UNICODE
  window_class_ex_s.hIconSm = NULL;
  ATOM atom = RegisterClassEx (&window_class_ex_s);
  if (unlikely (atom == 0)) // most likely reason: already registered this window class
  { DWORD error_i = ::GetLastError ();
    if (unlikely (error_i != ERROR_CLASS_ALREADY_EXISTS))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to RegisterClassEx(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (error_i).c_str ())));
      return NULL;
    } // end IF
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
  HWND handle_p =
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
  if (unlikely (!handle_p))
  { // ERROR_INVALID_PARAMETER: 87
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    return NULL;
  } // end IF

  return handle_p;
}
