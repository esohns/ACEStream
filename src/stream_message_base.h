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

#ifndef STREAM_MESSAGE_BASE_H
#define STREAM_MESSAGE_BASE_H

#include <string>

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Message_Block.h"

#include "common_iget.h"
#include "common_idumpstate.h"

#include "stream_common.h"
#include "stream_data_base.h"
#include "stream_imessage.h"

// forward declarations
class ACE_Allocator;

template <typename DataType = Stream_DataBase_T<Stream_CommandType_t>, // *TODO*: move this into a Stream_DataMessageBase_T
          typename MessageType = enum Stream_MessageType,
          typename CommandType = Stream_CommandType_t>
class Stream_MessageBase_T
 : public ACE_Message_Block
 , public Stream_IDataMessageBase_T<MessageType,
                                    CommandType>
 , public Common_ISet_T<Stream_SessionId_t>
 , public Common_IDumpState
{
  typedef ACE_Message_Block inherited;

 public:
  // convenient types
  typedef DataType DATA_T;

  virtual ~Stream_MessageBase_T ();

  // convenient types
  typedef MessageType MESSAGE_T;
  typedef CommandType COMMAND_T;
  typedef Stream_IDataMessage_T<DataType,
                                MessageType,
                                CommandType> IDATA_MESSAGE_T;

  // implement (part of) Stream_IDataMessage_T
  inline virtual bool expedited () const { return false; }
  inline virtual Stream_MessageId_t id () const { return id_; }
  inline virtual Stream_SessionId_t sessionId () const { return sessionId_; }
  inline virtual MessageType type () const { return type_; }
  inline virtual void set (const MessageType messageType_in) { type_ = messageType_in; }
  inline virtual CommandType command () const { ACE_ASSERT (inherited::data_block_); return static_cast<CommandType> (inherited::data_block_->msg_type ()); }
  virtual void defragment ();
  virtual void initialize (Stream_SessionId_t, // session id
                           ACE_Data_Block*     // data block to use
                           /*const ACE_Time_Value&*/); // scheduled execution time

  // implement Common_ISet_T
  inline virtual void set (const Stream_SessionId_t sessionId_in) { sessionId_ = sessionId_in; }

  // implement Common_IDumpState
  virtual void dump_state () const;

  // used for pre-allocated messages
  inline bool isInitialized () const { return isInitialized_; }

  // debug tools
  // *NOTE*: these specializations cover the library testcase applications only
  inline static std::string CommandTypeToString (CommandType) { return ACE_TEXT_ALWAYS_CHAR (""); }
  static void MessageTypeToString (enum Stream_MessageType, // message type
                                   std::string&);           // corresp. string

  // reset atomic id generator
  static void resetMessageIdGenerator ();

 protected:
  // convenient types
  typedef ACE_Message_Block MESSAGE_BLOCK_T;
  typedef Stream_MessageBase_T<DataType,
                               MessageType,
                               CommandType> OWN_TYPE_T;

  // ctor(s) for STREAM_MESSAGE_OBJECT (and derivates thereof)
  Stream_MessageBase_T (Stream_SessionId_t, // session id
                        MessageType);       // message type
  // ctor(s) for MB_STREAM_DATA
  explicit Stream_MessageBase_T (Stream_SessionId_t, // session id
                                 size_t);            // size
  // copy ctor, to be used by derivates
  Stream_MessageBase_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  Stream_MessageBase_T (Stream_SessionId_t, // session id
                        ACE_Data_Block*,    // data block to use
                        ACE_Allocator*,     // message allocator
                        bool = true);       // increment running message counter ?
  Stream_MessageBase_T (Stream_SessionId_t, // session id
                        ACE_Allocator*);    // message allocator

  Stream_MessageId_t id_;
  bool               isInitialized_;
  Stream_SessionId_t sessionId_;
  MessageType        type_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_T& operator= (const Stream_MessageBase_T&))

  // overrides from ACE_Message_Block
  // *IMPORTANT NOTE*: children ALWAYS need to override this too !
  virtual ACE_Message_Block* duplicate (void) const;

  // atomic ID generator
  typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX,
                        Stream_MessageId_t> ID_GENERATOR_T;
  static ID_GENERATOR_T currentId;
};

//////////////////////////////////////////

//#include "common_iget.h"

template <typename DataType, // = Stream_CommandType_t // *TODO*: move this into a Stream_DataMessageBase_2
          typename MessageType, // = enum Stream_MessageType
          ////////////////////////////////
          typename HeaderType,
          typename CommandType = Stream_CommandType_t>
class Stream_MessageBase_2
 : public Stream_MessageBase_T<DataType,
                               MessageType,
                               CommandType>
// , public Common_IGet_T<HeaderType>
// , public Common_IGet_T<ProtocolCommandType>
{
  typedef Stream_MessageBase_T<DataType,
                               MessageType,
                               CommandType> inherited;

 public:
  virtual ~Stream_MessageBase_2 ();

//  // implement Common_IGet_T
  virtual HeaderType get () const;

 protected:
  // convenient types
  typedef Stream_MessageBase_2<DataType,
                               MessageType,
                               HeaderType,
                               CommandType> OWN_TYPE_T;

  Stream_MessageBase_2 (Stream_SessionId_t, // session id
                        MessageType);       // message type
  explicit Stream_MessageBase_2 (Stream_SessionId_t, // session id
                                 size_t);            // size

  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Stream_MessageBase_2 (const OWN_TYPE_T&);
  // *NOTE*: to be used by allocators
  Stream_MessageBase_2 (Stream_SessionId_t, // session id
                        ACE_Data_Block*,    // data block to use
                        ACE_Allocator*,     // message allocator
                        bool = true);       // increment running message counter ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageBase_2& operator= (const Stream_MessageBase_2&))
};

// include template definition
#include "stream_message_base.inl"

#endif
