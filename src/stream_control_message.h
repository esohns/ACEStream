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

#include "stream_cachedmessageallocator.h"
#include "stream_messageallocatorheap_base.h"

// forward declarations
class ACE_Allocator;

template <typename ControlMessageType,
          ////////////////////////////////
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_ControlMessage_T
 : public ACE_Message_Block
 , public Common_IInitialize_T<ControlMessageType>
{
  // grant access to specific ctors
  friend class Stream_MessageAllocatorHeapBase_T<AllocatorConfigurationType,
                                                 Stream_ControlMessage_T<ControlMessageType,
                                                                         AllocatorConfigurationType,
                                                                         DataMessageType,
                                                                         SessionMessageType>,
                                                 DataMessageType,
                                                 SessionMessageType>;
  friend class Stream_CachedMessageAllocator_T<AllocatorConfigurationType,
                                               Stream_ControlMessage_T<ControlMessageType,
                                                                       AllocatorConfigurationType,
                                                                       DataMessageType,
                                                                       SessionMessageType>,
                                               DataMessageType,
                                               SessionMessageType>;

 public:
  // convenient types
  typedef Stream_ControlMessage_T<ControlMessageType,
                                  AllocatorConfigurationType,
                                  DataMessageType,
                                  SessionMessageType> OWN_TYPE_T;

  // *IMPORTANT NOTE*: fire-and-forget API (second argument)
  Stream_ControlMessage_T (ControlMessageType);
  virtual ~Stream_ControlMessage_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ControlMessageType&);

  ControlMessageType type () const;

  // debug tools
  static void ControlMessageType2String (ControlMessageType, // session message type
                                         std::string&);      // corresp. string

 protected:
  // (copy) ctor to be used by duplicate()
  Stream_ControlMessage_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  Stream_ControlMessage_T (ACE_Data_Block*,
                           ACE_Allocator*); // message allocator

  ControlMessageType type_;

 private:
  typedef ACE_Message_Block inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_ControlMessage_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_ControlMessage_T& operator= (const Stream_ControlMessage_T&))

  // overload from ACE_Message_Block
  // *WARNING*: derived classes need to overload this
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template definition
#include "stream_control_message.inl"

#endif
