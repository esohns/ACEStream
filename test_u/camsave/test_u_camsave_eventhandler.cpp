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

#include "test_u_camsave_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "stream_macros.h"

#include "test_u_camsave_defines.h"
#include "test_u_camsave_callbacks.h"

Stream_CamSave_EventHandler::Stream_CamSave_EventHandler (Stream_CamSave_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::Stream_CamSave_EventHandler"));

}

Stream_CamSave_EventHandler::~Stream_CamSave_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::~Stream_CamSave_EventHandler"));

}

void
Stream_CamSave_EventHandler::start (const Stream_CamSave_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::start"));

  sessionData_ = &sessionData_in;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.size = sessionData_->size;
  CBData_->eventStack.push_back (STREAM_GTKEVENT_START);
}

void
Stream_CamSave_EventHandler::notify (const Stream_CamSave_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.received += message_in.total_length ();
  CBData_->eventStack.push_back (STREAM_GTKEVENT_DATA);
}
void
Stream_CamSave_EventHandler::notify (const Stream_CamSave_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Stream_GTK_Event event = STREAM_GKTEVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_STATISTIC:
      event = STREAM_GTKEVENT_STATISTIC; break;
    default:
      return;
  } // end SWITCH

  {
    ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

    CBData_->eventStack.push_back (event);
  } // end lock scope
}

void
Stream_CamSave_EventHandler::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->lock);

  CBData_->eventStack.push_back (STREAM_GTKEVENT_END);

  guint event_source_id = g_idle_add (idle_session_end_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_start_target_UI_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);
}
