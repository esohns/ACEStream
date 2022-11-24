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

#include "d3dcommon.h"
#include "d3dcompiler.h"
#include "dxgi.h"
#include "dxgiformat.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_macros.h"

#include "stream_lib_directdraw_tools.h"
#include "stream_lib_directshow_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::Stream_Vis_Target_Direct3D11_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , context_ (NULL)
 , device_ (NULL)
 , format_ (GUID_NULL)
 , renderTargetView_ (NULL)
 , swapChain_ (NULL)
 , vertexShader_ (NULL)
 , pixelShader_ (NULL)
 , inputLayout_ (NULL)
 , vertexBuffer_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::Stream_Vis_Target_Direct3D11_T"));

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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::~Stream_Vis_Target_Direct3D11_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::~Stream_Vis_Target_Direct3D11_T"));

  if (swapChain_)
    swapChain_->Release ();
  if (renderTargetView_)
    renderTargetView_->Release ();
  if (device_)
    device_->Release ();
  if (context_)
    context_->Release ();
  if (vertexShader_)
    vertexShader_->Release ();
  if (pixelShader_)
    pixelShader_->Release ();
  if (inputLayout_)
    inputLayout_->Release ();
  if (vertexBuffer_)
    vertexBuffer_->Release ();
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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  if (!context_ || !swapChain_ || !texture_)
    return; // not ready (yet)

  struct D3D11_MAPPED_SUBRESOURCE mapped_subresource_s;
  ACE_OS::memset (&mapped_subresource_s, 0, sizeof (struct D3D11_MAPPED_SUBRESOURCE));
  HRESULT result = context_->Map (texture_,
                                  0,
                                  D3D11_MAP_WRITE_DISCARD,
                                  0,
                                  &mapped_subresource_s);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_OS::memcpy (mapped_subresource_s.pData,
                  message_inout->rd_ptr (),
                  message_inout->length ());
  context_->Unmap (texture_, 0);

  UINT vertex_count = 6;
  context_->Draw (vertex_count, 0);

  result = swapChain_->Present (1, 0);
  ACE_ASSERT (SUCCEEDED (result));
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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::handleSessionMessage"));

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
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited::getMediaType (session_data_r.formats.back (),
                               STREAM_MEDIATYPE_VIDEO,
                               media_type_s);

      inherited::resolution_ =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      format_ = media_type_s.subtype;

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
      { ACE_ASSERT (false); // *TODO*
      } // end ELSE

      break;

//error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
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
void
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::toggle"));

  ACE_ASSERT (false); // *TODO*
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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::initialize"));

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
    if (swapChain_)
      swapChain_->Release ();
    swapChain_ = NULL;
    if (renderTargetView_)
      renderTargetView_->Release ();
    renderTargetView_ = NULL;
    if (device_)
      device_->Release ();
    device_ = NULL;
    if (context_)
      context_->Release ();
    context_ = NULL;
    if (vertexShader_)
      vertexShader_->Release ();
    vertexShader_ = NULL;
    if (pixelShader_)
      pixelShader_->Release ();
    pixelShader_ = NULL;
    if (inputLayout_)
      inputLayout_->Release ();
    inputLayout_ = NULL;
    if (vertexBuffer_)
      vertexBuffer_->Release ();
    vertexBuffer_ = NULL;
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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::compileShaders (const std::string& filename_in,
                                                           ID3D11Device* device_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::compileShaders"));

  // sanity check(s)
  ACE_ASSERT (device_in);
  ACE_ASSERT (!vertexShader_);
  ACE_ASSERT (!inputLayout_);
  ACE_ASSERT (!pixelShader_);

  UINT flags_i = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined (_DEBUG) || defined (DEBUG)
  flags_i |= D3DCOMPILE_DEBUG; // add more debug output
#endif // _DEBUG || DEBUG
  ID3DBlob* blob_p = NULL;
  ID3DBlob* error_blob_p = NULL;

  // COMPILE VERTEX SHADER
  HRESULT result = D3DCompileFromFile (ACE_TEXT_ALWAYS_WCHAR (filename_in.c_str ()),
                                       NULL,
                                       D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                       ACE_TEXT_ALWAYS_CHAR ("vs_main"),
                                       ACE_TEXT_ALWAYS_CHAR ("vs_5_0"),
                                       flags_i,
                                       0,
                                       &blob_p,
                                       &error_blob_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to D3DCompileFromFile(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (filename_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    if (error_blob_p)
    {
      OutputDebugStringA ((char*)error_blob_p->GetBufferPointer ());
      error_blob_p->Release ();
    } // end IF
    if (blob_p)
    { blob_p->Release (); blob_p = NULL; } // end IF
    return false;
  } // end IF
  ACE_ASSERT (blob_p);

  result = device_in->CreateVertexShader (blob_p->GetBufferPointer (),
                                          blob_p->GetBufferSize (),
                                          NULL,
                                          &vertexShader_);
  ACE_ASSERT (SUCCEEDED (result) && vertexShader_);

  struct D3D11_INPUT_ELEMENT_DESC input_element_descriptor_s[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
  };
  result = device_in->CreateInputLayout (input_element_descriptor_s,
                                         ARRAYSIZE (input_element_descriptor_s),
                                         blob_p->GetBufferPointer (),
                                         blob_p->GetBufferSize (),
                                         &inputLayout_);
  ACE_ASSERT (SUCCEEDED (result) && inputLayout_);
  blob_p->Release (); blob_p = NULL;

  // COMPILE PIXEL SHADER
  result = D3DCompileFromFile (ACE_TEXT_ALWAYS_WCHAR (filename_in.c_str ()),
                               NULL,
                               D3D_COMPILE_STANDARD_FILE_INCLUDE,
                               ACE_TEXT_ALWAYS_CHAR ("ps_main"),
                               ACE_TEXT_ALWAYS_CHAR ("ps_5_0"),
                               flags_i,
                               0,
                               &blob_p,
                               &error_blob_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to D3DCompileFromFile(\"%s\"): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (filename_in.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
    if (error_blob_p)
    {
      OutputDebugStringA ((char*)error_blob_p->GetBufferPointer ());
      error_blob_p->Release ();
    } // end IF
    if (blob_p)
    { blob_p->Release (); blob_p = NULL; } // end IF
    return false;
  } // end IF

  result = device_in->CreatePixelShader (blob_p->GetBufferPointer (),
                                         blob_p->GetBufferSize (),
                                         NULL,
                                         &pixelShader_);
  ACE_ASSERT (SUCCEEDED (result) && pixelShader_);
  blob_p->Release (); blob_p = NULL;

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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::svc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::svc"));

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

  if (unlikely (!initialize_Direct3D (inherited::window_,
                                      format_)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize_Direct3D(), aborting\n"),
                inherited::mod_->name ()));
    CloseWindow (inherited::window_); inherited::window_ = NULL;
    return -1;
  } // end IF

  inherited::notify_ = true;

  struct tagMSG message_s;
  while (GetMessage (&message_s, inherited::window_, 0, 0) != -1)
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
Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::initialize_Direct3D (HWND window_in,
                                                                REFGUID format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D11_T::initialize_Direct3D"));

  struct DXGI_SWAP_CHAIN_DESC swap_chain_descriptor_s;
  ACE_OS::memset (&swap_chain_descriptor_s, 0, sizeof (DXGI_SWAP_CHAIN_DESC));
  swap_chain_descriptor_s.BufferCount = 1;
  swap_chain_descriptor_s.BufferDesc.RefreshRate.Numerator = 0;
  swap_chain_descriptor_s.BufferDesc.RefreshRate.Denominator = 1;
  swap_chain_descriptor_s.BufferDesc.Format =
    Stream_MediaFramework_DirectDraw_Tools::toFormat_2 (format_in);
  swap_chain_descriptor_s.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swap_chain_descriptor_s.OutputWindow = window_in;
  swap_chain_descriptor_s.SampleDesc.Count = 1;
  swap_chain_descriptor_s.SampleDesc.Quality = 0;
  swap_chain_descriptor_s.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // *TODO*: switch to flip model
  swap_chain_descriptor_s.Windowed = TRUE;

  enum D3D_FEATURE_LEVEL feature_level_e;
  UINT flags_i = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined (_DEBUG) || defined (DEBUG)
  flags_i |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG || DEBUG
  HRESULT result_2 = D3D11CreateDeviceAndSwapChain (NULL,
                                                    D3D_DRIVER_TYPE_HARDWARE,
                                                    NULL,
                                                    flags_i,
                                                    NULL,
                                                    0,
                                                    D3D11_SDK_VERSION,
                                                    &swap_chain_descriptor_s,
                                                    &swapChain_,
                                                    &device_,
                                                    &feature_level_e,
                                                    &context_);
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to D3D11CreateDeviceAndSwapChain(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (swapChain_);
  ACE_ASSERT (device_);
  ACE_ASSERT (context_);

  struct D3D11_SAMPLER_DESC sampler_descriptor_s;
  ACE_OS::memset (&sampler_descriptor_s, 0, sizeof (D3D11_SAMPLER_DESC));
  sampler_descriptor_s.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  sampler_descriptor_s.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  sampler_descriptor_s.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  sampler_descriptor_s.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  sampler_descriptor_s.BorderColor[0] = 1.0f;
  sampler_descriptor_s.BorderColor[1] = 1.0f;
  sampler_descriptor_s.BorderColor[2] = 1.0f;
  sampler_descriptor_s.BorderColor[3] = 1.0f;
  sampler_descriptor_s.ComparisonFunc = D3D11_COMPARISON_NEVER;
  result_2 = device_->CreateSamplerState (&sampler_descriptor_s,
                                          &samplerState_);
  ACE_ASSERT (SUCCEEDED (result_2) && samplerState_);
  context_->PSSetSamplers (0, 1, &samplerState_);

  struct D3D11_TEXTURE2D_DESC texture_descriptor_s;
  ACE_OS::memset (&texture_descriptor_s, 0, sizeof (struct D3D11_TEXTURE2D_DESC));
  texture_descriptor_s.Width = inherited::resolution_.cx;
  texture_descriptor_s.Height = inherited::resolution_.cy;
  texture_descriptor_s.MipLevels = 1;
  texture_descriptor_s.ArraySize = 1;
  texture_descriptor_s.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  texture_descriptor_s.SampleDesc.Count = 1;
  //texture_descriptor_s.SampleDesc.Quality = 0;
  texture_descriptor_s.Usage = D3D11_USAGE_DEFAULT;
  texture_descriptor_s.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  //texture_descriptor_s.CPUAccessFlags = 0; 
  //texture_descriptor_s.MiscFlags = 0;
  result_2 = device_->CreateTexture2D (&texture_descriptor_s,
                                       NULL,
                                       &depthStencilBuffer_);
  ACE_ASSERT (SUCCEEDED (result_2) && depthStencilBuffer_);
  result_2 = device_->CreateDepthStencilView (depthStencilBuffer_,
                                              NULL,
                                              &depthStencilView_);
  ACE_ASSERT (SUCCEEDED (result_2) && depthStencilView_);

  struct D3D11_DEPTH_STENCIL_DESC depth_stencil_descriptor_s;
  ACE_OS::memset (&depth_stencil_descriptor_s, 0, sizeof (struct D3D11_DEPTH_STENCIL_DESC));
  depth_stencil_descriptor_s.DepthEnable = TRUE;
  depth_stencil_descriptor_s.DepthWriteMask =
    D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_descriptor_s.DepthFunc =
    D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
  result_2 = device_->CreateDepthStencilState (&depth_stencil_descriptor_s,
                                                &depthStencilState_);
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D11Device::CreateDepthStencilState(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (depthStencilState_);
  //context_->OMSetDepthStencilState (depthStencilState_, 0);

  result_2 =
    swapChain_->GetBuffer (0,
                           __uuidof (ID3D11Texture2D),
                           reinterpret_cast<void**> (&backBuffer_));
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDXGISwapChain::GetBuffer(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (backBuffer_);
  result_2 = device_->CreateRenderTargetView (backBuffer_,
                                              NULL,
                                              &renderTargetView_);
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D11Device::CreateRenderTargetView(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (renderTargetView_);
  context_->OMSetRenderTargets (1, &renderTargetView_, NULL);
  //context_->OMSetRenderTargets (1, &renderTargetView_, depthStencilView_);

  ACE_OS::memset (&texture_descriptor_s, 0, sizeof (struct D3D11_TEXTURE2D_DESC));
  texture_descriptor_s.Width = inherited::resolution_.cx;
  texture_descriptor_s.Height = inherited::resolution_.cy;
  texture_descriptor_s.MipLevels = 1;
  texture_descriptor_s.ArraySize = 1;
  texture_descriptor_s.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  texture_descriptor_s.SampleDesc.Count = 1;
  texture_descriptor_s.Usage = D3D11_USAGE_DYNAMIC;
  texture_descriptor_s.BindFlags = D3D11_BIND_SHADER_RESOURCE;
  texture_descriptor_s.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  result_2 = device_->CreateTexture2D (&texture_descriptor_s,
                                       NULL,
                                       &texture_);
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D11Device::CreateTexture2D(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (texture_);

  struct tagRECT rect_s;
  GetClientRect (window_in, &rect_s);
  struct D3D11_VIEWPORT viewport_s;
  ACE_OS::memset (&viewport_s, 0, sizeof (struct D3D11_VIEWPORT));
  //viewport_s.TopLeftX = 0.0f;
  //viewport_s.TopLeftY = 0.0f;
  viewport_s.Width = (FLOAT)(rect_s.right - rect_s.left);
  viewport_s.Height = (FLOAT)(rect_s.bottom - rect_s.top);
  //viewport_s.MinDepth = 0.0f;
  viewport_s.MaxDepth = 1.0f;
  //ACE_ASSERT (viewport_s.Width == resolution_s.cx && viewport_s.Height == resolution_s.cy);
  context_->RSSetViewports (1, &viewport_s);

  if (!compileShaders (inherited::configuration_->shaderFile,
                       device_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to compileShaders(\"%s\"), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (inherited::configuration_->shaderFile.c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (vertexShader_);
  ACE_ASSERT (inputLayout_);
  ACE_ASSERT (pixelShader_);

  struct D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_descriptor_s;
  ACE_OS::memset (&shader_resource_view_descriptor_s, 0, sizeof (struct D3D11_SHADER_RESOURCE_VIEW_DESC));
  shader_resource_view_descriptor_s.Format = texture_descriptor_s.Format;
  shader_resource_view_descriptor_s.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shader_resource_view_descriptor_s.Texture2D.MostDetailedMip = 0;
  shader_resource_view_descriptor_s.Texture2D.MipLevels = 1;
  result_2 =
    device_->CreateShaderResourceView (texture_,
                                       &shader_resource_view_descriptor_s,
                                       &shaderResourceView_);
  if (unlikely (FAILED (result_2)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D11Device::CreateShaderResourceView(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (shaderResourceView_);
  context_->PSSetShaderResources (0, 1, &shaderResourceView_);

  struct D3D11_BUFFER_DESC vertex_buffer_descriptor_s;
  float vertex_data_array_a[] = {
    -1.0f, -1.0f,   0.0f, 1.0f, // point at bottom-left
    -1.0f,  1.0f,   0.0f, 0.0f, // point at top-left
     1.0f,  1.0f,   1.0f, 0.0f, // point at top-right

    -1.0f, -1.0f,   0.0f, 1.0f, // point at bottom-left
     1.0f,  1.0f,   1.0f, 0.0f, // point at top-right
     1.0f, -1.0f,   1.0f, 1.0f  // point at bottom-right
  };
  ACE_OS::memset (&vertex_buffer_descriptor_s, 0, sizeof (struct D3D11_BUFFER_DESC));
  vertex_buffer_descriptor_s.ByteWidth = 4 * sizeof (float) * ARRAYSIZE (vertex_data_array_a);
  vertex_buffer_descriptor_s.Usage = D3D11_USAGE_IMMUTABLE;
  vertex_buffer_descriptor_s.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  struct D3D11_SUBRESOURCE_DATA sub_resource_data_s;
  ACE_OS::memset (&sub_resource_data_s, 0, sizeof (struct D3D11_SUBRESOURCE_DATA));
  /*** load mesh data into vertex buffer **/
  sub_resource_data_s.pSysMem = vertex_data_array_a;
  result_2 = device_->CreateBuffer (&vertex_buffer_descriptor_s,
                                    &sub_resource_data_s,
                                    &vertexBuffer_);
  ACE_ASSERT (SUCCEEDED (result_2) && vertexBuffer_);

  context_->IASetPrimitiveTopology (D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context_->IASetInputLayout (inputLayout_);
  UINT vertex_stride = 4 * sizeof (float);
  UINT vertex_offset = 0;
  context_->IASetVertexBuffers (0,
                                1,
                                &vertexBuffer_,
                                &vertex_stride,
                                &vertex_offset);
  context_->VSSetShader (vertexShader_, NULL, 0);
  context_->PSSetShader (pixelShader_, NULL, 0);

  struct D3D11_RASTERIZER_DESC rasterizer_descriptor_s;
  ACE_OS::memset (&rasterizer_descriptor_s, 0, sizeof (struct D3D11_RASTERIZER_DESC));
  rasterizer_descriptor_s.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
  rasterizer_descriptor_s.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
  result_2 = device_->CreateRasterizerState (&rasterizer_descriptor_s,
                                             &rasterizerState_);
  ACE_ASSERT (SUCCEEDED (result_2) && rasterizerState_);
  context_->RSSetState (rasterizerState_);

  return true;
}
