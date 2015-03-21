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

#include "stream_allocatorheap.h"

#include "ace/Log_Msg.h"

#include "stream_macros.h"

Stream_AllocatorHeap::Stream_AllocatorHeap ()
 : inherited ()
 , poolSize_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::Stream_AllocatorHeap"));

}

Stream_AllocatorHeap::~Stream_AllocatorHeap ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::~Stream_AllocatorHeap"));

}

void*
Stream_AllocatorHeap::calloc (size_t bytes_in,
                              char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::calloc"));

  // delegate to base class
  void* result = inherited::calloc (bytes_in,
                                    initialValue_in);

  // update allocation counter ?
  if (result)
    poolSize_ += bytes_in;

  return result;
}

void*
Stream_AllocatorHeap::calloc (size_t numElements_in,
                              size_t sizePerElement_in,
                              char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::calloc"));

  // delegate to base class
  void* result = inherited::calloc (numElements_in,
                                    sizePerElement_in,
                                    initialValue_in);

  // update allocation counter ?
  if (result)
    poolSize_ += (numElements_in * sizePerElement_in);

  return result;
}

bool
Stream_AllocatorHeap::block ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::block"));

  return false;
}

void*
Stream_AllocatorHeap::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::malloc"));

  // delegate to base class
  void* result = inherited::malloc (bytes_in);

  // update allocation counter ?
  if (result)
    poolSize_ += bytes_in;

  return result;
}

void
Stream_AllocatorHeap::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::free"));

  // delegate to base class
  return inherited::free (handle_in);

  // *TODO*: how can this counter update ???
  // update allocation counter
//   poolSize_ -= bytes_in;
}

size_t
Stream_AllocatorHeap::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::cache_depth"));

  // *TODO*: specify a maximum size
  return -1;
}

size_t
Stream_AllocatorHeap::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::cache_size"));

  // *TODO*: how can this counter update (see free()) ???
  return poolSize_.value ();
}

void
Stream_AllocatorHeap::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::dump_state"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocated heap space: %u byte(s)\n"),
              poolSize_.value ()));
}

int
Stream_AllocatorHeap::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::remove"));

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::bind (const char* name_in,
                            void* pointer_in,
                            int duplicates_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::bind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);
  ACE_UNUSED_ARG (duplicates_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::trybind (const char* name_in,
                               void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::trybind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::find (const char* name_in,
                            void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::find"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::find (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::unbind (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::unbind"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::unbind (const char* name_in,
                              void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::unbind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::sync (ssize_t length_in,
                            int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::sync"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::sync (void* address_in,
                            size_t length_in,
                            int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::sync"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::protect (ssize_t length_in,
                               int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::protect"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

int
Stream_AllocatorHeap::protect (void* address_in,
                               size_t length_in,
                               int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_AllocatorHeap::malloc"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}
