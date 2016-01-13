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

#include "stream_defines.h"
#include "stream_macros.h"

template <typename AllocatorConfigurationType>
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::Stream_DirectShowMessageBase_T (unsigned int requestedSize_in)
 : inherited (requestedSize_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::Stream_DirectShowMessageBase_T"));

}

template <typename AllocatorConfigurationType>
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::Stream_DirectShowMessageBase_T (const Stream_DirectShowMessageBase_T<AllocatorConfigurationType>& message_in)
 : inherited (message_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::Stream_DirectShowMessageBase_T"));

}

template <typename AllocatorConfigurationType>
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::Stream_DirectShowMessageBase_T (ACE_Data_Block* dataBlock_in,
                                                                                            ACE_Allocator* messageAllocator_in,
                                                                                            bool incrementMessageCounter_in)
 : inherited (dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::Stream_DirectShowMessageBase_T"));

}

template <typename AllocatorConfigurationType>
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::Stream_DirectShowMessageBase_T (ACE_Allocator* messageAllocator_in)
 : inherited (messageAllocator_in) // re-use the same allocator
 , timeStamp_ (0.0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::Stream_DirectShowMessageBase_T"));

}

template <typename AllocatorConfigurationType>
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::~Stream_DirectShowMessageBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::~Stream_DirectShowMessageBase_T"));

  timeStamp_ = 0.0;
}

//template <typename AllocatorConfigurationType>
//void
//Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::initialize (DataType& data_in,
//                                                                        ACE_Data_Block* dataBlock_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::initialize"));
//
//  data_ = data_in;
//
//  // set data block (if any)
//  if (dataBlock_in)
//  {
//    inherited::initialize (dataBlock_in);
//
//    // (re)set correct message type
//    inherited::msg_type (STREAM_MESSAGE_OBJECT);
//  } // end IF
//
//  initialized_ = true;
//}

template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::GetPointer (BYTE** data_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::GetPointer"));

  // sanity check(s)
  ACE_ASSERT (data_out);
  ACE_ASSERT (!*data_out);

  *data_out = reinterpret_cast<BYTE*> (inherited::rd_ptr ());

  return S_OK;
}
template <typename AllocatorConfigurationType>
long STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::GetSize (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::GetSize"));

  return inherited::capacity ();
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::GetTime (REFERENCE_TIME* startTime_out,
                                                                     REFERENCE_TIME* endTime_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::GetTime"));

  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetTime (REFERENCE_TIME* startTime_in,
                                                                     REFERENCE_TIME* endTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetTime"));

  ACE_UNUSED_ARG (startTime_in);
  ACE_UNUSED_ARG (endTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::IsSyncPoint (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::IsSyncPoint"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);
  ACE_NOTREACHED (return S_FALSE;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetSyncPoint (BOOL isSyncPoint_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetSyncPoint"));

  ACE_UNUSED_ARG (isSyncPoint_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::IsPreroll (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::IsPreroll"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);
  ACE_NOTREACHED (return S_FALSE;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetPreroll (BOOL isPreroll_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetPreroll"));

  ACE_UNUSED_ARG (isPreroll_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
long STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::GetActualDataLength (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::GetActualDataLength"));

  return inherited::length ();
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetActualDataLength (long length_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetActualDataLength"));

  inherited::wr_ptr (length_in);

  return S_OK;
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::GetMediaType (AM_MEDIA_TYPE** mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::GetMediaType"));

  ACE_UNUSED_ARG (mediaType_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetMediaType (AM_MEDIA_TYPE* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetMediaType"));

  ACE_UNUSED_ARG (mediaType_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::IsDiscontinuity (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::IsDiscontinuity"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (S_FALSE);
  ACE_NOTREACHED (return S_FALSE;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetDiscontinuity (BOOL isDiscontinuity_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetDiscontinuity"));

  ACE_UNUSED_ARG (isDiscontinuity_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::GetMediaTime (LONGLONG* startTime_out,
                                                                          LONGLONG* endTime_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::GetMediaTime"));

  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::SetMediaTime (LONGLONG* startTime_in,
                                                                          LONGLONG* endTime_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::SetMediaTime"));

  ACE_UNUSED_ARG (startTime_in);
  ACE_UNUSED_ARG (endTime_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
HRESULT STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::QueryInterface (REFIID GUID_in,
                                                                            void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::QueryInterface"));

  ACE_UNUSED_ARG (GUID_in);
  ACE_UNUSED_ARG (interface_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}
template <typename AllocatorConfigurationType>
ULONG STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::AddRef (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::AddRef"));

  ACE_Message_Block::duplicate ();

  return inherited::reference_count ();
}
template <typename AllocatorConfigurationType>
ULONG STDMETHODCALLTYPE
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::Release (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::Release"));

  ULONG reference_count = inherited::reference_count ();

  inherited::release ();

  return --reference_count;
}

template <typename AllocatorConfigurationType>
void
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename AllocatorConfigurationType>
ACE_Message_Block*
Stream_DirectShowMessageBase_T<AllocatorConfigurationType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_DirectShowMessageBase_T::duplicate"));

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

    // *NOTE*: if "this" is initialized, so is the "clone" (and vice-versa)...

  return message_p;
}
