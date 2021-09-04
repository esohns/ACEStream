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

#ifndef STREAM_HEAD_TASK_H
#define STREAM_HEAD_TASK_H

#include <vector>

#include "ace/Global_Macros.h"
#include "ace/Message_Queue_T.h"
#include "ace/Module.h"
#include "ace/Stream_Modules.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;
class Stream_IMessageQueue;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
class Stream_HeadTask_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType>
{
  typedef ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType> inherited;

 public:
  Stream_HeadTask_T (Stream_IMessageQueue* = NULL); // message queue handle
  virtual ~Stream_HeadTask_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadTask_T (const Stream_HeadTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadTask_T& operator= (const Stream_HeadTask_T&))

  // convenient types
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> MESSAGE_QUEUE_T;

  bool                                 isLinked_;
  typename SessionMessageType::DATA_T* sessionData_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
class Stream_TailTask_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public ACE_Stream_Tail<ACE_SYNCH_USE,
                          TimePolicyType>
{
  typedef ACE_Stream_Tail<ACE_SYNCH_USE,
                          TimePolicyType> inherited;

 public:
  Stream_TailTask_T (Stream_IAllocator* = NULL);
  virtual ~Stream_TailTask_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

 protected:
  // convenient types
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_TailTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_TailTask_T (const Stream_TailTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TailTask_T& operator= (const Stream_TailTask_T&))

  // helper methods
  bool putControlMessage (typename ControlMessageType::CONTROL_T,              // control type
                          const typename SessionMessageType::DATA_T::DATA_T&); // session data

  Stream_IAllocator*                   allocator_;
  typename SessionMessageType::DATA_T* sessionData_;
};

// include template definition
#include "stream_head_task.inl"

#endif
