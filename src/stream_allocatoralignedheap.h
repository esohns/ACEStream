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

#ifndef Stream_AllocatorAlignedHeap_T_H
#define Stream_AllocatorAlignedHeap_T_H

#include <limits>

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Malloc_Allocator.h"
#include "ace/Synch_Traits.h"

#include "stream_allocatorbase.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment = 4096ULL>
class Stream_AllocatorAlignedHeap_T
 : public Stream_AllocatorBase_T<ConfigurationType>
 , public ACE_Allocator
{
  typedef Stream_AllocatorBase_T<ConfigurationType> inherited;
  typedef ACE_Allocator inherited2;

 public:
  Stream_AllocatorAlignedHeap_T ();
  inline virtual ~Stream_AllocatorAlignedHeap_T () {}

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
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorAlignedHeap_T (const Stream_AllocatorAlignedHeap_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorAlignedHeap_T& operator= (const Stream_AllocatorAlignedHeap_T&))

  // stub (part of) ACE_Allocator
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

  ACE_Atomic_Op<ACE_SYNCH_MUTEX_T, ACE_UINT64> poolSize_;
};

// include template definition
#include "stream_allocatoralignedheap.inl"

#endif
