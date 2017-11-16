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

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::Stream_DirectShowAllocatorBase_T (unsigned int maximumNumberOfMessages_in,
                                                                                        HEAP_ALLOCATOR_T* allocator_in,
                                                                                        bool block_in)
 : inherited ()
 , block_ (block_in)
 , dataBlockAllocator_ (allocator_in)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
 , freeMessageCounter_ ((maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                    : std::numeric_limits<signed int>::max ()),     // initial count
                        NULL,                                                                       // name
                        NULL,                                                                       // ACT
                        (maximumNumberOfMessages_in ? static_cast<int> (maximumNumberOfMessages_in) // maximum
                                                    : std::numeric_limits<int>::max ()))
#else
 , freeMessageCounter_ ((maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                    : SEM_VALUE_MAX),                               // initial count
                        NULL,                                                                       // name
                        NULL,                                                                       // ACT
                        (maximumNumberOfMessages_in ? static_cast<int> (maximumNumberOfMessages_in) // maximum
                                                    : std::numeric_limits<int>::max ()))
#endif
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::Stream_DirectShowAllocatorBase_T"));

}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void*
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                  MessageType,
                                  SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::malloc"));

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
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  } catch (...) {
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
  try {
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
  } catch (...) {
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

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void*
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                  MessageType,
                                  SessionMessageType>::calloc (size_t bytes_in,
                                                               char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::calloc"));

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
  try {
    message_p = inherited::malloc ((bytes_in ? sizeof (MessageType)
                                             : sizeof (SessionMessageType)));
  } catch (...) {
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

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                  MessageType,
                                  SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::free"));

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

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::SetProperties (ALLOCATOR_PROPERTIES* requested_in,
                                                                     ALLOCATOR_PROPERTIES* actual_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::SetProperties"));

  ACE_UNUSED_ARG (requested_in);
  ACE_UNUSED_ARG (actual_inout);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::GetProperties (ALLOCATOR_PROPERTIES* properties_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::GetProperties"));

  ACE_UNUSED_ARG (properties_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::Commit (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::Commit"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::Decommit (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::Decommit"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::GetBuffer (IMediaSample** mediaSample_inout,
                                                                 REFERENCE_TIME* startTime_inout,
                                                                 REFERENCE_TIME* endTime_inout,
                                                                 DWORD flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::GetBuffer"));

  ACE_UNUSED_ARG (startTime_inout);
  ACE_UNUSED_ARG (endTime_inout);
  ACE_UNUSED_ARG (flags_in);

  // sanity check(s)
  ACE_ASSERT (mediaSample_inout);
  ACE_ASSERT (!*mediaSample_inout);

  MessageType* message_p =
    static_cast<MessageType*> (this->malloc (std::numeric_limits<unsigned int>::max ()));
  if (!message_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_DirectShowAllocatorBase_T::maloc(), aborting\n")));
    return E_FAIL;
  } // end IF
  *mediaSample_inout = dynamic_cast<IMediaSample*> (message_p);
  if (!*mediaSample_inout)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IMediaSample*>(0x%@), aborting\n"),
                message_p));

    // clean up
    message_p->release ();

    return E_FAIL;
  } // end IF

  return S_OK;
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::ReleaseBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::ReleaseBuffer"));

  // sanity check(s)
  ACE_ASSERT (mediaSample_in);

  mediaSample_in->Release ();

  return S_OK;
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::QueryInterface (REFIID GUID_in,
                                                                      void** interface_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::QueryInterface"));

  ACE_UNUSED_ARG (GUID_in);
  ACE_UNUSED_ARG (interface_inout);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)

}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
ULONG STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::AddRef (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::AddRef"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (0);
  ACE_NOTREACHED (return 0;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
ULONG STDMETHODCALLTYPE
Stream_DirectShowAllocatorBase_T<ConfigurationType,
                                 MessageType,
                                 SessionMessageType>::Release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowAllocatorBase_T::Release"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (0);
  ACE_NOTREACHED (return 0;)
}
