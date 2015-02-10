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

#ifndef RPG_STREAM_IALLOCATOR_H
#define RPG_STREAM_IALLOCATOR_H

// #include <ace/Malloc_Base.h>

#include <stddef.h>

class RPG_Stream_IAllocator
//  : public virtual ACE_Allocator
{
 public:
  virtual ~RPG_Stream_IAllocator() {}

  virtual void* malloc(size_t) = 0; // bytes
  virtual void free(void*) = 0; // handle

  // *NOTE*: informational: current size (memory/chunk/...) of the cache
  virtual size_t cache_depth(void) const = 0;
  // *NOTE*: informational: total size (memory/chunk/...) of the cache
  virtual size_t cache_size(void) const = 0;
};

#endif
