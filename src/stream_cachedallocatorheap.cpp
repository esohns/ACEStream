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
#include "stdafx.h"

#include "stream_cachedallocatorheap.h"

#include <limits>

#include "stream_macros.h"

Stream_CachedAllocatorHeap::Stream_CachedAllocatorHeap (unsigned int poolSize_in,
                                                        unsigned int chunkSize_in)
 : inherited (((poolSize_in == 0) ? 1 : poolSize_in), chunkSize_in)
 , poolSize_ ((poolSize_in == 0) ? std::numeric_limits<unsigned int>::max ()
                                 : (poolSize_in * chunkSize_in))
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap::Stream_CachedAllocatorHeap"));

}

Stream_CachedAllocatorHeap::~Stream_CachedAllocatorHeap ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap::~Stream_CachedAllocatorHeap"));

}

bool
Stream_CachedAllocatorHeap::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap::block"));

  return true;
}

size_t
Stream_CachedAllocatorHeap::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap::cache_depth"));

  return const_cast<Stream_CachedAllocatorHeap*> (this)->pool_depth ();
}

size_t
Stream_CachedAllocatorHeap::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap::cache_size"));

  return poolSize_;
}
