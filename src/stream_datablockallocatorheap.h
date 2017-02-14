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

#ifndef Stream_DataBlockAllocatorHeap_T_H
#define Stream_DataBlockAllocatorHeap_T_H

#include <ace/Atomic_Op.h>
#include <ace/Lock_Adapter_T.h>
#include <ace/Malloc_Allocator.h>
#include <ace/Synch_Traits.h>

#include "common_idumpstate.h"

#include "stream_allocatorheap.h"
#include "stream_exports.h"
#include "stream_iallocator.h"

template <typename ConfigurationType>
class Stream_DataBlockAllocatorHeap_T
 : public ACE_New_Allocator
 , public Stream_IAllocator
 , public Common_IDumpState
{
 public:
  // convenient types
  typedef Stream_AllocatorHeap_T<ConfigurationType> HEAP_ALLOCATOR_T;
  // *NOTE*: serialize access to ACE_Data_Block reference counts, which may
  //         be modified concurrently by multiple threads
  typedef ACE_Lock_Adapter<ACE_SYNCH_MUTEX> DATABLOCK_LOCK_T;

  Stream_DataBlockAllocatorHeap_T (HEAP_ALLOCATOR_T*); // (heap) memory allocator
  virtual ~Stream_DataBlockAllocatorHeap_T ();

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
  // *TODO*: consider using a lock-per-message strategy
  static DATABLOCK_LOCK_T referenceCountLock_;

 private:
  typedef ACE_New_Allocator inherited;

  // convenient types
  typedef Stream_DataBlockAllocatorHeap_T<ConfigurationType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_DataBlockAllocatorHeap_T (const Stream_DataBlockAllocatorHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_DataBlockAllocatorHeap_T& operator= (const Stream_DataBlockAllocatorHeap_T&))

  // these methods are ALL no-ops and will FAIL !
  // *NOTE*: this method is a no-op and just returns NULL since the free list
  //         only works with fixed sized entities
  virtual void* calloc (size_t,       // # elements (not used)
                        size_t,       // bytes/element (not used)
                        char = '\0'); // initial value (not used)
  virtual int remove (void);
  virtual int bind (const char*, // name
                    void*,       // pointer
                    int = 0);    // duplicates
  virtual int trybind (const char*, // name
                       void*&);     // pointer
  virtual int find (const char*, // name
                    void*&);     // pointer
  virtual int find (const char*); // name
  virtual int unbind (const char*); // name
  virtual int unbind (const char*, // name
                      void*&);     // pointer
  virtual int sync (ssize_t = -1,   // length
                    int = MS_SYNC); // flags
  virtual int sync (void*,          // address
                    size_t,         // length
                    int = MS_SYNC); // flags
  virtual int protect (ssize_t = -1,     // length
                       int = PROT_RDWR); // protection
  virtual int protect (void*,            // address
                       size_t,           // length
                       int = PROT_RDWR); // protection

  HEAP_ALLOCATOR_T*                             heapAllocator_;
  ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> poolSize_;
};

// include template definition
#include "stream_datablockallocatorheap.inl"

#endif
