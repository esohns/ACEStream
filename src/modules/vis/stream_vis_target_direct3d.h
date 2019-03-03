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

#include "common_ui_ifullscreen.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

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
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Vis_Target_Direct3D_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_UI_IFullscreen
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Vis_Target_Direct3D_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_Direct3D_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&);
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

 protected:
  // helper methods
  HRESULT initialize_Direct3DDevice (HWND,                                                     // (target) window handle
                                     const struct _AMMediaType&,                               // (inbound) media type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                                     IDirect3DDevice9Ex*,                                      // Direct3D device handle
#else
                                     IDirect3DDevice9*,                                        // Direct3D device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                                     struct _D3DPRESENT_PARAMETERS_&,                          // in/out: Direct3D presentation parameters
                                     LONG&,                                                    // return value: stride
                                     struct tagRECT&);                                         // return value: destination rectangle
  bool initialize_Direct3D (struct Stream_MediaFramework_Direct3D_Configuration&, // in/out: configuration
                            const struct _AMMediaType&,                           // (inbound) media type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                            IDirect3DDevice9Ex*&,                                 // in/out: device handle
#else
                            IDirect3DDevice9*&,                                   // in/out: device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                            struct _D3DPRESENT_PARAMETERS_&,                      // in/out: Direct3D presentation parameters
                            LONG&,                                                // return value: stride
                            struct tagRECT&);                                     // return value: destination rectangle

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
                              bool&,               // return value: reset device ?
                              bool&);              // return value: destroy device ?
  // *NOTE*: behaviour depends on whether a device handle is passed in:
  //         yes: calls Reset(Ex) and 
  HRESULT resetDevice (const struct _AMMediaType&,                           // input media type
                       struct Stream_MediaFramework_Direct3D_Configuration&, // in/out: configuration
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
                       IDirect3DDevice9Ex*&,                                 // in/out: device handle
#else
                       IDirect3DDevice9*&,                                   // in/out: device handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
                       struct _D3DPRESENT_PARAMETERS_&,                      // return value: presentation parameters
                       LONG&,                                                // return value: stride
                       struct tagRECT&);                                     // return value: destination rectangle

  // *NOTE*: the 'current' window is presentationParameters_.hDeviceWindow
  HWND                                                 clientWindow_;
  bool                                                 closeWindow_;
  // *NOTE*: < 0 ? 'bottom-up' memory layout : 'top-down' memory layout
  //         see also: https://docs.microsoft.com/en-us/windows/desktop/medfound/image-stride
  //                   https://docs.microsoft.com/en-us/windows/desktop/directshow/top-down-vs--bottom-up-dibs
  LONG                                                 defaultStride_;
  struct tagRECT                                       destinationRectangle_;
  struct Stream_MediaFramework_Direct3D_Configuration* direct3DConfiguration_;
  enum Stream_MediaFramework_Type                      mediaFramework_;
  bool                                                 releaseDeviceHandle_; // configuration-
  bool                                                 resetMode_; // to desktop mode ?
  bool                                                 snapShotNextFrame_;
  // *NOTE*: this copies (!) the inbound image frame data from sample (virtual)
  //         memory to a Direct3D surface in (video) memory and converts the
  //         inbound (i.e. capture) format to RGB-32 for visualization
  // *TODO*: separate this two-step process (insert a MFT/DMO decoder filter)
  Stream_Vis_Target_Direct3D_TransformationCB          transformation_;

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
                                       SessionDataContainerType,
                                       MediaType> OWN_TYPE_T;

  // helper methods
  // *NOTE*: all image data needs to be transformed to RGB32
  bool isFormatSupported (REFGUID); // sub-type
  HRESULT setTransformation (REFGUID); // (inbound) sub-type
  HRESULT getFormat (DWORD,                // index
                     struct _GUID&) const; // return value: sub-type
//  HRESULT createSwapChain (HWND,                                  // (target) window handle
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//                           IDirect3DDevice9Ex*,                   // Direct3D device handle
//#else
//                           IDirect3DDevice9*,                     // Direct3D device handle
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
//                           const struct _D3DPRESENT_PARAMETERS_&, // presentation parameters
//                           REFGUID,                               // (input) media subtype
//                           IDirect3DSwapChain9*&);                // return value: Direct3D swap chain
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
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Vis_DirectShow_Target_Direct3D_T
 : public Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       MediaType>
{
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       MediaType> inherited;

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

// *NOTE*: this specialization 'unwraps' the IMFSample from the data message
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
          typename SessionDataContainerType,
          ////////////////////////////////
          typename MediaType>
class Stream_Vis_MediaFoundation_Target_Direct3D_T
 : public Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       MediaType>
{
  typedef Stream_Vis_Target_Direct3D_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       ConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       MediaType> inherited;

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

  enum _MFVideoInterlaceMode interlaceMode_;
  struct _MFRatio            pixelAspectRatio_;
};

// include template definition
#include "stream_vis_target_direct3d.inl"

#endif
