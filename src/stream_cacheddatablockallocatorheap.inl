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

#include "ace/Log_Msg.h"

#include "common_macros.h"

#include "stream_common.h"
#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL>
Stream_CachedDataBlockAllocatorHeap_T<ACE_SYNCH_USE>::Stream_CachedDataBlockAllocatorHeap_T (unsigned int chunks_in,
                                                                                             ACE_Allocator* allocator_in,
                                                                                             bool block_in)
 : inherited ((chunks_in == 0) ? STREAM_QUEUE_DEFAULT_CACHED_MESSAGES : chunks_in)
 , block_ (block_in)
 , heapAllocator_ (allocator_in)
 , poolSize_ ((chunks_in == 0) ? STREAM_QUEUE_DEFAULT_CACHED_MESSAGES : chunks_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap_T::Stream_CachedDataBlockAllocatorHeap_T"));

  // sanity check(s)
  if (unlikely (!chunks_in))
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot allocate unlimited memory, caching %d buffers...\n"),
                STREAM_QUEUE_DEFAULT_CACHED_MESSAGES));

  // *NOTE*: NULL --> use heap (== default allocator !)
  if (!heapAllocator_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default (== heap) message buffer allocation strategy...\n")));
}

template <ACE_SYNCH_DECL>
void*
Stream_CachedDataBlockAllocatorHeap_T<ACE_SYNCH_USE>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap_T::calloc"));

  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_NEW_MALLOC_NORETURN (data_block_p,
                             static_cast<ACE_Data_Block*> (inherited::calloc (sizeof (ACE_Data_Block))),
                             ACE_Data_Block (0,                                                         // size of data chunk
                                             STREAM_MESSAGE_CONTROL,                                    // message type
                                             NULL,                                                      // data --> use allocator !
                                             NULL,                                                      // allocator
                                             &OWN_TYPE_T::referenceCountLock_,                          // reference count lock
                                             0,                                                         // flags: release (heap) memory in dtor
                                             this));                                                    // data block allocator
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Data_Block(0)): \"%m\", continuing\n")));
  }
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block(0): \"%m\", aborting\n")));
    return NULL;
  } // end IF

  return data_block_p;
}

template <ACE_SYNCH_DECL>
void*
Stream_CachedDataBlockAllocatorHeap_T<ACE_SYNCH_USE>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedDataBlockAllocatorHeap_T::malloc"));

  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_NEW_MALLOC_NORETURN (data_block_p,
                             static_cast<ACE_Data_Block*> (inherited::malloc (sizeof (ACE_Data_Block))),
                             ACE_Data_Block (bytes_in,                                                   // size of data chunk
                                             (bytes_in ? STREAM_MESSAGE_DATA : STREAM_MESSAGE_SESSION),  // message type
                                             NULL,                                                       // data --> use allocator !
                                             (bytes_in ? heapAllocator_ : NULL),                         // allocator
                                             &OWN_TYPE_T::referenceCountLock_,                           // reference count lock
                                             0,                                                          // flags: release (heap) memory in dtor
                                             this));                                                     // data block allocator
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Data_Block(%u)): \"%m\", continuing\n"),
                bytes_in));
  }
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block(%u): \"%m\", aborting\n"),
                bytes_in));
    return NULL;
  } // end IF

  return data_block_p;
}
