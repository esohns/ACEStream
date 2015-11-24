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
#include "ace/Synch_Traits.h"

#include "stream_allocatorbase.h"

template <typename ConfigurationType>
class Stream_CachedAllocatorHeap_T
 : public Stream_AllocatorBase_T<ConfigurationType>
 , public ACE_Dynamic_Cached_Allocator<ACE_SYNCH_MUTEX>
{
 public:
  Stream_CachedAllocatorHeap_T (unsigned int,  // pool size
                                unsigned int); // chunk size
  virtual ~Stream_CachedAllocatorHeap_T ();

  // implement Stream_IAllocator
  virtual bool block (); // return value: block when full ?

  // *IMPORTANT NOTE*: need to implement these as ACE_Dynamic_Cached_Allocator
  // doesn't implement them as virtual (BUG)
  inline virtual void* malloc (size_t bytes_in)
  {
    // *TODO*: remove type inference
    size_t number_of_bytes = bytes_in + inherited::configuration_.buffer;
    return inherited2::malloc (number_of_bytes);
  };
  inline virtual void free (void* pointer_in)
  {
    return inherited2::free (pointer_in);
  };

  // *NOTE*: these return the amount of allocated (heap) memory...
  virtual size_t cache_depth () const;
  virtual size_t cache_size () const;

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  typedef Stream_AllocatorBase_T<ConfigurationType> inherited;
  typedef ACE_Dynamic_Cached_Allocator<ACE_SYNCH_MUTEX> inherited2;

  // convenient types
  typedef Stream_CachedAllocatorHeap_T<ConfigurationType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_CachedAllocatorHeap_T (const Stream_CachedAllocatorHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedAllocatorHeap_T& operator= (const Stream_CachedAllocatorHeap_T&))

  ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> poolSize_;
};

// include template implementation
#include "stream_cachedallocatorheap.inl"

#endif
