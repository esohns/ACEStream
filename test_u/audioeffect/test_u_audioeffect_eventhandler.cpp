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

#include "test_u_audioeffect_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "test_u_audioeffect_callbacks.h"
#include "test_u_audioeffect_defines.h"

#include "test_u_gtk_common.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
Test_U_AudioEffect_DirectShow_EventHandler::Test_U_AudioEffect_DirectShow_EventHandler (Test_U_AudioEffect_DirectShow_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::Test_U_AudioEffect_DirectShow_EventHandler"));

}

Test_U_AudioEffect_DirectShow_EventHandler::~Test_U_AudioEffect_DirectShow_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::~Test_U_AudioEffect_DirectShow_EventHandler"));

}

void
Test_U_AudioEffect_DirectShow_EventHandler::start (Stream_SessionId_t sessionID_in,
                                                   const Test_U_AudioEffect_DirectShow_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::start"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (sessionData_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_START);
}

void
Test_U_AudioEffect_DirectShow_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                                    const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_AudioEffect_DirectShow_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::end"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_END);

  guint event_source_id = g_idle_add (idle_session_end_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);

  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_U_AudioEffect_DirectShow_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                                    const Test_U_AudioEffect_DirectShow_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (sessionData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->progressData.statistic.bytes += message_in.total_length ();
  CBData_->eventStack.push_back (TEST_U_GTKEVENT_DATA);

  guint event_source_id = g_idle_add (idle_update_display_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_update_display_cb): \"%m\", returning\n")));
    return;
  } // end IF
//  CBData_->eventSourceIds.insert (event_source_id);
}
void
Test_U_AudioEffect_DirectShow_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                                    const Test_U_AudioEffect_DirectShow_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_DirectShow_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Stream_GTK_Event event =
    ((sessionMessage_in.type () == STREAM_SESSION_MESSAGE_STATISTIC) ? TEST_U_GTKEVENT_STATISTIC
                                                                     : STREAM_GKTEVENT_INVALID);

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->eventStack.push_back (event);
  } // end lock scope
}

//////////////////////////////////////////

Test_U_AudioEffect_MediaFoundation_EventHandler::Test_U_AudioEffect_MediaFoundation_EventHandler (Test_U_AudioEffect_MediaFoundation_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::Test_U_AudioEffect_MediaFoundation_EventHandler"));

}

Test_U_AudioEffect_MediaFoundation_EventHandler::~Test_U_AudioEffect_MediaFoundation_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::~Test_U_AudioEffect_MediaFoundation_EventHandler"));

}

void
Test_U_AudioEffect_MediaFoundation_EventHandler::start (Stream_SessionId_t sessionID_in,
                                                        const Test_U_AudioEffect_MediaFoundation_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::start"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (!sessionData_);

  sessionData_ =
    &const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (sessionData_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_START);
}

void
Test_U_AudioEffect_MediaFoundation_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                                         const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_AudioEffect_MediaFoundation_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::end"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_END);

  guint event_source_id = g_idle_add (idle_session_end_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);

  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_U_AudioEffect_MediaFoundation_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                                         const Test_U_AudioEffect_MediaFoundation_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->progressData.statistic.bytes += message_in.total_length ();
  CBData_->eventStack.push_back (TEST_U_GTKEVENT_DATA);

  guint event_source_id = g_idle_add (idle_update_display_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_update_display_cb): \"%m\", returning\n")));
    return;
  } // end IF
//  CBData_->eventSourceIds.insert (event_source_id);
}
void
Test_U_AudioEffect_MediaFoundation_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                                         const Test_U_AudioEffect_MediaFoundation_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_MediaFoundation_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Stream_GTK_Event event =
    ((sessionMessage_in.type () == STREAM_SESSION_MESSAGE_STATISTIC) ? TEST_U_GTKEVENT_STATISTIC
                                                                     : STREAM_GKTEVENT_INVALID);

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->eventStack.push_back (event);
  } // end lock scope
}
#else
Test_U_AudioEffect_EventHandler::Test_U_AudioEffect_EventHandler (Test_U_AudioEffect_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::Test_U_AudioEffect_EventHandler"));

}

Test_U_AudioEffect_EventHandler::~Test_U_AudioEffect_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::~Test_U_AudioEffect_EventHandler"));

}

void
Test_U_AudioEffect_EventHandler::start (Stream_SessionId_t sessionID_in,
                                        const Test_U_AudioEffect_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::start"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (!sessionData_);

  sessionData_ = &const_cast<Test_U_AudioEffect_SessionData&> (sessionData_in);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_START);
}

void
Test_U_AudioEffect_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                         const Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Test_U_AudioEffect_EventHandler::end (Stream_SessionId_t sessionID_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::end"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  CBData_->eventStack.push_back (TEST_U_GTKEVENT_END);

  guint event_source_id = g_idle_add (idle_session_end_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", returning\n")));
    return;
  } // end IF
  CBData_->eventSourceIds.insert (event_source_id);

  if (sessionData_)
    sessionData_ = NULL;
}

void
Test_U_AudioEffect_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                         const Test_U_AudioEffect_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

    CBData_->progressData.statistic.bytes += message_in.total_length ();
    CBData_->eventStack.push_back (TEST_U_GTKEVENT_DATA);
  } // end lock scope

  guint event_source_id = g_idle_add (idle_update_display_cb,
                                      CBData_);
  if (event_source_id == 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_idle_add(idle_update_display_cb): \"%m\", returning\n")));
    return;
  } // end IF
//  CBData_->eventSourceIds.insert (event_source_id);
}
void
Test_U_AudioEffect_EventHandler::notify (Stream_SessionId_t sessionID_in,
                                         const Test_U_AudioEffect_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_U_AudioEffect_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionID_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, CBData_->lock);

  Test_U_GTK_Event event = TEST_U_GTKEVENT_INVALID;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
      ACE_ASSERT (sessionData_);

      CBData_->progressData.statistic = sessionData_->currentStatistic;

      event = TEST_U_GTKEVENT_STATISTIC;
      break;
    }
    default:
      return;
  } // end SWITCH

  CBData_->eventStack.push_back (event);
}
#endif
