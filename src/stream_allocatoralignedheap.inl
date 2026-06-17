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

#include "common_macros.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::Stream_AllocatorAlignedHeap_T ()
 : inherited ()
 , inherited2 ()
 , poolSize_ (0ULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::Stream_AllocatorAlignedHeap_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
void*
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::calloc (size_t bytes_in,
                                                  char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::calloc"));

  // delegate to base class
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  void* result = _aligned_malloc (bytes_in,
                                  alignment);
#else
  void* result = NULL;
  int result_2 = posix_memalign (&result,
                                 alignment,
                                 bytes_in);
  ACE_ASSERT (result_2 == 0);
#endif // ACE_WIN32 || ACE_WIN64

  // update allocation counter
  if (likely (result))
  {
    ACE_OS::memset (result, initialValue_in, bytes_in);

    poolSize_ += static_cast<ACE_UINT64> (bytes_in);
  } // end IF

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
void*
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::calloc (size_t numberOfElements_in,
                                                  size_t sizePerElement_in,
                                                  char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::calloc"));

  // delegate to base class
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  void* result = _aligned_malloc (numberOfElements_in * sizePerElement_in,
                                  alignment);
#else
  void* result = NULL;
  int result_2 = posix_memalign (&result,
                                 alignment,
                                 numberOfElements_in * sizePerElement_in);
  ACE_ASSERT (result_2 == 0);
#endif // ACE_WIN32 || ACE_WIN64

  // update allocation counter
  if (likely (result))
  {
    ACE_OS::memset (result, initialValue_in, numberOfElements_in * sizePerElement_in);

    poolSize_ +=
      static_cast<ACE_UINT64> (numberOfElements_in * sizePerElement_in);
  } // end IF

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
void*
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::calloc"));

  // delegate to base class
  void* result = calloc (1,
                         sizeof (ACE_Message_Block),
                         '\0');

  // update allocation counter
  if (likely (result))
    poolSize_ += sizeof (ACE_Message_Block);

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
void*
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::malloc"));

  // delegate to base class
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  void* result = _aligned_malloc (bytes_in,
                                  alignment);
#else
  void* result = NULL;
  int result_2 = posix_memalign (&result,
                                 alignment,
                                 bytes_in);
  ACE_ASSERT (result_2 == 0);
#endif // ACE_WIN32 || ACE_WIN64

  // update allocation counter
  if (likely (result))
    poolSize_ += static_cast<ACE_UINT64> (bytes_in);

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
void
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::free"));

  // sanity check(s)
  ACE_ASSERT ((reinterpret_cast<intptr_t> (handle_in) % alignment) == 0);

  // delegate to base class
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    _aligned_free (handle_in);
#else
  ACE_OS::free (handle_in);
#endif // ACE_WIN32 || ACE_WIN64

  // *TODO*: how can this counter update ???
  // update allocation counter
//   poolSize_ -= bytes_in;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          ACE_UINT64 alignment>
void
Stream_AllocatorAlignedHeap_T<ACE_SYNCH_USE,
                              ConfigurationType,
                              alignment>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorAlignedHeap_T::dump_state"));

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("allocated heap space: %Q byte(s)\n"),
              poolSize_.value ()));
}
