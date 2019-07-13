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

#ifndef STREAM_CACHEDMESSAGEALLOCATORHEAP_BASE_H
#define STREAM_CACHEDMESSAGEALLOCATORHEAP_BASE_H

#include "ace/Global_Macros.h"
#include "ace/Malloc_T.h"
#include "ace/Message_Block.h"
#include "ace/Synch_Traits.h"

#include "stream_iallocator.h"
#include "stream_cacheddatablockallocatorheap.h"

template <typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_CachedMessageAllocatorHeapBase_T
 : public ACE_Cached_Allocator<DataMessageType,
                               ACE_SYNCH_MUTEX>
 , public Stream_IAllocator
{
  typedef ACE_Cached_Allocator<DataMessageType,
                               ACE_SYNCH_MUTEX> inherited;

 public:
  Stream_CachedMessageAllocatorHeapBase_T (unsigned int = STREAM_QUEUE_DEFAULT_CACHED_MESSAGES); // total number of concurrent messages {0: STREAM_QUEUE_DEFAULT_CACHED_MESSAGES}
  inline virtual ~Stream_CachedMessageAllocatorHeapBase_T () {}

  // implement Stream_IAllocator
  inline virtual bool block () { return false; } // return value: block when full ?
  virtual void* calloc ();
  // *NOTE*: returns a pointer to <MessageType>
  // *NOTE: passing a value of 0 will return a <SessionMessageType>
  // *TODO*: the way message IDs are implemented, they can be truly unique
  // only IF allocation is synchronized
  virtual void* malloc (size_t); // bytes
  virtual void free (void*); // handle
  virtual size_t cache_depth () const; // return value: #bytes allocated
  virtual size_t cache_size  () const; // return value: #inflight ACE_Message_Blocks

  //// *NOTE*: returns a pointer to <MessageType>/<SessionMessageType>
  ////         --> see above
  //virtual void* calloc (size_t,       // bytes
  //                      char = '\0'); // initial value

 private:
  typedef Stream_CachedMessageAllocatorHeapBase_T<ControlMessageType,
                                                  DataMessageType,
                                                  SessionMessageType> OWN_TYPE_T;
   
  ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocatorHeapBase_T (const Stream_CachedMessageAllocatorHeapBase_T&))
  // *NOTE*: apparently, ACE_UNIMPLEMENTED_FUNC gets confused with more than one template parameter
//   ACE_UNIMPLEMENTED_FUNC (Stream_CachedMessageAllocatorHeapBase_T<MessageType,
//                                                          SessionMessageType>& operator= (const Stream_CachedMessageAllocatorHeapBase_T<MessageType,
//                                                                                          SessionMessageType>&))

//  virtual void* calloc (size_t,
//                        size_t,
//                        char = '\0');
//  virtual void free (void*); // element handle
//  virtual int remove (void);
//
//  virtual int bind (const char*, void*, int = 0);
//  virtual int trybind (const char*, void*&);
//  virtual int find (const char*, void*&);
//  virtual int find (const char*);
//  virtual int unbind (const char*);
//  virtual int unbind (const char*, void*&);
//
//  virtual int sync (ssize_t = -1, int = MS_SYNC);
//  virtual int sync (void*, size_t, int = MS_SYNC);
//
//  virtual int protect (ssize_t = -1, int = PROT_RDWR);
//  virtual int protect (void*, size_t, int = PROT_RDWR);
//
//#if defined (ACE_HAS_MALLOC_STATS)
//  virtual void print_stats (void) const;
//#endif /* ACE_HAS_MALLOC_STATS */
//
//  virtual void dump (void) const;

  ACE_Cached_Allocator<ACE_Data_Block,
                       ACE_SYNCH_MUTEX> dataBlockAllocator_;
  ACE_Cached_Allocator<ControlMessageType,
                       ACE_SYNCH_MUTEX> controlMessageAllocator_;
  ACE_Cached_Allocator<SessionMessageType,
                       ACE_SYNCH_MUTEX> sessionMessageAllocator_;
};

// include template definition
#include "stream_cachedmessageallocatorheap_base.inl"

#endif
