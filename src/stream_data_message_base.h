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
#include "ace/Message_Block.h"

#include "stream_message_base.h"

// forward declarations
class ACE_Allocator;

template <typename DataType,
          typename CommandType>
class Stream_DataMessageBase_T
 : public Stream_MessageBase
{
 public:
  virtual ~Stream_DataMessageBase_T ();

  // initialization-after-construction
  // *NOTE*: assumes lifecycle responsibility for the first argument
  void initialize (DataType*&,              // data handle
                   ACE_Data_Block* = NULL); // buffer

  // *TODO*: clean this up !
  const DataType* const getData () const;
  virtual CommandType getCommand () const = 0; // return value: message type

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  // *NOTE*: assume lifetime responsibility for the argument !
  // *WARNING*: this ctor doesn't allocate a buffer off the heap...
  Stream_DataMessageBase_T (DataType*&); // data handle
  // copy ctor, to be used by derived::duplicate()
  // *WARNING*: while the clone inherits a "shallow copy" of the referenced
  // data block, it will NOT inherit the attached data --> use init()...
  Stream_DataMessageBase_T (const Stream_DataMessageBase_T&);

  // *NOTE*: to be used by message allocators...
  // *TODO*: these ctors are NOT threadsafe...
  Stream_DataMessageBase_T (ACE_Allocator*); // message allocator
  Stream_DataMessageBase_T (ACE_Data_Block*, // data block
                            ACE_Allocator*); // message allocator

 private:
  typedef Stream_MessageBase inherited;
  typedef Stream_DataMessageBase_T<DataType,
                                   CommandType> own_type;

  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_T ());
  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_T& operator= (const Stream_DataMessageBase_T&));

  // overriden from ACE_Message_Block
  virtual ACE_Message_Block* duplicate (void) const = 0;

  DataType* data_;
  bool      isInitialized_;
};

// include template implementation
#include "stream_data_message_base.inl"

#endif
