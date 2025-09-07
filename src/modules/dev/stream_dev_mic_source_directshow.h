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

#ifndef STREAM_DEV_MIC_SOURCE_DIRECTSHOW_H
#define STREAM_DEV_MIC_SOURCE_DIRECTSHOW_H

#include <string>

#include "BaseTyps.h"
#include "OAIdl.h"
#include "control.h"
#include "strmif.h"
#include "qedit.h"
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

extern const char libacestream_default_dev_mic_source_directshow_module_name_string[];

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
          typename StatisticContainerType,
          typename SessionManagerType,
          typename TimerManagerType> // implements Common_ITimer
class Stream_Dev_Mic_Source_DirectShow_T
 : public Stream_HeadModuleTaskBase_T<ACE_MT_SYNCH,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      struct Stream_UserData>
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
                                      StatisticContainerType,
                                      SessionManagerType,
                                      TimerManagerType,
                                      struct Stream_UserData> inherited;

 public:
  Stream_Dev_Mic_Source_DirectShow_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Mic_Source_DirectShow_T ();

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
                                    StatisticContainerType,
                                    SessionManagerType,
                                    TimerManagerType,
                                    struct Stream_UserData>::initialize;

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);
  //virtual void start ();
  //virtual void stop (bool = true,   // wait for completion ?
  //                   bool = false); // high priority ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

//  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
//                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement IMemAllocatorNotifyCallbackTemp
  virtual STDMETHODIMP NotifyRelease (void);

  // implement ISampleGrabberCB
  virtual STDMETHODIMP BufferCB (double, // SampleTime
                                 BYTE*,  // Buffer
                                 long);  // BufferLen
  virtual STDMETHODIMP SampleCB (double,         // SampleTime
                                 IMediaSample*); // Sample
  inline virtual STDMETHODIMP QueryInterface (const IID& iid_in, void** interfaceHandle_inout) { ACE_UNUSED_ARG (iid_in); ACE_UNUSED_ARG (interfaceHandle_inout); ACE_ASSERT (false); ACE_NOTSUP_RETURN (E_FAIL); ACE_NOTREACHED (return E_FAIL;) }
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return 1; }
  inline virtual STDMETHODIMP_ (ULONG) Release () { return 0; }

 private:
  // convenient types
  typedef Stream_Dev_Mic_Source_DirectShow_T<ACE_SYNCH_USE,
                                             ControlMessageType,
                                             DataMessageType,
                                             SessionMessageType,
                                             ConfigurationType,
                                             StreamControlType,
                                             StreamNotificationType,
                                             StreamStateType,
                                             StatisticContainerType,
                                             SessionManagerType,
                                             TimerManagerType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_DirectShow_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_DirectShow_T (const Stream_Dev_Mic_Source_DirectShow_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_DirectShow_T& operator= (const Stream_Dev_Mic_Source_DirectShow_T&))

  virtual int svc (void);

  // helper methods
  bool initialize_DirectShow (const struct Stream_Device_Identifier&, // device identifier
                              int,                                    // (target) audio output [-1: none]
                              ICaptureGraphBuilder2*&,                // return value: (capture) graph builder handle
                              IAMDroppedFrames*&,                     // return value: capture filter statistic handle
                              ISampleGrabber*&);                      // return value: sample grabber handle

  bool                   isFirst_;

  //HANDLE                 eventHandle_;
  IAMDroppedFrames*      IAMDroppedFrames_;
  ICaptureGraphBuilder2* ICaptureGraphBuilder2_;
  IGraphBuilder*         IGraphBuilder_;
  IMediaControl*         IMediaControl_;
  IMediaEventEx*         IMediaEventEx_;
  //bool                   manageCOM_;
  DWORD                  ROTID_;
};

// include template definition
#include "stream_dev_mic_source_directshow.inl"

#endif
