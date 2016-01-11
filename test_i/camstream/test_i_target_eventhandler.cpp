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

#include "test_i_target_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "gtk/gtk.h"

#include "test_i_common.h"
#include "test_i_callbacks.h"
#include "test_i_defines.h"

Test_I_Stream_Target_EventHandler::Test_I_Stream_Target_EventHandler (Test_I_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Target_EventHandler::Test_I_Stream_Target_EventHandler"));

}

Test_I_Stream_Target_EventHandler::~Test_I_Stream_Target_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Target_EventHandler::~Test_I_Stream_Target_EventHandler"));

}

void
Test_I_Stream_Target_EventHandler::start (const Test_I_Target_Stream_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Target_EventHandler::start"));

  sessionData_ = &sessionData_in;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.transferred = 0;
  CBData_->eventStack.push_back (STREAM_GTKEVENT_START);

  guint event_source_id = g_idle_add (idle_start_target_UI_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_target_UI_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);
}

void
Test_I_Stream_Target_EventHandler::notify (const Test_I_Target_Stream_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Target_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (sessionData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.transferred += message_in.total_length ();

  CBData_->eventStack.push_back (STREAM_GTKEVENT_DATA);
}
void
Test_I_Stream_Target_EventHandler::notify (const Test_I_Target_Stream_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Target_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  Stream_GTK_Event event = STREAM_GKTEVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_STATISTIC:
    {
      event = STREAM_GTKEVENT_STATISTIC;

      const Test_I_Target_Stream_SessionData_t& session_data_container_r =
        sessionMessage_in.get ();
      const Test_I_Target_Stream_SessionData& session_data_r =
        session_data_container_r.get ();
      CBData_->progressData.statistic = session_data_r.currentStatistic;

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
  CBData_->eventStack.push_back (event);
}

void
Test_I_Stream_Target_EventHandler::end ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Target_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->eventStack.push_back (STREAM_GTKEVENT_END);

  guint event_source_id = g_idle_add (idle_end_target_UI_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_target_UI_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);
}
