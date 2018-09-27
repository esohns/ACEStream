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

#ifndef STREAM_DEV_CAM_SOURCE_DIRECTSHOW_H
#define STREAM_DEV_CAM_SOURCE_DIRECTSHOW_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include <BaseTyps.h>
#include <OAIdl.h>
#include <control.h>
#include <qedit.h>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

extern const char libacestream_default_dev_cam_source_directshow_module_name_string[];

template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          typename StatisticContainerType,
          typename TimerManagerType, // implements Common_ITimer
          typename UserDataType,
          ////////////////////////////////
          bool MediaSampleIsDataMessage = false>
class Stream_Dev_Cam_Source_DirectShow_T
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
 , public IMemAllocatorNotifyCallbackTemp
 , public ISampleGrabberCB
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

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Cam_Source_DirectShow_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Cam_Source_DirectShow_T ();

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

  // implement (part of) Stream_ITaskBase
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)

  inline STDMETHODIMP_ (ULONG) AddRef () { InterlockedIncrement (&referenceCount_); return referenceCount_; }
  STDMETHODIMP_ (ULONG) Release ();
  STDMETHODIMP QueryInterface (REFIID,
                               LPVOID*);
  // implement IMemAllocatorNotifyCallbackTemp
  inline STDMETHODIMP NotifyRelease (void) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (E_FAIL); ACE_NOTREACHED (return E_FAIL;) }
  // implement ISampleGrabberCB
  inline STDMETHODIMP BufferCB (double, BYTE*, long) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (E_FAIL); ACE_NOTREACHED (return E_FAIL;) }
  STDMETHODIMP SampleCB (double,         // SampleTime
                         IMediaSample*); // Sample

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T (const Stream_Dev_Cam_Source_DirectShow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T& operator= (const Stream_Dev_Cam_Source_DirectShow_T&))

  // helper methods
  bool initialize_DirectShow (const std::string&,      // (source) device path
                              HWND,                    // (target) window handle [NULL: NullRenderer]
                              ICaptureGraphBuilder2*&, // return value: (capture) graph builder handle
                              IAMVideoControl*&,       // return value; capture filter video control
                              IAMDroppedFrames*&,      // return value: capture filter statistic handle
                              ISampleGrabber*&);       // return value: sample grabber handle

  bool                   isFirst_;
  IAMDroppedFrames*      IAMDroppedFrames_;
  IAMVideoControl*       IAMVideoControl_;
  ICaptureGraphBuilder2* ICaptureGraphBuilder2_;
  IMediaControl*         IMediaControl_;
  IMediaEventEx*         IMediaEventEx_;
  DWORD                  ROTID_;
  ULONG                  referenceCount_;
};

// include template definition
#include "stream_dev_cam_source_directshow.inl"

#endif
