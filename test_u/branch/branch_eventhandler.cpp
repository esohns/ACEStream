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

#include "branch_eventhandler.h"

#include "ace/Log_Msg.h"

#include "common_ui_common.h"

#include "stream_macros.h"

Branch_EventHandler::Branch_EventHandler (bool consoleMode_in)
 : consoleMode_ (consoleMode_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_EventHandler::Branch_EventHandler"));

}

void
Branch_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Branch_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);

}

void
Branch_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& messageType_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (messageType_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Branch_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Branch_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  ACE_ASSERT (message_in.isInitialized ());

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("received message (id: %u, session id: %u)\n"),
              message_in.id (),
              sessionId_in));
}

void
Branch_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Branch_SessionMessage& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  switch (message_in.type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received session begin message (session id: %u)\n"),
                  sessionId_in));
      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("received session end message (session id: %u)\n"),
                  sessionId_in));
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
}

void
Branch_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Branch_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);
}
