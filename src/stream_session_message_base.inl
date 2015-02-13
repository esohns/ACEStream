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

#include "ace/Malloc_Base.h"
#include "ace/Log_Msg.h"

#include "stream_macros.h"

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T (unsigned int sessionID_in,
                                                                             Stream_SessionMessageType_t messageType_in,
                                                                             ConfigurationType*& configuration_inout)
 : inherited (0,                                  // size
              STREAM_SESSION_MAP,                 // type
              NULL,                               // continuation
              NULL,                               // data
              NULL,                               // buffer allocator
              NULL,                               // locking strategy
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
              ACE_Time_Value::zero,               // execution time
              ACE_Time_Value::max_time,           // deadline time
              NULL,                               // data block allocator
              NULL)                               // message block allocator
 , configuration_ (configuration_inout)
 , sessionID_ (sessionID_in)
 , messageType_ (messageType_in)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set return value
  configuration_inout = NULL;
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , configuration_ (NULL)
 , sessionID_ (0)
 , messageType_ (STREAM_SESSION_MAP)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: this doesn't work, as we're assigned a (different) data block later...
  // --> do it in init()
//   msg_type (RPG_Stream_MessageBase::MB_STREAM_SESSION);

  // reset read/write pointers
  reset ();
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                             ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              0,                   // flags --> also "free" our data block upon destruction !
              messageAllocator_in) // re-use the same allocator
 , configuration_ (NULL)
 , sessionID_ (0)
 , messageType_ (STREAM_SESSION_MAP) // == Stream_MessageBase::MB_STREAM_SESSION
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: need to finalize initialization through init() !
  msg_type (STREAM_SESSION_MAP);

  // reset read/write pointers
  reset ();
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<ConfigurationType>& message_in)
 : inherited (message_in.data_block_->duplicate(), // make a "shallow" copy of the data block
              0,                                   // "own" the duplicate
              message_in.message_block_allocator_) // message allocator
 , configuration_ (message_in.configuration_)
 , sessionID_ (message_in.sessionID_)
 , messageType_ (message_in.messageType_)
 , isInitialized_ (message_in.isInitialized_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // increment reference counter
  if (configuration_)
    configuration_->increase ();

  // set read/write pointers
  rd_ptr (message_in.rd_ptr ());
  wr_ptr (message_in.wr_ptr ());
}

template <typename ConfigurationType>
Stream_SessionMessageBase_T<ConfigurationType>::~Stream_SessionMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::~Stream_SessionMessageBase_T"));

  sessionID_ = 0;
  messageType_ = STREAM_SESSION_MAP; // == Stream_MessageBase::MB_STREAM_SESSION

  // clean up
  if (configuration_)
    configuration_->decrease ();

  isInitialized_ = false;
}

template <typename ConfigurationType>
unsigned int
Stream_SessionMessageBase_T<ConfigurationType>::getID () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getID"));

  return sessionID_;
}

template <typename ConfigurationType>
Stream_SessionMessageType_t
Stream_SessionMessageBase_T<ConfigurationType>::getType () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getType"));

  return messageType_;
}

template <typename ConfigurationType>
const ConfigurationType* const
Stream_SessionMessageBase_T<ConfigurationType>::getConfiguration () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getConfiguration"));

  return configuration_;
}

template <typename ConfigurationType>
ACE_Message_Block*
Stream_SessionMessageBase_T<ConfigurationType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::duplicate"));

  Stream_SessionMessageBase_T<ConfigurationType>* nb = NULL;

  // create a new <Stream_SessionMessageBase_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (!message_block_allocator_)
  {
    ACE_NEW_NORETURN (nb,
                      Stream_SessionMessageBase_T<ConfigurationType> (*this));
  } // end IF
  else
  {
    // *WARNING*:we tell the allocator to return a Stream_SessionMessageBase_T<ConfigurationType>
    // by passing a 0 as argument to malloc()...
    ACE_NEW_MALLOC_NORETURN (nb,
                             static_cast<Stream_SessionMessageBase_T<ConfigurationType>*> (message_block_allocator_->malloc (0)),
                             Stream_SessionMessageBase_T<ConfigurationType> (*this));
  }
  if (!nb)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Stream_SessionMessageBase_T<ConfigurationType>, returning")));

    return NULL;
  }

  // increment the reference counts of all the continuation messages
  if (cont_)
  {
    nb->cont_ = cont_->duplicate ();

    // If things go wrong, release all of our resources and return
    if (nb->cont_ == 0)
    {
      nb->release ();
      nb = NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return nb;
}

template <typename ConfigurationType>
void
Stream_SessionMessageBase_T<ConfigurationType>::init(unsigned int sessionID_in,
                                                     Stream_SessionMessageType_t messageType_in,
                                                     ConfigurationType*& configuration_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::init"));

  ACE_ASSERT (!isInitialized_);
  ACE_ASSERT (sessionID_ == 0);
  ACE_ASSERT (messageType_ == STREAM_SESSION_MAP);
  ACE_ASSERT (configuration_ == NULL);

  sessionID_ = sessionID_in;
  messageType_ = messageType_in;
  configuration_ = configuration_inout;

  // bye bye... we take on the responsibility for config_inout
  configuration_inout = NULL;

  // OK !
  isInitialized_ = true;
}

template <typename ConfigurationType>
void
Stream_SessionMessageBase_T<ConfigurationType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::dump_state"));

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
                  ACE_TEXT ("caught exception in SessionConfiguration::dump_state(), continuing")));
    }
  } // end IF
}

template <typename ConfigurationType>
void
Stream_SessionMessageBase_T<ConfigurationType>::SessionMessageType2String (Stream_SessionMessageType_t messageType_in,
                                                                           std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::SessionMessageType2String"));

  // init return value(s)
  string_out = ACE_TEXT ("INVALID_TYPE");

  switch (messageType_in)
  {
    case SESSION_BEGIN:
    {
      string_out = ACE_TEXT ("SESSION_BEGIN");

      break;
    }
    case SESSION_STEP:
    {
      string_out = ACE_TEXT ("SESSION_STEP");

      break;
    }
    case SESSION_END:
    {
      string_out = ACE_TEXT ("SESSION_END");

      break;
    }
    case SESSION_STATISTICS:
    {
      string_out = ACE_TEXT ("SESSION_STATISTICS");

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
