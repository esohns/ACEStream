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

#include "ace/Global_Macros.h"

#include "common_statemachine_base.h"

#include "stream_exports.h"

enum Stream_StateMachine_ControlState
{
  STREAM_STATE_INVALID = -1,
  STREAM_STATE_INITIALIZED = 0,
  STREAM_STATE_RUNNING,
  STREAM_STATE_PAUSED,
  STREAM_STATE_STOPPED,
  STREAM_STATE_FINISHED,
  /////////////////////////////////////
  STREAM_STATE_MAX
};

template <typename StateType>
class Stream_StateMachine_IControl_T
 : virtual public Common_IStateMachine_T<Stream_StateMachine_ControlState>
{
 public:
  virtual ~Stream_StateMachine_IControl_T () {}

  virtual void finished () = 0;
};

class Stream_Export Stream_StateMachine_Control
 : public Common_StateMachine_Base_T<Stream_StateMachine_ControlState>
 , public Stream_StateMachine_IControl_T<Stream_StateMachine_ControlState>
{
 public:
  Stream_StateMachine_Control ();
  virtual ~Stream_StateMachine_Control ();

  // implement (part of) Common_IStateMachine_T
  virtual void initialize ();
  virtual void reset ();
  // *NOTE*: users need to provide absolute values (i.e. deadline)
  // *IMPORTANT NOTE*: upon return, processing has completed in the sense that
  //                   all data has been enqueued onto the stream (e.g. a file
  //                   has been read). Data processing may well still be ongoing
  //                   at this stage
  // *TODO*: allow waiting for discrete states
  virtual bool wait (const ACE_Time_Value* = NULL); // timeout ? : block
  virtual std::string state2String (Stream_StateMachine_ControlState) const;

  // implement Stream_StateMachine_IControl_T
  virtual void finished ();

 protected:
  // override (part of) Common_IStateMachine_T
  // *NOTE*: only children can change state
  // *WARNING*: PAUSED --> PAUSED is silently remapped to PAUSED --> RUNNING
  //            in the model of a traditional tape recorder
  //            --> children must implement the corresponding behavior !
  virtual bool change (Stream_StateMachine_ControlState); // new state

 private:
  typedef Common_StateMachine_Base_T<Stream_StateMachine_ControlState> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control (const Stream_StateMachine_Control&))
  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control& operator= (const Stream_StateMachine_Control&))
};

// convenience types
typedef Stream_StateMachine_ControlState Stream_StateType_t;

#endif
