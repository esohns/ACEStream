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

#ifndef STREAM_STATEMACHINE_CONTROL_H
#define STREAM_STATEMACHINE_CONTROL_H

#include <string>

#include <ace/Global_Macros.h>

#include "common_statemachine_base.h"

#include "stream_statemachine_common.h"

// forward declarations
class ACE_Time_Value;

template <ACE_SYNCH_DECL>
class Stream_StateMachine_Control_T
 : public Common_StateMachine_Base_T<ACE_SYNCH_USE,
                                     Stream_StateMachine_ControlState>
 , public Stream_StateMachine_IControl_T<Stream_StateMachine_ControlState>
{
 public:
  Stream_StateMachine_Control_T (ACE_SYNCH_MUTEX_T*); // lock handle
  virtual ~Stream_StateMachine_Control_T ();

  // implement (part of) Common_IStateMachine_T
  virtual void initialize ();
  virtual void reset ();
  // *NOTE*: users need to provide absolute values (i.e. deadline)
  // *IMPORTANT NOTE*: STREAM_STATE_FINISHED: processing has completed in the
  //                   sense that all data has been enqueued onto the stream
  //                   (e.g. a file has been read). Data processing may still be
  //                   ongoing at this stage
  virtual bool wait (Stream_StateMachine_ControlState,
                     const ACE_Time_Value* = NULL); // timeout (absolute) ? : block
  virtual std::string state2String (Stream_StateMachine_ControlState) const;

  // implement Stream_StateMachine_IControl_T
  virtual void finished ();

 protected:
  // convenient types
  typedef Common_StateMachine_Base_T<ACE_SYNCH_USE,
                                     Stream_StateMachine_ControlState> COMMON_STATEMACHINE_T;
  using COMMON_STATEMACHINE_T::initialize;

  // override (part of) Common_IStateMachine_T
  // *NOTE*: PAUSED --> PAUSED is silently remapped to PAUSED --> RUNNING
  //         in the model of a (traditional) tape recorder
  //         --> derived classes must implement the corresponding behavior
  virtual bool change (Stream_StateMachine_ControlState); // new state

 private:
  typedef Common_StateMachine_Base_T<ACE_SYNCH_USE,
                                     Stream_StateMachine_ControlState> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control_T (const Stream_StateMachine_Control_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control_T& operator= (const Stream_StateMachine_Control_T&))
};

// include template definition
#include "stream_statemachine_control.inl"

#endif
