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

#include <ace/Atomic_Op.h>
#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>

#include "common_idumpstate.h"

#include "stream_common.h"
#include "stream_idatamessage.h"
#include "stream_messageallocatorheap_base.h"

// forward declarations
class ACE_Allocator;

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          typename CommandType = int>
class Stream_MessageBase_T
 : public ACE_Message_Block
 , public Stream_IDataMessage<CommandType>
 , public Common_IDumpState
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<AllocatorConfigurationType,
                                                 ControlMessageType,
                                                 Stream_MessageBase_T<AllocatorConfigurationType,
                                                                      CommandType,
                                                                      ControlMessageType,
                                                                      SessionMessageType>,
                                                 SessionMessageType>;

 public:
  virtual ~Stream_MessageBase_T ();

  // implement (part of) Stream_IDataMessage<CommandType>
  virtual CommandType command () const;
  virtual int crunch (void);
  virtual unsigned int getID () const;
  static std::string CommandType2String (CommandType);

  // implement Common_IDumpState
  virtual void dump_state () const;

  // debug tools
  static void MessageType2String (Stream_MessageType, // message type
                                  std::string&);      // corresp. string

  // reset atomic id generator
  static void resetMessageIDGenerator ();

 protected:
  // convenient types
  typedef ACE_Message_Block MESSAGE_BLOCK_T;
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               ControlMessageType,
                               SessionMessageType,
                               CommandType> OWN_TYPE_T;

  // ctor(s) for STREAM_MESSAGE_OBJECT
  Stream_MessageBase_T ();
  // ctor(s) for MB_STREAM_DATA
  Stream_MessageBase_T (unsigned int); // size
  // copy ctor, to be used by derivates
  Stream_MessageBase_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  Stream_MessageBase_T (ACE_Data_Block*, // data block
                        ACE_Allocator*,  // message allocator
                        bool = true);    // increment running message counter ?
  Stream_MessageBase_T (ACE_Allocator*); // message allocator

  // used for pre-allocated messages
  void initialize (ACE_Data_Block*             // data block to use
                   /*const ACE_Time_Value&*/); // scheduled execution time

  Stream_MessageType type_;

 private:
  typedef ACE_Message_Block inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_T& operator= (const Stream_MessageBase_T&))

  // overrides from ACE_Message_Block
  // *IMPORTANT NOTE*: children ALWAYS need to override this too !
  virtual ACE_Message_Block* duplicate (void) const;

  // atomic ID generator
  static ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> currentID;

  unsigned int       messageID_;
};

//////////////////////////////////////////

#include "common_iget.h"

template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename HeaderType,
          typename CommandType = int>
class Stream_MessageBase_2
 : public Stream_MessageBase_T<AllocatorConfigurationType,
                               ControlMessageType,
                               SessionMessageType,
                               CommandType>
// , public Common_IGet_T<HeaderType>
// , public Common_IGet_T<ProtocolCommandType>
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<AllocatorConfigurationType,
                                                 ControlMessageType,
                                                 Stream_MessageBase_2<AllocatorConfigurationType,
                                                                      ControlMessageType,
                                                                      SessionMessageType,
                                                                      HeaderType,
                                                                      CommandType>,
                                                 SessionMessageType>;

 public:
  virtual ~Stream_MessageBase_2 ();

  // used for pre-allocated messages
  virtual void initialize (// Stream_MessageBase_T members
                           ACE_Data_Block*); // data block to use

//  // implement Common_IGet_T
  virtual HeaderType get () const;

 protected:
  Stream_MessageBase_2 (unsigned int); // size

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Stream_MessageBase_2 (const Stream_MessageBase_2&);
  // *NOTE*: to be used by allocators
  Stream_MessageBase_2 (ACE_Data_Block*, // data block to use
                        ACE_Allocator*); // message allocator

  bool isInitialized_;

 private:
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               ControlMessageType,
                               SessionMessageType,
                               CommandType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_2& operator= (const Stream_MessageBase_2&))
};

// include template definition
#include "stream_message_base.inl"

#endif
