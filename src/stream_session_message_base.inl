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

template <typename DataType>
Stream_SessionMessageBase_T<DataType>::Stream_SessionMessageBase_T (unsigned int sessionID_in,
                                                                    Stream_SessionMessageType_t messageType_in,
                                                                    DataType*& data_inout)
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
 , isInitialized_ (true)
 , messageType_ (messageType_in)
 , sessionID_ (sessionID_in)
 , data_ (data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set return value
  data_inout = NULL;
}

template <typename DataType>
Stream_SessionMessageBase_T<DataType>::Stream_SessionMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , isInitialized_ (false)
 , messageType_ (STREAM_SESSION_MAP)
 , sessionID_ (0)
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: this doesn't work, as we're assigned a (different) data block later...
  // --> do it in init(STREAM_SESSION_MAP)
//   msg_type ();

  // reset read/write pointers
  reset ();
}

template <typename DataType>
Stream_SessionMessageBase_T<DataType>::Stream_SessionMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                    ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) this data block
              0,                   // flags --> also "free" our data block upon destruction !
              messageAllocator_in) // re-use the same allocator
 , isInitialized_ (false)
 , messageType_ (STREAM_SESSION_MAP) // == Stream_MessageBase::MB_STREAM_SESSION
 , sessionID_ (0)
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: need to finalize initialization through init() !
  msg_type (STREAM_SESSION_MAP);

  // reset read/write pointers
  reset ();
}

template <typename DataType>
Stream_SessionMessageBase_T<DataType>::Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<DataType>& message_in)
 : inherited (message_in.data_block_->duplicate(), // make a "shallow" copy of the data block
              0,                                   // "own" the duplicate
              message_in.message_block_allocator_) // message allocator
 , isInitialized_ (message_in.isInitialized_)
 , messageType_ (message_in.messageType_)
 , sessionID_ (message_in.sessionID_)
 , data_ (message_in.data_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // increment reference counter
  // *TODO*: clean this up !
  if (data_)
    data_->increase ();

  // set read/write pointers
  rd_ptr (message_in.rd_ptr ());
  wr_ptr (message_in.wr_ptr ());
}

template <typename DataType>
Stream_SessionMessageBase_T<DataType>::~Stream_SessionMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::~Stream_SessionMessageBase_T"));

  sessionID_ = 0;
  messageType_ = STREAM_SESSION_MAP; // == Stream_MessageBase::MB_STREAM_SESSION

  // *TODO*: clean this up !
  if (data_)
    data_->decrease ();

  isInitialized_ = false;
}

template <typename DataType>
unsigned int
Stream_SessionMessageBase_T<DataType>::getID () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getID"));

  return sessionID_;
}

template <typename DataType>
Stream_SessionMessageType_t
Stream_SessionMessageBase_T<DataType>::getType () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getType"));

  return messageType_;
}

template <typename DataType>
const DataType*
Stream_SessionMessageBase_T<DataType>::getData () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getData"));

  return data_;
}

template <typename DataType>
ACE_Message_Block*
Stream_SessionMessageBase_T<DataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::duplicate"));

  Stream_SessionMessageBase_T<DataType>* nb = NULL;

  // create a new <Stream_SessionMessageBase_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_Data_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (!message_block_allocator_)
  {
    ACE_NEW_NORETURN (nb,
                      Stream_SessionMessageBase_T<DataType> (*this));
  } // end IF
  else
  {
    // *WARNING*:we tell the allocator to return a Stream_SessionMessageBase_T<DataType>
    // by passing a 0 as argument to malloc()...
    ACE_NEW_MALLOC_NORETURN (nb,
                             static_cast<Stream_SessionMessageBase_T<DataType>*> (message_block_allocator_->malloc (0)),
                             Stream_SessionMessageBase_T<DataType> (*this));
  }
  if (!nb)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate Stream_SessionMessageBase_T<DataType>, returning")));

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

template <typename DataType>
void
Stream_SessionMessageBase_T<DataType>::init(unsigned int sessionID_in,
                                            Stream_SessionMessageType_t messageType_in,
                                            DataType*& data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::init"));

  ACE_ASSERT (!isInitialized_);
  ACE_ASSERT (sessionID_ == 0);
  ACE_ASSERT (messageType_ == STREAM_SESSION_MAP);
  ACE_ASSERT (data_ == NULL);

  sessionID_ = sessionID_in;
  messageType_ = messageType_in;
  // *NOTE*: assumes lifetime responsibility for the handle !
  data_ = data_inout;

  // set return value
  data_inout = NULL;

  isInitialized_ = true;
}

template <typename DataType>
void
Stream_SessionMessageBase_T<DataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::dump_state"));

  std::string type_string;
  SessionMessageType2String (messageType_,
                             type_string);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("session (ID: %u) message type: \"%s\"\n"),
              sessionID_,
              ACE_TEXT (type_string.c_str ())));

  if (data_)
  {
    try
    {
      data_->dump_state ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in SessionConfiguration::dump_state(), continuing")));
    }
  } // end IF
}

template <typename DataType>
void
Stream_SessionMessageBase_T<DataType>::SessionMessageType2String (Stream_SessionMessageType_t messageType_in,
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
