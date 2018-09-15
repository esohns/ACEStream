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

#include "ace/Synch.h"
#include "test_i_target_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "test_i_common.h"
#include "test_i_callbacks.h"
#include "test_i_defines.h"

Test_I_Target_EventHandler::Test_I_Target_EventHandler (struct Test_I_Target_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler::Test_I_Target_EventHandler"));

}

void
Test_I_Target_EventHandler::start (Stream_SessionId_t sessionId_in,
                                   const struct Test_I_Target_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  sessionData_ =
    &const_cast<struct Test_I_Target_SessionData&> (sessionData_in);

#if defined (GTK_SUPPORT)
  guint event_source_id = 0;
#endif // GTK_SUPPORT
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState.lock);
#if defined (GTK_SUPPORT)
    event_source_id = g_idle_add (idle_start_target_UI_cb,
                                  CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_start_target_UI_cb): \"%m\", returning\n")));
      return;
    } // end IF
    CBData_->UIState.eventSourceIds.insert (event_source_id);
#endif // GTK_SUPPORT
    CBData_->UIState.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
}

void
Test_I_Target_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                    const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_Target_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_SUPPORT)
  guint event_source_id = 0;
#endif // GTK_SUPPORT
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState.lock);
#if defined (GTK_SUPPORT)
    event_source_id = g_idle_add (idle_end_target_UI_cb,
                                  CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_end_target_UI_cb): \"%m\", returning\n")));
      return;
    } // end IF
    CBData_->UIState.eventSourceIds.insert (event_source_id);
#endif // GTK_SUPPORT
    CBData_->UIState.eventStack.push (COMMON_UI_EVENT_STOPPED);
  } // end lock scope

  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_I_Target_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                    const Test_I_Target_Message_t& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState.lock);
    CBData_->UIState.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
}
void
Test_I_Target_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                    const Test_I_Target_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      return;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      // sanity check(s)
      if (!sessionData_)
        goto continue_;

      if (sessionData_->lock)
      {
        result = sessionData_->lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState.lock);
        CBData_->progressData.statistic = sessionData_->statistic;
      } // end lock scope

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

continue_:
      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  sessionMessage_in.type ()));
      return;
    }
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->UIState.lock);
    CBData_->UIState.eventStack.push (event_e);
  } // end lock scope
}
