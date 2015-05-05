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

#include "ace/Message_Block.h"
#include "ace/Log_Msg.h"

#include "stream_macros.h"
#include "stream_allocatorheap.h"
#include "stream_message_base.h"

template <typename MessageType,
          typename SessionMessageType>
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::Stream_MessageAllocatorHeapBase_T (unsigned int maxNumMessages_in,
                                                                                          Stream_AllocatorHeap* allocator_in,
                                                                                          bool block_in)
 : inherited ()
 , block_ (block_in)
 , dataBlockAllocator_ (allocator_in)
 , freeMessageCounter_ (maxNumMessages_in,
                        NULL,
                        NULL,
                        maxNumMessages_in)
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::Stream_MessageAllocatorHeapBase_T"));

}

template <typename MessageType,
          typename SessionMessageType>
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::~Stream_MessageAllocatorHeapBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::~Stream_MessageAllocatorHeapBase_T"));

}

template <typename MessageType,
          typename SessionMessageType>
bool
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType > ::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::block"));

  return block_;
}

template <typename MessageType,
          typename SessionMessageType>
void*
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::malloc"));

  int result = -1;
  // step0: wait for an empty slot ?
  if (block_)
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (result == -1)
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Semaphore::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  poolSize_++;

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try
  {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN(ACE_Data_Block(%u)), aborting\n"),
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

  // *NOTE*: must clean up data block beyond this point !

  // step2: get free message...
  ACE_Message_Block* message_p = NULL;
  try
  {
    // allocate memory and perform a placement new by invoking a ctor
    // on the allocated space
    if (bytes_in)
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<MessageType*> (inherited::malloc (sizeof (MessageType))),
                               MessageType (data_block_p, // use the newly allocated data block
                                            this));       // message allocator
    else
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<SessionMessageType*> (inherited::malloc (sizeof (SessionMessageType))),
                               SessionMessageType (data_block_p, // use the newly allocated data block
                                                   this));       // message allocator
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN((Session)MessageType(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate (Session)MessageType(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // *NOTE*: the caller knows what to expect (either MessageType ||
  //         SessionMessageType)
  return message_p;
}

template <typename MessageType,
          typename SessionMessageType>
void*
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::calloc (size_t bytes_in,
                                                               char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  int result = -1;
  // step0: wait for an empty slot ?
  if (block_)
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (result == -1)
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Semaphore::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  poolSize_++;

  // step1: allocate free message...
  void* message_p = NULL;
  try
  {
    message_p = inherited::malloc ((bytes_in ? sizeof (MessageType)
                                             : sizeof (SessionMessageType)));
  }
  catch (...)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_New_Allocator::malloc(%u), aborting\n"),
                (bytes_in ? sizeof (MessageType)
                          : sizeof (SessionMessageType))));
    return NULL;
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate (Session)MessageType(%u), aborting\n"),
                (bytes_in ? sizeof (MessageType)
                          : sizeof (SessionMessageType))));
    return NULL;
  } // end IF

  // ... and return the result
  // *NOTE*: the caller knows what to expect (either MessageType ||
  //         SessionMessageType)
  return message_p;
}

template <typename MessageType,
          typename SessionMessageType>
void
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::free"));

  int result = -1;

  // delegate to base class...
  inherited::free (handle_in);

  // OK: one slot just emptied...
  poolSize_--;
  result = freeMessageCounter_.release ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Thread_Semaphore::release(): \"%m\", continuing\n")));
}

template <typename MessageType,
          typename SessionMessageType>
size_t
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::cache_depth"));

  return dataBlockAllocator_.cache_depth ();
}

template <typename MessageType,
          typename SessionMessageType>
size_t
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::cache_size"));

  return poolSize_.value ();
}

template <typename MessageType,
          typename SessionMessageType>
void
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::dump_state"));

  return dataBlockAllocator_.dump_state ();
}

template <typename MessageType,
          typename SessionMessageType>
void*
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::calloc (size_t numElements_in,
                                                               size_t sizePerElement_in,
                                                               char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::calloc"));

  ACE_UNUSED_ARG (numElements_in);
  ACE_UNUSED_ARG (sizePerElement_in);
  ACE_UNUSED_ARG (initialValue_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (NULL);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::remove"));

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::bind (const char* name_in,
                                                             void* pointer_in,
                                                             int duplicates_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::bind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);
  ACE_UNUSED_ARG (duplicates_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::trybind (const char* name_in,
                                                                void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::trybind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::find (const char* name_in,
                                                             void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::find"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::find (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::unbind (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::unbind"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::unbind (const char* name_in,
                                                               void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::unbind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::sync (ssize_t length_in,
                                                             int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::sync"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::sync (void* address_in,
                                                             size_t length_in,
                                                             int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::sync"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::protect (ssize_t length_in,
                                                                int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::protect"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename MessageType,
          typename SessionMessageType>
int
Stream_MessageAllocatorHeapBase_T<MessageType,
                                  SessionMessageType>::protect (void* address_in,
                                                                size_t length_in,
                                                                int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::protect"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}
