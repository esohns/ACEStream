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

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
#include "gtk/gtk.h"
#endif // GTK_USE
#endif // GUI_SUPPORT

#include "ace/Synch.h"
#include "test_u_filecopy_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "test_u_filecopy_common.h"
#include "test_u_filecopy_defines.h"

Stream_Filecopy_EventHandler::Stream_Filecopy_EventHandler (struct Stream_Filecopy_UI_CBData* CBData_in)
#if defined (GUI_SUPPORT)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
#else
 : sessionData_ (NULL)
#endif // GUI_SUPPORT
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::Stream_Filecopy_EventHandler"));

}

void
Stream_Filecopy_EventHandler::start (Stream_SessionId_t sessionId_in,
                                     const struct Stream_Filecopy_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT
  ACE_ASSERT (!sessionData_);

  int result = -1;

  sessionData_ =
    &const_cast<struct Stream_Filecopy_SessionData&> (sessionData_in);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
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

//  //Common_UI_GladeXMLsIterator_t iterator =
//  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
//  Common_UI_GTKBuildersIterator_t iterator =
//    CBData_->UIState.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

//  // sanity check(s)
//  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
//  ACE_ASSERT (iterator != CBData_->UIState.builders.end ());

//  gdk_threads_enter ();
//  gdk_threads_leave ();

#if defined (GTK_USE)
    state_r.eventStack.push (COMMON_UI_EVENT_STARTED);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}

void
Stream_Filecopy_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                      const enum Stream_SessionMessageType& sessionEvent_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionEvent_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Stream_Filecopy_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

  Common_UI_GTK_BuildersIterator_t iterator;
  GtkTable* table_p = NULL;
  GtkAction* action_p = NULL;
#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    iterator =
      state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
    // sanity check(s)
    ACE_ASSERT (iterator != state_r.builders.end ());

    gdk_threads_enter ();
    table_p =
      GTK_TABLE (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TABLE_OPTIONS_NAME)));
    ACE_ASSERT (table_p);
    gtk_widget_set_sensitive (GTK_WIDGET (table_p), TRUE);

    action_p =
      GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_START_NAME)));
    ACE_ASSERT (action_p);
    gtk_action_set_stock_id (action_p, GTK_STOCK_MEDIA_PLAY);
    action_p =
      GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_STOP_NAME)));
    ACE_ASSERT (action_p);
    gtk_action_set_sensitive (action_p, FALSE);
    gdk_threads_leave ();
    state_r.eventStack.push (COMMON_UI_EVENT_FINISHED);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT

  if (sessionData_)
    sessionData_ = NULL;
}

void
Stream_Filecopy_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                      const Stream_Filecopy_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
#endif // GTK_USE
    CBData_->progressData.copied += message_in.total_length ();
#if defined (GTK_USE)
    state_r.eventStack.push (COMMON_UI_EVENT_DATA);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}
void
Stream_Filecopy_EventHandler::notify (Stream_SessionId_t sessionId_in,
                                      const Stream_Filecopy_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
#if defined (GUI_SUPPORT)
  ACE_ASSERT (CBData_);
#endif // GUI_SUPPORT

  enum Common_UI_EventType event_e =
    ((sessionMessage_in.type () == STREAM_SESSION_MESSAGE_STATISTIC) ? COMMON_UI_EVENT_STATISTIC
                                                                     : COMMON_UI_EVENT_INVALID);

#if defined (GUI_SUPPORT)
#if defined (GTK_USE)
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR_2 ());
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    state_r.eventStack.push (event_e);
  } // end lock scope
#endif // GTK_USE
#endif // GUI_SUPPORT
}
