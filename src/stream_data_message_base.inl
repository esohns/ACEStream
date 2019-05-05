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

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                 MessageType messageType_in,
                                                                 DataType& data_in)
 : inherited (sessionId_in,
              messageType_in)
 , data_ (data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::isInitialized_ = true;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_T (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , data_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_T (const Stream_DataMessageBase_T<DataType,
                                                                                                AllocatorConfigurationType,
                                                                                                MessageType,
                                                                                                CommandType>& message_in)
 : inherited (message_in)
 , data_ (const_cast<DataType&>(message_in.data_))
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

//  // ... and read/write pointers
//  inherited::rd_ptr (message_in.rd_ptr ());
//  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                 ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,        // session id
              messageAllocator_in) // message block allocator
 , data_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                 ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in,
                                                                 bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , data_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  inherited::isInitialized_ = false;
  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::~Stream_DataMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::~Stream_DataMessageBase_T"));

  //// *IMPORTANT NOTE*: this is an ugly hack to support some allocators
  ////                   (see e.g. stream_cachedmessageallocator.cpp:172)
  //inherited::priority_ = std::numeric_limits<unsigned long>::max ();
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::initialize (DataType& data_in,
                                                   Stream_SessionId_t sessionId_in,
                                                   ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::initialize"));

  data_ = data_in;

  inherited::initialize (sessionId_in,
                         dataBlock_in);
  inherited::type_ = STREAM_MESSAGE_OBJECT;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::setPR (DataType*& data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::setPR"));

  // sanity check(s)
  ACE_ASSERT (data_inout);

  data_ = *data_inout;

  // clean up
  delete data_inout; data_inout = NULL;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
ACE_Message_Block*
Stream_DataMessageBase_T<DataType,
                         AllocatorConfigurationType,
                         MessageType,
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

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_2 (Stream_SessionId_t sessionId_in,
                                                                 MessageType messageType_in,
                                                                 DataType*& data_inout)
 : inherited (sessionId_in,
              messageType_in)
 , data_ (data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  // *TODO*: this test doesn't suffice
  inherited::isInitialized_ = (data_ != NULL);

  // set return values
  data_inout = NULL;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_2 (MessageType messageType_in)
 : inherited (0,
              messageType_in)
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  // *TODO*: this test doesn't suffice
  inherited::isInitialized_ = false;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_2 (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  inherited::isInitialized_ = false;
  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_2 (const Stream_DataMessageBase_2<DataType,
                                                                                                AllocatorConfigurationType,
                                                                                                MessageType,
                                                                                                CommandType>& message_in)
 : inherited (message_in)
 , data_ (message_in.data_)
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

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_2 (Stream_SessionId_t sessionId_in,
                                                                 ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,        // session id
              messageAllocator_in) // message block allocator
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  inherited::isInitialized_ = false;
  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::Stream_DataMessageBase_2 (Stream_SessionId_t sessionId_in,
                                                                 ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in,
                                                                 bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , data_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::Stream_DataMessageBase_2"));

  inherited::isInitialized_ = false;
  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);

  // reset read/write pointers
  this->reset ();
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
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
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::initialize (DataType*& data_inout,
                                                   Stream_SessionId_t sesssionId_in,
                                                   ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::initialize"));

  if (inherited::isInitialized_)
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
  } // end IF

  // sanity check(s)
  ACE_ASSERT (!data_);

  if (data_inout)
  {
    data_ = data_inout; data_inout = NULL;
  } // end IF

  inherited::initialize (sesssionId_in,
                         dataBlock_in);
//  inherited::type_ = static_cast<MessageType> (STREAM_MESSAGE_OBJECT);
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::finalize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::finalize"));

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

  inherited::isInitialized_ = false;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
const DataType&
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::getR () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::getR"));

  // sanity check(s)
  if (!inherited::isInitialized_ || !data_)
  {
    ACE_ASSERT (false);
    ACE_NOTREACHED (return DataType ();)
  } // end IF
  ACE_ASSERT (data_);

  return *data_;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
                         CommandType>::setPR_2 (DataType*& data_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_2::setPR_2"));

  // sanity check(s)
  ACE_ASSERT (data_inout);

  if (inherited::isInitialized_)
  { ACE_ASSERT (data_);
    data_->decrease (); data_ = NULL;
  } // end IF

  data_ = data_inout;

  // clean up
  data_inout = NULL;
}

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
void
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
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

template <typename DataType,
          typename AllocatorConfigurationType,
          typename MessageType,
          typename CommandType>
ACE_Message_Block*
Stream_DataMessageBase_2<DataType,
                         AllocatorConfigurationType,
                         MessageType,
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
      message_p->release (); message_p = NULL;
      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}
