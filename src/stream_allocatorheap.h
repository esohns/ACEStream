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

#ifndef Stream_AllocatorHeap_T_H
#define Stream_AllocatorHeap_T_H

#include <limits>

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Malloc_Allocator.h"
#include "ace/Synch_Traits.h"

#include "stream_allocatorbase.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
class Stream_AllocatorHeap_T
 : public Stream_AllocatorBase_T<ConfigurationType>
 , public ACE_New_Allocator
{
  typedef Stream_AllocatorBase_T<ConfigurationType> inherited;
  typedef ACE_New_Allocator inherited2;

 public:
  Stream_AllocatorHeap_T ();
  inline virtual ~Stream_AllocatorHeap_T () {}

  // override (part of) ACE_Allocator
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value
  virtual void* calloc (size_t,       // # elements
                        size_t,       // bytes/element
                        char = '\0'); // initial value

  // implement Stream_IAllocator
  inline virtual bool block () { return false; } // return value: block when full ?
  virtual void* calloc ();
  virtual void* malloc (size_t); // bytes (? data- : session message)
  virtual void free (void*); // element handle
  inline virtual size_t cache_depth () const { return std::numeric_limits<size_t>::max (); }
  inline virtual size_t cache_size () const { return static_cast<size_t> (poolSize_.value ()); } // return value: #bytes allocated

  // implement Common_IDumpState
  virtual void dump_state () const;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorHeap_T (const Stream_AllocatorHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorHeap_T& operator= (const Stream_AllocatorHeap_T&))

  ACE_Atomic_Op<ACE_SYNCH_MUTEX_T, long> poolSize_;
};

// include template definition
#include "stream_allocatorheap.inl"

#endif
