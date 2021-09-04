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

#ifndef TEST_I_SESSION_MESSAGE_H
#define TEST_I_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#include "stream_session_message_base.h"

#include "test_i_http_get_stream_common.h"

// forward declaration(s)
class ACE_Allocator;
class Test_I_Stream_Message;
template <ACE_SYNCH_DECL,
          typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T;

class Test_I_Stream_SessionMessage
 : public Stream_SessionMessageBase_T<//struct Common_Parser_FlexAllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Stream_SessionData_t,
                                      struct Stream_UserData>
{
  typedef Stream_SessionMessageBase_T<//struct Common_Parser_FlexAllocatorConfiguration,
                                      enum Stream_SessionMessageType,
                                      Test_I_Stream_SessionData_t,
                                      struct Stream_UserData> inherited;

  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Common_AllocatorConfiguration,
                                                 Stream_ControlMessage_t,
                                                 Test_I_Stream_Message,
                                                 Test_I_Stream_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Test_I_Stream_SessionMessage (Stream_SessionId_t,
                                enum Stream_SessionMessageType,
                                Test_I_Stream_SessionData_t*&,    // session data container handle
                                struct Stream_UserData*);
  inline virtual ~Test_I_Stream_SessionMessage () {};

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  // copy ctor to be used by duplicate()
  Test_I_Stream_SessionMessage (const Test_I_Stream_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Test_I_Stream_SessionMessage (Stream_SessionId_t,
                                ACE_Allocator*); // message allocator
  Test_I_Stream_SessionMessage (Stream_SessionId_t,
                                ACE_Data_Block*, // data block to use
                                ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Test_I_Stream_SessionMessage& operator= (const Test_I_Stream_SessionMessage&))
};

#endif
