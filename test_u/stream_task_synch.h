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

#ifndef STREAM_TASK_SYNCH_H
#define STREAM_TASK_SYNCH_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common.h"

#include "stream_task.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;

class Stream_TaskSynch
 : public Stream_Task_T<ACE_NULL_SYNCH,
                        Common_TimePolicy_t>
{
 public:
  virtual ~Stream_TaskSynch ();

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  virtual int open (void* = NULL);
  virtual int close (u_long = 0);
  virtual int module_closed (void);

  // this is a NOP (not an active object)
  virtual void waitForIdleState () const;

 protected:
  Stream_TaskSynch ();

 private:
  typedef Stream_Task_T<ACE_NULL_SYNCH,
                        Common_TimePolicy_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskSynch (const Stream_TaskSynch&));
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskSynch& operator= (const Stream_TaskSynch&));
};

#endif
