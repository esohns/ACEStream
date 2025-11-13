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

#ifndef STREAM_MESSAGEALLOCATORHEAP_BASE_H
#define STREAM_MESSAGEALLOCATORHEAP_BASE_H

#include "ace/Atomic_Op.h"
#include "ace/Malloc_Allocator.h"
#include "ace/Synch_Traits.h"
#include "ace/Thread_Semaphore.h"

#include "common_idumpstate.h"

#include "stream_datablockallocatorheap.h"
#include "stream_iallocator.h"

// forward declarations
template <ACE_SYNCH_DECL,
          typename ConfigurationType>
class Stream_AllocatorHeap_T;

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T
 : public ACE_New_Allocator
 , public Stream_IAllocator
 , public Common_IDumpState
{
  typedef ACE_New_Allocator inherited;

 public:
  // convenient types
  typedef Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                                 ConfigurationType> HEAP_ALLOCATOR_T;
  typedef Stream_DataBlockAllocatorHeap_T<ACE_SYNCH_USE,
                                          ConfigurationType> DATABLOCK_ALLOCATOR_T;

  Stream_MessageAllocatorHeapBase_T (size_t = 0,               // number of concurrent messages (0: no limits)
                                     HEAP_ALLOCATOR_T* = NULL, // (heap) memory allocator handle
                                     bool = true);             // block until a buffer is available ?
  inline virtual ~Stream_MessageAllocatorHeapBase_T () {}

  // implement Stream_IAllocator
  inline virtual bool block () { return block_; }
  virtual void* calloc ();
  // *NOTE*: if argument is > 0, this returns a (pointer to) <MessageType>, and
  //         a (pointer to) <SessionMessageType> otherwise
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees a <MessageType>/<SessionMessageType>
  virtual void free (void*); // handle
  inline virtual size_t cache_depth () const { return dataBlockAllocator_.cache_depth (); } // return value: #bytes allocated
  inline virtual size_t cache_size () const { return poolSize_.value (); } // return value: #inflight ACE_Message_Blocks

  // implement (part of) ACE_Allocator
  // *NOTE*: returns a pointer to raw memory (!) of size <MessageType>/
  //         <SessionMessageType> --> see above
  // *NOTE*: no data block is allocated
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value (not used)

  // implement Common_IDumpState
  inline virtual void dump_state () const { return dataBlockAllocator_.dump_state (); }

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageAllocatorHeapBase_T (const Stream_MessageAllocatorHeapBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageAllocatorHeapBase_T& operator= (const Stream_MessageAllocatorHeapBase_T&))

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

  bool                                                 block_;
  DATABLOCK_ALLOCATOR_T                                dataBlockAllocator_;
  ACE_SYNCH_SEMAPHORE_T                                freeMessageCounter_;
  ACE_Atomic_Op<ACE_SYNCH_MUTEX_T, unsigned long long> poolSize_;
};

// include template definition
#include "stream_messageallocatorheap_base.inl"

#endif
