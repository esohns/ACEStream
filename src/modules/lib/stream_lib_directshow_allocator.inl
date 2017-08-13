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
#include "ace/Message_Block.h"

#include "stream_macros.h"

//#include "stream_lib_defines.h"

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::Stream_MediaFramework_DirectShow_AllocatorBase_T (TCHAR* name_in,
                                                                                                              LPUNKNOWN IUnknown_in,
                                                                                                              HRESULT* result_out,

                                                                                                              bool block_in)
 : inherited (name_in,
              IUnknown_in,
              result_out,
              true, // use semaphore ? (m_hSem)
              true) // enable callback
 , block_ (block_in)
 , dataBlockAllocator_ (allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::Stream_MediaFramework_DirectShow_AllocatorBase_T"));

}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::~Stream_MediaFramework_DirectShow_AllocatorBase_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::~Stream_MediaFramework_DirectShow_AllocatorBase_T"));

}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::SetProperties (struct _AllocatorProperties* requestedProperties_in,
                                                                                     struct _AllocatorProperties* actualProperties_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::SetProperties"));

  ACE_UNUSED_ARG (requestedProperties_in);
  ACE_UNUSED_ARG (actualProperties_inout);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::GetProperties (struct _AllocatorProperties* properties_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::GetProperties"));

  ACE_UNUSED_ARG (properties_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::Commit ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::Commit"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::Decommit ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::Decommit"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::GetBuffer (IMediaSample** mediaSample_out,
                                                                                 REFERENCE_TIME* startTime_out,
                                                                                 REFERENCE_TIME* endTime_out,
                                                                                 DWORD flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::GetBuffer"));

  ACE_UNUSED_ARG (mediaSample_out);
  ACE_UNUSED_ARG (startTime_out);
  ACE_UNUSED_ARG (endTime_out);
  ACE_UNUSED_ARG (flags_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}
template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
HRESULT
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::ReleaseBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::ReleaseBuffer"));

  ACE_UNUSED_ARG (mediaSample_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void*
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::malloc (size_t bytes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::malloc"));

  int result = -1;
  // step0: wait for an empty slot ?
  if (block_)
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (result == -1)
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_SEMAPHORE::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  poolSize_++;

  // step1: get free data block
  ACE_Data_Block* data_block_p = NULL;
  try {
    ACE_ALLOCATOR_NORETURN (data_block_p,
                            static_cast<ACE_Data_Block*> (dataBlockAllocator_.malloc (bytes_in)));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_ALLOCATOR_NORETURN(ACE_Data_Block(%u)), aborting\n"),
                bytes_in));
    return NULL;
  }
  if (!data_block_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate ACE_Data_Block(%u), aborting\n"),
                bytes_in));
    return NULL;
  } // end IF

  // *NOTE*: must clean up data block beyond this point !

  // step2: allocate message
  MessageType* message_p = NULL;
  SessionMessageType* session_message_p = NULL;
  try {
    // allocate memory and perform a placement new by invoking a ctor
    // on the allocated space
    if (bytes_in)
      ACE_NEW_MALLOC_NORETURN (message_p,
                               static_cast<MessageType*> (inherited::malloc (sizeof (MessageType))),
                               MessageType (data_block_p, // use the newly allocated data block
                                            this));       // message allocator
    else
      ACE_NEW_MALLOC_NORETURN (session_message_p,
                               static_cast<SessionMessageType*> (inherited::malloc (sizeof (SessionMessageType))),
                               SessionMessageType (data_block_p, // use the newly allocated data block
                                                   this));       // message allocator
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_NEW_MALLOC_NORETURN((Session)MessageType(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  }
  if (!message_p && ! session_message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate (Session)MessageType(%u), aborting\n"),
                bytes_in));

    // clean up
    data_block_p->release ();

    return NULL;
  } // end IF

  // *NOTE*: the caller knows what to expect; MessageType or SessionMessageType
  if (bytes_in)
    return message_p;
  return session_message_p;
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void*
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::calloc (size_t bytes_in,
                                                                              char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::calloc"));

  ACE_UNUSED_ARG (initialValue_in);

  int result = -1;
  // step0: wait for an empty slot ?
  if (block_)
    result = freeMessageCounter_.acquire ();
  else
    result = freeMessageCounter_.tryacquire ();
  if (result == -1)
  {
    if (block_)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Semaphore::acquire(): \"%m\", aborting\n")));
    return NULL;
  } // end IF
  poolSize_++;

  // step1: allocate free message...
  void* message_p = NULL;
  try {
    message_p = inherited::malloc ((bytes_in ? sizeof (MessageType)
                                             : sizeof (SessionMessageType)));
  } catch (...) {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("caught exception in ACE_New_Allocator::malloc(%u), aborting\n"),
                (bytes_in ? sizeof (MessageType)
                          : sizeof (SessionMessageType))));
    return NULL;
  }
  if (!message_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("unable to allocate (Session)MessageType(%u), aborting\n"),
                (bytes_in ? sizeof (MessageType)
                          : sizeof (SessionMessageType))));
    return NULL;
  } // end IF

  // ... and return the result
  // *NOTE*: the caller knows what to expect (either MessageType ||
  //         SessionMessageType)
  return message_p;
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::free (void* handle_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::free"));

  int result = -1;

  inherited::free (handle_in);

  // OK: one slot just emptied
  poolSize_--;
  result = freeMessageCounter_.release ();
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_SYNCH_SEMAPHORE::release(): \"%m\", continuing\n")));
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
bool
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::initialize"));

  // sanity check(s)
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.filterConfiguration);
  ACE_ASSERT (configuration_in.filterConfiguration->pinConfiguration);

  // initialize COM ?
  HRESULT result = E_FAIL;
  static bool first_run = true;
  bool COM_initialized = false;
  if (first_run)
  {
    first_run = false;

    result = CoInitializeEx (NULL, COINIT_MULTITHREADED);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
        ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
        ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    COM_initialized = true;
  } // end IF

  if (isInitialized_)
  {
    // clean up ?
    //if (IMemAllocator_)
    //{
    //  IMemAllocator_->Release ();
    //  IMemAllocator_ = NULL;
    //} // end IF
    //if (IMemInputPin_)
    //{
    //  IMemInputPin_->Release ();
    //  IMemInputPin_ = NULL;
    //} // end IF
    //if (!push_)
    //{
    inherited::queue_.waitForIdleState ();
    //} // end IF

    if (ROTID_)
    {
      IRunningObjectTable* ROT_p = NULL;
      result = GetRunningObjectTable (0, &ROT_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to GetRunningObjectTable() \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        ROTID_ = 0;

        goto continue_;
      } // end IF
      ACE_ASSERT (ROT_p);
      result = ROT_p->Revoke (ROTID_);
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IRunningObjectTable::Revoke(%d) \"%s\", continuing\n"),
                    ROTID_,
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      ROT_p->Release ();
      ROTID_ = 0;
    } // end IF

  continue_:
    if (IMediaEventEx_)
    {
      IMediaEventEx_->SetNotifyWindow (NULL, 0, 0);
      IMediaEventEx_->Release ();
      IMediaEventEx_ = NULL;
    } // end IF
    if (IMediaControl_)
    {
      IMediaControl_->Stop ();
      IMediaControl_->Release ();
      IMediaControl_ = NULL;
    } // end IF

    if (IGraphBuilder_)
    {
      IGraphBuilder_->Release ();
      IGraphBuilder_ = NULL;
    } // end IF

    configuration_ = NULL;
    //mediaType_ = NULL;
    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isInitialized_ = false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  //mediaType_ = &configuration_->mediaType;
  // *TODO*: remove type inference
  push_ = configuration_->push;
  configuration_->filterConfiguration->module = inherited::mod_;
  configuration_->filterConfiguration->pinConfiguration->queue =
    &(inherited::queue_);

  isInitialized_ = true;

  return true;
}

// -----------------------------------------------------------------------------

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
void*
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::calloc (size_t numElements_in,
                                                                              size_t sizePerElement_in,
                                                                              char initialValue_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::calloc"));

  ACE_UNUSED_ARG (numElements_in);
  ACE_UNUSED_ARG (sizePerElement_in);
  ACE_UNUSED_ARG (initialValue_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (NULL);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::remove (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::remove"));

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::bind (const char* name_in,
                                                                            void* pointer_in,
                                                                            int duplicates_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::bind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);
  ACE_UNUSED_ARG (duplicates_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::trybind (const char* name_in,
                                                                               void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::trybind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::find (const char* name_in,
                                                                            void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::find"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::find (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::find"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::unbind (const char* name_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::unbind"));

  ACE_UNUSED_ARG (name_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::unbind (const char* name_in,
                                                                              void*& pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::unbind"));

  ACE_UNUSED_ARG (name_in);
  ACE_UNUSED_ARG (pointer_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::sync (ssize_t length_in,
                                                                            int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::sync"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::sync (void* address_in,
                                                                            size_t length_in,
                                                                            int flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::sync"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::protect (ssize_t length_in,
                                                                               int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::protect"));

  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}

template <typename ConfigurationType,
          typename MessageType,
          typename SessionMessageType>
int
Stream_MediaFramework_DirectShow_AllocatorBase_T<ConfigurationType,
                                                 MessageType,
                                                 SessionMessageType>::protect (void* address_in,
                                                                               size_t length_in,
                                                                               int protection_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_AllocatorBase_T::protect"));

  ACE_UNUSED_ARG (address_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (protection_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (-1);
}
