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

#include "stream_defines.h"
#include "stream_macros.h"

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::Stream_CachedMessageAllocator_T (unsigned int maximumNumberOfMessages_in,
                                                                                      ACE_Allocator* allocator_in,
                                                                                      bool block_in)
 : inherited ()
 , block_ (block_in)
 , dataBlockAllocator_ ((maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                    : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES),
                        allocator_in)
 , controlMessageAllocator_ (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                        : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
 , dataMessageAllocator_ (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                     : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
 , sessionMessageAllocator_ (maximumNumberOfMessages_in ? maximumNumberOfMessages_in
                                                        : STREAM_QUEUE_DEFAULT_CACHED_MESSAGES)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::Stream_CachedMessageAllocator_T"));

  // sanity check(s)
  if (!maximumNumberOfMessages_in)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot allocate unlimited memory, caching %d buffers...\n"),
                STREAM_QUEUE_DEFAULT_CACHED_MESSAGES));
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::calloc ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::calloc"));

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (0)));
  }
  catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_RETURN(ACE_Data_Block), continuing\n")));
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate ACE_Data_Block(0), aborting\n")));
    return NULL;
  } // end IF

  // *NOTE*: must clean up data block beyond this point !

  // step2: get free message
  ACE_Message_Block* message_p = NULL;
  try {
    // allocate memory and perform a placement new by invoking a ctor on the
    // allocated space
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<ControlMessageType*> (controlMessageAllocator_.malloc (sizeof (ControlMessageType))),
                             ControlMessageType (data_block_p, // use the data block just allocated
                                                 this));       // remember allocator upon destruction
  }
  catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN(ControlMessageType(), continuing\n")));
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ControlMessageType(): \"%m\", aborting\n")));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // ... and return the result
  return message_p;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::malloc"));

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_RETURN(%u), continuing\n"),
                bytes_in));
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    return NULL;
  } // end IF

  // *NOTE*: must clean up data block beyond this point !

  // step2: get free message
  ACE_Message_Block* message_p = NULL;
  try {
    // allocate memory and perform a placement new by invoking a ctor on the
    // allocated space
    if (bytes_in)
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<DataMessageType*> (dataMessageAllocator_.malloc (sizeof (DataMessageType))),
                               DataMessageType (0,            // session id
                                                data_block_p, // use the data block just allocated
                                                this,         // remember allocator upon destruction
                                                true));       // increment message counter ?
    else
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<SessionMessageType*> (sessionMessageAllocator_.malloc (sizeof (SessionMessageType))),
                               SessionMessageType (0,            // session id
                                                   data_block_p, // use the data block just allocated
                                                   this));       // remember allocator upon destruction
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN([Session]MessageType(%u), continuing\n"),
                bytes_in));
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate [Session]MessageType(%u): \"%m\", aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // ... and return the result
  // *NOTE*: the caller knows what to expect (either MessageType ||
  //         SessionMessageType)
  return message_p;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::calloc (size_t bytes_in,
                                                             char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::calloc"));

  void* result_p = NULL;
  try {
    // allocate memory only
    if (bytes_in)
      ACE_ALLOCATOR_NORETURN (result_p,
                              dataMessageAllocator_.calloc (sizeof (DataMessageType),
                                                            initialValue_in));
    else
      ACE_ALLOCATOR_NORETURN (result_p,
                              sessionMessageAllocator_.calloc (sizeof (SessionMessageType),
                                                               initialValue_in));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN([Session]MessageType(%u), continuing\n"),
                bytes_in));
    result_p = NULL;
  }
  if (!result_p)
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate [Session]MessageType(%u): \"%m\", aborting\n"),
                bytes_in));

  return result_p;
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::free"));

  // *NOTE*: distinguish between different message types here

  ACE_Message_Block* message_block_p =
      static_cast<ACE_Message_Block*> (handle_in);
  ACE_ASSERT (message_block_p);

  // *WARNING*: cannot access the message type (data block has already gone)
  //switch (message_block_p->msg_type ())
  switch (message_block_p->msg_priority ())
  {
    case 0:
      sessionMessageAllocator_.free (handle_in);
      break;
    //case ACE_Message_Block::MB_NORMAL: // undifferentiated
    //case ACE_Message_Block::MB_BREAK:
    //case ACE_Message_Block::MB_FLUSH:
    //case ACE_Message_Block::MB_HANGUP:
    case STREAM_MESSAGE_CONTROL_PRIORITY:
      controlMessageAllocator_.free (handle_in);
      break;
    //case ACE_Message_Block::MB_DATA:
    //case ACE_Message_Block::MB_PROTO:
    case UINT32_MAX:
      dataMessageAllocator_.free (handle_in);
      break;
    //case ACE_Message_Block::MB_USER:
    default:
    {
      //ACE_DEBUG ((LM_ERROR,
      //            ACE_TEXT ("invalid/unknown message type (was: %d), returning\n"),
      //            message_block_p->msg_type ()));
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown message priority (was: %d), returning\n"),
                  message_block_p->msg_priority ()));
      break;
    }
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
size_t
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::cache_depth () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::cache_depth"));

  return dataBlockAllocator_.cache_depth ();
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
size_t
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::cache_size () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::cache_size"));

  return dataBlockAllocator_.cache_size ();
}

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void*
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::calloc (size_t,
                                                             size_t,
                                                             char)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::calloc"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (NULL);

  ACE_NOTREACHED (return NULL;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::remove"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::bind (const char*, void*, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::bind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::trybind (const char*, void*&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::trybind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::find (const char*, void*&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::find"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::find (const char*)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::find"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::unbind (const char*)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::unbind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::unbind (const char*, void*&)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::unbind"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::sync (ssize_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::sync"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::sync (void*, size_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::sync"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::protect (ssize_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::protect"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
int
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::protect (void*, size_t, int)
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::protect"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (-1);

  ACE_NOTREACHED (return -1;)
}

//////////////////////////////////////////

#if defined (ACE_HAS_MALLOC_STATS)
template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::print_stats (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::print_stats"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
#endif /* ACE_HAS_MALLOC_STATS */

template <ACE_SYNCH_DECL,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
void
Stream_CachedMessageAllocator_T<ACE_SYNCH_USE,
                                ConfigurationType,
                                ControlMessageType,
                                DataMessageType,
                                SessionMessageType>::dump (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_CachedMessageAllocator_T::dump"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}
