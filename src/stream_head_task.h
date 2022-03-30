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

#include "common_iget.h"

#include "stream_inotify.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;
class Stream_IAllocator;
class Stream_IMessageQueue;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename NotificationType>
class Stream_HeadReaderTask_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType>
 , public Common_ISet_T<bool> // enqueue incoming messages ? : release()
{
  typedef ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType> inherited;

 public:
  // convenient types
  typedef Stream_IEvent_T<NotificationType> IEVENT_T;

  Stream_HeadReaderTask_T (IEVENT_T*,             // event handle
                           Stream_IMessageQueue*, // message queue handle
                           bool);                 // queue incoming messages ? : release()
  inline virtual ~Stream_HeadReaderTask_T () {}

  // implement Common_ISet_T
  virtual void set (const bool); // queue incoming messages ? : release()

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadReaderTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadReaderTask_T (const Stream_HeadReaderTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadReaderTask_T& operator= (const Stream_HeadReaderTask_T&))

  // convenient types
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            TimePolicyType> MESSAGE_QUEUE_T;

  bool      enqueue_;
  IEVENT_T* event_;
};

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
class Stream_HeadWriterTask_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType>
{
  typedef ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType> inherited;

 public:
  // convenient types
  typedef Stream_INotify_T<SessionEventType> NOTIFY_T;

  Stream_HeadWriterTask_T (NOTIFY_T*); // stream handle
  virtual ~Stream_HeadWriterTask_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadWriterTask_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadWriterTask_T (const Stream_HeadWriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadWriterTask_T& operator= (const Stream_HeadWriterTask_T&))

  bool                                 isLinked_;
  NOTIFY_T*                            notify_;
  typename SessionMessageType::DATA_T* sessionData_;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionEventType>
class Stream_TailWriterTask_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public ACE_Stream_Tail<ACE_SYNCH_USE,
                          TimePolicyType>
{
  typedef ACE_Stream_Tail<ACE_SYNCH_USE,
                          TimePolicyType> inherited;

 public:
  Stream_TailWriterTask_T ();
  inline virtual ~Stream_TailWriterTask_T () {}

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_TailWriterTask_T (const Stream_TailWriterTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TailWriterTask_T& operator= (const Stream_TailWriterTask_T&))
};

// include template definition
#include "stream_head_task.inl"

#endif
