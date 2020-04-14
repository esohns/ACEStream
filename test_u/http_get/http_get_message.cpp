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

#include "stream_dec_common.h"

//#include "ace/Synch.h"
#include "http_get_message.h"

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"

#include "stream_macros.h"

#include "http_defines.h"

#include "http_get_stream_common.h"

HTTPGet_MessageDataContainer::HTTPGet_MessageDataContainer ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_MessageDataContainer::HTTPGet_MessageDataContainer"));

}

HTTPGet_MessageDataContainer::HTTPGet_MessageDataContainer (struct HTTPGet_MessageData*& messageData_in,
                                                            bool delete_in)
 : inherited (messageData_in,
              delete_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_MessageDataContainer::HTTPGet_MessageDataContainer"));

}

void
HTTPGet_MessageDataContainer::setPR (struct HTTP_Record*& record_inout)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_MessageDataContainer::setPR"));

  struct HTTPGet_MessageData* data_p = NULL;
  ACE_NEW_NORETURN (data_p,
                    struct HTTPGet_MessageData ());
  if (!data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Test_I_MessageData: \"%m\", returning\n")));
    return;
  } // end IF
  data_p->HTTPRecord = record_inout;
  record_inout = NULL;

  inherited::setPR (data_p);
}

//////////////////////////////////////////

HTTPGet_Message::HTTPGet_Message (unsigned int size_in)
 : inherited (size_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Message::HTTPGet_Message"));

}

HTTPGet_Message::HTTPGet_Message (const HTTPGet_Message& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Message::HTTPGet_Message"));

}

HTTPGet_Message::HTTPGet_Message (Stream_SessionId_t sessionId_in,
                                  ACE_Data_Block* dataBlock_in,
                                  ACE_Allocator* messageAllocator_in,
                                  bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,               // use (don't own (!) memory of-) this data block
              messageAllocator_in,        // message block allocator
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Message::HTTPGet_Message"));

}

HTTPGet_Message::HTTPGet_Message (Stream_SessionId_t sessionId_in,
                                  ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message block allocator
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Message::HTTPGet_Message"));

}

ACE_Message_Block*
HTTPGet_Message::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Message::duplicate"));

  HTTPGet_Message* message_p = NULL;

  // create a new HTTPGet_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      HTTPGet_Message (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc doesn't matter, as this will be
    //         a shallow copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<HTTPGet_Message*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                               '\0')),
                             HTTPGet_Message (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate HTTPGet_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to HTTPGet_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

HTTP_Method_t
HTTPGet_Message::command () const
{
  STREAM_TRACE (ACE_TEXT ("HTTPGet_Message::command"));

  // sanity check(s)
  if (!inherited::isInitialized_)
    return HTTP_Codes::HTTP_METHOD_INVALID;
  ACE_ASSERT (inherited::data_);

  const struct HTTPGet_MessageData& data_r = inherited::data_->getR ();

  return (data_r.HTTPRecord ? data_r.HTTPRecord->method
                            : HTTP_Codes::HTTP_METHOD_INVALID);
}
