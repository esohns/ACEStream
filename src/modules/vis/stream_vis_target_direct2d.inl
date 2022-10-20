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

#include "amvideo.h"

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
#include "stream_lib_directdraw_tools.h"
#include "stream_lib_directshow_tools.h"
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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::Stream_Vis_Target_Direct2D_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , bitmap_ (NULL)
 , closeWindow_ (false)
 , factory_ (NULL)
 , pitch_ (0)
 , renderTarget_ (NULL)
 , resolution_ ()
 , window_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::Stream_Vis_Target_Direct2D_T"));

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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::~Stream_Vis_Target_Direct2D_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::~Stream_Vis_Target_Direct2D_T"));

  if (closeWindow_)
  { ACE_ASSERT (window_);
    if (unlikely (!::CloseWindow (window_)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
  } // end IF

  if (bitmap_)
    bitmap_->Release ();

  if (renderTarget_)
    renderTarget_->Release ();

  if (factory_)
    factory_->Release ();
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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (!bitmap_ || !pitch_ || !renderTarget_)
    return; // not initialized (yet)

  struct D2D_RECT_U rectangle_2;
  ACE_OS::memset (&rectangle_2, 0, sizeof (struct D2D_RECT_U));
  rectangle_2.right = resolution_.cx;
  rectangle_2.bottom = resolution_.cy;

  //struct D2D_MATRIX_3X2_F flip = D2D1::Matrix3x2F (-1, 0, 0, 1, 0, 0);
  //struct D2D_MATRIX_3X2_F translate = D2D1::Matrix3x2F::Translation (resolution_.cx, 0);

  //struct D2D_POINT_2F center = D2D1::Point2F (resolution_.cx / 2.0f, resolution_.cy / 2.0f);
  //struct D2D_MATRIX_3X2_F rotate = D2D1::Matrix3x2F::Rotation (180.0F, center);

  struct D2D_RECT_F rectangle_s;
  ACE_OS::memset (&rectangle_s, 0, sizeof (struct D2D_RECT_F));
  rectangle_s.right = static_cast<FLOAT> (resolution_.cx);
  rectangle_s.bottom = static_cast<FLOAT> (resolution_.cy);

  HRESULT result = bitmap_->CopyFromMemory (&rectangle_2,             // destination rectangle
                                            message_inout->rd_ptr (),
                                            pitch_);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID2D1Bitmap::CopyFromMemory(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  renderTarget_->BeginDraw ();

  //renderTarget_->SetTransform (flip);
  //renderTarget_->SetTransform (translate);

  //renderTarget_->SetTransform (rotate);

  renderTarget_->DrawBitmap (bitmap_,
                             rectangle_s,                           // destination rectangle
                             1.0F,                                  // opacity
                             D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
                             rectangle_s);                          // source rectangle

  //renderTarget_->SetTransform (D2D1::Matrix3x2F::Identity ());

  result = renderTarget_->EndDraw ();
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID2D1HwndRenderTarget::EndDraw(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  return;

error:
  notify (STREAM_SESSION_MESSAGE_ABORT);
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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  HRESULT result_2 = E_FAIL;

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
      HWND window_handle_p = NULL;
      bool COM_initialized = Common_Tools::initializeCOM ();

      // sanity check(s)
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      pitch_ =
        Stream_MediaFramework_DirectShow_Tools::toRowStride (media_type_s);

      if (window_)
      {
        ACE_ASSERT (IsWindow (window_));
      } // end IF
      else
      {
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
        window_ =
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
        if (unlikely (!window_))
        { // ERROR_INVALID_PARAMETER: 87
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
      } // end IF
      ACE_ASSERT (window_);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@\n"),
                  inherited::mod_->name (),
                  window_));
      if (unlikely (!initialize_Direct2D (window_,
                                          media_type_s)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to initialize_Direct2D(), aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (COM_initialized) Common_Tools::finalizeCOM ();

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

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

      if (COM_initialized) Common_Tools::finalizeCOM ();

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      bool COM_initialized = Common_Tools::initializeCOM ();

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

      if (COM_initialized) Common_Tools::finalizeCOM ();

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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::toggle"));

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::initialize"));

  // initialize COM ?
  static bool first_run = true;
  bool COM_initialized = false;
  if (likely (first_run))
  {
    first_run = false;
    COM_initialized = Common_Tools::initializeCOM ();
  } // end IF

  if (inherited::isInitialized_)
  {
    if (closeWindow_)
    { ACE_ASSERT (window_);
      closeWindow_ = false;
      if (unlikely (!::CloseWindow (window_)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    } // end IF
    if (bitmap_)
    {
      bitmap_->Release (); bitmap_ = NULL;
    } // end IF
    pitch_ = 0;
    if (renderTarget_)
    {
      renderTarget_->Release (); renderTarget_ = NULL;
    } // end IF
    if (factory_)
    {
      factory_->Release (); factory_ = NULL;
    } // end IF
    window_ = NULL;
  } // end IF

  // *TODO*: remove type inferences
  HRESULT result = D2D1CreateFactory (D2D1_FACTORY_TYPE_SINGLE_THREADED,
                                      &factory_);
  if (unlikely (FAILED (result) || !factory_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to D2D1CreateFactory(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  window_ = configuration_in.window;
  closeWindow_ = !window_;

  if (COM_initialized) Common_Tools::finalizeCOM ();

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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::initialize_Direct2D (HWND window_in,
                                                              const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::initialize_Direct3D"));

  // sanity check(s)
  ACE_ASSERT (factory_);
  ACE_ASSERT (window_in);

  Common_Image_Resolution_t resolution_s =
    Stream_MediaFramework_DirectShow_Tools::toResolution (mediaType_in);

  struct D2D1_RENDER_TARGET_PROPERTIES target_properties_s;
  target_properties_s.dpiX = 96.0F;
  target_properties_s.dpiY = 96.0F;
  target_properties_s.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
  target_properties_s.pixelFormat =
    D2D1::PixelFormat (DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_UNKNOWN);
  // D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_UNKNOWN);
  target_properties_s.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
  target_properties_s.usage = D2D1_RENDER_TARGET_USAGE_NONE;

  HRESULT result =
    factory_->CreateHwndRenderTarget (target_properties_s,//D2D1::RenderTargetProperties (),
                                      D2D1::HwndRenderTargetProperties (window_in,
                                                                        D2D1::SizeU (resolution_s.cx,
                                                                                     resolution_s.cy)),
                                      &renderTarget_
                                     );
  if (unlikely (FAILED (result) || !renderTarget_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID2D1Factory::CreateHwndRenderTarget(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  struct D2D1_BITMAP_PROPERTIES bitmap_properties_s;
  bitmap_properties_s.dpiX = 96.0F;
  bitmap_properties_s.dpiY = 96.0F;
  bitmap_properties_s.pixelFormat =
    D2D1::PixelFormat (DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
  // D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);

  result = renderTarget_->CreateBitmap (D2D1::SizeU (resolution_s.cx,
                                                     resolution_s.cy),
                                       bitmap_properties_s,//D2D1::BitmapProperties (),
                                       &bitmap_);
  if (unlikely (FAILED (result) || !bitmap_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID2D1HwndRenderTarget::CreateBitmap(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    renderTarget_->Release (); renderTarget_ = NULL;
    return false;
  } // end IF

  return true;
}
