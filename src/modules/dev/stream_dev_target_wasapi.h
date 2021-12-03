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

#ifndef STREAM_DEV_TARGET_WASAPI_H
#define STREAM_DEV_TARGET_WASAPI_H

#include <string>

#include "audiopolicy.h"
#include "Audioclient.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_defines.h"

extern const char libacestream_default_dev_target_wasapi_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionControlType,
          typename SessionEventType,
          typename UserDataType,
          typename MediaType>
class Stream_Dev_Target_WASAPI_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE, 
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 UserDataType>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public IAudioSessionEvents
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionControlType,
                                 SessionEventType,
                                 UserDataType> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Dev_Target_WASAPI_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Target_WASAPI_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement IAudioSessionEvents
  // IUnknown
  virtual STDMETHODIMP QueryInterface (REFIID,
                                       void**);
  inline virtual STDMETHODIMP_ (ULONG) AddRef () { return 1; }
  inline virtual STDMETHODIMP_ (ULONG) Release () { return 1; }
  // IAudioSessionEvents
  virtual STDMETHODIMP OnDisplayNameChanged (LPCWSTR NewDisplayName,
                                             LPCGUID EventContext);
  virtual STDMETHODIMP OnIconPathChanged (LPCWSTR NewIconPath,
                                          LPCGUID EventContext);
  virtual STDMETHODIMP OnSimpleVolumeChanged (float NewVolume,
                                              BOOL NewMute,
                                              LPCGUID EventContext);
  virtual STDMETHODIMP OnChannelVolumeChanged (DWORD ChannelCount,
                                               float NewChannelVolumeArray [],
                                               DWORD ChangedChannel,
                                               LPCGUID EventContext);
  virtual STDMETHODIMP OnGroupingParamChanged (LPCGUID NewGroupingParam,
                                               LPCGUID EventContext);
  virtual STDMETHODIMP OnStateChanged (AudioSessionState NewState);
  virtual STDMETHODIMP OnSessionDisconnected (AudioSessionDisconnectReason DisconnectReason);

 private:
  // convenient types
  typedef Stream_Dev_Target_WASAPI_T<ACE_SYNCH_USE,
                                     TimePolicyType,
                                     ConfigurationType,
                                     ControlMessageType,
                                     DataMessageType,
                                     SessionMessageType,
                                     SessionControlType,
                                     SessionEventType,
                                     UserDataType,
                                     MediaType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_WASAPI_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_WASAPI_T (const Stream_Dev_Target_WASAPI_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_WASAPI_T& operator= (const Stream_Dev_Target_WASAPI_T&))

  // block and get next buffer. If the return value is NULL, stop
  ACE_Message_Block* get ();

  // enqueue MB_STOP --> stop worker thread(s)
  virtual void stop (bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)
  // override (part of) ACE_Task_Base
  virtual int svc (void);

  IAudioClient*                       audioClient_;
  IAudioRenderClient*                 audioRenderClient_;
  UINT32                              bufferSize_; // in #frames
  HANDLE                              event_;
  size_t                              frameSize_; // byte(s)
  typename inherited::MESSAGE_QUEUE_T queue_;
  HANDLE                              task_;
};

// include template definition
#include "stream_dev_target_wasapi.inl"

#endif
