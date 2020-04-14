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

#ifndef TEST_I_MESSAGE_H
#define TEST_I_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_message_base.h"

#include "test_i_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;

template <typename MessageType,
          typename SessionMessageType>
class Test_I_Message_T
 : public Stream_MessageBase_T<//struct Common_AllocatorConfiguration,
                               MessageType,
                               Stream_CommandType_t>
{
  typedef Stream_MessageBase_T<//struct Common_AllocatorConfiguration,
                               MessageType,
                               Stream_CommandType_t> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Message_T<MessageType,
                                                                  SessionMessageType>,
                                                 SessionMessageType>;

 public:
  Test_I_Message_T (unsigned int); // size
  inline virtual ~Test_I_Message_T () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  inline Stream_CommandType_t command () const { return ACE_Message_Block::MB_DATA; };
  static std::string CommandTypeToString (Stream_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_I_Message_T (const Test_I_Message_T&);

 private:
  // convenient types
  typedef Test_I_Message_T<MessageType,
                           SessionMessageType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Test_I_Message_T ())
  // *NOTE*: to be used by message allocators
  Test_I_Message_T (Stream_SessionId_t,
                    ACE_Data_Block*, // data block to use
                    ACE_Allocator*,  // message allocator
                    bool = true);    // increment running message counter ?
  Test_I_Message_T (Stream_SessionId_t,
                    ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_I_Message_T& operator= (const Test_I_Message_T&))
};

// include template definition
#include "test_i_message.inl"

#endif
