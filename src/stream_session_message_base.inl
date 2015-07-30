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

template <typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (Stream_SessionMessageType messageType_in,
                                                                        SessionDataType*& sessionData_inout,
                                                                        UserDataType* userData_in)
 : inherited (0,                                  // size
              MESSAGE_SESSION,                    // type
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
 , messageType_ (messageType_in)
 , sessionData_ (sessionData_inout)
 , userData_ (userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // *NOTE*: assumes responsibility for the handle !
  sessionData_inout = NULL;
}

template <typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message block allocator
 , isInitialized_ (false)
 , messageType_ (STREAM_SESSION_MAP)
 , sessionData_ (NULL)
 , userData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // reset read/write pointers
  inherited::reset ();
}

template <typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                        ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use (don't own (!) memory of-) data block
              0,                   // flags --> also "free" data block in dtor
              messageAllocator_in) // re-use the same allocator
 , isInitialized_ (false)
 , messageType_ (STREAM_SESSION_MAP) // == Stream_MessageBase::MB_STREAM_SESSION
 , sessionData_ (NULL)
 , userData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // set correct message type
  // *WARNING*: need to finalize initialization through initialize()
  inherited::msg_type (MESSAGE_SESSION);

  // reset read/write pointers
  inherited::reset ();
}

template <typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<SessionDataType,
                                                                                                          UserDataType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // make a "shallow" copy of the data block
              0,                                    // "own" the duplicate
              message_in.message_block_allocator_)  // message allocator
 , isInitialized_ (message_in.isInitialized_)
 , messageType_ (message_in.messageType_)
 , sessionData_ (message_in.sessionData_)
 , userData_ (message_in.userData_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // increment reference counter
  // *TODO*: clean this up !
  if (sessionData_)
    sessionData_->increase ();

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::~Stream_SessionMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::~Stream_SessionMessageBase_T"));

  // *IMPORTANT NOTE*: this is an ugly hack to enable some allocators
  //                   (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::min ();

  messageType_ = STREAM_SESSION_MAP; // == Stream_MessageBase::MB_STREAM_SESSION

  if (sessionData_)
    sessionData_->decrease ();

  isInitialized_ = false;
}

template <typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageType
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::type () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::type"));

  return messageType_;
}

template <typename SessionDataType,
          typename UserDataType>
const SessionDataType&
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::get"));

  if (sessionData_)
    return *sessionData_;

  return SessionDataType ();
}

template <typename SessionDataType,
          typename UserDataType>
const UserDataType&
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::data () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::data"));

  if (userData_)
    return *userData_;

  return UserDataType ();
}

template <typename SessionDataType,
          typename UserDataType>
ACE_Message_Block*
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::duplicate"));

  SELF_T* message_p = NULL;

  // create a new <Stream_SessionMessageBase_T> that contains unique copies of
  // the message block fields, but a reference counted duplicate of
  // the <ACE_sessionData_Block>.

  // if there is no allocator, use the standard new and delete calls.
  if (!inherited::message_block_allocator_)
    ACE_NEW_NORETURN (message_p,
                      SELF_T (*this));
  else
  {
    // *NOTE*: instruct the allocator to return a session message by passing 0
    //         as argument to malloc()...
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<SELF_T*> (inherited::message_block_allocator_->malloc (0)),
                             SELF_T (*this));
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

template <typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::initialize (Stream_SessionMessageType messageType_in,
                                                       SessionDataType*& sessionData_inout,
                                                       UserDataType* userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::initialize"));

  ACE_ASSERT (!isInitialized_);
  ACE_ASSERT (messageType_ == STREAM_SESSION_MAP);
  ACE_ASSERT (!sessionData_);
  ACE_ASSERT (!userData_);

  messageType_ = messageType_in;
  // *NOTE*: assumes responsibility for the handle !
  sessionData_ = sessionData_inout;
  sessionData_inout = NULL;
  userData_ = userData_in;

  isInitialized_ = true;
}

template <typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::dump_state"));

  std::string type_string;
  SessionMessageType2String (messageType_,
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

template <typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<SessionDataType,
                            UserDataType>::SessionMessageType2String (Stream_SessionMessageType messageType_in,
                                                                      std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::SessionMessageType2String"));

  // initialize return value(s)
  string_out = ACE_TEXT_ALWAYS_CHAR ("INVALID_TYPE");

  switch (messageType_in)
  {
    case SESSION_BEGIN:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_BEGIN");
      break;
    }
    case SESSION_STEP:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_STEP");
      break;
    }
    case SESSION_END:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_END");
      break;
    }
    case SESSION_STATISTICS:
    {
      string_out = ACE_TEXT_ALWAYS_CHAR ("SESSION_STATISTICS");
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
