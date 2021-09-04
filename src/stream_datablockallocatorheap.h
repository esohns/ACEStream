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

#ifndef STREAM_DATABLOCKALLOCATORHEAP_T_H
#define STREAM_DATABLOCKALLOCATORHEAP_T_H

#include "ace/Atomic_Op.h"
#include "ace/Lock_Adapter_T.h"
#include "ace/Malloc_Allocator.h"
#include "ace/Synch_Traits.h"

#include "common_idumpstate.h"

#include "stream_allocatorheap.h"
#include "stream_iallocator.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
class Stream_DataBlockAllocatorHeap_T
 : public ACE_New_Allocator
 , public Stream_IAllocator
 , public Common_IDumpState
{
  typedef ACE_New_Allocator inherited;

 public:
  // convenient types
  typedef Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                                 ConfigurationType> HEAP_ALLOCATOR_T;
  // *NOTE*: serialize access to ACE_Data_Block reference counts, which may
  //         be modified concurrently by multiple threads
  typedef ACE_Lock_Adapter<ACE_SYNCH_MUTEX> DATABLOCK_LOCK_T;

  Stream_DataBlockAllocatorHeap_T (HEAP_ALLOCATOR_T*); // (heap) memory allocator
  inline virtual ~Stream_DataBlockAllocatorHeap_T () {}

  // implement Stream_IAllocator
  inline virtual bool block () { return true; };
  virtual void* calloc ();
  // *NOTE*: returns a pointer to ACE_Data_Block
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees an ACE_Data_Block
  virtual void free (void*); // element handle
  virtual size_t cache_depth () const; // return value: #bytes allocated
  inline virtual size_t cache_size () const { return poolSize_.value (); }; // return value: #inflight ACE_Data_Blocks

  // implement (part of) ACE_Allocator
  inline virtual void* calloc (size_t bytes_in,
                               char = '\0') { return malloc (bytes_in); };

  // implement Common_IDumpState
  virtual void dump_state () const;

  // locking
  // *NOTE*: currently, ALL data blocks use one static lock (OK for single-
  //         streamed scenarios)
  // *TODO*: consider using a lock-per-session strategy
  static DATABLOCK_LOCK_T referenceCountLock_;

 private:
  // convenient types
  typedef Stream_DataBlockAllocatorHeap_T<ACE_SYNCH_USE,
                                          ConfigurationType> OWN_TYPE_T;
  // *NOTE*: 'long' allows efficient atomic increments on many platforms (see
  //         available ACE_Atomic_Op template specializations)
  typedef ACE_Atomic_Op<ACE_SYNCH_MUTEX_T, long> CACHE_SIZE_COUNTER_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_DataBlockAllocatorHeap_T (const Stream_DataBlockAllocatorHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_DataBlockAllocatorHeap_T& operator= (const Stream_DataBlockAllocatorHeap_T&))

  // stub (part of) ACE_Allocator
  inline virtual void* calloc (size_t, size_t, char = '\0') { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
  inline virtual int remove (void) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int bind (const char*, void*, int = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int trybind (const char*, void*&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int find (const char*, void*&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int find (const char*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int unbind (const char*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int unbind (const char*, void*&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int sync (ssize_t = -1, int = MS_SYNC) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int sync (void*, size_t, int = MS_SYNC) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int protect (ssize_t = -1, int = PROT_RDWR) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int protect (void*, size_t, int = PROT_RDWR) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
#if defined (ACE_HAS_MALLOC_STATS)
  inline virtual void print_stats (void) const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
#endif /* ACE_HAS_MALLOC_STATS */
  inline virtual void dump (void) const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }

  HEAP_ALLOCATOR_T*       heapAllocator_;
  CACHE_SIZE_COUNTER_T    poolSize_;
};

// include template definition
#include "stream_datablockallocatorheap.inl"

#endif
