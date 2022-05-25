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

#ifndef TEST_U_MESSAGE_H
#define TEST_U_MESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_control_message.h"
#include "stream_data_base.h"
#include "stream_message_base.h"
#include "stream_messageallocatorheap_base.h"

#include "test_u_stream_common.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Test_U_SessionMessage;

//////////////////////////////////////////

class Test_U_Message
 : public Stream_MessageBase_T<Stream_DataBase_T<Stream_CommandType_t>,
                               enum Stream_MessageType,
                               Stream_CommandType_t>
{
  typedef Stream_MessageBase_T<Stream_DataBase_T<Stream_CommandType_t>,
                               enum Stream_MessageType,
                               Stream_CommandType_t> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_U_Message,
                                                 Test_U_SessionMessage>;

 public:
  Test_U_Message (Stream_SessionId_t, // session id
                  unsigned int);      // size
  // *NOTE*: to be used by message allocators
  // *TODO*: --> make this private
  Test_U_Message (Stream_SessionId_t, // session id
                  ACE_Data_Block*,    // data block to use
                  ACE_Allocator*,     // message allocator
                  bool = true);       // increment running message counter ?
  inline virtual ~Test_U_Message () {}

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy of ourselves that references the same packet
  // *NOTE*: this uses our allocator (if any) to create a new message
  //virtual ACE_Message_Block* duplicate (void) const;

  // implement Stream_MessageBase_T
  //virtual HTTP_Method_t command () const; // return value: message type
  //inline static std::string CommandTypeToString (HTTP_Method_t method_in) { return (method_in == HTTP_Codes::HTTP_METHOD_INVALID ? ACE_TEXT_ALWAYS_CHAR (HTTP_COMMAND_STRING_RESPONSE) : HTTP_Tools::MethodToString (method_in)); }

 protected:
  // copy ctor to be used by duplicate() and derived classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Test_U_Message (const Test_U_Message&);

 private:
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message ())
  Test_U_Message (Stream_SessionId_t,
                  ACE_Allocator*);    // message allocator
  ACE_UNIMPLEMENTED_FUNC (Test_U_Message& operator= (const Test_U_Message&))
};

#endif
