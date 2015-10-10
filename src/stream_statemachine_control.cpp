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
#include "ace/Reverse_Lock_T.h"

#include "stream_macros.h"

Stream_StateMachine_Control::Stream_StateMachine_Control ()
 : inherited (STREAM_STATE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::Stream_StateMachine_Control"));

}

Stream_StateMachine_Control::~Stream_StateMachine_Control ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::~Stream_StateMachine_Control"));

}

void
Stream_StateMachine_Control::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::initialize"));

  if (!change (STREAM_STATE_INITIALIZED))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_StateMachine_Control::change(STREAM_STATE_INITIALIZED), continuing\n")));
}

void
Stream_StateMachine_Control::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::reset"));

  Stream_StateMachine_Control::initialize ();
}

bool
Stream_StateMachine_Control::wait (const ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::wait"));

  int result = -1;

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (inherited::stateLock_);

    while (inherited::state_ != STREAM_STATE_FINISHED)
    {
      result = condition_.wait (timeout_in);
      if (result == -1)
      {
        int error = ACE_OS::last_error ();
        if (error != ETIME) // 137: timed out
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Condition::wait(%#T): \"%m\", continuing\n"),
                      timeout_in));
      } // end IF
    } // end WHILE
  } // end lock scope

  return true;
}

bool
Stream_StateMachine_Control::change (Stream_StateMachine_ControlState newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::change"));

  int result = -1;
  ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> reverse_lock (inherited::stateLock_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (inherited::stateLock_);

  switch (inherited::state_)
  {
    case STREAM_STATE_INVALID:
    {
      switch (newState_in)
      {
        // good case
        case STREAM_STATE_INITIALIZED:
        {
          //           ACE_DEBUG ((LM_DEBUG,
          //                       ACE_TEXT ("state switch: INVALID --> INITIALIZED\n")));

          {
            ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end lock scope

          return true;
        }
        // error case
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_INITIALIZED:
    {
      switch (newState_in)
      {
        // good case
        case STREAM_STATE_RUNNING:
        {
          //           ACE_DEBUG ((LM_DEBUG,
          //                       ACE_TEXT ("state switch: INITIALIZED --> RUNNING\n")));

          // *WARNING*: falls through
        }
        case STREAM_STATE_FINISHED: // !active
        case STREAM_STATE_INITIALIZED:
        {
          {
            ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end lock scope

          if (newState_in == STREAM_STATE_FINISHED)
          {
            result = condition_.broadcast ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
          } // end IF

          return true;
        }
        // error case
        case STREAM_STATE_PAUSED:
        case STREAM_STATE_STOPPED:
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

          {
            ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end lock scope

          // signal waiting thread(s)
          if (newState_in == STREAM_STATE_FINISHED)
          {
            result = condition_.broadcast ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
          } // end IF

          // *IMPORTANT NOTE*: make sure the transition RUNNING
          //                   [--> STOPPED] --> FINISHED
          //                   works for the nonactive (!) case as well...
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
        case STREAM_STATE_STOPPED: // asynchronous (normal) mode
        case STREAM_STATE_FINISHED: // synchronous (blocking) mode
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

          {
            ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> > aGuard_2 (reverse_lock);
            inherited::change (new_state);
          } // end lock scope

          return true;
        }
        // error case
        case STREAM_STATE_INITIALIZED:
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
        case STREAM_STATE_INITIALIZED:
        case STREAM_STATE_RUNNING:
        case STREAM_STATE_FINISHED:
        {
//           ACE_DEBUG ((LM_DEBUG,
//                       ACE_TEXT ("state switch: STOPPED --> FINISHED\n")));

          {
            ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end lock scope

          // signal waiting thread(s)
          if (newState_in == STREAM_STATE_FINISHED)
          {
            result = condition_.broadcast ();
            if (result == -1)
              ACE_DEBUG ((LM_ERROR,
                          ACE_TEXT ("failed to ACE_SYNCH_CONDITION::broadcast(): \"%m\", continuing\n")));
          } // end IF

          return true;
        }
        // error cases
        case STREAM_STATE_PAUSED:
        default:
          break;
      } // end SWITCH

      break;
    }
    case STREAM_STATE_FINISHED:
    {
      switch (newState_in)
      {
        // good case
        case STREAM_STATE_RUNNING:
        {
          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("state switch: FINISHED --> RUNNING\n")));

          // *WARNING*: falls through
        }
        case STREAM_STATE_INITIALIZED:
        {
          {
            ACE_Guard<ACE_Reverse_Lock<ACE_SYNCH_RECURSIVE_MUTEX> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end lock scope

          return true;
        }
        // error case
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
              ACE_TEXT ("invalid state transition: \"%s\" --> \"%s\": check implementation !, aborting\n"),
              ACE_TEXT (state2String (state_).c_str ()),
              ACE_TEXT (state2String (newState_in).c_str ())));

  return false;
}

void
Stream_StateMachine_Control::finished ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::finished"));

  change (STREAM_STATE_FINISHED);
}

std::string
Stream_StateMachine_Control::state2String (Stream_StateMachine_ControlState state_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control::state2String"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID");

  switch (state_in)
  {
    case STREAM_STATE_INVALID:
      break;
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
