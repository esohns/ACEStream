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

#include "dshow.h"
#include "qedit.h"

#include "common_istatistic.h"
#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

template <typename LockType,
          ///////////////////////////////
          typename SessionMessageType,
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename StreamStateType,
          ///////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ///////////////////////////////
          typename StatisticContainerType>
class Stream_Dev_Cam_Source_DirectShow_T
 : public Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType>
 , public Common_IStatistic_T<StatisticContainerType>
 , public IMemAllocatorNotifyCallbackTemp
 , public ISampleGrabberCB
{
 public:
  Stream_Dev_Cam_Source_DirectShow_T ();
  virtual ~Stream_Dev_Cam_Source_DirectShow_T ();

  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<LockType,
                                    ACE_MT_SYNCH,
                                    Common_TimePolicy_t,
                                    SessionMessageType,
                                    ProtocolMessageType,
                                    ConfigurationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);

  // info
  bool isInitialized () const;

//  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
//                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  virtual void report () const;

  // implement IMemAllocatorNotifyCallbackTemp
  virtual STDMETHODIMP NotifyRelease (void);

  // implement ISampleGrabberCB
  virtual STDMETHODIMP BufferCB (double, // SampleTime
                                 BYTE*,  // Buffer
                                 long);  // BufferLen
  virtual STDMETHODIMP SampleCB (double,         // SampleTime
                                 IMediaSample*); // Sample
  virtual HRESULT STDMETHODCALLTYPE QueryInterface (const IID&, void**);
  virtual ULONG STDMETHODCALLTYPE AddRef ();
  virtual ULONG STDMETHODCALLTYPE Release ();

//  virtual void upStream (Stream_Base_t*);
//  virtual Stream_Base_t* upStream () const;

 private:
  typedef Stream_HeadModuleTaskBase_T<LockType,
                                      ///
                                      ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StreamStateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T (const Stream_Dev_Cam_Source_DirectShow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Cam_Source_DirectShow_T& operator= (const Stream_Dev_Cam_Source_DirectShow_T&))

  virtual int svc (void);

  // helper methods
  bool initialize_DirectShow (const std::string&,      // (source) device name (FriendlyName)
                              const HWND,              // (target) window handle [NULL: NullRenderer]
                              ICaptureGraphBuilder2*&, // return value: (capture) graph builder handle
                              ISampleGrabber*&);       // return value: sample grabber handle
  bool putStatisticMessage (const StatisticContainerType&) const; // statistics info

  bool                              isInitialized_;

  // timer
  Stream_StatisticHandler_Reactor_t statisticCollectionHandler_;
  long                              timerID_;

  // DirectShow
  bool                              isFirst_;
  ICaptureGraphBuilder2*            ICaptureGraphBuilder2_;
  IMediaControl*                    IMediaControl_;
  IMediaEventEx*                    IMediaEventEx_;
  //ISampleGrabber*                   ISampleGrabber_;
  DWORD                             ROTID_;
};

#include "stream_dev_cam_source_directshow.inl"

#endif
