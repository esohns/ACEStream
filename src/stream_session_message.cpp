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

#include "stream_session_message.h"

#include "ace/Malloc_Base.h"

#include "stream_macros.h"
#include "stream_message_base.h"
#include "stream_session_configuration.h"

Stream_SessionMessage::Stream_SessionMessage (unsigned int sessionID_in,
                                              Stream_SessionMessageType_t messageType_in,
                                              Stream_SessionConfiguration*& sessionConfiguration_inout)
 : inherited (sessionID_in,
              messageType_in,
              sessionConfiguration_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

}

Stream_SessionMessage::Stream_SessionMessage (const Stream_SessionMessage& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

}

Stream_SessionMessage::Stream_SessionMessage (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

}

Stream_SessionMessage::Stream_SessionMessage (ACE_Data_Block* dataBlock_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              messageAllocator_in) // re-use the same allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

}

Stream_SessionMessage::~Stream_SessionMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::~Stream_SessionMessage"));

}

ACE_Message_Block*
Stream_SessionMessage::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::duplicate"));

  Stream_SessionMessage* nb = NULL;

  // create a new <Stream_SessionMessage> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
  {
    ACE_NEW_RETURN (nb,
                    Stream_SessionMessage (*this),
                    NULL);
  } // end IF

  // *WARNING*: the allocator returns a Stream_SessionMessageBase<ConfigType>
  // when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (nb,
                         static_cast<Stream_SessionMessage*> (inherited::message_block_allocator_->malloc (0)),
                         Stream_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (inherited::cont_)
  {
    nb->cont_ = inherited::cont_->duplicate ();

    // when things go wrong, release all resources and return
    if (nb->cont_ == 0)
    {
      nb->release ();
      nb = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return nb;
}
