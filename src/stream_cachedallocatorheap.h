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

#ifndef STREAM_CACHEDALLOCATORHEAP_H
#define STREAM_CACHEDALLOCATORHEAP_H

#include "ace/Global_Macros.h"
#include "ace/Malloc_T.h"
#include "ace/Synch_Traits.h"

#include "stream_exports.h"
#include "stream_iallocator.h"

class Stream_Export Stream_CachedAllocatorHeap
 : public Stream_IAllocator,
   public ACE_Dynamic_Cached_Allocator<ACE_SYNCH_MUTEX>
{
 public:
  Stream_CachedAllocatorHeap (unsigned int,  // pool size
                              unsigned int); // chunk size
  virtual ~Stream_CachedAllocatorHeap ();

  // implement Stream_IAllocator
  virtual bool block (); // return value: block when full ?

  // *IMPORTANT NOTE*: need to implement these as ACE_Dynamic_Cached_Allocator
  // doesn't implement them as virtual (BUG)
  inline virtual void* malloc (size_t numBytes_in)
  {
    return inherited::malloc (numBytes_in);
  };
  inline virtual void free (void* pointer_in)
  {
    return inherited::free (pointer_in);
  };

  // *NOTE*: these return the amount of allocated (heap) memory...
  virtual size_t cache_depth () const;
  virtual size_t cache_size () const;

 private:
  typedef ACE_Dynamic_Cached_Allocator<ACE_SYNCH_MUTEX> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CachedAllocatorHeap (const Stream_CachedAllocatorHeap&));
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedAllocatorHeap& operator= (const Stream_CachedAllocatorHeap&));

  unsigned long poolSize_;
};

#endif
