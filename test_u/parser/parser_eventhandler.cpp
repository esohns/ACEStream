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

#include "parser_eventhandler.h"

//#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
//#include "ace/Synch_Traits.h"

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//#include "gtk/gtk.h"
//#endif // GTK_USE
//#endif // GUI_SUPPORT
//#include "common_ui_common.h"

#include "stream_macros.h"

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//#include "http_get_callbacks.h"
//#endif // GTK_USE
//#endif // GUI_SUPPORT

Parser_EventHandler::Parser_EventHandler (
//#if defined (GUI_SUPPORT)
//                                            struct Parser_UI_CBData* CBData_in,
//#endif // GUI_SUPPORT
                                            bool consoleMode_in)
 : consoleMode_ (consoleMode_in)
//#if defined (GUI_SUPPORT)
// , CBData_ (CBData_in)
//#endif // GUI_SUPPORT
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::Parser_EventHandler"));

  // sanity check(s)
//#if defined (GUI_SUPPORT)
//  ACE_ASSERT (CBData_);
//#endif // GUI_SUPPORT
}

void
Parser_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Parser_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

  // sanity check(s)
//#if defined (GUI_SUPPORT)
//  ACE_ASSERT (CBData_);
//#endif // GUI_SUPPORT

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
//  guint event_source_id = 0;
//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//    event_source_id = g_idle_add (idle_session_start_cb,
//                                  CBData_);
//    if (!event_source_id)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_session_start_cb): \"%m\", continuing\n")));
//    else
//      state_r.eventSourceIds.insert (event_source_id);
//    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
//  } // end lock scope
//#endif // GTK_USE
//#endif // GUI_SUPPORT
}

void
Parser_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (messageType_in);
}

void
Parser_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Parser_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
//#if defined (GUI_SUPPORT)
//  ACE_ASSERT (CBData_);
//#endif // GUI_SUPPORT
  ACE_ASSERT (message_in.isInitialized ());

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
//  } // end lock scope
//#endif // GTK_USE
//#endif // GUI_SUPPORT

  const Parser_MessageData_t& data_container_r = message_in.getR ();
  const struct Parser_MessageData& data_r = data_container_r.getR ();
  ACE_ASSERT (data_r.dictionary);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT("%s\n"),
              ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*data_r.dictionary).c_str ())));
}

void
Parser_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Parser_SessionMessage& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
//#if defined (GUI_SUPPORT)
//  ACE_ASSERT (CBData_);
//#endif // GUI_SUPPORT

//  int result = -1;
  //enum Common_UI_EventType event_e = COMMON_UI_EVENT_SESSION;
//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
//#endif // GTK_USE
//#endif // GUI_SUPPORT
  switch (message_in.type ())
  {
    //case STREAM_SESSION_MESSAGE_CONNECT:
    //  event_e = COMMON_UI_EVENT_CONNECT; break;
    //case STREAM_SESSION_MESSAGE_DISCONNECT:
    //  event_e = COMMON_UI_EVENT_DISCONNECT; break;
    //case STREAM_SESSION_MESSAGE_ABORT:
    //case STREAM_SESSION_MESSAGE_LINK:
    //case STREAM_SESSION_MESSAGE_UNLINK:
      //event_e = COMMON_UI_EVENT_CONTROL; break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
//      const Parser_SessionData_t& session_data_container_r =
//        message_in.getR ();
//      struct Parser_SessionData& session_data_r =
//        const_cast<struct Parser_SessionData&> (session_data_container_r.getR ());

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//      { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//#endif // GTK_USE
//        if (session_data_r.lock)
//        {
//          result = session_data_r.lock->acquire ();
//          if (result == -1)
//            ACE_DEBUG ((LM_ERROR,
//                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
//        } // end IF
//
//        CBData_->progressData.statistic = session_data_r.statistic;
//
//        if (session_data_r.lock)
//        {
//          result = session_data_r.lock->release ();
//          if (result == -1)
//            ACE_DEBUG ((LM_ERROR,
//                        ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
//        } // end IF
//#if defined (GTK_USE)
//      } // end lock scope
//#endif // GTK_USE
//#endif // GUI_SUPPORT

      //event_e = COMMON_UI_EVENT_STATISTIC;
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

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//    state_r.eventStack.push (event_e);
//  } // end lock scope
//#else
//  ACE_UNUSED_ARG (event_e);
//#endif // GTK_USE
//#endif // GUI_SUPPORT
}

void
Parser_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
//#if defined (GUI_SUPPORT)
//  ACE_ASSERT (CBData_);
//#endif // GUI_SUPPORT

//#if defined (GUI_SUPPORT)
//#if defined (GTK_USE)
//  Common_UI_GTK_Manager_t* gtk_manager_p =
//    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
//  ACE_ASSERT (gtk_manager_p);
//  Common_UI_GTK_State_t& state_r =
//    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
//  guint event_source_id = 0;
//  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
//    event_source_id = g_idle_add (idle_session_end_cb,
//                                  CBData_);
//    if (!event_source_id)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
//    else
//      state_r.eventSourceIds.insert (event_source_id);
//    state_r.eventStack.push (COMMON_UI_EVENT_STOPPED);
//  } // end lock scope
//#endif // GTK_USE
//#endif // GUI_SUPPORT
}
