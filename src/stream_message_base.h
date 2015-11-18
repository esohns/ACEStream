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

// forward declaration(s)
class ACE_Data_Block;
class ACE_Allocator;

enum Stream_MessageType_t
{
  // *NOTE*: see "ace/Message_Block.h" for details
  STREAM_MESSAGE_MAP   = ACE_Message_Block::MB_USER, // session (== 0x200)
  // *** control ***
  STREAM_MESSAGE_SESSION,
  // *** control - END ***
  STREAM_MESSAGE_MAP_2 = 0x300,                      // data
  // *** data ***
  STREAM_MESSAGE_DATA,   // protocol data
  STREAM_MESSAGE_OBJECT, // (OO) message object type (--> dynamic type)
  // *** data - END ***
  STREAM_MESSAGE_MAP_3 = 0x400,                      // protocol
  // *** protocol ***
  // *** protocol - END ***
  ///////////////////////////////////////
  STREAM_MESSAGE_MAX,
  STREAM_MESSAGE_INVALID
};

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
  typedef ACE_Message_Block MESSAGE_BLOCK_T;

  // ctor(s) for STREAM_MESSAGE_OBJECT
  Stream_MessageBase ();

  // ctor(s) for MB_STREAM_DATA
  Stream_MessageBase (unsigned int); // size
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

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase& operator= (const Stream_MessageBase&))

  // overrides from ACE_Message_Block
  // *IMPORTANT NOTE*: children ALWAYS need to override this too !
  virtual ACE_Message_Block* duplicate (void) const;

  // atomic ID generator
  static ACE_Atomic_Op<ACE_SYNCH_MUTEX,
                       unsigned int> currentID;

  unsigned int messageID_;
};

/////////////////////////////////////////

#include "common_iget.h"

template <typename HeaderType,
          typename ProtocolCommandType>
class Stream_MessageBase_T
 : public Stream_MessageBase
// , public Common_IGet_T<HeaderType>
// , public Common_IGet_T<ProtocolCommandType>
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_MessageBase_T<HeaderType,
                                                                      ProtocolCommandType>,
                                                 Stream_SessionMessageBase_T<Stream_SessionData,
                                                                             Stream_UserData> >;

 public:
  virtual ~Stream_MessageBase_T ();

  // used for pre-allocated messages...
  virtual void initialize (// Stream_MessageBase members
                           ACE_Data_Block*); // data block to use

//  // implement Common_IGet_T
  virtual HeaderType get () const;
  virtual ProtocolCommandType command () const = 0; // return value: message type
//  static std::string CommandType2String (ProtocolCommandType);

 protected:
  Stream_MessageBase_T (unsigned int); // size

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Stream_MessageBase_T (const Stream_MessageBase_T&);
  // *NOTE*: to be used by allocators...
  Stream_MessageBase_T (ACE_Data_Block*, // data block to use
                        ACE_Allocator*); // message allocator

 private:
  typedef Stream_MessageBase inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_T& operator= (const Stream_MessageBase_T&))

  bool isInitialized_;
};

#include "stream_message_base.inl"

#endif
