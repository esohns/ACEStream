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

#include "test_u_eventhandler.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_defines.h"

#if defined (GTK_SUPPORT)
#include "common_ui_gtk_manager_common.h"
#endif // GTK_SUPPORT

#include "stream_macros.h"

#if defined (GTK_SUPPORT)
#include "test_u_gtk_common.h"
#endif // GTK_SUPPORT

#if defined (GTK_SUPPORT)
#include "test_u_gtk_callbacks.h"
#endif // GTK_SUPPORT
#include "test_u_mic_visualize_common.h"
#include "test_u_mic_visualize_defines.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_DirectShow_EventHandler::Test_U_DirectShow_EventHandler (struct Test_U_MicVisualize_DirectShow_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_EventHandler::Test_U_DirectShow_EventHandler"));

}

void
Test_U_DirectShow_EventHandler::start (Stream_SessionId_t sessionId_in,
                                       const Test_U_MicVisualize_DirectShow_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<Test_U_MicVisualize_DirectShow_SessionData&> (sessionData_in);

#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

void
Test_U_DirectShow_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
  guint event_source_id = 0;
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      // *NOTE*: do not use g_idle_add, because that will never be called;
      //         the system is never idle while in-session, because it's busy
      //         updating the display...
      event_source_id = g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS,
                                       idle_session_end_cb,
                                       CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add(idle_session_end_cb): \"%m\", continuing\n")));
        goto continue_;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
      state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
    } // end lock scope
#endif // GTK_USE
  } // end IF

#if defined (GTK_USE)
continue_:
#endif // GTK_USE
  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_U_DirectShow_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                        const Test_U_DirectShow_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      CBData_->progressData.statistic.bytes += message_in.total_length ();
      state_r.eventStack.push (COMMON_UI_EVENT_DATA);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

void
Test_U_DirectShow_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                        const Test_U_DirectShow_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_DirectShow_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      event_e = COMMON_UI_EVENT_ABORT; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
      event_e = COMMON_UI_EVENT_STATISTIC; break;
    case STREAM_SESSION_MESSAGE_STEP:
      event_e = COMMON_UI_EVENT_STEP; break;
    default:
    {
      std::string type_string;
      Test_U_DirectShow_SessionMessage::MessageTypeToString (sessionMessage_in.type (),
                                                             type_string);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message (type was: \"%s\"), continuing\n"),
                  ACE_TEXT (type_string.c_str ())));
      break;
    }
  } // end SWITCH

#if defined (GTK_USE)
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (event_e);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

//////////////////////////////////////////

Test_U_MediaFoundation_EventHandler::Test_U_MediaFoundation_EventHandler (struct Test_U_MicVisualize_MediaFoundation_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_EventHandler::Test_U_MediaFoundation_EventHandler"));

}

void
Test_U_MediaFoundation_EventHandler::start (Stream_SessionId_t sessionId_in,
                                            const Test_U_MicVisualize_MediaFoundation_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<Test_U_MicVisualize_MediaFoundation_SessionData&> (sessionData_in);

  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

void
Test_U_MediaFoundation_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

#if defined (GTK_USE)
  guint event_source_id = 0;
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      event_source_id = g_idle_add (idle_session_end_cb,
                                    CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
        goto continue_;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
      state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
    } // end lock scope
#endif // GTK_USE
  } // end IF

#if defined (GTK_USE)
continue_:
#endif // GTK_USE
  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_U_MediaFoundation_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                             const Test_U_MediaFoundation_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

#if defined (GTK_USE)
  guint event_source_id = 0;
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      CBData_->progressData.statistic.bytes += message_in.total_length ();
      state_r.eventStack.push (COMMON_UI_EVENT_DATA);
      //event_source_id = g_idle_add (idle_update_display_cb,
      //                              CBData_);
      //if (event_source_id == 0)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to g_idle_add(idle_update_display_cb): \"%m\", returning\n")));
      //  return;
      //} // end IF
  //  CBData_->eventSourceIds.insert (event_source_id);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

void
Test_U_MediaFoundation_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                             const Test_U_MediaFoundation_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_MediaFoundation_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
      event_e = COMMON_UI_EVENT_ABORT; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
      event_e = COMMON_UI_EVENT_STATISTIC; break;
    case STREAM_SESSION_MESSAGE_STEP:
      event_e = COMMON_UI_EVENT_STEP; break;
    default:
    {
      std::string type_string;
      Test_U_MediaFoundation_SessionMessage::MessageTypeToString (sessionMessage_in.type (),
                                                                  type_string);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message (type was: \"%s\"), continuing\n"),
                  ACE_TEXT (type_string.c_str ())));
      break;
    }
  } // end SWITCH

  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (event_e);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}
#else
Test_U_EventHandler::Test_U_EventHandler (struct Test_U_MicVisualize_UI_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::Test_U_EventHandler"));

}

void
Test_U_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const Test_U_MicVisualize_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (!sessionData_);

  sessionData_ = &const_cast<Test_U_MicVisualize_SessionData&> (sessionData_in);

  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Stream_SessionMessageType& sessionEvent_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

#if defined (GTK_USE)
  guint event_source_id = 0;
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      // *NOTE*: do not use g_idle_add, because that will never be called;
      //         the system is never idle while in-session, because it's busy
      //         updating the display...
      event_source_id = g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS,
                                       idle_session_end_cb,
                                       CBData_);
      if (event_source_id == 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_timeout_add(idle_session_end_cb): \"%m\", continuing\n")));
        goto continue_;
      } // end IF
      state_r.eventSourceIds.insert (event_source_id);
      state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
    } // end lock scope
#endif // GTK_USE
  } // end IF

#if defined (GTK_USE)
continue_:
#endif // GTK_USE

  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

#if defined (GTK_USE)
//  guint event_source_id = 0;
#endif // GTK_USE
  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      CBData_->progressData.statistic.bytes += message_in.total_length ();
      state_r.eventStack.push (COMMON_UI_EVENT_DATA);
      //event_source_id = g_idle_add (idle_update_display_cb,
      //                              CBData_);
      //if (event_source_id == 0)
      //{
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to g_idle_add(idle_update_display_cb): \"%m\", returning\n")));
      //  return;
      //} // end IF
  //  CBData_->UIState.eventSourceIds.insert (event_source_id);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}

void
Test_U_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Test_U_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    {
#if defined (GTK_USE)
      guint event_source_id = 0;
#endif // GTK_USE
      if (CBData_)
      {
#if defined (GTK_USE)
        Common_UI_GTK_State_t& state_r =
          const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
          // *NOTE*: do not use g_idle_add, because that will never be called;
          //         the system is never idle while in-session, because it's busy
          //         updating the display...
          event_source_id = g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_CAIRO_MS,
                                           idle_session_end_cb,
                                           CBData_);
          if (event_source_id == 0)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to g_timeout_add(idle_session_end_cb): \"%m\", continuing\n")));
            goto continue_;
          } // end IF
          state_r.eventSourceIds.insert (event_source_id);
        } // end lock scope
#endif // GTK_USE
      } // end IF

#if defined (GTK_USE)
continue_:
#endif // GTK_USE

      event_e = COMMON_UI_EVENT_ABORT;

      if (sessionData_)
        sessionData_ = NULL;
      break;
    }
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      if (CBData_)
      {
#if defined (GTK_USE)
        Common_UI_GTK_State_t& state_r =
          const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
        { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
          CBData_->progressData.statistic = sessionData_->statistic;
        } // end lock scope
#endif // GTK_USE
      } // end IF

      event_e = COMMON_UI_EVENT_STATISTIC;
      break;
    }
    case STREAM_SESSION_MESSAGE_STEP:
      event_e = COMMON_UI_EVENT_STEP;
      break;
    default:
    {
      std::string type_string;
      Test_U_SessionMessage::MessageTypeToString (sessionMessage_in.type (),
                                                  type_string);
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message (type was: \"%s\"), continuing\n"),
                  ACE_TEXT (type_string.c_str ())));
      break;
    }
  } // end SWITCH

  if (CBData_)
  {
#if defined (GTK_USE)
    Common_UI_GTK_State_t& state_r =
      const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
      state_r.eventStack.push (event_e);
    } // end lock scope
#endif // GTK_USE
  } // end IF
}
#endif // ACE_WIN32 || ACE_WIN64
