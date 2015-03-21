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

#ifndef STREAM_DATABLOCKALLOCATORHEAP_H
#define STREAM_DATABLOCKALLOCATORHEAP_H

#include "ace/Atomic_Op.h"
#include "ace/Lock_Adapter_T.h"
#include "ace/Malloc_Allocator.h"
#include "ace/Synch.h"

#include "common_idumpstate.h"

#include "stream_exports.h"
#include "stream_iallocator.h"

// forward declarations
class Stream_AllocatorHeap;

class Stream_Export Stream_DataBlockAllocatorHeap
 : public ACE_New_Allocator
 , public Stream_IAllocator
 , public Common_IDumpState
{
 public:
  Stream_DataBlockAllocatorHeap (Stream_AllocatorHeap*); // (heap) memory allocator...
  virtual ~Stream_DataBlockAllocatorHeap ();

  // implement Stream_IAllocator
  virtual bool block (); // return value: block when full ?
  // *NOTE*: returns a pointer to ACE_Data_Block...
  virtual void* malloc (size_t); // bytes
  // *NOTE*: frees an ACE_Data_Block...
  virtual void free (void*); // element handle
  virtual size_t cache_depth () const; // return value: #bytes allocated
  virtual size_t cache_size () const;  // return value: #inflight ACE_Data_Blocks

  // implement (part of) ACE_Allocator
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value (not used)

  // implement Common_IDumpState
  virtual void dump_state () const;

  // some convenience typedefs --> save us some typing...
  // *NOTE*: serialize access to ACE_Data_Block reference count which may
  // be decremented from multiple threads...
  typedef ACE_Lock_Adapter<ACE_Thread_Mutex> DATABLOCK_LOCK_TYPE;

  // locking
  // *NOTE*: currently, ALL data blocks use one static lock (OK for stream usage)...
  // *TODO*: consider using a lock-per-message strategy...
  static DATABLOCK_LOCK_TYPE referenceCountLock_;

 private:
  typedef ACE_New_Allocator inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_DataBlockAllocatorHeap (const Stream_DataBlockAllocatorHeap&));
  ACE_UNIMPLEMENTED_FUNC (Stream_DataBlockAllocatorHeap& operator= (const Stream_DataBlockAllocatorHeap&));

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

  Stream_AllocatorHeap*       heapAllocator_;
  ACE_Atomic_Op<ACE_Thread_Mutex,
                unsigned int> poolSize_;
};

#endif
