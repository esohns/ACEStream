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

#include "stream_macros.h"
#include "stream_message_base.h"

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (Stream_SessionMessageType messageType_in,
                                                                        SessionDataType*& sessionData_inout,
                                                                        UserDataType* userData_in)
 : inherited (0,                                  // size
              STREAM_SESSION_MESSAGE_MAP,         // type
              NULL,                               // continuation
              NULL,                               // data
              NULL,                               // buffer allocator
              NULL,                               // locking strategy
              ACE_DEFAULT_MESSAGE_BLOCK_PRIORITY, // priority
              ACE_Time_Value::zero,               // execution time
              ACE_Time_Value::max_time,           // deadline time
              NULL,                               // data block allocator
              NULL)                               // message block allocator
 , isInitialized_ (true)
 , sessionData_ (sessionData_inout)
 , type_ (messageType_in)
 , userData_ (userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // *NOTE*: assumes responsibility for the handle !
  sessionData_inout = NULL;

//  if (sessionData_)
//    sessionData_->increase ();
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , isInitialized_ (false)
 , sessionData_ (NULL)
 , type_ (STREAM_SESSION_INVALID)
 , userData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: need to finalize initialization through initialize()
  inherited::msg_type (STREAM_SESSION_MESSAGE_MAP);

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                        ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) data block
              0,                   // flags --> also "free" data block in dtor
              messageAllocator_in) // re-use the same allocator
 , isInitialized_ (false)
 , sessionData_ (NULL)
 , type_ (STREAM_SESSION_INVALID)
 , userData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: need to finalize initialization through initialize()
  inherited::msg_type (STREAM_SESSION_MESSAGE_MAP);

  // reset read/write pointers
  inherited::reset ();
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<AllocatorConfigurationType, 
                                                                                                          SessionDataType,
                                                                                                          UserDataType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , isInitialized_ (message_in.isInitialized_)
 , sessionData_ (message_in.sessionData_)
 , type_ (message_in.type_)
 , userData_ (message_in.userData_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // increment reference counter
  // *TODO*: clean this up !
  if (sessionData_)
    sessionData_->increase ();

  inherited::msg_type (STREAM_SESSION_MESSAGE_MAP);

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::~Stream_SessionMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::~Stream_SessionMessageBase_T"));

  isInitialized_ = false;
  if (sessionData_)
  {
    sessionData_->decrease ();
    sessionData_ = NULL;
  } // end IF
  type_ = STREAM_SESSION_INVALID;
  userData_ = NULL;

//  // *WARNING*: cannot do that anymore (data block has already gone away)...
//  inherited::msg_type (STREAM_SESSION_MESSAGE_MAP);

  // *IMPORTANT NOTE*: this is an ugly hack to enable some allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::min ();
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageType
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::type () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::type"));

  return type_;
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
const SessionDataType&
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::get"));

  if (sessionData_)
    return *sessionData_;

  ACE_ASSERT (false);
  return SessionDataType ();
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
const UserDataType&
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::data () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::data"));

  if (userData_)
    return *userData_;

  ACE_ASSERT (false);
  return UserDataType ();
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
ACE_Message_Block*
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new <Stream_SessionMessageBase_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_sessionData_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else
  {
    // *NOTE*: instruct the allocator to return a session message by passing 0
    //         as argument to malloc()...
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
                  ACE_TEXT ("failed to allocate Stream_SessionMessageBase_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::initialize (Stream_SessionMessageType messageType_in,
                                                       SessionDataType*& sessionData_inout,
                                                       UserDataType* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::initialize"));

  ACE_ASSERT (!isInitialized_);
  ACE_ASSERT (!sessionData_);
  ACE_ASSERT (!userData_);

  isInitialized_ = true;
  sessionData_ = sessionData_inout;
  type_ = messageType_in;
  userData_ = userData_in;

  inherited::msg_type (STREAM_SESSION_MESSAGE_MAP);

  // *NOTE*: assumes responsibility for the handle !
  sessionData_inout = NULL;
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::dump_state"));

  std::string type_string;
  SessionMessageType2String (type_,
                             type_string);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("session message type: \"%s\"\n"),
              ACE_TEXT (type_string.c_str ())));

  if (sessionData_)
  {
    try
    {
      sessionData_->dump_state ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in SessionDataType::dump_state(), continuing")));
    }
  } // end IF
}

template <typename AllocatorConfigurationType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<AllocatorConfigurationType,
                            SessionDataType,
                            UserDataType>::SessionMessageType2String (ACE_Message_Type messageType_in,
                                                                      std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::SessionMessageType2String"));

  // initialize return value(s)
  string_out = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (messageType_in)
  {
    // *** control ***
    case STREAM_SESSION_BEGIN:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_BEGIN");
      break;
    }
    case STREAM_SESSION_STEP:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_STEP");
      break;
    }
    case STREAM_SESSION_END:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_END");
      break;
    }
    // *** data ***
    case STREAM_SESSION_STATISTIC:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_STATISTIC");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message type: \"%u\", aborting\n"),
                  messageType_in));
      break;
    }
  } // end SWITCH
}
