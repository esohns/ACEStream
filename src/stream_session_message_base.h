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

#include "stream_messageallocatorheap_base.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;

template <typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType, // implements Common_IReferenceCount !
          typename UserDataType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType>
class Stream_SessionMessageBase_T
 : public ACE_Message_Block
 , public Common_IGet_T<SessionDataType>
// , public Common_IGet_T<UserDataType>
 , public Common_IDumpState
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<AllocatorConfigurationType,
                                                 ControlMessageType,
                                                 DataMessageType,
                                                 Stream_SessionMessageBase_T<AllocatorConfigurationType,
                                                                             SessionMessageType,
                                                                             SessionDataType,
                                                                             UserDataType,
                                                                             ControlMessageType,
                                                                             DataMessageType> >;

 public:
  // convenience types
  typedef SessionDataType DATA_T;
  typedef UserDataType USER_DATA_T;

  // *IMPORTANT NOTE*: fire-and-forget API (second argument)
  Stream_SessionMessageBase_T (SessionMessageType,
                               SessionDataType*&, // in/out
                               UserDataType*);
  virtual ~Stream_SessionMessageBase_T ();

  // initialization-after-construction
  // *IMPORTANT NOTE*: fire-and-forget API (second argument)
  //                   --> caller increase()s the argument, if applicable
  void initialize (SessionMessageType,
                   SessionDataType*&, // in/out
                   UserDataType*);

  // info
  SessionMessageType type () const;
  // implement Common_IGet_T
  virtual const SessionDataType& get () const;
  const UserDataType& data () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

  // debug tools
  static void MessageType2String (SessionMessageType, // session message type
                                  std::string&);      // corresp. string

 protected:
  // (copy) ctor to be used by duplicate()
  Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<AllocatorConfigurationType,
                                                                 SessionMessageType,
                                                                 SessionDataType,
                                                                 UserDataType,
                                                                 ControlMessageType,
                                                                 DataMessageType>&);

  // *NOTE*: to be used by message allocators
  Stream_SessionMessageBase_T (ACE_Allocator*); // message allocator
  Stream_SessionMessageBase_T (ACE_Data_Block*, // data block
                               ACE_Allocator*); // message allocator

  SessionDataType*   data_;
  bool               isInitialized_;
  SessionMessageType type_;
  UserDataType*      userData_;

 private:
  typedef ACE_Message_Block inherited;

  // convenient types
  typedef Stream_SessionMessageBase_T<AllocatorConfigurationType,
                                      SessionMessageType,
                                      SessionDataType,
                                      UserDataType,
                                      ControlMessageType,
                                      DataMessageType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T& operator= (const Stream_SessionMessageBase_T&))

  // overload from ACE_Message_Block
  // *WARNING*: derived classes need to overload this
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template definition
#include "stream_session_message_base.inl"

#endif
