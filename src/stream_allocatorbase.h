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

#ifndef STREAM_ALLOCATORBASE_H
#define STREAM_ALLOCATORBASE_H

#include <ace/Global_Macros.h>
#include <ace/Malloc_Base.h>

#include "common_idumpstate.h"
#include "common_iinitialize.h"

#include "stream_iallocator.h"

template <typename ConfigurationType>
class Stream_AllocatorBase_T
 : public Stream_IAllocator
 , public Common_IDumpState
 , public Common_IInitialize_T<ConfigurationType>
 //, virtual public ACE_Allocator
{
 public:
  virtual ~Stream_AllocatorBase_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  ConfigurationType* configuration_;

 protected:
  Stream_AllocatorBase_T ();

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorBase_T (const Stream_AllocatorBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_AllocatorBase_T& operator= (const Stream_AllocatorBase_T&))

  // implement (part of) ACE_Allocator
  virtual void* calloc (size_t,
                        size_t,
                        char = '\0');

  virtual int remove (void);

  virtual int bind (const char*, void*, int = 0);
  virtual int trybind (const char*, void*&);
  virtual int find (const char*, void*&);
  virtual int find (const char*);
  virtual int unbind (const char*);
  virtual int unbind (const char*, void*&);

  virtual int sync (ssize_t = -1, int = MS_SYNC);
  virtual int sync (void*, size_t, int = MS_SYNC);

  virtual int protect (ssize_t = -1, int = PROT_RDWR);
  virtual int protect (void*, size_t, int = PROT_RDWR);

#if defined (ACE_HAS_MALLOC_STATS)
  virtual void print_stats (void) const;
#endif /* ACE_HAS_MALLOC_STATS */

  virtual void dump (void) const;
};

// include template definition
#include "stream_allocatorbase.inl"

#endif
