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

#ifndef STREAM_DEV_MIC_SOURCE_WASAPI_H
#define STREAM_DEV_MIC_SOURCE_WASAPI_H

#include "Audioclient.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_defines.h"

extern const char libacestream_default_dev_mic_source_wasapi_module_name_string[];

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
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename MediaType>
  class Stream_Dev_Mic_Source_WASAPI_T
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
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
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
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Dev_Mic_Source_WASAPI_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Mic_Source_WASAPI_T ();

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

  // implement (part of) Stream_ITaskBase
//  virtual void handleDataMessage (ProtocolMessageType*&, // data message handle
//                                  bool&);                // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef Stream_Dev_Mic_Source_WASAPI_T<ACE_SYNCH_USE,
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
                                         MediaType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_WASAPI_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_WASAPI_T (const Stream_Dev_Mic_Source_WASAPI_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_WASAPI_T& operator= (const Stream_Dev_Mic_Source_WASAPI_T&))

  virtual int svc (void);

  IAudioClient*        audioClient_;
  IAudioCaptureClient* audioCaptureClient_;
  HANDLE               event_;
  size_t               frameSize_; // byte(s)
  HANDLE               task_;
};

// include template definition
#include "stream_dev_mic_source_wasapi.inl"

#endif
