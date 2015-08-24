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

#include "test_i_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "test_i_defines.h"

Stream_EventHandler::Stream_EventHandler (Stream_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
 , sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_EventHandler::Stream_EventHandler"));

}

Stream_EventHandler::~Stream_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_EventHandler::~Stream_EventHandler"));

}

void
Stream_EventHandler::start (const Test_I_Stream_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_EventHandler::start"));

  sessionData_ = &sessionData_in;

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.size = sessionData_->size;

//  //Common_UI_GladeXMLsIterator_t iterator =
//  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
//  Common_UI_GTKBuildersIterator_t iterator =
//    CBData_->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

//  // sanity check(s)
//  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
//  ACE_ASSERT (iterator != CBData_->builders.end ());

//  gdk_threads_enter ();
//  gdk_threads_leave ();

  CBData_->eventStack.push_back (STREAM_GTKEVENT_START);
}

void
Stream_EventHandler::notify (const Stream_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);
  ACE_ASSERT (sessionData_);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->lock);

  CBData_->progressData.sent += message_in.total_length ();

  CBData_->eventStack.push_back (STREAM_GTKEVENT_DATA);
}
void
Stream_EventHandler::notify (const Stream_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  Stream_GTK_Event event =
    ((sessionMessage_in.type () == STREAM_SESSION_STATISTIC) ? STREAM_GTKEVENT_STATISTIC
                                                             : STREAM_GKTEVENT_INVALID);

  {
    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->lock);

    CBData_->eventStack.push_back (event);
  } // end lock scope
}

void
Stream_EventHandler::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_EventHandler::end"));

  // sanity check(s)
  ACE_ASSERT (CBData_);

  ACE_Guard<ACE_SYNCH_MUTEX> aGuard (CBData_->lock);

  //Common_UI_GladeXMLsIterator_t iterator =
  //  data_p->gladeXML.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  Common_UI_GTKBuildersIterator_t iterator =
    CBData_->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));

  // sanity check(s)
  //ACE_ASSERT (iterator != CBData_->gladeXML.end ());
  ACE_ASSERT (iterator != CBData_->builders.end ());

  gdk_threads_enter ();
  GtkTable* table_p =
    GTK_TABLE (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_TABLE_OPTIONS_NAME)));
  ACE_ASSERT (table_p);
  gtk_widget_set_sensitive (GTK_WIDGET (table_p), TRUE);

  GtkAction* action_p =
      GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_START_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_stock_id (action_p, GTK_STOCK_MEDIA_PLAY);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_I_STREAM_UI_GTK_ACTION_STOP_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);
  gdk_threads_leave ();

  CBData_->eventStack.push_back (STREAM_GTKEVENT_END);
}
