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

#ifndef STREAM_IMESSAGE_T_H
#define STREAM_IMESSAGE_T_H

#include "common_iget.h"

#include "stream_common.h"

class Stream_IMessage
{
 public:
  virtual bool expedited () const = 0; // always goes to the top of any queue
  virtual Stream_MessageId_t id () const = 0;
  virtual Stream_SessionId_t sessionId () const = 0;
};

template <typename MessageType>
class Stream_IMessage_T
 : public Stream_IMessage
{
 public:
  virtual MessageType type () const = 0;
};

template <typename MessageType,
          typename CommandType>
class Stream_IDataMessageBase_T
 : public Stream_IMessage_T<MessageType>
 , public Common_ISet_T<MessageType>
{
 public:
  virtual CommandType command () const = 0;

  // This functionality is intended to "normalize" data in the message
  // fragment(s).
  // *NOTE*: when applied 'early' in the processing, and (!):
  //         - the (leading/first) message buffer has capacity for a 'complete'
  //           message (i.e. in regard to maximum allowed PDU size)
  //         - the head module/upstream/peer adheres to the configured stream
  //           buffer size/protocol (i.e. doesn't enqueue oversized messages),
  //         then this method enforces all (head) message buffers to contain
  //         contiguous, COMPLETE messages
  // *NOTE*: as some parsers may not implement support for fragmented buffers,
  //         this function may be overloaded to pre-process the message data

  // *NOTE*: the default, ACE_Message_Block-derived implementation follows these
  //         steps:
  // 1. aligning the rd_ptr with base()
  //    --> ACE_Message_Block::crunch()/::memmove()]
  // *NOTE*: for obvious reasons, this will not work with "shared" buffers (i.e.
  //         buffers that have been ACE_Message_Block::duplicate()d)
  // 2. copying all bits from any continuation(s) into the "head" buffer (i.e.
  //    until ACE_Message_Block::capacity() is reached)
  // 3. adjusting the write pointer
  // 4. releasing any (obsoleted) continuations
  virtual void defragment () = 0;

  virtual void initialize (Stream_SessionId_t,             // session id
                           ACE_Data_Block*                 // data block to use
                           /*const ACE_Time_Value&*/) = 0; // scheduled execution time
};

template <typename DataType,
          typename MessageType,
          typename CommandType>
class Stream_IDataMessage_T
 : public Stream_IDataMessageBase_T<MessageType,
                                    CommandType>
 , public Common_IGetR_T<DataType>
 , public Common_ISetPR_T<DataType>
{
 public:
  // use assignment of first argument
  virtual void initialize (DataType&,                      // data
                           Stream_SessionId_t,             // session id
                           ACE_Data_Block*                 // data block to use
                           /*const ACE_Time_Value&*/) = 0; // scheduled execution time

  // *IMPORTANT NOTE*: fire-and-forget first argument
  virtual void initialize (DataType*&,                     // data handle
                           Stream_SessionId_t,             // session id
                           ACE_Data_Block*                 // data block to use
                           /*const ACE_Time_Value&*/) = 0; // scheduled execution time
};

#endif
