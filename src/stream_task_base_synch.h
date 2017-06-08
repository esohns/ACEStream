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

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "stream_task_base.h"

// forward declaration(s)
class ACE_Message_Block;
class ACE_Time_Value;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionIdType,
          typename SessionControlType,
          typename SessionEventType,
          ////////////////////////////////
          typename UserDataType>
class Stream_TaskBaseSynch_T
// *TODO*: figure out how to use ACE_NULL_SYNCH in this case
 : public Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionIdType,
                            SessionControlType,
                            SessionEventType,
                            UserDataType>
{
 public:
  inline virtual ~Stream_TaskBaseSynch_T () {};

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value
  inline virtual int open (void* = NULL) { return 0; };
  inline virtual int close (u_long = 0) { return 0; };
  // *NOTE*: invoked by an external thread either from:
  //         - the ACE_Module dtor or
  //         - during explicit ACE_Module::close()
  inline virtual int module_closed (void) { return 0; };

  // *NOTE*: a NOP in this instance
  inline virtual void waitForIdleState () const {};

 protected:
  Stream_TaskBaseSynch_T (ISTREAM_T* = NULL); // stream handle

 private:
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionIdType,
                            SessionControlType,
                            SessionEventType,
                            UserDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseSynch_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseSynch_T (const Stream_TaskBaseSynch_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBaseSynch_T& operator= (const Stream_TaskBaseSynch_T&))
};

// include template definition
#include "stream_task_base_synch.inl"

#endif
