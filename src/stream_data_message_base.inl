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

#include "ace/Malloc_Base.h"
#include "ace/Log_Msg.h"

#include "stream_macros.h"
#include "stream_defines.h"

template <typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         CommandType>::Stream_DataMessageBase_T (DataType*& data_inout)
 : inherited (0,                                  // size
              Stream_MessageBase::MB_STREAM_OBJ,  // type
              NULL,                               // continuation
              NULL,                               // data
              NULL,                               // buffer allocator
              NULL,                               // locking strategy
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
              ACE_Time_Value::zero,               // execution time
              ACE_Time_Value::max_time,           // deadline time
              NULL,                               // data block allocator
              NULL)                               // message block allocator
 , data_ (data_inout)
 , isInitialized_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  // set return values
  data_inout = NULL;
}

template <typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         CommandType>::Stream_DataMessageBase_T (const Stream_DataMessageBase_T<DataType,
                                                                                                CommandType>& message_in)
 : inherited (message_in)
 , data_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  // maintain the same message type
  msg_type (message_in.msg_type ());

  // ... and read/write pointers
  rd_ptr (message_in.rd_ptr ());
  wr_ptr (message_in.wr_ptr ());
}

template <typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         CommandType>::Stream_DataMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , data_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  // set correct message type
  msg_type (Stream_MessageBase::MB_STREAM_OBJ);

  // reset read/write pointers
  reset ();
}

template <typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         CommandType>::Stream_DataMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                 ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,
              messageAllocator_in)
 , data_ (NULL)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::Stream_DataMessageBase_T"));

  // set correct message type
  msg_type (Stream_MessageBase::MB_STREAM_OBJ);

  // reset read/write pointers
  reset ();
}

template <typename DataType,
          typename CommandType>
Stream_DataMessageBase_T<DataType,
                         CommandType>::~Stream_DataMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::~Stream_DataMessageBase_T"));

  // clean up
  if (data_)
  {
    // decrease reference counter...
    try
    {
      data_->decrease ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in decrease(), continuing")));
    }
    data_ = NULL;
  } // end IF

  isInitialized_ = false;
}

template <typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_T<DataType,
                         CommandType>::init (DataType*& data_inout,
                                             ACE_Data_Block* dataBlock_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::init"));

  ACE_ASSERT (!isInitialized_);
  ACE_ASSERT (data_inout);

  data_ = data_inout;

  // set return values
  data_inout = NULL;

  // set our data block (if any)
  if (dataBlock_in)
  {
    inherited::init (dataBlock_in);

    // (re)set correct message type
    msg_type (Stream_MessageBase::MB_STREAM_OBJ);
  } // end IF

  isInitialized_ = true;
}

template <typename DataType,
          typename CommandType>
const DataType* const
Stream_DataMessageBase_T<DataType,
                         CommandType>::getData () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::getData"));

  ACE_ASSERT (isInitialized_);

  return data_;
}

template <typename DataType,
          typename CommandType>
void
Stream_DataMessageBase_T<DataType,
                         CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::dump_state"));

  // dump data...
  if (data_)
  {
    try
    {
      data_->dump_state ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in dump_state(), continuing")));
    }
  } // end IF
//   //delegate to base
//   inherited::dump_state ();
}

//template <typename DataType,
//          typename CommandType>
//ACE_Message_Block*
//Stream_DataMessageBase_T<DataType,
//                         CommandType>::duplicate (void) const
//{
//STREAM_TRACE (ACE_TEXT ("Stream_DataMessageBase_T::duplicate"));

//  own_type* new_message = NULL;

//  // create a new <Stream_DataMessageBase_T> that contains unique copies of
//  // the message block fields, but a reference counted duplicate of
//  // the <ACE_Data_Block>.

//  // if there is no allocator, use the standard new and delete calls.
//  if (message_block_allocator_ == NULL)
//    ACE_NEW_RETURN (new_message,
//                    own_type (*this), // invoke copy ctor
//                    NULL);

//  ACE_NEW_MALLOC_RETURN (new_message,
//                         static_cast<own_type*> (message_block_allocator_->malloc (capacity ())),
//                         own_type (*this), // invoke copy ctor
//                         NULL);

//  // increment the reference counts of all the continuation messages
//  if (cont_)
//  {
//    new_message->cont_ = cont_->duplicate ();

//    // when things go wrong, release all resources and return
//    if (new_message->cont_ == 0)
//    {
//      new_message->release ();
//      new_message = NULL;
//    } // end IF
//  } // end IF

//  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

//  return new_message;
//}
