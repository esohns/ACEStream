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
#include "stdafx.h"

#include "stream_cacheddatablockallocatorheap.h"

#include "stream_defines.h"
#include "stream_macros.h"

// initialize statics
Stream_CachedDataBlockAllocatorHeap::DATABLOCK_LOCK_TYPE
Stream_CachedDataBlockAllocatorHeap::referenceCountLock_;

Stream_CachedDataBlockAllocatorHeap::Stream_CachedDataBlockAllocatorHeap (unsigned int chunks_in,
                                                                          ACE_Allocator* allocator_in)
 : inherited ((chunks_in == 0) ? STREAM_QUEUE_DEFAULT_CACHED_MESSAGES : chunks_in)
 , heapAllocator_ (allocator_in)
 , poolSize_ ((chunks_in == 0) ? STREAM_QUEUE_DEFAULT_CACHED_MESSAGES : chunks_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::Stream_CachedDataBlockAllocatorHeap"));

  // sanity check(s)
  if (!chunks_in)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot allocate unlimited memory, caching %d buffers...\n"),
                STREAM_QUEUE_DEFAULT_CACHED_MESSAGES));

  // *NOTE*: NULL --> use heap (== default allocator !)
  if (!heapAllocator_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default (== heap) message buffer allocation strategy...\n")));
}

Stream_CachedDataBlockAllocatorHeap::~Stream_CachedDataBlockAllocatorHeap ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::~Stream_CachedDataBlockAllocatorHeap"));

}

void*
Stream_CachedDataBlockAllocatorHeap::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::calloc"));

  ACE_Data_Block* data_block_p = NULL;
  try {
    // delegate allocation to the base class and:
    // - use placement new to invoke a ctor on the allocated space
    // - perform necessary initialization...
    ACE_NEW_MALLOC_NORETURN (data_block_p,
                             static_cast<ACE_Data_Block*> (inherited::calloc (sizeof (ACE_Data_Block))),
                             ACE_Data_Block (0,                                                         // size of data chunk
                                             ACE_Message_Block::MB_NORMAL,                              // message type
                                             NULL,                                                      // data --> use allocator !
                                             NULL,                                                      // allocator
                                             &Stream_CachedDataBlockAllocatorHeap::referenceCountLock_, // reference count lock
                                             0,                                                         // flags: release (heap) memory in dtor
                                             this));                                                    // data block allocator
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Data_Block()), continuing\n")));
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ACE_Data_Block(), aborting\n")));
    return NULL;
  } // end IF

  return data_block_p;
}

void*
Stream_CachedDataBlockAllocatorHeap::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::malloc"));

  ACE_Data_Block* data_block_p = NULL;
  try {
    // delegate allocation to the base class and:
    // - use placement new to invoke a ctor on the allocated space
    // - perform necessary initialization...
    ACE_NEW_MALLOC_NORETURN (data_block_p,
                             static_cast<ACE_Data_Block*> (inherited::malloc (sizeof (ACE_Data_Block))),
                             ACE_Data_Block (bytes_in,                                                  // size of data chunk
                                             (bytes_in ? ACE_Message_Block::MB_DATA : ACE_Message_Block::MB_USER), // message type
                                             NULL,                                                      // data --> use allocator !
                                             (bytes_in ? heapAllocator_ : NULL),                        // allocator
                                             &Stream_CachedDataBlockAllocatorHeap::referenceCountLock_, // reference count lock
                                             0,                                                         // flags: release (heap) memory in dtor
                                             this));                                                    // data block allocator
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Data_Block(%u)), continuing\n"),
                bytes_in));
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    return NULL;
  } // end IF

  return data_block_p;
}

size_t
Stream_CachedDataBlockAllocatorHeap::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::cache_depth"));

  return const_cast<Stream_CachedDataBlockAllocatorHeap*> (this)->pool_depth ();
}

size_t
Stream_CachedDataBlockAllocatorHeap::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::cache_size"));

  return poolSize_;
}
