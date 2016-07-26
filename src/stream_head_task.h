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

#include "ace/Global_Macros.h"
#include "ace/Stream_Modules.h"
#include "ace/Synch_Traits.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionIdType,
          typename SessionEventType>
class Stream_HeadTask_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType>
{
 public:
  Stream_HeadTask_T ();
  virtual ~Stream_HeadTask_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

 private:
  typedef ACE_Stream_Head<ACE_SYNCH_USE,
                          TimePolicyType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_HeadTask_T (const Stream_HeadTask_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_HeadTask_T& operator= (const Stream_HeadTask_T&))

  bool                                 isLinked_;
  typename SessionMessageType::DATA_T* sessionData_;
};

// include template definition
#include "stream_head_task.inl"

#endif
