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

#include "stream_macros.h"

template <typename MessageType,
          typename SessionMessageType>
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::Stream_CachedMessageAllocatorHeapBase_T (unsigned int maxNumMessages_in,
                                                                                                      ACE_Allocator* allocator_in)
 : dataBlockAllocator_ (maxNumMessages_in,
                        allocator_in)
 , messageAllocator_ (maxNumMessages_in)
 , sessionMessageAllocator_ (maxNumMessages_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::Stream_CachedMessageAllocatorHeapBase_T"));

}

template <typename MessageType,
          typename SessionMessageType>
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::~Stream_CachedMessageAllocatorHeapBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::~Stream_CachedMessageAllocatorHeapBase_T"));

}

template <typename MessageType,
          typename SessionMessageType>
bool
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::block"));

  return dataBlockAllocator_.block ();
}

template <typename MessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::malloc"));

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try
  {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_RETURN(%u), aborting\n"),
                bytes_in));
    return NULL;
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    return NULL;
  }

  // *NOTE*: must clean up data block beyond this point !

  // step2: get free message...
  ACE_Message_Block* message_p = NULL;
  try
  {
    // allocate memory and perform a placement new by invoking a ctor
    // on the allocated space
    if (bytes_in)
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<MessageType*> (messageAllocator_.malloc (sizeof (MessageType))),
                               MessageType (data_block_p,         // use the data block we just allocated
                                            &messageAllocator_)); // remember allocator upon destruction...
    else
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<SessionMessageType*> (sessionMessageAllocator_.malloc (sizeof (SessionMessageType))),
                               SessionMessageType (data_block_p,                // use the data block we just allocated
                                                   &sessionMessageAllocator_)); // remember allocator upon destruction...
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN([Session]MessageType(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate [Session]MessageType(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // ... and return the result
  // *NOTE*: the caller knows what to expect (either MessageType || SessionMessageType)
  return message_p;
}

template <typename MessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::calloc (size_t bytes_in,
                                                                     char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::calloc"));

  // ignore this
  ACE_UNUSED_ARG (initialValue_in);

  // just delegate this...
  return malloc (bytes_in);
}

template <typename MessageType,
          typename SessionMessageType>
void
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::free"));

  // *NOTE*: messages should contact their individual allocators !!!
  ACE_ASSERT (false);
}

template <typename MessageType,
          typename SessionMessageType>
size_t
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::cache_depth"));

  return dataBlockAllocator_.cache_depth ();
}

template <typename MessageType,
          typename SessionMessageType>
size_t
Stream_CachedMessageAllocatorHeapBase_T<MessageType,
                                        SessionMessageType>::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeapBase_T::cache_size"));

  return dataBlockAllocator_.cache_size ();
}
