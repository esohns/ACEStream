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

HTTPGet_EventHandler::HTTPGet_EventHandler (struct HTTPGet_GtkCBData* GtkCBData_in,
                                            bool consoleMode_in)
 : consoleMode_ (consoleMode_in)
 , GtkCBData_ (GtkCBData_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::HTTPGet_EventHandler"));

  // sanity check(s)
  ACE_ASSERT (GtkCBData_);
}

HTTPGet_EventHandler::~HTTPGet_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::~HTTPGet_EventHandler"));

}

void
HTTPGet_EventHandler::start (Stream_SessionId_t sessionID_in,
                             const struct HTTPGet_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::start"));

  // sanity check(s)
  ACE_ASSERT (GtkCBData_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, GtkCBData_->lock);

    guint event_source_id = g_idle_add (idle_session_start_cb,
                                        GtkCBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_start_cb): \"%m\", returning\n")));
      return;
    } // end IF
    GtkCBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionID_in,
                              const enum Stream_SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionID_in,
                              const HTTPGet_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (GtkCBData_);

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, GtkCBData_->lock);

    GtkCBData_->eventStack.push_back (COMMON_UI_EVENT_DATA);
  } // end lock scope
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionID_in,
                              const HTTPGet_SessionMessage& message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (GtkCBData_);
  ACE_ASSERT (GtkCBData_->progressData);

  int result = -1;
  enum Common_UI_Event event = COMMON_UI_EVENT_INVALID;
  switch (message_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
      event = COMMON_UI_EVENT_CONNECT; break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event = COMMON_UI_EVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_ABORT:
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_UNLINK:
      event = COMMON_UI_EVENT_CONTROL; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      const HTTPGet_SessionData_t& session_data_container_r =
        message_in.get ();
      struct HTTPGet_SessionData& session_data_r =
        const_cast<struct HTTPGet_SessionData&> (session_data_container_r.get ());

      if (session_data_r.lock)
      {
        result = session_data_r.lock->acquire ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
      } // end IF

      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, GtkCBData_->lock);
        GtkCBData_->progressData->statistic = session_data_r.statistic;
      } // end lock scope

      if (session_data_r.lock)
      {
        result = session_data_r.lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

      event = COMMON_UI_EVENT_CONTROL;
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
  GtkCBData_->eventStack.push_back (event);
}

void
HTTPGet_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::end"));

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, GtkCBData_->lock);

    GtkCBData_->eventStack.push_back (COMMON_UI_EVENT_DISCONNECT);

    guint event_source_id = g_idle_add (idle_session_end_cb,
                                        GtkCBData_);
    if (event_source_id == 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", returning\n")));
      return;
    } // end IF
    GtkCBData_->eventSourceIds.insert (event_source_id);
  } // end lock scope
}
