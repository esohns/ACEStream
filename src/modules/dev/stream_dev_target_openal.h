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

#ifndef STREAM_DEV_TARGET_OPENAL_H
#define STREAM_DEV_TARGET_OPENAL_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "al.h"
#include "alc.h"
#else
#include "AL/al.h"
#include "AL/alc.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Message_Queue_T.h"
#include "ace/Synch_Traits.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

extern const char libacestream_default_dev_target_openal_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename MediaType>
class Stream_Dev_Target_OpenAL_T
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
  Stream_Dev_Target_OpenAL_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Dev_Target_OpenAL_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef ACE_Message_Queue_Ex<ALuint,
                               ACE_SYNCH_USE,
                               TimePolicyType> QUEUE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_OpenAL_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_OpenAL_T (const Stream_Dev_Target_OpenAL_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Dev_Target_OpenAL_T& operator= (const Stream_Dev_Target_OpenAL_T&))

  // enqueue MB_STOP --> stop worker thread(s)
  virtual void stop (bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)
  // override (part of) ACE_Task_Base
  virtual int svc (void);

  bool queueBuffers ();

  QUEUE_T                             bufferQueue_;
	ALuint                              buffers_[STREAM_DEV_OPENAL_DEFAULT_NUMBER_OF_BUFFERS];
  ALCcontext*                         context_;
  ALCdevice*                          device_;
  ALenum                              format_;
  typename inherited::MESSAGE_QUEUE_T queue_;
  ALsizei                             sampleRate_;
  ALuint                              source_;
};

// include template definition
#include "stream_dev_target_openal.inl"

#endif
