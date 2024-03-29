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

template <typename DataType,
          typename MessageType>
Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                        MessageType>::Stream_MediaFramework_MediaFoundation_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                                               unsigned int requestedSize_in)
 : inherited (sessionId_in,
              requestedSize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::Stream_MediaFramework_MediaFoundation_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType>
Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                        MessageType>::Stream_MediaFramework_MediaFoundation_DataMessageBase_T (const Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                                                                                                                                                             MessageType>& message_in)
 : inherited (message_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::Stream_MediaFramework_MediaFoundation_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType>
Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                        MessageType>::Stream_MediaFramework_MediaFoundation_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                                               ACE_Data_Block* dataBlock_in,
                                                                                                                               ACE_Allocator* messageAllocator_in,
                                                                                                                               bool incrementMessageCounter_in)
 : inherited (sessionId_in,
              dataBlock_in,
              messageAllocator_in,
              incrementMessageCounter_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::Stream_MediaFramework_MediaFoundation_DataMessageBase_T"));

}

template <typename DataType,
          typename MessageType>
Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                        MessageType>::Stream_MediaFramework_MediaFoundation_DataMessageBase_T (Stream_SessionId_t sessionId_in,
                                                                                                                               ACE_Allocator* messageAllocator_in)
 : inherited (sessionId_in,
              messageAllocator_in) // message allocator
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::Stream_MediaFramework_MediaFoundation_DataMessageBase_T"));

}

//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::GetSampleFlags (DWORD* flags_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::GetSampleFlags"));
//
//  // sanity check(s)
//  ACE_ASSERT (flags_out);
//  ACE_ASSERT (!*flags_out);
//
//  *flags_out = *reinterpret_cast<DWORD*> (inherited::rd_ptr ());
//
//  return S_OK;
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::SetSampleFlags (DWORD flags_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::SetSampleFlags"));
//
//  ACE_UNUSED_ARG (flags_in);
//
//  return S_OK;
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::GetSampleTime (LONGLONG* sampleTime_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::GetSampleTime"));
//
//  ACE_UNUSED_ARG (sampleTime_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::SetSampleTime (LONGLONG sampleTime_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::SetSampleTime"));
//
//  ACE_UNUSED_ARG (sampleTime_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::GetSampleDuration (LONGLONG* sampleDuration_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::GetSampleDuration"));
//
//  ACE_UNUSED_ARG (sampleDuration_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_FALSE);
//  ACE_NOTREACHED (return S_FALSE;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::SetSampleDuration (LONGLONG sampleDuration_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::SetSampleDuration"));
//
//  ACE_UNUSED_ARG (sampleDuration_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::GetBufferCount (DWORD* bufferCount_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::GetBufferCount"));
//
//  ACE_UNUSED_ARG (bufferCount_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_FALSE);
//  ACE_NOTREACHED (return S_FALSE;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::GetBufferByIndex (DWORD index_in,
//                                                                 IMFMediaBuffer** mediaBuffer_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::GetBufferByIndex"));
//
//  ACE_UNUSED_ARG (index_in);
//  ACE_UNUSED_ARG (mediaBuffer_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::ConvertToContiguousBuffer (IMFMediaBuffer** mediaBuffer_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::ConvertToContiguousBuffer"));
//
//  ACE_UNUSED_ARG (mediaBuffer_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::AddBuffer (IMFMediaBuffer* mediaBuffer_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::AddBuffer"));
//
//  ACE_UNUSED_ARG (mediaBuffer_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::RemoveBufferByIndex (DWORD index_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::RemoveBufferByIndex"));
//
//  ACE_UNUSED_ARG (index_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::RemoveAllBuffers (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::RemoveAllBuffers"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::GetTotalLength (DWORD* totalLength_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::GetTotalLength"));
//
//  ACE_UNUSED_ARG (totalLength_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (S_FALSE);
//  ACE_NOTREACHED (return S_FALSE;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::CopyToBuffer (IMFMediaBuffer* mediaBuffer_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::CopyToBuffer"));
//
//  ACE_UNUSED_ARG (mediaBuffer_in);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//
//template <typename AllocatorConfigurationType,
//          typename DataType>
//HRESULT STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::QueryInterface (REFIID GUID_in,
//                                                               void** interface_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::QueryInterface"));
//
//  ACE_UNUSED_ARG (GUID_in);
//  ACE_UNUSED_ARG (interface_out);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//ULONG STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::AddRef (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::AddRef"));
//
//  ACE_Message_Block::duplicate ();
//
//  return inherited::reference_count ();
//}
//template <typename AllocatorConfigurationType,
//          typename DataType>
//ULONG STDMETHODCALLTYPE
//Stream_MediaFramework_MediaFoundation_DataMessageBase_T<AllocatorConfigurationType,
//                                    DataType>::Release (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::Release"));
//
//  ULONG reference_count = inherited::reference_count ();
//
//  inherited::release ();
//
//  return --reference_count;
//}

template <typename DataType,
          typename MessageType>
void
Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                        MessageType>::dump_state () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::dump_state"));

  ACE_ASSERT (false);
  ACE_NOTSUP;

  ACE_NOTREACHED (return;)
}

template <typename DataType,
          typename MessageType>
ACE_Message_Block*
Stream_MediaFramework_MediaFoundation_DataMessageBase_T<DataType,
                                                        MessageType>::duplicate (void) const
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_DataMessageBase_T::duplicate"));

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
