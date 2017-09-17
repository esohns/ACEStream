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

#ifndef STREAM_IALLOCATOR_H
#define STREAM_IALLOCATOR_H

#include <cstddef>

//#include "ace/Malloc_Base.h"

class Stream_IAllocatorStatistic
{
 public:
  // *NOTE*: informational: total size (memory/chunk/...) of the cache (i.e.
  //         when empty)
  virtual size_t cache_depth () const = 0;
  // *NOTE*: informational: current size (memory/chunk/...) of the cache
  virtual size_t cache_size () const = 0;
};

// *NOTE*: this cannot inherit from ACE_Allocator directly, as it is not
//         consistenly virtually-inherited in all implementations
//         --> use dynamic "sidecasts"
class Stream_IAllocator
 //: virtual public ACE_Allocator
 : public Stream_IAllocatorStatistic
{
 public:
  virtual bool block () = 0; // return value: block when pool is full ?

  // *IMPORTANT NOTE*: calloc/malloc return ACE_Message_Block* (or NULL) that
  //                   may (or may not) be dynamic_cast-able to derived types !

  // allocate control message
  virtual void* calloc () = 0;
  // allocate data/session message
  virtual void* malloc (size_t) = 0; // bytes (? data- : session message)
  virtual void free (void*) = 0; // handle
};

#endif
