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

#ifndef STREAM_TASK_BASE_SYNCH_H
#define STREAM_TASK_BASE_SYNCH_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "stream_task_base.h"

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
class Stream_TaskBaseSynch_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionIdType,
                            SessionEventType>
{
 public:
  virtual ~Stream_TaskBaseSynch_T ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);

  // *NOTE*: a NOP in this instance
  virtual void waitForIdleState () const;

 protected:
  Stream_TaskBaseSynch_T ();

 private:
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionIdType,
                            SessionEventType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseSynch_T (const Stream_TaskBaseSynch_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseSynch_T& operator= (const Stream_TaskBaseSynch_T&))
};

// include template definition
#include "stream_task_base_synch.inl"

#endif
