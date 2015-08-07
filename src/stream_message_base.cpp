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

#include "stream_iallocator.h"
#include "stream_message_base.h"

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"

#include "stream_macros.h"

// initialize statics
ACE_Atomic_Op<ACE_Thread_Mutex, unsigned int> Stream_MessageBase::currentID = 0;

Stream_MessageBase::Stream_MessageBase (unsigned int requestedSize_in)
 : inherited (requestedSize_in,
              ACE_Message_Block::MB_DATA,
              NULL,
              NULL,
              NULL,
              NULL,
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,
              ACE_Time_Value::zero,
              ACE_Time_Value::max_time,
              NULL,
              NULL)
 , messageID_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  ++currentID;
  messageID_ = currentID.value ();
}

// *NOTE*: this is implicitly invoked by duplicate() as well...
Stream_MessageBase::Stream_MessageBase (const Stream_MessageBase& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of
                                                    // the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , messageID_ (message_in.messageID_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  // set read/write pointers
  rd_ptr (message_in.rd_ptr ());
  wr_ptr (message_in.wr_ptr ());
}

Stream_MessageBase::Stream_MessageBase (ACE_Data_Block* dataBlock_in,
                                        ACE_Allocator* messageAllocator_in,
                                        bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) data block
              0,                   // flags --> also "free" data block in dtor
              messageAllocator_in) // re-use the same allocator
// , messageID_ (++currentID.value ())
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  if (incrementMessageCounter_in)
    ++currentID;
  messageID_ = currentID.value ();

  // set correct message type
  inherited::msg_type (STREAM_MESSAGE_DATA);

  // reset read/write pointers
  reset ();
}

Stream_MessageBase::Stream_MessageBase (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // re-use the same allocator
 , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  // reset read/write pointers
  reset ();
}

Stream_MessageBase::~Stream_MessageBase ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::~Stream_MessageBase"));

  // *NOTE*: will be called BEFORE this is passed back to the allocator

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT ("freeing message (ID: %d)...\n"),
//              messageID_));
}

void
Stream_MessageBase::initialize (ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::initialize"));

  // set data block
  data_block (dataBlock_in);

  // set correct (?) message type
  inherited::msg_type (STREAM_MESSAGE_DATA);

  // set scheduled execution time
  //msg_execution_time ();
}

unsigned int
Stream_MessageBase::getID () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::getID"));

  return messageID_;
}

//int
//Stream_MessageBase::getCommand () const
//{
//STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::getCommand"));

//  return inherited::msg_type ();
//}

ACE_Message_Block*
Stream_MessageBase::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::duplicate"));

  Stream_MessageBase* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      Stream_MessageBase (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    // a "shallow" copy which just references our data block...
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<Stream_MessageBase*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                            '\0')),
                             Stream_MessageBase (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

void
Stream_MessageBase::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("message (ID: %u)...\n"),
              getID ()));
}

void
Stream_MessageBase::resetMessageIDGenerator()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::resetIDGenerator"));

  currentID = 1;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("reset message IDs...\n")));
}

void
Stream_MessageBase::MessageType2String (ACE_Message_Type type_in,
                                        std::string& typeString_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::MessageType2String"));

  // init return value(s)
  typeString_out = ACE_TEXT ("INVALID_TYPE");

  switch (type_in)
  {
    case STREAM_MESSAGE_SESSION:
    {
      typeString_out = ACE_TEXT ("MESSAGE_SESSION");

      break;
    }
    case STREAM_MESSAGE_DATA:
    {
      typeString_out = ACE_TEXT ("MESSAGE_DATA");

      break;
    }
    case STREAM_MESSAGE_OBJECT:
    {
      typeString_out = ACE_TEXT ("MESSAGE_OBJECT");

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (type: \"%d\"), aborting\n"),
                  type_in));

      break;
    }
  } // end SWITCH
}
