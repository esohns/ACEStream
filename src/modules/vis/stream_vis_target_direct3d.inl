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

#include "stream_macros.h"
#include "stream_session_message_base.h"

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
          typename SessionDataContainerType>
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::Stream_Vis_Target_Direct3D_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , closeWindow_ (false)
 , defaultStride_ (0)
 , destinationRectangle_ ()
 , format_ (D3DFMT_UNKNOWN)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
 , fullscreenDisplayMode_ ()
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
 , presentationParameters_ ()
 , height_ (0)
 , width_ (0)
 , window_ (NULL)
 , deviceHandle_ (NULL)
 , swapChain_ (NULL)
 , transformation_ (NULL)
 , mediaFramework_ (MODULE_LIB_DEFAULT_MEDIAFRAMEWORK)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::Stream_Vis_Target_Direct3D_T"));

  if (!SetRectEmpty (&destinationRectangle_))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to SetRectEmpty(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  ACE_OS::memset (&fullscreenDisplayMode_,
                  0,
                  sizeof (struct D3DDISPLAYMODEEX));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  ACE_OS::memset (&presentationParameters_,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::~Stream_Vis_Target_Direct3D_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::~Stream_Vis_Target_Direct3D_T"));

  if (deviceHandle_)
    deviceHandle_->Release ();

  if (closeWindow_)
  { ACE_ASSERT (window_);
    if (!::CloseWindow (window_))
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
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  bool reset_device = false;
  HRESULT result = E_FAIL;
  IDirect3DSurface9* d3d_surface_p = NULL;
  IDirect3DSurface9* d3d_backbuffer_p = NULL;
  BYTE* scanline0_p = NULL;
  struct _D3DLOCKED_RECT d3d_locked_rectangle;
  bool unlock_rect = false;

  // sanity check(s)
  ACE_ASSERT (direct3DConfiguration_);
  //ACE_ASSERT (message_inout->length () == inherited::configuration_->format->lSampleSize);
  ACE_ASSERT (deviceHandle_);
  ACE_ASSERT (swapChain_);
  ACE_ASSERT (transformation_);

  checkCooperativeLevel (deviceHandle_,
                         reset_device);
  if (unlikely (reset_device))
  {
    // sanity check(s)
    ACE_ASSERT (inherited::sessionData_);
    const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
    ACE_ASSERT (session_data_r.inputFormat);

    result = resetDevice (*direct3DConfiguration_,
                          window_,
                          presentationParameters_,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                          fullscreenDisplayMode_,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                          width_,
                          height_,
                          defaultStride_,
                          *session_data_r.inputFormat,
                          format_,
                          deviceHandle_,
                          swapChain_,
                          destinationRectangle_);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::resetDevice(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF

  if (defaultStride_ < 0)
  {
    // Bottom-up orientation. Return a pointer to the start of the last row
    // *in memory*, which is the top row of the image
    scanline0_p =
      (reinterpret_cast<BYTE*> (message_inout->rd_ptr ()) +
       (::abs (defaultStride_) * (height_ - 1)));
  } // end IF
  else
  {
    // Top-down orientation. Return a pointer to the start of the buffer
    scanline0_p = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());
  } // end ELSE

  result = swapChain_->GetBackBuffer (0,
                                      D3DBACKBUFFER_TYPE_MONO,
                                      &d3d_surface_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DSwapChain9::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (d3d_surface_p);
  ACE_OS::memset (&d3d_locked_rectangle, 0, sizeof (struct _D3DLOCKED_RECT));
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle,
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
  unlock_rect = true;

  // *NOTE*: convert the frame (this also copies it to the Direct3D surface)
  ACE_ASSERT (::abs (d3d_locked_rectangle.Pitch) >= ::abs (defaultStride_));
  try {
    transformation_ ((BYTE*)d3d_locked_rectangle.pBits,
                     d3d_locked_rectangle.Pitch,
                     scanline0_p,
                     defaultStride_,
                     width_,
                     height_);
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
  unlock_rect = false;

  result = deviceHandle_->GetBackBuffer (0,
                                         0,
                                         D3DBACKBUFFER_TYPE_MONO,
                                         &d3d_backbuffer_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (d3d_backbuffer_p);
  //// color-fill the back buffer ?
  //result = deviceHandle_->ColorFill (d3d_backbuffer_p,
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

  // save the frame
  //if(saveframe == true)
  //{
  //  D3DXSaveSurfaceToFile (L"Capture.jpg",
  //                         D3DXIFF_JPG,
  //                         d3d_surface_p,
  //                         NULL,
  //                         NULL);
  //  saveframe= false;
  //} // end IF

  // blit the frame
  result = deviceHandle_->StretchRect (d3d_surface_p,
                                       NULL,
                                       d3d_backbuffer_p,
                                       //&destinationRectangle_,
                                       NULL,
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
  d3d_backbuffer_p->Release (); d3d_backbuffer_p = NULL;

  // present the frame
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = deviceHandle_->PresentEx (NULL, // pSourceRect
                                     NULL, // pDestRect
                                     NULL, // hDestWindowOverride
                                     NULL, // pDirtyRegion
                                     0);   // dwFlags
#else
  result = deviceHandle_->Present (NULL,  // pSourceRect
                                   NULL,  // pDestRect
                                   NULL,  // hDestWindowOverride
                                   NULL); // pDirtyRegion
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::PresentEx(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  return;

error:
  if (unlock_rect)
  { ACE_ASSERT (d3d_surface_p);
    result = d3d_surface_p->UnlockRect ();
    if (unlikely (FAILED (result)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IDirect3DSurface9::UnlockRect(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
  if (d3d_surface_p)
    d3d_surface_p->Release ();
  if (d3d_backbuffer_p)
    d3d_backbuffer_p->Release ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
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
      if (deviceHandle_)
      {
        deviceHandle_->Release (); deviceHandle_ = NULL;
      } // end IF
      ACE_ASSERT (session_data_r.inputFormat);

      ACE_ASSERT (InlineIsEqualGUID (session_data_r.inputFormat->formattype, FORMAT_VideoInfo) &&
                  (session_data_r.inputFormat->cbFormat == sizeof (struct tagVIDEOINFOHEADER)));
      struct tagVIDEOINFOHEADER* video_info_header_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.inputFormat->pbFormat);

      if (!window_)
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
        window_ =
          CreateWindowEx (window_style_ex,                         // dwExStyle
#if defined (UNICODE)
                          ACE_TEXT_ALWAYS_WCHAR ("EDIT"),                   // lpClassName
                          ACE_TEXT_ALWAYS_WCHAR (inherited::mod_->name ()), // lpWindowName
#else
                          ACE_TEXT_ALWAYS_CHAR ("EDIT"),                    // lpClassName
                          ACE_TEXT_ALWAYS_CHAR (inherited::mod_->name ()),  // lpWindowName
#endif // UNICODE
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
        if (!window_)
        { // ERROR_INVALID_PARAMETER: 87
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        //BOOL result_3 = ShowWindow (window_, SW_SHOWNA);
        //ACE_UNUSED_ARG (result_3);
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@\n"),
                  inherited::mod_->name (),
                  window_));

      //destinationRectangle_ = configuration_->area;
      SetRectEmpty (&destinationRectangle_);
      if (session_data_r.direct3DDevice)
      {
        session_data_r.direct3DDevice->AddRef ();
        deviceHandle_ = session_data_r.direct3DDevice;

        ACE_ASSERT (direct3DConfiguration_);
        direct3DConfiguration_->presentationParameters.BackBufferWidth =
          video_info_header_p->bmiHeader.biWidth;
        direct3DConfiguration_->presentationParameters.BackBufferHeight =
          video_info_header_p->bmiHeader.biHeight;
        result_2 = initialize_Direct3DDevice (*direct3DConfiguration_,
                                              window_,
                                              *session_data_r.inputFormat,
                                              deviceHandle_,
                                              width_,
                                              height_,
                                              defaultStride_,
                                              destinationRectangle_);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error;
        } // end IF
      } // end IF
      else
      { ACE_ASSERT (!deviceHandle_);
        if (!initialize_Direct3D (*direct3DConfiguration_,
                                  window_,
                                  *session_data_r.inputFormat,
                                  deviceHandle_,
                                  presentationParameters_,
                                  width_,
                                  height_,
                                  defaultStride_,
                                  destinationRectangle_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to initialize_Direct3D(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end ELSE
      ACE_ASSERT (deviceHandle_);

      goto continue_;

error:
      if (deviceHandle_)
      {
        deviceHandle_->Release (); deviceHandle_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

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

continue_:
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

      if (deviceHandle_)
      {
        deviceHandle_->Release (); deviceHandle_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      {
        closeWindow_ = false;
        if (!::CloseWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        window_ = NULL;
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
          typename SessionDataContainerType>
bool
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                    Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (deviceHandle_)
    {
      deviceHandle_->Release (); deviceHandle_ = NULL;
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
  } // end IF

  // *TODO*: remove type inferences
  direct3DConfiguration_ = configuration_in.direct3DConfiguration;
  mediaFramework_ = configuration_in.mediaFramework;
  window_ = configuration_in.window;
  if (window_)
  {
    ACE_ASSERT (IsWindow (window_));
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
          typename SessionDataContainerType>
bool
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::initialize_Direct3D (const struct Stream_Module_Device_Direct3DConfiguration& configuration_in,
                                                                             HWND windowHandle_in,
                                                                             const struct _AMMediaType& mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                             IDirect3DDevice9Ex*& deviceHandle_out,
#else
                                                                             IDirect3DDevice9*& deviceHandle_out,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                             struct _D3DPRESENT_PARAMETERS_& presentationParameters_out,
                                                                             LONG& width_out,
                                                                             LONG& height_out,
                                                                             LONG& stride_out,
                                                                             struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize_Direct3D"));

  // initialize return value(s)
  ACE_OS::memset (&presentationParameters_out,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));

  // sanity check(s)
  ACE_ASSERT (!deviceHandle_out);

  IDirect3DDeviceManager9* direct3d_manager_p = NULL;
  UINT reset_token = 0;
  HRESULT result = E_FAIL;

  if (!Stream_Module_Device_Tools::getDirect3DDevice (configuration_in,
                                                      windowHandle_in,
                                                      mediaType_in,
                                                      deviceHandle_out,
                                                      presentationParameters_out,
                                                      direct3d_manager_p,
                                                      reset_token))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_Tools::getDirect3DDevice(), aborting\n"),
                inherited::mod_->name ()));
    return false;
  } // end IF
  ACE_ASSERT (deviceHandle_out);
  direct3d_manager_p->Release (); direct3d_manager_p = NULL;

  result = initialize_Direct3DDevice (configuration_in,
                                      windowHandle_in,
                                      mediaType_in,
                                      deviceHandle_out,
                                      width_out,
                                      height_out,
                                      stride_out,
                                      destinationRectangle_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    deviceHandle_out->Release (); deviceHandle_out = NULL;
    ACE_OS::memset (&presentationParameters_out,
                    0,
                    sizeof (struct _D3DPRESENT_PARAMETERS_));
    return false;
  } // end IF

  return true;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                             SessionDataContainerType>::checkCooperativeLevel (IDirect3DDevice9Ex* deviceHandle_in,
#else
                             SessionDataContainerType>::checkCooperativeLevel (IDirect3DDevice9* deviceHandle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                               bool& resetDevice_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::checkCooperativeLevel"));

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  HRESULT result = deviceHandle_in->TestCooperativeLevel ();
  switch (result)
  {
    case D3D_OK:
      break;
    case D3DERR_DEVICELOST:
      result = S_OK;
      // *WARNING*: falls through here
    case D3DERR_DEVICENOTRESET:
    {
      resetDevice_out = true;
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
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::resetDevice (const struct Stream_Module_Device_Direct3DConfiguration& configuration_in,
                                                                     HWND windowHandle_in,
                                                                     struct _D3DPRESENT_PARAMETERS_& presentationParameters_inout,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                     struct D3DDISPLAYMODEEX& fullScreenDisplayMode_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                     LONG& width_out,
                                                                     LONG& height_out,
                                                                     LONG& stride_out,
                                                                     const struct _AMMediaType& mediaType_in,
                                                                     enum _D3DFORMAT format_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                     IDirect3DDevice9Ex*& deviceHandle_inout,
#else
                                                                     IDirect3DDevice9*& deviceHandle_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                     IDirect3DSwapChain9*& swapChain_inout,
                                                                     struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::resetDevice"));

  HRESULT result = S_OK;
  bool release_device = false;

  if (deviceHandle_inout)
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    result = deviceHandle_inout->ResetEx (&presentationParameters_inout,
                                          &fullScreenDisplayMode_inout);
#else
    result = deviceHandle_inout->Reset (&presentationParameters_inout);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::ResetEx(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    } // end IF
  } // end IF

  if (!deviceHandle_inout)
  {
    if (!initialize_Direct3D (configuration_in,
                              windowHandle_in,
                              mediaType_in,
                              deviceHandle_inout,
                              presentationParameters_inout,
                              width_out,
                              height_out,
                              stride_out,
                              destinationRectangle_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3D(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return E_FAIL;
    } // end IF
    release_device = true;
  } // end IF
  ACE_ASSERT (deviceHandle_inout);

  if (!swapChain_inout &&
      (format_in != D3DFMT_UNKNOWN))
  {
    result = createSwapChain (windowHandle_in,
                              deviceHandle_inout,
                              configuration_in.presentationParameters,
                              mediaType_in.subtype,
                              swapChain_inout);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::createSwapChain(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      if (release_device)
      {
        deviceHandle_inout->Release (); deviceHandle_inout = NULL;
      } // end IF
      return E_FAIL;
    } // end IF

    struct tagRECT source_rectangle = { 0, 0, width_out, height_out };
    updateDestinationRectangle (windowHandle_in,
                                source_rectangle,
                                destinationRectangle_out);
  } // end IF
  ACE_ASSERT (swapChain_inout);

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
tagRECT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::letterbox_rectangle (const struct tagRECT& source_in,
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
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::updateDestinationRectangle (HWND windowHandle_in,
                                                                                    const struct tagRECT& sourceRectangle_in,
                                                                                    struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::updateDestinationRectangle"));

  struct tagRECT destination_rectangle = destinationRectangle_;
  if (IsRectEmpty (&destinationRectangle_out))
  { ACE_ASSERT (window_);
    if (!::GetClientRect (windowHandle_in,
                          &destinationRectangle_out))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to GetClientRect(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      return;
    } // end IF
  } // end IF
  destinationRectangle_out = letterbox_rectangle (sourceRectangle_in,
                                                  destinationRectangle_out);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::createSwapChain (HWND windowHandle_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                         IDirect3DDevice9Ex* deviceHandle_in,
#else
                                                                         IDirect3DDevice9* deviceHandle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                         const struct _D3DPRESENT_PARAMETERS_& presentationParameters_in,
                                                                         REFGUID subType_in,
                                                                         IDirect3DSwapChain9*& swapChain_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::createSwapChain"));

  HRESULT result = S_OK;

  // initialize return value(s)
  if (swapChain_inout)
  {
    swapChain_inout->Release (); swapChain_inout = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (deviceHandle_in);

  struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters =
    presentationParameters_in;
  ACE_ASSERT (d3d_presentation_parameters.BackBufferWidth);
  ACE_ASSERT (d3d_presentation_parameters.BackBufferHeight);
  //d3d_presentation_parameters.Windowed = TRUE;
  //d3d_presentation_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
  d3d_presentation_parameters.hDeviceWindow = windowHandle_in;
  switch (mediaFramework_)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB24))
        d3d_presentation_parameters.BackBufferFormat = D3DFMT_R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MEDIASUBTYPE_ARGB32) ||
               InlineIsEqualGUID (subType_in, MEDIASUBTYPE_RGB32))
        d3d_presentation_parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
      else
      {
        d3d_presentation_parameters.BackBufferFormat = D3DFMT_UNKNOWN;
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: invalid/unknown media subtype (was: \"%s\"), continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (subType_in, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ())));
        //return E_FAIL;
      } // end ELSE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB24))
        d3d_presentation_parameters.BackBufferFormat = D3DFMT_R8G8B8;
      else if (InlineIsEqualGUID (subType_in, MFVideoFormat_RGB32))
        d3d_presentation_parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
      else
      {
        d3d_presentation_parameters.BackBufferFormat = D3DFMT_UNKNOWN;
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("%s: invalid/unknown media subtype (was: \"%s\"), continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (subType_in, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
        //return E_FAIL;
      } // end ELSE
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
  ACE_ASSERT (d3d_presentation_parameters.Flags & D3DPRESENTFLAG_VIDEO);
  ACE_ASSERT (d3d_presentation_parameters.Flags & D3DPRESENTFLAG_DEVICECLIP);
  ACE_ASSERT (d3d_presentation_parameters.Flags & D3DPRESENTFLAG_LOCKABLE_BACKBUFFER);
  //ACE_ASSERT (d3d_presentation_parameters.PresentationInterval == D3DPRESENT_INTERVAL_IMMEDIATE);
  //ACE_ASSERT (d3d_presentation_parameters.BackBufferCount == MODULE_DEV_CAM_DIRECT3D_DEFAULT_BACK_BUFFERS);

  result =
    deviceHandle_in->CreateAdditionalSwapChain (&d3d_presentation_parameters,
                                                &swapChain_inout);

  return result;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::initialize_Direct3DDevice (const struct Stream_Module_Device_Direct3DConfiguration& configuration_in,
                                                                                   HWND windowHandle_in,
                                                                                   const struct _AMMediaType& mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                                                                   IDirect3DDevice9Ex* deviceHandle_in,
#else
                                                                                   IDirect3DDevice9* deviceHandle_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                                                                   LONG& width_out,
                                                                                   LONG& height_out,
                                                                                   LONG& stride_out,
                                                                                   struct tagRECT& destinationRectangle_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize_Direct3DDevice"));

  HRESULT result = S_OK;

  result = setTransformation (mediaType_in.subtype);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::setTransformation(%s): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mediaType_in.subtype, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  format_ = (D3DFORMAT)mediaType_in.subtype.Data1;

  ACE_ASSERT (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo));
  ACE_ASSERT (mediaType_in.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
  struct tagVIDEOINFOHEADER* video_info_header_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_in.pbFormat);
  width_out = video_info_header_p->bmiHeader.biWidth;
  height_out = video_info_header_p->bmiHeader.biHeight;

  // *NOTE*: see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd318229(v=vs.85).aspx
  // *NOTE*: "...A bottom-up image has a negative stride, because stride is
  //         defined as the number of bytes need to move down a row of pixels,
  //         relative to the displayed image. YUV images should always be
  //         top-down, and any image that is contained in a Direct3D surface
  //         must be top-down. RGB images in system memory are usually bottom-up. ..."
  //         see also: https://docs.microsoft.com/en-us/windows/desktop/medfound/image-stride
  stride_out =
    -((((width_out * video_info_header_p->bmiHeader.biBitCount) + 31) & ~31) >> 3);

  ACE_ASSERT (configuration_in.presentationParameters.BackBufferWidth == width_out);
  ACE_ASSERT (configuration_in.presentationParameters.BackBufferHeight == height_out);
  result = createSwapChain (windowHandle_in,
                            deviceHandle_in,
                            configuration_in.presentationParameters,
                            mediaType_in.subtype,
                            swapChain_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::createSwapChain(): \"%s\", continuing\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (swapChain_);

  struct tagRECT source_rectangle = { 0, 0, width_out, height_out };
  updateDestinationRectangle (windowHandle_in,
                              source_rectangle,
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
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::setTransformation (REFGUID subType_in)
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
          typename SessionDataContainerType>
bool
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::isFormatSupported (REFGUID subType_in)
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
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType>::getFormat (DWORD index_in,
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
          typename SessionDataContainerType>
  Stream_Vis_DirectShow_Target_Direct3D_T<ACE_SYNCH_USE,
                                          TimePolicyType,
                                          ConfigurationType,
                                          ControlMessageType,
                                          DataMessageType,
                                          SessionMessageType,
                                          SessionDataType,
                                          SessionDataContainerType>::Stream_Vis_DirectShow_Target_Direct3D_T (ISTREAM_T* stream_in)
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
          typename SessionDataContainerType>
void
Stream_Vis_DirectShow_Target_Direct3D_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ConfigurationType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        SessionDataType,
                                        SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                      bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_DirectShow_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  bool reset_device = false;
  const typename DataMessageType::DATA_T& message_data_r =
    message_inout->getR ();
  IMediaSample* media_sample_p = NULL;
  BYTE* data_p = NULL;
  IDirect3DSurface9* d3d_surface_p = NULL;
  IDirect3DSurface9* d3d_backbuffer_p = NULL;
  bool unlock_rect = false;
  BYTE* scanline0_p = NULL;
  LONG stride = 0;
  struct _D3DLOCKED_RECT d3d_locked_rectangle;

  // sanity check(s)
  ACE_ASSERT (inherited::deviceHandle_);
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

  inherited::checkCooperativeLevel (inherited::deviceHandle_,
                                    reset_device);
  if (reset_device)
  {
    // sanity check(s)
    ACE_ASSERT (inherited::sessionData_);
    const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
    ACE_ASSERT (session_data_r.inputFormat);

    result = inherited::resetDevice (inherited::window_,
                                     inherited::presentationParameters_,
                                     inherited::fullscreenDisplayMode_,
                                     inherited::width_,
                                     inherited::height_,
                                     inherited::defaultStride_,
                                     *session_data_r.inputFormat,
                                     inherited::format_,
                                     inherited::deviceHandle_,
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
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle,
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
  unlock_rect = true;

  // *NOTE*: convert the frame (this also copies it to the Direct3D surface)
  try {
    inherited::transformation_ ((BYTE*)d3d_locked_rectangle.pBits,
                                d3d_locked_rectangle.Pitch,
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
    inherited::deviceHandle_->GetBackBuffer (0,
                                             0,
                                             D3DBACKBUFFER_TYPE_MONO,
                                             &d3d_backbuffer_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  //// color-fill the back buffer ?
  //result = deviceHandle_->ColorFill (d3d_backbuffer_p,
  //                                         NULL,
  //                                         D3DCOLOR_XRGB (0, 0, 0x80));
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
    inherited::deviceHandle_->StretchRect (d3d_surface_p,
                                           NULL,
                                           d3d_backbuffer_p,
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
  d3d_backbuffer_p->Release (); d3d_backbuffer_p = NULL;

  // present the frame
  result = inherited::deviceHandle_->Present (NULL,
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
  if (d3d_backbuffer_p)
    d3d_backbuffer_p->Release ();

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
          typename SessionDataContainerType>
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType>::Stream_Vis_MediaFoundation_Target_Direct3D_T (ISTREAM_T* stream_in)
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
          typename SessionDataContainerType>
void
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  const typename DataMessageType::DATA_T& message_data_r =
    message_inout->getR ();
  IMFMediaBuffer* media_buffer_p = NULL;
  BYTE* data_p = NULL;
  bool unlock_media_buffer = false;
  bool reset_device = false;
  bool unlock_rect = false;
  IDirect3DSurface9* d3d_surface_p = NULL;
  IDirect3DSurface9* d3d_backbuffer_p = NULL;
  BYTE* scanline0_p = NULL;
  LONG stride = 0;
  struct _D3DLOCKED_RECT d3d_locked_rectangle;

  // sanity check(s)
  ACE_ASSERT (inherited::deviceHandle_);
  ACE_ASSERT (inherited::swapChain_);
  ACE_ASSERT (inherited::transformation_);

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

  checkCooperativeLevel (inherited::deviceHandle_,
                         reset_device);
  if (reset_device)
  {
    // sanity check(s)
    ACE_ASSERT (inherited::sessionData_);
    const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
    ACE_ASSERT (session_data_r.inputFormat);

    result = resetDevice (*inherited::direct3DConfiguration_,
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
                          inherited::deviceHandle_,
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
  if (defaultStride_ < 0)
  {
    // Bottom-up orientation. Return a pointer to the start of the last row
    // *in memory*, which is the top row of the image
    scanline0_p = data_p + ::abs (inherited::defaultStride_) * (height_ - 1);
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
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle,
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
  unlock_rect = true;

  // *NOTE*: convert the frame (this also copies it to the Direct3D surface)
  try {
    inherited::transformation_ ((BYTE*)d3d_locked_rectangle.pBits,
                                d3d_locked_rectangle.Pitch,
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

  result =
    inherited::deviceHandle_->GetBackBuffer (0,
                                             0,
                                             D3DBACKBUFFER_TYPE_MONO,
                                             &d3d_backbuffer_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  //// color-fill the back buffer ?
  //result = deviceHandle_->ColorFill (d3d_backbuffer_p,
  //                                   NULL,
  //                                   D3DCOLOR_XRGB (0, 0, 0x80));
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
    inherited::deviceHandle_->StretchRect (d3d_surface_p,
                                           NULL,
                                           d3d_backbuffer_p,
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
  d3d_backbuffer_p->Release (); d3d_backbuffer_p = NULL;

  // present the frame
  result = inherited::deviceHandle_->Present (NULL,
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
  if (d3d_backbuffer_p)
    d3d_backbuffer_p->Release ();
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

  return;

continue_:
  return;
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  //int result = -1;
  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  ACE_ASSERT (inherited::sessionData_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (inherited::sessionData_->getR ());

      IMFTopology* topology_p = NULL;
      IMFMediaType* media_type_p = NULL;
      enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
        MFSESSION_GETFULLTOPOLOGY_CURRENT;
      TOPOID node_id = 0;

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
      if (inherited::deviceHandle_)
      {
        inherited::deviceHandle_->Release (); inherited::deviceHandle_ = NULL;
      } // end IF
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

      if (session_data_r.inputFormat)
        Stream_MediaFramework_DirectShow_Tools::deleteMediaType (session_data_r.inputFormat);
      ACE_ASSERT (!session_data_r.inputFormat);
      session_data_r.inputFormat =
        static_cast<struct _AMMediaType*> (CoTaskMemAlloc (sizeof (struct _AMMediaType)));
      if (!session_data_r.inputFormat)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory, continuing\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF
      ACE_OS::memset (session_data_r.inputFormat, 0, sizeof (struct _AMMediaType));
      ACE_ASSERT (!session_data_r.inputFormat->pbFormat);

      result_2 = MFInitAMMediaTypeFromMFMediaType (media_type_p,
                                                   GUID_NULL,
                                                   session_data_r.inputFormat);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to MFInitAMMediaTypeFromMFMediaType(): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      media_type_p->Release (); media_type_p = NULL;
      ACE_ASSERT (session_data_r.inputFormat);

      //HWND parent_window_handle = configuration_->window;
      if (!inherited::window_)
      {
        DWORD window_style = (WS_BORDER      |
                              WS_CAPTION     |
                              WS_CHILD       |
                              WS_MINIMIZEBOX |
                              WS_SYSMENU     |
                              WS_VISIBLE);
        inherited::window_ =
          CreateWindowEx (0,                             // dwExStyle
                          NULL,                          // lpClassName
                          ACE_TEXT_ALWAYS_CHAR ("EDIT"), // lpWindowName
                          window_style,                  // dwStyle
                          CW_USEDEFAULT,                 // x
                          SW_SHOWNA,                     // y
                          640,                           // nWidth
                          480,                           // nHeight
                          //parent_window_handle,          // hWndParent
                          NULL,
                          NULL,                          // hMenu
                          GetModuleHandle (NULL),        // hInstance
                          NULL);                         // lpParam
        if (!inherited::window_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        //BOOL result_3 =
        //  ShowWindow (inherited::configuration_->window, SW_SHOWNA);
        //ACE_UNUSED_ARG (result_3);
        inherited::closeWindow_ = true;
      } // end IF
      ACE_ASSERT (inherited::window_);

      //destinationRectangle_ = inherited::configuration_->area;
      SetRectEmpty (&(inherited::destinationRectangle_));
      if (session_data_r.direct3DDevice)
      {
        session_data_r.direct3DDevice->AddRef ();
        deviceHandle_ = session_data_r.direct3DDevice;

        ACE_ASSERT (inherited::direct3DConfiguration_);
        result_2 =
          inherited::initialize_Direct3DDevice (*inherited::direct3DConfiguration_,
                                                inherited::window_,
                                                *session_data_r.inputFormat,
                                                inherited::deviceHandle_,
                                                inherited::width_, inherited::height_,
                                                inherited::defaultStride_,
                                                inherited::destinationRectangle_);
        if (FAILED (result_2))
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
        if (!initialize_Direct3D (*inherited::direct3DConfiguration_,
                                  inherited::window_,
                                  *session_data_r.inputFormat,
                                  inherited::deviceHandle_,
                                  inherited::presentationParameters_,
                                  inherited::width_,
                                  inherited::height_,
                                  inherited::defaultStride_,
                                  inherited::destinationRectangle_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3D(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end ELSE

      topology_p->Release (); topology_p = NULL;

      goto continue_;

error:
      if (topology_p)
        topology_p->Release ();

      if (inherited::deviceHandle_)
      {
        inherited::deviceHandle_->Release (); inherited::deviceHandle_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (inherited::closeWindow_)
      { ACE_ASSERT (inherited::window_);
        inherited::closeWindow_ = false;
        if (!::CloseWindow (inherited::window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        inherited::window_ = NULL;
      } // end IF

      notify (STREAM_SESSION_MESSAGE_ABORT);

continue_:
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

      if (inherited::deviceHandle_)
      {
        inherited::deviceHandle_->Release (); inherited::deviceHandle_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (inherited::closeWindow_)
      {
        inherited::closeWindow_ = false;
        if (!::CloseWindow (inherited::window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        inherited::window_ = NULL;
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
          typename SessionDataContainerType>
HRESULT
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType>::get_default_stride (IMFMediaType* mediaType_in,
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
          typename SessionDataContainerType>
tagRECT
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType>::normalize_aspect_ratio (const struct tagRECT& rectangle_in,
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

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_MediaFoundation_Target_Direct3D_T<ACE_SYNCH_USE,
                                             TimePolicyType,
                                             ConfigurationType,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             SessionDataType,
                                             SessionDataContainerType>::update_destination_rectangle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_MediaFoundation_Target_Direct3D_T::update_destination_rectangle"));

  struct tagRECT source_rectangle =
    { 0, 0, inherited::width_, inherited::height_ };
  source_rectangle =
    normalize_aspect_ratio (source_rectangle, pixelAspectRatio_);

  struct tagRECT destination_rectangle = inherited::destinationRectangle_;
  if (IsRectEmpty (&destination_rectangle))
  { ACE_ASSERT (window_);
    if (!::GetClientRect (inherited::window_, &destination_rectangle))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to GetClientRect(): \"%s\", returning\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      return;
    } // end IF
  } // end IF
  inherited::destinationRectangle_ =
    inherited::letterbox_rectangle (source_rectangle,
                                    destination_rectangle);
}
