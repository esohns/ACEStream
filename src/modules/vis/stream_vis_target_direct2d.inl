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
#include "strmif.h"

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
 , factory_ (NULL)
 , format_ (GUID_NULL)
 , pitch_ (0)
 , renderTarget_ (NULL)
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

  if (factory_)
    factory_->Release ();

  if (renderTarget_)
    renderTarget_->Release ();

  if (bitmap_)
    bitmap_->Release ();
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
  ACE_ASSERT (bitmap_ && pitch_ && renderTarget_);

  struct D2D_RECT_U rectangle_2;
  ACE_OS::memset (&rectangle_2, 0, sizeof (struct D2D_RECT_U));
  rectangle_2.right = inherited::resolution_.cx;
  rectangle_2.bottom = inherited::resolution_.cy;

  //struct D2D_MATRIX_3X2_F flip = D2D1::Matrix3x2F (-1, 0, 0, 1, 0, 0);
  //struct D2D_MATRIX_3X2_F translate = D2D1::Matrix3x2F::Translation (resolution_.cx, 0);

  //struct D2D_POINT_2F center = D2D1::Point2F (resolution_.cx / 2.0f, resolution_.cy / 2.0f);
  //struct D2D_MATRIX_3X2_F rotate = D2D1::Matrix3x2F::Rotation (180.0F, center);

  HRESULT result =
    bitmap_->CopyFromMemory (&rectangle_2,             // destination rectangle
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

  struct D2D_RECT_F rectangle_s;
  ACE_OS::memset (&rectangle_s, 0, sizeof (struct D2D_RECT_F));
  rectangle_s.right = static_cast<FLOAT> (inherited::resolution_.cx);
  rectangle_s.bottom = static_cast<FLOAT> (inherited::resolution_.cy);

  renderTarget_->BeginDraw ();

  //renderTarget_->SetTransform (flip);
  //renderTarget_->SetTransform (translate);

  //renderTarget_->SetTransform (rotate);

  renderTarget_->DrawBitmap (bitmap_,
                             rectangle_s,                           // destination rectangle
                             1.0f,                                  // opacity
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
    case STREAM_SESSION_MESSAGE_ABORT:
      goto end;
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
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);
      inherited::resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      format_ = media_type_s.subtype;
      pitch_ =
        Stream_MediaFramework_DirectShow_Tools::toRowStride (media_type_s);

      // start message pump ?
      if (!inherited::window_)
      { // need a window/message pump
        inherited::threadCount_ = 1;
        inherited::start (NULL);
        inherited::threadCount_ = 0;

        while (!inherited::thr_count_ || !inherited::window_ || !renderTarget_ || !bitmap_); // *TODO*: never do this
      } // end IF
      else
      {
        if (unlikely (!initialize_Direct2D (inherited::window_,
                                            format_)))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to initialize_Direct2D(), aborting\n"),
                      inherited::mod_->name ()));
          goto error;
        } // end IF
      } // end ELSE

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);
      if (COM_initialized)
        Common_Tools::finalizeCOM ();

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
end:
      if (inherited::window_)
      {
        inherited::notify_ = false;

        if (unlikely (!PostMessage (inherited::window_, WM_QUIT, 0, 0)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to PostMessage(%@): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::window_,
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      } // end IF

      if (inherited::thr_count_)
        inherited::wait (true); // wait for message queue ?

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

  if (COM_initialized)
    Common_Tools::finalizeCOM ();

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
                                                              REFGUID format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::initialize_Direct2D"));

  // sanity check(s)
  ACE_ASSERT (factory_);
  ACE_ASSERT (window_in);

  struct D2D1_RENDER_TARGET_PROPERTIES target_properties_s;
  ACE_OS::memset (&target_properties_s, 0, sizeof (struct D2D1_RENDER_TARGET_PROPERTIES));
  target_properties_s.dpiX = 96.0F;
  target_properties_s.dpiY = 96.0F;
  target_properties_s.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
  target_properties_s.pixelFormat =
    D2D1::PixelFormat (Stream_MediaFramework_DirectDraw_Tools::toFormat_2 (format_in),
                       D2D1_ALPHA_MODE_UNKNOWN);
  // D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_UNKNOWN);
  target_properties_s.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
  target_properties_s.usage = D2D1_RENDER_TARGET_USAGE_NONE;

  HRESULT result =
    factory_->CreateHwndRenderTarget (target_properties_s,//D2D1::RenderTargetProperties (),
                                      D2D1::HwndRenderTargetProperties (window_in,
                                                                        D2D1::SizeU (inherited::resolution_.cx,
                                                                                     inherited::resolution_.cy)),
                                      &renderTarget_);
  if (unlikely (FAILED (result) || !renderTarget_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID2D1Factory::CreateHwndRenderTarget(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  struct D2D1_BITMAP_PROPERTIES bitmap_properties_s;
  ACE_OS::memset (&bitmap_properties_s, 0, sizeof (struct D2D1_BITMAP_PROPERTIES));
  bitmap_properties_s.dpiX = 96.0F;
  bitmap_properties_s.dpiY = 96.0F;
  bitmap_properties_s.pixelFormat =
    D2D1::PixelFormat (DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
  // D2D1::PixelFormat (DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);

  result = renderTarget_->CreateBitmap (D2D1::SizeU (inherited::resolution_.cx,
                                                     inherited::resolution_.cy),
                                        bitmap_properties_s,
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
Stream_Vis_Target_Direct2D_T<ACE_SYNCH_USE,
                             TimePolicyType,
                             ConfigurationType,
                             ControlMessageType,
                             DataMessageType,
                             SessionMessageType,
                             SessionDataType,
                             SessionDataContainerType,
                             MediaType>::svc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct2D_T::svc"));

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

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window handle: 0x%@\n"),
              inherited::mod_->name (),
              inherited::window_));

  if (unlikely (!initialize_Direct2D (inherited::window_,
                                      format_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize_Direct2D(), aborting\n"),
                inherited::mod_->name ()));
    DestroyWindow (inherited::window_); inherited::window_ = NULL;
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return -1;
  } // end IF

  inherited::notify_ = true;

  BOOL result;
  struct tagMSG message_s;
  bool running_b = true;
  while ((result = GetMessage (&message_s,
                              inherited::window_,
                              0,
                              0) != 0) && running_b)
  {
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to GetMessage(%@): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  inherited::window_,
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      break;
    } // end IF

    TranslateMessage (&message_s);
    switch (message_s.message)
    {
      case WM_CLOSE:
      case WM_DESTROY:
        PostMessage (inherited::window_, WM_QUIT, 0, 0);
        break;
      case WM_QUIT:
        running_b = false;
        break;
      default:
        break;
    } // end SWITCH
    DispatchMessage (&message_s);

    if (!running_b)
      break;
  } // end WHILE
  DestroyWindow (inherited::window_); inherited::window_ = NULL;

  if (unlikely (inherited::notify_))
  { inherited::notify_ = false;
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: spawned thread (id: %t, group id: %d) leaving\n"),
              ACE_TEXT (STREAM_VIS_RENDERER_WINDOW_DEFAULT_MESSAGE_PUMP_THREAD_NAME),
              STREAM_MODULE_TASK_GROUP_ID));

  return 0;
}
