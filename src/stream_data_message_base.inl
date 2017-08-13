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

#include <limits>

#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"

#include "stream_defines.h"
#include "stream_iallocator.h"
#include "stream_macros.h"

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_T (MessageType messageType_in,
                                                                 DataType& data_in)
 : inherited (messageType_in)
 , data_ (data_in)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_T (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , data_ ()
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_T (const Stream_DataMessageBase_T<AllocatorConfigurationType,
                                                                                                MessageType,
                                                                                                DataType,
                                                                                                CommandType>& message_in)
 : inherited (message_in)
 , data_ (const_cast<DataType&>(message_in.data_))
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

//  // ... and read/write pointers
//  inherited::rd_ptr (message_in.rd_ptr ());
//  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , data_ ()
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in,
                                                                 bool incrementMessageCounter_in)
 : inherited (dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , data_ ()
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::~Stream_DataMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::~Stream_DataMessageBase_T"));

  // *IMPORTANT NOTE*: this is an ugly hack to support some allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::max ();

  isInitialized_ = false;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::initialize (DataType& data_in,
                                                   ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::initialize"));

  data_ = data_in;

  // set data block (if any)
  if (dataBlock_in)
    inherited::initialize (dataBlock_in);
  inherited::type_ = STREAM_MESSAGE_OBJECT;

  isInitialized_ = true;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::set (DataType*& data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::set"));

  // sanity check(s)
  ACE_ASSERT (data_inout);

  data_ = *data_inout;

  // clean up
  delete data_inout;
  data_inout = NULL;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
ACE_Message_Block*
Stream_DataMessageBase_T<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

////////////////////////////////////////////////////////////////////////////////

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_2 (MessageType messageType_in,
                                                                 DataType*& data_inout)
 : inherited (messageType_in)
 , data_ (data_inout)
 , isInitialized_ (data_inout != NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  // set return values
  data_inout = NULL;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_2 (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , data_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);
}


template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_2 (const Stream_DataMessageBase_2<AllocatorConfigurationType,
                                                                                                MessageType,
                                                                                                DataType,
                                                                                                CommandType>& message_in)
 : inherited (message_in)
 , data_ (message_in.data_)
 , isInitialized_ (data_ != NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  if (data_)
  {
    try {
      // *TODO*: remove type inference
      data_->increase ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IReferenceCount::increase(), continuing\n")));
    }
  } // end IF

  // ... and read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_2 (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , data_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::Stream_DataMessageBase_2 (ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in,
                                                                 bool incrementMessageCounter_in)
 : inherited (dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , data_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::~Stream_DataMessageBase_2 ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::~Stream_DataMessageBase_2"));

  // *NOTE*: will be called just BEFORE this is passed back to the allocator

//  // *IMPORTANT NOTE*: this is an ugly hack to enable some allocators
//  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
//  inherited::priority_ = std::numeric_limits<unsigned long>::max ();

  // clean up
  if (data_)
  {
    try {
      // *TODO*: remove type inference
      data_->decrease ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IReferenceCount::decrease(), continuing\n")));
    }
    data_ = NULL;
  } // end IF

  isInitialized_ = false;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::initialize (DataType*& data_inout,
                                                   ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::initialize"));

  // sanity check(s)
  ACE_ASSERT (data_inout);

  if (isInitialized_)
  {
    if (data_)
    {
      try {
        // *TODO*: remove type inference
        data_->decrease ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Common_IReferenceCount::decrease(), continuing\n")));
      }
      data_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  if (data_inout)
  {
    data_ = data_inout;
    data_inout = NULL;
  } // end IF

  // set data block (if any)
  if (dataBlock_in)
    inherited::initialize (dataBlock_in);
//  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  isInitialized_ = true;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
const DataType&
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::get"));

  // sanity check(s)
  if (!isInitialized_ || !data_)
  {
    ACE_ASSERT (false);
    ACE_NOTREACHED (return DataType ();)
  } // end IF
  ACE_ASSERT (data_);

  return *data_;
}
template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::set (DataType*& data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::set"));

  // sanity check(s)
  ACE_ASSERT (data_inout);

  if (isInitialized_)
  {
    // sanity check(s)
    ACE_ASSERT (data_);

    data_->decrease ();
    data_ = NULL;
  } // end IF

  data_ = data_inout;

  // clean up
  data_inout = NULL;
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::dump_state"));

  // dump data
  if (data_)
  {
    try { // *TODO*: remove type inference
      data_->dump_state ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_IDumpState::dump_state(), continuing\n")));
    }
  } // end IF
  //   //delegate to base
  //   inherited::dump_state ();
}

template <typename AllocatorConfigurationType,
          typename MessageType,
          typename DataType,
          typename CommandType>
ACE_Message_Block*
Stream_DataMessageBase_2<AllocatorConfigurationType,
                         MessageType,
                         DataType,
                         CommandType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block...
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}
