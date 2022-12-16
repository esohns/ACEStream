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

#include "AudioClient.h"
#include "mfapi.h"
#include "mferror.h"
#include "shlwapi.h"
#include "wxdebug.h"

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"

#include "stream_macros.h"

#include "stream_lib_mediafoundation_tools.h"

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::CreateInstance (IUnknown* parent_in,
                                                                                      REFIID interfaceID_in,
                                                                                      void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::CreateInstance"));

  // sanity check(s)
  CheckPointer (parent_in, E_POINTER);
  CheckPointer (interface_out, E_POINTER);

  MediaSourceType* media_source_p = dynamic_cast<MediaSourceType*> (parent_in);
  ACE_ASSERT (media_source_p);

  OWN_TYPE_T* instance_p = NULL;
  ACE_NEW_NORETURN (instance_p,
                    OWN_TYPE_T (media_source_p));
  if (!instance_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  HRESULT result = instance_p->QueryInterface (interfaceID_in, interface_out);
  if (FAILED (result) || !interface_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::QueryInterface(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    delete instance_p; instance_p = NULL;
    return result;
  } // end IF

  return result;
}

template <typename MediaSourceType>
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::Stream_MediaFramework_MediaFoundation_MediaStream_T (MediaSourceType* mediaSource_in)
 : selected_ (false)
 , eventQueue_ (NULL)
 , mediaSource_ (mediaSource_in)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::Stream_MediaFramework_MediaFoundation_MediaStream_T"));

  HRESULT result = MFCreateEventQueue (&eventQueue_);
  if (FAILED (result) || !eventQueue_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateEventQueue(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  ACE_ASSERT (mediaSource_);
}

template <typename MediaSourceType>
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::Stream_MediaFramework_MediaFoundation_MediaStream_T (HRESULT* result_out)
 : selected_ (false)
 , eventQueue_ (NULL)
 , mediaSource_ (NULL)
 , referenceCount_ (1)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::Stream_MediaFramework_MediaFoundation_MediaStream_T"));

  // sanity check(s)
  ACE_ASSERT (result_out);

  HRESULT result = MFCreateEventQueue (&eventQueue_);
  if (FAILED (result) || !eventQueue_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateEventQueue(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    *result_out = result;
  } // end IF
  else
    *result_out = S_OK;
}

template <typename MediaSourceType>
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::~Stream_MediaFramework_MediaFoundation_MediaStream_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::~Stream_MediaFramework_MediaFoundation_MediaStream_T"));

  if (eventQueue_)
  {
    HRESULT result = eventQueue_->Shutdown ();
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaEventQueue::Shutdown(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    eventQueue_->Release ();
  } // end IF
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::QueryInterface (REFIID IID_in,
                                                                                      void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IUnknown),
    QITABENT (OWN_TYPE_T, IMFMediaEventGenerator),
    QITABENT (OWN_TYPE_T, IMFMediaStream),
    { 0 },
  };

  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}

template <typename MediaSourceType>
ULONG
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0)
  //delete this;

  return count;
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::BeginGetEvent (IMFAsyncCallback* asynchCallback_in,
                                                                                     IUnknown* state_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::BeginGetEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->BeginGetEvent (asynchCallback_in,
                                     state_in);
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::EndGetEvent (IMFAsyncResult* asynchResult_in,
                                                                                   IMFMediaEvent** event_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::EndGetEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->EndGetEvent (asynchResult_in,
                                   event_out);
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::GetEvent (DWORD flags_in,
                                                                                IMFMediaEvent** event_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::GetEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->GetEvent (flags_in,
                                event_out);
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::QueueEvent (MediaEventType type_in,
                                                                                  REFGUID extendedType_in,
                                                                                  HRESULT status_in,
                                                                                  const PROPVARIANT* value_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::QueueEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->QueueEventParamVar (type_in,
                                          extendedType_in,
                                          status_in,
                                          value_in);
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::GetStreamDescriptor (IMFStreamDescriptor** ppStreamDescriptor_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::GetStreamDescriptor"));

  // sanity check(s)
  ACE_ASSERT (ppStreamDescriptor_out && !*ppStreamDescriptor_out);
  ACE_ASSERT (mediaSource_);

  HRESULT result = E_FAIL;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  BOOL selected_b = FALSE;

  if (mediaSource_->presentationDescriptor_)
  {
    mediaSource_->presentationDescriptor_->AddRef ();
    presentation_descriptor_p = mediaSource_->presentationDescriptor_;
  } // end IF
  else
  {
    result = mediaSource_->CreatePresentationDescriptor (&presentation_descriptor_p);
    ACE_ASSERT (SUCCEEDED (result) && presentation_descriptor_p);
  } // end ELSE
  ACE_ASSERT (presentation_descriptor_p);
  
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &selected_b,
                                                           ppStreamDescriptor_out);
  ACE_ASSERT (SUCCEEDED (result) && selected_b && *ppStreamDescriptor_out);

  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;

  return S_OK;
}

template <typename MediaSourceType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::RequestSample (IUnknown* pToken)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::RequestSample"));

  // sanity check(s)
  ACE_ASSERT (mediaSource_);
  if (mediaSource_->state_ == MediaSourceType::STATE_SHUTDOWN)
    return MF_E_SHUTDOWN;

  if (pToken)
    pToken->AddRef ();
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, mediaSource_->lock_, E_FAIL);
    mediaSource_->tokens_.push_back (pToken);
  } // end lock scope

  return S_OK;
}

template <typename MediaSourceType>
void
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::start ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::start"));

  // send MEStreamStarted
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  property_s.vt = VT_I8;
  property_s.hVal.QuadPart = 0;
  HRESULT result = QueueEvent (MEStreamStarted,
                               GUID_NULL,
                               S_OK,
                               &property_s);
  ACE_ASSERT (SUCCEEDED (result));
  PropVariantClear (&property_s);
}

template <typename MediaSourceType>
void
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::stop ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::stop"));

  // send MEStreamStarted
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  HRESULT result = QueueEvent (MEStreamStopped,
                               GUID_NULL,
                               S_OK,
                               &property_s);
  ACE_ASSERT (SUCCEEDED (result));
  PropVariantClear (&property_s);
}

template <typename MediaSourceType>
void
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::end"));

  // send MEEndOfStream
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  HRESULT result = QueueEvent (MEEndOfStream,
                               GUID_NULL,
                               S_OK,
                               &property_s);
  ACE_ASSERT (SUCCEEDED (result));
  PropVariantClear (&property_s);
}

template <typename MediaSourceType>
void
Stream_MediaFramework_MediaFoundation_MediaStream_T<MediaSourceType>::operator delete (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaStream_T::operator delete"));

  //// *NOTE*: when used as a regular C++ library (template) class, applications
  ////         instantiate filter objects through the default ctor. In this case,
  ////         class instances behave as standard COM objects, and must therefore
  ////         implement the IUnknown reference-counting mechanism to avoid memory
  ////         leakage.
  //if (hasCOMReference_)
  //{
  //  ULONG reference_count = Release ();
  //  return; // dtor has been invoked --> done
  //} // end IF

  // *NOTE*: when applications instantiate filter (COM) objects from DLLs, that
  //         filter instance may be allocated in a separate heap (this depends
  //         on the C runtime version (and, apparently, also type, i.e. static/
  //         dynamic) that was compiled into(/with ? ...) the DLL) and needs to
  //         be deallocated 'from' the same heap; i.e. the global 'delete'
  //         operator may (see above) fail in this particular scenario (
  //         _CrtIsValidHeapPointer() assertion), which is a known and long-
  //         standing issue. *TODO*: does this affect _DEBUG builds only ?
  //         --> overload the delete operator and forward the instance handle to
  //             a static function 'inside' (see 'translation/compilation units'
  //             and/or scope/namespace issues on how to address the 'global
  //             delete' operator) the DLL
  //         This implementation also handles the scenario where filter
  //         instances are allocated from 'plugin' DLLs that can be loaded/
  //         unloaded at runtime
  //OWN_TYPE_T::DeleteInstance (pointer_in);
}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename MediaType,
//          typename ModuleType>
//void
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       MediaType,
//                                       ModuleType>::operator delete (void* pointer_in,
//                                                                     size_t bytes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::operator delete"));
//
//  ACE_UNUSED_ARG (bytes_in);
//
//  // *NOTE*: see above
//  OWN_TYPE_T::DeleteInstance (pointer_in);
//}

//////////////////////////////////////////

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::CreateInstance (IUnknown* parent_in,
                                                                                        REFIID interfaceID_in,
                                                                                        void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::CreateInstance"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);
  // This object does not support aggregation.
  if (parent_in)
    return CLASS_E_NOAGGREGATION;

  HRESULT result = E_FAIL;
  OWN_TYPE_T* instance_p = NULL;
  ACE_NEW_NORETURN (instance_p,
                    OWN_TYPE_T (&result));
  if (!instance_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to instantiate Stream_MediaFramework_MediaFoundation_MediaSource_T: \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    delete instance_p; instance_p = NULL;
    return result;
  } // end IF
  result = instance_p->QueryInterface (interfaceID_in, interface_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::QueryInterface(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    delete instance_p; instance_p = NULL;
    return result;
  } // end IF
  ACE_ASSERT (interface_out);

  return result;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Stream_MediaFramework_MediaFoundation_MediaSource_T ()
 : configuration_ (NULL)
 , eventQueue_ (NULL)
//, hasCOMReference_ (false)
 , presentationDescriptor_ (NULL)
 , lock_ ()
 , referenceCount_ (0)
 , state_ (STATE_INVALID)
 , tokens_ ()
 , mediaStream_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Stream_MediaFramework_MediaFoundation_MediaSource_T"));

  HRESULT result = MFCreateEventQueue (&eventQueue_);
  if (FAILED (result) || !eventQueue_)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateEventQueue(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  ACE_NEW_NORETURN (mediaStream_,
                    STREAM_T (this));
  if (!mediaStream_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return;
  } // end IF
  mediaStream_->selected_ = true;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Stream_MediaFramework_MediaFoundation_MediaSource_T (HRESULT* result_out)
 : buffering_ (true)
 , configuration_ (NULL)
 , eventQueue_ (NULL)
//, hasCOMReference_ (false)
 , lock_ ()
 , mediaStream_ (NULL)
 , presentationDescriptor_ (NULL)
 , referenceCount_ (0)
 , state_ (STATE_INVALID)
 , tokens_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Stream_MediaFramework_MediaFoundation_MediaSource_T"));

  // sanity check(s)
  ACE_ASSERT (result_out);

  HRESULT result = MFCreateEventQueue (&eventQueue_);
  if (FAILED (result) || !eventQueue_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateEventQueue(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    *result_out = result;
    return;
  } // end IF

  ACE_NEW_NORETURN (mediaStream_,
                    STREAM_T (this));
  if (!mediaStream_)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    *result_out = E_OUTOFMEMORY;
    return;
  } // end IF
  mediaStream_->selected_ = true;

  *result_out = S_OK;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::~Stream_MediaFramework_MediaFoundation_MediaSource_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::~Stream_MediaFramework_MediaFoundation_MediaSource_T"));

  HRESULT result = E_FAIL;

  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, lock_);
    if ((state_ != STATE_INVALID) && 
        (state_ != STATE_SHUTDOWN))
    {
      result = Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_MediaSource_T::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    } // end IF

    if (eventQueue_)
    {
      result = eventQueue_->Shutdown ();
      if (FAILED (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaEventQueue::Shutdown(): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      eventQueue_->Release ();
    } // end IF

    if (presentationDescriptor_)
      presentationDescriptor_->Release ();

    for (TOKEN_LIST_ITERATOR_T iterator = tokens_.begin ();
         iterator != tokens_.end ();
         ++iterator)
      if (*iterator)
        (*iterator)->Release ();
    tokens_.clear ();
  } // end lock scope

  if (mediaStream_)
    delete mediaStream_;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::BeginCreateObject (LPCWSTR pwszURL,
                                                                                           DWORD dwFlags,
                                                                                           IPropertyStore* pProps,
                                                                                           IUnknown** ppIUnknownCancelCookie,
                                                                                           IMFAsyncCallback* pCallback,
                                                                                           IUnknown* punkState)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::BeginCreateObject"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::CancelObjectCreation (IUnknown* pIUnknownCancelCookie)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::CancelObjectCreation"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::EndCreateObject (IMFAsyncResult* pResult,
                                                                                         MF_OBJECT_TYPE* pObjectType,
                                                                                         IUnknown** ppObject)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::EndCreateObject"));

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::QueryInterface (REFIID IID_in,
                                                                                        void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::QueryInterface"));

  static const QITAB query_interface_table[] =
  {
    QITABENT (OWN_TYPE_T, IMFSchemeHandler),
    QITABENT (OWN_TYPE_T, IMFMediaEventGenerator),
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
    QITABENT (OWN_TYPE_T, IMFMediaSourceEx),
#else
    QITABENT (OWN_TYPE_T, IMFMediaSource),
#endif // _WIN32_WINNT_WIN8
    QITABENT (OWN_TYPE_T, IMFGetService),
    QITABENT (OWN_TYPE_T, IMFPMPClient),
    { 0 },
  };

  // *TODO*: {0B5E1C7E-BD76-46BC-896C-B2EDB40DD803}
  return QISearch (this,
                   query_interface_table,
                   IID_in,
                   interface_out);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
ULONG
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Release ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Release"));

  ULONG count = InterlockedDecrement (&referenceCount_);
  //if (count == 0)
  //delete this;

  return count;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::BeginGetEvent (IMFAsyncCallback* asynchCallback_in,
                                                                                       IUnknown* state_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::BeginGetEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->BeginGetEvent (asynchCallback_in,
                                     state_in);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::EndGetEvent (IMFAsyncResult* asynchResult_in,
                                                                                     IMFMediaEvent** event_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::EndGetEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->EndGetEvent (asynchResult_in,
                                   event_out);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::GetEvent (DWORD flags_in,
                                                                                  IMFMediaEvent** event_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->GetEvent (flags_in,
                                event_out);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::QueueEvent (MediaEventType type_in,
                                                                                    REFGUID extendedType_in,
                                                                                    HRESULT status_in,
                                                                                    const PROPVARIANT* value_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::QueueEvent"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);

  return eventQueue_->QueueEventParamVar (type_in,
                                          extendedType_in,
                                          status_in,
                                          value_in);
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::GetCharacteristics (DWORD* characteristics_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetCharacteristics"));

  // sanity check(s)
  ACE_ASSERT (characteristics_out);
  if (state_ == STATE_SHUTDOWN)
    return MF_E_SHUTDOWN;

  *characteristics_out =
    (MFMEDIASOURCE_IS_LIVE             |
     MFMEDIASOURCE_CAN_PAUSE           |
     MFMEDIASOURCE_DOES_NOT_USE_NETWORK); // *TODO*: this may not always be true !

  return S_OK;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::CreatePresentationDescriptor (IMFPresentationDescriptor** presentationDescriptor_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::CreatePresentationDescriptor"));

  // sanity check(s)
  ACE_ASSERT (presentationDescriptor_out && !*presentationDescriptor_out);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->mediaType);

  HRESULT result = E_FAIL;
  IMFStreamDescriptor** stream_descriptors_a = NULL;
  IMFMediaType** media_types_a = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;

  if (presentationDescriptor_)
    goto clone;

  ACE_NEW_NORETURN (stream_descriptors_a,
                    IMFStreamDescriptor*[1]);
  if (!stream_descriptors_a)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  ACE_OS::memset (stream_descriptors_a, 0, sizeof (IMFStreamDescriptor*[1]));

  ACE_NEW_NORETURN (media_types_a,
                    IMFMediaType*[1]);
  if (!media_types_a)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    delete [] stream_descriptors_a;
    return E_OUTOFMEMORY;
  } // end IF
  ACE_OS::memset (media_types_a, 0, sizeof (IMFMediaType*[1]));
  media_types_a[0] =
    Stream_MediaFramework_MediaFoundation_Tools::copy (configuration_->mediaType);
  ACE_ASSERT (media_types_a[0]);
  result = MFCreateStreamDescriptor (0,
                                     1,
                                     media_types_a,
                                     &(stream_descriptors_a[0]));
  ACE_ASSERT (SUCCEEDED (result) && stream_descriptors_a[0]);
  result =
    stream_descriptors_a[0]->SetString (MF_SD_STREAM_NAME,
                                        STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_DEFAULT_STREAM_NAME_L);
  ACE_ASSERT (SUCCEEDED (result));
  result = stream_descriptors_a[0]->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
  result =
    media_type_handler_p->SetCurrentMediaType (configuration_->mediaType);
  ACE_ASSERT (SUCCEEDED (result));
  result = MFCreatePresentationDescriptor (1,
                                           stream_descriptors_a,
                                           &presentationDescriptor_);
  ACE_ASSERT (SUCCEEDED (result) && presentationDescriptor_);
  UINT32 bytes_per_second_i = 0;
  result =
    configuration_->mediaType->GetUINT32 (MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                          &bytes_per_second_i);
  ACE_ASSERT (SUCCEEDED (result));
  result = presentationDescriptor_->SetUINT32 (MF_PD_AUDIO_ENCODING_BITRATE,
                                               bytes_per_second_i * 8);
  ACE_ASSERT (SUCCEEDED (result));
  //MF_PD_VIDEO_ENCODING_BITRATE
  result = presentationDescriptor_->SetUINT32 (MF_PD_AUDIO_ISVARIABLEBITRATE,
                                               0);
  ACE_ASSERT (SUCCEEDED (result));
  result = presentationDescriptor_->SetUINT64 (MF_PD_DURATION,
                                               static_cast<UINT64> (std::numeric_limits<INT64>::max ()));
  ACE_ASSERT (SUCCEEDED (result));

  result = presentationDescriptor_->SelectStream (0);
  ACE_ASSERT (SUCCEEDED (result));

  // clean up
  media_type_handler_p->Release (); media_type_handler_p = NULL;
  media_types_a[0]->Release ();
  delete [] media_types_a; media_types_a = NULL;
  stream_descriptors_a[0]->Release ();
  delete [] stream_descriptors_a; stream_descriptors_a = NULL;

clone:
  result = presentationDescriptor_->Clone (presentationDescriptor_out);
  ACE_ASSERT (SUCCEEDED (result) && *presentationDescriptor_out);

  return S_OK;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Start (IMFPresentationDescriptor* presentationDescriptor_in,
                                                                               const GUID* timeFormat_in,
                                                                               const PROPVARIANT* startPosition_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Start"));

  ACE_UNUSED_ARG (presentationDescriptor_in);
  ACE_UNUSED_ARG (startPosition_in);
  
  // sanity check(s)
  if ((timeFormat_in != NULL) && (*timeFormat_in != GUID_NULL))
    return MF_E_UNSUPPORTED_TIME_FORMAT;
  if (state_ == STATE_SHUTDOWN)
    return MF_E_SHUTDOWN;
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->mediaType);
  ACE_ASSERT (eventQueue_);
  ACE_ASSERT (mediaStream_);

  // step1: validate presentation descriptor
  DWORD num_stream_descriptors = 0;
  BOOL selected_b = FALSE;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  IMFMediaType* media_type_p = NULL;
  DWORD flags_i = 0;
  HRESULT result =
    presentationDescriptor_in->GetStreamDescriptorCount (&num_stream_descriptors);
  ACE_ASSERT (SUCCEEDED (result));
  if (num_stream_descriptors != 1)
    return MF_E_UNSUPPORTED_REPRESENTATION;
  result =
    presentationDescriptor_in->GetStreamDescriptorByIndex (0,
                                                           &selected_b,
                                                           &stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result) && stream_descriptor_p);
  if (!selected_b)
  {
    stream_descriptor_p->Release (); stream_descriptor_p = NULL;
    return MF_E_UNSUPPORTED_REPRESENTATION;
  } // end IF
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;
  result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
  ACE_ASSERT (SUCCEEDED (result) && media_type_p);
  media_type_handler_p->Release (); media_type_handler_p = NULL;
  result = configuration_->mediaType->IsEqual (media_type_p, &flags_i);
  if (FAILED (result) || (result == S_FALSE))
  {
    media_type_p->Release (); media_type_p = NULL;
    return MF_E_INVALIDMEDIATYPE;
  } // end IF
  media_type_p->Release (); media_type_p = NULL;

  // step2: send events
  // send MENewStream
  IUnknown* media_stream_p = NULL;
  result =
    mediaStream_->QueryInterface (IID_PPV_ARGS (&media_stream_p));
  ACE_ASSERT (SUCCEEDED (result) && media_stream_p);
  result = eventQueue_->QueueEventParamUnk (MENewStream,
                                            GUID_NULL,
                                            S_OK,
                                            media_stream_p);
  ACE_ASSERT (SUCCEEDED (result));

  // send MEUpdatedStream
  result = eventQueue_->QueueEventParamUnk (MEUpdatedStream,
                                            GUID_NULL,
                                            S_OK,
                                            media_stream_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_stream_p->Release (); media_stream_p = NULL;

  // send MESourceStarted
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  //property_s.vt = VT_EMPTY;
  property_s.vt = VT_I8;
  //property_s.hVal.QuadPart = MFGetSystemTime ();
  property_s.hVal.QuadPart = 0;
  IMFMediaEvent* media_event_p = NULL;
  result = MFCreateMediaEvent (MESourceStarted,
                               GUID_NULL,
                               S_OK,
                               &property_s,
                               &media_event_p);
  ACE_ASSERT (SUCCEEDED (result) && media_event_p);
  result = media_event_p->SetUINT64 (MF_EVENT_SOURCE_ACTUAL_START,
                                     0);
  ACE_ASSERT (SUCCEEDED (result));
  result = eventQueue_->QueueEvent (media_event_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_event_p->Release (); media_event_p = NULL;
  PropVariantClear (&property_s);

  // send MEStreamStarted
  mediaStream_->start ();

  //// send MEBufferingStarted
  //buffering_ = true;
  //PropVariantClear (&property_s);
  //property_s.vt = VT_EMPTY;
  //result = eventQueue_->QueueEventParamVar (MEBufferingStarted,
  //                                          GUID_NULL,
  //                                          S_OK,
  //                                          &property_s);
  //ACE_ASSERT (SUCCEEDED (result));
  //PropVariantClear (&property_s);

  state_ = STATE_STARTED;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("media source started\n")));

  return S_OK;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Stop (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Stop"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);
  ACE_ASSERT (mediaStream_);

  // send MEStreamStopped
  mediaStream_->stop ();

  // send MESourceStopped
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  property_s.vt = VT_EMPTY;
  HRESULT result = eventQueue_->QueueEventParamVar (MESourceStopped,
                                                    GUID_NULL,
                                                    S_OK,
                                                    &property_s);
  ACE_ASSERT (SUCCEEDED (result));
  PropVariantClear (&property_s);

  state_ = STATE_STOPPED;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("media source stopped\n")));

  return S_OK;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Pause (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Pause"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP_RETURN (E_FAIL);

  state_ = STATE_PAUSED;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("media source paused\n")));

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::Shutdown (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Shutdown"));

  state_ = STATE_SHUTDOWN;

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("media source shutdown\n")));

  return S_OK;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::GetSourceAttributes (IMFAttributes** attributes_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetSourceAttributes"));

  ACE_UNUSED_ARG (attributes_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::GetStreamAttributes (DWORD streamIdentifier_in,
                                                                                             IMFAttributes** attributes_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetStreamAttributes"));

  ACE_UNUSED_ARG (streamIdentifier_in);
  ACE_UNUSED_ARG (attributes_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::SetD3DManager (IUnknown* Direct3DManager_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::SetD3DManager"));

  ACE_UNUSED_ARG (Direct3DManager_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);

  ACE_NOTREACHED (return E_FAIL;)
}

//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::Clone (IMFPresentationDescriptor** ppPresentationDescriptor)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::Clone"));
//
//  ACE_UNUSED_ARG (ppPresentationDescriptor);
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::DeselectStream (DWORD dwIndex)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::DeselectStream"));
//
//  ACE_UNUSED_ARG (dwIndex);
//
//  // sanity check(s)
//  ACE_ASSERT (mediaStream_);
//
//  mediaStream_->selected_ = false;
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("stream deselected (index was: %d)\n"),
//              dwIndex));
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetStreamDescriptorByIndex (DWORD dwIndex,
//                                                                                                    BOOL* pfSelected,
//                                                                                                    IMFStreamDescriptor** ppStreamDescriptor)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetStreamDescriptorByIndex"));
//
//  ACE_UNUSED_ARG (dwIndex);
//
//  // sanity check(s)
//  ACE_ASSERT (pfSelected);
//  ACE_ASSERT (ppStreamDescriptor && !*ppStreamDescriptor);
//
//  *pfSelected = (mediaStream_ ? (mediaStream_->selected_ ? TRUE : FALSE)
//                              : FALSE);
//
//  return QueryInterface (IID_PPV_ARGS (ppStreamDescriptor));
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetStreamDescriptorCount (DWORD* pdwDescriptorCount)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::DeselectStream"));
//
//  // sanity check(s)
//  ACE_ASSERT (pdwDescriptorCount);
//
//  *pdwDescriptorCount = 1;
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::SelectStream (DWORD dwIndex)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::SelectStream"));
//
//  ACE_UNUSED_ARG (dwIndex);
//
//  // sanity check(s)
//  ACE_ASSERT (mediaStream_);
//
//  mediaStream_->selected_ = true;
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("stream selected (index was: %d)\n"),
//              dwIndex));
//
//  return S_OK;
//}

//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetMediaTypeHandler (IMFMediaTypeHandler** ppMediaTypeHandler)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetMediaTypeHandler"));
//
//  // sanity check(s)
//  ACE_ASSERT (ppMediaTypeHandler && !*ppMediaTypeHandler);
//
//  return QueryInterface (IID_PPV_ARGS (ppMediaTypeHandler));
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetStreamIdentifier (DWORD* pdwStreamIdentifier)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetStreamIdentifier"));
//
//  // sanity check(s)
//  ACE_ASSERT (pdwStreamIdentifier);
//
//  *pdwStreamIdentifier = 0;
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetUINT32 (REFGUID guidKey_in,
//                                                                                   UINT32* punValue)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetUINT32"));
//
//  // sanity check(s)
//  ACE_ASSERT (punValue);
//
//  // {00AF2181-BDC2-423C-ABCA-F503593BC121}
//  if (InlineIsEqualGUID (guidKey_in, MF_SD_PROTECTED))
//  {
//    *punValue = FALSE;
//    return S_OK;
//  } // end IF
//
//  // *TODO*: {FAFB680A-39EA-4850-AF71-78FDFB651088}
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("invalid/unknown key GUID (was: \"%s\"), aborting\n"),
//              ACE_TEXT (Common_Tools::GUIDToString (guidKey_in).c_str ())));
//
//  return MF_E_ATTRIBUTENOTFOUND;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetUINT64 (REFGUID guidKey_in,
//                                                                                   UINT64* punValue)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetUINT64"));
//
//  // sanity check(s)
//  ACE_ASSERT (punValue);
//
//  // {6C990D3B-BB8E-477A-8598-0D5D96FCD88A}
//  if (InlineIsEqualGUID (guidKey_in, MF_PD_PLAYBACK_BOUNDARY_TIME))
//  {
//    *punValue = 0;
//    return S_OK;
//  } // end IF
//
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("invalid/unknown key GUID (was: \"%s\"), aborting\n"),
//              ACE_TEXT (Common_Tools::GUIDToString (guidKey_in).c_str ())));
//
//  return MF_E_ATTRIBUTENOTFOUND;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetBlobSize (REFGUID guidKey_in,
//                                                                                     UINT32* pcbBlobSize)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetBlobSize"));
//
//  // sanity check(s)
//  ACE_ASSERT (pcbBlobSize);
//
//#if (NTDDI_VERSION >= NTDDI_WIN10_RS4)
//  // {F715CF3E-A964-4C3F-94AE-9D6BA7264641}
//  if (InlineIsEqualGUID (guidKey_in, MF_SD_AMBISONICS_SAMPLE3D_DESCRIPTION))
//  {
//    *pcbBlobSize = sizeof (struct AMBISONICS_PARAMS);
//    return S_OK;
//  } // end IF
//#endif // NTDDI_VERSION >= NTDDI_WIN10_RS4
//
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("invalid/unknown key GUID (was: \"%s\"), aborting\n"),
//              ACE_TEXT (Common_Tools::GUIDToString (guidKey_in).c_str ())));
//
//  return MF_E_ATTRIBUTENOTFOUND;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::DeleteItem (REFGUID guidKey_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::DeleteItem"));
//
//  //if (InlineIsEqualGUID (guidKey_in, GUID_NULL))
//  //{
//  //  return S_OK;
//  //} // end IF
//
//  // *TODO*: {0B5E1C7E-BD76-46BC-896C-B2EDB40DD803}
//  ACE_DEBUG ((LM_WARNING,
//              ACE_TEXT ("invalid/unknown key GUID (was: \"%s\"), continuing\n"),
//              ACE_TEXT (Common_Tools::GUIDToString (guidKey_in).c_str ())));
//
//  IUNKNOWN_MAP_ITERATOR_T iterator = unknowns_.find (guidKey_in);
//  ACE_ASSERT (iterator != unknowns_.end ());
//  (*iterator).second->Release ();
//  unknowns_.erase (iterator);
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::SetUnknown (REFGUID guidKey_in,
//                                                                                    IUnknown* unknown_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::SetUnknown"));
//
//  // sanity check(s)
//  ACE_ASSERT (unknown_in);
//
//  //if (InlineIsEqualGUID (guidKey_in, GUID_NULL))
//  //{
//  //  return S_OK;
//  //} // end IF
//
//  // *TODO*: {0B5E1C7E-BD76-46BC-896C-B2EDB40DD803}
//  ACE_DEBUG ((LM_WARNING,
//              ACE_TEXT ("invalid/unknown key GUID (was: \"%s\"), continuing\n"),
//              ACE_TEXT (Common_Tools::GUIDToString (guidKey_in).c_str ())));
//
//  unknowns_.insert (std::make_pair (guidKey_in, unknown_in));
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetCurrentMediaType (IMFMediaType** ppMediaType)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetCurrentMediaType"));
//
//  // sanity check(s)
//  ACE_ASSERT (ppMediaType && !*ppMediaType);
//  if (!configuration_ || !configuration_->mediaType)
//    return MF_E_NOT_INITIALIZED;
//
//  HRESULT result = MFCreateMediaType (ppMediaType);
//  if (FAILED (result) || !*ppMediaType)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return result;
//  } // end IF
//  result = configuration_->mediaType->CopyAllItems (*ppMediaType);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaType::CopyAllItems(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    (*ppMediaType)->Release (); *ppMediaType = NULL;
//    return result;
//  } // end IF
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetMajorType (GUID* pguidMajorType)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetMajorType"));
//
//  // sanity check(s)
//  ACE_ASSERT (pguidMajorType);
//
//  *pguidMajorType = MFMediaType_Video;
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetMediaTypeByIndex (DWORD dwIndex,
//                                                                                             IMFMediaType** ppType)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetMediaTypeByIndex"));
//
//  // sanity check(s)
//  ACE_ASSERT (ppType && !*ppType);
//  if (dwIndex > 0)
//    return MF_E_NO_MORE_TYPES;
//  ACE_ASSERT (configuration_);
//  ACE_ASSERT (configuration_->mediaType);
//
//  HRESULT result = MFCreateMediaType (ppType);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return result;
//  } // end IF
//  ACE_ASSERT (*ppType);
//  result = configuration_->mediaType->CopyAllItems (*ppType);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaType::CopyAllItems(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    (*ppType)->Release (); *ppType = NULL;
//    return result;
//  } // end IF
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::GetMediaTypeCount (DWORD* pdwTypeCount)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetMediaTypeCount"));
//
//  // sanity check(s)
//  ACE_ASSERT (pdwTypeCount);
//
//  *pdwTypeCount = 1;
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::IsMediaTypeSupported (IMFMediaType* pMediaType,
//                                                                                              IMFMediaType** ppMediaType)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::IsMediaTypeSupported"));
//
//  // sanity check(s)
//  ACE_ASSERT (pMediaType);
//
//  ACE_ASSERT (false);
//
//  return S_OK;
//}
//
//template <typename TimePolicyType,
//          typename MessageType,
//          typename ConfigurationType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    MessageType,
//                                                    ConfigurationType>::SetCurrentMediaType (IMFMediaType* pMediaType)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::IsMediaTypeSupported"));
//
//  // sanity check(s)
//  ACE_ASSERT (pMediaType);
//  ACE_ASSERT (configuration_);
//  ACE_ASSERT (configuration_->mediaType);
//
//  DWORD flags = 0;
//  return configuration_->mediaType->IsEqual (pMediaType, &flags);
//}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
HRESULT
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::GetService (REFGUID guidService,
                                                                                    REFIID riid,
                                                                                    LPVOID* ppvObject)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetService"));

  // sanity check(s)
  ACE_ASSERT (ppvObject);

  //if (InlineIsEqualGUID (guidService, GUID_NULL))
  //{
  //  return S_OK;
  //} // end IF

  // MF_RATE_CONTROL_SERVICE: {866FA297-B802-4BF8-9DC9-5E3B6A9F53C9}
  // IID_IMFRateControl: {88DDCD21-03C3-4275-91ED-55EE3929328F}
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("invalid/unknown service/interface GUID (was: \"%s\",\"%s\"), aborting\n"),
              ACE_TEXT (Common_Tools::GUIDToString (guidService).c_str ()),
              ACE_TEXT (Common_Tools::GUIDToString (riid).c_str ())));

  return MF_E_UNSUPPORTED_SERVICE;
}

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::GetItem (REFGUID guidKey,
//                                                                         PROPVARIANT* pValue)
//{
//  ACE_ASSERT (false);
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::GetItemType (REFGUID guidKey,
//                                                                             MF_ATTRIBUTE_TYPE* pType)
//{
//  ACE_ASSERT (false);
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::CompareItem (REFGUID guidKey,
//                                                                             REFPROPVARIANT Value,
//                                                                             BOOL* pbResult)
//{
//  ACE_ASSERT (false);
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::Compare (IMFAttributes* pTheirs,
//                                                                         MF_ATTRIBUTES_MATCH_TYPE MatchType,
//                                                                         BOOL* pbResult)
//{
//  ACE_ASSERT (false);
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::GetUINT32 (REFGUID guidKey,
//                                                                           UINT32* punValue)
//{
//  ACE_ASSERT (false);
//}
  //virtual STDMETHODIMP GetUINT64 (REFGUID, // guidKey
  //                                UINT64); // punValue
  //virtual STDMETHODIMP GetDouble (REFGUID,  // guidKey
  //                                double*); // pfValue
  //virtual STDMETHODIMP GetGUID (REFGUID, // guidKey
  //                              GUID*);  // pguidValue
  //virtual STDMETHODIMP GetStringLength (REFGUID,  // guidKey
  //                                      UINT32*); // pcchLength
  //virtual STDMETHODIMP GetString (REFGUID,  // guidKey
  //                                LPWSTR,   // pwszValue
  //                                UINT32,   // cchBufSize
  //                                UINT32*); // pcchLength
  //virtual STDMETHODIMP GetAllocatedString (REFGUID,  // guidKey
  //                                         LPWSTR*,  // ppwszValue
  //                                         UINT32*); // pcchLength
  //virtual STDMETHODIMP GetBlobSize (REFGUID,  // guidKey
  //                                  UINT32*); // pcbBlobSize
  //virtual STDMETHODIMP GetBlob (REFGUID,  // guidKey
  //                              UINT8*,   // pBuf
  //                              UINT32,   // cbBufSize
  //                              UINT32*); // pcbBlobSize
  //virtual STDMETHODIMP GetAllocatedBlob (REFGUID,  // guidKey
  //                                       UINT8**,  // ppBuf
  //                                       UINT32*); // pcbSize
  //virtual STDMETHODIMP GetUnknown (REFGUID, // guidKey
  //                                 REFIID,  // riid
  //                                 LPVOID); // ppv
  //virtual STDMETHODIMP SetItem (REFGUID,         // guidKey
  //                              REFPROPVARIANT); // Value
  //virtual STDMETHODIMP DeleteItem (REFGUID); // guidKey
  //virtual STDMETHODIMP DeleteAllItems (void);
  //virtual STDMETHODIMP SetUINT32 (REFGUID, // guidKey
  //                                UINT32); // unValue
  //virtual STDMETHODIMP SetUINT64 (REFGUID, // guidKey
  //                                UINT64); // unValue
  //virtual STDMETHODIMP SetDouble (REFGUID, // guidKey
  //                                double); // fValue
  //virtual STDMETHODIMP SetGUID (REFGUID,  // guidKey
  //                              REFGUID); // guidValue
  //virtual STDMETHODIMP SetString (REFGUID,  // guidKey
  //                                LPCWSTR); // wszValue
  //virtual STDMETHODIMP SetBlob (REFGUID,      // guidKey
  //                              const UINT8*, // pBuf
  //                              UINT32);      // cbBufSize
  //virtual STDMETHODIMP SetUnknown (REFGUID,    // guidKey
  //                                 IUnknown*); // pUnknown
  //virtual STDMETHODIMP LockStore (void);
  //virtual STDMETHODIMP UnlockStore (void);
  //virtual STDMETHODIMP GetCount (UINT32*); // pcItems
  //virtual STDMETHODIMP GetItemByIndex (UINT32,        // unIndex
  //                                     GUID*,         // pguidKey
  //                                     PROPVARIANT*); // pValue
  //virtual STDMETHODIMP CopyAllItems (IMFAttributes); // pDest


//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::GetUnmarshalClass (REFIID riid,
//                                                                                   void *pv,
//                                                                                   DWORD dwDestContext,
//                                                                                   void *pvDestContext,
//                                                                                   DWORD mshlflags,
//                                                                                   CLSID *pCid)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetUnmarshalClass"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::GetMarshalSizeMax (REFIID riid,
//                                                                                   void *pv,
//                                                                                   DWORD dwDestContext,
//                                                                                   void *pvDestContext,
//                                                                                   DWORD mshlflags,
//                                                                                   DWORD *pSize)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::GetMarshalSizeMax"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::MarshalInterface (IStream *pStm,
//                                                                                  REFIID riid,
//                                                                                  void *pv,
//                                                                                  DWORD dwDestContext,
//                                                                                  void *pvDestContext,
//                                                                                  DWORD mshlflags)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::MarshalInterface"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::UnmarshalInterface (IStream *pStm,
//                                                                                    REFIID riid,
//                                                                                    void **ppv)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::UnmarshalInterface"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::ReleaseMarshalData (IStream *pStm)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::ReleaseMarshalData"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                                    SessionMessageType,
//                                                    ProtocolMessageType,
//                                                    ConfigurationType,
//                                                    MediaType>::DisconnectObject (DWORD dwReserved)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::DisconnectObject"));
//
//  ACE_ASSERT (false);
//  ACE_NOTSUP_RETURN (E_FAIL);
//
//  ACE_NOTREACHED (return E_FAIL;)
//}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
bool
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::initialize"));

  buffering_ = true;
  configuration_ = &const_cast<ConfigurationType&> (configuration_in);
  if (presentationDescriptor_)
  {
    presentationDescriptor_->Release (); presentationDescriptor_ = NULL;
  } // end IF
  state_ = STATE_INITIALIZED;
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, lock_, false);
    for (TOKEN_LIST_ITERATOR_T iterator = tokens_.begin ();
         iterator != tokens_.end ();
         ++iterator)
      if (*iterator)
        (*iterator)->Release ();
    tokens_.clear ();
  } // end lock scope

  return true;
}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
void
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::operator delete (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::operator delete"));

  //// *NOTE*: when used as a regular C++ library (template) class, applications
  ////         instantiate filter objects through the default ctor. In this case,
  ////         class instances behave as standard COM objects, and must therefore
  ////         implement the IUnknown reference-counting mechanism to avoid memory
  ////         leakage.
  //if (hasCOMReference_)
  //{
  //  ULONG reference_count = Release ();
  //  return; // dtor has been invoked --> done
  //} // end IF

  // *NOTE*: when applications instantiate filter (COM) objects from DLLs, that
  //         filter instance may be allocated in a separate heap (this depends
  //         on the C runtime version (and, apparently, also type, i.e. static/
  //         dynamic) that was compiled into(/with ? ...) the DLL) and needs to
  //         be deallocated 'from' the same heap; i.e. the global 'delete'
  //         operator may (see above) fail in this particular scenario (
  //         _CrtIsValidHeapPointer() assertion), which is a known and long-
  //         standing issue. *TODO*: does this affect _DEBUG builds only ?
  //         --> overload the delete operator and forward the instance handle to
  //             a static function 'inside' (see 'translation/compilation units'
  //             and/or scope/namespace issues on how to address the 'global
  //             delete' operator) the DLL
  //         This implementation also handles the scenario where filter
  //         instances are allocated from 'plugin' DLLs that can be loaded/
  //         unloaded at runtime
  //OWN_TYPE_T::DeleteInstance (pointer_in);
}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename MediaType,
//          typename ModuleType>
//void
//Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       MediaType,
//                                       ModuleType>::operator delete (void* pointer_in,
//                                                                     size_t bytes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::operator delete"));
//
//  ACE_UNUSED_ARG (bytes_in);
//
//  // *NOTE*: see above
//  OWN_TYPE_T::DeleteInstance (pointer_in);
//}

template <typename TimePolicyType,
          typename MessageType,
          typename ConfigurationType>
void
Stream_MediaFramework_MediaFoundation_MediaSource_T<TimePolicyType,
                                                    MessageType,
                                                    ConfigurationType>::end ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_MediaSource_T::end"));

  // sanity check(s)
  ACE_ASSERT (eventQueue_);
  ACE_ASSERT (mediaStream_);

  // send MEEndOfStream
  mediaStream_->end ();

  // send MEEndOfPresentation
  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  property_s.vt = VT_EMPTY;
  HRESULT result = eventQueue_->QueueEventParamVar (MEEndOfPresentation,
                                                    GUID_NULL,
                                                    S_OK,
                                                    &property_s);
  ACE_ASSERT (SUCCEEDED (result));
  PropVariantClear (&property_s);
}
