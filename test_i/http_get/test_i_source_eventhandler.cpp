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

#include "test_i_source_eventhandler.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Test_I_Stream_Source_EventHandler::Test_I_Stream_Source_EventHandler ()
 : sessionData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::Test_I_Stream_Source_EventHandler"));

}

Test_I_Stream_Source_EventHandler::~Test_I_Stream_Source_EventHandler ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::~Test_I_Stream_Source_EventHandler"));

}

void
Test_I_Stream_Source_EventHandler::start (const Test_I_Stream_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::start"));

  sessionData_ = &sessionData_in;
}

void
Test_I_Stream_Source_EventHandler::notify (const Test_I_Stream_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::notify"));

  // sanity check(s)
  ACE_ASSERT (sessionData_);
}
void
Test_I_Stream_Source_EventHandler::notify (const Test_I_Stream_SessionMessage& sessionMessage_in)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::notify"));
}

void
Test_I_Stream_Source_EventHandler::end ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_Stream_Source_EventHandler::end"));
}
