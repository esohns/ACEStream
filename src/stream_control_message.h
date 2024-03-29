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

#ifndef STREAM_CONTROL_MESSAGE_H
#define STREAM_CONTROL_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_iinitialize.h"

//#include "stream_cachedmessageallocator.h"
#include "stream_imessage.h"
//#include "stream_messageallocatorheap_base.h"

// forward declarations
class ACE_Allocator;

template <typename ControlType,
          typename MessageType>
//          ////////////////////////////////
//          typename AllocatorConfigurationType,
//          ////////////////////////////////
//          typename DataMessageType,
//          typename SessionMessageType>
class Stream_ControlMessage_T
 : public ACE_Message_Block
 , public Stream_IMessage_T<MessageType>
{
  // grant access to specific ctors
  //friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
  //                                               AllocatorConfigurationType,
  //                                               Stream_ControlMessage_T<ControlType,
  //                                                                       MessageType,
  //                                                                       AllocatorConfigurationType>,
  //                                               DataMessageType,
  //                                               SessionMessageType>;
  //friend class Stream_CachedMessageAllocator_T<ACE_MT_SYNCH,
  //                                             AllocatorConfigurationType,
  //                                             Stream_ControlMessage_T<ControlMessageType,
  //                                                                     AllocatorConfigurationType>,
  //                                             DataMessageType,
  //                                             SessionMessageType>;

  typedef ACE_Message_Block inherited;

 public:
  // convenient types
  typedef ControlType CONTROL_T;
  typedef Stream_ControlMessage_T<ControlType,
                                  MessageType> OWN_TYPE_T;

  Stream_ControlMessage_T (ControlType);
  // *NOTE*: to be used by message allocators
  // *TODO*: find a way to make this 'protected' (i.e. usable by allocators
  //         only)
  Stream_ControlMessage_T (ACE_Data_Block*,
                           ACE_Allocator*); // message allocator
  virtual ~Stream_ControlMessage_T ();

  // overload from ACE_Message_Block
  // *WARNING*: derived classes need to overload this
  virtual ACE_Message_Block* duplicate () const;

  // implement Stream_IMessage_T
  inline virtual bool expedited () const { return false; }
  inline virtual Stream_MessageId_t id () const { return id_; }
  inline virtual Stream_SessionId_t sessionId () const { return sessionId_; }
  inline virtual MessageType type () const { return type_; }

  bool initialize (Stream_SessionId_t,  // session id
                   const ControlType&);

  // debug tools
  static std::string ControlMessageTypeToString (MessageType); // message type

 protected:
  // (copy) ctor to be used by duplicate()
  Stream_ControlMessage_T (const OWN_TYPE_T&);

  Stream_MessageId_t id_;
  Stream_SessionId_t sessionId_;
  MessageType        type_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_ControlMessage_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_ControlMessage_T& operator= (const Stream_ControlMessage_T&))
};

// include template definition
#include "stream_control_message.inl"

//////////////////////////////////////////

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType> Stream_ControlMessage_t;

#endif
