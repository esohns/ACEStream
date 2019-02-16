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

#ifndef STREAM_MODULE_VIS_TARGET_MEDIAFOUNDATION_H
#define STREAM_MODULE_VIS_TARGET_MEDIAFOUNDATION_H

#include <d3d9.h>
#include <evr.h>
#include <Guiddef.h>
#include <mfidl.h>
#include <mfobjects.h>
#include <PropIdl.h>
#include <strmif.h>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"
#include "common_time_common.h"

#include "common_ui_ifullscreen.h"
#include "common_ui_windowtype_converter.h"

#include "stream_common.h"
#include "stream_iallocator.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediafoundation_target.h"

extern const char libacestream_default_vis_mediafoundation_module_name_string[];

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
          typename UserDataType>
class Stream_Vis_Target_MediaFoundation_T
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
                                 UserDataType>
 , public Common_UI_WindowTypeConverter_T<HWND>
 , public Common_UI_IFullscreen
 , public Common_IInitialize_T<struct _AMMediaType>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
 , public IMFMediaSourceEx
#else
 , public IMFMediaSource
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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
                                 UserDataType> inherited;
  typedef Common_UI_WindowTypeConverter_T<HWND> inherited2;

 public:
  Stream_Vis_Target_MediaFoundation_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Vis_Target_MediaFoundation_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_UI_IFullscreen
  virtual void toggle ();

  // implement Common_IInitialize_T
  // *NOTE*: this allocates the media source presentation descriptor
  virtual bool initialize (const struct _AMMediaType&); // media type

  // implement IMFMediaSourceEx (IUnknown, IMediaEventGenerator, IMFMediaSource)
  STDMETHODIMP QueryInterface (const IID&, // IID
                               void**);
  inline ULONG STDMETHODCALLTYPE AddRef () { return InterlockedIncrement (&referenceCount_); }
  inline ULONG STDMETHODCALLTYPE Release () { return InterlockedDecrement (&referenceCount_); }

  STDMETHODIMP BeginGetEvent (IMFAsyncCallback*, // callback handle
                              IUnknown*);        // callback state object handle
  STDMETHODIMP EndGetEvent (IMFAsyncResult*,  // result handle
                            IMFMediaEvent**); // return value: event handle
  STDMETHODIMP GetEvent (DWORD,            // flags
                         IMFMediaEvent**); // return value: event handle
  STDMETHODIMP QueueEvent (MediaEventType,                // event type
                           REFGUID,                       // extended type
                           HRESULT,                       // status
                           const struct tagPROPVARIANT*); // event value
  STDMETHODIMP RemoteBeginGetEvent (IMFRemoteAsyncCallback*); // callback handle
  STDMETHODIMP RemoteEndGetEvent (IUnknown*, // result handle
                                  DWORD*,
                                  BYTE**);

  STDMETHODIMP CreatePresentationDescriptor (IMFPresentationDescriptor**); // return value: presentation descriptor
  STDMETHODIMP GetCharacteristics (DWORD*); // return value: characteristics
  STDMETHODIMP Pause ();
  STDMETHODIMP RemoteCreatePresentationDescriptor (DWORD*,
                                                   BYTE**,
                                                   IMFPresentationDescriptor**); // return value: presentation descriptor handle
  STDMETHODIMP Shutdown ();
  STDMETHODIMP Start (IMFPresentationDescriptor*,    // presentation descriptor handle
                      const struct _GUID*,           // time format
                      const struct tagPROPVARIANT*); // start position
  STDMETHODIMP Stop ();

  STDMETHODIMP GetSourceAttributes (IMFAttributes**); // return value: attributes handle
  STDMETHODIMP GetStreamAttributes (DWORD,            // stream identifier
                                    IMFAttributes**); // return value: attributes handle
  STDMETHODIMP SetD3DManager (IUnknown*); // handle to the DXGI manager

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_MediaFoundation_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_MediaFoundation_T (const Stream_Vis_Target_MediaFoundation_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_MediaFoundation_T& operator= (const Stream_Vis_Target_MediaFoundation_T&))

  typedef Stream_Vis_Target_MediaFoundation_T<ACE_SYNCH_DECL,
                                              TimePolicyType,
                                              ConfigurationType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              SessionDataType,
                                              SessionDataContainerType,
                                              UserDataType> OWN_TYPE_T;

  // helper methods
  bool initialize_Session (HWND,                      // (target) window handle
                           const struct tagRECT&,     // (target) window area
                           TOPOID,                    // renderer node id
                           IMFMediaSink*&,            // return value: media sink handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                           IMFVideoDisplayControl2*&, // return value: video display control handle
#else
                           IMFVideoDisplayControl*&,  // return value: video display control handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                           //IMFVideoSampleAllocator*&, // return value: video sample allocator handle
                           IMFMediaSession*);         // media session handle

//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//  IDirect3DDevice9Ex*        direct3DDevice_;
//#else
//  IDirect3DDevice9*          direct3DDevice_;
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  IMFMediaSession*           mediaSession_;
  IMFPresentationDescriptor* presentationDescriptor_;
  long                       referenceCount_;
  //IMFStreamSink*             streamSink_;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  //IMFVideoDisplayControl2*   videoDisplayControl_;
#else
  //IMFVideoDisplayControl*    videoDisplayControl_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  ////IMFVideoSampleAllocator* videoSampleAllocator_;
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
class Stream_Vis_Target_MediaFoundation_2
 : public Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                                         TimePolicyType,
                                                         ConfigurationType,
                                                         ControlMessageType,
                                                         DataMessageType,
                                                         SessionMessageType,
                                                         SessionDataType,
                                                         SessionDataContainerType>
{
  typedef Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                                         TimePolicyType,
                                                         ConfigurationType,
                                                         ControlMessageType,
                                                         DataMessageType,
                                                         SessionMessageType,
                                                         SessionDataType,
                                                         SessionDataContainerType> inherited;

 public:
  Stream_Vis_Target_MediaFoundation_2 (ISTREAM_T*); // stream handle
  inline virtual ~Stream_Vis_Target_MediaFoundation_2 () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  //// implement Stream_IModuleHandler_T
  //virtual const ConfigurationType& get () const;

  //// override (part of) IMFSampleGrabberSinkCallback2
  //STDMETHODIMP OnProcessSampleEx (REFGUID,         // major media type
  //                                DWORD,           // flags
  //                                LONGLONG,        // timestamp
  //                                LONGLONG,        // duration
  //                                const BYTE*,     // buffer
  //                                DWORD,           // buffer size
  //                                IMFAttributes*); // media sample attributes

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_MediaFoundation_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_MediaFoundation_2 (const Stream_Vis_Target_MediaFoundation_2&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Vis_Target_MediaFoundation_2& operator= (const Stream_Vis_Target_MediaFoundation_2&))
};

// include template definition
#include "stream_vis_target_mediafoundation.inl"

#endif
