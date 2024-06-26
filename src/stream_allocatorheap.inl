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

#include "common_macros.h"

#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::Stream_AllocatorHeap_T ()
 : inherited ()
 , inherited2 ()
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::Stream_AllocatorHeap_T"));

}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
void*
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::calloc (size_t bytes_in,
                                                   char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::calloc"));

  // delegate to base class
  void* result = inherited2::calloc (bytes_in,
                                     initialValue_in);

  // update allocation counter
  if (result)
    poolSize_ += static_cast<unsigned long> (bytes_in);

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
void*
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::calloc (size_t numberOfElements_in,
                                                   size_t sizePerElement_in,
                                                   char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::calloc"));

  // delegate to base class
  void* result = inherited2::calloc (numberOfElements_in,
                                     sizePerElement_in,
                                     initialValue_in);

  // update allocation counter
  if (result)
    poolSize_ +=
      static_cast<unsigned long> (numberOfElements_in * sizePerElement_in);

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
void*
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::calloc"));

  // delegate to base class
  void* result = inherited2::calloc (1,
                                     sizeof (ACE_Message_Block),
                                     '\0');

  // update allocation counter
  if (result)
    poolSize_ += sizeof (ACE_Message_Block);

  return result;
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType>
void*
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::malloc"));

  // delegate to base class
  void* result = inherited2::malloc (bytes_in);

  // update allocation counter
  if (likely (result))
    poolSize_ += static_cast<unsigned long> (bytes_in);

  return result;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
void
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::free"));

  // delegate to base class
  return inherited2::free (handle_in);

  // *TODO*: how can this counter update ???
  // update allocation counter
//   poolSize_ -= bytes_in;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType>
void
Stream_AllocatorHeap_T<ACE_SYNCH_USE,
                       ConfigurationType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap_T::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated heap space: %u byte(s)\n"),
              poolSize_.value ()));
}
