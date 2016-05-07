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

#include "d3d9types.h"
#include "mfapi.h"
#include "mferror.h"
#include "mfidl.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_vis_defines.h"

// initialize statics
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
STREAM_VIS_TARGET_DIRECT3D_CONVERSION_T
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::formatConversions[] =
{
  { MFVideoFormat_RGB32, TransformImage_RGB32 },
  { MFVideoFormat_RGB24, TransformImage_RGB24 },
  { MFVideoFormat_YUY2,  TransformImage_YUY2 },
  { MFVideoFormat_NV12,  TransformImage_NV12 }
};

// *WARNING*: this needs to be set to ARRAYSIZE (formatConversions) !
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
const DWORD
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::formats = 4;
//ARRAYSIZE (Stream_Vis_Target_Direct3D_T<SessionMessageType,\
//                                        MessageType,\
//                                        ConfigurationType,\
//                                        SessionDataType,\
//                                        SessionDataContainerType>::formatConversions);

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::Stream_Vis_Target_Direct3D_T ()
 : inherited ()
 , configuration_ (NULL)
 , isInitialized_ (false)
 , width_ (0)
 , height_ (0)
 , defaultStride_ (0)
 , format_ (D3DFMT_UNKNOWN)
 , presentationParameters_ ()
 , transformation_ (NULL)
 , IDirect3DDevice9Ex_ (NULL)
 , IDirect3DSwapChain9_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::Stream_Vis_Target_Direct3D_T"));

  ACE_OS::memset (&presentationParameters_,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::~Stream_Vis_Target_Direct3D_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::~Stream_Vis_Target_Direct3D_T"));

  if (IDirect3DDevice9Ex_)
    IDirect3DDevice9Ex_->Release ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::handleDataMessage (MessageType*& message_inout,
                                                                           bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result = E_FAIL;
  typename const MessageType::DATA_T& message_data_r = message_inout->get ();

  // sanity check(s)
  ACE_ASSERT (transformation_);
  ACE_ASSERT (IDirect3DDevice9Ex_);
  ACE_ASSERT (IDirect3DSwapChain9_);
  ACE_ASSERT (message_data_r.sample);

  //DWORD count = 0;
  //result = message_data_r.sample->GetBufferCount (&count);
  //ACE_ASSERT (SUCCEEDED (result));
  //ACE_ASSERT (count == 1);
  IMFMediaBuffer* media_buffer_p = NULL;
  result = message_data_r.sample->GetBufferByIndex (0, &media_buffer_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFSample::GetBufferByIndex(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
  ACE_ASSERT (media_buffer_p);

  result = test_cooperative_level ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Vis_Target_Direct3D_T::test_cooperative_level(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  BYTE* scanline0_p = NULL;
  LONG stride = 0;
  D3DLOCKED_RECT d3d_locked_rectangle;
  IDirect3DSurface9* d3d_surface_p = NULL;
  IDirect3DSurface9* d3d_backbuffer_p = NULL;

  // Lock the video buffer. This method returns a pointer to the first scan
  // line in the image, and the stride in bytes.
  bool media_buffer_is_locked = false;
  BYTE* data_p = NULL;
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
  media_buffer_is_locked = true;

  stride = defaultStride_;
  if (defaultStride_ < 0)
  {
    // Bottom-up orientation. Return a pointer to the start of the
    // last row *in memory* which is the top row of the image.
    scanline0_p = data_p + abs (defaultStride_) * (height_ - 1);
  } // end IF
  else
  {
    // Top-down orientation. Return a pointer to the start of the
    // buffer.
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

  // Convert the frame. This also copies it to the Direct3D surface.
  try
  {
    transformation_ ((BYTE*)d3d_locked_rectangle.pBits,
                     d3d_locked_rectangle.Pitch,
                     scanline0_p,
                     stride,
                     width_,
                     height_);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in transformation function, continuing\n")));
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
  result = media_buffer_p->Unlock ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaBuffer::Unlock(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  media_buffer_is_locked = false;
  media_buffer_p->Release ();
  media_buffer_p = NULL;

  // Color fill the back buffer.
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

  // Blit the frame.
  result = IDirect3DDevice9Ex_->StretchRect (d3d_surface_p,
                                             NULL,
                                             d3d_backbuffer_p,
                                             &destinationRectangle_,
                                             D3DTEXF_LINEAR);
  if (FAILED (result))
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

  // Present the frame.
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
  if (media_buffer_is_locked)
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

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::handleSessionMessage"));

  int result = -1;
  HRESULT result_2 = E_FAIL;
  bool COM_initialized = false;

  // sanity check(s)
  ACE_ASSERT (configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
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
      ACE_ASSERT (!IDirect3DDevice9Ex_);
      ACE_ASSERT (session_data_r.format);

      if (session_data_r.direct3DDevice)
      {
        session_data_r.direct3DDevice->AddRef ();
        IDirect3DDevice9Ex_ = session_data_r.direct3DDevice;
      } // end IF
      else
      {
        if (!initialize_Direct3D (configuration_->window,
                                  configuration_->area,
                                  session_data_r.format,
                                  IDirect3DDevice9Ex_,
                                  presentationParameters_))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to initialize_Direct3D(), aborting\n")));
          goto error;
        } // end IF
      } // end ELSE

      goto continue_;

error:
      session_data_r.aborted = true;

continue_:
      if (COM_initialized)
        CoUninitialize ();

      break;
    }
    case STREAM_SESSION_END:
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

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize"));

  if (isInitialized_)
  {
    isInitialized_ = false;

    if (IDirect3DDevice9Ex_)
    {
      IDirect3DDevice9Ex_->Release ();
      IDirect3DDevice9Ex_ = NULL;
    } // end IF
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  isInitialized_ = true;

  return true;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
const ConfigurationType&
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::initialize_Direct3D (const HWND windowHandle_in,
                                                                             const struct tagRECT& windowArea_in,
                                                                             const IMFMediaType* mediaType_in,
                                                                             IDirect3DDevice9Ex*& IDirect3DDevice9Ex_out,
                                                                             struct _D3DPRESENT_PARAMETERS_& presentationParameters_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::initialize_Direct3D"));

  HRESULT result = E_FAIL;

  ACE_UNUSED_ARG (windowArea_in);

  // initialize return value(s)
  ACE_ASSERT (!IDirect3DDevice9Ex_out);
  ACE_OS::memset (&presentationParameters_out,
                  0,
                  sizeof (struct _D3DPRESENT_PARAMETERS_));

  IDirect3DDeviceManager9* direct3d_manager_p = NULL;
  UINT reset_token = 0;
  if (!Stream_Module_Device_Tools::getDirect3DDevice (windowHandle_in,
                                                      mediaType_in,
                                                      IDirect3DDevice9Ex_out,
                                                      presentationParameters_out,
                                                      direct3d_manager_p,
                                                      reset_token))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getDirect3DDevice(), aborting\n")));
    return false;
  } // end IF
  direct3d_manager_p->Release ();

  return true;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::test_cooperative_level ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::test_cooperative_level"));

  // sanity check(s)
  if (!IDirect3DDevice9Ex_)
    return E_FAIL;

  HRESULT result = IDirect3DDevice9Ex_->TestCooperativeLevel ();
  switch (result)
  {
    case D3D_OK:
      break;
    case D3DERR_DEVICELOST:
      result = S_OK;
      // *WARNING*: falls through here
    case D3DERR_DEVICENOTRESET:
      result = this->reset_device ();
      break;
    default:
      break;
  } // end SWITCH

  return result;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::reset_device ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::reset_device"));

  HRESULT result = S_OK;

  if (IDirect3DDevice9Ex_)
  {
    struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters =
      presentationParameters_;
    result = IDirect3DDevice9Ex_->Reset (&d3d_presentation_parameters);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IDirect3DDevice9Ex::Reset(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      IDirect3DDevice9Ex_->Release ();
      IDirect3DDevice9Ex_ = NULL;
      if (IDirect3DSwapChain9_)
      {
        IDirect3DSwapChain9_->Release ();
        IDirect3DSwapChain9_ = NULL;
      } // end IF
    } // end IF
  } // end IF

  if (!IDirect3DDevice9Ex_)
  {
    // sanity check(s)
    ACE_ASSERT (configuration_);
    ACE_ASSERT (sessionData_);
    ACE_ASSERT (sessionData_->format);

    if (!this->initialize_Direct3D (configuration_->window,
                                    configuration_->area,
                                    sessionData_->format,
                                    IDirect3DDevice9Ex_,
                                    presentationParameters_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Vis_Target_Direct3D_T::initialize_Direct3D(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return E_FAIL;
    } // end IF
  } // end IF

  if (!IDirect3DSwapChain9_ && (format_ != D3DFMT_UNKNOWN))
  {
    result = this->create_swap_chains (configuration_->window,
                                       width_, height_);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Vis_Target_Direct3D_T::create_swap_chains(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return E_FAIL;
    } // end IF

    this->update_destination_rectangle ();
  } // end IF

  return result;
}

//-------------------------------------------------------------------
// Takes a src rectangle and constructs the largest possible 
// destination rectangle within the specifed destination rectangle 
// such thatthe video maintains its current shape.
//
// This function assumes that pels are the same shape within both the 
// source and destination rectangles.
//
//-------------------------------------------------------------------
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
RECT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::letterbox_rectangle (const RECT& source_in,
                                                                             const RECT& destination_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::letterbox_rectangle"));

  // figure out src/dest scale ratios
  int iSrcWidth = (source_in.right - source_in.left);
  int iSrcHeight = (source_in.bottom - source_in.top);

  int iDstWidth = (destination_in.right - destination_in.left);
  int iDstHeight = (destination_in.bottom - destination_in.top);

  int iDstLBWidth;
  int iDstLBHeight;

  if (MulDiv (iSrcWidth,
              iDstHeight,
              iSrcHeight) <= iDstWidth)
  {
    // Column letter boxing ("pillar box")
    iDstLBWidth = MulDiv (iDstHeight,
                          iSrcWidth,
                          iSrcHeight);
    iDstLBHeight = iDstHeight;
  } // end IF
  else
  {
    // Row letter boxing.
    iDstLBWidth = iDstWidth;
    iDstLBHeight = MulDiv (iDstWidth,
                           iSrcHeight,
                           iSrcWidth);
  } // end ELSE

  // Create a centered rectangle within the current destination rect
  RECT result;
  LONG left = destination_in.left + ((iDstWidth - iDstLBWidth) / 2);
  LONG top = destination_in.top + ((iDstHeight - iDstLBHeight) / 2);

  SetRect (&result, left, top, left + iDstLBWidth, top + iDstLBHeight);

  return result;
}

//-----------------------------------------------------------------------------
// Converts a rectangle from the source's pixel aspect ratio (PAR) to 1:1 PAR.
// Returns the corrected rectangle.
//
// For example, a 720 x 486 rect with a PAR of 9:10, when converted to 1x1 PAR,  
// is stretched to 720 x 540. 
//-----------------------------------------------------------------------------
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
RECT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::correct_aspect_ratio (const RECT& source_in,
                                                                              const MFRatio& sourcePixelAspectRatio_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::correct_aspect_ratio"));

  // Start with a rectangle the same size as src, but offset to the origin (0,0).
  RECT result =
    { 0, 0,
      source_in.right - source_in.left, source_in.bottom - source_in.top };

  if ((sourcePixelAspectRatio_in.Numerator != 1)  ||
      (sourcePixelAspectRatio_in.Denominator != 1))
  {
    // Correct for the source's PAR.
    if (sourcePixelAspectRatio_in.Numerator > sourcePixelAspectRatio_in.Denominator)
    {
      // The source has "wide" pixels, so stretch the width.
      result.right = MulDiv (result.right,
                             sourcePixelAspectRatio_in.Numerator,
                             sourcePixelAspectRatio_in.Denominator);
    } // end IF
    else if (sourcePixelAspectRatio_in.Numerator < sourcePixelAspectRatio_in.Denominator)
    {
      // The source has "tall" pixels, so stretch the height.
      result.bottom = MulDiv (result.bottom,
                              sourcePixelAspectRatio_in.Denominator,
                              sourcePixelAspectRatio_in.Numerator);
    } // end ELSE IF
    // else: PAR is 1:1, which is a no-op.
  } // end IF

  return result;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::update_destination_rectangle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::update_destination_rectangle"));

  // sanity check(s)
  ACE_ASSERT (configuration_);
  
  RECT client_rectangle = { 0, 0, 0, 0 };
  if (!GetClientRect (configuration_->window, &client_rectangle))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetClientRect(): \"%s\", returning\n")));
    return;
  } // end IF

  RECT source_rectangle = { 0, 0, width_, height_ };
  source_rectangle = correct_aspect_ratio (source_rectangle, pixelAspectRatio_);

  destinationRectangle_ = letterbox_rectangle (source_rectangle,
                                               client_rectangle);
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::create_swap_chains (const HWND windowHandle_in,
                                                                            UINT32 width_in,
                                                                            UINT32 height_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::create_swap_chains"));

  HRESULT result = S_OK;

  // initialize return value(s)
  if (IDirect3DSwapChain9_)
  {
    IDirect3DSwapChain9_->Release ();
    IDirect3DSwapChain9_ = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (IDirect3DDevice9Ex_);

  struct _D3DPRESENT_PARAMETERS_ d3d_presentation_parameters = { 0 };
  d3d_presentation_parameters.BackBufferWidth = width_in;
  d3d_presentation_parameters.BackBufferHeight = height_in;
  d3d_presentation_parameters.Windowed = TRUE;
  d3d_presentation_parameters.SwapEffect = D3DSWAPEFFECT_FLIP;
  d3d_presentation_parameters.hDeviceWindow = windowHandle_in;
  d3d_presentation_parameters.BackBufferFormat = D3DFMT_X8R8G8B8;
  d3d_presentation_parameters.Flags =
    (D3DPRESENTFLAG_VIDEO              |
     D3DPRESENTFLAG_DEVICECLIP         |
     D3DPRESENTFLAG_LOCKABLE_BACKBUFFER);
  d3d_presentation_parameters.PresentationInterval =
    D3DPRESENT_INTERVAL_IMMEDIATE;
  d3d_presentation_parameters.BackBufferCount =
    MODULE_DEV_CAM_MEDIAFOUNDATION_DEFAULT_BACK_BUFFERS;

  result =
    IDirect3DDevice9Ex_->CreateAdditionalSwapChain (&d3d_presentation_parameters,
                                                    &IDirect3DSwapChain9_);

  return result;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::get_default_stride (IMFMediaType* mediaType_in,
                                                                            LONG* stride_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::get_default_stride"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (stride_out);

  // initialize return value(s)
  *stride_out = 0;

  HRESULT result = mediaType_in->GetUINT32 (MF_MT_DEFAULT_STRIDE,
                                            (UINT32*)stride_out);
  if (SUCCEEDED (result))
    return result;

  struct _GUID sub_type = GUID_NULL;
  UINT32 width = 0;
  UINT32 height = 0;

  result = mediaType_in->GetGUID (MF_MT_SUBTYPE, &sub_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  result = MFGetAttributeSize (mediaType_in,
                                MF_MT_FRAME_SIZE,
                                &width, &height);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  result = MFGetStrideForBitmapInfoHeader (sub_type.Data1, width, stride_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFGetStrideForBitmapInfoHeader(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  result = mediaType_in->SetUINT32 (MF_MT_DEFAULT_STRIDE,
                                    *(UINT32*)stride_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_DEFAULT_STRIDE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  return S_OK;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::set_video_type (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::set_video_type"));

  HRESULT result = S_OK;

  struct _GUID GUID_s = { 0 };
  result = mediaType_in->GetGUID (MF_MT_SUBTYPE, &GUID_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  result = this->set_transformation (GUID_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Vis_Target_Direct3D_T::set_transformation(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF
  format_ = (D3DFORMAT)GUID_s.Data1;

  result = MFGetAttributeSize (mediaType_in,
                               MF_MT_FRAME_SIZE,
                               &width_, &height_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  interlace_ =
    (MFVideoInterlaceMode)MFGetAttributeUINT32 (mediaType_in,
                                                MF_MT_INTERLACE_MODE,
                                                MFVideoInterlace_Unknown);

  result = get_default_stride (mediaType_in, &defaultStride_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Vis_Target_Direct3D_T::get_default_stride(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  pixelAspectRatio_ = { 0 };
  result = MFGetAttributeRatio (mediaType_in,
                                MF_MT_PIXEL_ASPECT_RATIO,
                                (UINT32*)&pixelAspectRatio_.Numerator,
                                (UINT32*)&pixelAspectRatio_.Denominator);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_PIXEL_ASPECT_RATIO): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    pixelAspectRatio_.Numerator = pixelAspectRatio_.Denominator = 1;
  } // end IF

  result = create_swap_chains ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Vis_Target_Direct3D_T::create_swap_chains(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  update_destination_rectangle ();

  return S_OK;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::set_transformation (REFGUID subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::set_transformation"));

  transformation_ = NULL;

  for (int i = 0; i < OWN_TYPE_T::formats; ++i)
    if (subType_in == OWN_TYPE_T::formatConversions[i].subType)
    {
      transformation_ = OWN_TYPE_T::formatConversions[i].transform;
      return S_OK;
    } // end IF

  return MF_E_INVALIDMEDIATYPE;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::is_supported (REFGUID subType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::is_supported"));

  for (int i = 0; i < OWN_TYPE_T::formats; ++i)
    if (subType_in == OWN_TYPE_T::formatConversions[i].subtype)
      return true;

  return false;
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
HRESULT
Stream_Vis_Target_Direct3D_T<SessionMessageType,
                             MessageType,
                             ConfigurationType,
                             SessionDataType,
                             SessionDataContainerType>::get_format (DWORD index_in,
                                                                    struct _GUID* subType_out) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D_T::get_format"));

  // sanity check(s)
  ACE_ASSERT (subType_out);

  if (index < OWN_TYPE_T::formats)
  {
    *subType_out = OWN_TYPE_T::formatConversions[i].subtype;
    return S_OK;
  } // end IF

  return MF_E_NO_MORE_TYPES;
}

//////////////////////////////////////////

__forceinline BYTE Clip (int clr)
{
  return (BYTE)(clr < 0 ? 0 : (clr > 255 ? 255 : clr));
}

__forceinline RGBQUAD ConvertYCrCbToRGB (int y,
                                         int cr,
                                         int cb)
{
  RGBQUAD rgbq;

  int c = y - 16;
  int d = cb - 128;
  int e = cr - 128;

  rgbq.rgbRed = Clip ((298 * c + 409 * e + 128) >> 8);
  rgbq.rgbGreen = Clip ((298 * c - 100 * d - 208 * e + 128) >> 8);
  rgbq.rgbBlue = Clip ((298 * c + 516 * d + 128) >> 8);

  return rgbq;
}

//-------------------------------------------------------------------
// TransformImage_RGB24 
//
// RGB-24 to RGB-32
//-------------------------------------------------------------------
void TransformImage_RGB24 (BYTE*       pDest,
                           LONG        lDestStride,
                           const BYTE* pSrc,
                           LONG        lSrcStride,
                           DWORD       dwWidthInPixels,
                           DWORD       dwHeightInPixels)
{
  for (DWORD y = 0; y < dwHeightInPixels; y++)
  {
    RGBTRIPLE *pSrcPel = (RGBTRIPLE*)pSrc;
    DWORD *pDestPel = (DWORD*)pDest;

    for (DWORD x = 0; x < dwWidthInPixels; x++)
      pDestPel[x] = D3DCOLOR_XRGB (pSrcPel[x].rgbtRed,
                                   pSrcPel[x].rgbtGreen,
                                   pSrcPel[x].rgbtBlue);

    pSrc += lSrcStride;
    pDest += lDestStride;
  } // end FOR
}

//-------------------------------------------------------------------
// TransformImage_RGB32
//
// RGB-32 to RGB-32 
//
// Note: This function is needed to copy the image from system
// memory to the Direct3D surface.
//-------------------------------------------------------------------
void TransformImage_RGB32 (BYTE*       pDest,
                           LONG        lDestStride,
                           const BYTE* pSrc,
                           LONG        lSrcStride,
                           DWORD       dwWidthInPixels,
                           DWORD       dwHeightInPixels)
{
  HRESULT result = MFCopyImage (pDest,
                                lDestStride,
                                pSrc,
                                lSrcStride,
                                dwWidthInPixels * 4,
                                dwHeightInPixels);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCopyImage(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
}

//-------------------------------------------------------------------
// TransformImage_YUY2 
//
// YUY2 to RGB-32
//-------------------------------------------------------------------
void TransformImage_YUY2 (BYTE*       pDest,
                          LONG        lDestStride,
                          const BYTE* pSrc,
                          LONG        lSrcStride,
                          DWORD       dwWidthInPixels,
                          DWORD       dwHeightInPixels)
{
  for (DWORD y = 0; y < dwHeightInPixels; y++)
  {
    RGBQUAD *pDestPel = (RGBQUAD*)pDest;
    WORD    *pSrcPel = (WORD*)pSrc;

    for (DWORD x = 0; x < dwWidthInPixels; x += 2)
    {
      // Byte order is U0 Y0 V0 Y1
      int y0 = (int)LOBYTE (pSrcPel[x]);
      int u0 = (int)HIBYTE (pSrcPel[x]);
      int y1 = (int)LOBYTE (pSrcPel[x + 1]);
      int v0 = (int)HIBYTE (pSrcPel[x + 1]);

      pDestPel[x] = ConvertYCrCbToRGB (y0, v0, u0);
      pDestPel[x + 1] = ConvertYCrCbToRGB (y1, v0, u0);
    } // end FOR

    pSrc += lSrcStride;
    pDest += lDestStride;
  } // end FOR
}

//-------------------------------------------------------------------
// TransformImage_NV12
//
// NV12 to RGB-32
//-------------------------------------------------------------------
void TransformImage_NV12 (BYTE* pDst,
                          LONG dstStride,
                          const BYTE* pSrc,
                          LONG srcStride,
                          DWORD dwWidthInPixels,
                          DWORD dwHeightInPixels)
{
  const BYTE* lpBitsY = pSrc;
  const BYTE* lpBitsCb = lpBitsY + (dwHeightInPixels * srcStride);;
  const BYTE* lpBitsCr = lpBitsCb + 1;

  for (UINT y = 0; y < dwHeightInPixels; y += 2)
  {
    const BYTE* lpLineY1 = lpBitsY;
    const BYTE* lpLineY2 = lpBitsY + srcStride;
    const BYTE* lpLineCr = lpBitsCr;
    const BYTE* lpLineCb = lpBitsCb;

    LPBYTE lpDibLine1 = pDst;
    LPBYTE lpDibLine2 = pDst + dstStride;

    for (UINT x = 0; x < dwWidthInPixels; x += 2)
    {
      int  y0 = (int)lpLineY1[0];
      int  y1 = (int)lpLineY1[1];
      int  y2 = (int)lpLineY2[0];
      int  y3 = (int)lpLineY2[1];
      int  cb = (int)lpLineCb[0];
      int  cr = (int)lpLineCr[0];

      RGBQUAD r = ConvertYCrCbToRGB (y0, cr, cb);
      lpDibLine1[0] = r.rgbBlue;
      lpDibLine1[1] = r.rgbGreen;
      lpDibLine1[2] = r.rgbRed;
      lpDibLine1[3] = 0; // Alpha

      r = ConvertYCrCbToRGB (y1, cr, cb);
      lpDibLine1[4] = r.rgbBlue;
      lpDibLine1[5] = r.rgbGreen;
      lpDibLine1[6] = r.rgbRed;
      lpDibLine1[7] = 0; // Alpha

      r = ConvertYCrCbToRGB (y2, cr, cb);
      lpDibLine2[0] = r.rgbBlue;
      lpDibLine2[1] = r.rgbGreen;
      lpDibLine2[2] = r.rgbRed;
      lpDibLine2[3] = 0; // Alpha

      r = ConvertYCrCbToRGB (y3, cr, cb);
      lpDibLine2[4] = r.rgbBlue;
      lpDibLine2[5] = r.rgbGreen;
      lpDibLine2[6] = r.rgbRed;
      lpDibLine2[7] = 0; // Alpha

      lpLineY1 += 2;
      lpLineY2 += 2;
      lpLineCr += 2;
      lpLineCb += 2;

      lpDibLine1 += 8;
      lpDibLine2 += 8;
    } // end FOR

    pDst += (2 * dstStride);
    lpBitsY += (2 * srcStride);
    lpBitsCr += srcStride;
    lpBitsCb += srcStride;
  } // end FOR
}

//////////////////////////////////////////
