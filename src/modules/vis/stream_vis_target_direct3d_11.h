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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECT3D_11_T_H
#define STREAM_MODULE_VIS_TARGET_DIRECT3D_11_T_H

#include "d3d11.h"
#include "guiddef.h"

#include <string>

#include "ace/Global_Macros.h"

#include "stream_common.h"

#include "stream_vis_target_win32_base.h"

extern const char libacestream_default_vis_direct3d11_module_name_string[];

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
class Stream_Vis_Target_Direct3D11_T
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
  Stream_Vis_Target_Direct3D11_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_Direct3D11_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 protected:
  bool compileShaders (const std::string&, // (FQ) filename
                       ID3D11Device*);     // device handle

  ID3D11DeviceContext*      context_;
  ID3D11Device*             device_;
  struct _GUID              format_;
  ID3D11RenderTargetView*   renderTargetView_;
  ID3D11ShaderResourceView* shaderResourceView_;
  IDXGISwapChain*           swapChain_;
  ID3D11VertexShader*       vertexShader_;
  ID3D11PixelShader*        pixelShader_;
  ID3D11InputLayout*        inputLayout_;
  ID3D11Buffer*             vertexBuffer_;
  ID3D11Texture2D*          backBuffer_;
  ID3D11SamplerState*       samplerState_;
  ID3D11Texture2D*          texture_;
  ID3D11Texture2D*          depthStencilBuffer_;
  ID3D11DepthStencilView*   depthStencilView_;
  ID3D11DepthStencilState*  depthStencilState_;
  ID3D11RasterizerState*    rasterizerState_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D11_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D11_T (const Stream_Vis_Target_Direct3D11_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D11_T& operator= (const Stream_Vis_Target_Direct3D11_T&))

  // helper types
  typedef Stream_Vis_Target_Direct3D11_T<ACE_SYNCH_USE,
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
};

// include template definition
#include "stream_vis_target_direct3d_11.inl"

#endif
