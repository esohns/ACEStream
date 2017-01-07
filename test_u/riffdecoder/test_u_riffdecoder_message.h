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

#ifndef TEST_U_RIFFDECODER_MESSAGE_H
#define TEST_U_RIFFDECODER_MESSAGE_H

#include <ace/Global_Macros.h>

#include "stream_message_base.h"

#include "test_u_riffdecoder_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
struct Test_U_RIFFDecoder_AllocatorConfiguration;

class Test_U_RIFFDecoder_Message
 : public Stream_MessageBase_T<struct Test_U_RIFFDecoder_AllocatorConfiguration,
                               enum Stream_MessageType,
                               int>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Test_U_RIFFDecoder_AllocatorConfiguration,
                                                 Test_U_ControlMessage_t,
                                                 Test_U_RIFFDecoder_Message,
                                                 Test_U_RIFFDecoder_SessionMessage>;

 public:
  Test_U_RIFFDecoder_Message (unsigned int); // size
  virtual ~Test_U_RIFFDecoder_Message ();

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: uses the allocator (if any)
  virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  virtual int command () const; // return value: message type

  static std::string CommandType2String (int);

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_U_RIFFDecoder_Message (const Test_U_RIFFDecoder_Message&);

 private:
  typedef Stream_MessageBase_T<struct Test_U_RIFFDecoder_AllocatorConfiguration,
                               enum Stream_MessageType,
                               int> inherited;

  ACE_UNIMPLEMENTED_FUNC (Test_U_RIFFDecoder_Message ())
  // *NOTE*: to be used by message allocators
  Test_U_RIFFDecoder_Message (ACE_Data_Block*, // data block
                              ACE_Allocator*,  // message allocator
                              bool = true);    // increment running message counter ?
  Test_U_RIFFDecoder_Message (ACE_Allocator*); // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_U_RIFFDecoder_Message& operator= (const Test_U_RIFFDecoder_Message&))
};

#endif
