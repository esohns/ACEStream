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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Reverse_Lock_T.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL>
Stream_StateMachine_Control_T<ACE_SYNCH_USE>::Stream_StateMachine_Control_T (ACE_SYNCH_MUTEX_T* lock_in)
 : inherited (lock_in,
              STREAM_STATE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::Stream_StateMachine_Control_T"));

}

template <ACE_SYNCH_DECL>
bool
Stream_StateMachine_Control_T<ACE_SYNCH_USE>::change (Stream_StateMachine_ControlState newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::change"));

  // sanity check(s)
  ACE_ASSERT (inherited::stateLock_);

  bool result = false;
  bool set_result = true;
  ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T> reverse_lock (*inherited::stateLock_);

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX_T, aGuard, *inherited::stateLock_, false);
    switch (inherited::state_)
    {
      case STREAM_STATE_INVALID:
      {
        switch (newState_in)
        {
          // good case
          case STREAM_STATE_INITIALIZED:
          {
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("state switch: INVALID --> INITIALIZED\n")));

            { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard_2, reverse_lock, false);
              result = inherited::change (newState_in);
            } // end lock scope

            goto continue_;
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
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("state switch: INITIALIZED --> RUNNING\n")));

            // *WARNING*: falls through
          }
          case STREAM_STATE_FINISHED: // *TODO*: remove this
          case STREAM_STATE_INITIALIZED:
          case STREAM_STATE_STOPPED: // *TODO*: remove this
          {
            { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard_2, reverse_lock, false);
              result = inherited::change (newState_in);
            } // end lock scope

            goto continue_;
          }
          // error case
          case STREAM_STATE_PAUSED:
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
            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("state switch: RUNNING --> %s\n"),
            //            ACE_TEXT (stateToString (newState_in).c_str ())));

            { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard_2, reverse_lock, false);
              //// *IMPORTANT NOTE*: make sure the transition RUNNING --> FINISHED
              ////                   is actually RUNNING --> STOPPED --> FINISHED
              //if (newState_in == STREAM_STATE_FINISHED)
              //  inherited::change (STREAM_STATE_STOPPED);
              result = inherited::change (newState_in);
            } // end lock scope

            //// *IMPORTANT NOTE*: make sure the transition RUNNING
            ////                   [--> STOPPED] --> FINISHED works for the
            ////                   'passive' case as well
            //if (inherited::state_ != STREAM_STATE_FINISHED)
            //  inherited::state_ = newState_in;

            goto continue_;
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
          case STREAM_STATE_PAUSED:  // behave like a tape recorder...
          case STREAM_STATE_RUNNING: // ...but allow resume
          case STREAM_STATE_STOPPED:
          case STREAM_STATE_FINISHED:
          {
            // map PAUSED --> PAUSED to PAUSED --> RUNNING
            enum Stream_StateMachine_ControlState new_state =
                ((newState_in == STREAM_STATE_PAUSED) ? STREAM_STATE_RUNNING
                                                      : newState_in);

            //ACE_DEBUG ((LM_DEBUG,
            //            ACE_TEXT ("state switch: PAUSED --> %s\n"),
            //            ACE_TEXT (stateToString (new_state).c_str ())));
            { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard_2, reverse_lock, false);
              // *IMPORTANT NOTE*: the transition PAUSED --> [STOPPED/]FINISHED
              //                   is actually PAUSED --> RUNNING [--> STOPPED]
              //                   --> FINISHED
              if ((new_state == STREAM_STATE_STOPPED) ||
                  (new_state == STREAM_STATE_FINISHED))
                result = inherited::change (STREAM_STATE_RUNNING);
              if (new_state == STREAM_STATE_FINISHED)
                result = inherited::change (STREAM_STATE_STOPPED);

              result = inherited::change (new_state);
            } // end lock scope

            goto continue_;
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
            { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard_2, reverse_lock, false);
              result = inherited::change (newState_in);
            } // end lock scope

            //          if (newState_in == STREAM_STATE_FINISHED)
            //            ACE_DEBUG ((LM_DEBUG,
            //                        ACE_TEXT ("state switch: STOPPED --> FINISHED\n")));

            goto continue_;
          }
          case STREAM_STATE_STOPPED: // *NOTE*: allow STOPPED --> STOPPED
            return true; // done
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
            { ACE_GUARD_RETURN (ACE_Reverse_Lock<ACE_SYNCH_MUTEX_T>, aGuard_2, reverse_lock, false);
              result = inherited::change (newState_in);
            } // end lock scope

            // *WARNING*: falls through
            set_result = false;
          }
          case STREAM_STATE_STOPPED:  // *NOTE*: allow FINISHED --> (STOPPED)
          case STREAM_STATE_FINISHED: // *NOTE*: allow FINISHED --> (FINISHED)
          {
            if (set_result)
              result = true;

            goto continue_;
          }
          // error case
          case STREAM_STATE_PAUSED:
          default:
            break;
        } // end SWITCH

        break;
      }
      default:
        break;
    } // end SWITCH
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid state transition: \"%s\" --> \"%s\", aborting\n"),
                ACE_TEXT (stateToString (inherited::state_).c_str ()),
                ACE_TEXT (stateToString (newState_in).c_str ())));
  } // end lock scope

continue_:
  return result;
}

template <ACE_SYNCH_DECL>
std::string
Stream_StateMachine_Control_T<ACE_SYNCH_USE>::stateToString (Stream_StateMachine_ControlState state_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::stateToString"));

  // initialize return value(s)
  std::string result = ACE_TEXT_ALWAYS_CHAR ("INVALID");

  switch (state_in)
  {
    case STREAM_STATE_INVALID:
      break;
    case STREAM_STATE_INITIALIZED:
      result = ACE_TEXT_ALWAYS_CHAR ("INITIALIZED"); break;
    case STREAM_STATE_RUNNING:
      result = ACE_TEXT_ALWAYS_CHAR ("RUNNING"); break;
    case STREAM_STATE_PAUSED:
      result = ACE_TEXT_ALWAYS_CHAR ("PAUSED"); break;
    case STREAM_STATE_STOPPED:
      result = ACE_TEXT_ALWAYS_CHAR ("STOPPED"); break;
    case STREAM_STATE_FINISHED:
      result = ACE_TEXT_ALWAYS_CHAR ("FINISHED"); break;
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
