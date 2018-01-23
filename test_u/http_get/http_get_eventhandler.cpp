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

#include "stream_dec_common.h"

#include "ace/Synch.h"
#include "http_get_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "stream_macros.h"

#include "http_get_common.h"
#include "http_get_callbacks.h"

HTTPGet_EventHandler::HTTPGet_EventHandler (struct HTTPGet_GtkCBData* CBData_in,
                                            bool consoleMode_in)
 : consoleMode_ (consoleMode_in)
 , CBData_ (CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::HTTPGet_EventHandler"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
}

void
HTTPGet_EventHandler::start (Stream_SessionId_t sessionId_in,
                             const struct HTTPGet_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    event_source_id = g_idle_add (idle_session_start_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_start_cb): \"%m\", continuing\n")));
    else
      CBData_->eventSourceIds.insert (event_source_id);
    CBData_->eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionId_in,
                              const enum Stream_SessionMessageType& messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (messageType_in);
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionId_in,
                              const HTTPGet_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (message_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionId_in,
                              const HTTPGet_SessionMessage& message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (CBData_->progressData);

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (message_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT; break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_ABORT:
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_UNLINK:
      event_e = COMMON_UI_EVENT_CONTROL; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      const HTTPGet_SessionData_t& session_data_container_r =
        message_in.getR ();
      struct HTTPGet_SessionData& session_data_r =
        const_cast<struct HTTPGet_SessionData&> (session_data_container_r.getR ());

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
        if (session_data_r.lock)
        {
          result = session_data_r.lock->acquire ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
        } // end IF

        CBData_->progressData->statistic = session_data_r.statistic;

        if (session_data_r.lock)
        {
          result = session_data_r.lock->release ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
        } // end IF
      } // end lock scope

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), returning\n"),
                  message_in.type ()));
      return;
    }
  } // end SWITCH

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    CBData_->eventStack.push (event_e);
  } // end lock scope
}

void
HTTPGet_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);
    event_source_id = g_idle_add (idle_session_end_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
    else
      CBData_->eventSourceIds.insert (event_source_id);
    CBData_->eventStack.push (COMMON_UI_EVENT_STOPPED);
  } // end lock scope
}
