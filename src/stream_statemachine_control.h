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
#include "ace/Recursive_Thread_Mutex.h"
//#include "ace/Synch.h"

#include "stream_exports.h"

class Stream_Export Stream_StateMachine_Control
{
 public:
  enum StateMachine_State
  {
    STATE_INVALID = -1,
    STATE_INITIALIZED,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_STOPPED,
    STATE_FINISHED,
    /////////////////////////////////////
    STATE_MAX
  };

  Stream_StateMachine_Control ();
  virtual ~Stream_StateMachine_Control ();

  // info
  static void ControlState2String (StateMachine_State, // state
                                   std::string&);      // return value: state string

 protected:
  // only children can retrieve state
  StateMachine_State getState () const;

  // only children can change state
  // *WARNING*: PAUSED --> PAUSED is silently remapped to PAUSED --> RUNNING
  // in order to resemble a traditional tape recorder...
  // --> children must implement the corresponding behavior !
  bool changeState (StateMachine_State); // new state

  // callback invoked on change of state
  // *TODO*: make this an interface !
  virtual void onStateChange (StateMachine_State) = 0; // new state

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control (const Stream_StateMachine_Control&));
  ACE_UNIMPLEMENTED_FUNC (Stream_StateMachine_Control& operator= (const Stream_StateMachine_Control&));

  // helper method
  // *IMPORTANT NOTE*: this method needs to be called with the lock held !
  void invokeCallback (StateMachine_State); // new state

  // current state
  StateMachine_State               state_;
//   *IMPORTANT NOTE*: this MUST be recursive, so children can retrieve current
//                     state from within onStateChange without deadlocking
  //ACE_Condition<ACE_Recursive_Thread_Mutex> myCondition;
  mutable ACE_Recursive_Thread_Mutex lock_;
};

// convenience types
typedef Stream_StateMachine_Control::StateMachine_State Stream_StateType_t;

#endif
