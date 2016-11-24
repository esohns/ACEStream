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

#include <ace/Log_Msg.h>

#include "stream_macros.h"

template <ACE_SYNCH_DECL>
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::Stream_CachedMessageAllocatorHeap_T (unsigned int chunks_in,
                                                                                         ACE_Allocator* allocator_in,
                                                                                         bool block_in)
 : inherited (chunks_in)
 , block_ (block_in)
 , dataBlockAllocator_ (chunks_in,
                        allocator_in,
                        block_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::Stream_CachedMessageAllocatorHeap_T"));

}

template <ACE_SYNCH_DECL>
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::~Stream_CachedMessageAllocatorHeap_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::~Stream_CachedMessageAllocatorHeap_T"));

}

template <ACE_SYNCH_DECL>
void*
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::calloc"));

  ACE_Message_Block* message_block_p = NULL;
  try {
    ACE_NEW_MALLOC_NORETURN (message_block_p,
                             static_cast<ACE_Message_Block*> (inherited::malloc (sizeof (ACE_Message_Block))),
                             ACE_Message_Block (NULL,                           // <-- no data
                                                ACE_Message_Block::DONT_DELETE, // flags
                                                this));                         // notify this allocator upon destruction
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Message_Block()), continuing\n")));
  }
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Message_Block(), aborting\n")));
    return NULL;
  } // end IF

  return message_block_p;
}

template <ACE_SYNCH_DECL>
void*
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::malloc"));

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_RETURN (data_block_p,
                          static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)),
                          NULL);
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_RETURN(%u), aborting\n"),
                bytes_in));
    return NULL;
  }

  // *NOTE*: must clean up data block beyond this point !

  // step2: get free message
  ACE_Message_Block* message_block_p = NULL;
  try {
    // allocate memory and perform a placement new by invoking a ctor
    // on the allocated space
    ACE_NEW_MALLOC_NORETURN (message_block_p,
                             static_cast<ACE_Message_Block*> (inherited::malloc (sizeof (ACE_Message_Block))),
                             ACE_Message_Block (data_block_p, // reference the data block we just allocated
                                                0,            // flags: release our data block when we die
                                                this));       // remember us upon destruction
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Message_Block(%u)), continuing\n"),
                bytes_in));
  }
  if (!message_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Message_Block(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  return message_block_p;
}

template <ACE_SYNCH_DECL>
void
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::free"));

  inherited::free (handle_in);
}

template <ACE_SYNCH_DECL>
size_t
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::cache_depth"));

  return const_cast<Stream_CachedMessageAllocatorHeap_T*> (this)->pool_depth ();
}

template <ACE_SYNCH_DECL>
size_t
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::cache_size"));

  return dataBlockAllocator_.cache_size ();
}

template <ACE_SYNCH_DECL>
void*
Stream_CachedMessageAllocatorHeap_T<ACE_SYNCH_USE>::calloc (size_t bytes_in,
                                                            char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocatorHeap_T::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  return malloc (bytes_in);
}
