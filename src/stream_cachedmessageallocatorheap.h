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

#ifndef Stream_CachedMessageAllocatorHeap_T_H
#define Stream_CachedMessageAllocatorHeap_T_H

#include <ace/Malloc_T.h>
#include <ace/Message_Block.h>
#include <ace/Synch_Traits.h>

#include "stream_iallocator.h"
#include "stream_cacheddatablockallocatorheap.h"

template <ACE_SYNCH_DECL>
class Stream_CachedMessageAllocatorHeap_T
 : public ACE_Cached_Allocator<ACE_Message_Block,
                               ACE_SYNCH_MUTEX_T>
 , public Stream_IAllocator
{
 public:
  Stream_CachedMessageAllocatorHeap_T (unsigned int,   // total number of chunks
                                       ACE_Allocator*, // (heap) memory allocator
                                       bool = true);   // block until a buffer is available ?
  virtual ~Stream_CachedMessageAllocatorHeap_T ();

  // implement Stream_IAllocator
  // *IMPORTANT NOTE*: whatever is passed into the ctors' 3rd argument, this
  //                   NEVER blocks; elements are allocated dynamically when lwm
  //                   is reached (see: ACE_Locked_Free_List)
  inline virtual bool block () { return false; };
  virtual void* calloc ();
  // *NOTE*: returns a pointer to ACE_Message_Block
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees an ACE_Message_Block
  virtual void free (void*); // handle
  // *NOTE*: these return the # of online ACE_Message_Blocks
  virtual size_t cache_depth () const;
  virtual size_t cache_size () const;

  // overload ACE_Allocator
  // *NOTE*: returns a pointer to ACE_Message_Block
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value

 private:
  typedef ACE_Cached_Allocator<ACE_Message_Block,
                               ACE_SYNCH_MUTEX_T> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocatorHeap_T (const Stream_CachedMessageAllocatorHeap_T&));
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocatorHeap_T& operator= (const Stream_CachedMessageAllocatorHeap_T&));

  bool                                                 block_;
  Stream_CachedDataBlockAllocatorHeap_T<ACE_SYNCH_USE> dataBlockAllocator_;
};

// include template definition
#include "stream_cachedmessageallocatorheap.inl"

#endif
