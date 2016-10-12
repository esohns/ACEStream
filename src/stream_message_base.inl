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

#include <ace/Log_Msg.h>

#include "stream_macros.h"

// initialize statics
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::currentID = 0;

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::Stream_MessageBase_T ()
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
 , type_ (STREAM_MESSAGE_DATA)
 , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
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
 , type_ (STREAM_MESSAGE_DATA)
 , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

}

// *NOTE*: implicitly invoked by duplicate()
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::Stream_MessageBase_T (const Stream_MessageBase_T<AllocatorConfigurationType,
                                                                                    ControlMessageType,
                                                                                    SessionMessageType,
                                                                                    CommandType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of
                                                    // the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , type_ (message_in.type_)
 , messageID_ (message_in.messageID_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::Stream_MessageBase_T (ACE_Data_Block* dataBlock_in,
                                                         ACE_Allocator* messageAllocator_in,
                                                         bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) data block
              0,                   // flags --> also "free" data block in dtor
              messageAllocator_in) // re-use the same allocator
 , type_ (STREAM_MESSAGE_DATA)
// , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  if (incrementMessageCounter_in)
    ++currentID;
  messageID_ = currentID.value ();

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::Stream_MessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // re-use the same allocator
 , type_ (STREAM_MESSAGE_DATA)
 , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // *WARNING*: need to finalize initialization through initialize()

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::~Stream_MessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::~Stream_MessageBase_T"));

  // *NOTE*: will be called BEFORE this is passed back to the allocator

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("freeing message (ID: %d)...\n"),
  //            messageID_));

  type_ = STREAM_MESSAGE_INVALID;
  messageID_ = 0;

  // *WARNING*: cannot reset the message type (data block has already gone)
//  inherited::msg_type (ACE_Message_Block::MB_USER);
  // *IMPORTANT NOTE*: this is an ugly hack to support message allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::max ();
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::initialize (ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::initialize"));

  // set data block
  inherited::data_block (dataBlock_in);

  // set correct (?) message types
  inherited::msg_type (ACE_Message_Block::MB_DATA);
  type_ = STREAM_MESSAGE_DATA;

  // set scheduled execution time
  //msg_execution_time ();
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
CommandType
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::command () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::command"));

  return static_cast<CommandType> (ACE_Message_Block::MB_DATA);
}
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
int
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::crunch (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::crunch"));

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
  ACE_ASSERT (inherited::total_length () <=
              inherited::data_block_->capacity ());

  // step1: shift head message data down to the base and adust the pointers
  result = inherited::crunch ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::crunch(): \"%m\", aborting\n")));
    return false;
  } // end IF

  // step2: copy data from any continuations into the head message block buffer
  ACE_Message_Block* message_block_p = NULL;
  for (message_block_p = inherited::cont_;
       message_block_p;
       message_block_p = message_block_p->cont ())
  {
    result = inherited::copy (message_block_p->rd_ptr (),
                              message_block_p->length ());
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
      return false;
    } // end IF
  } // end FOR

  // step3: release any continuations
  ACE_Message_Block* message_block_2 = NULL;
  message_block_p = inherited::cont_;
  while (message_block_p)
  {
    message_block_2 = message_block_p->cont ();

    message_block_p->cont (NULL);
    message_block_p->release ();

    message_block_p = message_block_2;
  } // end WHILE
  inherited::cont_ = NULL;

  return true;
}
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
unsigned int
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::getID () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::getID"));

  return messageID_;
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
std::string
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::CommandType2String (CommandType type_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::CommandType2String"));

  ACE_UNUSED_ARG (type_in);

  return ACE_TEXT_ALWAYS_CHAR ("");
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
ACE_Message_Block*
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
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
                  ACE_TEXT ("failed to allocate Stream_MessageBase_T: \"%m\", aborting\n")));
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
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("message (ID: %u)...\n"),
              getID ()));
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::MessageType2String (Stream_MessageType type_in,
                                                       std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::MessageType2String"));

  // initialize return value(s)
  string_out = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (type_in)
  {
    case STREAM_MESSAGE_CONTROL:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("CONTROL");
      break;
    }
    case STREAM_MESSAGE_SESSION:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION");
      break;
    }
    case STREAM_MESSAGE_DATA:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("DATA");
      break;
    }
    case STREAM_MESSAGE_OBJECT:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("OBJECT");
      break;
    }
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
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     CommandType>::resetMessageIDGenerator ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::resetIDGenerator"));

  currentID = 1;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("reset message IDs...\n")));
}

//////////////////////////////////////////

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

// *NOTE*: this is implicitly invoked by duplicate() as well
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (const Stream_MessageBase_2& message_in)
 : inherited (message_in)
 , isInitialized_ (message_in.isInitialized_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (ACE_Data_Block* dataBlock_in,
                                                         ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in, // allocator
              true)                // increment the message ID ?
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     HeaderType,
                     CommandType>::~Stream_MessageBase_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::~Stream_MessageBase_2"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator

  isInitialized_ = false;
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename HeaderType,
          typename CommandType>
void
Stream_MessageBase_2<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
                     HeaderType,
                     CommandType>::initialize (ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::initialize"));

  // sanity check(s)
  ACE_ASSERT (!isInitialized_);

  // initialize base class
  inherited::initialize (dataBlock_in);

  isInitialized_ = true;
}

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename HeaderType,
          typename CommandType>
HeaderType
Stream_MessageBase_2<AllocatorConfigurationType,
                     ControlMessageType,
                     SessionMessageType,
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

  // --> part of the header data lies in the continuation

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

    // skip over any "empty" continuations...
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

// void
// Stream_MessageBase_2::adjustDataOffset (const Stream_MessageHeader_t& headerType_in)
// {
//STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::adjustDataOffset"));
//
//   unsigned long dataOffset = 0;
//
//   // create header
//   switch (headerType_in)
//   {
//     case RPG_Stream_Protocol_Layer::ETHERNET:
//     {
//       // *NOTE*: Ethernet headers are 14 bytes long...
//       dataOffset = ETH_HLEN;
//
//       break;
//     }
//     case RPG_Stream_Protocol_Layer::FDDI_LLC_SNAP:
//     {
//       // *NOTE*:
//       // - FDDI LLC headers are 13 bytes long...
//       // - FDDI SNAP headers are 8 bytes long...
//       dataOffset = FDDI_K_SNAP_HLEN;
//
//       break;
//     }
//     case RPG_Stream_Protocol_Layer::IPv4:
//     {
//       // *NOTE*: IPv4 header field "Header Length" gives the size of the
//       // IP header in 32 bit words...
//       // *NOTE*: use our current offset...
//       dataOffset = (reinterpret_cast<iphdr*> (//                                          rd_ptr())->ihl * 4);
//
//       break;
//     }
//     case RPG_Stream_Protocol_Layer::TCP:
//     {
//       // *NOTE*: TCP header field "Data Offset" gives the size of the
//       // TCP header in 32 bit words...
//       // *NOTE*: use our current offset...
//       dataOffset = (reinterpret_cast<tcphdr*> (//                                          rd_ptr())->doff * 4);
//
//       break;
//     }
//     case RPG_Stream_Protocol_Layer::UDP:
//     {
//       // *NOTE*: UDP headers are 8 bytes long...
//       dataOffset = 8;
//
//       break;
//     }
// //     case RPG_Stream_Protocol_Layer::ASTERIX_offset:
// //     {
// //       // *NOTE*: ASTERIX "resilience" headers are 4 bytes long...
// //       dataOffset = FLB_RPS_ASTERIX_RESILIENCE_BYTES;
// //
// //       break;
// //     }
// //     case RPG_Stream_Protocol_Layer::ASTERIX:
// //     {
// //       // *NOTE*: ASTERIX headers are 3 bytes long...
// //       dataOffset = FLB_RPS_ASTERIX_HEADER_SIZE;
// //
// //       break;
// //     }
//     default:
//     {
//       std::string type_string;
//       RPG_Stream_Protocol_Layer::ProtocolLayer2String(headerType_in,
//                                                           type_string);
//       ACE_DEBUG((LM_ERROR,
//                  ACE_TEXT("message (ID: %u) header (type: \"%s\") is currently unsupported, continuing\n"),
//                  getID(),
//                  type_string.c_str()));
//
//       break;
//     }
//   } // end SWITCH
//
//   // advance rd_ptr() to the start of the data (or to the next header in the stack)...
//   rd_ptr(dataOffset);
// }
