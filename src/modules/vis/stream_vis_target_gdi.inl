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
 , context_ (NULL)
 , header_ ()
 //, CBData_ ()
 , resolution_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::Stream_Vis_Target_GDI_T"));

  ACE_OS::memset (&header_, 0, sizeof (struct tagBITMAPINFO));
  //ACE_OS::memset (&CBData_, 0, sizeof (struct libacestream_gdi_window_proc_cb_data));
  ACE_OS::memset (&resolution_2, 0, sizeof (struct tagRECT));

  //CBData_.dc = &context_;
  //CBData_.lock = &(inherited::lock_);
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

  if (unlikely (context_ && inherited::window_))
    if (unlikely (!ReleaseDC (inherited::window_, context_)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
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
                        MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (context_);

  //GetWindowRect (inherited::window_, &resolution_2);

  if (unlikely (StretchDIBits (context_,
                               0, 0, resolution_2.right - resolution_2.left, resolution_2.bottom - resolution_2.top,
                               0, 0, inherited::resolution_.cx, inherited::resolution_.cy,
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

      // sanity check(s)
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);
      inherited::resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);

      header_.bmiHeader.biBitCount =
        Stream_MediaFramework_DirectShow_Tools::toFrameBits (media_type_s);
      header_.bmiHeader.biCompression = BI_RGB;
      header_.bmiHeader.biWidth = resolution_.cx;
      // *IMPORTANT NOTE*: "...StretchDIBits creates a top-down image if the
      //                   sign of the biHeight member of the BITMAPINFOHEADER
      //                   structure for the DIB is negative. ..."
      header_.bmiHeader.biHeight = resolution_.cy;
      header_.bmiHeader.biPlanes = 1;
      header_.bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
      header_.bmiHeader.biSizeImage = DIBSIZE (header_.bmiHeader);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      // start message pump ?
      if (!inherited::window_)
      { // need a window/message pump
        inherited::threadCount_ = 1;
        inherited::start (NULL);
        inherited::threadCount_ = 0;

        while (inherited::thr_count_ && !inherited::window_); // *TODO*: never do this
      } // end IF
      else
      { ACE_ASSERT (!context_);
        context_ = GetDC (inherited::window_);
        ACE_ASSERT (context_);
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: window handle: 0x%@ (drawing context: 0x%@\n"),
                    inherited::mod_->name (),
                    inherited::window_,
                    context_));
      } // end ELSE
      ACE_ASSERT (inherited::window_);
      GetWindowRect (inherited::window_, &resolution_2);

      break;

//error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (context_ && inherited::window_)
      {
        if (!ReleaseDC (inherited::window_, context_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        context_ = NULL;
      } // end IF

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (context_ && inherited::window_)
      {
        if (!ReleaseDC (inherited::window_, context_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        context_ = NULL;
      } // end IF

      if (inherited::window_)
      {
        inherited::notify_ = false;
        if (unlikely (!CloseWindow (inherited::window_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        inherited::window_ = NULL;
      } // end IF

      if (inherited::thr_count_)
        inherited::wait ();

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
    if (context_ && inherited::window_)
    {
      if (!ReleaseDC (window_, context_))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ReleaseDC(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      context_ = NULL;
    } // end IF
  } // end IF

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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_GDI_T::svc"));

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

  inherited::window_ = inherited::createWindow ();
  if (unlikely (!inherited::window_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to create window, aborting\n"),
                inherited::mod_->name ()));
    return -1;
  } // end IF
  //SetWindowLongPtr (window_, GWLP_USERDATA, (LONG_PTR)&CBData_);

  ACE_ASSERT (!context_);
  context_ = GetDC (inherited::window_);
  ACE_ASSERT (context_);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window handle: 0x%@; drawing context: 0x%@\n"),
              inherited::mod_->name (),
              inherited::window_,
              context_));

  inherited::notify_ = true;

  struct tagMSG message_s;
  while (GetMessage (&message_s, window_, 0, 0) != -1)
  {
    TranslateMessage (&message_s);
    DispatchMessage (&message_s);
  } // end WHILE

  if (unlikely (inherited::notify_))
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: spawned thread (id: %t, group id: %d) leaving\n"),
              ACE_TEXT (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
              STREAM_MODULE_TASK_GROUP_ID));

  return 0;
}
