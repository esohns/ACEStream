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
#include "test_u_camsave_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "stream_macros.h"

#include "test_u_camsave_defines.h"
#include "test_u_camsave_callbacks.h"

Stream_CamSave_EventHandler::Stream_CamSave_EventHandler (struct Stream_CamSave_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::Stream_CamSave_EventHandler"));

}

void
Stream_CamSave_EventHandler::start (Stream_SessionId_t sessionId_in,
                                    const Stream_CamSave_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::start"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<struct Stream_CamSave_SessionData&> (sessionData_in);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
}

void
Stream_CamSave_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                     const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Stream_CamSave_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    event_source_id = g_idle_add (idle_session_end_cb,
                                  CBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", returning\n")));
      return;
    } // end IF
    CBData_->eventSourceIds.insert (event_source_id);

    CBData_->eventStack.push (COMMON_UI_EVENT_FINISHED);
  } // end lock scope

  if (sessionData_)
    sessionData_ = NULL;
}

void
Stream_CamSave_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                     const Stream_CamSave_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->progressData.statistic.bytes += message_in.total_length ();
    CBData_->eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope

  guint event_source_id = g_idle_add (idle_update_video_display_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_update_video_display_cb): \"%m\", returning\n")));
    return;
  } // end IF
//  CBData_->eventSourceIds.insert (event_source_id);
}
void
Stream_CamSave_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                     const Stream_CamSave_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CamSave_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      float current_bytes = 0.0F;

      // sanity check(s)
      if (!sessionData_)
        goto continue_;

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
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
        CBData_->progressData.statistic = sessionData_->statistic;
        CBData_->progressData.statistic.bytes = current_bytes;

        if (sessionData_->lock)
        {
          result = sessionData_->lock->release ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
        } // end IF
      } // end lock scope

continue_:
      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
      return;
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push (event_e);
  } // end lock scope
}
