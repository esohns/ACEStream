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

#include "ace/Assert.h"
#include "ace/config-lite.h"
#include "ace/Log_Msg.h"
#include "ace/OS_Memory.h"

#include "stream_defines.h"
#include "stream_macros.h"

template <typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::Stream_MediaFramework_DirectShow_MessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                             unsigned int requestedSize_in)
 : inherited (sessionId_in,
              requestedSize_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::Stream_MediaFramework_DirectShow_MessageBase_T"));

}

template <typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::Stream_MediaFramework_DirectShow_MessageBase_T (const Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                                                                                                                                                  CommandType>& message_in)
 : inherited (message_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::Stream_MediaFramework_DirectShow_MessageBase_T"));

}

template <typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::Stream_MediaFramework_DirectShow_MessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                             ACE_Data_Block* dataBlock_in,
                                                                                                             ACE_Allocator* messageAllocator_in,
                                                                                                             bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::Stream_MediaFramework_DirectShow_MessageBase_T"));

}

template <typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::Stream_MediaFramework_DirectShow_MessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                             ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message allocator
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::Stream_MediaFramework_DirectShow_MessageBase_T"));

}

template <typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::~Stream_MediaFramework_DirectShow_MessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::~Stream_MediaFramework_DirectShow_MessageBase_T"));

  timeStamp_ = 0.0;
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::GetPointer (BYTE** data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::GetPointer"));

  // sanity check(s)
  ACE_ASSERT (data_out);
  ACE_ASSERT (!*data_out);

  *data_out = reinterpret_cast<BYTE*> (inherited::rd_ptr ());

  return S_OK;
}

template <typename MessageType,
          typename CommandType>
long STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::GetSize (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::GetSize"));

  return inherited::capacity ();
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::GetTime (REFERENCE_TIME* startTime_out,
                                                                      REFERENCE_TIME* endTime_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::GetTime"));

  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetTime (REFERENCE_TIME* startTime_in,
                                                                      REFERENCE_TIME* endTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetTime"));

  ACE_UNUSED_ARG (startTime_in);
  ACE_UNUSED_ARG (endTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::IsSyncPoint (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::IsSyncPoint"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);

  ACE_NOTREACHED (return S_FALSE;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetSyncPoint (BOOL isSyncPoint_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetSyncPoint"));

  ACE_UNUSED_ARG (isSyncPoint_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::IsPreroll (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::IsPreroll"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);

  ACE_NOTREACHED (return S_FALSE;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetPreroll (BOOL isPreroll_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetPreroll"));

  ACE_UNUSED_ARG (isPreroll_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
long STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::GetActualDataLength (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::GetActualDataLength"));

  return inherited::length ();
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetActualDataLength (long length_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetActualDataLength"));

  inherited::wr_ptr (length_in);

  return S_OK;
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::GetMediaType (AM_MEDIA_TYPE** mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::GetMediaType"));

  ACE_UNUSED_ARG (mediaType_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetMediaType (AM_MEDIA_TYPE* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetMediaType"));

  ACE_UNUSED_ARG (mediaType_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::IsDiscontinuity (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::IsDiscontinuity"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);

  ACE_NOTREACHED (return S_FALSE;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetDiscontinuity (BOOL isDiscontinuity_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetDiscontinuity"));

  ACE_UNUSED_ARG (isDiscontinuity_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::GetMediaTime (LONGLONG* startTime_out,
                                                                           LONGLONG* endTime_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::GetMediaTime"));

  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::SetMediaTime (LONGLONG* startTime_in,
                                                                           LONGLONG* endTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::SetMediaTime"));

  ACE_UNUSED_ARG (startTime_in);
  ACE_UNUSED_ARG (endTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::QueryInterface (REFIID GUID_in,
                                                                             void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::QueryInterface"));

  ACE_UNUSED_ARG (GUID_in);
  ACE_UNUSED_ARG (interface_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename MessageType,
          typename CommandType>
ULONG STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::AddRef (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::AddRef"));

  ACE_Message_Block::duplicate ();

  return inherited::reference_count ();
}

template <typename MessageType,
          typename CommandType>
ULONG STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::Release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::Release"));

  ULONG reference_count = inherited::reference_count ();

  inherited::release ();

  return --reference_count;
}

template <typename MessageType,
          typename CommandType>
void
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename MessageType,
          typename CommandType>
ACE_Message_Block*
Stream_MediaFramework_DirectShow_MessageBase_T<MessageType,
                                               CommandType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_MessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block...
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}

//////////////////////////////////////////

template <typename DataType,
          typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::Stream_MediaFramework_DirectShow_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                                     unsigned int requestedSize_in)
 : inherited (sessionId_in,
              requestedSize_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::Stream_MediaFramework_DirectShow_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::Stream_MediaFramework_DirectShow_DataMessageBase_T (const Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                                                                                                                                              MessageType,
                                                                                                                                                                              CommandType>& message_in)
 : inherited (message_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::Stream_MediaFramework_DirectShow_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::Stream_MediaFramework_DirectShow_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                                     ACE_Data_Block* dataBlock_in,
                                                                                                                     ACE_Allocator* messageAllocator_in,
                                                                                                                     bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::Stream_MediaFramework_DirectShow_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::Stream_MediaFramework_DirectShow_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                                     ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message allocator
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::Stream_MediaFramework_DirectShow_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType,
          typename CommandType>
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::~Stream_MediaFramework_DirectShow_DataMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::~Stream_MediaFramework_DirectShow_DataMessageBase_T"));

  timeStamp_ = 0.0;
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::GetPointer (BYTE** data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::GetPointer"));

  // sanity check(s)
  ACE_ASSERT (data_out);
  ACE_ASSERT (!*data_out);

  *data_out = reinterpret_cast<BYTE*> (inherited::rd_ptr ());

  return S_OK;
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
long STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::GetSize (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::GetSize"));

  return inherited::capacity ();
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::GetTime (REFERENCE_TIME* startTime_out,
                                                                          REFERENCE_TIME* endTime_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::GetTime"));

  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetTime (REFERENCE_TIME* startTime_in,
                                                                          REFERENCE_TIME* endTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetTime"));

  ACE_UNUSED_ARG (startTime_in);
  ACE_UNUSED_ARG (endTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::IsSyncPoint (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::IsSyncPoint"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);

  ACE_NOTREACHED (return S_FALSE;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetSyncPoint (BOOL isSyncPoint_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetSyncPoint"));

  ACE_UNUSED_ARG (isSyncPoint_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::IsPreroll (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::IsPreroll"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);

  ACE_NOTREACHED (return S_FALSE;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetPreroll (BOOL isPreroll_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetPreroll"));

  ACE_UNUSED_ARG (isPreroll_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
long STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::GetActualDataLength (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::GetActualDataLength"));

  return inherited::length ();
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetActualDataLength (long length_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetActualDataLength"));

  inherited::wr_ptr (length_in);

  return S_OK;
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::GetMediaType (AM_MEDIA_TYPE** mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::GetMediaType"));

  ACE_UNUSED_ARG (mediaType_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetMediaType (AM_MEDIA_TYPE* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetMediaType"));

  ACE_UNUSED_ARG (mediaType_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::IsDiscontinuity (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::IsDiscontinuity"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);

  ACE_NOTREACHED (return S_FALSE;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetDiscontinuity (BOOL isDiscontinuity_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetDiscontinuity"));

  ACE_UNUSED_ARG (isDiscontinuity_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::GetMediaTime (LONGLONG* startTime_out,
                                                                               LONGLONG* endTime_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::GetMediaTime"));

  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::SetMediaTime (LONGLONG* startTime_in,
                                                                               LONGLONG* endTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::SetMediaTime"));

  ACE_UNUSED_ARG (startTime_in);
  ACE_UNUSED_ARG (endTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
HRESULT STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::QueryInterface (REFIID GUID_in,
                                                                                 void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::QueryInterface"));

  ACE_UNUSED_ARG (GUID_in);
  ACE_UNUSED_ARG (interface_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
ULONG STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::AddRef (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::AddRef"));

  ACE_Message_Block::duplicate ();

  return inherited::reference_count ();
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
ULONG STDMETHODCALLTYPE
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::Release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::Release"));

  ULONG reference_count = inherited::reference_count ();

  inherited::release ();

  return --reference_count;
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
void
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename DataType,
          typename MessageType,
          typename CommandType>
ACE_Message_Block*
Stream_MediaFramework_DirectShow_DataMessageBase_T<DataType,
                                                   MessageType,
                                                   CommandType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_DataMessageBase_T::duplicate"));

  OWN_TYPE_T* message_p = NULL;

  // create a new Stream_MessageBase that contains unique copies of
  // the message block fields, but a (reference counted) shallow duplicate of
  // the ACE_Data_Block

  // if there is no allocator, use the standard new and delete calls.
  if (inherited::message_block_allocator_ == NULL)
    ACE_NEW_NORETURN (message_p,
                      OWN_TYPE_T (*this));
  else // otherwise, use the existing message_block_allocator
  {
    // *NOTE*: the argument to malloc SHOULDN'T really matter, as this will be
    //         a "shallow" copy which just references the same data block...
    ACE_NEW_MALLOC_NORETURN (message_p,
                             static_cast<OWN_TYPE_T*> (inherited::message_block_allocator_->calloc (inherited::capacity (),
                                                                                                    '\0')),
                             OWN_TYPE_T (*this));
  } // end ELSE
  if (!message_p)
  {
    Stream_IAllocator* allocator_p =
      dynamic_cast<Stream_IAllocator*> (inherited::message_block_allocator_);
    ACE_ASSERT (allocator_p);
    if (allocator_p->block ())
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate Stream_MessageBase_T: \"%m\", aborting\n")));
    return NULL;
  } // end IF

    // increment the reference counts of any continuation messages
  if (inherited::cont_)
  {
    message_p->cont_ = inherited::cont_->duplicate ();
    if (!message_p->cont_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MessageBase_T::duplicate(): \"%m\", aborting\n")));

      // clean up
      message_p->release ();

      return NULL;
    } // end IF
  } // end IF

  // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)

  return message_p;
}
