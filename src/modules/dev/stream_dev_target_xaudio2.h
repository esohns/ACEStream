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

#ifndef STREAM_DEV_TARGET_XAUDIO2_H
#define STREAM_DEV_TARGET_XAUDIO2_H

#include "xaudio2.h"

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_defines.h"

extern const char libacestream_default_dev_target_xaudio2_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename MediaType>
class Stream_Dev_Target_XAudio2_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE, 
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public IXAudio2VoiceCallback
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
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;

 public:
  Stream_Dev_Target_XAudio2_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Target_XAudio2_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleControlMessage (ControlMessageType&); // control message handle
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // IXAudio2VoiceCallback
  inline STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired) {}
  inline STDMETHOD_ (void, OnVoiceProcessingPassEnd) () {}
  inline STDMETHOD_ (void, OnStreamEnd) () { SetEvent (streamEndEvent_); }
  inline STDMETHOD_ (void, OnBufferStart) (void* pBufferContext) {}
  STDMETHOD_ (void, OnBufferEnd) (void* pBufferContext);
  inline STDMETHOD_ (void, OnLoopEnd) (void* pBufferContext) {}
  STDMETHOD_(void, OnVoiceError) (void*, HRESULT);

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_XAudio2_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_XAudio2_T (const Stream_Dev_Target_XAudio2_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_XAudio2_T& operator= (const Stream_Dev_Target_XAudio2_T&))

  HANDLE                  bufferEndEvent_;
  IXAudio2*               engineHandle_;
  IXAudio2MasteringVoice* masterVoice_;
  IXAudio2SourceVoice*    sourceVoice_;
  HANDLE                  streamEndEvent_;
};

// include template definition
#include "stream_dev_target_xaudio2.inl"

#endif
