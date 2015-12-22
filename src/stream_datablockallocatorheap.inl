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
#include "ace/Message_Block.h"

#include "stream_macros.h"
#include "stream_allocatorheap.h"

// initialize statics
template <typename ConfigurationType>
typename Stream_DataBlockAllocatorHeap_T<ConfigurationType>::DATABLOCK_LOCK_T
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::referenceCountLock_;

template <typename ConfigurationType>
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::Stream_DataBlockAllocatorHeap_T (HEAP_ALLOCATOR_T* allocator_in)
 : inherited ()
 , heapAllocator_ (allocator_in)
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::Stream_DataBlockAllocatorHeap_T"));

  // *NOTE*: NULL --> use heap (== default allocator !)
  if (!heapAllocator_)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("using default (== heap) message buffer allocation strategy...\n")));
  } // end IF
}

template <typename ConfigurationType>
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::~Stream_DataBlockAllocatorHeap_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::~Stream_DataBlockAllocatorHeap_T"));

}

template <typename ConfigurationType>
bool
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::block"));

  return true;
}

template <typename ConfigurationType>
void*
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::malloc"));

  int result = -1;

  ACE_Data_Block* data_block_p = NULL;
  // *TODO*: remove type inference
  size_t number_of_bytes =
    (bytes_in ? (heapAllocator_ ? bytes_in + heapAllocator_->configuration_.buffer
                                : bytes_in)
              : 0);
  try
  {
    // - delegate allocation to base class and...
    // - use placement new by invoking a ctor on the allocated space and...
    // - perform necessary initialization
    ACE_NEW_MALLOC_NORETURN (data_block_p,
                             static_cast<ACE_Data_Block*> (inherited::malloc (sizeof (ACE_Data_Block))),
                             ACE_Data_Block (number_of_bytes,                          // size of data chunk
                                             (bytes_in ? ACE_Message_Block::MB_DATA    // message type
                                                       : ACE_Message_Block::MB_PROTO), // *TODO*: make this configurable ?
                                             NULL,                                     // data --> use allocator !
                                             heapAllocator_,                           // heap allocator ? : heap
                                             &OWN_TYPE_T::referenceCountLock_,         // reference count lock
                                             0,                                        // flags: release (heap) memory in dtor
                                             this));                                   // data block allocator
  }
  catch (...)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ACE_Data_Block(%u)), continuing\n"),
                bytes_in));
    data_block_p = NULL;
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    return NULL;
  } // end IF
  if (bytes_in)
  {
    result = data_block_p->size (bytes_in);
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Data_Block::size(%u): \"%m\", aborting\n"),
                  bytes_in));

      // clean up
      delete data_block_p;

      return NULL;
    } // end IF
  } // end IF

  // increment running counter...
//   poolSize_ += data_block->capacity ();
  poolSize_++;

  return data_block_p;
}

template <typename ConfigurationType>
void*
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::calloc (size_t bytes_in,
                                                            char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  return malloc (bytes_in);
}

template <typename ConfigurationType>
void
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::free"));

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

template <typename ConfigurationType>
size_t
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::cache_depth"));

  if (heapAllocator_)
    return heapAllocator_->cache_size ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return std::numeric_limits<unsigned int>::max ();
#else
  return -1;
#endif
}

template <typename ConfigurationType>
size_t
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::cache_size"));

  return poolSize_.value ();
}

template <typename ConfigurationType>
void
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::dump_state (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("# ACE_Data_Blocks in flight: %u\n"),
              poolSize_.value ()));
}

template <typename ConfigurationType>
void*
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::calloc (size_t numElements_in,
                                                            size_t sizePerElement_in,
                                                            char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::calloc"));

  ACE_UNUSED_ARG (numElements_in);
  ACE_UNUSED_ARG (sizePerElement_in);
  ACE_UNUSED_ARG (initialValue_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (NULL);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::remove"));

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::bind (const char* name_in,
                                                          void* pointer_in,
                                                          int duplicates_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::bind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);
  ACE_UNUSED_ARG (duplicates_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::trybind (const char* name_in,
                                                             void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::trybind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::find (const char* name_in,
                                                          void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::find"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::find (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::unbind (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::unbind"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::unbind (const char* name_in,
                                                            void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::unbind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::sync (ssize_t length_in,
                                                          int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::sync"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::sync (void* address_in,
                                                          size_t length_in,
                                                          int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::sync"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::protect (ssize_t length_in,
                                                             int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::protect"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType>
int
Stream_DataBlockAllocatorHeap_T<ConfigurationType>::protect (void* address_in,
                                                             size_t length_in,
                                                             int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DataBlockAllocatorHeap_T::protect"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}