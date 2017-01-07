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

#include <ace/Global_Macros.h>

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
          typename CommandType>
class Stream_DataMessageBase_T
 : public Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType>
 , public Common_IGet_T<DataType>
 , public Common_ISetPP_T<DataType>
{
 public:
  // convenient typedefs
  typedef Stream_DataMessageBase_T<AllocatorConfigurationType,
                                   MessageType,
                                   DataType,
                                   CommandType> OWN_TYPE_T;
  typedef DataType DATA_T;

  // initialization-after-construction
  void initialize (DataType&,               // data
                   ACE_Data_Block* = NULL); // buffer
  bool isInitialized () const;

  // implement Common_IGet_T
  inline virtual const DataType& get () const { return data_; };
  // implement Common_ISetPP_T
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void set (DataType*&); // data

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  Stream_DataMessageBase_T (unsigned int); // size
  // *WARNING*: this ctor doesn't allocate a buffer off the heap
  Stream_DataMessageBase_T (DataType&); // data handle
  // copy ctor, to be used by derived::duplicate()
  // *WARNING*: while the clone inherits a "shallow copy" of the referenced
  //            data block, it will NOT inherit the attached data
  //            --> use initialize()
  Stream_DataMessageBase_T (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  // *TODO*: these ctors are NOT thread-safe
  Stream_DataMessageBase_T (ACE_Allocator*); // message allocator
  Stream_DataMessageBase_T (ACE_Data_Block*, // data block
                            ACE_Allocator*,  // message allocator
                            bool = true);    // increment running message counter ?

  virtual ~Stream_DataMessageBase_T ();

  DataType data_;
  bool     isInitialized_;

 private:
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType> inherited;

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
          typename DataType, // *NOTE*: this implements Common_IReferenceCount !
          typename CommandType>
class Stream_DataMessageBase_2
 : public Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType>
 , public Common_IGet_T<DataType>
{
 public:
  // convenient types
  typedef Stream_DataMessageBase_2<AllocatorConfigurationType,
                                   MessageType,
                                   DataType,
                                   CommandType> OWN_TYPE_T;
  typedef DataType DATA_T;

  // initialization-after-construction
  // *IMPORTANT NOTE*: fire-and-forget API (first argument)
  void initialize (DataType*&,              // data handle
                   ACE_Data_Block* = NULL); // buffer
  bool isInitialized () const;

  // override Common_IGet_T
  virtual const DataType& get () const;
  // override Common_ISetPP_T
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void set (DataType*&); // data

  // implement Common_IDumpState
  virtual void dump_state () const;

 protected:
  Stream_DataMessageBase_2 (unsigned int); // size
  // *IMPORTANT NOTE*: fire-and-forget API
  // *WARNING*: this ctor doesn't allocate a buffer off the heap
  Stream_DataMessageBase_2 (DataType*&); // data handle
  // copy ctor, to be used by derived::duplicate()
  // *WARNING*: while the clone inherits a "shallow copy" of the referenced
  //            data block, it will NOT inherit the attached data
  //            --> use initialize()
  Stream_DataMessageBase_2 (const OWN_TYPE_T&);

  // *NOTE*: to be used by message allocators
  // *TODO*: these ctors are NOT thread-safe
  Stream_DataMessageBase_2 (ACE_Allocator*); // message allocator
  Stream_DataMessageBase_2 (ACE_Data_Block*, // data block
                            ACE_Allocator*,  // message allocator
                            bool = true);    // increment running message counter ?

  virtual ~Stream_DataMessageBase_2 ();

  DataType* data_;
  bool      isInitialized_;

 private:
  typedef Stream_MessageBase_T<AllocatorConfigurationType,
                               MessageType,
                               CommandType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_2 ())
  ACE_UNIMPLEMENTED_FUNC (Stream_DataMessageBase_2& operator= (const Stream_DataMessageBase_2&))

  // overriden from ACE_Message_Block
  // *NOTE*: derived classes need to override this
  virtual ACE_Message_Block* duplicate (void) const;
};

// include template definition
#include "stream_data_message_base.inl"

#endif
