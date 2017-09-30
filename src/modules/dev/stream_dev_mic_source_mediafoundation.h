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

#ifndef STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_H
#define STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_H

#include <string>

#include <mfidl.h>
#include <mfreadwrite.h>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

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
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType> // implements Common_ITimer
class Stream_Dev_Mic_Source_MediaFoundation_T
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
                                      struct Stream_UserData>
 //, public IMFSampleGrabberSinkCallback
 , public IMFSampleGrabberSinkCallback2
 //, public IMFAsyncCallback
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Mic_Source_MediaFoundation_T (ISTREAM_T* = NULL,                                                         // stream handle
                                           bool = false,                                                              // auto-start ? (active mode only)
                                           enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE); // concurrency mode
  virtual ~Stream_Dev_Mic_Source_MediaFoundation_T ();

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
                                    struct Stream_UserData>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

//  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
//                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement IMFSampleGrabberSinkCallback2
  STDMETHODIMP QueryInterface (const IID&,
                               void**);
  inline ULONG STDMETHODCALLTYPE AddRef () { return InterlockedIncrement (&referenceCount_); };
  inline ULONG STDMETHODCALLTYPE Release () { return InterlockedDecrement (&referenceCount_); };
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
                                  IMFAttributes*);     // media sample attributes
  STDMETHODIMP OnSetPresentationClock (IMFPresentationClock*); // presentation clock handle
  STDMETHODIMP OnShutdown ();
  //// implement IMFAsyncCallback
  //STDMETHODIMP GetParameters (DWORD*,  // return value: flags
  //                            DWORD*); // return value: queue handle
  //STDMETHODIMP Invoke (IMFAsyncResult*); // asynchronous result handle

 private:
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
                                      struct Stream_UserData> inherited;
  typedef IMFSampleGrabberSinkCallback2 inherited2;

  typedef Stream_Dev_Mic_Source_MediaFoundation_T<ACE_SYNCH_USE,
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
                                                  TimerManagerType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_MediaFoundation_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_MediaFoundation_T (const Stream_Dev_Mic_Source_MediaFoundation_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_MediaFoundation_T& operator= (const Stream_Dev_Mic_Source_MediaFoundation_T&))

  //virtual int svc (void);

  // helper methods
  // *NOTE*: (if any,) fire-and-forget the media source handle (third argument)
  bool initialize_MediaFoundation (const std::string&,                  // (source) device name (FriendlyName)
                                   int,                                 // (target) output handle [0: none]
                                   const IMFMediaType*,                 // media type handle
                                   IMFMediaSource*&,                    // media source handle (in/out)
                                   const IMFSampleGrabberSinkCallback*, // grabber sink callback handle [NULL: do not use tee/grabber]
                                   IMFTopology*&);                      // return value: topology handle

  LONGLONG              baseTimeStamp_;

  bool                  hasFinished_;
  bool                  isFirst_;

  WCHAR*                symbolicLink_;
  UINT32                symbolicLinkSize_;

  IMFPresentationClock* presentationClock_;
  long                  referenceCount_;
  TOPOID                sampleGrabberSinkNodeId_;

  IMFMediaSession*      mediaSession_;
  bool                  releaseSessionSession_;
};

// include template definition
#include "stream_dev_mic_source_mediafoundation.inl"

#endif
