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

#include "stream_macros.h"

// init statics
Stream_CachedDataBlockAllocatorHeap::DATABLOCK_LOCK_TYPE Stream_CachedDataBlockAllocatorHeap::referenceCountLock_;

Stream_CachedDataBlockAllocatorHeap::Stream_CachedDataBlockAllocatorHeap (unsigned int chunks_in,
                                                                          ACE_Allocator* allocator_in)
 : inherited (chunks_in)
 , heapAllocator_ (allocator_in)
 , poolSize_ (chunks_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::Stream_CachedDataBlockAllocatorHeap"));

  // *NOTE*: NULL --> use heap (== default allocator !)
  if (!heapAllocator_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default (== heap) message buffer allocation strategy...\n")));
  } // end IF
}

Stream_CachedDataBlockAllocatorHeap::~Stream_CachedDataBlockAllocatorHeap ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::~Stream_CachedDataBlockAllocatorHeap"));

}

void*
Stream_CachedDataBlockAllocatorHeap::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::malloc"));

  ACE_Data_Block* data_block = NULL;
  try
  {
    // - delegate allocation to our base class and
    // - perform a placement new by invoking a ctor on the allocated space
    // --> perform necessary initialization...
    ACE_NEW_MALLOC_RETURN (data_block,
                           static_cast<ACE_Data_Block*> (inherited::malloc (sizeof (ACE_Data_Block))),
                           ACE_Data_Block (bytes_in,                                 // size of data chunk
                                           ACE_Message_Block::MB_DATA,               // message type
                                           NULL,                                     // data --> use allocator !
                                           heapAllocator_,                           // allocator
                                           //NULL,                                   // no allocator --> allocate this off the heap !
                                           &Stream_CachedDataBlockAllocatorHeap::referenceCountLock_, // reference count lock
                                           0,                                        // flags: release our (heap) memory when we die
                                           this),                                    // remember us upon destruction...
                           NULL);
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_RETURN(ACE_Data_Block(%u)), aborting\n"),
                bytes_in));

    // *TODO*: what else can we do ?
    return NULL;
  }

  return data_block;
}

void*
Stream_CachedDataBlockAllocatorHeap::calloc (size_t bytes_in,
                                             char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  // just delegate this (for now)...
  return malloc (bytes_in);
}

void
Stream_CachedDataBlockAllocatorHeap::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap::free"));

  inherited::free (handle_in);
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
