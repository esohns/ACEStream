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
                                              const SessionMessageType& messageType_in,
                                              Stream_SessionConfiguration*& sessionConfiguration_inout)
 : inherited (0,                                     // size
              Stream_MessageBase::MB_STREAM_SESSION, // type
              NULL,                                  // continuation
              NULL,                                  // data
              NULL,                                  // buffer allocator
              NULL,                                  // locking strategy
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY,    // priority
              ACE_Time_Value::zero,                  // execution time
              ACE_Time_Value::max_time,              // deadline time
              NULL,                                  // data block allocator
              NULL)                                  // message block allocator
 , sessionID_ (sessionID_in)
 , messageType_ (messageType_in)
 , configuration_ (sessionConfiguration_inout)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

  // set return value
  sessionConfiguration_inout = NULL;
}

Stream_SessionMessage::Stream_SessionMessage (const Stream_SessionMessage& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , sessionID_ (message_in.sessionID_)
 , messageType_ (message_in.messageType_)
 , configuration_ (message_in.configuration_)
 , isInitialized_ (message_in.isInitialized_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

  // increment reference counter
  if (configuration_)
    configuration_->increase ();

  // set read/write pointers
  rd_ptr (message_in.rd_ptr ());
  wr_ptr (message_in.wr_ptr ());
}

Stream_SessionMessage::Stream_SessionMessage (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , sessionID_ (0)
 , messageType_ (MB_BEGIN_STREAM_SESSION_MAP) // == RPG_Stream_MessageBase::MB_STREAM_SESSION
 , configuration_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

  // set correct message type
  // *WARNING*: this doesn't work, as we're assigned a (different) data block later...
  // --> do it in init()
//   msg_type (Stream_MessageBase::MB_STREAM_SESSION);

  // reset read/write pointers
  reset ();
}

Stream_SessionMessage::Stream_SessionMessage (ACE_Data_Block* dataBlock_in,
                                              ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              0,                   // flags --> also "free" our data block upon destruction !
              messageAllocator_in) // re-use the same allocator
 , sessionID_ (0)
 , messageType_ (MB_BEGIN_STREAM_SESSION_MAP) // == RPG_Stream_MessageBase::MB_STREAM_SESSION
 , configuration_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::Stream_SessionMessage"));

  // set correct message type
  // *WARNING*: need to finalize initialization through init() !
  msg_type (Stream_MessageBase::MB_STREAM_SESSION);

  // reset read/write pointers
  reset ();
}

Stream_SessionMessage::~Stream_SessionMessage ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::~Stream_SessionMessage"));

  sessionID_ = 0;
  messageType_ = MB_BEGIN_STREAM_SESSION_MAP; // == RPG_Stream_MessageBase::MB_STREAM_SESSION
  // clean up
  if (configuration_)
  {
    // decrease reference counter...
    configuration_->decrease ();
    configuration_ = NULL;
  } // end IF

  isInitialized_ = false;
}

void
Stream_SessionMessage::init (unsigned int sessionID_in,
                             const Stream_SessionMessageType& messageType_in,
                             Stream_SessionConfiguration*& configuration_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::init"));

  ACE_ASSERT (!isInitialized_);
  ACE_ASSERT (sessionID_ == 0);
  // *WARNING*: gcc warns about this, but that's OK...
  ACE_ASSERT (messageType_ == MB_BEGIN_STREAM_SESSION_MAP); // == RPG_Stream_MessageBase::MB_STREAM_SESSION
  ACE_ASSERT (configuration_ == NULL);

  sessionID_ = sessionID_in;
  messageType_ = messageType_in;
  configuration_ = configuration_inout;

  // bye bye... we take on the responsibility for config_inout
  configuration_inout = NULL;

  // OK !
  isInitialized_ = true;
}

unsigned int
Stream_SessionMessage::getID () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::getID"));

  return sessionID_;
}

Stream_SessionMessage::SessionMessageType
Stream_SessionMessage::getType () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::getType"));

  return messageType_;
}

const Stream_SessionConfiguration*
const Stream_SessionMessage::getConfiguration () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::getConfiguration"));

  return configuration_;
}

void
Stream_SessionMessage::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::dump_state"));

  std::string type_string;
  SessionMessageType2String (messageType_,
                             type_string);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("session (ID: %u) message type: \"%s\"\n"),
              sessionID_,
              ACE_TEXT (type_string.c_str ())));

  if (configuration_)
  {
    try
    {
      configuration_->dump_state ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in RPG_Stream_SessionConfig::dump_state(), continuing")));
    }
  } // end IF
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
  if (message_block_allocator_ == NULL)
  {
    ACE_NEW_RETURN (nb,
                    Stream_SessionMessage (*this),
                    NULL);
  } // end IF

  // *WARNING*: the allocator returns a Stream_SessionMessageBase<ConfigType>
  // when passing 0 as argument to malloc()...
  ACE_NEW_MALLOC_RETURN (nb,
                         static_cast<Stream_SessionMessage*> (message_block_allocator_->malloc (0)),
                         Stream_SessionMessage (*this),
                         NULL);

  // increment the reference counts of all the continuation messages
  if (cont_)
  {
    nb->cont_ = cont_->duplicate ();

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

void
Stream_SessionMessage::SessionMessageType2String (SessionMessageType messageType_in,
                                                  std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessage::SessionMessageType2String"));

  // init return value(s)
  string_out = ACE_TEXT ("INVALID_TYPE");

  switch (messageType_in)
  {
    case MB_STREAM_SESSION_BEGIN:
    {
      string_out = ACE_TEXT ("MB_STREAM_SESSION_BEGIN");

      break;
    }
    case MB_STREAM_SESSION_STEP:
    {
      string_out = ACE_TEXT ("MB_STREAM_SESSION_STEP");

      break;
    }
    case MB_STREAM_SESSION_END:
    {
      string_out = ACE_TEXT ("MB_STREAM_SESSION_END");

      break;
    }
    case MB_STREAM_SESSION_STATISTICS:
    {
      string_out = ACE_TEXT ("MB_STREAM_SESSION_STATISTICS");

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type: \"%u\", aborting\n"),
                  messageType_in));

      break;
    }
  } // end SWITCH
}
