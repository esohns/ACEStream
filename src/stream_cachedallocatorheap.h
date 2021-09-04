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

#ifndef Stream_CachedAllocatorHeap_T_H
#define Stream_CachedAllocatorHeap_T_H

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Malloc_T.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_allocatorbase.h"

template <typename ConfigurationType>
class Stream_CachedAllocatorHeap_T
 : public Stream_AllocatorBase_T<ConfigurationType>
 , public ACE_Dynamic_Cached_Allocator<ACE_SYNCH_MUTEX>
{
  typedef Stream_AllocatorBase_T<ConfigurationType> inherited;
  typedef ACE_Dynamic_Cached_Allocator<ACE_SYNCH_MUTEX> inherited2;

 public:
  Stream_CachedAllocatorHeap_T (unsigned int,  // pool size
                                unsigned int); // chunk size
  virtual ~Stream_CachedAllocatorHeap_T ();

  // implement Stream_IAllocator
  inline virtual bool block () { return true; }
  inline virtual void* calloc () { return inherited2::calloc (sizeof (ACE_Message_Block), '\0'); }
  // *IMPORTANT NOTE*: need to implement these as ACE_Dynamic_Cached_Allocator
  // doesn't implement them as virtual (BUG)
  inline virtual void* malloc (size_t bytes_in) { return inherited2::malloc (bytes_in); }
  inline virtual void free (void* address_in) { inherited2::free (address_in); }

  // *NOTE*: these return the amount of allocated (heap) memory
  virtual size_t cache_depth () const;
  virtual size_t cache_size () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  // convenient types
  typedef Stream_CachedAllocatorHeap_T<ConfigurationType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_CachedAllocatorHeap_T (const Stream_CachedAllocatorHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedAllocatorHeap_T& operator= (const Stream_CachedAllocatorHeap_T&))

  ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> poolSize_;
};

// include template definition
#include "stream_cachedallocatorheap.inl"

#endif
