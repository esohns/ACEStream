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

#ifndef STREAM_TASK_ASYNCH_H
#define STREAM_TASK_ASYNCH_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common.h"

#include "stream_task.h"
#include "stream_messagequeue.h"

// forward declarations
class ACE_Message_Block;
class ACE_Time_Value;

// *NOTE*: the message queue needs to be synched so that shutdown can be
// asynchronous...
class Stream_TaskAsynch
 : public Stream_Task_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t>
{
 public:
  virtual ~Stream_TaskAsynch ();

  // override task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);
  virtual int svc (void);

  virtual void waitForIdleState () const;

 protected:
  Stream_TaskAsynch ();

  ACE_thread_t            threadID_;

 private:
  typedef Stream_Task_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskAsynch (const Stream_TaskAsynch&));
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskAsynch& operator= (const Stream_TaskAsynch&));

  // helper methods
  // enqueue MB_STOP --> stop worker thread
  // *WARNING*: handle with EXTREME care, you should NEVER use this directly
  // if in stream context (i.e. task/module is part of a stream)
  void shutdown ();

  Stream_MessageQueue queue_;
};

#endif
