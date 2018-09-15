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

#ifndef STREAM_MODULE_VIS_TARGET_DIRECT3D_T_H
#define STREAM_MODULE_VIS_TARGET_DIRECT3D_T_H

#include <d3d9.h>
#include <guiddef.h>
#include <mfobjects.h>
#include <strmif.h>

#include "ace/Global_Macros.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_vis_direct3d_module_name_string[];

typedef void (*Stream_Vis_Target_Direct3D_TransformationCB) (BYTE*,       // destination
                                                             LONG,        // destination stride
                                                             const BYTE*, // source
                                                             LONG,        // source stride
                                                             DWORD,       // width
                                                             DWORD);      // height
// *NOTE*: must be an 'aggregate' type to support 'list initialization'
struct Stream_Vis_Target_Direct3D_Transformation
{
  struct _GUID                                subType;
  Stream_Vis_Target_Direct3D_TransformationCB transformationCB;
};

void libacestream_vis_transform_image_RGB24 (BYTE*, LONG, const BYTE*, LONG, DWORD, DWORD);
void libacestream_vis_transform_image_RGB32 (BYTE*, LONG, const BYTE*, LONG, DWORD, DWORD);
void libacestream_vis_transform_image_YUY2 (BYTE*, LONG, const BYTE*, LONG, DWORD, DWORD);
void libacestream_vis_transform_image_NV12 (BYTE*, LONG, const BYTE*, LONG, DWORD, DWORD);

extern struct Stream_Vis_Target_Direct3D_Transformation libacestream_vis_directshow_format_transformations[];
extern struct Stream_Vis_Target_Direct3D_Transformation libacestream_vis_mediafoundation_format_transformations[];
extern DWORD                                            libacestream_vis_number_of_format_transformations;

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
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  Stream_Vis_Target_Direct3D_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_Direct3D_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // helper methods
  HRESULT initialize_Direct3DDevice (const struct Stream_Module_Device_Direct3DConfiguration&, // configuration
                                     HWND,                                                     // (target) window handle
                                     const struct _AMMediaType&,                               // media type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                     IDirect3DDevice9Ex*,                                      // Direct3D device handle
#else
                                     IDirect3DDevice9*,                                        // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                     LONG&,                                                    // return value: width
                                     LONG&,                                                    // return value: height
                                     LONG&,                                                    // return value: stride
                                     struct tagRECT&);                                         // return value: destination rectangle
  bool initialize_Direct3D (const struct Stream_Module_Device_Direct3DConfiguration&, // configuration
                            HWND,                                                     // (target) window handle
                            const struct _AMMediaType&,                               // (inbound) media type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                            IDirect3DDevice9Ex*&,                                     // return value: Direct3D device handle
#else
                            IDirect3DDevice9*&,                                       // return value: Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                            struct _D3DPRESENT_PARAMETERS_&,                          // return value: Direct3D presentation parameters
                            LONG&,                                                    // return value: width
                            LONG&,                                                    // return value: height
                            LONG&,                                                    // return value: stride
                            struct tagRECT&);                                         // return value: destination rectangle

  // *NOTE*: takes a source rectangle and constructs the largest possible
  //         centered rectangle within the specified destination rectangle such
  //         that the image maintains its current aspect ratio. This function
  //         assumes that pixels are the same shape within both the source and
  //         destination rectangles
  struct tagRECT letterbox_rectangle (const struct tagRECT&,  // source rectangle
                                      const struct tagRECT&); // destination rectangle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  void checkCooperativeLevel (IDirect3DDevice9Ex*, // Direct3D device handle
#else
  void checkCooperativeLevel (IDirect3DDevice9*,   // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                              bool&);              // return value: reset device ?
  HRESULT resetDevice (const struct Stream_Module_Device_Direct3DConfiguration&, // Direct3D configuration
                       HWND,                                                     // (target) window handle
                       struct _D3DPRESENT_PARAMETERS_&,                          // in/out: Direct3D presentation parameters
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                       struct D3DDISPLAYMODEEX&,                                 // in/out: Direct3D fullscreen display mode
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                       LONG&,                                                    // in/out width
                       LONG&,                                                    // in/out height
                       LONG&,                                                    // return value: stride
                       const struct _AMMediaType&,                               // media type
                       enum _D3DFORMAT,                                          // Direct3D format
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                       IDirect3DDevice9Ex*&,                                     // return value: Direct3D device handle
#else
                       IDirect3DDevice9*&,                                       // return value: Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                       IDirect3DSwapChain9*&,                                    // return value: Direct3D swap chain handle
                       struct tagRECT&);                                         // return value:: destination rectangle

  bool                                               closeWindow_;
  // *NOTE*: < 0 ? 'bottom-up' memory layout : 'top-down' memory layout
  //         see also: https://docs.microsoft.com/en-us/windows/desktop/medfound/image-stride
  //                   https://docs.microsoft.com/en-us/windows/desktop/directshow/top-down-vs--bottom-up-dibs
  LONG                                               defaultStride_;
  struct tagRECT                                     destinationRectangle_;
  struct Stream_Module_Device_Direct3DConfiguration* direct3DConfiguration_;
  enum _D3DFORMAT                                    format_;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  struct D3DDISPLAYMODEEX                            fullscreenDisplayMode_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  struct _D3DPRESENT_PARAMETERS_                     presentationParameters_;
  LONG                                               height_;
  LONG                                               width_;
  HWND                                               window_;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IDirect3DDevice9Ex*                                deviceHandle_;
#else
  IDirect3DDevice9*                                  deviceHandle_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  IDirect3DSwapChain9*                               swapChain_;
  // *NOTE*: this copies (!) the inbound image frame data from sample (virtual)
  //         memory to a Direct3D surface in (video) memory and converts the
  //         inbound (i.e. capture) format to RGB-32 for visualization
  // *TODO*: separate this two-step process (insert a MFT/DMO decoder filter)
  Stream_Vis_Target_Direct3D_TransformationCB        transformation_;

  enum Stream_MediaFramework_Type                    mediaFramework_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_Direct3D_T ())
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

  // helper methods
  // *NOTE*: all image data needs to be transformed to RGB32
  bool isFormatSupported (REFGUID); // sub-type
  HRESULT setTransformation (REFGUID); // (inbound) sub-type
  HRESULT getFormat (DWORD,                // index
                     struct _GUID&) const; // return value: sub-type
  HRESULT createSwapChain (HWND,                                  // (target) window handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                           IDirect3DDevice9Ex*,                   // Direct3D device handle
#else
                           IDirect3DDevice9*,                     // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                           const struct _D3DPRESENT_PARAMETERS_&, // presentation parameters
                           REFGUID,                               // (input) media subtype
                           IDirect3DSwapChain9*&);                // return value: Direct3D swap chain
  virtual void updateDestinationRectangle (HWND,                  // (target) window handle
                                           const struct tagRECT&, // source rectangle
                                           struct tagRECT&);      // destination rectangle
};

//////////////////////////////////////////

// *NOTE*: this specialization 'unwraps' the IMediaSample from the data message
//         before presentation
// *TODO*: strategize behaviourisms into template traits
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
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> inherited;

 public:
  Stream_Vis_DirectShow_Target_Direct3D_T (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Vis_DirectShow_Target_Direct3D_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_DirectShow_Target_Direct3D_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_DirectShow_Target_Direct3D_T (const Stream_Vis_DirectShow_Target_Direct3D_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_DirectShow_Target_Direct3D_T& operator= (const Stream_Vis_DirectShow_Target_Direct3D_T&))
};

// *NOTE*: this 'specialization' merely 'unwraps' the IMFSample from the data
//         message before presentation
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
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType> inherited;

 public:
  Stream_Vis_MediaFoundation_Target_Direct3D_T (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Vis_MediaFoundation_Target_Direct3D_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_MediaFoundation_Target_Direct3D_T ())
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
