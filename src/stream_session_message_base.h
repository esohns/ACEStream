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

#ifndef STREAM_SESSION_MESSAGE_BASE_H
#define STREAM_SESSION_MESSAGE_BASE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_idumpstate.h"
#include "common_iget.h"

#include "stream_common.h"
#include "stream_messageallocatorheap_base.h"

// forward declarations
class ACE_Allocator;
class Stream_MessageBase;

enum Stream_SessionMessageType
{
  // *NOTE*: see <stream_message_base.h> for details...
  STREAM_SESSION_MAP = ACE_Message_Block::MB_USER,
  // *** STREAM CONTROL ***
  SESSION_BEGIN,
  SESSION_STEP,
  SESSION_END,
  SESSION_STATISTICS
  // *** STREAM CONTROL - END ***
};

template <typename SessionDataType,
          typename UserDataType>
class Stream_SessionMessageBase_T
 : public ACE_Message_Block
 , public Common_IDumpState
 , public Common_IGet_T<SessionDataType>
    // , public Common_IGet_T<UserDataType>
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<Stream_MessageBase,
                                                 Stream_SessionMessageBase_T<Stream_SessionData,
                                                                             Stream_UserData> >;

 public:
  // *NOTE*: assumes responsibility for the second argument !
  Stream_SessionMessageBase_T (Stream_SessionMessageType,
                               SessionDataType*&, // in/out
                               UserDataType*);
  virtual ~Stream_SessionMessageBase_T ();

  // initialization-after-construction
  // *NOTE*: assumes responsibility for the second argument !
  void initialize (Stream_SessionMessageType,
                   SessionDataType*&, // in/out
                   UserDataType*);

  // info
  Stream_SessionMessageType type () const;
  // implement Common_IGet_T
  virtual const SessionDataType& get () const;
  const UserDataType& data () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

  // debug tools
  static void SessionMessageType2String (Stream_SessionMessageType, // message type
                                         std::string&);             // corresp. string

  // convenience types
  typedef SessionDataType SESSION_DATA_TYPE;
  typedef UserDataType USER_DATA_TYPE;

 protected:
  // (copy) ctor to be used by duplicate()
   Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<SessionDataType,
                                                                  UserDataType>&);

  // *NOTE*: these may be used by message allocators...
  // *WARNING*: these ctors are NOT threadsafe...
  Stream_SessionMessageBase_T (ACE_Allocator*); // message allocator
  Stream_SessionMessageBase_T (ACE_Data_Block*, // data block
                               ACE_Allocator*); // message allocator

  bool                      isInitialized_;
  Stream_SessionMessageType messageType_;
  SessionDataType*          sessionData_;
  UserDataType*             userData_;

 private:
  typedef ACE_Message_Block inherited;

  // convenient types
  typedef Stream_SessionMessageBase_T<SessionDataType,
                                      UserDataType> SELF_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T& operator= (const Stream_SessionMessageBase_T&))

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this too !
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template implementation
#include "stream_session_message_base.inl"

#endif
