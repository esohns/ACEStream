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

#include <limits>

#include <ace/Log_Msg.h>

#include "stream_defines.h"
#include "stream_macros.h"

template <typename ConfigurationType>
Stream_CachedAllocatorHeap_T<ConfigurationType>::Stream_CachedAllocatorHeap_T (unsigned int poolSize_in,
                                                                               unsigned int chunkSize_in)
 : inherited ()
 , inherited2 (((poolSize_in == 0) ? STREAM_QUEUE_DEFAULT_CACHED_MESSAGES
                                   : poolSize_in),
              chunkSize_in)
 , poolSize_ ((poolSize_in == 0) ? std::numeric_limits<unsigned int>::max ()
                                 : poolSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap_T::Stream_CachedAllocatorHeap_T"));

  // sanity check(s)
  if (!poolSize_in)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot allocate unlimited memory, caching %d buffers...\n"),
                STREAM_QUEUE_DEFAULT_CACHED_MESSAGES));
}

template <typename ConfigurationType>
Stream_CachedAllocatorHeap_T<ConfigurationType>::~Stream_CachedAllocatorHeap_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap_T::~Stream_CachedAllocatorHeap_T"));

}

template <typename ConfigurationType>
size_t
Stream_CachedAllocatorHeap_T<ConfigurationType>::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap_T::cache_depth"));

  return const_cast<OWN_TYPE_T*> (this)->pool_depth ();
}

template <typename ConfigurationType>
size_t
Stream_CachedAllocatorHeap_T<ConfigurationType>::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap_T::cache_size"));

  return poolSize_.value ();
}

template <typename ConfigurationType>
void
Stream_CachedAllocatorHeap_T<ConfigurationType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedAllocatorHeap_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated heap space: %u byte(s)\n"),
              poolSize_.value ()));
}
