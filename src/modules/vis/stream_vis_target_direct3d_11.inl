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
 , inherited2 ()
 , inherited3 ()
 , clientWindow_ (NULL)
 , closeWindow_ (false)
 , context_ (NULL)
 , device_ (NULL)
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

  if (closeWindow_)
    if (unlikely (!CloseWindow (clientWindow_)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));

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
  ACE_ASSERT (context_);
  ACE_ASSERT (swapChain_);
  ACE_ASSERT (texture_);

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
      Common_Image_Resolution_t resolution_s;
      HWND window_handle_p = NULL;
      bool initialize_device_b = (clientWindow_ == NULL);
      float vertex_data_array_a[] =
      {
        -1.0f, -1.0f,   0.0f, 1.0f, // point at bottom-left
        -1.0f,  1.0f,   0.0f, 0.0f, // point at top-left
         1.0f,  1.0f,   1.0f, 0.0f, // point at top-right

        -1.0f, -1.0f,   0.0f, 1.0f, // point at bottom-left
         1.0f,  1.0f,   1.0f, 0.0f, // point at top-right
         1.0f, -1.0f,   1.0f, 1.0f  // point at bottom-right
      };
      /*** load mesh data into vertex buffer **/
      struct D3D11_BUFFER_DESC vertex_buffer_descriptor_s;
      struct D3D11_SUBRESOURCE_DATA sub_resource_data_s;
      struct tagRECT rect_s;
      struct D3D11_VIEWPORT viewport_s;
      struct D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_descriptor_s;
      struct D3D11_SAMPLER_DESC sampler_descriptor_s;
      struct D3D11_DEPTH_STENCIL_DESC depth_stencil_descriptor_s;
      struct D3D11_RASTERIZER_DESC rasterizer_descriptor_s;
      struct D3D11_TEXTURE2D_DESC texture_descriptor_s;
      bool COM_initialized = Common_Tools::initializeCOM ();

      // sanity check(s)
      ACE_ASSERT (!session_data_r.formats.empty ());
      inherited2::getMediaType (session_data_r.formats.back (),
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_s);
      resolution_s =
        Stream_MediaFramework_DirectShow_Tools::toResolution (media_type_s);
      window_handle_p = clientWindow_;
      if (window_handle_p)
      {
        ACE_ASSERT (IsWindow (window_handle_p));
      } // end IF

      if (!window_handle_p)
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
        clientWindow_ =
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
                          resolution_s.cx,                         // nWidth
                          resolution_s.cy,                         // nHeight
                          NULL,                                    // hWndParent
                          NULL,                                    // hMenu
                          GetModuleHandle (NULL),                  // hInstance
                          NULL);                                   // lpParam
        if (unlikely (!clientWindow_))
        { // ERROR_INVALID_PARAMETER: 87
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CreateWindowEx(): \"%s\", aborting\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
          goto error;
        } // end IF
        window_handle_p = clientWindow_;
        closeWindow_ = true;
      } // end IF
      ACE_ASSERT (window_handle_p);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: window handle: 0x%@\n"),
                  inherited::mod_->name (),
                  window_handle_p));

      if (initialize_device_b)
      {
        struct DXGI_SWAP_CHAIN_DESC swap_chain_descriptor_s;
        ACE_OS::memset (&swap_chain_descriptor_s, 0, sizeof (DXGI_SWAP_CHAIN_DESC));
        swap_chain_descriptor_s.BufferCount = 1;
        swap_chain_descriptor_s.BufferDesc.RefreshRate.Numerator = 0;
        swap_chain_descriptor_s.BufferDesc.RefreshRate.Denominator = 1;
        swap_chain_descriptor_s.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        swap_chain_descriptor_s.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_chain_descriptor_s.OutputWindow = clientWindow_;
        swap_chain_descriptor_s.SampleDesc.Count = 1;
        swap_chain_descriptor_s.SampleDesc.Quality = 0;
        swap_chain_descriptor_s.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swap_chain_descriptor_s.Windowed = true;

        enum D3D_FEATURE_LEVEL feature_level_e;
        UINT flags_i = D3D11_CREATE_DEVICE_SINGLETHREADED;
    #if defined (_DEBUG) || defined (DEBUG)
        flags_i |= D3D11_CREATE_DEVICE_DEBUG;
    #endif // _DEBUG || DEBUG
        result_2 = D3D11CreateDeviceAndSwapChain (NULL,
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
          goto error;
        } // end IF
      } // end IF
      ACE_ASSERT (swapChain_);
      ACE_ASSERT (device_);
      ACE_ASSERT (context_);

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

      ACE_OS::memset (&texture_descriptor_s, 0, sizeof (struct D3D11_TEXTURE2D_DESC));
      texture_descriptor_s.Width = resolution_s.cx;
      texture_descriptor_s.Height = resolution_s.cy;
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

      ACE_OS::memset (&depth_stencil_descriptor_s, 0, sizeof (struct D3D11_DEPTH_STENCIL_DESC));
      depth_stencil_descriptor_s.DepthEnable = true;
      depth_stencil_descriptor_s.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
      depth_stencil_descriptor_s.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
      result_2 = device_->CreateDepthStencilState (&depth_stencil_descriptor_s,
                                                   &depthStencilState_);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ID3D11Device::CreateDepthStencilState(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
        goto error;
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
        goto error;
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
        goto error;
      } // end IF
      ACE_ASSERT (renderTargetView_);

      context_->OMSetRenderTargets (1, &renderTargetView_, NULL);
      //context_->OMSetRenderTargets (1, &renderTargetView_, depthStencilView_);

      ACE_OS::memset (&texture_descriptor_s, 0, sizeof (struct D3D11_TEXTURE2D_DESC));
      texture_descriptor_s.Width = resolution_s.cx;
      texture_descriptor_s.Height = resolution_s.cy;
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
        goto error;
      } // end IF
      ACE_ASSERT (texture_);

      GetClientRect (clientWindow_, &rect_s);
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
        goto error;
      } // end IF
      ACE_ASSERT (vertexShader_);
      ACE_ASSERT (inputLayout_);
      ACE_ASSERT (pixelShader_);

      ACE_OS::memset (&shader_resource_view_descriptor_s, 0, sizeof (struct D3D11_SHADER_RESOURCE_VIEW_DESC));
      shader_resource_view_descriptor_s.Format = texture_descriptor_s.Format;
      shader_resource_view_descriptor_s.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      shader_resource_view_descriptor_s.Texture2D.MostDetailedMip = 0;
      shader_resource_view_descriptor_s.Texture2D.MipLevels = 1;
      result_2 = device_->CreateShaderResourceView (texture_,
                                                    &shader_resource_view_descriptor_s,
                                                    &shaderResourceView_);
      if (unlikely (FAILED (result_2)))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ID3D11Device::CreateShaderResourceView(): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (shaderResourceView_);

      context_->PSSetShaderResources (0, 1, &shaderResourceView_);

      ACE_OS::memset (&vertex_buffer_descriptor_s, 0, sizeof (struct D3D11_BUFFER_DESC));
      vertex_buffer_descriptor_s.ByteWidth = 4 * sizeof (float) * ARRAYSIZE (vertex_data_array_a);
      vertex_buffer_descriptor_s.Usage = D3D11_USAGE_IMMUTABLE;
      vertex_buffer_descriptor_s.BindFlags = D3D11_BIND_VERTEX_BUFFER;
      ACE_OS::memset (&sub_resource_data_s, 0, sizeof (struct D3D11_SUBRESOURCE_DATA));
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

      ACE_OS::memset (&rasterizer_descriptor_s, 0, sizeof (struct D3D11_RASTERIZER_DESC));
      rasterizer_descriptor_s.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
      rasterizer_descriptor_s.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
      result_2 = device_->CreateRasterizerState (&rasterizer_descriptor_s,
                                                 &rasterizerState_);
      ACE_ASSERT (SUCCEEDED (result_2) && rasterizerState_);
      context_->RSSetState (rasterizerState_);

      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (COM_initialized) Common_Tools::finalizeCOM ();

      break;

error:
      Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

      if (closeWindow_)
      {
        closeWindow_ = false;
        if (!CloseWindow (clientWindow_))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        clientWindow_ = NULL;
      } // end IF

      if (COM_initialized) Common_Tools::finalizeCOM ();

      notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      bool COM_initialized = Common_Tools::initializeCOM ();

      if (closeWindow_)
      {
        if (unlikely (!CloseWindow (clientWindow_)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
        clientWindow_ = NULL;
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

  // *IMPORTANT NOTE*: the configuration has to be updated at this stage !

  // update configuration
  //if (direct3DConfiguration_->presentationParameters.Windowed)
  //{ // --> switch to windowed mode
  //  ACE_ASSERT (clientWindow_);
  //  ACE_ASSERT (direct3DConfiguration_->focusWindow == clientWindow_);

  //  // *NOTE*: 0 --> use window format
  //  direct3DConfiguration_->presentationParameters.BackBufferWidth =
  //    resolution_s.cx;
  //    //0;
  //  direct3DConfiguration_->presentationParameters.BackBufferHeight =
  //    resolution_s.cy;
  //    //0;
  //  direct3DConfiguration_->presentationParameters.BackBufferFormat =
  //    D3DFMT_UNKNOWN;
  //  direct3DConfiguration_->presentationParameters.hDeviceWindow =
  //    clientWindow_;
  //  //direct3DConfiguration_->presentationParameters.Windowed = TRUE;
  //  direct3DConfiguration_->presentationParameters.FullScreen_RefreshRateInHz =
  //    0;
  //  //direct3DConfiguration_->presentationParameters.PresentationInterval = ;
  //} // end IF
  //else
  //{ // --> switch to fullscreen mode
  //  ACE_ASSERT (!direct3DConfiguration_->presentationParameters.hDeviceWindow);
  //  ACE_ASSERT (direct3DConfiguration_->focusWindow);
  //  struct _D3DDISPLAYMODE display_mode_s =
  //    Stream_MediaFramework_DirectDraw_Tools::getDisplayMode (direct3DConfiguration_->adapter,
  //                                                            STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT,
  //                                                            resolution_s);
  //  if ((display_mode_s.Format != STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT) ||
  //      ((display_mode_s.Width  != resolution_s.cx) ||
  //       (display_mode_s.Height != resolution_s.cy)))
  //  {
  //    // *TODO*: select closest possible format
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("%s: adapter (was: %d) does not support fullscreen mode (was: %ux%u), returning\n"),
  //                inherited::mod_->name (),
  //                direct3DConfiguration_->adapter,
  //                resolution_s.cx, resolution_s.cy));
  //    goto error;
  //  } // end IF

  //  direct3DConfiguration_->presentationParameters.BackBufferWidth =
  //    resolution_s.cx;
  //  direct3DConfiguration_->presentationParameters.BackBufferHeight =
  //    resolution_s.cy;
  //  direct3DConfiguration_->presentationParameters.BackBufferFormat =
  //    STREAM_LIB_DIRECTDRAW_3D_DEFAULT_FORMAT;
  //  clientWindow_ = direct3DConfiguration_->focusWindow;
  //  direct3DConfiguration_->presentationParameters.hDeviceWindow = NULL;
  //  //direct3DConfiguration_->presentationParameters.Windowed = FALSE;
  //  direct3DConfiguration_->presentationParameters.FullScreen_RefreshRateInHz =
  //    display_mode_s.RefreshRate;
  //  //direct3DConfiguration_->presentationParameters.PresentationInterval = ;
  //} // end ELSE

  Stream_MediaFramework_DirectShow_Tools::free (media_type_s);

  return;

//error:
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
    if (closeWindow_)
    {
      closeWindow_ = false;
      if (unlikely (!CloseWindow (clientWindow_)))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CloseWindow(): \"%s\", continuing\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
    } // end IF
    clientWindow_ = NULL;
  
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

  inherited3::getWindowType (configuration_in.window, clientWindow_);
  if (clientWindow_)
  {
    struct DXGI_SWAP_CHAIN_DESC swap_chain_descriptor_s;
    ACE_OS::memset (&swap_chain_descriptor_s, 0, sizeof (DXGI_SWAP_CHAIN_DESC));
    swap_chain_descriptor_s.BufferCount = 1;
    swap_chain_descriptor_s.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_chain_descriptor_s.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_descriptor_s.BufferDesc.RefreshRate.Denominator = 1;
    swap_chain_descriptor_s.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_chain_descriptor_s.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    swap_chain_descriptor_s.OutputWindow = clientWindow_;
    swap_chain_descriptor_s.SampleDesc.Count = 1;
    swap_chain_descriptor_s.SampleDesc.Quality = 0;
    swap_chain_descriptor_s.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_chain_descriptor_s.Windowed = true;

    enum D3D_FEATURE_LEVEL feature_level_e;
    UINT flags_i = D3D11_CREATE_DEVICE_SINGLETHREADED;
#if defined (_DEBUG) || defined (DEBUG)
    flags_i |= D3D11_CREATE_DEVICE_DEBUG;
#endif // _DEBUG || DEBUG
    HRESULT result = D3D11CreateDeviceAndSwapChain (NULL,
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
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to D3D11CreateDeviceAndSwapChain(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (swapChain_);
    ACE_ASSERT (device_);
    ACE_ASSERT (context_);
  } // end IF

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
