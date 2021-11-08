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

#include "common_macros.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_CachedMessageAllocatorHeapBase_T<ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType>::Stream_CachedMessageAllocatorHeapBase_T (unsigned int maximumNumberOfMessages_in)
 : inherited (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                         : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
 , dataBlockAllocator_ (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                   : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
 , controlMessageAllocator_ (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                        : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
 , sessionMessageAllocator_ (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                        : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::Stream_CachedMessageAllocatorHeapBase_T"));

  // sanity check(s)
  if (unlikely (!maximumNumberOfMessages_in))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot allocate unlimited memory, caching %u buffer(s)\n"),
                STREAM_QUEUE_DEFAULT_CACHED_MESSAGES));
}

template <typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocatorHeapBase_T<ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::calloc"));

  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.calloc (sizeof (ACE_Data_Block))));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN(ACE_Data_Block), continuing\n")));
  }
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block, aborting\n")));
    return NULL;
  } // end IF

  ACE_Message_Block* message_block_p = NULL;
  try {
    ACE_NEW_MALLOC_NORETURN (message_block_p,
                             static_cast<ControlMessageType*> (controlMessageAllocator_.malloc (sizeof (ACE_Message_Block))),
                             ControlMessageType (data_block_p,
                                                 this));
  }
  catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Message_Block), continuing\n")));
  }
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate control message, aborting\n")));
    delete data_block_p; data_block_p = NULL;
    return NULL;
  } // end IF
  message_block_p->msg_priority (STREAM_MESSAGE_CONTROL_PRIORITY);

  return message_block_p;
}

template <typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocatorHeapBase_T<ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::malloc"));

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  }
  catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN(ACE_Data_Block), continuing\n")));
  }
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block, aborting\n")));
    return NULL;
  } // end IF

  // *NOTE*: must clean up data block beyond this point !

  // step2: get free message
  ACE_Message_Block* message_block_p = NULL;
  try {
    // allocate memory and perform a placement new by invoking a ctor on the
    // allocated space
    if (bytes_in)
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<DataMessageType*> (inherited::malloc (sizeof (DataMessageType))),
                               DataMessageType (-1,           // session id
                                                data_block_p, // use the data block just allocated
                                                this));       // notify allocator upon destruction
    else
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<SessionMessageType*> (sessionMessageAllocator_.malloc (sizeof (SessionMessageType))),
                               SessionMessageType (-1,           // session id
                                                   data_block_p, // use the data block just allocated
                                                   this));       // notify allocator upon destruction
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN([Session]MessageType(%u), continuing\n"),
                bytes_in));
  }
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate (session) message, aborting\n"),
                bytes_in));
    delete data_block_p; data_block_p = NULL;
    return NULL;
  } // end IF
  message_block_p->msg_priority (bytes_in ? UINT64_MAX : 0);

  // ... and return the result
  // *NOTE*: the caller knows what to expect (either MessageType || SessionMessageType)
  return message_block_p;
}

template <typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CachedMessageAllocatorHeapBase_T<ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::free"));

  // *NOTE*: distinguish between different message types here

  ACE_Message_Block* message_block_p =
      static_cast<ACE_Message_Block*> (handle_in);
  ACE_ASSERT (message_block_p);

  // *WARNING*: cannot access the message type (data block has already gone)
  //switch (message_block_p->msg_type ())
  switch (message_block_p->msg_priority ())
  {
    case 0:
      sessionMessageAllocator_.free (handle_in);
      break;
    case STREAM_MESSAGE_CONTROL_PRIORITY:
      controlMessageAllocator_.free (handle_in);
      break;
    case UINT64_MAX:
      inherited::free (handle_in);
      break;
    default:
    {
      //ACE_DEBUG ((LM_CRITICAL,
      //            ACE_TEXT ("invalid/unknown message type (was: %d), continuing\n"),
      //            message_block_p->msg_type ()));
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("invalid/unknown message priority (was: %d), continuing\n"),
                  message_block_p->msg_priority ()));
      break;
    }
  } // end SWITCH
}

//////////////////////////////////////////

//template <typename MessageType>
//void*
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::calloc (size_t,
//                                                              size_t,
//                                                              char)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::calloc"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return NULL);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::remove (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::remove"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}

//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::bind (const char*, void*, int)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::bind"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::trybind (const char*, void*&)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::trybind"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::find (const char*, void*&)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::find"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::find (const char*)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::find"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::unbind (const char*)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::unbind"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}MESSAGE_T
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::unbind (const char*, void*&)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::unbind"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}

//template <typename MessageType>
//intMESSAGE_T
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::sync (ssize_t, int)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::sync"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::sync (void*, size_t, int)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::sync"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}

//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::protect (ssize_t, int)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::protect"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::protect (void*, size_t, int)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::protect"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}

//////////////////////////////////////////

#if defined (ACE_HAS_MALLOC_STATS)
//template <typename MessageType>
//void
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::print_stats (void) const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::print_stats"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return);
//}
#endif /* ACE_HAS_MALLOC_STATS */

//template <typename MessageType>
//void
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::dump (void) const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::dump"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return);
//}
