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

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/Malloc_Base.h"
#include "ace/Time_Value.h"

#include "common_macros.h"

#include "stream_iallocator.h"
#include "stream_macros.h"

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                        SessionMessageType messageType_in,
                                                                        SessionDataType*& data_inout,
                                                                        UserDataType* userData_in,
                                                                        bool expedited_in)
 : inherited (0,                                  // size
              ACE_Message_Block::MB_EVENT,        // type
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
 , expedited_ (expedited_in)
 , isInitialized_ (!!data_inout)
 , sessionId_ (sessionId_in)
 , type_ (messageType_in)
 , userData_ (userData_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // initialize return value(s)
  data_inout = NULL;
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                        ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // message allocator
 , data_ (NULL)
 , expedited_ (false)
 , isInitialized_ (false)
 , sessionId_ (sessionId_in)
 , type_ (STREAM_SESSION_MESSAGE_INVALID)
 , userData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // *WARNING*: need to finalize initialization through initialize()

  // reset read/write pointers
  inherited::reset ();
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                        ACE_Data_Block* dataBlock_in,
                                                                        ACE_Allocator* messageAllocator_in)
 : inherited (dataBlock_in,        // use ((; don't necessarily own-) memory of-) data block
              0,                   // flags --> 'release' data block in dtor
              messageAllocator_in) // message allocator
 , data_ (NULL)
 , expedited_ (false)
 , isInitialized_ (false)
 , sessionId_ (sessionId_in)
 , type_ (STREAM_SESSION_MESSAGE_INVALID)
 , userData_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // *WARNING*: need to finalize initialization through initialize()

  // reset read/write pointers
  inherited::reset ();
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::Stream_SessionMessageBase_T (const Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                                                                                                          SessionMessageType,
                                                                                                          SessionDataType,
                                                                                                          UserDataType>& message_in)
 : inherited (message_in.data_block_->duplicate (), // 'shallow'-copy the data block
              0,                                    // flags --> 'release' data block in dtor
              message_in.message_block_allocator_)  // reuse message allocator
 , data_ (message_in.data_)
 , expedited_ (message_in.expedited_)
 , isInitialized_ (message_in.isInitialized_)
 , sessionId_ (message_in.sessionId_)
 , type_ (message_in.type_)
 , userData_ (message_in.userData_)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::Stream_SessionMessageBase_T"));

  // increment reference counter
  // *TODO*: clean this up
  if (data_)
    data_->increase ();

  // set read/write pointers
  inherited::rd_ptr (message_in.rd_ptr ());
  inherited::wr_ptr (message_in.wr_ptr ());
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::~Stream_SessionMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::~Stream_SessionMessageBase_T"));

  if (likely (data_))
  {
    data_->decrease (); data_ = NULL;
  } // end IF
  expedited_ = false;
  isInitialized_ = false;
  sessionId_ = 0;
  type_ = STREAM_SESSION_MESSAGE_INVALID;
  userData_ = NULL;

  // *WARNING*: cannot reset the message type (data block has already gone)
//  inherited::msg_type (STREAM_MESSAGE_SESSION);
  // *WORKAROUND*: this is an ugly hack to support message allocators
  //               (see e.g. stream_cachedmessageallocator.cpp:172)
  inherited::priority_ = std::numeric_limits<unsigned long>::min ();
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
const SessionDataType&
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::getR () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::getR"));

  if (likely (data_))
    return *data_;

  static typename SessionDataType::DATA_T* data_p = NULL;
  static SessionDataType dummy (data_p);
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (dummy);
  ACE_NOTREACHED (return dummy;)
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::setP (SessionDataType* data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::setP"));

  // sanity check(s)
  ACE_ASSERT (isInitialized_);
  ACE_ASSERT (data_in);

  if (likely (data_))
  {
    const typename SessionDataType::DATA_T& session_data_r =
        data_->getR ();
    typename SessionDataType::DATA_T& session_data_2 =
        const_cast<typename SessionDataType::DATA_T&> (data_in->getR ());
    ACE_ASSERT (session_data_r.lock && session_data_2.lock);
    int result = -1;
    bool release_lock_b = false;
//    ACE_ASSERT (session_data_r.lock != session_data_2.lock);
    { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *session_data_r.lock);
//      ACE_GUARD (ACE_SYNCH_MUTEX, aGuard_2, *session_data_2.lock);
      if (session_data_r.lock != session_data_2.lock)
      {
        result = session_data_2.lock->acquire ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", continuing\n")));
        else
          release_lock_b = true;
      } // end IF
      session_data_2 += session_data_r;
      if (likely (release_lock_b))
      {
        result = session_data_2.lock->release ();
        if (unlikely (result == -1))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
      } // end IF
    } // end lock scope
    data_->decrease ();
  } // end IF

  data_ = data_in;
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
ACE_Message_Block*
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new <Stream_SessionMessageBase_T> that contains unique copies of
  // the message block fields, but a 'shallow' duplicate of the <ACE_Data_Block>

  // if there is no message allocator, use the standard new and delete calls
retry:
  if (likely (inherited::message_block_allocator_))
  {
    // *NOTE*: instruct the allocator to return a session message by passing 0
    //         as argument to malloc()
    // *NOTE*: 'placement new' invokes the copy ctor on the allocated memory
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->malloc (0)),
                             OWN_TYPE_T (*this));
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  if (unlikely (!message_p))
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    if (allocator_p && !allocator_p->block ())
      goto retry;
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return NULL;
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::initialize (Stream_SessionId_t sessionId_in,
                                                       SessionMessageType messageType_in,
                                                       SessionDataType*& data_inout,
                                                       UserDataType* userData_in,
                                                       bool expedited_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::initialize"));

  if (isInitialized_)
  {
    if (likely (data_))
    {
      data_->decrease (); data_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  if (likely (data_inout))
  {
    data_ = data_inout;
    data_inout = NULL;
  } // end IF
  expedited_ = expedited_in;
  isInitialized_ = true;
  sessionId_ = sessionId_in;
  type_ = messageType_in;
  userData_ = userData_in;

//  inherited::msg_type (STREAM_MESSAGE_SESSION);
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::dump_state"));

  std::string type_string;
  OWN_TYPE_T::MessageTypeToString (type_,
                                   type_string);
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("session message type: \"%s\"\n"),
              ACE_TEXT (type_string.c_str ())));
  if (likely (data_))
  {
    try {
      data_->dump_state ();
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in SessionDataType::dump_state(), continuing")));
    }
  } // end IF
}

template <//typename AllocatorConfigurationType,
          typename SessionMessageType,
          typename SessionDataType,
          typename UserDataType>
void
Stream_SessionMessageBase_T<//AllocatorConfigurationType,
                            SessionMessageType,
                            SessionDataType,
                            UserDataType>::MessageTypeToString (SessionMessageType type_in,
                                                                std::string& string_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_SessionMessageBase_T::MessageTypeToString"));

  // initialize return value(s)
  string_out = ACE_TEXT_ALWAYS_CHAR ("INVALID");

  switch (type_in)
  {
    // *** notification ***
    case STREAM_SESSION_MESSAGE_ABORT:
      string_out = ACE_TEXT_ALWAYS_CHAR ("ABORT"); break;
    case STREAM_SESSION_MESSAGE_CONNECT:
      string_out = ACE_TEXT_ALWAYS_CHAR ("CONNECT"); break;
    case STREAM_SESSION_MESSAGE_DISCONNECT:
      string_out = ACE_TEXT_ALWAYS_CHAR ("DISCONNECT"); break;
    case STREAM_SESSION_MESSAGE_LINK:
      string_out = ACE_TEXT_ALWAYS_CHAR ("LINK"); break;
    case STREAM_SESSION_MESSAGE_UNLINK:
      string_out = ACE_TEXT_ALWAYS_CHAR ("UNLINK"); break;
    // *** control ***
    case STREAM_SESSION_MESSAGE_BEGIN:
      string_out = ACE_TEXT_ALWAYS_CHAR ("BEGIN"); break;
    case STREAM_SESSION_MESSAGE_END:
      string_out = ACE_TEXT_ALWAYS_CHAR ("END"); break;
    case STREAM_SESSION_MESSAGE_STEP:
      string_out = ACE_TEXT_ALWAYS_CHAR ("STEP"); break;
    // *** data ***
    case STREAM_SESSION_MESSAGE_STATISTIC:
      string_out = ACE_TEXT_ALWAYS_CHAR ("STATISTIC"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown session message type (was: %d), aborting\n"),
                  type_in));
      break;
    }
  } // end SWITCH
}
