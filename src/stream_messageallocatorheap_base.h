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

#include <limits>

#include <ace/Atomic_Op.h>
#include <ace/Malloc_Allocator.h>
#include <ace/Synch_Traits.h>
#include <ace/Thread_Semaphore.h>

#include "common_idumpstate.h"

#include "stream_datablockallocatorheap.h"
#include "stream_iallocator.h"

// forward declarations
template <typename AllocatorConfigurationType>
class Stream_AllocatorHeap_T;

template <typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_MessageAllocatorHeapBase_T
 : public ACE_New_Allocator
 , public Stream_IAllocator
 , public Common_IDumpState
{
 public:
  // convenient types
  typedef Stream_AllocatorHeap_T<ConfigurationType> HEAP_ALLOCATOR_T;
  typedef Stream_DataBlockAllocatorHeap_T<ConfigurationType> DATABLOCK_ALLOCATOR_T;

  Stream_MessageAllocatorHeapBase_T (unsigned int = std::numeric_limits<unsigned int>::max (), // total number of concurrent messages
                                     HEAP_ALLOCATOR_T* = NULL,                                 // (heap) memory allocator handle
                                     bool = true);                                             // block until a buffer is available ?
  virtual ~Stream_MessageAllocatorHeapBase_T ();

  // implement Stream_IAllocator
  inline virtual bool block () { return block_; };
  virtual void* calloc ();
  // *NOTE*: if argument is > 0, this returns a (pointer to) <MessageType>, and
  //         a (pointer to) <SessionMessageType> otherwise
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees a <MessageType>/<SessionMessageType>
  virtual void free (void*); // handle
  virtual size_t cache_depth () const; // return value: #bytes allocated
  virtual size_t cache_size () const;  // return value: #inflight ACE_Message_Blocks

  // implement (part of) ACE_Allocator
  // *NOTE*: returns a pointer to raw memory (!) of size <MessageType>/
  //         <SessionMessageType> --> see above
  // *NOTE*: no data block is allocated
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value (not used)

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  typedef ACE_New_Allocator inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_MessageAllocatorHeapBase_T (const Stream_MessageAllocatorHeapBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MessageAllocatorHeapBase_T& operator= (const Stream_MessageAllocatorHeapBase_T&))

  // these methods are ALL no-ops and will FAIL !
  // *NOTE*: this method is a no-op and just returns NULL
  // since the free list only works with fixed sized entities
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

  bool                                          block_;
  DATABLOCK_ALLOCATOR_T                         dataBlockAllocator_;
  ACE_SYNCH_SEMAPHORE                           freeMessageCounter_;
  // *NOTE*: only the (unsigned) 'long' specialization may have support the
  //         interlocked exchange_add (see ace/Atomic_Op.h)
  ACE_Atomic_Op<ACE_SYNCH_MUTEX, unsigned long> poolSize_;
};

// include template definition
#include "stream_messageallocatorheap_base.inl"

#endif
