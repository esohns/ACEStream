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

#include "stream_macros.h"

// initialize statics
template <typename AllocatorConfigurationType,
          typename CommandType>
ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long>
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::currentID = 0;

template <typename AllocatorConfigurationType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::Stream_MessageBase_T ()
 : inherited (0,
              STREAM_MESSAGE_OBJECT,
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
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  ++currentID;
  messageID_ = currentID.value ();
}

template <typename AllocatorConfigurationType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
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
 , messageID_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  ++currentID;
  messageID_ = currentID.value ();
}

// *NOTE*: implicitly invoked by duplicate()
template <typename AllocatorConfigurationType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::Stream_MessageBase_T (const Stream_MessageBase_T<AllocatorConfigurationType,
                                                                                    CommandType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of
                                                    // the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , messageID_ (message_in.messageID_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename AllocatorConfigurationType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::Stream_MessageBase_T (ACE_Data_Block* dataBlock_in,
                                                         ACE_Allocator* messageAllocator_in,
                                                         bool incrementMessageCounter_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) data block
              0,                   // flags --> also "free" data block in dtor
              messageAllocator_in) // re-use the same allocator
// , messageID_ (++currentID.value ())
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  if (incrementMessageCounter_in)
    ++currentID;
  messageID_ = currentID.value ();

  // set correct message type
  inherited::msg_type (STREAM_MESSAGE_DATA);

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::Stream_MessageBase_T (ACE_Allocator* messageAllocator_in)
  : inherited (messageAllocator_in) // re-use the same allocator
  , messageID_ (++currentID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::Stream_MessageBase_T"));

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename CommandType>
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::~Stream_MessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::~Stream_MessageBase_T"));

  // *NOTE*: will be called BEFORE this is passed back to the allocator

  //   ACE_DEBUG((LM_DEBUG,
  //              ACE_TEXT ("freeing message (ID: %d)...\n"),
  //              messageID_));

  // *IMPORTANT NOTE*: this is an ugly hack to enable some allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::max ();
}

template <typename AllocatorConfigurationType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::initialize (ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::initialize"));

  // set data block
  inherited::data_block (dataBlock_in);

  // set correct (?) message type
  inherited::msg_type (STREAM_MESSAGE_DATA);

  // set scheduled execution time
  //msg_execution_time ();
}

template <typename AllocatorConfigurationType,
          typename CommandType>
unsigned int
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::getID () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::getID"));

  return messageID_;
}

// partial specialization
template <typename AllocatorConfigurationType,
          typename CommandType>
CommandType
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::command () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::command"));

  return static_cast<CommandType> (ACE_Message_Block::MB_DATA);
}
template <typename AllocatorConfigurationType,
          typename CommandType>
std::string
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::CommandType2String (CommandType type_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::CommandType2String"));

  ACE_UNUSED_ARG (type_in);

  return ACE_TEXT_ALWAYS_CHAR ("");
}

template <typename AllocatorConfigurationType,
          typename CommandType>
ACE_Message_Block*
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block...
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

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

template <typename AllocatorConfigurationType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("message (ID: %u)...\n"),
              getID ()));
}

template <typename AllocatorConfigurationType,
          typename CommandType>
void
Stream_MessageBase_T<AllocatorConfigurationType,
                     CommandType>::resetMessageIDGenerator ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_T::resetIDGenerator"));

  currentID = 1;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("reset message IDs...\n")));
}

/////////////////////////////////////////

template <typename AllocatorConfigurationType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

// *NOTE*: this is implicitly invoked by duplicate() as well...
template <typename AllocatorConfigurationType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (const Stream_MessageBase_2& message_in)
 : inherited (message_in)
 , isInitialized_ (message_in.isInitialized_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     HeaderType,
                     CommandType>::Stream_MessageBase_2 (ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own !) this data block
              messageAllocator_in, // use this when destruction is imminent...
              true)                // increment the message ID ?
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::Stream_MessageBase_2"));

}

template <typename AllocatorConfigurationType,
          typename HeaderType,
          typename CommandType>
Stream_MessageBase_2<AllocatorConfigurationType,
                     HeaderType,
                     CommandType>::~Stream_MessageBase_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageBase_2::~Stream_MessageBase_2"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator

  // *IMPORTANT NOTE*: this is an ugly hack to enable some allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::max ();

  // clean up
  isInitialized_ = false;
}

template <typename AllocatorConfigurationType,
          typename HeaderType,
          typename CommandType>
void
Stream_MessageBase_2<AllocatorConfigurationType,
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
          typename HeaderType,
          typename CommandType>
HeaderType
Stream_MessageBase_2<AllocatorConfigurationType,
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
