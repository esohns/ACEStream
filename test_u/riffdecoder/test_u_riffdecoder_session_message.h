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

#ifndef TEST_U_RIFFDECODER_SESSION_MESSAGE_H
#define TEST_U_RIFFDECODER_SESSION_MESSAGE_H

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "stream_common.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include "stream_directshow_allocator_base.h"
#include "stream_messageallocatorheap_base.h"
#else
#include "stream_messageallocatorheap_base.h"
#endif
#include "stream_session_message_base.h"

#include "test_u_riffdecoder_common.h"

// forward declaratation(s)
class ACE_Allocator;
class Stream_RIFFDecoder_Message;
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;

class Stream_RIFFDecoder_SessionMessage
 : public Stream_SessionMessageBase_T<Test_U_AllocatorConfiguration,
                                      ///
                                      Stream_RIFFDecoder_SessionData_t,
                                      Stream_UserData>
{
  // grant access to specific private ctors
  friend class Stream_MessageAllocatorHeapBase_T<Test_U_AllocatorConfiguration,

                                                 Stream_RIFFDecoder_Message,
                                                 Stream_RIFFDecoder_SessionMessage>;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  // *TODO*: (using gcc) cannot pass reference to pointer for some reason...
  Stream_RIFFDecoder_SessionMessage (Stream_SessionMessageType,          // session message type
                                     Stream_RIFFDecoder_SessionData_t*&, // session data container handle
                                     Stream_UserData*);                  // user data handle
  virtual ~Stream_RIFFDecoder_SessionMessage ();

  // overloaded from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Test_U_AllocatorConfiguration,
                                      ///
                                      Stream_RIFFDecoder_SessionData_t,
                                      Stream_UserData> inherited;

  // copy ctor to be used by duplicate()
  Stream_RIFFDecoder_SessionMessage (const Stream_RIFFDecoder_SessionMessage&);

  // *NOTE*: these may be used by message allocators...
  // *WARNING*: these ctors are NOT threadsafe...
  Stream_RIFFDecoder_SessionMessage (ACE_Allocator*); // message allocator
  Stream_RIFFDecoder_SessionMessage (ACE_Data_Block*, // data block
                                     ACE_Allocator*); // message allocator

  ACE_UNIMPLEMENTED_FUNC (Stream_RIFFDecoder_SessionMessage ())
  ACE_UNIMPLEMENTED_FUNC (Stream_RIFFDecoder_SessionMessage& operator= (const Stream_RIFFDecoder_SessionMessage&))
};

#endif
