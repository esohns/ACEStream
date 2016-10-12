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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECT3D_H
#define STREAM_MODULE_VIS_TARGET_DIRECT3D_H

#include <ace/Global_Macros.h>

#include <d3d9.h>
#include <evr.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_task_base_synch.h"

typedef void (*STREAM_VIS_TARGET_DIRECT3D_ADAPTER_T) (BYTE*,       // destination
                                                      LONG,        // destination stride
                                                      const BYTE*, // source
                                                      LONG,        // source stride
                                                      DWORD,       // width
                                                      DWORD);      // height
struct STREAM_VIS_TARGET_DIRECT3D_CONVERSION_T
{
  struct _GUID                         subType;
  STREAM_VIS_TARGET_DIRECT3D_ADAPTER_T adapter;
};

void TransformImage_RGB24 (BYTE*,
                           LONG,
                           const BYTE*,
                           LONG,
                           DWORD,
                           DWORD);
void TransformImage_RGB32 (BYTE*,
                           LONG,
                           const BYTE*,
                           LONG,
                           DWORD,
                           DWORD);
void TransformImage_YUY2 (BYTE*,
                          LONG,
                          const BYTE*,
                          LONG,
                          DWORD,
                          DWORD);
void TransformImage_NV12 (BYTE*,
                          LONG,
                          const BYTE*,
                          LONG,
                          DWORD,
                          DWORD);

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
          typename SessionDataContainerType>
class Stream_Vis_Target_Direct3D_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType>
{
 public:
  Stream_Vis_Target_Direct3D_T ();
  virtual ~Stream_Vis_Target_Direct3D_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
  // *NOTE*: (on success,) this sets the MF_MT_DEFAULT_STRIDE in the media type
  HRESULT initialize_Direct3DDevice (HWND,                        // (target) window handle
                                     const struct _AMMediaType&); // media type handle
  // *NOTE*: (on success,) this sets the MF_MT_DEFAULT_STRIDE in the media type
  bool initialize_Direct3D (HWND,                                   // (target) window handle
                            const struct _AMMediaType&,             // media type handle
                            IDirect3DDevice9Ex*&,                   // return value: Direct3D device handle
                            struct _D3DPRESENT_PARAMETERS_&,        // return value: Direct3D presentation parameters
                            // *NOTE*: input (capture) format --> RGB-32 transformation
                            STREAM_VIS_TARGET_DIRECT3D_ADAPTER_T&); // return value: transformation function pointer

  // *NOTE*: takes a source rectangle and constructs the largest possible
  //         centered rectangle within the specified destination rectangle such
  //         that the image maintains its current aspect ratio. This function
  //         assumes that pixels are the same shape within both the source and
  //         destination rectangles
  struct tagRECT letterbox_rectangle (const struct tagRECT&,  // source rectangle
                                      const struct tagRECT&); // destination rectangle
  HRESULT test_cooperative_level ();

  bool                                 closeWindow_;
  LONG                                 defaultStride_;
  struct tagRECT                       destinationRectangle_;
  struct _D3DPRESENT_PARAMETERS_       presentationParameters_;
  LONG                                 height_;
  LONG                                 width_;
  HWND                                 window_;

  // *NOTE*: this copies (!) the inbound image frame data from sample (virtual)
  //         memory to a Direct3D surface in (video) memory and converts the
  //         inbound (i.e. capture) format to RGB-32 for visualization
  // *TODO*: separate this two-step process (insert a MFT/DMO decoder filter)
  STREAM_VIS_TARGET_DIRECT3D_ADAPTER_T adapter_;
  IDirect3DDevice9Ex*                  IDirect3DDevice9Ex_;
  IDirect3DSwapChain9*                 IDirect3DSwapChain9_;

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D_T (const Stream_Vis_Target_Direct3D_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D_T& operator= (const Stream_Vis_Target_Direct3D_T&))

  // helper types
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> OWN_TYPE_T;
  static STREAM_VIS_TARGET_DIRECT3D_CONVERSION_T formatConversions[];
  static const DWORD                             formats;

  // helper methods
  HRESULT set_adapter (REFGUID); // (inbound) sub-type
  HRESULT get_format (DWORD,                // index
                      struct _GUID&) const; // return value: sub-type
  bool is_supported (REFGUID); // sub-type
  HRESULT create_swap_chains (HWND,     // (target) window handle
                              UINT32,   // width
                              UINT32,   // height
                              REFGUID); // input subtype
  virtual void update_destination_rectangle ();
  HRESULT reset_device ();

  // format information
  enum _D3DFORMAT                      format_;
};

//////////////////////////////////////////

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
          typename SessionDataContainerType>
class Stream_Vis_DirectShow_Target_Direct3D_T
 : public Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType>
{
 public:
  Stream_Vis_DirectShow_Target_Direct3D_T ();
  virtual ~Stream_Vis_DirectShow_Target_Direct3D_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

 private:
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_DirectShow_Target_Direct3D_T (const Stream_Vis_DirectShow_Target_Direct3D_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_DirectShow_Target_Direct3D_T& operator= (const Stream_Vis_DirectShow_Target_Direct3D_T&))
};

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
          typename SessionDataContainerType>
class Stream_Vis_MediaFoundation_Target_Direct3D_T
 : public Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType>
{
 public:
  Stream_Vis_MediaFoundation_Target_Direct3D_T ();
  virtual ~Stream_Vis_MediaFoundation_Target_Direct3D_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_MediaFoundation_Target_Direct3D_T (const Stream_Vis_MediaFoundation_Target_Direct3D_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_MediaFoundation_Target_Direct3D_T& operator= (const Stream_Vis_MediaFoundation_Target_Direct3D_T&))

  // helper methods
  // *NOTE*: (on success,) this sets the MF_MT_DEFAULT_STRIDE in the media type
  HRESULT get_default_stride (IMFMediaType*, // media type handle
                              LONG&);        // return value: default stride
  // *NOTE*: transforms a rectangle from its current pixel aspect ratio (PAR) to
  //         1:1 PAR. For example, a 720 x 486 rectangle with a PAR of 9:10,
  //         when converted to 1:1 PAR, is stretched to 720 x 540
  struct tagRECT normalize_aspect_ratio (const struct tagRECT&,   // rectangle
                                         const struct _MFRatio&); // pixel aspect ratio
  virtual void update_destination_rectangle ();

  enum _MFVideoInterlaceMode interlaceMode_;
  struct _MFRatio            pixelAspectRatio_;
};

// include template definition
#include "stream_vis_target_direct3d.inl"

#endif
