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

#ifndef STREAM_CACHEDDATABLOCKALLOCATORHEAP_H
#define STREAM_CACHEDDATABLOCKALLOCATORHEAP_H

#include "ace/Malloc_T.h"
#include "ace/Message_Block.h"
#include "ace/Synch.h"
#include "ace/Lock_Adapter_T.h"

#include "stream_exports.h"
#include "stream_iallocator.h"

class Stream_Export Stream_CachedDataBlockAllocatorHeap
 : public ACE_Cached_Allocator<ACE_Data_Block, ACE_SYNCH_MUTEX>,
   public Stream_IAllocator
{
 public:
  Stream_CachedDataBlockAllocatorHeap (unsigned int,    // number of chunks
                                       ACE_Allocator*); // (heap) memory allocator...
  virtual ~Stream_CachedDataBlockAllocatorHeap ();

  // implement Stream_IAllocator
  virtual bool block (); // return value: block when full ?
  // *NOTE*: returns a pointer to ACE_Data_Block...
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees an ACE_Data_Block...
  virtual void free (void*); // element handle
  virtual size_t cache_depth () const; // return value: #bytes allocated
  virtual size_t cache_size () const;  // return value: #inflight ACE_Data_Blocks

  // *NOTE*: returns a pointer to ACE_Data_Block...
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value

  // convenience types
  // *NOTE*: serialize access to ACE_Data_Block reference count which may
  // be decremented from multiple threads...
  typedef ACE_Lock_Adapter<ACE_Thread_Mutex> DATABLOCK_LOCK_TYPE;

  // locking
  // *NOTE*: currently, ALL data blocks use one static lock (OK for stream usage)...
  // *TODO*: consider using a lock-per-message strategy...
  static DATABLOCK_LOCK_TYPE referenceCountLock_;

 private:
  typedef ACE_Cached_Allocator<ACE_Data_Block, ACE_SYNCH_MUTEX> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CachedDataBlockAllocatorHeap (const Stream_CachedDataBlockAllocatorHeap&));
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedDataBlockAllocatorHeap& operator= (const Stream_CachedDataBlockAllocatorHeap&));

  ACE_Allocator*             heapAllocator_;
  unsigned int               poolSize_;
};

#endif
