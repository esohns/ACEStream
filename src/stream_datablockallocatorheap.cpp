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

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "stream_datablockallocatorheap.h"

#include "stream_macros.h"
#include "stream_allocatorheap.h"

// init statics
Stream_DataBlockAllocatorHeap::DATABLOCK_LOCK_TYPE Stream_DataBlockAllocatorHeap::referenceCountLock_;

Stream_DataBlockAllocatorHeap::Stream_DataBlockAllocatorHeap (Stream_AllocatorHeap* allocator_in)
 : inherited ()
 , heapAllocator_ (allocator_in)
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::Stream_DataBlockAllocatorHeap"));

  // *NOTE*: NULL --> use heap (== default allocator !)
  if (!heapAllocator_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default (== heap) message buffer allocation strategy...\n")));
  } // end IF
}

Stream_DataBlockAllocatorHeap::~Stream_DataBlockAllocatorHeap ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::~Stream_DataBlockAllocatorHeap"));

}

bool
Stream_DataBlockAllocatorHeap::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::block"));

  return true;
}

void*
Stream_DataBlockAllocatorHeap::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::malloc"));

  ACE_Data_Block* data_block_p = NULL;
  try
  {
    // - delegate allocation to base class and
    // - use placement new by invoking a ctor on the allocated space and
    // - perform necessary initialization...
    ACE_NEW_MALLOC_NORETURN (data_block_p,
                             static_cast<ACE_Data_Block*> (inherited::malloc (sizeof (ACE_Data_Block))),
                             ACE_Data_Block (bytes_in,                                 // size of data chunk
                                             // *TODO*: make this configurable ?
                                             (bytes_in ? ACE_Message_Block::MB_DATA    // message type
                                                       : ACE_Message_Block::MB_PROTO),
                                             NULL,                                     // data --> use allocator !
                                             heapAllocator_,                           // heap allocator
                                             //NULL,                                   // no allocator --> allocate off the heap
                                             &Stream_DataBlockAllocatorHeap::referenceCountLock_, // reference count lock
                                             0,                                        // flags: release (heap) memory in dtor
                                             this));                                   // data block allocator
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Data_Block(%u)), aborting\n"),
                bytes_in));
    return NULL;
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    return NULL;
  } // end IF

  // increment running counter...
//   poolSize_ += data_block->capacity ();
  poolSize_++;

  return data_block_p;
}

void*
Stream_DataBlockAllocatorHeap::calloc (size_t bytes_in,
                                       char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  return malloc (bytes_in);
}

void
Stream_DataBlockAllocatorHeap::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::free"));

  // delegate to base class...
  inherited::free (handle_in);

  // *NOTE*: handle_in really is a ACE_Data_Block*...
//   ACE_Data_Block* data_block = NULL;
//   data_block = static_cast<ACE_Data_Block*> (handle_in);
//   ACE_ASSERT (data_block);

  // update allocation counter
//   poolSize_ -= data_block->capacity ();
  poolSize_--;
}

size_t
Stream_DataBlockAllocatorHeap::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::cache_depth"));

  if (heapAllocator_)
    return heapAllocator_->cache_size ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return std::numeric_limits<unsigned int>::max ();
#else
  return -1;
#endif
}

size_t
Stream_DataBlockAllocatorHeap::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::cache_size"));

  return poolSize_.value ();
}

void
Stream_DataBlockAllocatorHeap::dump_state (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("# ACE_Data_Blocks in flight: %u\n"),
              poolSize_.value ()));
}

void*
Stream_DataBlockAllocatorHeap::calloc (size_t numElements_in,
                                       size_t sizePerElement_in,
                                       char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::calloc"));

  ACE_UNUSED_ARG (numElements_in);
  ACE_UNUSED_ARG (sizePerElement_in);
  ACE_UNUSED_ARG (initialValue_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (NULL);
}

int
Stream_DataBlockAllocatorHeap::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::remove"));

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::bind (const char* name_in,
                                     void* pointer_in,
                                     int duplicates_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::bind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);
  ACE_UNUSED_ARG (duplicates_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::trybind (const char* name_in,
                                        void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::trybind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::find (const char* name_in,
                                     void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::find"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::find (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::unbind (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::unbind"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::unbind (const char* name_in,
                                       void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::unbind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::sync (ssize_t length_in,
                                     int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::sync"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::sync (void* address_in,
                                     size_t length_in,
                                     int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::sync"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::protect (ssize_t length_in,
                                        int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::protect"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_DataBlockAllocatorHeap::protect (void* address_in,
                                        size_t length_in,
                                        int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap::protect"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}
