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

#ifndef STREAM_DATA_MESSAGE_BASE_H
#define STREAM_DATA_MESSAGE_BASE_H

#include "ace/Global_Macros.h"

#include "common_iget.h"

#include "stream_message_base.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

template <typename AllocatorConfigurationType,
          typename MessageType,
          ////////////////////////////////
          typename DataType,
          typename CommandType = int>
class Stream_DataMessageBase_T
 : public Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType>
 , public Common_IGetR_T<DataType>
 , public Common_ISetPR_T<DataType>
{
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType> inherited;

 public:
  // convenient types
  typedef Stream_DataMessageBase_T<AllocatorConfigurationType,
                                   MessageType,
                                   DataType,
                                   CommandType> OWN_TYPE_T;
  typedef MessageType MESSAGE_T;
  typedef DataType DATA_T;
  typedef CommandType COMMAND_T;

  // initialization-after-construction
  using inherited::initialize;
  void initialize (DataType&,               // data
                   Stream_SessionId_t,      // session id
                   ACE_Data_Block* = NULL); // data block to use
  inline bool isInitialized () const { return isInitialized_; }

  // implement Common_IGet_T
  inline virtual const DataType& getR () const { return data_; }
  // implement Common_ISetPP_T
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void setPR (DataType*&); // data

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // *NOTE*: this ctor doesn't allocate a buffer off the heap
  Stream_DataMessageBase_T (Stream_SessionId_t, // session id
                            MessageType,        // message type
                            DataType&);         // data handle
  explicit Stream_DataMessageBase_T (unsigned int); // size
  // copy ctor, to be used by derived::duplicate()
  // *WARNING*: while the clone inherits a "shallow copy" of the referenced
  //            data block, it will NOT inherit the attached data
  //            --> use initialize()
  Stream_DataMessageBase_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  // *TODO*: these ctors are NOT thread-safe
  Stream_DataMessageBase_T (Stream_SessionId_t, // session id
                            ACE_Allocator*);    // message allocator
  Stream_DataMessageBase_T (Stream_SessionId_t, // session id
                            ACE_Data_Block*,    // data block to use
                            ACE_Allocator*,     // message allocator
                            bool = true);       // increment running message counter ?

  virtual ~Stream_DataMessageBase_T ();

  DataType data_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_T& operator= (const Stream_DataMessageBase_T&))

  // overriden from ACE_Message_Block
  // *NOTE*: derived classes need to override this
  virtual ACE_Message_Block* duplicate (void) const;
};

////////////////////////////////////////////////////////////////////////////////

template <typename AllocatorConfigurationType,
          typename MessageType,
          ////////////////////////////////
          typename DataType, // *NOTE*: this implements Common_IReferenceCount
          typename CommandType = int>
class Stream_DataMessageBase_2
 : public Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType>
 , public Common_IGetR_T<DataType>
 , public Common_ISetPR_2_T<DataType>
{
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType> inherited;

 public:
  // convenient types
  typedef Stream_DataMessageBase_2<AllocatorConfigurationType,
                                   MessageType,
                                   DataType,
                                   CommandType> OWN_TYPE_T;
  typedef DataType DATA_T;

  // initialization-after-construction
  using inherited::initialize;
  // *IMPORTANT NOTE*: fire-and-forget API (first argument)
  void initialize (DataType*&,              // data handle
                   Stream_SessionId_t,      // session id
                   ACE_Data_Block* = NULL); // data block to use
  inline bool isInitialized () const { return isInitialized_; }

  // implement Common_IGetR_T
  virtual const DataType& getR () const;
  // implement Common_ISetPR_T
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void setPR_2 (DataType*&); // data

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // *IMPORTANT NOTE*: fire-and-forget API
  // *NOTE*: this ctor doesn't allocate a buffer off the heap
  Stream_DataMessageBase_2 (Stream_SessionId_t, // session id
                            MessageType,        // message type
                            DataType*&);        // data handle
  Stream_DataMessageBase_2 (MessageType);       // message type
  explicit Stream_DataMessageBase_2 (unsigned int); // size
  // copy ctor, to be used by derived::duplicate()
  // *WARNING*: while the clone inherits a "shallow copy" of the referenced
  //            data block, it will NOT inherit the attached data
  //            --> use initialize()
  Stream_DataMessageBase_2 (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  // *TODO*: these ctors are NOT thread-safe
  Stream_DataMessageBase_2 (Stream_SessionId_t, // session id
                            ACE_Allocator*);    // message allocator
  Stream_DataMessageBase_2 (Stream_SessionId_t, // session id
                            ACE_Data_Block*,    // data block to use
                            ACE_Allocator*,     // message allocator
                            bool = true);       // increment running message counter ?

  virtual ~Stream_DataMessageBase_2 ();

  DataType* data_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_2& operator= (const Stream_DataMessageBase_2&))

  // overriden from ACE_Message_Block
  // *NOTE*: derived classes need to override this
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template definition
#include "stream_data_message_base.inl"

#endif
