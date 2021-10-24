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

#ifndef STREAM_LIB_DIRECTSHOW_ALLOCATOR_H
#define STREAM_LIB_DIRECTSHOW_ALLOCATOR_H

#include "mmreg.h"
#include "mmsystem.h"

#undef NANOSECONDS
#include "streams.h"

#include "ace/Global_Macros.h"
#include "ace/Malloc_Base.h"
#include "ace/Synch_Traits.h"

#include "common_idumpstate.h"
#include "common_iinitialize.h"

#include "stream_iallocator.h"
#include "stream_datablockallocatorheap.h"

template <typename ConfigurationType,
          ////////////////////////////////
          typename MessageType,
          typename SessionMessageType>
class Stream_MediaFramework_DirectShow_AllocatorBase_T
 : public CBaseAllocator
 , public ACE_Allocator
 , public Stream_IAllocator
 , public Common_IInitialize_T<ConfigurationType>
 , public Common_IDumpState
{
  typedef CBaseAllocator inherited;

 public:
  Stream_MediaFramework_DirectShow_AllocatorBase_T (TCHAR*,       // name
                                                    LPUNKNOWN,    // aggregating IUnknown interface handle ('owner')
                                                    HRESULT*,     // return value: result

                                                    bool = true); // block until a buffer is available ?
  inline virtual ~Stream_MediaFramework_DirectShow_AllocatorBase_T () {}

  // implement IMemAllocator
  virtual STDMETHODIMP SetProperties (struct _AllocatorProperties*, // requested
                                      struct _AllocatorProperties*); // return value: actual
  virtual STDMETHODIMP GetProperties (struct _AllocatorProperties*); // return value: properties

  virtual STDMETHODIMP Commit (void);
  virtual STDMETHODIMP Decommit (void);

  virtual STDMETHODIMP GetBuffer (IMediaSample**,  // return value: media sample handle
                                  REFERENCE_TIME*, // return value: start time
                                  REFERENCE_TIME*, // return value: end time
                                  DWORD);          // flags
  virtual STDMETHODIMP ReleaseBuffer (IMediaSample*); // media sample handle

  // implement Stream_IAllocator
  inline virtual bool block () { return block_; } // return value: block when pool is empty ?
  // *NOTE*: if argument is > 0, this returns a (pointer to) <MessageType>, and
  //         a (pointer to) <SessionMessageType> otherwise
  virtual void* malloc (size_t); // bytes
                                 // *NOTE*: frees a <MessageType>/<SessionMessageType>
  virtual void free (void*); // element handle
  inline virtual size_t cache_depth () const { return dataBlockAllocator_.cache_depth (); } // return value: #bytes allocated
  inline virtual size_t cache_size () const { return poolSize_.value (); } // return value: #inflight ACE_Message_Blocks

  // implement (part of) ACE_Allocator
  // *NOTE*: returns a pointer to raw memory (!) of size <MessageType>/
  //         <SessionMessageType> --> see above
  // *NOTE*: no data block is allocated
  virtual void* calloc (size_t,       // bytes
                        char = '\0'); // initial value (not used)

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  // implement Common_IDumpState
  inline virtual void dump_state () const { return dataBlockAllocator_.dump_state (); }

 private:
  // convenient types
  //typedef Stream_AllocatorHeap_T<ConfigurationType> HEAP_ALLOCATOR_T;
  typedef Stream_DataBlockAllocatorHeap_T<ACE_MT_SYNCH,
                                          ConfigurationType> DATABLOCK_ALLOCATOR_T;

  //ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_AllocatorBase_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_AllocatorBase_T (const Stream_MediaFramework_DirectShow_AllocatorBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_AllocatorBase_T& operator= (const Stream_MediaFramework_DirectShow_AllocatorBase_T&))

  // these methods are ALL no-ops and will FAIL !
  // *NOTE*: this method is a no-op and just returns NULL
  // since the free list only works with fixed sized entities
  inline virtual void* calloc (size_t, size_t, char = '\0') { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return -1;) }
  inline virtual int remove (void) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int bind (const char*, void*, int = 0) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int trybind (const char*, void*&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int find (const char*, void*&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int find (const char*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int unbind (const char*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int unbind (const char*, void*&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int sync (ssize_t = -1, int = MS_SYNC) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int sync (void*, size_t, int = MS_SYNC) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int protect (ssize_t = -1, int = PROT_RDWR) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }
  inline virtual int protect (void*, size_t, int = PROT_RDWR) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (-1); ACE_NOTREACHED (return -1;) }

  bool                  block_;
  DATABLOCK_ALLOCATOR_T dataBlockAllocator_;
  //ACE_SYNCH_SEMAPHORE   freeMessageCounter_;
};

////////////////////////////////////////////////////////////////////////////////

//template <typename ConfigurationType,
//          ////////////////////////////////
//          typename MessageType,
//          typename SessionMessageType>
//class Stream_MediaFramework_DirectShow_HeapAllocatorBase_T
// : public CMemAllocator
// , public ACE_Allocator
// , public Stream_IAllocator
// , public Common_IInitialize_T<ConfigurationType>
// , public Common_IDumpState
//{
// public:
//  Stream_MediaFramework_DirectShow_HeapAllocatorBase_T ();
//  virtual ~Stream_MediaFramework_DirectShow_HeapAllocatorBase_T ();
//
//  // implement Common_IInitialize_T
//  virtual bool initialize (const ConfigurationType&);
//
// private:
//  typedef CMemAllocator inherited;
//
//  //ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_HeapAllocatorBase_T ())
//  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_HeapAllocatorBase_T (const Stream_MediaFramework_DirectShow_HeapAllocatorBase_T&))
//  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_HeapAllocatorBase_T& operator= (const Stream_MediaFramework_DirectShow_HeapAllocatorBase_T&))
//};

// include template implementation
#include "stream_lib_directshow_allocator.inl"

#endif
