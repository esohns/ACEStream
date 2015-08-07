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

#include "test_u_filecopy_eventhandler.h"

#include "ace/Guard_T.h"
#include "ace/Synch_Traits.h"

#include "stream_macros.h"

Stream_Filecopy_EventHandler::Stream_Filecopy_EventHandler (Stream_Filecopy_GTK_CBData* CBData_in)
 : CBData_ (CBData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::Stream_Filecopy_EventHandler"));

}

Stream_Filecopy_EventHandler::~Stream_Filecopy_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::~Stream_Filecopy_EventHandler"));

}

void
Stream_Filecopy_EventHandler::start (const Stream_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::start"));

  ACE_UNUSED_ARG (sessionData_in);

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (STREAM_GTKEVENT_START);
}

void
Stream_Filecopy_EventHandler::notify (const Stream_Filecopy_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::notify"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (STREAM_GTKEVENT_DATA);
}

void
Stream_Filecopy_EventHandler::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Filecopy_EventHandler::end"));

  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (CBData_->stackLock);

  CBData_->eventStack.push_back (STREAM_GTKEVENT_END);
}
