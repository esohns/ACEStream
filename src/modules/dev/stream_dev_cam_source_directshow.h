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

#include <strmif.h>
#include <control.h>
#include <qedit.h>
#include <windef.h>

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
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          typename StatisticHandlerType,
          ////////////////////////////////
          typename UserDataType>
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
                                      StatisticHandlerType,
                                      UserDataType>
 , public IMemAllocatorNotifyCallbackTemp
 , public ISampleGrabberCB
{
 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Cam_Source_DirectShow_T (ISTREAM_T* = NULL,                                                         // stream handle
                                      bool = false,                                                              // auto-start ? (active mode only)
                                      enum Stream_HeadModuleConcurrency = STREAM_HEADMODULECONCURRENCY_PASSIVE); // concurrency mode
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
                                    StatisticHandlerType,
                                    UserDataType>::initialize;

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

  inline STDMETHODIMP_ (ULONG) AddRef () { InterlockedIncrement (&referenceCount_); return referenceCount_; };
  STDMETHODIMP_ (ULONG) Release ();
  STDMETHODIMP QueryInterface (REFIID,
                               void**);
  // implement IMemAllocatorNotifyCallbackTemp
  STDMETHODIMP NotifyRelease (void);
  // implement ISampleGrabberCB
  STDMETHODIMP BufferCB (double, // SampleTime
                         BYTE*,  // Buffer
                         long);  // BufferLen
  STDMETHODIMP SampleCB (double,         // SampleTime
                         IMediaSample*); // Sample

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
                                      StatisticHandlerType,
                                      UserDataType> inherited;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T (const Stream_Dev_Cam_Source_DirectShow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T& operator= (const Stream_Dev_Cam_Source_DirectShow_T&))

  // helper methods
  bool initialize_DirectShow (const std::string&,      // (source) device name (FriendlyName)
                              const HWND,              // (target) window handle [NULL: NullRenderer]
                              ICaptureGraphBuilder2*&, // return value: (capture) graph builder handle
                              IAMDroppedFrames*&,      // return value: capture filter statistic handle
                              ISampleGrabber*&);       // return value: sample grabber handle

  bool                   isFirst_;
  IAMDroppedFrames*      IAMDroppedFrames_;
  ICaptureGraphBuilder2* ICaptureGraphBuilder2_;
  IMediaControl*         IMediaControl_;
  IMediaEventEx*         IMediaEventEx_;
  DWORD                  ROTID_;
  ULONG                  referenceCount_;
};

// include template definition
#include "stream_dev_cam_source_directshow.inl"

#endif
