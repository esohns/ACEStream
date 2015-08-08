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
#include "ace/Log_Msg.h"

#include "stream_macros.h"

Stream_StateMachine_Control::Stream_StateMachine_Control ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::Stream_StateMachine_Control"));

  inherited::state_ = STREAM_STATE_INITIALIZED;
}

Stream_StateMachine_Control::~Stream_StateMachine_Control ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::~Stream_StateMachine_Control"));

}

bool
Stream_StateMachine_Control::change (Stream_StateMachine_ControlState newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::change"));

  // synchronize access to state machine...
  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (inherited::lock_);

  switch (inherited::state_)
  {
    case STREAM_STATE_INITIALIZED:
    {
      switch (newState_in)
      {
        // good case
        case STREAM_STATE_RUNNING:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: INITIALIZED --> RUNNING\n")));

          inherited::change (newState_in);

          return true;
        }
        // error case
        case STREAM_STATE_INITIALIZED:
        case STREAM_STATE_PAUSED:
        case STREAM_STATE_STOPPED:
        case STREAM_STATE_FINISHED:
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_RUNNING:
    {
      switch (newState_in)
      {
        // good case
        case STREAM_STATE_PAUSED:
        case STREAM_STATE_STOPPED:
        case STREAM_STATE_FINISHED:
        {
          //std::string newStateString;
          //ControlState2String (newState_in,
          //                     newStateString);
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: RUNNING --> %s\n"),
//                       ACE_TEXT (newStateString.c_str())));

          inherited::change (newState_in);

          // *IMPORTANT NOTE*: make sure the transition RUNNING
          //                   [--> STOPPED] --> FINISHED
          //                   works for the inactive (!) case as well...
          if (inherited::state_ != STREAM_STATE_FINISHED)
            inherited::state_ = newState_in;

          return true;
        }
        // error case
        case STREAM_STATE_INITIALIZED:
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_PAUSED:
    {
      switch (newState_in)
      {
        // good case
        case STREAM_STATE_PAUSED: // just like a tape-recorder...
        case STREAM_STATE_RUNNING: // ...but also allow this to resume
        case STREAM_STATE_STOPPED:
        {
          // handle a special case: PAUSED --> PAUSED is logically mapped to
          // PAUSED --> RUNNING, just like a tape recorder...
          // *IMPORTANT NOTE*: make sure children are aware of this behaviour
          Stream_StateMachine_ControlState new_state =
            ((newState_in == STREAM_STATE_PAUSED) ? STREAM_STATE_RUNNING
                                                  : newState_in);

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("state switch: PAUSED --> %s\n"),
          //            ACE_TEXT (state2String (new_state).c_str ())));

          inherited::change (new_state);

          return true;
        }
        // error case
        case STREAM_STATE_INITIALIZED:
        case STREAM_STATE_FINISHED:
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_STOPPED:
    {
      switch (newState_in)
      {
        // good cases
        // *NOTE*: have to allow this...
        // (scenario: asynchronous user abort via stop())
        case STREAM_STATE_FINISHED:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: STOPPED --> FINISHED\n")));

          inherited::change (newState_in);

          return true;
        }
        // error cases
        case STREAM_STATE_INITIALIZED:
        case STREAM_STATE_PAUSED:
        case STREAM_STATE_RUNNING:
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      switch (newState_in)
      {
        // *NOTE*: the whole stream needs to re-initialize BEFORE this happens
        // good case
        case STREAM_STATE_RUNNING:
        {
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("state switch: FINISHED --> RUNNING\n")));

          inherited::change (newState_in);

          return true;
        }
        // error case
        case STREAM_STATE_INITIALIZED:
        case STREAM_STATE_PAUSED:
        case STREAM_STATE_STOPPED:
        default:
          break;
      } // end SWITCH

      break;
    }
    default:
      break;
  } // end SWITCH
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("invalid state switch: \"%s\" --> \"%s\": check implementation !, aborting\n"),
              ACE_TEXT (state2String (state_).c_str ()),
              ACE_TEXT (state2String (newState_in).c_str ())));

  return false;
}

std::string
Stream_StateMachine_Control::state2String (Stream_StateMachine_ControlState state_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::state2String"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID");

  switch (state_in)
  {
    case STREAM_STATE_INITIALIZED:
    {
      result = ACE_TEXT_ALWAYS_CHAR ("INITIALIZED");
      break;
    }
    case STREAM_STATE_RUNNING:
    {
      result = ACE_TEXT_ALWAYS_CHAR ("RUNNING");
      break;
    }
    case STREAM_STATE_PAUSED:
    {
      result = ACE_TEXT_ALWAYS_CHAR ("PAUSED");
      break;
    }
    case STREAM_STATE_STOPPED:
    {
      result = ACE_TEXT_ALWAYS_CHAR ("STOPPED");
      break;
    }
    case STREAM_STATE_FINISHED:
    {
      result = ACE_TEXT_ALWAYS_CHAR ("FINISHED");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("unknown/invalid state (was: %d), aborting\n"),
                  state_in));
      break;
    }
  } // end SWITCH

  return result;
}
