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

#include <limits>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "stream_iallocator.h"
#include "stream_macros.h"

// initialize statics
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
ACE_Atomic_Op<ACE_SYNCH_MUTEX, Stream_MessageId_t>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::currentId = 0;

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::Stream_MessageBase_T (Stream_SessionId_t sessionId_in,
                                                         MessageType messageType_in)
 : inherited (0,
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
 , id_ (++OWN_TYPE_T::currentId)
 , sessionId_ (sessionId_in)
 , type_ (messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // sanity check(s)
  ACE_ASSERT (sessionId_in);
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::Stream_MessageBase_T (unsigned int requestedSize_in)
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
 , id_ (++OWN_TYPE_T::currentId)
 , sessionId_ (0)
 , type_ (static_cast<MessageType> (STREAM_MESSAGE_DATA))
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

}

// *NOTE*: implicitly invoked by duplicate()
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::Stream_MessageBase_T (const Stream_MessageBase_T<AllocatorConfigurationType,
                                                                                    MessageType,
                                                                                    CommandType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of
                                                    // the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , id_ (message_in.id_)
 , sessionId_ (message_in.sessionId_)
 , type_ (message_in.type_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::Stream_MessageBase_T (Stream_SessionId_t sessionId_in,
                                                         ACE_Data_Block* dataBlock_in,
                                                         ACE_Allocator* messageAllocator_in,
                                                         bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // 'own' this data block reference
              0,                   // flags --> also "free" data block in dtor
              messageAllocator_in) // re-use the same allocator
 , id_ (0)
 , sessionId_ (sessionId_in)
 , type_ (static_cast<MessageType> (STREAM_MESSAGE_DATA))
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // sanity check(s)
  //ACE_ASSERT (sessionId_in);

  if (incrementMessageCounter_in)
    ++OWN_TYPE_T::currentId;
  id_ = OWN_TYPE_T::currentId.value ();

  // reset read/write pointers
  //inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::Stream_MessageBase_T (Stream_SessionId_t sessionId_in,
                                                         ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // re-use the same allocator
 , id_ (++OWN_TYPE_T::currentId)
 , sessionId_ (sessionId_in)
 , type_ (static_cast<MessageType> (STREAM_MESSAGE_DATA))
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // *WARNING*: need to finalize initialization through initialize()

  // sanity check(s)
  ACE_ASSERT (sessionId_in);

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::~Stream_MessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::~Stream_MessageBase_T"));

  // *NOTE*: will be called BEFORE this is passed back to the allocator

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("freeing message (ID: %d)\n"),
  //            id_));

  id_ = 0;
  sessionId_ = 0;
  type_ = static_cast<MessageType> (STREAM_MESSAGE_INVALID);

  // *WARNING*: cannot reset the message type (data block has already gone)
//  inherited::msg_type (ACE_Message_Block::MB_USER);
  // *IMPORTANT NOTE*: this is an ugly hack to support message allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::max ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::initialize (Stream_SessionId_t sessionId_in,
                                               ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (sessionId_in);

  if (dataBlock_in)
  { ACE_ASSERT (!inherited::data_block_);
    inherited::data_block (dataBlock_in);
  } // end IF
  ACE_ASSERT (inherited::data_block_);
  inherited::data_block_->msg_type (ACE_Message_Block::MB_DATA);
  sessionId_ = sessionId_in;
  type_ = static_cast<MessageType> (STREAM_MESSAGE_DATA);
  //msg_execution_time ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::defragment ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::defragment"));

  int result = -1;

  // sanity check(s)
  // *NOTE*: assuming stream processing is indeed single-threaded (CHECK !!!),
  //         then the reference count at this stage should be <=2: "this", and
  //         (most probably), the next, trailing "message head" (of course, it
  //         could be just "this")
  // *IMPORTANT NOTE*: this check is NOT enough. Also, there may be trailing
  //                   messages (in fact, that should be the norm), and/or
  //                   (almost any) number(s) of fragments referencing the same
  //                   buffer
  // *TODO*: to be clarified
  //ACE_ASSERT (inherited::reference_count () <= 2);

  ACE_ASSERT (inherited::data_block_);
  //if (inherited::total_length () > inherited::data_block_->capacity ());
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("not enough capacity to crunch message (had: %u, needed: %u, returning\n"),
  //              inherited::data_block_->capacity (),
  //              inherited::total_length ()));

  // step1: shift head message data down to the base and adust the pointers
  result = inherited::crunch ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", returning\n")));
    return;
  } // end IF

  // step2: consecutively copy data from any continuations into the preceding
  //        buffers
  unsigned int free_space = 0;
  unsigned int bytes_to_copy = 0;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Message_Block* message_block_2 = inherited::cont_;
  ACE_Message_Block* message_block_3 = this;
fill:
  free_space = message_block_3->space ();
  for (message_block_p = message_block_2;
       message_block_p;
       message_block_p = message_block_p->cont ())
  {
    bytes_to_copy =
        std::min (static_cast<unsigned int> (message_block_p->length ()),
                  free_space);
    result = message_block_3->copy (message_block_p->rd_ptr (),
                                    bytes_to_copy);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(%u): \"%m\", returning\n"),
                  bytes_to_copy));
      return;
    } // end IF
    message_block_p->rd_ptr (bytes_to_copy);
    free_space -= bytes_to_copy;

    if (!message_block_3->space ()) break;
  } // end FOR
  if (message_block_2)
  {
    if (!message_block_3->space ())
    {
      message_block_3 = message_block_2;
      result = message_block_3->crunch ();
      if (result == -1)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", returning\n")));
        return;
      } // end IF
    } // end IF
    message_block_2 = message_block_2->cont ();
    if (message_block_2) goto fill;
  } // end IF

  // step3: release any empty continuations
  message_block_p = inherited::cont_;
  message_block_2 = this;
  while (message_block_p)
  {
    message_block_3 = message_block_p->cont ();

    if (!message_block_p->length ())
    {
      message_block_2->cont (message_block_3);

      message_block_p->cont (NULL);
      message_block_p->release ();
    } // end IF
    else
    {
      message_block_2->cont (message_block_p);
      message_block_2 = message_block_p;
    } // end ELSE

    message_block_p = message_block_3;
  } // end WHILE
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
ACE_Message_Block*
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("message (id: %u, type: %d)\n"),
              id_,
              type_));
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::MessageTypeToString (enum Stream_MessageType type_in,
                                                        std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::MessageTypeToString"));

  // initialize return value(s)
  string_out = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (type_in)
  {
    case STREAM_MESSAGE_CONTROL:
      string_out = ACE_TEXT_ALWAYS_CHAR ("CONTROL"); break;
    case STREAM_MESSAGE_SESSION:
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION"); break;
    case STREAM_MESSAGE_DATA:
      string_out = ACE_TEXT_ALWAYS_CHAR ("DATA"); break;
    case STREAM_MESSAGE_OBJECT:
      string_out = ACE_TEXT_ALWAYS_CHAR ("OBJECT"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     MessageType,
                     CommandType>::resetMessageIdGenerator ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::resetMessageIdGenerator"));

  OWN_TYPE_T::currentId = 0;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("reset message ids\n")));
}

//////////////////////////////////////////

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (Stream_SessionId_t sessionId_in,
                                                         MessageType messageType_in)
 : inherited (sessionId_in,
              messageType_in)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

// *NOTE*: this is implicitly invoked by duplicate() as well
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (const Stream_MessageBase_2& message_in)
 : inherited (message_in)
 , isInitialized_ (message_in.isInitialized_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (Stream_SessionId_t sessionId_in,
                                                         ACE_Data_Block* dataBlock_in,
                                                         ACE_Allocator* messageAllocator_in,
                                                         bool incrementMessageCounter_in)
 : inherited (sessionId_in,               // session id
              dataBlock_in,               // use (don't own !) this data block
              messageAllocator_in,        // allocator
              incrementMessageCounter_in) // increment the message ID ?
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::~Stream_MessageBase_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::~Stream_MessageBase_2"));

  // *NOTE*: will be called just before (!) this is passed back to the allocator

  isInitialized_ = false;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
void
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::initialize (Stream_SessionId_t sessionId_in,
                                               ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

  // initialize base class
  inherited::initialize (sessionId_in,
                         dataBlock_in);

  isInitialized_ = true;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename HeaderType,
          typename CommandType>
HeaderType
Stream_MessageBase_2<AllocatorConfigurationType,
                     MessageType,
                     HeaderType,
                     CommandType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::get"));

  // initialize return value(s)
  HeaderType message_header;
  ACE_OS::memset (&message_header, 0, sizeof (HeaderType));

  // sanity check(s)
  ACE_ASSERT (inherited::size () >= sizeof (HeaderType)); // enough space ?
  if (inherited::total_length () < sizeof (HeaderType))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("not enough data (needed: %u, had: %u), aborting\n"),
                sizeof (HeaderType),
                inherited::total_length ()));
    return message_header;
  } // end IF

  if (inherited::length () >= sizeof (HeaderType))
  {
    ACE_OS::memcpy (&message_header,
                    inherited::rd_ptr (),
                    sizeof (HeaderType));
    return message_header;
  } // end IF

  // --> part of the header data lies in a continuation

  const ACE_Message_Block* source_message_block_p = this;
  size_t missing_data = sizeof (HeaderType) - inherited::length ();

  // copy first bit
  ACE_OS::memcpy (&message_header,
                  inherited::rd_ptr (),
                  inherited::length ());

  size_t amount = 0;
  unsigned char* destination_p =
    reinterpret_cast<unsigned char*> (&message_header) + inherited::length ();
  while (missing_data)
  {
    source_message_block_p = inherited::cont ();
    ACE_ASSERT (source_message_block_p);

    // skip over any "empty" continuations
    while (source_message_block_p->length () == 0)
      source_message_block_p = source_message_block_p->cont ();

    // copy some data over...
    amount =
      ((source_message_block_p->length () < missing_data) ? source_message_block_p->length ()
                                                          : missing_data);
    ACE_OS::memcpy (destination_p,
                    source_message_block_p->rd_ptr (),
                    amount);

    destination_p += amount;
    missing_data -= amount;
  } // end WHILE

  return message_header;
}
