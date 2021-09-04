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

#ifndef STREAM_LIB_MEDIAFOUNDATION_TARGET_H
#define STREAM_LIB_MEDIAFOUNDATION_TARGET_H

#include <d3d9.h>
#include <dxva2api.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

 //#include "common_time_common.h"

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
          typename SessionDataType,          // session data
          typename SessionDataContainerType> // session message payload (reference counted)
class Stream_MediaFramework_MediaFoundation_Target_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE, 
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 //, public Stream_IModuleHandler_T<ConfigurationType>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
 , public IMFSampleGrabberSinkCallback2
#else
 , public IMFSampleGrabberSinkCallback
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE, 
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  //// convenience types
  //typedef Common_IInitialize_T<ConfigurationType> IINITIALIZE_T;

  Stream_MediaFramework_MediaFoundation_Target_T (ISTREAM_T*); // stream handle
  virtual ~Stream_MediaFramework_MediaFoundation_Target_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  //// implement Stream_IModuleHandler_T
  //virtual const ConfigurationType& get () const;

  // implement IMFSampleGrabberSinkCallback2
  virtual STDMETHODIMP QueryInterface (const IID&,
                                       void**);
  virtual STDMETHODIMP_ (ULONG) AddRef ();
  virtual STDMETHODIMP_ (ULONG) Release ();
  //STDMETHODIMP OnEvent (DWORD,           // stream index
  //                      IMFMediaEvent*); // event handle
  //STDMETHODIMP OnFlush (DWORD); // stream index
  //STDMETHODIMP OnReadSample (HRESULT,     // result
  //                           DWORD,       // stream index
  //                           DWORD,       // stream flags
  //                           LONGLONG,    // timestamp
  //                           IMFSample*); // sample handle
  virtual STDMETHODIMP OnClockStart (MFTIME,    // (system) clock start time
                                     LONGLONG); // clock start offset
  virtual STDMETHODIMP OnClockStop (MFTIME); // (system) clock start time
  virtual STDMETHODIMP OnClockPause (MFTIME); // (system) clock pause time
  virtual STDMETHODIMP OnClockRestart (MFTIME); // (system) clock restart time
  virtual STDMETHODIMP OnClockSetRate (MFTIME, // (system) clock rate set time
                                       float); // new playback rate
  virtual STDMETHODIMP OnProcessSample (REFGUID,     // major media type
                                        DWORD,       // flags
                                        LONGLONG,    // timestamp
                                        LONGLONG,    // duration
                                        const BYTE*, // buffer
                                        DWORD);      // buffer size
  virtual STDMETHODIMP OnProcessSampleEx (REFGUID,         // major media type
                                          DWORD,           // flags
                                          LONGLONG,        // timestamp
                                          LONGLONG,        // duration
                                          const BYTE*,     // buffer
                                          DWORD,           // buffer size
                                          IMFAttributes*); // media sample attributes
  virtual STDMETHODIMP OnSetPresentationClock (IMFPresentationClock*); // presentation clock handle
  virtual STDMETHODIMP OnShutdown ();

 protected:
  SessionDataContainerType* sessionData_;

 private:
  // convenient types
  typedef Stream_MediaFramework_MediaFoundation_Target_T<ACE_SYNCH_USE,
                                                         TimePolicyType,
                                                         ConfigurationType,
                                                         ControlMessageType,
                                                         DataMessageType,
                                                         SessionMessageType,
                                                         SessionDataType,
                                                         SessionDataContainerType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Target_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Target_T (const Stream_MediaFramework_MediaFoundation_Target_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_MediaFoundation_Target_T& operator= (const Stream_MediaFramework_MediaFoundation_Target_T&))

  // helper methods
  bool initialize_MediaFoundation (IMFMediaType*,                  // sample grabber sink input media type handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                   IMFSampleGrabberSinkCallback2*, // sample grabber sink callback handle
#else
                                   IMFSampleGrabberSinkCallback*,  // sample grabber sink callback handle
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                   TOPOID&,                        // return value: node id
                                   IMFMediaSession*&);             // intput/return value: media session handle
  inline void finalize_MediaFoundation () {}

  bool                      isFirst_;

  LONGLONG                  baseTimeStamp_;
  IMFMediaSession*          mediaSession_;
  IMFPresentationClock*     presentationClock_;
  long                      referenceCount_;
  TOPOID                    sampleGrabberSinkNodeId_;
};

// include template definition
#include "stream_lib_mediafoundation_target.inl"

#endif
