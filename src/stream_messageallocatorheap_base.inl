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

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_MessageAllocatorHeapBase_T<ACE_SYNCH_USE,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType>::Stream_MessageAllocatorHeapBase_T (unsigned int maximumNumberOfMessages_in,
                                                                                          HEAP_ALLOCATOR_T* allocator_in,
                                                                                          bool block_in)
 : inherited ()
 , block_ (block_in)
 , dataBlockAllocator_ (allocator_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , freeMessageCounter_ ((!maximumNumberOfMessages_in || // *TODO*: this is wrong --> implement 'no limit' feature
                         (maximumNumberOfMessages_in >= static_cast<unsigned int> (std::numeric_limits<LONG>::max ())) ? std::numeric_limits<LONG>::max ()
                                                                                                                       : maximumNumberOfMessages_in), // initial count
                        NULL,                                                                                             // name
                        NULL,                                                                                             // ACT
                        (!maximumNumberOfMessages_in || // *TODO*: this is wrong --> implement 'no limit' feature
                         (maximumNumberOfMessages_in >= static_cast<unsigned int> (std::numeric_limits<LONG>::max ())) ? std::numeric_limits<LONG>::max ()
                                                                                                                       : maximumNumberOfMessages_in)) // maximum
#else
 , freeMessageCounter_ ((!maximumNumberOfMessages_in || // *TODO*: this is wrong --> implement 'no limit' feature
                         (maximumNumberOfMessages_in >= SEM_VALUE_MAX) ? SEM_VALUE_MAX
                                                                       : maximumNumberOfMessages_in), // initial count
                        NULL,                                                                         // name
                        NULL,                                                                         // ACT
                        (!maximumNumberOfMessages_in || // *TODO*: this is wrong --> implement 'no limit' feature
                         (maximumNumberOfMessages_in >= SEM_VALUE_MAX) ? SEM_VALUE_MAX
                                                                       : maximumNumberOfMessages_in)) // maximum
#endif // ACE_WIN32 || ACE_WIN64
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::Stream_MessageAllocatorHeapBase_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_MessageAllocatorHeapBase_T<ACE_SYNCH_USE,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::calloc"));

  int result = -1;

  // step0: wait for an empty slot ?
  if (block_)
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (unlikely (result == -1))
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_SEMAPHORE::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  ++poolSize_;

  // step1: get a free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.calloc ()));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN(ACE_Data_Block()), continuing\n")));
  }
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block(), aborting\n")));
    --poolSize_;
    freeMessageCounter_.release ();
    return NULL;
  } // end IF
  // *NOTE*: must release() data_block_p beyond this point !

  // step2: allocate a control message
  // *NOTE*: fire-and-forget data_block_p if this is successful
  ControlMessageType* message_p = NULL;
  try {
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<ControlMessageType*> (inherited::malloc (sizeof (ControlMessageType))),
                             ControlMessageType (data_block_p,
                                                 this)); // message allocator
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ControlMessageType(), continuing\n")));
  }
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate control message, aborting\n")));
    data_block_p->release (); data_block_p = NULL;
    --poolSize_;
    freeMessageCounter_.release ();
    return NULL;
  } // end IF

  return message_p;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_MessageAllocatorHeapBase_T<ACE_SYNCH_USE,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::malloc"));

  int result = -1;
  // step0: wait for an empty slot ?
  if (block_)
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (unlikely (result == -1))
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_SEMAPHORE::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  ++poolSize_;

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN(ACE_Data_Block(%u)), continuing\n"),
                bytes_in));
  }
  if (unlikely (!data_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    --poolSize_;
    freeMessageCounter_.release ();
    return NULL;
  } // end IF

  // *NOTE*: must clean up data block beyond this point !

  // step2: allocate message
  ACE_Message_Block* message_block_p = NULL;
  try {
    // allocate memory and perform a placement new by invoking a ctor
    // on the allocated space
    if (bytes_in)
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<DataMessageType*> (inherited::malloc (sizeof (DataMessageType))),
                               DataMessageType (0,            // session id
                                                data_block_p, // use the newly allocated data block
                                                this,         // remember allocator upon destruction
                                                true));       // increment message counter ?
    else
      ACE_NEW_MALLOC_NORETURN (message_block_p,
                               static_cast<SessionMessageType*> (inherited::malloc (sizeof (SessionMessageType))),
                               SessionMessageType (0,            // session id
                                                   data_block_p, // use the newly allocated data block
                                                   this));       // remember allocator upon destruction
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN((Session)MessageType(%u), continuing\n"),
                bytes_in));
  }
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate (Session)MessageType(%u), aborting\n"),
                bytes_in));
    data_block_p->release (); data_block_p = NULL;
    --poolSize_;
    freeMessageCounter_.release ();
    return NULL;
  } // end IF

  // *NOTE*: the caller knows what to expect; MessageType or SessionMessageType
  return message_block_p;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_MessageAllocatorHeapBase_T<ACE_SYNCH_USE,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType>::calloc (size_t bytes_in,
                                                               char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  // sanity check(s)
  ACE_ASSERT ((bytes_in == sizeof (ControlMessageType)) ||
              (bytes_in == sizeof (DataMessageType))    ||
              (bytes_in == sizeof (SessionMessageType)));

  int result = -1;
  // step0: wait for an empty slot ?
  if (likely (block_))
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (unlikely (result == -1))
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Semaphore::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  ++poolSize_;

  // step1: allocate free message
  void* message_p = NULL;
  try {
    message_p = inherited::malloc (bytes_in);
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_New_Allocator::malloc(%u), continuing\n"),
                bytes_in));
  }
  if (unlikely (!message_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate (Session)MessageType(%u), aborting\n"),
                bytes_in));
    poolSize_--;
    freeMessageCounter_.release ();
    return NULL;
  } // end IF

  // *NOTE*: the caller knows what to expect
  return message_p;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_MessageAllocatorHeapBase_T<ACE_SYNCH_USE,
                                  ConfigurationType,
                                  ControlMessageType,
                                  DataMessageType,
                                  SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MessageAllocatorHeapBase_T::free"));

  int result = -1;

  inherited::free (handle_in);

  // OK: one slot just emptied
  --poolSize_;
  result = freeMessageCounter_.release ();
  if (unlikely (result == -1))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_SEMAPHORE::release(): \"%m\", continuing\n")));
}
