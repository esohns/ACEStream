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

#ifndef STREAM_TASK_BASE_H
#define STREAM_TASK_BASE_H

#include "ace/Global_Macros.h"

#include "common_task_base.h"

#include "stream_itask.h"

template <typename TaskSynchStrategyType,
          typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType>
class Stream_TaskBase_T
 : public Common_TaskBase_T<TaskSynchStrategyType,
                            TimePolicyType>
 , public Stream_ITask_T<SessionMessageType,
                         ProtocolMessageType>
{
 public:
  virtual ~Stream_TaskBase_T ();

  // implement (part of) Stream_ITaskBase_T
  // *NOTE*: these are just default (essentially NOP) implementations...
  virtual bool initialize (const void*); // configuration handle
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass this message downstream ?
  virtual void handleProcessingError (const ACE_Message_Block* const); // message handle

  // implement Common_IDumpState
  // *NOTE*: this is just a stub implementation...
  virtual void dump_state () const;

 protected:
  Stream_TaskBase_T ();

  // helper methods
  // standard message handling (to be used by both asynch/synch children !!!)
  void handleMessage (ACE_Message_Block*, // message handle
                      bool&);             // return value: stop processing ?

  // default implementation to handle control messages
  virtual void handleControlMessage (ACE_Message_Block*, // control message
                                     bool&,              // return value: stop processing ?
                                     bool&);             // return value: pass message downstream ?

 private:
  typedef Common_TaskBase_T<TaskSynchStrategyType,
                            TimePolicyType> inherited;
  typedef Stream_ITask_T<SessionMessageType,
                         ProtocolMessageType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T (const Stream_TaskBase_T&));
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T& operator= (const Stream_TaskBase_T&));
};

// include template implementation
#include "stream_task_base.inl"

#endif
