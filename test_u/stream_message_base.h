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

#ifndef STREAM_MESSAGE_BASE_H
#define STREAM_MESSAGE_BASE_H

#include <string>

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "common_idumpstate.h"

#include "stream_common.h"
#include "stream_exports.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_message_base.h"

enum Stream_MessageType_t
{
  // *NOTE*: see <ace/Message_Block.h> for details...
  MESSAGE_SESSION_MAP  = ACE_Message_Block::MB_USER,
  // *** STREAM CONTROL ***
  MESSAGE_SESSION,
  // *** STREAM CONTROL - END ***
  MESSAGE_DATA_MAP     = 0x300,
  // *** STREAM DATA ***
  MESSAGE_DATA,   // protocol data
  MESSAGE_OBJECT, // (OO) message object type (--> can be downcast dynamically)
  // *** STREAM DATA - END ***
  MESSAGE_PROTOCOL_MAP = 0x400
};

// forward declaratation(s)
class ACE_Allocator;
//template <> class Stream_SessionMessageBase_T<Stream_SessionData, Stream_UserData>;

class Stream_Export Stream_MessageBase
 : public ACE_Message_Block,
   public Common_IDumpState
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_MessageBase,
                                                 Stream_SessionMessageBase_T<Stream_SessionData,
                                                                             Stream_UserData> >;

 public:
  virtual ~Stream_MessageBase ();

  // info
  unsigned int getID () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

  // reset atomic id generator
  static void resetMessageIDGenerator ();

  // helper methods
  static void MessageType2String (ACE_Message_Type, // as returned by msg_type()
                                  std::string&);    // return value: type string

 protected:
  Stream_MessageBase (unsigned int); // size

//   // ctor(s) for MB_STREAM_DATA
//   Stream_MessageBase(const unsigned long&); // total size of message data (off-wire)
  // copy ctor, to be used by derivates
  Stream_MessageBase (const Stream_MessageBase&);
  // *NOTE*: to be used by message allocators...
  Stream_MessageBase (ACE_Data_Block*, // data block
                      ACE_Allocator*,  // message allocator
                      bool = true);    // increment running message counter ?
  Stream_MessageBase (ACE_Allocator*); // message allocator

  // used for pre-allocated messages...
  void initialize (ACE_Data_Block*          // data block to use
                   /*const ACE_Time_Value&*/); // scheduled execution time

 private:
  typedef ACE_Message_Block inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase& operator= (const Stream_MessageBase&))

  // overrides from ACE_Message_Block
  // *IMPORTANT NOTE*: children ALWAYS need to override this too !
  virtual ACE_Message_Block* duplicate (void) const;

  // atomic ID generator
  static ACE_Atomic_Op<ACE_SYNCH_MUTEX,
                       unsigned int> currentID;

  unsigned int messageID_;
};

#endif
