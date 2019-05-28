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

#ifndef STREAM_CACHEDMESSAGEALLOCATOR_H
#define STREAM_CACHEDMESSAGEALLOCATOR_H

#include "ace/config-lite.h"
#include "ace/Malloc_T.h"
#include "ace/Synch_Traits.h"

#include "stream_cacheddatablockallocatorheap.h"
#include "stream_iallocator.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CachedMessageAllocator_T
 : public ACE_Allocator
 , public Stream_IAllocator
{
  typedef ACE_Allocator inherited;

 public:
  Stream_CachedMessageAllocator_T (unsigned int,   // total number of concurrent messages
                                   ACE_Allocator*, // (heap) memory allocator
                                   bool = true);   // block until a buffer is available ?
  inline virtual ~Stream_CachedMessageAllocator_T () {}

  // implement Stream_IAllocator
  // *IMPORTANT NOTE*: whatever is passed into the ctors' 3rd argument, this
  //                   NEVER blocks; elements are allocated dynamically when lwm
  //                   is reached (see: ACE_Locked_Free_List)
  inline virtual bool block () { return false; }
  virtual void* calloc ();
  // *NOTE*: returns a pointer to <MessageType>
  // *NOTE: passing '0' will return a <SessionMessageType>
  // *TODO*: the way message IDs are implemented, they can be truly unique
  //         only IF allocation is synchronized
  virtual void* malloc (size_t); // bytes
  virtual void free (void*); // handle
  inline virtual size_t cache_depth () const { return dataBlockAllocator_.cache_depth (); } // return value: #bytes allocated
  inline virtual size_t cache_size  () const { return dataBlockAllocator_.cache_size (); } // return value: #inflight ACE_Message_Blocks

  // *NOTE*: returns a pointer to <MessageType>/<SessionMessageType>
  //         --> see above
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocator_T (const Stream_CachedMessageAllocator_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocator_T& operator= (const Stream_CachedMessageAllocator_T&))

  inline virtual void* calloc (size_t, size_t, char = '\0') { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }

  // *NOTE*: frees an <MessageType>/<SessionMessageType>
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
  Stream_CachedDataBlockAllocatorHeap_T<ACE_SYNCH_USE> dataBlockAllocator_;
  ACE_Cached_Allocator<ControlMessageType,
                       ACE_SYNCH_MUTEX_T>              controlMessageAllocator_;
  ACE_Cached_Allocator<DataMessageType,
                       ACE_SYNCH_MUTEX_T>              dataMessageAllocator_;
  ACE_Cached_Allocator<SessionMessageType,
                       ACE_SYNCH_MUTEX_T>              sessionMessageAllocator_;
};

// include template definition
#include "stream_cachedmessageallocator.inl"

#endif
