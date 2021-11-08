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

#ifndef Stream_CachedDataBlockAllocatorHeap_T_T_H
#define Stream_CachedDataBlockAllocatorHeap_T_T_H

#include "ace/Global_Macros.h"
#include "ace/Lock_Adapter_T.h"
#include "ace/Malloc_Base.h"
#include "ace/Malloc_T.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

//#include "stream_exports.h"
#include "stream_iallocator.h"

template <ACE_SYNCH_DECL>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class Stream_CachedDataBlockAllocatorHeap_T
#else
class Stream_CachedDataBlockAllocatorHeap_T
#endif
 : public ACE_Cached_Allocator<ACE_Data_Block,
                               ACE_SYNCH_MUTEX_T>
 , public Stream_IAllocator
{
  typedef ACE_Cached_Allocator<ACE_Data_Block,
                               ACE_SYNCH_MUTEX_T> inherited;

 public:
  Stream_CachedDataBlockAllocatorHeap_T (unsigned int,   // number of chunks
                                         ACE_Allocator*, // (heap) memory allocator
                                         bool = true);   // block until a buffer is available ?
  inline virtual ~Stream_CachedDataBlockAllocatorHeap_T () {}

  // implement (part of) Stream_IAllocator
  // *IMPORTANT NOTE*: whatever is passed into the ctors' 3rd argument, this
  //                   NEVER blocks; elements are allocated dynamically when lwm
  //                   is reached (see: ACE_Locked_Free_List)
  inline virtual bool block () { return false; }
  // *NOTE*: returns a pointer to ACE_Data_Block
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees an ACE_Data_Block
  inline virtual void free (void* handle_in) { inherited::free (handle_in); } // handle
  inline virtual size_t cache_depth () const { return const_cast<OWN_TYPE_T*> (this)->pool_depth (); }
  inline virtual size_t cache_size () const { return poolSize_; }

  // *NOTE*: returns a pointer to ACE_Data_Block
  inline virtual void* calloc (size_t bytes_in,
                               char = '\0') { return malloc (bytes_in); }

  // convenience types
  // *NOTE*: serialize access to ACE_Data_Block reference count, which may be
  //         decremented from multiple threads
  typedef ACE_Lock_Adapter<ACE_SYNCH_MUTEX_T> DATABLOCK_LOCK_T;

  // locking
  // *NOTE*: currently, ALL data blocks use one static lock, this is OK for most
  //         scenarios)
  // *TODO*: implement lock-per-message/session strategies
  static DATABLOCK_LOCK_T referenceCountLock_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedDataBlockAllocatorHeap_T (const Stream_CachedDataBlockAllocatorHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedDataBlockAllocatorHeap_T& operator= (const Stream_CachedDataBlockAllocatorHeap_T&))

  // convenient types
  typedef Stream_CachedDataBlockAllocatorHeap_T<ACE_SYNCH_USE> OWN_TYPE_T;

  // implement (part of) Stream_IAllocator
  virtual void* calloc ();

  bool           block_;
  ACE_Allocator* heapAllocator_;
  unsigned int   poolSize_;
};

// include template definition
#include "stream_cacheddatablockallocatorheap.inl"

#endif
