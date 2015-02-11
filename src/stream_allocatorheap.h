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

#ifndef STREAM_ALLOCATORHEAP_H
#define STREAM_ALLOCATORHEAP_H

#include "ace/Atomic_Op.h"
#include "ace/Malloc_Allocator.h"
#include "ace/Synch.h"

#include "stream_exports.h"
#include "stream_iallocator.h"

class Stream_Export Stream_AllocatorHeap
 : public ACE_New_Allocator,
   public Stream_IAllocator
{
 public:
  Stream_AllocatorHeap ();
  virtual ~Stream_AllocatorHeap ();

  // overloads from ACE_Allocator
  virtual void* malloc (size_t); // bytes
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value
  virtual void* calloc (size_t,       // # elements
                        size_t,       // bytes/element
                        char = '\0'); // initial value
  virtual void free (void*); // element handle

  // *NOTE*: these return the amount of allocated (heap) memory...
  virtual size_t cache_depth () const;
  virtual size_t cache_size () const;

  // dump current state
  virtual void dump () const;

 private:
  typedef ACE_New_Allocator inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorHeap (const Stream_AllocatorHeap&));
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorHeap& operator= (const Stream_AllocatorHeap&));

  // these methods are ALL no-ops and will FAIL ! --> hide from user
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

  ACE_Atomic_Op<ACE_Thread_Mutex,
                unsigned int> poolSize_;
};

#endif
