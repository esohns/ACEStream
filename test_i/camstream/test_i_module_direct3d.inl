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

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Test_I_DirectShow_Module_Direct3D_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::Test_I_DirectShow_Module_Direct3D_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Module_Direct3D_T::Test_I_DirectShow_Module_Direct3D_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Test_I_DirectShow_Module_Direct3D_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::~Test_I_DirectShow_Module_Direct3D_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Module_Direct3D_T::~Test_I_DirectShow_Module_Direct3D_T"));

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
Test_I_DirectShow_Module_Direct3D_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                  bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Module_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  const typename DataMessageType::DATA_T& message_data_r =
    message_inout->get ();
  IMFMediaBuffer* media_buffer_p = NULL;
  BYTE* data_p = NULL;
  bool unlock_media_buffer = false;
  IDirect3DSurface9* d3d_surface_p = NULL;
  IDirect3DSurface9* d3d_backbuffer_p = NULL;
  BYTE* scanline0_p = NULL;
  LONG stride = 0;

  // sanity check(s)
  ACE_ASSERT (inherited::adapter_);
  ACE_ASSERT (inherited::IDirect3DDevice9Ex_);
  ACE_ASSERT (inherited::IDirect3DSwapChain9_);

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
                  ACE_TEXT ("failed to IMFSample::GetBufferByIndex(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
                  ACE_TEXT ("failed to IMFMediaBuffer::Lock(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    unlock_media_buffer = true;
  } // end IF
  else
    data_p = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());

  result = test_cooperative_level ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_I_DirectShow_Module_Direct3D_T::test_cooperative_level(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  D3DLOCKED_RECT d3d_locked_rectangle;

  stride = inherited::defaultStride_;
  if (defaultStride_ < 0)
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

  result = IDirect3DSwapChain9_->GetBackBuffer (0,
                                                D3DBACKBUFFER_TYPE_MONO,
                                                &d3d_surface_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DSwapChain9::GetBackBuffer(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle,
                                    NULL,
                                    D3DLOCK_NOSYSLOCK);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DSurface9::LockRect(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  // *NOTE*: convert the frame (this also copies it to the Direct3D surface)
  try {
    adapter_ ((BYTE*)d3d_locked_rectangle.pBits,
              d3d_locked_rectangle.Pitch,
              scanline0_p,
              stride,
              width_,
              height_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in adapter function, continuing\n")));
    goto error;
  }

  result = d3d_surface_p->UnlockRect ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DSurface9::UnlockRect(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  if (unlock_media_buffer)
  {
    result = media_buffer_p->Unlock ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaBuffer::Unlock(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    unlock_media_buffer = false;
    media_buffer_p->Release ();
    media_buffer_p = NULL;
  } // end IF

  result = IDirect3DDevice9Ex_->GetBackBuffer (0,
                                               0,
                                               D3DBACKBUFFER_TYPE_MONO,
                                               &d3d_backbuffer_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //// color-fill the back buffer ?
  //result = IDirect3DDevice9Ex_->ColorFill (d3d_backbuffer_p,
  //                                         NULL,
  //                                         D3DCOLOR_XRGB (0, 0, 0x80));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IDirect3DDevice9Ex::ColorFill(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  // blit the frame
  result = IDirect3DDevice9Ex_->StretchRect (d3d_surface_p,
                                             NULL,
                                             d3d_backbuffer_p,
                                             //&destinationRectangle_,
                                             NULL,
                                             D3DTEXF_NONE);
  if (FAILED (result)) // D3DERR_INVALIDCALL: 0x8876086c
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::StretchRect(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  d3d_surface_p->Release ();
  d3d_surface_p = NULL;
  d3d_backbuffer_p->Release ();
  d3d_backbuffer_p = NULL;

  // present the frame
  result = IDirect3DDevice9Ex_->Present (NULL,
                                         NULL,
                                         NULL,
                                         NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::Present(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (unlock_media_buffer)
  {
    result = media_buffer_p->Unlock ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaBuffer::Unlock(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF
  if (media_buffer_p)
    media_buffer_p->Release ();

  if (d3d_surface_p)
    d3d_surface_p->Release ();
  if (d3d_backbuffer_p)
    d3d_backbuffer_p->Release ();

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
Test_I_DirectShow_Module_Direct3D_T<ACE_SYNCH_USE,
                                    TimePolicyType,
                                    ConfigurationType,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    SessionDataType,
                                    SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                    bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_DirectShow_Module_Direct3D_T::handleSessionMessage"));

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
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (session_data_container_r.get ());

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
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!IDirect3DDevice9Ex_);
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
                    ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      if (!Stream_Module_Device_Tools::getOutputFormat (topology_p,
                                                        node_id,
                                                        media_type_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
        goto error;
      } // end IF

      //HWND parent_window_handle = configuration_->window;
      if (!window_)
      {
        DWORD window_style = (WS_BORDER      |
                              WS_CAPTION     |
                              WS_CHILD       |
                              WS_MINIMIZEBOX |
                              WS_SYSMENU     |
                              WS_VISIBLE);
        window_ =
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
        if (!window_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CreateWindowEx(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        //BOOL result_3 = ShowWindow (configuration_->window, SW_SHOWNA);
        //ACE_UNUSED_ARG (result_3);
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_);

      //destinationRectangle_ = configuration_->area;
      SetRectEmpty (&destinationRectangle_);
      if (session_data_r.direct3DDevice)
      {
        session_data_r.direct3DDevice->AddRef ();
        IDirect3DDevice9Ex_ = session_data_r.direct3DDevice;

        result_2 = initialize_Direct3DDevice (window_,
                                              media_type_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Test_I_DirectShow_Module_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
          goto error;
        } // end IF
      } // end IF
      else
      {
        if (!initialize_Direct3D (window_,
                                  media_type_p,
                                  IDirect3DDevice9Ex_,
                                  presentationParameters_,
                                  adapter_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_Direct3D(), aborting\n")));
          goto error;
        } // end IF
      } // end ELSE

      topology_p->Release ();
      media_type_p->Release ();

      goto continue_;

error:
      if (topology_p)
        topology_p->Release ();
      if (media_type_p)
      media_type_p->Release ();

      if (IDirect3DDevice9Ex_)
      {
        IDirect3DDevice9Ex_->Release ();
        IDirect3DDevice9Ex_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      { ACE_ASSERT (window_);
        closeWindow_ = false;
        if (!::CloseWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CloseWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
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
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      if (IDirect3DDevice9Ex_)
      {
        IDirect3DDevice9Ex_->Release ();
        IDirect3DDevice9Ex_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      {
        closeWindow_ = false;
        if (!::CloseWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CloseWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
        window_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
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
Test_I_MediaFoundation_Module_Direct3D_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         SessionDataContainerType>::Test_I_MediaFoundation_Module_Direct3D_T ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Module_Direct3D_T::Test_I_MediaFoundation_Module_Direct3D_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Test_I_MediaFoundation_Module_Direct3D_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         SessionDataContainerType>::~Test_I_MediaFoundation_Module_Direct3D_T ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Module_Direct3D_T::~Test_I_MediaFoundation_Module_Direct3D_T"));

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
Test_I_MediaFoundation_Module_Direct3D_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                                       bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Module_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  const typename DataMessageType::DATA_T& message_data_r =
    message_inout->get ();
  IMFMediaBuffer* media_buffer_p = NULL;
  BYTE* data_p = NULL;
  bool unlock_media_buffer = false;
  IDirect3DSurface9* d3d_surface_p = NULL;
  IDirect3DSurface9* d3d_backbuffer_p = NULL;
  BYTE* scanline0_p = NULL;
  LONG stride = 0;

  // sanity check(s)
  ACE_ASSERT (adapter_);
  ACE_ASSERT (IDirect3DDevice9Ex_);
  ACE_ASSERT (IDirect3DSwapChain9_);

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
                  ACE_TEXT ("failed to IMFSample::GetBufferByIndex(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
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
                  ACE_TEXT ("failed to IMFMediaBuffer::Lock(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    unlock_media_buffer = true;
  } // end IF
  else
    data_p = reinterpret_cast<BYTE*> (message_inout->rd_ptr ());

  result = test_cooperative_level ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Test_I_MediaFoundation_Module_Direct3D_T::test_cooperative_level(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  D3DLOCKED_RECT d3d_locked_rectangle;

  stride = defaultStride_;
  if (defaultStride_ < 0)
  {
    // Bottom-up orientation. Return a pointer to the start of the last row
    // *in memory*, which is the top row of the image
    scanline0_p = data_p + abs (defaultStride_) * (height_ - 1);
  } // end IF
  else
  {
    // Top-down orientation. Return a pointer to the start of the buffer
    scanline0_p = data_p;
  } // end ELSE

  result = IDirect3DSwapChain9_->GetBackBuffer (0,
                                                D3DBACKBUFFER_TYPE_MONO,
                                                &d3d_surface_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DSwapChain9::GetBackBuffer(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  result = d3d_surface_p->LockRect (&d3d_locked_rectangle,
                                    NULL,
                                    D3DLOCK_NOSYSLOCK);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DSurface9::LockRect(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  // *NOTE*: convert the frame (this also copies it to the Direct3D surface)
  try {
    adapter_ ((BYTE*)d3d_locked_rectangle.pBits,
              d3d_locked_rectangle.Pitch,
              scanline0_p,
              stride,
              width_,
              height_);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in adapter function, continuing\n")));
    goto error;
  }

  result = d3d_surface_p->UnlockRect ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DSurface9::UnlockRect(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  if (unlock_media_buffer)
  {
    result = media_buffer_p->Unlock ();
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaBuffer::Unlock(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    unlock_media_buffer = false;
    media_buffer_p->Release ();
    media_buffer_p = NULL;
  } // end IF

  result = IDirect3DDevice9Ex_->GetBackBuffer (0,
                                               0,
                                               D3DBACKBUFFER_TYPE_MONO,
                                               &d3d_backbuffer_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::GetBackBuffer(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  //// color-fill the back buffer ?
  //result = IDirect3DDevice9Ex_->ColorFill (d3d_backbuffer_p,
  //                                         NULL,
  //                                         D3DCOLOR_XRGB (0, 0, 0x80));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IDirect3DDevice9Ex::ColorFill(): \"%s\", returning\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  goto error;
  //} // end IF

  // blit the frame
  result = IDirect3DDevice9Ex_->StretchRect (d3d_surface_p,
                                             NULL,
                                             d3d_backbuffer_p,
                                             //&destinationRectangle_,
                                             NULL,
                                             D3DTEXF_NONE);
  if (FAILED (result)) // D3DERR_INVALIDCALL: 0x8876086c
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::StretchRect(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  d3d_surface_p->Release ();
  d3d_surface_p = NULL;
  d3d_backbuffer_p->Release ();
  d3d_backbuffer_p = NULL;

  // present the frame
  result = IDirect3DDevice9Ex_->Present (NULL,
                                         NULL,
                                         NULL,
                                         NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IDirect3DDevice9Ex::Present(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  goto continue_;

error:
  if (unlock_media_buffer)
  {
    result = media_buffer_p->Unlock ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaBuffer::Unlock(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF
  if (media_buffer_p)
    media_buffer_p->Release ();

  if (d3d_surface_p)
    d3d_surface_p->Release ();
  if (d3d_backbuffer_p)
    d3d_backbuffer_p->Release ();

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
Test_I_MediaFoundation_Module_Direct3D_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                          bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MediaFoundation_Module_Direct3D_T::handleSessionMessage"));

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
      const SessionDataContainerType& session_data_container_r =
        message_inout->get ();
      SessionDataType& session_data_r =
        const_cast<SessionDataType&> (session_data_container_r.get ());

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
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      COM_initialized = true;

      // sanity check(s)
      ACE_ASSERT (!IDirect3DDevice9Ex_);
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
                    ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF
      if (!Stream_Module_Device_Tools::getOutputFormat (topology_p,
                                                        node_id,
                                                        media_type_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::getOutputFormat(), aborting\n")));
        goto error;
      } // end IF

      //HWND parent_window_handle = configuration_->window;
      if (!window_)
      {
        DWORD window_style = (WS_BORDER      |
                              WS_CAPTION     |
                              WS_CHILD       |
                              WS_MINIMIZEBOX |
                              WS_SYSMENU     |
                              WS_VISIBLE);
        window_ =
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
        if (!window_)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CreateWindowEx(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        //BOOL result_3 = ShowWindow (configuration_->window, SW_SHOWNA);
        //ACE_UNUSED_ARG (result_3);
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_);

      //destinationRectangle_ = configuration_->area;
      SetRectEmpty (&destinationRectangle_);
      if (session_data_r.direct3DDevice)
      {
        session_data_r.direct3DDevice->AddRef ();
        IDirect3DDevice9Ex_ = session_data_r.direct3DDevice;

        result_2 = initialize_Direct3DDevice (window_,
                                              media_type_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to Test_I_MediaFoundation_Module_Direct3D_T::initialize_Direct3DDevice(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
          goto error;
        } // end IF
      } // end IF
      else
      {
        if (!initialize_Direct3D (window_,
                                  media_type_p,
                                  IDirect3DDevice9Ex_,
                                  presentationParameters_,
                                  adapter_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_Direct3D(), aborting\n")));
          goto error;
        } // end IF
      } // end ELSE

      topology_p->Release ();
      media_type_p->Release ();

      goto continue_;

error:
      if (topology_p)
        topology_p->Release ();
      if (media_type_p)
      media_type_p->Release ();

      if (IDirect3DDevice9Ex_)
      {
        IDirect3DDevice9Ex_->Release ();
        IDirect3DDevice9Ex_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      { ACE_ASSERT (window_);
        closeWindow_ = false;
        if (!::CloseWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CloseWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
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
                    ACE_TEXT ("failed to CoInitializeEx(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        break;
      } // end IF
      COM_initialized = true;

      if (IDirect3DDevice9Ex_)
      {
        IDirect3DDevice9Ex_->Release ();
        IDirect3DDevice9Ex_ = NULL;
      } // end IF

      if (COM_initialized)
        CoUninitialize ();

      if (closeWindow_)
      {
        closeWindow_ = false;
        if (!::CloseWindow (window_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to CloseWindow(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Tools::error2String (::GetLastError ()).c_str ())));
        window_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}
