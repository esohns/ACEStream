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

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Parser_EventHandler::Parser_EventHandler (bool consoleMode_in)
 : consoleMode_ (consoleMode_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::Parser_EventHandler"));

}

void
Parser_EventHandler::start (Stream_SessionId_t sessionId_in,
                            const struct Parser_SessionData& sessionData_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::start"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (sessionData_in);
}

void
Parser_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const enum Stream_SessionMessageType& messageType_in,
                             bool expedite_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);
  ACE_UNUSED_ARG (messageType_in);
  ACE_UNUSED_ARG (expedite_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

void
Parser_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Parser_Message& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

  // sanity check(s)
  ACE_ASSERT (message_in.isInitialized ());

  const Parser_MessageData_t& data_container_r = message_in.getR ();
  const struct Parser_MessageData& data_r = data_container_r.getR ();
  ACE_ASSERT (data_r.element.type == Bencoding_Element::BENCODING_TYPE_DICTIONARY);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s\n"),
              ACE_TEXT (Common_Parser_Bencoding_Tools::DictionaryToString (*data_r.element.dictionary).c_str ())));
}

void
Parser_EventHandler::notify (Stream_SessionId_t sessionId_in,
                             const Parser_SessionMessage& message_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::notify"));

  ACE_UNUSED_ARG (sessionId_in);

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
    case STREAM_SESSION_MESSAGE_STEP_DATA:
      break;
    case STREAM_SESSION_MESSAGE_STATISTIC:
    {
     // const Parser_SessionData_t& session_data_container_r =
     //   message_in.getR ();
     // struct Parser_SessionData& session_data_r =
     //   const_cast<struct Parser_SessionData&> (session_data_container_r.getR ());

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
Parser_EventHandler::end (Stream_SessionId_t sessionId_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_EventHandler::end"));

  ACE_UNUSED_ARG (sessionId_in);
}
