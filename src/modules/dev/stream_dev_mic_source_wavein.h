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

#ifndef STREAM_DEV_MIC_SOURCE_WAVEIN_H
#define STREAM_DEV_MIC_SOURCE_WAVEIN_H

#include "mmeapi.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_defines.h"

extern const char libacestream_default_dev_mic_source_wavein_module_name_string[];

extern void CALLBACK
stream_dev_wavein_data_cb (HWAVEIN,    // hwi: device context handle
                           UINT,       // uMsg: message type
                           DWORD_PTR,  // dwInstance: user instance data
                           DWORD_PTR,  // dwParam1: message parameter
                           DWORD_PTR); // dwParam2: message parameter

struct stream_dev_wavein_cbdata
{
  ACE_Message_Block* buffers[STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS];
  size_t             inFlightBuffers;
  Stream_Task_t*     task;
};

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
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename MediaType>
class Stream_Dev_Mic_Source_WaveIn_T
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
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 //, Common_ISet_T<unsigned int>
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
                                      struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Dev_Mic_Source_WaveIn_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Mic_Source_WaveIn_T ();

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

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef Stream_Dev_Mic_Source_WaveIn_T<ACE_SYNCH_USE,
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
                                         MediaType> OWN_TYPE_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_WaveIn_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_WaveIn_T (const Stream_Dev_Mic_Source_WaveIn_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Mic_Source_WaveIn_T& operator= (const Stream_Dev_Mic_Source_WaveIn_T&))

  bool allocateBuffers (Stream_IAllocator*, // allocator handle
                        unsigned int);      // buffer size

  struct wavehdr_tag              bufferHeaders_[STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS];
  struct stream_dev_wavein_cbdata CBData_;
  HWAVEIN                         handle_;
};

// include template definition
#include "stream_dev_mic_source_wavein.inl"

#endif
