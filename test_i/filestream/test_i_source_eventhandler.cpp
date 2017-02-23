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

#include <ace/Synch.h>
#include "test_i_source_eventhandler.h"

#include <ace/Guard_T.h>
#include <ace/Synch_Traits.h>

#include <gtk/gtk.h>

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "test_i_callbacks.h"
#include "test_i_common.h"
#include "test_i_defines.h"

Test_I_Source_EventHandler::Test_I_Source_EventHandler (Test_I_Source_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::Test_I_Source_EventHandler"));

}

Test_I_Source_EventHandler::~Test_I_Source_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::~Test_I_Source_EventHandler"));

}

void
Test_I_Source_EventHandler::start (Stream_SessionId_t sessionID_in,
                                   const struct Test_I_Source_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::start"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  //ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<struct Test_I_Source_SessionData&> (sessionData_in);

  int result = -1;

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  if (sessionData_->lock)
  {
    result = sessionData_->lock->acquire ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
  } // end IF
  CBData_->progressData.size = sessionData_->size;
  if (sessionData_->lock)
  {
    result = sessionData_->lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  CBData_->eventStack.push_back (TEST_I_GTKEVENT_START);
}

void
Test_I_Source_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                    const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_I_Source_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::end"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

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

  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_I_Source_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                    const Test_I_Source_Message_t& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  unsigned int length = message_in.total_length ();
  CBData_->progressData.transferred += length;
  CBData_->eventStack.push_back (TEST_I_GTKEVENT_DATA);
}
void
Test_I_Source_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                    const Test_I_Source_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Source_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  Test_I_GTK_Event event = TEST_I_GTKEVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
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
      CBData_->progressData.statistic = sessionData_->currentStatistic;
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

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push_back (event);
  } // end lock scope
}
