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

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#if defined (GTK_SUPPORT)
#include "gtk/gtk.h"
#endif // GTK_SUPPORT

#include "stream_macros.h"

#include "stream_vis_defines.h"

#include "test_i_target_session_message.h"
#include "test_i_target_stream.h"
#if defined (GTK_SUPPORT)
#include "test_i_callbacks.h"
#endif // GTK_SUPPORT

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
Test_I_Target_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::Test_I_Target_EventHandler_T (CallbackDataType* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler_T::Test_I_Target_EventHandler_T"));

}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_Target_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::start (Stream_SessionId_t sessionId_in,
                                                       const SessionDataType& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler_T::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (!sessionData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  sessionData_ = &const_cast<SessionDataType&> (sessionData_in);

#if defined (GTK_USE)
  CBData_->progressData.transferred = 0;

  ACE_ASSERT (!sessionData_->connectionStates.empty ());
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_I_Target_DirectShow_TCPConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_DIRECTSHOW_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  Net_IINETConnection_t* connection_p =
    connection_manager_p->get ((*sessionData_->connectionStates.begin ()).first);
  ACE_ASSERT (connection_p);
  Test_I_Target_DirectShow_ITCPConnection_t* tcp_connection_p =
    dynamic_cast<Test_I_Target_DirectShow_ITCPConnection_t*> (connection_p);
  ACE_ASSERT (tcp_connection_p);
  CBData_->stream =
    &const_cast<Test_I_Target_DirectShow_TCPStream&> (tcp_connection_p->stream ());
#else
  Test_I_Target_TCPConnectionManager_t* connection_manager_p =
    TEST_I_TARGET_TCP_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connection_manager_p);
  Net_IINETConnection_t* connection_p =
    connection_manager_p->get ((*sessionData_->connectionStates.begin ()).first);
  ACE_ASSERT (connection_p);
  Test_I_Target_ITCPConnection_t* tcp_connection_p =
    dynamic_cast<Test_I_Target_ITCPConnection_t*> (connection_p);
  ACE_ASSERT (tcp_connection_p);
  CBData_->stream =
    &const_cast<Test_I_Target_TCPStream&> (tcp_connection_p->stream ());
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (CBData_->stream);
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (CBData_->stream->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_CAIRO_DEFAULT_NAME_STRING)));
  ACE_ASSERT (module_p);
  CBData_->dispatch = dynamic_cast<Common_IDispatch*> (module_p->writer ());
  ACE_ASSERT (CBData_->dispatch);
  connection_p->decrease (); connection_p = NULL;

  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    event_source_id = g_idle_add (idle_start_target_UI_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_start_target_UI_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
#endif // GTK_USE
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_Target_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler_T::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  CBData_->dispatch = NULL;

  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    event_source_id = g_idle_add (idle_end_target_UI_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_end_target_UI_cb): \"%m\", continuing\n")));
    else
      state_r.eventSourceIds.insert (event_source_id);
    state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
  } // end lock scope
#endif // GTK_USE

  if (sessionData_)
    sessionData_ = NULL;
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_Target_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                        const MessageType& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

#if defined (GTK_USE)
  guint event_source_id = 0;
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    CBData_->progressData.transferred += message_in.total_length ();
    event_source_id = g_idle_add (idle_update_video_display_cb,
                                  CBData_);
    if (!event_source_id)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_idle_add(idle_update_video_display_cb): \"%m\", continuing\n")));
    //else
    //  CBData_->UIState.eventSourceIds.insert (event_source_id);
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE
}

template <typename SessionDataType,
          typename SessionEventType,
          typename MessageType,
          typename SessionMessageType,
          typename CallbackDataType>
void
Test_I_Target_EventHandler_T<SessionDataType,
                             SessionEventType,
                             MessageType,
                             SessionMessageType,
                             CallbackDataType>::notify (Stream_SessionId_t sessionId_in,
                                                        const SessionMessageType& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Target_EventHandler_T::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (CBData_);

#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
#endif // GTK_USE

  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
  switch (sessionMessage_in.type ())
  {
    case STREAM_SESSION_MESSAGE_ABORT:
    case STREAM_SESSION_MESSAGE_CONNECT:
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      return;
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

#if defined (GTK_USE)
      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
        CBData_->progressData.statistic = sessionData_->statistic;
#if defined (GTK_USE)
      } // end lock scope
#endif // GTK_USE

      if (sessionData_->lock)
      {
        result = sessionData_->lock->release ();
        if (result == -1)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF

continue_:
      event_e = COMMON_UI_EVENT_STATISTIC;
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

#if defined (GTK_USE)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE
}
