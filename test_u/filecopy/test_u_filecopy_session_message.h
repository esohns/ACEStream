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

#ifndef TEST_U_FILECOPY_SESSION_MESSAGE_H
#define TEST_U_FILECOPY_SESSION_MESSAGE_H

#include <ace/Global_Macros.h>
#include <ace/Message_Block.h>

#include "stream_common.h"
#include "stream_session_message_base.h"

#include "test_u_filecopy_common.h"

// forward declaratation(s)
class ACE_Allocator;
class Stream_Filecopy_Message;
template <typename AllocatorConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Stream_Filecopy_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Stream_Filecopy_SessionData_t,
                                      Stream_UserData,
                                      Test_U_ControlMessage_t,
                                      Stream_Filecopy_Message>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 Test_U_ControlMessage_t,
                                                 Stream_Filecopy_Message,
                                                 Stream_Filecopy_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason
  Stream_Filecopy_SessionMessage (Stream_SessionMessageType,       // session message type
                                  Stream_Filecopy_SessionData_t*&, // session data container handle
                                  Stream_UserData*);               // user data handle
  virtual ~Stream_Filecopy_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Stream_Filecopy_SessionData_t,
                                      Stream_UserData,
                                      Test_U_ControlMessage_t,
                                      Stream_Filecopy_Message> inherited;

  // copy ctor to be used by duplicate()
  Stream_Filecopy_SessionMessage (const Stream_Filecopy_SessionMessage&);

  // *NOTE*: these may be used by message allocators
  // *WARNING*: these ctors are NOT threadsafe
  Stream_Filecopy_SessionMessage (ACE_Allocator*); // message allocator
  Stream_Filecopy_SessionMessage (ACE_Data_Block*, // data block
                                  ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Filecopy_SessionMessage& operator= (const Stream_Filecopy_SessionMessage&))
};

#endif
