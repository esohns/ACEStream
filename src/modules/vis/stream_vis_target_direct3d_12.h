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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECT3D_12_T_H
#define STREAM_MODULE_VIS_TARGET_DIRECT3D_12_T_H

#include "dxgi1_4.h"
#include "d3d12.h"
#include "d3dx12.h"
#include "DirectXMath.h"
#include "guiddef.h"

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "stream_vis_target_win32_base.h"

extern const char libacestream_default_vis_direct3d12_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Vis_Target_Direct3D12_T
 : public Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType>
{
  typedef Stream_Vis_Target_Win32_Base_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType> inherited;

 public:
  typedef typename inherited::ISTREAM_T ISTREAM_T;
  Stream_Vis_Target_Direct3D12_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_Direct3D12_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D12_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D12_T (const Stream_Vis_Target_Direct3D12_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D12_T& operator= (const Stream_Vis_Target_Direct3D12_T&))

  // helper types
  struct Vertex
  {
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT2 uv;
  };

  typedef Stream_Vis_Target_Direct3D12_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         SessionDataType,
                                         SessionDataContainerType,
                                         MediaType> OWN_TYPE_T;

  // override (part of) ACE_Task_Base
  virtual int svc ();

  // helper methods
  bool initialize_Direct3D (HWND,     // (target-) window handle
                            REFGUID); // (input-) format
  void waitForPreviousFrame ();

  // pipeline objects
  CD3DX12_VIEWPORT           viewport_;
  CD3DX12_RECT               scissorRect_;
  IDXGISwapChain3*           swapChain_;
  ID3D12Device*              device_;
  ID3D12Resource*            renderTargets_[STREAM_VIS_RENDERER_VIDEO_DIRECTDRAW_3D_12_DEFAULT_FRAME_COUNT];
  ID3D12CommandAllocator*    commandAllocator_;
  ID3D12CommandQueue*        commandQueue_;
  ID3D12RootSignature*       rootSignature_;
  ID3D12DescriptorHeap*      rtvHeap_;
  ID3D12DescriptorHeap*      srvHeap_;
  ID3D12PipelineState*       pipelineState_;
  ID3D12GraphicsCommandList* commandList_;
  UINT                       rtvDescriptorSize_;

  // application resources
  ID3D12Resource*            vertexBuffer_;
  D3D12_VERTEX_BUFFER_VIEW   vertexBufferView_;
  ID3D12Resource*            texture_;

  // synchronization objects
  UINT                       frameIndex_;
  HANDLE                     fenceEvent_;
  ID3D12Fence*               fence_;
  UINT64                     fenceValue_;

  struct _GUID               format_;
};

// include template definition
#include "stream_vis_target_direct3d_12.inl"

#endif
