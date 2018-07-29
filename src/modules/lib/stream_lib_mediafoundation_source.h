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

#ifndef STREAM_LIB_MEDIAFOUNDATION_SOURCE_H
#define STREAM_LIB_MEDIAFOUNDATION_SOURCE_H

#include <d3d9.h>
#include <dxva2api.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataContainerType,
          typename SessionDataType,
          typename MediaType,
          ////////////////////////////////
          typename UserDataType>
class Stream_MediaFramework_MediaFoundation_Source_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
 , public IMFSampleGrabberSinkCallback2
#else
 , public IMFSampleGrabberSinkCallback
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
{
 public:
  //// convenience types
  //typedef Common_IInitialize_T<ConfigurationType> IINITIALIZE_T;

  Stream_MediaFramework_MediaFoundation_Source_T ();
  virtual ~Stream_MediaFramework_MediaFoundation_Source_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  //// implement Stream_IModuleHandler_T
  //virtual const ConfigurationType& get () const;

  // implement IMFSampleGrabberSinkCallback2
  STDMETHODIMP QueryInterface (const IID&,
                               void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return InterlockedIncrement (&referenceCount_); };
  virtual STDMETHODIMP_ (ULONG) Release ();
  //STDMETHODIMP OnEvent (DWORD,           // stream index
  //                      IMFMediaEvent*); // event handle
  //STDMETHODIMP OnFlush (DWORD); // stream index
  //STDMETHODIMP OnReadSample (HRESULT,     // result
  //                           DWORD,       // stream index
  //                           DWORD,       // stream flags
  //                           LONGLONG,    // timestamp
  //                           IMFSample*); // sample handle
  STDMETHODIMP OnClockStart (MFTIME,    // (system) clock start time
                             LONGLONG); // clock start offset
  STDMETHODIMP OnClockStop (MFTIME); // (system) clock start time
  STDMETHODIMP OnClockPause (MFTIME); // (system) clock pause time
  STDMETHODIMP OnClockRestart (MFTIME); // (system) clock restart time
  STDMETHODIMP OnClockSetRate (MFTIME, // (system) clock rate set time
                               float); // new playback rate
  STDMETHODIMP OnProcessSample (const struct _GUID&, // major media type
                                DWORD,               // flags
                                LONGLONG,            // timestamp
                                LONGLONG,            // duration
                                const BYTE*,         // buffer
                                DWORD);              // buffer size
  STDMETHODIMP OnProcessSampleEx (const struct _GUID&, // major media type
                                  DWORD,               // flags
                                  LONGLONG,            // timestamp
                                  LONGLONG,            // duration
                                  const BYTE*,         // buffer
                                  DWORD,               // buffer size
                                  IMFAttributes*);     // attributes handle
  STDMETHODIMP OnSetPresentationClock (IMFPresentationClock*); // presentation clock handle
  STDMETHODIMP OnShutdown ();

 protected:
  SessionDataType*      sessionData_;

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

  // convenient types
  typedef Stream_MediaFramework_MediaFoundation_Source_T<ACE_SYNCH_USE,
                                               TimePolicyType,
                                               ConfigurationType,
                                               ControlMessageType,
                                               DataMessageType,
                                               SessionMessageType,
                                               SessionDataContainerType,
                                               SessionDataType,
                                               MediaType,
                                               UserDataType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Source_T (const Stream_MediaFramework_MediaFoundation_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Source_T& operator= (const Stream_MediaFramework_MediaFoundation_Source_T&))

  // helper methods
  DataMessageType* allocateMessage (unsigned int); // (requested) size
  bool initialize_MediaFoundation (const HWND,                           // (target) window handle [NULL: NullRenderer]
                                   const IMFMediaType*,                  // media type handle
                                   IMFMediaSource*&,                     // media source handle (in/out)
                                   WCHAR*&,                              // return value: symbolic link
                                   UINT32&,                              // return value: symbolic link size
                                   const IDirect3DDeviceManager9*,       // Direct3D device manager handle
                                   const IMFSampleGrabberSinkCallback2*, // grabber sink callback handle [NULL: do not use tee/grabber]
                                   TOPOID&,                              // return value: sample grabber sink node id
                                   TOPOID&,                              // return value: EVR sink node id
                                   IMFMediaSession*&);                   // input/return value: media session handle
  void finalize_MediaFoundation ();

  bool                  isFirst_;

  LONGLONG              baseTimeStamp_;
  IMFMediaSession*      mediaSession_;
  IMFPresentationClock* presentationClock_;
  volatile long         referenceCount_;
};

// include template definition
#include "stream_lib_mediafoundation_source.inl"

#endif
