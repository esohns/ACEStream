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

#ifndef STREAM_DEV_TARGET_WAVOUT_H
#define STREAM_DEV_TARGET_WAVOUT_H

#include "Mmeapi.h"

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_dev_target_wavout_module_name_string[];

struct Stream_Device_WavOut_Playback_AsynchCBData
{
  bool                    done;
  size_t                  inFlightBuffers;
  //ACE_Message_Queue_Base* queue;
};
static void
CALLBACK stream_dev_target_wavout_async_callback (HWAVEOUT,
                                                  UINT,
                                                  DWORD_PTR,
                                                  DWORD_PTR,
                                                  DWORD_PTR);

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType>
class Stream_Dev_Target_WavOut_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
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

 public:
  Stream_Dev_Target_WavOut_T (ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Target_WavOut_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

//  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> QUEUE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_WavOut_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_WavOut_T (const Stream_Dev_Target_WavOut_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_WavOut_T& operator= (const Stream_Dev_Target_WavOut_T&))

  struct Stream_Device_WavOut_Playback_AsynchCBData CBData_;
  HWAVEOUT                                          handle_;
};

// include template definition
#include "stream_dev_target_wavout.inl"

#endif
