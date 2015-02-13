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

#include "stream_message_base.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

// init statics
ACE_Atomic_Op<ACE_Thread_Mutex, unsigned int> Stream_MessageBase::currentID = 0;

Stream_MessageBase::Stream_MessageBase (const Stream_MessageBase& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
// , messageID_ (++currentID) // *WARNING*: DON'T change (it's already been set !)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  // set correct message type
//   msg_type (MB_STREAM_OBJ);

  // set read/write pointers
  rd_ptr (message_in.rd_ptr ());
  wr_ptr (message_in.wr_ptr ());
}

Stream_MessageBase::Stream_MessageBase (ACE_Data_Block* dataBlock_in,
                                        ACE_Allocator* messageAllocator_in,
                                        bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              0,                   // flags --> also "free" our data block upon destruction !
              messageAllocator_in) // re-use the same allocator
// , messageID_ (++currentID.value ())
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  if (incrementMessageCounter_in)
    currentID++;
  messageID_ = currentID.value ();

  // set correct message type
  msg_type (MESSAGE_DATA);

  // reset read/write pointers
  reset ();
}

Stream_MessageBase::Stream_MessageBase (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // re-use the same allocator
 , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::Stream_MessageBase"));

  // *WARNING*: this wouldn't work, as we're assigned a different data block later...
  // --> do it in init()
//   // set correct message type
//   msg_type (MB_STREAM_DATA);

  // reset read/write pointers
  reset ();
}

Stream_MessageBase::~Stream_MessageBase ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::~Stream_MessageBase"));

  // *NOTE*: will be called BEFORE we're passed back to the allocator

//   ACE_DEBUG((LM_DEBUG,
//              ACE_TEXT ("freeing message (ID: %d)...\n"),
//              messageID_));
}

void
Stream_MessageBase::init (ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase::init"));

  // set data block
  data_block (dataBlock_in);

  // set correct (?) message type
  msg_type (MESSAGE_DATA);

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

  Stream_MessageBase* new_message = NULL;

  // create a new Net_Message that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block.

  // if there is no allocator, use the standard new and delete calls.
  if (message_block_allocator_ == NULL)
  {
    ACE_NEW_RETURN (new_message,
                    Stream_MessageBase (*this),
                    NULL);
  } // end IF
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    // a "shallow" copy which just references our data block...
    ACE_NEW_MALLOC_RETURN (new_message,
                           static_cast<Stream_MessageBase*> (message_block_allocator_->malloc (inherited::capacity ())),
                           Stream_MessageBase (*this),
                           NULL);
  } // end ELSE

  // increment the reference counts of all the continuation messages
  if (cont_)
  {
    new_message->cont_ = cont_->duplicate ();

    // If things go wrong, release all of our resources and return
    if (new_message->cont_ == NULL)
    {
      new_message->release ();
      new_message = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return new_message;
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
    case MESSAGE_SESSION:
    {
      typeString_out = ACE_TEXT ("MESSAGE_SESSION");

      break;
    }
    case MESSAGE_DATA:
    {
      typeString_out = ACE_TEXT ("MESSAGE_DATA");

      break;
    }
    case MESSAGE_OBJECT:
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
