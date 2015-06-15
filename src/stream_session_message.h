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

#ifndef STREAM_SESSION_MESSAGE_H
#define STREAM_SESSION_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_idumpstate.h"

#include "stream_common.h"
#include "stream_exports.h"
#include "stream_messageallocatorheap.h"
#include "stream_session_data.h"
#include "stream_session_message_base.h"

// forward declaratation(s)
class ACE_Allocator;

class Stream_Export Stream_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_State,
                                      Stream_SessionData_t>
{
  // need access to specific ctors
  friend class Stream_MessageAllocatorHeap;
//   friend class Stream_MessageAllocatorHeapBase<arg1, arg2>;

 public:
  // *NOTE*: assume lifetime responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason...
  Stream_SessionMessage (Stream_SessionMessageType, // session message type
                         Stream_State*,             // stream state handle
                         Stream_SessionData_t*);    // session data handle
  virtual ~Stream_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_State,
                                      Stream_SessionData_t> inherited;

  // copy ctor to be used by duplicate()
  Stream_SessionMessage (const Stream_SessionMessage&);

  // *NOTE*: these may be used by message allocators...
  // *WARNING*: these ctors are NOT threadsafe...
  Stream_SessionMessage (ACE_Allocator*); // message allocator
  Stream_SessionMessage (ACE_Data_Block*, // data block
                         ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessage ());
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessage& operator= (const Stream_SessionMessage&));
};

#endif
