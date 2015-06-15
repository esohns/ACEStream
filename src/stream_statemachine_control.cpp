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
#include "stdafx.h"

#include "stream_statemachine_control.h"

#include "ace/Guard_T.h"
#include "ace/Synch.h"

#include "stream_macros.h"

Stream_StateMachine_Control::Stream_StateMachine_Control ()
 : state_ (Stream_StateMachine_Control::STATE_INITIALIZED)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::Stream_StateMachine_Control"));

}

Stream_StateMachine_Control::~Stream_StateMachine_Control ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::~Stream_StateMachine_Control"));

}

Stream_StateMachine_Control::StateMachine_State
Stream_StateMachine_Control::getState () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::getState"));

  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  return state_;
}

bool
Stream_StateMachine_Control::changeState (StateMachine_State newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::changeState"));

  // synchronize access to state machine...
  ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard (lock_);

  switch (state_)
  {
    case Stream_StateMachine_Control::STATE_INITIALIZED:
    {
      switch (newState_in)
      {
        // good case
        case Stream_StateMachine_Control::STATE_RUNNING:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: INIT --> RUNNING\n")));

          invokeCallback (newState_in);

          state_ = newState_in;

          return true;
        }
        // error case
        case Stream_StateMachine_Control::STATE_INITIALIZED:
        case Stream_StateMachine_Control::STATE_PAUSED:
        case Stream_StateMachine_Control::STATE_STOPPED:
        case Stream_StateMachine_Control::STATE_FINISHED:
        default:
        {
          // what else can we do ?

          break;
        }
      } // end SWITCH

      break;
    }
    case Stream_StateMachine_Control::STATE_RUNNING:
    {
      switch (newState_in)
      {
        // good case
        case Stream_StateMachine_Control::STATE_PAUSED:
        case Stream_StateMachine_Control::STATE_STOPPED:
        case Stream_StateMachine_Control::STATE_FINISHED:
        {
          //std::string newStateString;
          //ControlState2String (newState_in,
          //                     newStateString);
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: RUNNING --> %s\n"),
//                       ACE_TEXT (newStateString.c_str())));

          invokeCallback (newState_in);

          // *IMPORTANT NOTE*: make sure the transition to RUNNING [--> STOPPED] --> FINISHED
          //                   works for the inactive (!) case as well...
          if (state_ != Stream_StateMachine_Control::STATE_FINISHED)
            state_ = newState_in;

          return true;
        }
        // error case
        case Stream_StateMachine_Control::STATE_INITIALIZED:
        default:
        {
          // what else can we do ?

          break;
        }
      } // end SWITCH

      break;
    }
    case Stream_StateMachine_Control::STATE_PAUSED:
    {
      switch (newState_in)
      {
        // good case
        case Stream_StateMachine_Control::STATE_PAUSED: // just like a tape-recorder...
        case Stream_StateMachine_Control::STATE_RUNNING: // ...but we also allow this to resume
        case Stream_StateMachine_Control::STATE_STOPPED:
        {
          // need to handle a special case: PAUSED --> PAUSED is logically mapped to
          // PAUSED --> RUNNING, just like a tape recorder...
          // *IMPORTANT NOTE*: make sure our children are aware of this behaviour !!!
          StateMachine_State newState =
            ((newState_in == Stream_StateMachine_Control::STATE_PAUSED) ? Stream_StateMachine_Control::STATE_RUNNING
                                                                        : newState_in);

          std::string newStateString;
          ControlState2String (newState,
                               newStateString);
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("state switch: PAUSED --> %s\n"),
                      ACE_TEXT (newStateString.c_str ())));

          invokeCallback (newState);

          state_ = newState;

          return true;
        }
        // error case
        case Stream_StateMachine_Control::STATE_INITIALIZED:
        case Stream_StateMachine_Control::STATE_FINISHED:
        default:
        {
          // what else can we do ?

          break;
        }
      } // end SWITCH

      break;
    }
    case Stream_StateMachine_Control::STATE_STOPPED:
    {
      switch (newState_in)
      {
        // good cases
        // *NOTE*: we have to allow this...
        // (scenario: asynchronous user abort via stop())
        case Stream_StateMachine_Control::STATE_FINISHED:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: STOPPED --> FINISHED\n")));

          invokeCallback (newState_in);

          state_ = newState_in;

          return true;
        }
        // error cases
        case Stream_StateMachine_Control::STATE_INITIALIZED:
        case Stream_StateMachine_Control::STATE_PAUSED:
        case Stream_StateMachine_Control::STATE_RUNNING:
        default:
        {
          // what else can we do ?

          break;
        }
      } // end SWITCH

      break;
    }
    case Stream_StateMachine_Control::STATE_FINISHED:
    {
      switch (newState_in)
      {
        // *IMPORTANT NOTE*: the whole stream needs to re-initialize BEFORE this happens...
        // good case
        case Stream_StateMachine_Control::STATE_RUNNING:
        {
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("state switch: FINISHED --> RUNNING\n")));

          invokeCallback (newState_in);

          state_ = newState_in;

          return true;
        }
        // error case
        case Stream_StateMachine_Control::STATE_INITIALIZED:
        case Stream_StateMachine_Control::STATE_PAUSED:
        case Stream_StateMachine_Control::STATE_STOPPED:
        default:
        {
          // what else can we do ?

          break;
        }
      } // end SWITCH

      break;
    }
    default:
    {
      // what else can we do ?

      break;
    }
  } // end SWITCH

  // Note: when we get here, an invalid state change happened... --> check implementation !!!!
  std::string currentStateString;
  std::string newStateString;
  ControlState2String (state_,
                       currentStateString);
  ControlState2String (newState_in,
                       newStateString);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid state switch: \"%s\" --> \"%s\" --> check implementation !, returning\n"),
              ACE_TEXT (currentStateString.c_str ()),
              ACE_TEXT (newStateString.c_str ())));

  return false;
}

void
Stream_StateMachine_Control::invokeCallback (StateMachine_State newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::invokeCallback"));

  // invoke callback...
  try
  {
    onStateChange (newState_in);
  }
  catch (...)
  {
    std::string currentStateString;
    std::string newStateString;
    ControlState2String (state_,
                         currentStateString);
    ControlState2String (newState_in,
                         newStateString);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Stream_StateMachine_Control::onStateChange: \"%s --> %s\", continuing\n"),
                ACE_TEXT (currentStateString.c_str ()),
                ACE_TEXT (newStateString.c_str ())));
  }
}

void
Stream_StateMachine_Control::ControlState2String (StateMachine_State state_in,
                                                  std::string& stateString_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::ControlState2String"));

  // initialize return value(s)
  stateString_out = ACE_TEXT ("UNDEFINED_STATE");

  switch (state_in)
  {
    case Stream_StateMachine_Control::STATE_INITIALIZED:
    {
      stateString_out = ACE_TEXT ("INITIALIZED");
      break;
    }
    case Stream_StateMachine_Control::STATE_RUNNING:
    {
      stateString_out = ACE_TEXT ("RUNNING");
      break;
    }
    case Stream_StateMachine_Control::STATE_PAUSED:
    {
      stateString_out = ACE_TEXT ("PAUSED");
      break;
    }
    case Stream_StateMachine_Control::STATE_STOPPED:
    {
      stateString_out = ACE_TEXT ("STOPPED");
      break;
    }
    case Stream_StateMachine_Control::STATE_FINISHED:
    {
      stateString_out = ACE_TEXT ("FINISHED");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid state: %d, aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH
}
