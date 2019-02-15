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

#include <d3d9types.h>
#include <mferror.h>
#include <mfidl.h>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_tools.h"

#include "common_image_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_file_defines.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#include "stream_lib_directdraw_tools.h"
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::Stream_Vis_Target_Direct3D_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , clientWindow_ (NULL)
 , closeWindow_ (false)
 , defaultStride_ (0)
 , destinationRectangle_ ()
 , direct3DConfiguration_ (NULL)
 , mediaFramework_ (STREAM_LIB_DEFAULT_MEDIAFRAMEWORK)
 , releaseDeviceHandle_ (false)
 , resetMode_ (false)
 , snapShotNextFrame_ (false)
 , transformation_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::Stream_Vis_Target_Direct3D_T"));

  if (unlikely (!SetRectEmpty (&destinationRectangle_)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to SetRectEmpty(): \"%s\", continuing\n"),
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::~Stream_Vis_Target_Direct3D_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::~Stream_Vis_Target_Direct3D_T"));

  if (resetMode_)
  { ACE_ASSERT (direct3DConfiguration_);
    if (!direct3DConfiguration_->presentationParameters.Windowed)
    {
      direct3DConfiguration_->presentationParameters.Windowed = TRUE;
      toggle ();
    } // end IF
  } // end IF

  if (closeWindow_)
  { ACE_ASSERT (direct3DConfiguration_);
    ACE_ASSERT (direct3DConfiguration_->presentationParameters.hDeviceWindow);
    if (unlikely (!::CloseWindow (direct3DConfiguration_->presentationParameters.hDeviceWindow)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
  } // end IF

  if (releaseDeviceHandle_)
  { ACE_ASSERT (direct3DConfiguration_);
    ACE_ASSERT (direct3DConfiguration_->handle);
    direct3DConfiguration_->handle->Release (); direct3DConfiguration_->handle = NULL;
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::handleControlMessage (ControlMessageType& controlMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::handleControlMessage"));

  switch (controlMessage_in.type ())
  {
    case STREAM_CONTROL_MESSAGE_STEP_2:
      snapShotNextFrame_ = true; break;
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  bool reset_device_b = false;
  bool destroy_device_b = false;
  bool unlock_rect_b = false;
  IDirect3DSurface9* d3d_surface_p = NULL, *d3d_surface_2 = NULL;
  struct _D3DLOCKED_RECT d3d_locked_rectangle_s;
  BYTE* scanline0_p = NULL;
  std::string filename_string;

  // sanity check(s)
  //ACE_ASSERT (message_inout->length () == inherited::configuration_->format->lSampleSize);
  ACE_ASSERT (direct3DConfiguration_);
  if (unlikely (!direct3DConfiguration_->presentationParameters.Windowed &&
                !direct3DConfiguration_->presentationParameters.hDeviceWindow &&
                !direct3DConfiguration_->focusWindow))
    return; // --> nothing to do

  // *TODO*: remove ASAP
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, direct3DConfiguration_->lock);

  // *IMPORTANT NOTE*: "...For these reasons, Direct3D is designed so that the
  //                   methods IDirect3DDevice9::Reset,
  //                   IDirect3D9::CreateDevice,
  //                   IDirect3DDevice9::TestCooperativeLevel [!], or the final
  //                   Release of IDirect3DDevice9 can only be called from the
  //                   same thread that handles window messages. ..."
  //ACE_ASSERT (ACE_OS::thr_self () == direct3DConfiguration_.threadId);
  // *TODO*: remove this check
//  checkCooperativeLevel (handle_,
//                         reset_device_b,
//                         destroy_device_b);
//  ACE_ASSERT (!(reset_device_b && destroy_device_b));
//  if (unlikely (reset_device_b))
//  {
//    // sanity check(s)
//    ACE_ASSERT (inherited::sessionData_);
//    const typename SessionDataContainerType::DATA_T& session_data_r =
//      inherited::sessionData_->getR ();
//    ACE_ASSERT (session_data_r.inputFormat);
//    ACE_ASSERT (direct3DConfiguration_);
//
//    result = resetDevice (*direct3DConfiguration_,
//                          *session_data_r.inputFormat,
//                          presentationParameters_,
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//                          fullscreenDisplayMode_,
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
//                          defaultStride_,
//                          handle_,
//                          //swapChain_,
//                          destinationRectangle_);
//    if (unlikely (FAILED (result)))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::resetDevice(): \"%s\", aborting\n"),
//                  inherited::mod_->name (),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//  else if (unlikely (destroy_device_b))
//  { // *TODO*
//    ACE_ASSERT (false);
//    ACE_NOTSUP;
//    ACE_NOTREACHED (return;)
//  } // end ELSE IF

  if (defaultStride_ < 0)
  {
    // Bottom-up orientation. Return a pointer to the start of the last row
    // *in memory*, which is the top row of the image
    scanline0_p =
      (reinterpret_cast<BYTE*> (message_inout->rd_ptr ()) +
        (::abs (defaultStride_) * (direct3DConfiguration_->presentationParameters.BackBufferHeight - 1)));
  } // end IF
  else
  {
    // Top-down orientation. Return a pointer to the start of the buffer
    scanline0_p = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());
  } // end ELSE

  // convert the frame to the display format by applying the appropriate
  // transformation (also copies it to a Direct3D surface)
  // sanity check(s)
  ACE_ASSERT (direct3DConfiguration_->handle);
  ACE_ASSERT (direct3DConfiguration_->presentationParameters.BackBufferCount >= 2);
  //ACE_ASSERT (swapChain_);
  //result = swapChain_->GetBackBuffer (0,
  //                                    D3DBACKBUFFER_TYPE_MONO,
  //                                    &d3d_surface_p);
  result =
    direct3DConfiguration_->handle->GetBackBuffer (0,                       // swap chain
                                                   1,                       // back buffer
                                                   D3DBACKBUFFER_TYPE_MONO, // type
                                                   &d3d_surface_p);         // return value: handle
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("%s: failed to IDirect3DSwapChain9::GetBackBuffer(): \"%s\", returning\n"),
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (d3d_surface_p);

  ACE_OS::memset (&d3d_locked_rectangle_s, 0, sizeof (struct _D3DLOCKED_RECT));
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle_s,
                                    NULL,
                                    D3DLOCK_NOSYSLOCK);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSurface9::LockRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  unlock_rect_b = true;

  // sanity check(s)
  ACE_ASSERT (transformation_);
  ACE_ASSERT (::abs (d3d_locked_rectangle_s.Pitch) >= ::abs (defaultStride_));
  try {
    transformation_ ((BYTE*)d3d_locked_rectangle_s.pBits,
                     d3d_locked_rectangle_s.Pitch,
                     scanline0_p,
                     defaultStride_,
                     direct3DConfiguration_->presentationParameters.BackBufferWidth,
                     direct3DConfiguration_->presentationParameters.BackBufferHeight);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in transformation callback, continuing\n"),
                inherited::mod_->name ()));
    goto error;
  }

  result = d3d_surface_p->UnlockRect ();
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  unlock_rect_b = false;

  // save the frame ?
  if (likely (!snapShotNextFrame_))
    goto continue_;
  ACE_ASSERT (inherited::configuration_);
  filename_string =
    ACE_TEXT_ALWAYS_CHAR (ACE::dirname (inherited::configuration_->targetFileName.c_str (),
                                        ACE_DIRECTORY_SEPARATOR_CHAR));
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DEFAULT_SCREENSHOT_FILENAME_STRING);
  enum _D3DXIMAGE_FILEFORMAT format_e =
    STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_SCREENSHOT_DEFAULT_FORMAT;
  filename_string +=
    Stream_MediaFramework_DirectDraw_Tools::toFilenameExtension (format_e);
  if (unlikely (!Common_Image_Tools::save (filename_string,
                                           format_e,
                                           d3d_surface_p)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_Image_Tools::save(), continuing\n"),
                inherited::mod_->name ()));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: saved screenshot \"%s\"\n"),
                inherited::mod_->name (),
                ACE_TEXT (filename_string.c_str ())));
#endif // _DEBUG
  snapShotNextFrame_ = false;

continue_:
  // stretch the frame to the output window dimensions
  result =
    direct3DConfiguration_->handle->GetBackBuffer (0,                       // swap chain
                                                   0,                       // back buffer
                                                   D3DBACKBUFFER_TYPE_MONO, // type
                                                   &d3d_surface_2);         // return value: handle
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (d3d_surface_2);
  //// color-fill the back buffer ?
  //result = handle_->ColorFill (d3d_surface_2,
  //                                         NULL,
  //                                         D3DCOLOR_XRGB (0, 0, 0x80));
  //if (unlikely (FAILED (result)))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::ColorFill(): \"%s\", returning\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  result =
    direct3DConfiguration_->handle->StretchRect (d3d_surface_p,
                                                 NULL,
                                                 d3d_surface_2,
                                                 //&destinationRectangle_,
                                                 NULL, // use window/fullscreen mode dimensions
                                                 D3DTEXF_NONE);
  if (unlikely (FAILED (result))) // D3DERR_INVALIDCALL: 0x8876086c
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::StretchRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  d3d_surface_p->Release (); d3d_surface_p = NULL;
  d3d_surface_2->Release (); d3d_surface_2 = NULL;

  // present the frame
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    direct3DConfiguration_->handle->PresentEx (NULL, // pSourceRect
                                               NULL, // pDestRect
                                               NULL, // hDestWindowOverride
                                               NULL, // pDirtyRegion
                                               0);   // dwFlags
#else
  result =
    direct3DConfiguration_->handle->Present (NULL,  // pSourceRect
                                             NULL,  // pDestRect
                                             NULL,  // hDestWindowOverride
                                             NULL); // pDirtyRegion
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (unlikely (FAILED (result)))
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::PresentEx(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9::Present(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    goto error;
  } // end IF

  return;

error:
  if (unlock_rect_b)
  { ACE_ASSERT (d3d_surface_p);
    result = d3d_surface_p->UnlockRect ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
  if (d3d_surface_p)
    d3d_surface_p->Release ();
  if (d3d_surface_2)
    d3d_surface_2->Release ();
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::handleSessionMessage"));

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
      ACE_ASSERT (direct3DConfiguration_);
      if (unlikely (!direct3DConfiguration_->presentationParameters.Windowed &&
                    !direct3DConfiguration_->presentationParameters.hDeviceWindow &&
                    !direct3DConfiguration_->focusWindow))
        return; // --> nothing to do
      struct _AMMediaType media_type_s;
      Common_Image_Resolution_t resolution_s;
      HWND window_handle_p = NULL;

      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.front (),
                                media_type_s);
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      ACE_ASSERT ((resolution_s.cx == direct3DConfiguration_->presentationParameters.BackBufferWidth) && (resolution_s.cy == direct3DConfiguration_->presentationParameters.BackBufferHeight));
      window_handle_p =
        (direct3DConfiguration_->presentationParameters.Windowed ? direct3DConfiguration_->presentationParameters.hDeviceWindow
                                                                 : direct3DConfiguration_->focusWindow);
      if (window_handle_p)
      {
        ACE_ASSERT (IsWindow (window_handle_p));
      } // end IF

      if (direct3DConfiguration_->presentationParameters.Windowed)
      {
        resetMode_ = true;
        if (!window_handle_p)
        { ACE_ASSERT (!direct3DConfiguration_->presentationParameters.hDeviceWindow);
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
          direct3DConfiguration_->presentationParameters.hDeviceWindow =
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
                            direct3DConfiguration_->presentationParameters.BackBufferWidth,  // nWidth
                            direct3DConfiguration_->presentationParameters.BackBufferHeight, // nHeight
                            NULL,                                    // hWndParent
                            NULL,                                    // hMenu
                            GetModuleHandle (NULL),                  // hInstance
                            NULL);                                   // lpParam
          if (unlikely (!direct3DConfiguration_->presentationParameters.hDeviceWindow))
          { // ERROR_INVALID_PARAMETER: 87
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                        inherited::mod_->name (),
                        ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
            goto error;
          } // end IF
          window_handle_p =
            direct3DConfiguration_->presentationParameters.hDeviceWindow;
          closeWindow_ = true;
        } // end IF
      } // end IF
      ACE_ASSERT (window_handle_p);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@\n"),
                  inherited::mod_->name (),
                  window_handle_p));
#endif // _DEBUG
      if (direct3DConfiguration_->presentationParameters.Windowed)
        clientWindow_ = window_handle_p;

      defaultStride_ = 0;
      if (unlikely (!SetRectEmpty (&destinationRectangle_)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SetRectEmpty(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      IDirect3DDevice9Ex* device_handle_p =
#else
      IDirect3DDevice9* device_handle_p =
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
        session_data_r.direct3DDevice; // prefer session data over configuration
      if (device_handle_p)
      {
        if (direct3DConfiguration_->handle)
        {
          direct3DConfiguration_->handle->Release (); direct3DConfiguration_->handle = NULL;
        } // end IF
        ACE_ASSERT (!direct3DConfiguration_->handle);
        session_data_r.direct3DDevice->AddRef ();
        direct3DConfiguration_->handle = session_data_r.direct3DDevice;
        releaseDeviceHandle_ = true;
      } // end IF
      else
      {
        if (direct3DConfiguration_->handle)
        {
          direct3DConfiguration_->handle->AddRef ();
          session_data_r.direct3DDevice = direct3DConfiguration_->handle;
          device_handle_p = direct3DConfiguration_->handle;
        } // end IF
      } // end ELSE
      if (device_handle_p)
      {
        result_2 =
          initialize_Direct3DDevice ((direct3DConfiguration_->presentationParameters.Windowed ? window_handle_p
                                                                                              : NULL),
                                     media_type_s,
                                     direct3DConfiguration_->handle,
                                     direct3DConfiguration_->presentationParameters,
                                     defaultStride_,
                                     destinationRectangle_);
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error;
        } // end IF
      } // end IF
      else
      {
        // sanity check(s)
        ACE_ASSERT (!direct3DConfiguration_->handle);
        if (unlikely (!initialize_Direct3D (*direct3DConfiguration_,
                                            media_type_s,
                                            direct3DConfiguration_->handle,
                                            direct3DConfiguration_->presentationParameters,
                                            defaultStride_,
                                            destinationRectangle_)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to initialize_Direct3D(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_ASSERT (direct3DConfiguration_->handle);
        releaseDeviceHandle_ = true;
        device_handle_p = direct3DConfiguration_->handle;
      } // end ELSE
      ACE_ASSERT (device_handle_p);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      break;

error:
      if (direct3DConfiguration_->handle)
      {
        direct3DConfiguration_->handle->Release (); direct3DConfiguration_->handle = NULL;
      } // end IF
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      { ACE_ASSERT (direct3DConfiguration_->presentationParameters.hDeviceWindow);
        closeWindow_ = false;
        if (!::CloseWindow (direct3DConfiguration_->presentationParameters.hDeviceWindow))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        direct3DConfiguration_->presentationParameters.hDeviceWindow = NULL;
      } // end IF

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
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      if (resetMode_)
      { ACE_ASSERT (direct3DConfiguration_);
        if (!direct3DConfiguration_->presentationParameters.Windowed)
        {
          direct3DConfiguration_->presentationParameters.Windowed = TRUE;
          toggle ();
        } // end IF
        resetMode_ = false;
      } // end IF

      if (releaseDeviceHandle_)
      { ACE_ASSERT (direct3DConfiguration_);
        ACE_ASSERT (direct3DConfiguration_->handle);
        direct3DConfiguration_->handle->Release (); direct3DConfiguration_->handle = NULL;
        releaseDeviceHandle_ = false;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      { ACE_ASSERT (direct3DConfiguration_);
        ACE_ASSERT (direct3DConfiguration_->presentationParameters.hDeviceWindow);
        if (unlikely (!::CloseWindow (direct3DConfiguration_->presentationParameters.hDeviceWindow)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        direct3DConfiguration_->presentationParameters.hDeviceWindow = NULL;
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::toggle"));

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  SessionDataType& session_data_r =
    const_cast<SessionDataType&> (inherited::sessionData_->getR ());
  ACE_ASSERT (!session_data_r.formats.empty ());
  ACE_ASSERT (direct3DConfiguration_);
  ACE_ASSERT (direct3DConfiguration_->handle);

  struct _AMMediaType media_type_s;
  inherited2::getMediaType (session_data_r.formats.front (),
                            media_type_s);
  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);

  // *IMPORTANT NOTE*: the configuration has to be updated at this stage !

  // update configuration
  if (direct3DConfiguration_->presentationParameters.Windowed)
  { // --> switch to windowed mode
    ACE_ASSERT (clientWindow_);
    ACE_ASSERT (direct3DConfiguration_->focusWindow == clientWindow_);

    // *NOTE*: 0 --> use window format
    direct3DConfiguration_->presentationParameters.BackBufferWidth =
      resolution_s.cx;
      //0;
    direct3DConfiguration_->presentationParameters.BackBufferHeight =
      resolution_s.cy;
      //0;
    direct3DConfiguration_->presentationParameters.BackBufferFormat =
      D3DFMT_UNKNOWN;
    direct3DConfiguration_->presentationParameters.hDeviceWindow =
      clientWindow_;
    //direct3DConfiguration_->presentationParameters.Windowed = TRUE;
    direct3DConfiguration_->presentationParameters.FullScreen_RefreshRateInHz =
      0;
    //direct3DConfiguration_->presentationParameters.PresentationInterval = ;
  } // end IF
  else
  { // --> switch to fullscreen mode
    ACE_ASSERT (!direct3DConfiguration_->presentationParameters.hDeviceWindow);
    ACE_ASSERT (direct3DConfiguration_->focusWindow);
    struct _D3DDISPLAYMODE display_mode_s =
      Stream_MediaFramework_DirectDraw_Tools::getDisplayMode (direct3DConfiguration_->adapter,
                                                              STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT,
                                                              resolution_s);
    if ((display_mode_s.Format != STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT) ||
        ((display_mode_s.Width  != resolution_s.cx) ||
         (display_mode_s.Height != resolution_s.cy)))
    {
      // *TODO*: select closest possible format
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: adapter (was: %d) does not support fullscreen mode (was: %ux%u), returning\n"),
                  inherited::mod_->name (),
                  direct3DConfiguration_->adapter,
                  resolution_s.cx, resolution_s.cy));
      goto error;
    } // end IF

    direct3DConfiguration_->presentationParameters.BackBufferWidth =
      resolution_s.cx;
    direct3DConfiguration_->presentationParameters.BackBufferHeight =
      resolution_s.cy;
    direct3DConfiguration_->presentationParameters.BackBufferFormat =
      STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT;
    clientWindow_ = direct3DConfiguration_->focusWindow;
    direct3DConfiguration_->presentationParameters.hDeviceWindow = NULL;
    //direct3DConfiguration_->presentationParameters.Windowed = FALSE;
    direct3DConfiguration_->presentationParameters.FullScreen_RefreshRateInHz =
      display_mode_s.RefreshRate;
    //direct3DConfiguration_->presentationParameters.PresentationInterval = ;
  } // end ELSE

  HRESULT result =
    resetDevice (media_type_s,
                 *direct3DConfiguration_,
                 direct3DConfiguration_->handle,
                 direct3DConfiguration_->presentationParameters,
                 defaultStride_,
                 destinationRectangle_);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::resetDevice(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return;

error:
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.direct3DConfiguration);

  if (inherited::isInitialized_)
  {
    clientWindow_ = NULL;
    if (closeWindow_)
    { ACE_ASSERT (direct3DConfiguration_);
      ACE_ASSERT (direct3DConfiguration_->presentationParameters.hDeviceWindow);
      closeWindow_ = false;
      if (unlikely (!::CloseWindow (direct3DConfiguration_->presentationParameters.hDeviceWindow)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    } // end IF
    defaultStride_ = 0;
    if (unlikely (!SetRectEmpty (&destinationRectangle_)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to SetRectEmpty(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    mediaFramework_ = STREAM_LIB_DEFAULT_MEDIAFRAMEWORK;
    if (releaseDeviceHandle_)
    { ACE_ASSERT (direct3DConfiguration_);
      ACE_ASSERT (direct3DConfiguration_->handle);
      direct3DConfiguration_->handle->Release (); direct3DConfiguration_->handle = NULL;
      releaseDeviceHandle_ = false;
    } // end IF
    direct3DConfiguration_ = NULL;
    snapShotNextFrame_ = false;
    transformation_ = NULL;
  } // end IF

  // *TODO*: remove type inferences
  ACE_ASSERT (configuration_in.direct3DConfiguration);
  clientWindow_ =
    (configuration_in.direct3DConfiguration->presentationParameters.Windowed ? configuration_in.direct3DConfiguration->presentationParameters.hDeviceWindow
                                                                             : NULL);
  direct3DConfiguration_ = configuration_in.direct3DConfiguration;
  mediaFramework_ = configuration_in.mediaFramework;

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
bool
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::initialize_Direct3D (struct Stream_MediaFramework_Direct3D_Configuration& configuration_inout,
                                                              const struct _AMMediaType& mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                              IDirect3DDevice9Ex*& handle_inout,
#else
                                                              IDirect3DDevice9*& handle_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                              struct _D3DPRESENT_PARAMETERS_& presentationParameters_inout,
                                                              LONG& stride_out,
                                                              struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize_Direct3D"));

  // initialize return value(s)
  //ACE_OS::memset (&presentationParameters_out, 0, sizeof (struct _D3DPRESENT_PARAMETERS_));
  stride_out = 0;
  ACE_OS::memset (&destinationRectangle_out, 0, sizeof (struct tagRECT));

  // sanity check(s)
  ACE_ASSERT (!configuration_inout.handle);

  IDirect3DDeviceManager9* direct3d_manager_p = NULL;
  UINT reset_token = 0;
  HRESULT result = E_FAIL;

  if (handle_inout)
    goto continue_;

  if (unlikely (!Stream_MediaFramework_DirectDraw_Tools::getDevice (configuration_inout,
                                                                    direct3d_manager_p,
                                                                    reset_token)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectDraw_Tools::getDevice(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  ACE_ASSERT (configuration_inout.handle);
  direct3d_manager_p->Release (); direct3d_manager_p = NULL;
  configuration_inout.handle->AddRef ();
  handle_inout = configuration_inout.handle;

continue_:
  ACE_ASSERT (handle_inout);

  result =
    initialize_Direct3DDevice (configuration_inout.presentationParameters.hDeviceWindow,
                               mediaType_in,
                               handle_inout,
                               presentationParameters_inout,
                               stride_out,
                               destinationRectangle_out);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    configuration_inout.handle->Release (); configuration_inout.handle = NULL;
    handle_inout->Release (); handle_inout = NULL;
    goto error;
  } // end IF

  return true;

error:
  //ACE_OS::memset (&presentationParameters_inout, 0, sizeof (struct _D3DPRESENT_PARAMETERS_));
  stride_out = 0;
  ACE_OS::memset (&destinationRectangle_out, 0, sizeof (struct tagRECT));

  return false;
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                             MediaType>::checkCooperativeLevel (IDirect3DDevice9Ex* handle_in,
#else
                             MediaType>::checkCooperativeLevel (IDirect3DDevice9* handle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                bool& resetDevice_out,
                                                                bool& destroyDevice_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::checkCooperativeLevel"));

  // initialize return value(s)
  resetDevice_out = false;
  destroyDevice_out = false;

  // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_ASSERT (presentationParameters_.hDeviceWindow);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_ASSERT (handle_in);

  HRESULT result =
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    handle_in->CheckDeviceState (presentationParameters_.hDeviceWindow);
#else
    handle_in->TestCooperativeLevel ();
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  switch (result)
  {
    case D3D_OK:
      break;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    case D3DERR_DEVICEHUNG: // try reset first
      resetDevice_out = true;
      break;
    case D3DERR_DEVICEREMOVED:
      destroyDevice_out = true;
      break;
    case D3DERR_OUTOFVIDEOMEMORY:
      resetDevice_out = true;
      break;
    case S_PRESENT_MODE_CHANGED:
      resetDevice_out = true;
      break;
    case S_PRESENT_OCCLUDED:
      break;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    case D3DERR_DEVICELOST:
      break;
    case D3DERR_DEVICENOTRESET:
      resetDevice_out = true;
      break;
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
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::resetDevice (const struct _AMMediaType& mediaType_in,
                                                      struct Stream_MediaFramework_Direct3D_Configuration& configuration_inout,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                      IDirect3DDevice9Ex*& handle_inout,
#else
                                                      IDirect3DDevice9*& handle_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                      struct _D3DPRESENT_PARAMETERS_& presentationParameters_inout,
                                                      LONG& stride_out,
                                                      struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::resetDevice"));

  HRESULT result = D3D_OK;
  bool release_device_b = false;

  // step1: reset/initialize device
  if (handle_inout)
  {
    // sanity check(s)
    if (unlikely (!ACE_OS::thr_equal (ACE_OS::thr_self (), configuration_inout.threadId)))
    {
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: only the device owner thread (was: %d, current: %t) can reset it, continuing\n"),
                  inherited::mod_->name (),
                  configuration_inout.threadId));
#endif // _DEBUG
      goto continue_;
    } // end IF

    // *TODO*: remove ASAP
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, configuration_inout.lock, E_FAIL);

    // *NOTE*: may toggle the device between windowed/fullscreen mode
    if (unlikely (!Stream_MediaFramework_DirectDraw_Tools::reset (handle_inout,
                                                                  configuration_inout)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectDraw_Tools::reset(), aborting\n"),
                  inherited::mod_->name ()));
      return E_FAIL;
    } // end IF
  } // end IF
  else
  { ACE_ASSERT (presentationParameters_inout.hDeviceWindow);
    if (unlikely (!initialize_Direct3D (configuration_inout,
                                        mediaType_in,
                                        configuration_inout.handle,
                                        presentationParameters_inout,
                                        stride_out,
                                        destinationRectangle_out)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3D(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return E_FAIL;
    } // end IF
    release_device_b = true;
  } // end IF
  ACE_ASSERT (handle_inout);

  // step3: update destination rectangle
continue_:
  struct tagRECT source_rectangle_s = {
    0,
    0,
    presentationParameters_inout.BackBufferWidth,
    presentationParameters_inout.BackBufferHeight
  };
  updateDestinationRectangle (presentationParameters_inout.hDeviceWindow,
                              source_rectangle_s,
                              destinationRectangle_out);

  return result;
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
tagRECT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::letterbox_rectangle (const struct tagRECT& source_in,
                                                              const struct tagRECT& destination_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::letterbox_rectangle"));

  int source_width = (source_in.right - source_in.left);
  int source_height = (source_in.bottom - source_in.top);
  int destination_width = (destination_in.right - destination_in.left);
  int destination_height = (destination_in.bottom - destination_in.top);

  int letterbox_width, letterbox_height;
  if (MulDiv (source_width,
              destination_height,
              source_height) <= destination_width)
  {
    // column letter boxing ("pillar box")
    letterbox_width = MulDiv (destination_height,
                              source_width,
                              source_height);
    letterbox_height = destination_height;
  } // end IF
  else
  {
    // row letter boxing
    letterbox_width = destination_width;
    letterbox_height = MulDiv (destination_width,
                               source_height,
                               source_width);
  } // end ELSE

  // construct a centered rectangle within the current destination rectangle
  struct tagRECT result;// SetRectEmpty (&result);
  LONG left = destination_in.left + ((destination_width - letterbox_width) / 2);
  LONG top = destination_in.top + ((destination_height - letterbox_height) / 2);
  SetRect (&result, left, top, left + letterbox_width, top + letterbox_height);

  return result;
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::updateDestinationRectangle (HWND windowHandle_in,
                                                                     const struct tagRECT& sourceRectangle_in,
                                                                     struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::updateDestinationRectangle"));

  if (windowHandle_in)
  {
    if (::IsRectEmpty (&destinationRectangle_out))
      if (unlikely (!::GetClientRect (windowHandle_in,
                                      &destinationRectangle_out)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to GetClientRect(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
  } // end IF
  else
    if (unlikely (!::CopyRect (&destinationRectangle_out,
                               &sourceRectangle_in)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CopyRect(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
  destinationRectangle_out = letterbox_rectangle (sourceRectangle_in,
                                                  destinationRectangle_out);
}

//template <ACE_SYNCH_DECL,
//          typename TimePolicyType,
//          typename ConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType,
//          typename SessionDataType,
//          typename SessionDataContainerType>
//HRESULT
//Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
//                             TimePolicyType,
//                             ConfigurationType,
//                             ControlMessageType,
//                             DataMessageType,
//                             SessionMessageType,
//                             SessionDataType,
//                             SessionDataContainerType>::createSwapChain (HWND windowHandle_in,
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//                                                                         IDirect3DDevice9Ex* handle_in,
//#else
//                                                                         IDirect3DDevice9* handle_in,
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
//                                                                         const struct _D3DPRESENT_PARAMETERS_& presentationParameters_in,
//                                                                         REFGUID subType_in,
//                                                                         IDirect3DSwapChain9*& swapChain_inout)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::createSwapChain"));
//
//  HRESULT result = E_FAIL;
//
//  // initialize return value(s)
//  if (swapChain_inout)
//  {
//    swapChain_inout->Release (); swapChain_inout = NULL;
//  } // end IF
//
//  // sanity check(s)
//  ACE_ASSERT (handle_in);
//  // *NOTE*: Can't Create Additional SwapChain with fullscreen device
//  if (!windowHandle_in ||
//      !presentationParameters_in.Windowed)
//    return E_FAIL;
//
//  struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters_s =
//    presentationParameters_in;
//  ACE_ASSERT (d3d_presentation_parameters_s.BackBufferWidth);
//  ACE_ASSERT (d3d_presentation_parameters_s.BackBufferHeight);
//  //d3d_presentation_parameters.Windowed = TRUE;
//  //d3d_presentation_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
//  d3d_presentation_parameters_s.hDeviceWindow = windowHandle_in;
//  switch (mediaFramework_)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//      if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB24))
//        d3d_presentation_parameters_s.BackBufferFormat = D3DFMT_R8G8B8;
//      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32) ||
//               InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32))
//        d3d_presentation_parameters_s.BackBufferFormat = D3DFMT_X8R8G8B8;
//      else
//      {
//        d3d_presentation_parameters_s.BackBufferFormat = D3DFMT_UNKNOWN;
//        ACE_DEBUG ((LM_WARNING,
//                    ACE_TEXT ("%s: invalid/unknown media subtype (was: \"%s\"), continuing\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (subType_in, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
//        //return E_FAIL;
//      } // end ELSE
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//      if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB24))
//        d3d_presentation_parameters_s.BackBufferFormat = D3DFMT_R8G8B8;
//      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB32))
//        d3d_presentation_parameters_s.BackBufferFormat = D3DFMT_X8R8G8B8;
//      else
//      {
//        d3d_presentation_parameters_s.BackBufferFormat = D3DFMT_UNKNOWN;
//        ACE_DEBUG ((LM_WARNING,
//                    ACE_TEXT ("%s: invalid/unknown media subtype (was: \"%s\"), continuing\n"),
//                    inherited::mod_->name (),
//                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (subType_in, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
//        //return E_FAIL;
//      } // end ELSE
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: invalid/unknown media framework (was: %d), aborting\n"),
//                  inherited::mod_->name (),
//                  mediaFramework_));
//      return E_FAIL;
//    }
//  } // end SWITCH
//  ACE_ASSERT (d3d_presentation_parameters_s.Flags & D3DPRESENTFLAG_VIDEO);
//  ACE_ASSERT (d3d_presentation_parameters_s.Flags & D3DPRESENTFLAG_DEVICECLIP);
//  ACE_ASSERT (d3d_presentation_parameters_s.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER);
//  //ACE_ASSERT (d3d_presentation_parameters_s.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE);
//  //ACE_ASSERT (d3d_presentation_parameters_s.BackBufferCount == MODULE_DEV_CAM_DIRECT3D_DEFAULT_BACK_BUFFERS);
//
//  result =
//    handle_in->CreateAdditionalSwapChain (&d3d_presentation_parameters_s,
//                                          &swapChain_inout);
//
//  return result;
//}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::initialize_Direct3DDevice (HWND windowHandle_in,
                                                                    const struct _AMMediaType& mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                    IDirect3DDevice9Ex* handle_in,
#else
                                                                    IDirect3DDevice9* handle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                    struct _D3DPRESENT_PARAMETERS_& presentationParameters_inout,
                                                                    LONG& stride_out,
                                                                    struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice"));

  HRESULT result = setTransformation (mediaType_in.subtype);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::setTransformation(%s): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mediaType_in.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  presentationParameters_inout.BackBufferFormat =
    Stream_MediaFramework_DirectDraw_Tools::toFormat (mediaType_in.subtype);

  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo));
  ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);

  stride_out =
    ((((video_info_header_p->bmiHeader.biWidth * video_info_header_p->bmiHeader.biBitCount) + 31) & ~31) >> 3);
  // *NOTE*: see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd318229(v=vs.85).aspx
  // *NOTE*: "...A bottom-up image has a negative stride, because stride is
  //         defined as the number of bytes need to move down a row of pixels,
  //         relative to the displayed image. YUV images should always be
  //         top-down, and any image that is contained in a Direct3D surface
  //         must be top-down. RGB images in system memory are usually bottom-up. ..."
  //         see also: https://docs.microsoft.com/en-us/windows/desktop/medfound/image-stride
  if (Stream_MediaFramework_Tools::isRGB (mediaType_in.subtype,
                                          mediaFramework_))
    stride_out = -stride_out;

  //ACE_ASSERT (!swapChain_);
  //ACE_ASSERT (presentationParameters_inout.BackBufferWidth == video_info_header_p->bmiHeader.biWidth);
  //ACE_ASSERT (presentationParameters_inout.BackBufferHeight == video_info_header_p->bmiHeader.biHeight);
  //result = createSwapChain (windowHandle_in,
  //                          handle_in,
  //                          presentationParameters_inout,
  //                          mediaType_in.subtype,
  //                          swapChain_);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::createSwapChain(): \"%s\", continuing\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  return result;
  //} // end IF
  //ACE_ASSERT (swapChain_);

  struct tagRECT source_rectangle_s = {
    0,
    0,
    presentationParameters_inout.BackBufferWidth,
    presentationParameters_inout.BackBufferHeight
  };
  updateDestinationRectangle (windowHandle_in,
                              source_rectangle_s,
                              destinationRectangle_out);

  return S_OK;
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
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::setTransformation (REFGUID subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::setTransformation"));

  HRESULT result = S_OK;
  transformation_ = NULL;

  for (DWORD i = 0;
       i < libacestream_vis_number_of_format_transformations;
       ++i)
  {
    switch (mediaFramework_)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        if (InlineIsEqualGUID (subType_in, libacestream_vis_directshow_format_transformations[i].subType))
          transformation_ =
            libacestream_vis_directshow_format_transformations[i].transformationCB;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        if (InlineIsEqualGUID (subType_in, libacestream_vis_mediafoundation_format_transformations[i].subType))
          transformation_ =
            libacestream_vis_mediafoundation_format_transformations[i].transformationCB;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown media framework (was: %d), aborting\n"),
                    inherited::mod_->name (),
                    mediaFramework_));
        return E_FAIL;
      }
    } // end SWITCH
    if (transformation_)
      return S_OK;
  } // end FOR

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: media subtype \"%s\" is currently not supported, aborting\n"),
              inherited::mod_->name (),
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (subType_in, mediaFramework_).c_str ())));
  switch (mediaFramework_)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      result = VFW_E_INVALIDMEDIATYPE;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      result = MF_E_INVALIDMEDIATYPE;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown media framework (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  mediaFramework_));
      return E_FAIL;
    }
  } // end SWITCH

  return result;
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
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::isFormatSupported (REFGUID subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::isFormatSupported"));

  for (int i = 0;
       i < libacestream_vis_number_of_format_transformations;
       ++i)
  {
    switch (mediaFramework_)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        if (InlineIsEqualGUID (subType_in, libacestream_vis_directshow_format_transformations[i].subtype))
          return true;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        if (InlineIsEqualGUID (subType_in, libacestream_vis_mediafoundation_format_transformations[i].subtype))
          return true;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown media framework (was: %d), aborting\n"),
                    inherited::mod_->name (),
                    mediaFramework_));
        break;
      }
    } // end SWITCH
  } // end FOR

  return false;
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
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::getFormat (DWORD index_in,
                                                    struct _GUID& subType_out) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::getFormat"));

  // initialize return value(s)
  subType_out = GUID_NULL;

  if (unlikely (index_in >= libacestream_vis_number_of_format_transformations))
  {
    switch (mediaFramework_)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        return VFW_E_NO_MORE_TYPES;
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        return MF_E_NO_MORE_TYPES;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown media framework (was: %d), aborting\n"),
                    inherited::mod_->name (),
                    mediaFramework_));
        return E_FAIL;
      }
    } // end SWITCH
  } // end IF

  switch (mediaFramework_)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      subType_out =
        libacestream_vis_directshow_format_transformations[index_in].subtype;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      subType_out =
        libacestream_vis_mediafoundation_format_transformations[index_in].subtype;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown media framework (was: %d), aborting\n"),
                  inherited::mod_->name (),
                  mediaFramework_));
      return E_FAIL;
    }
  } // end SWITCH

  return S_OK;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
  Stream_Vis_DirectShow_Target_Direct3D_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          SessionDataContainerType,
                                          MediaType>::Stream_Vis_DirectShow_Target_Direct3D_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_DirectShow_Target_Direct3D_T::Stream_Vis_DirectShow_Target_Direct3D_T"));

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
Stream_Vis_DirectShow_Target_Direct3D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_DirectShow_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  bool reset_device_b = false;
  bool unlock_rect_b = false;
  const typename DataMessageType::DATA_T& message_data_r =
    message_inout->getR ();
  IMediaSample* media_sample_p = NULL;
  BYTE* data_p = NULL;
  IDirect3DSurface9* d3d_surface_p = NULL, *d3d_surface_2 = NULL;
  BYTE* scanline0_p = NULL;
  LONG stride = 0;
  struct _D3DLOCKED_RECT d3d_locked_rectangle_s;

  // sanity check(s)
  ACE_ASSERT (inherited::handle_);
  ACE_ASSERT (inherited::swapChain_);
  ACE_ASSERT (inherited::transformation_);

  if (message_data_r.sample)
  {
    result = message_data_r.sample->GetPointer (&data_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMediaSample::GetPointer(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    ACE_ASSERT (data_p);
  } // end IF
  else
    data_p = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());

  inherited::checkCooperativeLevel (inherited::handle_,
                                    reset_device_b);
  if (reset_device_b)
  {
    // sanity check(s)
    ACE_ASSERT (inherited::sessionData_);
    const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
    ACE_ASSERT (session_data_r.inputFormat);

    result = inherited::resetDevice (*inherited::direct3DConfiguration_,
                                     inherited::window_,
                                     inherited::presentationParameters_,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                     inherited::fullscreenDisplayMode_,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                     inherited::width_,
                                     inherited::height_,
                                     inherited::defaultStride_,
                                     *session_data_r.inputFormat,
                                     inherited::format_,
                                     inherited::handle_,
                                     inherited::swapChain_,
                                     inherited::destinationRectangle_);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::resetDevice(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF

  stride = inherited::defaultStride_;
  if (inherited::defaultStride_ < 0)
  {
    // Bottom-up orientation. Return a pointer to the start of the last row
    // *in memory*, which is the top row of the image
    scanline0_p =
      data_p + ::abs (inherited::defaultStride_) * (inherited::height_ - 1);
  } // end IF
  else
  {
    // Top-down orientation. Return a pointer to the start of the buffer
    scanline0_p = data_p;
  } // end ELSE

  result =
    inherited::swapChain_->GetBackBuffer (0,
                                          D3DBACKBUFFER_TYPE_MONO,
                                          &d3d_surface_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSwapChain9::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle_s,
                                    NULL,
                                    D3DLOCK_NOSYSLOCK);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSurface9::LockRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  unlock_rect_b = true;

  // *NOTE*: convert the frame (this also copies it to the Direct3D surface)
  try {
    inherited::transformation_ ((BYTE*)d3d_locked_rectangle_s.pBits,
                                d3d_locked_rectangle_s.Pitch,
                                scanline0_p,
                                stride,
                                inherited::width_,
                                inherited::height_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in transformation callback, continuing\n"),
                inherited::mod_->name ()));
    goto error;
  }

  result = d3d_surface_p->UnlockRect ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  unlock_rect = false;

  result =
    inherited::handle_->GetBackBuffer (0,
                                       0,
                                       D3DBACKBUFFER_TYPE_MONO,
                                       &d3d_surface_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  //// color-fill the back buffer ?
  //result = handle_->ColorFill (d3d_surface_2,
  //                             NULL,
  //                             D3DCOLOR_XRGB (0, 0, 0x80));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::ColorFill(): \"%s\", returning\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  // blit the frame
  result =
    inherited::handle_->StretchRect (d3d_surface_p,
                                     NULL,
                                     d3d_surface_2,
                                     //&destinationRectangle_,
                                     NULL,
                                     D3DTEXF_NONE);
  if (FAILED (result)) // D3DERR_INVALIDCALL: 0x8876086c
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::StretchRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  d3d_surface_p->Release (); d3d_surface_p = NULL;
  d3d_surface_2->Release (); d3d_surface_2 = NULL;

  // present the frame
  result = inherited::handle_->Present (NULL,
                                        NULL,
                                        NULL,
                                        NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::Present(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (unlock_rect)
  {
    result = d3d_surface_p->UnlockRect ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
  if (d3d_surface_p)
    d3d_surface_p->Release ();
  if (d3d_surface_2)
    d3d_surface_2->Release ();

  return;

continue_:
  return;
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType,
                                             MediaType>::Stream_Vis_MediaFoundation_Target_Direct3D_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , interlaceMode_ (MFVideoInterlace_Unknown)
 , pixelAspectRatio_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::Stream_Vis_MediaFoundation_Target_Direct3D_T"));

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
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  bool reset_device_b = false;
  bool destroy_device_b = false;
  bool unlock_rect_b = false;
  IDirect3DSurface9* d3d_surface_p = NULL, *d3d_surface_2 = NULL;
  struct _D3DLOCKED_RECT d3d_locked_rectangle_s;
  BYTE* scanline0_p = NULL;
  std::string filename_string;
  BYTE* data_p = NULL;
  bool unlock_media_buffer = false;
  const typename DataMessageType::DATA_T& message_data_r =
    message_inout->getR ();
  IMFMediaBuffer* media_buffer_p = NULL;

  // sanity check(s)
  //ACE_ASSERT (message_inout->length () == inherited::configuration_->format->lSampleSize);
  ACE_ASSERT (inherited::direct3DConfiguration_);
  if (unlikely (!inherited::direct3DConfiguration_->presentationParameters.Windowed &&
                !inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow &&
                !inherited::direct3DConfiguration_->focusWindow))
    return; // --> nothing to do

  // *TODO*: remove ASAP
  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, inherited::direct3DConfiguration_->lock);

  if (message_data_r.sample)
  {
    //DWORD count = 0;
    //result = message_data_r.sample->GetBufferCount (&count);
    //ACE_ASSERT (SUCCEEDED (result));
    //ACE_ASSERT (count == 1);
    result = message_data_r.sample->GetBufferByIndex (0, &media_buffer_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFSample::GetBufferByIndex(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return;
    } // end IF
    ACE_ASSERT (media_buffer_p);

    // *NOTE*: lock the video buffer. This method returns a pointer to the first
    //         scan line in the image, and the stride in bytes
    result = media_buffer_p->Lock (&data_p,
                                   NULL,
                                   NULL);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaBuffer::Lock(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    unlock_media_buffer = true;
  } // end IF
  else
    data_p = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());

//  checkCooperativeLevel (inherited::handle_,
//                         reset_device,
//                         destroy_device);
//  if (reset_device)
//  {
//    // sanity check(s)
//    ACE_ASSERT (inherited::sessionData_);
//    const typename SessionDataContainerType::DATA_T& session_data_r =
//      inherited::sessionData_->getR ();
//    ACE_ASSERT (session_data_r.inputFormat);
//
//    result = resetDevice (*inherited::direct3DConfiguration_,
//                          *session_data_r.inputFormat,
//                          inherited::presentationParameters_,
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//                          inherited::fullscreenDisplayMode_,
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
//                          inherited::defaultStride_,
//                          inherited::handle_,
//                          //inherited::swapChain_,
//                          inherited::destinationRectangle_);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::resetDevice(): \"%s\", aborting\n"),
//                  inherited::mod_->name (),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//  else if (destroy_device)
//  { // *TODO*
//    ACE_ASSERT (false);
//    ACE_NOTSUP;
//    ACE_NOTREACHED (return;)
//  } // end ELSE IF

  if (inherited::defaultStride_ < 0)
  { ACE_ASSERT (inherited::direct3DConfiguration_->presentationParameters.BackBufferHeight);
    // Bottom-up orientation. Return a pointer to the start of the last row
    // *in memory*, which is the top row of the image
    scanline0_p =
      data_p +
        (::abs (inherited::defaultStride_) * (inherited::direct3DConfiguration_->presentationParameters.BackBufferHeight - 1));
  } // end IF
  else
  {
    // Top-down orientation. Return a pointer to the start of the buffer
    scanline0_p = data_p;
  } // end ELSE

  // convert the frame to the display format by applying the appropriate
  // transformation (also copies it to a Direct3D surface)
  // sanity check(s)
  ACE_ASSERT (inherited::direct3DConfiguration_->handle);
  ACE_ASSERT (inherited::direct3DConfiguration_->presentationParameters.BackBufferCount >= 2);
  //ACE_ASSERT (swapChain_);
  //result = swapChain_->GetBackBuffer (0,
  //                                    D3DBACKBUFFER_TYPE_MONO,
  //                                    &d3d_surface_p);
  result =
    inherited::direct3DConfiguration_->handle->GetBackBuffer (0,                       // swap chain
                                                              1,                       // back buffer
                                                              D3DBACKBUFFER_TYPE_MONO, // type
                                                              &d3d_surface_p);         // return value: handle
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("%s: failed to IDirect3DSwapChain9::GetBackBuffer(): \"%s\", returning\n"),
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (d3d_surface_p);

  ACE_OS::memset (&d3d_locked_rectangle_s, 0, sizeof (struct _D3DLOCKED_RECT));
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle_s,
                                    NULL,
                                    D3DLOCK_NOSYSLOCK);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSurface9::LockRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  unlock_rect_b = true;

  // sanity check(s)
  ACE_ASSERT (inherited::transformation_);
  ACE_ASSERT (::abs (d3d_locked_rectangle_s.Pitch) >= ::abs (inherited::defaultStride_));
  try {
    inherited::transformation_ ((BYTE*)d3d_locked_rectangle_s.pBits,
                                d3d_locked_rectangle_s.Pitch,
                                scanline0_p,
                                inherited::defaultStride_,
                                inherited::direct3DConfiguration_->presentationParameters.BackBufferWidth,
                                inherited::direct3DConfiguration_->presentationParameters.BackBufferHeight);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: caught exception in transformation callback, continuing\n"),
                inherited::mod_->name ()));
    goto error;
  }

  result = d3d_surface_p->UnlockRect ();
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  unlock_rect_b = false;

  if (unlock_media_buffer)
  {
    result = media_buffer_p->Unlock ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaBuffer::Unlock(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    unlock_media_buffer = false;
    media_buffer_p->Release (); media_buffer_p = NULL;
  } // end IF

  // save the frame ?
  if (likely (!inherited::snapShotNextFrame_))
    goto continue_;
  ACE_ASSERT (inherited::configuration_);
  filename_string =
    ACE_TEXT_ALWAYS_CHAR (ACE::dirname (inherited::configuration_->targetFileName.c_str (),
                                        ACE_DIRECTORY_SEPARATOR_CHAR));
  filename_string += ACE_DIRECTORY_SEPARATOR_CHAR;
  filename_string +=
    ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_DEFAULT_SCREENSHOT_FILENAME_STRING);
  enum _D3DXIMAGE_FILEFORMAT format_e =
    STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_SCREENSHOT_DEFAULT_FORMAT;
  filename_string +=
    Stream_MediaFramework_DirectDraw_Tools::toFilenameExtension (format_e);
  if (unlikely (!Common_Image_Tools::save (filename_string,
                                           format_e,
                                           d3d_surface_p)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Common_Image_Tools::save(), continuing\n"),
                inherited::mod_->name ()));
#if defined (_DEBUG)
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: saved screenshot \"%s\"...\n"),
                inherited::mod_->name (),
                ACE_TEXT (filename_string.c_str ())));
#endif // _DEBUG
  inherited::snapShotNextFrame_ = false;

continue_:
  // stretch the frame to the output window dimensions
  result =
    inherited::direct3DConfiguration_->handle->GetBackBuffer (0,                       // swap chain
                                                              0,                       // back buffer
                                                              D3DBACKBUFFER_TYPE_MONO, // type
                                                              &d3d_surface_2);         // return value: handle
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (d3d_surface_2);
  //// color-fill the back buffer ?
  //result = handle_->ColorFill (d3d_surface_2,
  //                                         NULL,
  //                                         D3DCOLOR_XRGB (0, 0, 0x80));
  //if (unlikely (FAILED (result)))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::ColorFill(): \"%s\", returning\n"),
  //              inherited::mod_->name (),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  result =
    inherited::direct3DConfiguration_->handle->StretchRect (d3d_surface_p,
                                                            NULL,
                                                            d3d_surface_2,
                                                            //&destinationRectangle_,
                                                            NULL, // use window/fullscreen mode dimensions
                                                            D3DTEXF_NONE);
  if (unlikely (FAILED (result))) // D3DERR_INVALIDCALL: 0x8876086c
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::StretchRect(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  d3d_surface_p->Release (); d3d_surface_p = NULL;
  d3d_surface_2->Release (); d3d_surface_2 = NULL;

  // present the frame
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    inherited::direct3DConfiguration_->handle->PresentEx (NULL, // pSourceRect
                                                          NULL, // pDestRect
                                                          NULL, // hDestWindowOverride
                                                          NULL, // pDirtyRegion
                                                          0);   // dwFlags
#else
  result =
    inherited::direct3DConfiguration_->handle->Present (NULL,  // pSourceRect
                                                        NULL,  // pDestRect
                                                        NULL,  // hDestWindowOverride
                                                        NULL); // pDirtyRegion
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (FAILED (result))
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::PresentEx(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9::Present(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    goto error;
  } // end IF

  return;

error:
  if (unlock_rect_b)
  { ACE_ASSERT (d3d_surface_p);
    result = d3d_surface_p->UnlockRect ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
  if (d3d_surface_p)
    d3d_surface_p->Release ();
  if (d3d_surface_2)
    d3d_surface_2->Release ();

  if (unlock_media_buffer)
  {
    result = media_buffer_p->Unlock ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFMediaBuffer::Unlock(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
  if (media_buffer_p)
    media_buffer_p->Release ();
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
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  //int result = -1;
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
      ACE_ASSERT (inherited::direct3DConfiguration_);
      if (unlikely (!inherited::direct3DConfiguration_->presentationParameters.Windowed &&
                    !inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow &&
                    !inherited::direct3DConfiguration_->focusWindow))
        return; // --> nothing to do

      IMFTopology* topology_p = NULL;
      IMFMediaType* media_type_p = NULL;
      enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
        MFSESSION_GETFULLTOPOLOGY_CURRENT;
      TOPOID node_id = 0;
      Common_Image_Resolution_t resolution_s;

      result_2 = CoInitializeEx (NULL,
                                 (COINIT_MULTITHREADED    |
                                  COINIT_DISABLE_OLE1DDE  |
                                  COINIT_SPEED_OVER_MEMORY));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (session_data_r.session);
      // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
      //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
      //         --> (try to) wait for the next MESessionTopologySet event
      // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
      //         still fails with MF_E_INVALIDREQUEST)
      do
      {
        result_2 = session_data_r.session->GetFullTopology (flags,
                                                            0,
                                                            &topology_p);
      } while (result_2 == MF_E_INVALIDREQUEST);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (topology_p);

      if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_p,
                                                                         node_id,
                                                                         media_type_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_ASSERT (media_type_p);
      MediaType media_type_s;
      inherited::getMediaType (media_type_p,
                               media_type_s);
      session_data_r.formats.push_back (media_type_s);
      struct _AMMediaType media_type_2;
      inherited::getMediaType (media_type_s,
                               media_type_2);

      // sanity check(s)
      UINT32 width_i = 0, height_i = 0;
      HRESULT result =
        MFGetAttributeSize (media_type_p,
                            MF_MT_FRAME_SIZE,
                            &width_i, &height_i);
      resolution_s.cx = static_cast<LONG> (width_i);
      resolution_s.cy = static_cast<LONG> (height_i);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT ((resolution_s.cx == inherited::direct3DConfiguration_->presentationParameters.BackBufferWidth) && (resolution_s.cy == inherited::direct3DConfiguration_->presentationParameters.BackBufferHeight));
      media_type_p->Release (); media_type_p = NULL;
      HWND window_handle_p =
        (inherited::direct3DConfiguration_->presentationParameters.Windowed ? inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow
                                                                            : inherited::direct3DConfiguration_->focusWindow);
      if (window_handle_p)
      {
        ACE_ASSERT (IsWindow (window_handle_p));
      } // end IF

      if (inherited::direct3DConfiguration_->presentationParameters.Windowed &&
          !window_handle_p)
      { ACE_ASSERT (!inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow);
        ACE_ASSERT (!inherited::direct3DConfiguration_->focusWindow);
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
        inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow =
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
                          inherited::direct3DConfiguration_->presentationParameters.BackBufferWidth,  // nWidth
                          inherited::direct3DConfiguration_->presentationParameters.BackBufferHeight, // nHeight
                          NULL,                                    // hWndParent
                          NULL,                                    // hMenu
                          GetModuleHandle (NULL),                  // hInstance
                          NULL);                                   // lpParam
        if (unlikely (!inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow))
        { // ERROR_INVALID_PARAMETER: 87
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        window_handle_p =
          inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow;
        inherited::closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_handle_p);
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@\n"),
                  inherited::mod_->name (),
                  window_handle_p));
#endif // _DEBUG
      if (inherited::direct3DConfiguration_->presentationParameters.Windowed)
        inherited::clientWindow_ = window_handle_p;

      inherited::defaultStride_ = 0;
      if (unlikely (!SetRectEmpty (&(inherited::destinationRectangle_))))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to SetRectEmpty(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      IDirect3DDevice9Ex* device_handle_p =
#else
      IDirect3DDevice9* device_handle_p =
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
        session_data_r.direct3DDevice; // prefer session data over configuration
      if (device_handle_p)
      {
        if (inherited::direct3DConfiguration_->handle)
        {
          inherited::direct3DConfiguration_->handle->Release (); inherited::direct3DConfiguration_->handle = NULL;
        } // end IF
        ACE_ASSERT (!inherited::direct3DConfiguration_->handle);
        session_data_r.direct3DDevice->AddRef ();
        inherited::direct3DConfiguration_->handle = session_data_r.direct3DDevice;
        inherited::releaseDeviceHandle_ = true;
      } // end IF
      else
      {
        if (inherited::direct3DConfiguration_->handle)
        {
          inherited::direct3DConfiguration_->handle->AddRef ();
          session_data_r.direct3DDevice =
            inherited::direct3DConfiguration_->handle;
          device_handle_p = inherited::direct3DConfiguration_->handle;
        } // end IF
      } // end ELSE
      if (device_handle_p)
      {
        result_2 =
          initialize_Direct3DDevice (inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow,
                                     media_type_2,
                                     inherited::direct3DConfiguration_->handle,
                                     inherited::direct3DConfiguration_->presentationParameters,
                                     inherited::defaultStride_,
                                     inherited::destinationRectangle_);
        if (unlikely (FAILED (result_2)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error;
        } // end IF
      } // end IF
      else
      {
        // sanity check(s)
        ACE_ASSERT (!inherited::direct3DConfiguration_->handle);
        if (unlikely (!initialize_Direct3D (*inherited::direct3DConfiguration_,
                                            media_type_2,
                                            inherited::direct3DConfiguration_->handle,
                                            inherited::direct3DConfiguration_->presentationParameters,
                                            inherited::defaultStride_,
                                            inherited::destinationRectangle_)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to initialize_Direct3D(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
        ACE_ASSERT (inherited::direct3DConfiguration_->handle);
        inherited::releaseDeviceHandle_ = true;
        device_handle_p = inherited::direct3DConfiguration_->handle;
      } // end ELSE
      ACE_ASSERT (device_handle_p);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);

      topology_p->Release (); topology_p = NULL;

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_2);

      if (topology_p)
      {
        topology_p->Release (); topology_p = NULL;
      } // end IF

      if (inherited::direct3DConfiguration_->handle)
      {
        inherited::direct3DConfiguration_->handle->Release (); inherited::direct3DConfiguration_->handle = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (inherited::closeWindow_)
      { ACE_ASSERT (inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow);
        inherited::closeWindow_ = false;
        if (!::CloseWindow (inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow =
          NULL;
      } // end IF

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
                    ACE_TEXT ("%s: failed to CoInitializeEx(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      if (inherited::releaseDeviceHandle_)
      { ACE_ASSERT (inherited::direct3DConfiguration_);
        ACE_ASSERT (inherited::direct3DConfiguration_->handle);
        inherited::direct3DConfiguration_->handle->Release (); inherited::direct3DConfiguration_->handle = NULL;
        inherited::releaseDeviceHandle_ = false;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (inherited::closeWindow_)
      { ACE_ASSERT (inherited::direct3DConfiguration_);
        ACE_ASSERT (inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow);
        inherited::closeWindow_ = false;
        if (!::CloseWindow (inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        inherited::direct3DConfiguration_->presentationParameters.hDeviceWindow =
          NULL;
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
HRESULT
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType,
                                             MediaType>::get_default_stride (IMFMediaType* mediaType_in,
                                                                             LONG& stride_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::get_default_stride"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  // initialize return value(s)
  stride_out = 0;

  HRESULT result = mediaType_in->GetUINT32 (MF_MT_DEFAULT_STRIDE,
                                            (UINT32*)&stride_out);
  if (SUCCEEDED (result))
    return result;

  struct _GUID sub_type = GUID_NULL;
  result = mediaType_in->GetGUID (MF_MT_SUBTYPE, &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  UINT32 width = 0;
  UINT32 height = 0;
  result = MFGetAttributeSize (mediaType_in,
                               MF_MT_FRAME_SIZE,
                               &width, &height);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  result = MFGetStrideForBitmapInfoHeader (sub_type.Data1,
                                           width,
                                           &stride_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to MFGetStrideForBitmapInfoHeader(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  result = mediaType_in->SetUINT32 (MF_MT_DEFAULT_STRIDE,
                                    (UINT32)stride_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFMediaType::SetUINT32(MF_MT_DEFAULT_STRIDE): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF

  return S_OK;
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
tagRECT
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType,
                                             MediaType>::normalize_aspect_ratio (const struct tagRECT& rectangle_in,
                                                                                 const struct _MFRatio& pixelAspectRatio_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::normalize_aspect_ratio"));

  // start with a rectangle the same size as the input, but offset to the origin
  // (0,0)
  struct tagRECT result =
    { 0, 0,
      rectangle_in.right - rectangle_in.left, rectangle_in.bottom - rectangle_in.top };

  if ((pixelAspectRatio_in.Numerator != 1)  ||
      (pixelAspectRatio_in.Denominator != 1))
  {
    // adjust the PAR
    if (pixelAspectRatio_in.Numerator > pixelAspectRatio_in.Denominator)
    {
      // input has "wide" pixels --> stretch the width
      result.right = MulDiv (result.right,
                             pixelAspectRatio_in.Numerator,
                             pixelAspectRatio_in.Denominator);
    } // end IF
    else if (pixelAspectRatio_in.Numerator < pixelAspectRatio_in.Denominator)
    {
      // input has "tall" pixels --> stretch the height
      result.bottom = MulDiv (result.bottom,
                              pixelAspectRatio_in.Denominator,
                              pixelAspectRatio_in.Numerator);
    } // end ELSE IF
    // else: PAR is 1:1 --> nothing to do
  } // end IF

  return result;
}
