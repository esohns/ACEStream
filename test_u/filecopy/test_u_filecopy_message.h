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

#ifndef TEST_U_FILECOPY_MESSAGE_H
#define TEST_U_FILECOPY_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_message_base.h"

#include "test_u_filecopy_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Stream_Filecopy_SessionMessage;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename MessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;

class Stream_Filecopy_Message
 : public Stream_MessageBase_T<struct Stream_AllocatorConfiguration,
                               enum Stream_MessageType,
                               Stream_CommandType_t>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 Stream_AllocatorConfiguration,
                                                 Test_U_ControlMessage_t,
                                                 Stream_Filecopy_Message,
                                                 Stream_Filecopy_SessionMessage>;

 public:
  Stream_Filecopy_Message (unsigned int); // size
  inline virtual ~Stream_Filecopy_Message () {};

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  inline virtual Stream_CommandType_t command () const { return ACE_Message_Block::MB_DATA; };
  static std::string CommandTypeToString (Stream_CommandType_t);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Stream_Filecopy_Message (const Stream_Filecopy_Message&);

 private:
  typedef Stream_MessageBase_T<struct Stream_AllocatorConfiguration,
                               enum Stream_MessageType,
                               Stream_CommandType_t> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Message ())
  // *NOTE*: to be used by message allocators
  Stream_Filecopy_Message (Stream_SessionId_t, // session id
                           ACE_Data_Block*,    // data block
                           ACE_Allocator*,     // message allocator
                           bool = true);       // increment running message counter ?
  Stream_Filecopy_Message (Stream_SessionId_t, // session id
                           ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_Message& operator= (const Stream_Filecopy_Message&))
};

#endif
