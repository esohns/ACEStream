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

#ifndef STREAM_DEV_TARGET_PIPEWIRE_H
#define STREAM_DEV_TARGET_PIPEWIRE_H

#include "spa/param/audio/format-utils.h"

#include "pipewire/pipewire.h"

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_dev_common.h"

void acestream_dev_target_pw_on_stream_param_changed_cb (void* , uint32_t, const struct spa_pod*);
void acestream_dev_target_pw_on_process_cb (void*);

extern const char libacestream_default_dev_target_pipewire_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_Dev_Target_Pipewire_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<struct spa_audio_info>
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
  typedef Stream_MediaFramework_MediaTypeConverter_T<struct spa_audio_info> inherited2;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Dev_Target_Pipewire_T (ISTREAM_T* = NULL); // stream handle
  virtual ~Stream_Dev_Target_Pipewire_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleControlMessage (ControlMessageType&); // control message handle
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  //ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_Pipewire_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_Pipewire_T (const Stream_Dev_Target_Pipewire_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_Pipewire_T& operator= (const Stream_Dev_Target_Pipewire_T&))

  // helper methods
  // override some task-based members
  virtual int svc (void);

  struct Stream_Device_Pipewire_Playback_CBData CBData_;
  struct pw_stream_events                       events_;
  struct pw_main_loop*                          loop_;
  uint8_t                                       PODBuffer_[STREAM_DEV_PIPEWIRE_DEFAULT_POD_BUFFER_SIZE];
  typename inherited::MESSAGE_QUEUE_T           queue_;
};

// include template definition
#include "stream_dev_target_pipewire.inl"

#endif
