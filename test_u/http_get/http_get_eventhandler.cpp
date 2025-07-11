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

#include "http_get_eventhandler.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Synch_Traits.h"

#include "stream_macros.h"

#include "stream_dec_common.h"

#include "http_get_common.h"
#if defined (GTK_SUPPORT)
#include "http_get_callbacks.h"
#endif // GTK_SUPPORT

HTTPGet_EventHandler::HTTPGet_EventHandler (struct HTTPGet_UI_CBData* CBData_in,
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

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    event_source_id = g_idle_add (idle_session_start_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_start_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
#endif // GTK_USE
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionId_in,
                              const enum Stream_SessionMessageType& messageType_in,
                              bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (messageType_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
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

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  CBData_->progressData.statistic.bytes += message_in.total_length ();

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE
}

void
HTTPGet_EventHandler::notify (Stream_SessionId_t sessionId_in,
                              const HTTPGet_SessionMessage& message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  switch (message_in.type ())
  {
    case STREAM_SESSION_MESSAGE_CONNECT:
      event_e = COMMON_UI_EVENT_CONNECT; break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      event_e = COMMON_UI_EVENT_DISCONNECT; break;
    case STREAM_SESSION_MESSAGE_ABORT:
    {
      event_e = COMMON_UI_EVENT_ABORT;
      endSession ();
      break;
    }
    case STREAM_SESSION_MESSAGE_LINK:
    case STREAM_SESSION_MESSAGE_UNLINK:
      event_e = COMMON_UI_EVENT_SESSION; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      const HTTPGet_SessionData_t& session_data_container_r =
        message_in.getR ();
      struct HTTPGet_SessionData& session_data_r =
        const_cast<struct HTTPGet_SessionData&> (session_data_container_r.getR ());

#if defined (GTK_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
        if (session_data_r.lock)
        {
          result = session_data_r.lock->acquire ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
        } // end IF

        CBData_->progressData.statistic = session_data_r.statistic;

        if (session_data_r.lock)
        {
          result = session_data_r.lock->release ();
          if (result == -1)
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
        } // end IF
#if defined (GTK_USE)
      } // end lock scope
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
    {
#if defined (GTK_USE)
      guint event_source_id = 0;
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
        event_source_id = g_idle_add (idle_session_end_cb,
                                      CBData_);
        if (!event_source_id)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
        else
          state_r.eventSourceIds.insert (event_source_id);
      } // end lock scope
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_STEP;
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

#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#else
  ACE_UNUSED_ARG (event_e);
#endif // GTK_USE
}

void
HTTPGet_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  endSession ();

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
  } // end lock scope
#endif // GTK_USE
}

void
HTTPGet_EventHandler::endSession ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    event_source_id = g_idle_add (idle_session_end_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
  } // end lock scope
#endif // GTK_USE
}
