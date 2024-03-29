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
#include "stream_imessage.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_MessageAllocatorHeapBase_T;
//template <ACE_SYNCH_DECL,
//          typename AllocatorConfigurationType,
//          typename ControlMessageType,
//          typename DataMessageType,
//          typename SessionMessageType> class Stream_CachedMessageAllocator_T;

template <//typename AllocatorConfigurationType,
          ////////////////////////////////
          typename SessionMessageType,
          typename SessionDataType, // implements Common_IReferenceCount
          ////////////////////////////////
          typename UserDataType>
class Stream_SessionMessageBase_T
 : public ACE_Message_Block
 , public Stream_IMessage_T<SessionMessageType>
 , public Common_IGetR_T<SessionDataType>
 , public Common_ISetP_T<SessionDataType>
 , public Common_IGetR_2_T<UserDataType>
 , public Common_IDumpState
{
  typedef ACE_Message_Block inherited;

  //// grant access to specific ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               AllocatorConfigurationType,
  //                                               ControlMessageType,
  //                                               DataMessageType,
  //                                               Stream_SessionMessageBase_T<AllocatorConfigurationType,
  //                                                                           SessionMessageType,
  //                                                                           SessionDataType,
  //                                                                           UserDataType,
  //                                                                           ControlMessageType,
  //                                                                           DataMessageType> >;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             AllocatorConfigurationType,
  //                                             ControlMessageType,
  //                                             DataMessageType,
  //                                             Stream_SessionMessageBase_T<AllocatorConfigurationType,
  //                                                                         SessionMessageType,
  //                                                                         SessionDataType,
  //                                                                         UserDataType,
  //                                                                         ControlMessageType,
  //                                                                         DataMessageType> >;

 public:
  // convenient types
  typedef SessionMessageType MESSAGE_T;
  typedef SessionDataType DATA_T;
  typedef UserDataType USER_DATA_T;
  typedef Stream_IMessage_T<SessionMessageType> IMESSAGE_T;

  // *IMPORTANT NOTE*: fire-and-forget API (third argument)
  Stream_SessionMessageBase_T (Stream_SessionId_t,
                               SessionMessageType,
                               SessionDataType*&, // in/out
                               UserDataType*,
                               bool); // expedited ?
  virtual ~Stream_SessionMessageBase_T ();

  // initialization-after-construction
  // *IMPORTANT NOTE*: fire-and-forget API (third argument)
  //                   --> caller increase()s the argument iff applicable
  void initialize (Stream_SessionId_t,
                   SessionMessageType,
                   SessionDataType*&, // in/out
                   UserDataType*,
                   bool); // expedited ?

  // implement (part of) Stream_IMessage_T
  inline virtual bool expedited () const { return expedited_; }
  inline virtual Stream_SessionId_t sessionId () const { return sessionId_; }
  inline virtual SessionMessageType type () const { return type_; }

  // implement Common_IGet_T/Common_ISetP_T
  virtual const SessionDataType& getR () const;
  // *NOTE*: re-sets the session data; Notice how this really makes sense only
  //         after initialize()
  // *IMPORTANT NOTE*: fire-and-forget API (first argument)
  // *NOTE*: this will 'merge' any existing session data to the replacement
  //         using the (overloaded) operator+= first
  virtual void setP (SessionDataType*); // session data handle
  inline virtual const UserDataType& getR_2 () const { ACE_ASSERT (userData_); return *userData_; }

  // implement Common_IDumpState
  virtual void dump_state () const;

  // debug tools
  static void MessageTypeToString (SessionMessageType, // session message type
                                   std::string&);      // corresp. string

 protected:
  // convenient types
  typedef Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                                      SessionMessageType,
                                      SessionDataType,
                                      UserDataType> OWN_TYPE_T;

  // (copy) ctor to be used by duplicate()
  Stream_SessionMessageBase_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  Stream_SessionMessageBase_T (Stream_SessionId_t, // session id
                               ACE_Allocator*);    // message allocator
  Stream_SessionMessageBase_T (Stream_SessionId_t, // session id
                               ACE_Data_Block*,    // data block to use
                               ACE_Allocator*);    // message allocator

  SessionDataType*   data_;
  bool               expedited_;
  bool               isInitialized_;
  Stream_SessionId_t sessionId_;
  SessionMessageType type_;
  UserDataType*      userData_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_SessionMessageBase_T& operator= (const Stream_SessionMessageBase_T&))

  // overload from ACE_Message_Block
  // *WARNING*: derived classes need to overload this
  virtual ACE_Message_Block* duplicate (void) const;

  // implement (part of) Stream_IMessage_T
  inline virtual Stream_MessageId_t id () const { return 0; }
};

// include template definition
#include "stream_session_message_base.inl"

#endif
