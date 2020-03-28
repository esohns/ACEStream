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

#include "parser_session_message.h"

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"
#include "ace/Message_Block.h"

#include "stream_macros.h"

Parser_SessionMessage::Parser_SessionMessage (Stream_SessionId_t sessionId_in,
                                              enum Stream_SessionMessageType messageType_in,
                                              Parser_SessionData_t*& sessionData_in,
                                              struct Stream_UserData* userData_in)
 : inherited (sessionId_in,
              messageType_in,
              sessionData_in,
              userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_SessionMessage::Parser_SessionMessage"));

}

Parser_SessionMessage::Parser_SessionMessage (const Parser_SessionMessage& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Parser_SessionMessage::Parser_SessionMessage"));

}

Parser_SessionMessage::Parser_SessionMessage (Stream_SessionId_t sessionId_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Parser_SessionMessage::Parser_SessionMessage"));

}

Parser_SessionMessage::Parser_SessionMessage (Stream_SessionId_t sessionId_in,
                                              ACE_Data_Block* dataBlock_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Parser_SessionMessage::Parser_SessionMessage"));

}

ACE_Message_Block*
Parser_SessionMessage::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Parser_SessionMessage::duplicate"));

  Parser_SessionMessage* message_p = NULL;

  // create a new <Parser_SessionMessage> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_RETURN (message_p,
                    Parser_SessionMessage (*this),
                    NULL);

  // *WARNING*: the allocator returns a Parser_SessionMessageBase<ConfigurationType>
  //            when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (message_p,
                         static_cast<Parser_SessionMessage*> (inherited::message_block_allocator_->malloc (0)),
                         Parser_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();

    // when things go wrong, release all resources and return
    if (message_p->cont_ == 0)
    {
      message_p->release ();
      message_p = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
