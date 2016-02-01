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

template <typename LockType>
Stream_StateMachine_Control_T<LockType>::Stream_StateMachine_Control_T (LockType* lock_in)
 : inherited (lock_in,
              STREAM_STATE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::Stream_StateMachine_Control_T"));

}

template <typename LockType>
Stream_StateMachine_Control_T<LockType>::~Stream_StateMachine_Control_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::~Stream_StateMachine_Control_T"));

}

template <typename LockType>
void
Stream_StateMachine_Control_T<LockType>::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::initialize"));

  if (!change (STREAM_STATE_INITIALIZED))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_StateMachine_Control_T::change(STREAM_STATE_INITIALIZED), continuing\n")));
}

template <typename LockType>
void
Stream_StateMachine_Control_T<LockType>::reset ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::reset"));

  initialize ();
}

template <typename LockType>
bool
Stream_StateMachine_Control_T<LockType>::wait (Stream_StateMachine_ControlState state_in,
                                               const ACE_Time_Value* timeout_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::wait"));

  bool result = false;

  // sanity check(s)
  ACE_ASSERT (inherited::condition_);

  int result_2 = -1;

  if (inherited::stateLock_)
  {
    result_2 = inherited::stateLock_->acquire ();
    if (result_2 == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to LockType::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  while (inherited::state_ != state_in)
  {
    result_2 = inherited::condition_->wait (timeout_in);
    if (result_2 == -1)
    {
      int error = ACE_OS::last_error ();
      if (error != ETIME) // 137: timed out
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Condition::wait(%#T): \"%m\", aborting\n"),
                    timeout_in));

      goto unlock; // timed out ?
    } // end IF
  } // end WHILE
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("reached state \"%s\"...\n"),
  //            ACE_TEXT (state2String (state_in).c_str ())));
  result = true;

unlock:
  if (inherited::stateLock_)
  {
    result_2 = inherited::stateLock_->release ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to LockType::release(): \"%m\", continuing\n")));
  } // end IF

  return result;
}

template <typename LockType>
bool
Stream_StateMachine_Control_T<LockType>::change (Stream_StateMachine_ControlState newState_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::change"));

  bool result = false;

  int result_2 = -1;
  ACE_Reverse_Lock<LockType> reverse_lock (*inherited::stateLock_);

  if (inherited::stateLock_)
  {
    result_2 = inherited::stateLock_->acquire ();
    if (result_2 == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to LockType::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

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

          if (!inherited::stateLock_)
            inherited::change (newState_in);
          else
          {
            ACE_Guard<ACE_Reverse_Lock<LockType> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end ELSE

          result = true;
          goto unlock;
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
          if (!inherited::stateLock_)
            inherited::change (newState_in);
          else
          {
            ACE_Guard<ACE_Reverse_Lock<LockType> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end ELSE

          result = true;
          goto unlock;
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
          //            ACE_TEXT (state2String (newState_in).c_str ())));

          if (!inherited::stateLock_)
            inherited::change (newState_in);
          else
          {
            ACE_Guard<ACE_Reverse_Lock<LockType> > aGuard_2 (reverse_lock);

            //// *IMPORTANT NOTE*: make sure the transition RUNNING --> FINISHED
            ////                   is actually RUNNING --> STOPPED --> FINISHED
            //if (newState_in == STREAM_STATE_FINISHED)
            //  inherited::change (STREAM_STATE_STOPPED);

            inherited::change (newState_in);
          } // end ELSE

          //// *IMPORTANT NOTE*: make sure the transition RUNNING
          ////                   [--> STOPPED] --> FINISHED works for the
          ////                   'passive' case as well
          //if (inherited::state_ != STREAM_STATE_FINISHED)
          //  inherited::state_ = newState_in;

          result = true;
          goto unlock;
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
          Stream_StateMachine_ControlState new_state =
            ((newState_in == STREAM_STATE_PAUSED) ? STREAM_STATE_RUNNING
                                                  : newState_in);

          //ACE_DEBUG ((LM_DEBUG,
          //            ACE_TEXT ("state switch: PAUSED --> %s\n"),
          //            ACE_TEXT (state2String (new_state).c_str ())));
          if (!inherited::stateLock_)
            inherited::change (newState_in);
          else
          {
            ACE_Guard<ACE_Reverse_Lock<LockType> > aGuard_2 (reverse_lock);

            // *IMPORTANT NOTE*: the transition PAUSED --> [STOPPED/]FINISHED
            //                   is actually PAUSED --> RUNNING [--> STOPPED]
            //                   --> FINISHED
            if ((new_state == STREAM_STATE_STOPPED) ||
                (new_state == STREAM_STATE_FINISHED))
              inherited::change (STREAM_STATE_RUNNING);
            if (new_state == STREAM_STATE_FINISHED)
              inherited::change (STREAM_STATE_STOPPED);

            inherited::change (new_state);
          } // end ELSE

          result = true;
          goto unlock;
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
          if (!inherited::stateLock_)
            inherited::change (newState_in);
          else
          {
            ACE_Guard<ACE_Reverse_Lock<LockType> > aGuard_2 (reverse_lock);
            inherited::change (newState_in);
          } // end ELSE

//          if (newState_in == STREAM_STATE_FINISHED)
//            ACE_DEBUG ((LM_DEBUG,
//                        ACE_TEXT ("state switch: STOPPED --> FINISHED\n")));

          result = true;
          goto unlock;
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
          if (!inherited::stateLock_)
            inherited::change (newState_in);
          else
          {
            ACE_Reverse_Lock<LockType> reverse_lock (*inherited::stateLock_);
            {
              ACE_Guard<ACE_Reverse_Lock<LockType> > aGuard_2 (reverse_lock);
              inherited::change (newState_in);
            } // end lock scope
          } // end ELSE

          // *WARNING*: falls through
        }
        case STREAM_STATE_FINISHED: // *NOTE*: allow FINISHED --> FINISHED
        {
          result = true;
          goto unlock;
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
              ACE_TEXT (state2String (inherited::state_).c_str ()),
              ACE_TEXT (state2String (newState_in).c_str ())));

unlock:
  if (inherited::stateLock_)
  {
    result_2 = inherited::stateLock_->release ();
    if (result_2 == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to LockType::release(): \"%m\", continuing\n")));
  } // end IF

  return result;
}

template <typename LockType>
void
Stream_StateMachine_Control_T<LockType>::finished ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::finished"));

  change (STREAM_STATE_FINISHED);
}

template <typename LockType>
std::string
Stream_StateMachine_Control_T<LockType>::state2String (Stream_StateMachine_ControlState state_in) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_StateMachine_Control_T::state2String"));

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
