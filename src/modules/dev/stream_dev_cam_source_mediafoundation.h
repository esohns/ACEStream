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

#ifndef STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_H
#define STREAM_DEV_CAM_SOURCE_MEDIAFOUNDATION_H

#include <string>

#include "d3d9.h"
#include "dxva2api.h"
#undef GetObject
#include "mfidl.h"
#include "mfreadwrite.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_dev_cam_source_mediafoundation_module_name_string[];

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename UserDataType,
          ////////////////////////////////
          typename MediaType>
class Stream_Dev_Cam_Source_MediaFoundation_T
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
 , public IMFSampleGrabberSinkCallback2
#else
 , public IMFSampleGrabberSinkCallback
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
 //, public IMFAsyncCallback
{
  typedef Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Cam_Source_MediaFoundation_T (ISTREAM_T* = NULL); // stream handle
  virtual ~Stream_Dev_Cam_Source_MediaFoundation_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType,
                                    TimerManagerType,
                                    UserDataType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

//  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement IMFSampleGrabberSinkCallback2
  STDMETHODIMP QueryInterface (const IID&,
                               void**);
  inline virtual ULONG STDMETHODCALLTYPE AddRef () { return InterlockedIncrement (&referenceCount_); }
  inline virtual ULONG STDMETHODCALLTYPE Release () { ULONG count = InterlockedDecrement (&referenceCount_); return count; }
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
  STDMETHODIMP OnProcessSample (REFGUID,     // major media type
                                DWORD,       // flags
                                LONGLONG,    // timestamp
                                LONGLONG,    // duration
                                const BYTE*, // buffer
                                DWORD);      // buffer size
  STDMETHODIMP OnProcessSampleEx (REFGUID,         // major media type
                                  DWORD,           // flags
                                  LONGLONG,        // timestamp
                                  LONGLONG,        // duration
                                  const BYTE*,     // buffer
                                  DWORD,           // buffer size
                                  IMFAttributes*); // media sample attributes
  STDMETHODIMP OnSetPresentationClock (IMFPresentationClock*); // presentation clock handle
  STDMETHODIMP OnShutdown ();
  //// implement IMFAsyncCallback
  //STDMETHODIMP GetParameters (DWORD*,  // return value: flags
  //                            DWORD*); // return value: queue handle
  //STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

 private:
  // comvenient types
  typedef Stream_Dev_Cam_Source_MediaFoundation_T<ACE_SYNCH_USE,
                                                  ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType,
                                                  ConfigurationType,
                                                  StreamControlType,
                                                  StreamNotificationType,
                                                  StreamStateType,
                                                  SessionDataType,          // session data
                                                  SessionDataContainerType, // session message payload (reference counted)
                                                  StatisticContainerType,
                                                  TimerManagerType,
                                                  UserDataType,
                                                  MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_MediaFoundation_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_MediaFoundation_T (const Stream_Dev_Cam_Source_MediaFoundation_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_MediaFoundation_T& operator= (const Stream_Dev_Cam_Source_MediaFoundation_T&))

  // helper methods
  // *NOTE*: (if any,) fire-and-forget the media source handle (third argument)
  bool initialize_MediaFoundation (const std::string&,             // (source) device name (FriendlyName)
                                   HWND,                           // (target) window handle [NULL: NullRenderer]
                                   IDirect3DDeviceManager9*,       // direct 3d manager handle
                                   IMFMediaType*,                  // media type handle
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                   IMFMediaSourceEx*&,             // media source handle (in/out)
#else
                                   IMFMediaSource*&,               // media source handle (in/out)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                   WCHAR*&,                        // return value: symbolic link
                                   UINT32&,                        // return value: symbolic link size
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                   IMFSampleGrabberSinkCallback2*, // grabber sink callback handle [NULL: do not use tee/grabber]
#else
                                   IMFSampleGrabberSinkCallback*,  // grabber sink callback handle [NULL: do not use tee/grabber]
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                   IMFTopology*&);                 // return value: topology handle

  LONGLONG              baseTimeStamp_;

  bool                  hasFinished_;
  bool                  isFirst_;

  WCHAR*                symbolicLink_;
  UINT32                symbolicLinkSize_;

  IMFPresentationClock* presentationClock_;
  long                  referenceCount_;
  TOPOID                sampleGrabberSinkNodeId_;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IMFMediaSession*      mediaSession_;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
};

// include template definition
#include "stream_dev_cam_source_mediafoundation.inl"

#endif
