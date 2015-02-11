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

#ifndef STREAM_CACHEDMESSAGEALLOCATORHEAP_H
#define STREAM_CACHEDMESSAGEALLOCATORHEAP_H

#include "ace/Malloc_T.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_iallocator.h"
#include "stream_cacheddatablockallocatorheap.h"

class Stream_CachedMessageAllocatorHeap
 : public ACE_Cached_Allocator<ACE_Message_Block, ACE_SYNCH_MUTEX>,
   public Stream_IAllocator
{
 public:
  Stream_CachedMessageAllocatorHeap (unsigned int,    // total number of chunks
                                     ACE_Allocator*); // (heap) memory allocator...
  virtual ~Stream_CachedMessageAllocatorHeap ();

  // overload ACE_Allocator
  // *NOTE*: returns a pointer to ACE_Message_Block...
  virtual void* malloc (size_t); // bytes

  // *NOTE*: returns a pointer to RPG_Stream_MessageBase...
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value

  // *NOTE*: frees an ACE_Message_Block...
  virtual void free (void*); // element handle

  // *NOTE*: these return the # of online ACE_Message_Blocks...
  virtual size_t cache_depth () const;
  virtual size_t cache_size () const;

 private:
  typedef ACE_Cached_Allocator<ACE_Message_Block, ACE_SYNCH_MUTEX> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocatorHeap (const Stream_CachedMessageAllocatorHeap&));
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocatorHeap& operator= (const Stream_CachedMessageAllocatorHeap&));

  // data block allocator
  Stream_CachedDataBlockAllocatorHeap dataBlockAllocator_;
};

#endif
