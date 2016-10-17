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

#include <ace/Log_Msg.h>
#include <ace/Malloc_Base.h>
#include <ace/Time_Value.h>

#include "stream_common.h"
#include "stream_macros.h"

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::Stream_ControlMessage_T (ControlMessageType messageType_in)
 : inherited (0,                                  // size
              messageType_in,                     // type
              NULL,                               // continuation
              NULL,                               // data
              NULL,                               // buffer allocator
              NULL,                               // locking strategy
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
              ACE_Time_Value::zero,               // execution time
              ACE_Time_Value::max_time,           // deadline time
              NULL,                               // data block allocator
              NULL)                               // message block allocator
 , type_ (messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::Stream_ControlMessage_T"));

  ACE_ASSERT (intialize (messageType_in));
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::Stream_ControlMessage_T (ACE_Data_Block* dataBlock_in,
                                                                      ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // data block (may be NULL)
              0,                   // pass ownership to base class
              messageAllocator_in) // message block allocator
 , type_ (STREAM_CONTROL_MESSAGE_INVALID)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::Stream_ControlMessage_T"));

  // *WARNING*: need to finalize initialization through initialize()
  inherited::msg_type (ACE_Message_Block::MB_NORMAL);

  // reset read/write pointers
  inherited::reset ();
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::Stream_ControlMessage_T (const Stream_ControlMessage_T<ControlMessageType,
                                                                                                    AllocatorConfigurationType,
                                                                                                    DataMessageType,
                                                                                                    SessionMessageType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , type_ (message_in.type_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::Stream_ControlMessage_T"));

  inherited::msg_type (message_in.msg_type ());

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::~Stream_ControlMessage_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::~Stream_ControlMessage_T"));

  type_ = STREAM_CONTROL_MESSAGE_INVALID;

  // *WARNING*: cannot reset the message type (data block has already gone)
//  inherited::msg_type (ACE_Message_Block::MB_NORMAL);
  // *IMPORTANT NOTE*: this is an ugly hack to support message allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = STREAM_MESSAGE_CONTROL_PRIORITY;
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
bool
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::initialize (const ControlMessageType& messageType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::initialize"));

  type_ = messageType_in;

  // *NOTE*: some control message types have not (yet) been defined in ACE
  //         --> until further notice, assign ACE_Message_Block::MB_NORMAL (0)
  switch (messageType_in)
  {
    case STREAM_CONTROL_MESSAGE_CONNECT:
    case STREAM_CONTROL_MESSAGE_LINK:
    case STREAM_CONTROL_MESSAGE_STEP:
      inherited::msg_type (ACE_Message_Block::MB_NORMAL);
      break;
    default:
      inherited::msg_type (messageType_in);
      break;
  } // end SWITCH

  return true;
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
ControlMessageType
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::type () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::type"));

  return type_;
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
ACE_Message_Block*
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new <Stream_ControlMessage_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_sessionData_Block>

  // if there is no allocator, use the standard new and delete calls
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else
  {
    // *NOTE*: instruct the allocator to return a session message by passing 0
    //         as argument to malloc()
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->malloc (0)),
                             OWN_TYPE_T (*this));
  }
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_ControlMessage_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

template <typename ControlMessageType,
          typename AllocatorConfigurationType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_ControlMessage_T<ControlMessageType,
                        AllocatorConfigurationType,
                        DataMessageType,
                        SessionMessageType>::ControlMessageType2String (ControlMessageType type_in,
                                                                        std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_ControlMessage_T::ControlMessageType2String"));

  // initialize return value(s)
  string_out = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (type_in)
  {
    case STREAM_CONTROL_MESSAGE_CONNECT:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("CONNECT");
      break;
    }
    case STREAM_CONTROL_DISCONNECT:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("DISCONNECT");
      break;
    }
    case STREAM_CONTROL_FLUSH:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("FLUSH");
      break;
    }
    case STREAM_CONTROL_MESSAGE_LINK:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("LINK");
      break;
    }
    case STREAM_CONTROL_MESSAGE_STEP:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("STEP");
      break;
    }
    case STREAM_CONTROL_UNLINK:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("UNLINK");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown control message type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH
}
