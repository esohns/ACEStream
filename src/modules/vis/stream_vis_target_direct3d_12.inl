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
#include "dxgi1_6.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_file_tools.h"

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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::Stream_Vis_Target_Direct3D12_T (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , viewport_ (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f)
 , scissorRect_ (0, 0, 0, 0)
 , swapChain_ (NULL)
 , device_ (NULL)
 , renderTargets_ ()
 , commandAllocator_ (NULL)
 , commandQueue_ (NULL)
 , rootSignature_ (NULL)
 , rtvHeap_ (NULL)
 , srvHeap_ (NULL)
 , pipelineState_ (NULL)
 , commandList_ (NULL)
 , rtvDescriptorSize_ (0)
 , vertexBuffer_ (NULL)
 , vertexBufferView_ ()
 , texture_ (NULL)
 , frameIndex_ (0)
 , fenceEvent_ (ACE_INVALID_HANDLE)
 , fence_ (NULL)
 , fenceValue_ (0)
 , format_ (GUID_NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::Stream_Vis_Target_Direct3D12_T"));

  ACE_OS::memset (renderTargets_, 0, sizeof (ID3D12Resource*[STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT]));
  ACE_OS::memset (&vertexBufferView_, 0, sizeof (D3D12_VERTEX_BUFFER_VIEW));
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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::~Stream_Vis_Target_Direct3D12_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::~Stream_Vis_Target_Direct3D12_T"));

  if (swapChain_)
    swapChain_->Release ();
  if (device_)
    device_->Release ();
  for (UINT i = 0;
       i < STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT;
       i++)
    if (renderTargets_[i])
      renderTargets_[i]->Release ();
  if (commandAllocator_)
    commandAllocator_->Release ();
  if (commandQueue_)
    commandQueue_->Release ();
  if (rootSignature_)
    rootSignature_->Release ();
  if (rtvHeap_)
    rtvHeap_->Release ();
  if (srvHeap_)
    srvHeap_->Release ();
  if (pipelineState_)
    pipelineState_->Release ();
  if (commandList_)
    commandList_->Release ();
  if (vertexBuffer_)
    vertexBuffer_->Release ();
  if (texture_)
    texture_->Release ();
  CloseHandle (fenceEvent_);
  if (fence_)
    fence_->Release ();
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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  // *TODO*: make this an 'active' module and remove this
  if (!device_ || !texture_ || !swapChain_)
    return; // --> not ready yet

  // Note: ComPtr's are CPU objects but this resource needs to stay in scope until
  // the command list that references it has finished executing on the GPU.
  // We will flush the GPU at the end of this method to ensure the resource is not
  // prematurely destroyed.
  ID3D12Resource* textureUploadHeap = NULL;

  const UINT64 uploadBufferSize = GetRequiredIntermediateSize (texture_, 0, 1);

  // Create the GPU upload buffer.
  HRESULT result_2 = device_->CreateCommittedResource (&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD),
                                                       D3D12_HEAP_FLAG_NONE,
                                                       &CD3DX12_RESOURCE_DESC::Buffer (uploadBufferSize),
                                                       D3D12_RESOURCE_STATE_GENERIC_READ,
                                                       NULL,
                                                       IID_PPV_ARGS (&textureUploadHeap));
  ACE_ASSERT (SUCCEEDED (result_2) && textureUploadHeap);

  // Copy data to the intermediate upload heap and then schedule a copy 
  // from the upload heap to the Texture2D.
  D3D12_SUBRESOURCE_DATA textureData = {};
  textureData.pData = message_inout->rd_ptr ();
  textureData.RowPitch = inherited::resolution_.cx * (4 * sizeof (uint8_t)); // RGBA
  textureData.SlicePitch = textureData.RowPitch * inherited::resolution_.cy;

  // Command list allocators can only be reset when the associated 
  // command lists have finished execution on the GPU; apps should use 
  // fences to determine GPU execution progress.
  result_2 = commandAllocator_->Reset ();
  //ACE_ASSERT (SUCCEEDED (result_2));

  // However, when ExecuteCommandList() is called on a particular command 
  // list, that command list can then be reset at any time and must be before 
  // re-recording.
  result_2 = commandList_->Reset (commandAllocator_, pipelineState_);
  //ACE_ASSERT (SUCCEEDED (result_2));

  // Set necessary state.
  commandList_->SetGraphicsRootSignature (rootSignature_);

  ID3D12DescriptorHeap* ppHeaps[] = { srvHeap_ };
  commandList_->SetDescriptorHeaps (_countof (ppHeaps), ppHeaps);

  commandList_->SetGraphicsRootDescriptorTable (0, srvHeap_->GetGPUDescriptorHandleForHeapStart ());
  commandList_->RSSetViewports (1, &viewport_);
  commandList_->RSSetScissorRects (1, &scissorRect_);

  // Indicate that the back buffer will be used as a render target.
  commandList_->ResourceBarrier (1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition (renderTargets_[frameIndex_],
                                                                        D3D12_RESOURCE_STATE_PRESENT,
                                                                        D3D12_RESOURCE_STATE_RENDER_TARGET));

  CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle_h (rtvHeap_->GetCPUDescriptorHandleForHeapStart (), frameIndex_, rtvDescriptorSize_);
  commandList_->OMSetRenderTargets (1, &rtvHandle_h, FALSE, NULL);

  // Record commands.
  UpdateSubresources (commandList_, texture_, textureUploadHeap, 0, 0, 1, &textureData);

  const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
  commandList_->ClearRenderTargetView (rtvHandle_h, clearColor, 0, NULL);
  commandList_->IASetPrimitiveTopology (D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  commandList_->IASetVertexBuffers (0, 1, &vertexBufferView_);
  //commandList_->DrawInstanced (3, 1, 0, 0);
  commandList_->DrawInstanced (6, 1, 0, 0);

  // Indicate that the back buffer will now be used to present.
  commandList_->ResourceBarrier (1,
                                 &CD3DX12_RESOURCE_BARRIER::Transition (renderTargets_[frameIndex_],
                                                                        D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                                        D3D12_RESOURCE_STATE_PRESENT));

  // Close the command list and execute it to begin the initial GPU setup.
  result_2 = commandList_->Close ();
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D12GraphicsCommandList::Close(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    goto error;
  } // end IF

  ID3D12CommandList* ppCommandLists[] = { commandList_ };
  commandQueue_->ExecuteCommandLists (_countof (ppCommandLists), ppCommandLists);

  result_2 = swapChain_->Present (1, 0);
  ACE_ASSERT (SUCCEEDED (result_2));

  // Wait for the command list to execute; we are reusing the same command 
  // list in our main loop but for now, we just want to wait for setup to 
  // complete before continuing.
  waitForPreviousFrame ();

  textureUploadHeap->Release (); textureUploadHeap = NULL;

  return;

error:
  if (textureUploadHeap)
    textureUploadHeap->Release ();

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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::handleSessionMessage"));

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
        if (unlikely (!PostMessage (inherited::window_, WM_QUIT, 0, 0)))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to PostMessage(%@): \"%s\", continuing\n"),
                      inherited::mod_->name (),
                      inherited::window_,
                      ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::toggle"));

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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
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
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::initialize"));

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
    if (device_)
      device_->Release ();
    device_ = NULL;
    for (UINT i = 0;
         i < STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT;
         i++)
    {
      renderTargets_[i]->Release (); renderTargets_[i] = NULL;
    } // end FOR
    if (commandAllocator_)
      commandAllocator_->Release ();
    commandAllocator_ = NULL;
    if (commandQueue_)
      commandQueue_->Release ();
    commandQueue_ = NULL;
    if (rootSignature_)
      rootSignature_->Release ();
    rootSignature_ = NULL;
    if (rtvHeap_)
      rtvHeap_->Release ();
    rtvHeap_ = NULL;
    if (srvHeap_)
      srvHeap_->Release ();
    srvHeap_ = NULL;
    if (pipelineState_)
      pipelineState_->Release ();
    pipelineState_ = NULL;
    if (commandList_)
      commandList_->Release ();
    commandList_ = NULL;
    if (vertexBuffer_)
      vertexBuffer_->Release ();
    vertexBuffer_ = NULL;
    if (texture_)
      texture_->Release ();
    texture_ = NULL;
    CloseHandle (fenceEvent_); fenceEvent_ = ACE_INVALID_HANDLE;
    if (fence_)
      fence_->Release ();
    fence_ = NULL;
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
int
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::svc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::svc"));

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
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return -1;
  } // end IF
  //SetWindowLongPtr (window_, GWLP_USERDATA, (LONG_PTR)&CBData_);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: window handle: 0x%@\n"),
              inherited::mod_->name (),
              inherited::window_));

  if (unlikely (!initialize_Direct3D (inherited::window_,
                                      inherited::configuration_->direct3DConfiguration->useSoftwareRenderer))) // use software renderer ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to initialize_Direct3D(), aborting\n"),
                inherited::mod_->name ()));
    CloseWindow (inherited::window_); inherited::window_ = NULL;
    inherited::notify (STREAM_SESSION_MESSAGE_ABORT);
    return -1;
  } // end IF

  inherited::notify_ = true;

  struct tagMSG message_s;
  BOOL bRet;
  while (bRet = GetMessage (&message_s, inherited::window_, 0, 0) != 0)
  {
    if (bRet == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to GetMessage(%@): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  inherited::window_,
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError ()).c_str ())));
      break;
    } // end IF

    TranslateMessage (&message_s);
    DispatchMessage (&message_s);

    if (message_s.message == WM_CLOSE)
      break;
  } // end WHILE
  DestroyWindow (inherited::window_); inherited::window_ = NULL;

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
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::initialize_Direct3D (HWND window_in,
                                                                bool useSoftwareRenderer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Vis_Target_Direct3D12_T::initialize_Direct3D"));

  HRESULT result_2 = E_FAIL;

  UINT dxgiFactoryFlags = 0;
#if defined (_DEBUG)
  // Enable the debug layer (requires the Graphics Tools "optional feature").
  // NOTE: Enabling the debug layer after device creation will invalidate the active device.
  ID3D12Debug* debug_controller_p = NULL;
  result_2 = D3D12GetDebugInterface (IID_PPV_ARGS (&debug_controller_p));
  if (FAILED (result_2) || !debug_controller_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to D3D12GetDebugInterface(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF
  debug_controller_p->EnableDebugLayer ();
  debug_controller_p->Release (); debug_controller_p = NULL;

  // Enable additional debug layers.
  dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // _DEBUG

  IDXGIFactory4* factory_p = NULL;
  result_2 = CreateDXGIFactory2 (dxgiFactoryFlags, IID_PPV_ARGS (&factory_p));
  if (FAILED (result_2) || !factory_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CreateDXGIFactory2(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF

  IDXGIFactory6* factory_2 = NULL;
  result_2 = factory_p->QueryInterface (IID_PPV_ARGS (&factory_2));
  if (FAILED (result_2) || !factory_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDXGIFactory4::QueryInterface(IID_IDXGIFactory6): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF

  IDXGIAdapter1* adapter_p = NULL;
  for (UINT adapterIndex = 0;
       SUCCEEDED (factory_2->EnumAdapterByGpuPreference (adapterIndex,
                                                         DXGI_GPU_PREFERENCE_UNSPECIFIED,//DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                         IID_PPV_ARGS (&adapter_p)));
       ++adapterIndex)
  {
    DXGI_ADAPTER_DESC1 desc;
    adapter_p->GetDesc1 (&desc);

    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
    {
      if (useSoftwareRenderer_in)
        break;

      // Don't select the Basic Render Driver adapter.
      adapter_p->Release (); adapter_p = NULL;
      continue;
    } // end IF

    // --> not a software renderer device

    // Check to see whether the adapter supports Direct3D 12, but don't create the
    // actual device yet.
    if (!useSoftwareRenderer_in &&
        SUCCEEDED (D3D12CreateDevice (adapter_p,
                                      D3D_FEATURE_LEVEL_11_0,
                                      _uuidof (ID3D12Device),
                                      NULL)))
      break;

    adapter_p->Release (); adapter_p = NULL;
  } // end FOR
  factory_2->Release (); factory_2 = NULL;
  if (!adapter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to retrieve Direct3D 12 compatible graphics adapter, aborting\n"),
                inherited::mod_->name ()));
    factory_p->Release ();
    return false;
  } // end IF

  result_2 = D3D12CreateDevice (adapter_p,
                                D3D_FEATURE_LEVEL_11_0,
                                IID_PPV_ARGS (&device_));
  if (FAILED (result_2) || !device_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to D3D12CreateDevice(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    adapter_p->Release ();
    factory_p->Release ();
    return false;
  } // end IF
  adapter_p->Release (); adapter_p = NULL;

  D3D12_COMMAND_QUEUE_DESC queueDesc = {};
  queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
  queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
  result_2 = device_->CreateCommandQueue (&queueDesc, IID_PPV_ARGS (&commandQueue_));
  if (FAILED (result_2) || !commandQueue_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D12Device::CreateCommandQueue(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    factory_p->Release ();
    return false;
  } // end IF

  //struct tagRECT client_rect_s;
  //BOOL result = GetClientRect (window_in, &client_rect_s);
  //ACE_ASSERT (result);

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
  swapChainDesc.BufferCount = STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT;
  //swapChainDesc.Width = client_rect_s.right - client_rect_s.left;
  //swapChainDesc.Height = client_rect_s.bottom - client_rect_s.top;
  swapChainDesc.Width = inherited::resolution_.cx;
  swapChainDesc.Height = inherited::resolution_.cy;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
  swapChainDesc.SampleDesc.Count = 1;

  IDXGISwapChain1* swap_chain_p = NULL;
  result_2 = factory_p->CreateSwapChainForHwnd (commandQueue_, // Swap chain needs the queue so that it can force a flush on it.
                                                window_in,
                                                &swapChainDesc,
                                                NULL,
                                                NULL,
                                                &swap_chain_p);
  if (FAILED (result_2) || !swap_chain_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDXGIFactory4::CreateSwapChainForHwnd(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    factory_p->Release ();
    return false;
  } // end IF
  // do not support fullscreen transitions
  result_2 = factory_p->MakeWindowAssociation (window_in, DXGI_MWA_NO_ALT_ENTER);
  ACE_ASSERT (SUCCEEDED (result_2));
  factory_p->Release (); factory_p = NULL;

  result_2 = swap_chain_p->QueryInterface (IID_PPV_ARGS (&swapChain_));
  if (FAILED (result_2) || !swapChain_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IDXGISwapChain1::QueryInterface(IID_IDXGISwapChain3): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    swap_chain_p->Release ();
    return false;
  } // end IF
  swap_chain_p->Release (); swap_chain_p = NULL;
  frameIndex_ = swapChain_->GetCurrentBackBufferIndex ();

  // Create descriptor heaps
  { // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    result_2 = device_->CreateDescriptorHeap (&rtvHeapDesc, IID_PPV_ARGS (&rtvHeap_));
    if (FAILED (result_2) || !rtvHeap_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateDescriptorHeap(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
      return false;
    } // end IF

    // Describe and create a shader resource view (SRV) heap for the texture.
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    result_2 = device_->CreateDescriptorHeap (&srvHeapDesc, IID_PPV_ARGS (&srvHeap_));
    if (FAILED (result_2) || !srvHeap_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateDescriptorHeap(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
      return false;
    } // end IF

    rtvDescriptorSize_ = device_->GetDescriptorHandleIncrementSize (D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  }

  // Create frame resources.
  { CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle_h (rtvHeap_->GetCPUDescriptorHandleForHeapStart ());

    // Create a RTV for each frame.
    for (UINT n = 0;
         n < STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT;
         n++)
    {
      result_2 = swapChain_->GetBuffer (n, IID_PPV_ARGS (&renderTargets_[n]));
      ACE_ASSERT (SUCCEEDED (result_2) && renderTargets_[n]);
      device_->CreateRenderTargetView (renderTargets_[n], NULL, rtvHandle_h);
      rtvHandle_h.Offset (1, rtvDescriptorSize_);
    } // end FOR
  }
  result_2 = device_->CreateCommandAllocator (D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS (&commandAllocator_));
  if (FAILED (result_2) || !commandAllocator_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D12Device::CreateCommandAllocator(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF

  // create assets ----------------------

  // Create the root signature.
  { D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};

    // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED (device_->CheckFeatureSupport (D3D12_FEATURE_ROOT_SIGNATURE,
                                              &featureData,
                                              sizeof (D3D12_FEATURE_DATA_ROOT_SIGNATURE))))
      featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;

    CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
    ranges[0].Init (D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsDescriptorTable (1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC sampler = {};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    sampler.MipLODBias = 0;
    sampler.MaxAnisotropy = 0;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
    sampler.MinLOD = 0.0f;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;
    sampler.RegisterSpace = 0;
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init_1_1 (_countof (rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob* signature_p = NULL;
    ID3DBlob* error_p = NULL;
    result_2 = D3DX12SerializeVersionedRootSignature (&rootSignatureDesc, featureData.HighestVersion, &signature_p, &error_p);
    ACE_ASSERT (SUCCEEDED (result_2) && signature_p && !error_p);
    result_2 = device_->CreateRootSignature (0, signature_p->GetBufferPointer (), signature_p->GetBufferSize (), IID_PPV_ARGS (&rootSignature_));
    ACE_ASSERT (SUCCEEDED (result_2) && rootSignature_);

    signature_p->Release (); signature_p = NULL;
  }

  // Create the pipeline state, which includes compiling and loading shaders.
  { ID3DBlob* vertexShaderBlob_p = NULL;
    ID3DBlob* pixelShaderBlob_p = NULL;
    ID3DBlob* error_blob_p = NULL;

#if defined (_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif // _DEBUG

    uint8_t* data_p = NULL;
    ACE_UINT64 file_size_i = 0;
    bool result = Common_File_Tools::load (inherited::configuration_->shaderFile,
                                           data_p,
                                           file_size_i,
                                           0);
    if (!result)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_File_Tools::load(\"%s\"), aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (inherited::configuration_->shaderFile.c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (data_p && file_size_i);

    //result_2 = D3DCompileFromFile (inherited::configuration_->shaderFile.c_str (), NULL, NULL, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader_p, NULL);
    result_2 = D3DCompile (data_p,
                           file_size_i,
                           NULL,
                           NULL,
                           NULL, // D3D_COMPILE_STANDARD_FILE_INCLUDE
                           ACE_TEXT_ALWAYS_CHAR ("VSMain"),
                           ACE_TEXT_ALWAYS_CHAR ("vs_5_0"),
                           compileFlags,
                           0,
                           &vertexShaderBlob_p,
                           &error_blob_p);
    ACE_ASSERT (SUCCEEDED (result_2) && vertexShaderBlob_p);
    if (error_blob_p)
    {
      error_blob_p->Release (); error_blob_p = NULL;
    } // end IF

    //result_2 = D3DCompileFromFile (inherited::configuration_->shaderFile.c_str (), NULL, NULL, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader_p, NULL);
    result_2 = D3DCompile (data_p,
                           file_size_i,
                           NULL,
                           NULL,
                           NULL, // D3D_COMPILE_STANDARD_FILE_INCLUDE
                           ACE_TEXT_ALWAYS_CHAR ("PSMain"),
                           ACE_TEXT_ALWAYS_CHAR ("ps_5_0"),
                           compileFlags,
                           0,
                           &pixelShaderBlob_p,
                           &error_blob_p);
    ACE_ASSERT (SUCCEEDED (result_2) && pixelShaderBlob_p);
    if (error_blob_p)
    {
      error_blob_p->Release (); error_blob_p = NULL;
    } // end IF
    delete [] data_p; data_p = NULL;

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof (inputElementDescs) };
    psoDesc.pRootSignature = rootSignature_;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE (vertexShaderBlob_p);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE (pixelShaderBlob_p);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC (D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC (D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    result_2 = device_->CreateGraphicsPipelineState (&psoDesc, IID_PPV_ARGS (&pipelineState_));
    if (FAILED (result_2) || !pipelineState_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateGraphicsPipelineState(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
      vertexShaderBlob_p->Release ();
      pixelShaderBlob_p->Release ();
      return false;
    } // end IF

    vertexShaderBlob_p->Release ();
    pixelShaderBlob_p->Release ();
  }

  // Create the command list.
  result_2 = device_->CreateCommandList (0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator_, pipelineState_, IID_PPV_ARGS (&commandList_));
  if (FAILED (result_2) || !commandList_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ID3D12Device::CreateCommandList(): \"%s\", aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
    return false;
  } // end IF

  // Create the vertex buffer.
  {
    // Define the geometry for a triangle.
    float aspectRatio = static_cast<float> (inherited::resolution_.cx) / static_cast<float> (inherited::resolution_.cy);

    Vertex triangleVertices[] =
    {
      {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // point at bottom-left
      {{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}}, // point at top-left
      {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}}, // point at top-right

      {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // point at bottom-left
      {{ 1.0f,  1.0f, 0.0f}, {1.0f, 0.0f}}, // point at top-right
      {{ 1.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}  // point at bottom-right

      //{ { 0.0f, 0.25f * aspectRatio, 0.0f }, { 0.5f, 0.0f } },
      //{ { 0.25f, -0.25f * aspectRatio, 0.0f }, { 1.0f, 1.0f } },
      //{ { -0.25f, -0.25f * aspectRatio, 0.0f }, { 0.0f, 1.0f } }
    };

    const UINT vertexBufferSize = sizeof (triangleVertices);

    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.
    result_2 = device_->CreateCommittedResource (&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_UPLOAD),
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &CD3DX12_RESOURCE_DESC::Buffer (vertexBufferSize),
                                                 D3D12_RESOURCE_STATE_GENERIC_READ,
                                                 NULL,
                                                 IID_PPV_ARGS (&vertexBuffer_));
    if (FAILED (result_2) || !vertexBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateCommittedResource(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
      return false;
    } // end IF

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin = NULL;
    CD3DX12_RANGE readRange (0, 0);        // We do not intend to read from this resource on the CPU.
    result_2 = vertexBuffer_->Map (0, &readRange, reinterpret_cast<void**> (&pVertexDataBegin));
    ACE_ASSERT (SUCCEEDED (result_2) && pVertexDataBegin);
    ACE_OS::memcpy (pVertexDataBegin, triangleVertices, sizeof (triangleVertices));
    vertexBuffer_->Unmap (0, NULL);

    // Initialize the vertex buffer view.
    vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress ();
    vertexBufferView_.StrideInBytes = sizeof (Vertex);
    vertexBufferView_.SizeInBytes = vertexBufferSize;
  }

  // Create the texture.
  {
    // Describe and create a Texture2D.
    D3D12_RESOURCE_DESC textureDesc = {};
    textureDesc.MipLevels = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Width = inherited::resolution_.cx;
    textureDesc.Height = inherited::resolution_.cy;
    textureDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
    textureDesc.DepthOrArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

    result_2 = device_->CreateCommittedResource (&CD3DX12_HEAP_PROPERTIES (D3D12_HEAP_TYPE_DEFAULT),
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &textureDesc,
                                                 D3D12_RESOURCE_STATE_COPY_DEST,
                                                 NULL,
                                                 IID_PPV_ARGS (&texture_));
    if (FAILED (result_2) || !texture_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateCommittedResource(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
      return false;
    } // end IF

    // Describe and create a SRV for the texture.
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = textureDesc.Format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device_->CreateShaderResourceView (texture_, &srvDesc, srvHeap_->GetCPUDescriptorHandleForHeapStart ());
  }

  // Create synchronization objects and wait until assets have been uploaded to the GPU.
  { result_2 = device_->CreateFence (0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS (&fence_));
    if (FAILED (result_2) || !fence_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateFence(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
      return false;
    } // end IF

    fenceValue_ = 1;

    // Create an event handle to use for frame synchronization.
    fenceEvent_ = CreateEvent (NULL, FALSE, FALSE, NULL);
    if (fenceEvent_ == NULL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ID3D12Device::CreateFence(): \"%s\", aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (Common_Error_Tools::errorToString (::GetLastError (), false, false).c_str ())));
      return false;
    } // end IF
  }

  viewport_.Width = static_cast<float> (inherited::resolution_.cx);
  viewport_.Height = static_cast<float> (inherited::resolution_.cy);
  scissorRect_.right = inherited::resolution_.cx;
  scissorRect_.bottom = inherited::resolution_.cy;

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
void
Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataType,
                               SessionDataContainerType,
                               MediaType>::waitForPreviousFrame ()
{
  // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
  // This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
  // sample illustrates how to use fences for efficient resource usage and to
  // maximize GPU utilization.

  // Signal and increment the fence value.
  const UINT64 fence = fenceValue_;
  HRESULT result_2 = commandQueue_->Signal (fence_, fence);
  ACE_ASSERT (SUCCEEDED (result_2));
  fenceValue_++;

  // Wait until the previous frame is finished.
  if (fence_->GetCompletedValue () < fence)
  {
    result_2 = fence_->SetEventOnCompletion (fence, fenceEvent_);
    WaitForSingleObject (fenceEvent_, INFINITE);
  } // end IF

  frameIndex_ = swapChain_->GetCurrentBackBufferIndex ();
}
