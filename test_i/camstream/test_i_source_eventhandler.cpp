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

#include "test_i_source_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "test_i_callbacks.h"
#include "test_i_common.h"
#include "test_i_defines.h"

Test_I_Stream_Source_EventHandler::Test_I_Stream_Source_EventHandler (Test_I_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::Test_I_Stream_Source_EventHandler"));

}

Test_I_Stream_Source_EventHandler::~Test_I_Stream_Source_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::~Test_I_Stream_Source_EventHandler"));

}

void
Test_I_Stream_Source_EventHandler::start (const Test_I_Source_Stream_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::start"));

  sessionData_ = &sessionData_in;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.transferred = 0;

  CBData_->eventStack.push_back (TEST_I_GTKEVENT_START);
}

void
Test_I_Stream_Source_EventHandler::notify (const Test_I_Source_Stream_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (sessionData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.transferred += message_in.total_length ();

  CBData_->eventStack.push_back (TEST_I_GTKEVENT_DATA);
}
void
Test_I_Stream_Source_EventHandler::notify (const Test_I_Source_Stream_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::notify"));

  int result = -1;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  Test_I_GTK_Event event = TEST_I_GKTEVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_STATISTIC:
    {
      float current_bytes = 0.0F;

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

        // *NOTE*: the byte counter is more current than what is received here
        //         (see above) --> do not update
      current_bytes = CBData_->progressData.statistic.bytes;
      CBData_->progressData.statistic = sessionData_->currentStatistic;
      CBData_->progressData.statistic.bytes = current_bytes;

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

continue_:
      event = TEST_I_GTKEVENT_STATISTIC;
      break;
    }
    default:
      return;
  } // end SWITCH

  CBData_->eventStack.push_back (event);
}

void
Test_I_Stream_Source_EventHandler::end ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  guint event_source_id = g_idle_add (idle_end_source_UI_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_end_source_UI_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);

  CBData_->eventStack.push_back (TEST_I_GTKEVENT_END);
}
