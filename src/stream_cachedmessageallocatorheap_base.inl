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

#include "stream_defines.h"
#include "stream_macros.h"

template <typename MessageType>
Stream_CachedMessageAllocatorHeapBase_T<MessageType>::Stream_CachedMessageAllocatorHeapBase_T (unsigned int maxNumMessages_in)
 : inherited ((maxNumMessages_in == 0) ? STREAM_QUEUE_DEFAULT_CACHED_MESSAGES
                                       : maxNumMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::Stream_CachedMessageAllocatorHeapBase_T"));

  // sanity check(s)
  if (!maxNumMessages_in)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot allocate unlimited memory, caching %d buffers...\n"),
                STREAM_QUEUE_DEFAULT_CACHED_MESSAGES));
}

template <typename MessageType>
Stream_CachedMessageAllocatorHeapBase_T<MessageType>::~Stream_CachedMessageAllocatorHeapBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::~Stream_CachedMessageAllocatorHeapBase_T"));

}

template <typename MessageType>
bool
Stream_CachedMessageAllocatorHeapBase_T<MessageType>::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::block"));

  return false;
}

//template <typename MessageType>
//void*
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::malloc (size_t bytes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::malloc"));

//  // step1: get free data block
//  ACE_Data_Block* data_block_p = NULL;
//  try
//  {
//    ACE_ALLOCATOR_NORETURN (data_block_p,
//                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("caught exception in ACE_ALLOCATOR_RETURN(%u), aborting\n"),
//                bytes_in));
//    return NULL;
//  }
//  if (!data_block_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("unable to allocate ACE_Data_Block(%u), aborting\n"),
//                bytes_in));
//    return NULL;
//  }

//  // *NOTE*: must clean up data block beyond this point !

//  // step2: get free message...
//  ACE_Message_Block* message_p = NULL;
//  try
//  {
//    // allocate memory and perform a placement new by invoking a ctor
//    // on the allocated space
//    if (bytes_in)
//      ACE_NEW_MALLOC_NORETURN (message_p,
//                               static_cast<MessageType*> (messageAllocator_.malloc (sizeof (MessageType))),
//                               MessageType (data_block_p, // use the data block just allocated
//                                            this));       // remember allocator upon destruction...
//    else
//      ACE_NEW_MALLOC_NORETURN (message_p,
//                               static_cast<SessionMessageType*> (sessionMessageAllocator_.malloc (sizeof (SessionMessageType))),
//                               SessionMessageType (data_block_p, // use the data block just allocated
//                                                   this));       // remember allocator upon destruction...
//  }
//  catch (...)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN([Session]MessageType(%u), aborting\n"),
//                bytes_in));

//    // clean up
//    data_block_p->release ();

//    return NULL;
//  }
//  if (!message_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("unable to allocate [Session]MessageType(%u), aborting\n"),
//                bytes_in));

//    // clean up
//    data_block_p->release ();

//    return NULL;
//  } // end IF

//  // ... and return the result
//  // *NOTE*: the caller knows what to expect (either MessageType || SessionMessageType)
//  return message_p;
//}

//template <typename MessageType>
//void*
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::calloc (size_t bytes_in,
//                                                              char initialValue_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::calloc"));

//  ACE_UNUSED_ARG (initialValue_in);

//  // just delegate this...
//  return malloc (bytes_in);
//}

//template <typename MessageType>
//void
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::free (void* handle_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::free"));

//}

//template <typename MessageType>
//size_t
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::cache_depth () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::cache_depth"));

//  return dataBlockAllocator_.cache_depth ();
//}

//template <typename MessageType>
//size_t
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::cache_size () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::cache_size"));

//  return dataBlockAllocator_.cache_size ();
//}

/////////////////////////////////////////

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
//}
//template <typename MessageType>
//int
//Stream_CachedMessageAllocatorHeapBase_T<MessageType>::unbind (const char*, void*&)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::unbind"));

//  ACE_ASSERT (false);
//  ACE_NOTREACHED (return -1);
//}

//template <typename MessageType>
//int
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

/////////////////////////////////////////

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
