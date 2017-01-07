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

#include "stream_common.h"

template <typename MessageType>
class Stream_IMessage_T
{
 public:
  virtual ~Stream_IMessage_T () {}

  virtual Stream_MessageId_t id () const = 0;
  virtual MessageType type () const = 0;
};

template <typename MessageType,
          typename CommandType>
class Stream_IDataMessage_T
 : public Stream_IMessage_T<MessageType>
{
 public:
  virtual ~Stream_IDataMessage_T () {}

  virtual CommandType command () const = 0;
  // this is meant to "normalize" the PDU data in this message (fragment)
  // *NOTE*: steps to consider when implemented on top of an ACE_Message_Block:
  // 1. aligning the rd_ptr with base()
  //    --> ACE_Message_Block::crunch()/::memmove()]
  // *WARNING*: for obvious reasons, this will not work with shared buffers
  //            (i.e. ACE_Message_Block::duplicate())
  // 2. copying all bits from any continuation(s) into the head buffer (until
  //    ACE_Message_Block::capacity() is reached)
  // 3. adjusting the write pointer
  // 4. releasing any (obsoleted) continuations
  // --> *NOTE*: when applied throughout, AND:
  //     - the (leading/first) message buffer has capacity for a complete
  //       message (i.e. maximum allowed size)
  //     - the peer keeps to the standard and doesn't send oversized (!)
  //       messages
  //     THEN this method can be used to ensure that all (head) message buffers
  //     contain CONSISTENT (as in contiguous) and therefore COMPLETE messages.
  //     This function may be required to simplify parsing of protocol PDUs
  // *NOTE*: the C-ish signature reflects the fact that this may be implemented
  //         as an overload to ACE_Message_Block::crunch() (see above)
  virtual void crunch () = 0;
};

#endif
