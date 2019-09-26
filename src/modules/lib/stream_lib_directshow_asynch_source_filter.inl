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

#include <Guiddef.h>
#include <dvdmedia.h>
#include <vfwmsgs.h>

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"

template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
CUnknown* WINAPI
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::CreateInstance (LPUNKNOWN IUnknown_in,
                                                                                    HRESULT* result_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::CreateInstance"));

  // sanity check(s)
  ACE_ASSERT (result_out);

  // initialize return value(s)
  *result_out = S_OK;

  CUnknown* unknown_p = NULL;
  ACE_NEW_NORETURN (unknown_p,
                    OWN_TYPE_T (NAME (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE),
                                IUnknown_in,
                                CLSID_ACEStream_MediaFramework_Asynch_Source_Filter,
                                result_out));
  if (unlikely (!unknown_p))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    *result_out = E_OUTOFMEMORY;
  } // end IF

  return unknown_p;
} // CreateInstance
template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
void WINAPI
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::DeleteInstance (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::DeleteInstance"));

  // sanity check(s)
  ACE_ASSERT (pointer_in);

  // *WARNING*: make sure that this is only ever called by the base class
  //            Release(). That call delegates to the overloaded delete operator
  //            --> safe to cast

  OWN_TYPE_T* instance_p = static_cast<OWN_TYPE_T*> (pointer_in);

  ::delete instance_p;
} // DeleteInstance

// ------------------------------------

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename PinConfigurationType,
//          typename MediaType>
//const CMediaType*
//Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
//                                              SessionMessageType,
//                                              DataMessageType,
//                                              PinConfigurationType,
//                                              MediaType>::LoadType () const
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::LoadType"));
//
//  return &mediaType_;
//}

// ------------------------------------

template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
void
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::operator delete (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::operator delete"));

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
  OWN_TYPE_T::DeleteInstance (pointer_in);
}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename DataMessageType,
//          typename MediaType,
//          typename ModuleType>
//void
//Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
//                                       SessionMessageType,
//                                       DataMessageType,
//                                       MediaType,
//                                       ModuleType>::operator delete (void* pointer_in,
//                                                                     size_t bytes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::operator delete"));
//
//  ACE_UNUSED_ARG (bytes_in);
//
//  // *NOTE*: see above
//  OWN_TYPE_T::DeleteInstance (pointer_in);
//}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T ()
 : inherited (NAME (STREAM_LIB_DS_WIN32_FILTER_NAME_ASYNCH_SOURCE), // name
              NULL,                                                 // owner
              CLSID_ACEStream_MediaFramework_Asynch_Source_Filter,  // CLSID
              NULL)                                                 // result
 , filterConfiguration_ (NULL)
//, hasCOMReference_ (false)
 , lock_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T"));

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  OUTPUT_PIN_T* pin_p = NULL;
  HRESULT result = E_FAIL;
  ACE_NEW_NORETURN (pin_p,
                    OUTPUT_PIN_T (NULL,
                                  this,
                                  MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME));
  if (unlikely (!pin_p || FAILED (result)))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: added output pin \"%s\"\n"),
  //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
  //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));
} // (Constructor)
template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::~Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::~Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T"));

  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
  HRESULT result = inherited::QueryFilterInfo (&filter_info);
  if (unlikely (FAILED (result)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  // step1: disconnect from graph
  if (unlikely (!Stream_MediaFramework_DirectShow_Tools::disconnect (this)))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), continuing\n")));

  // step2: remove from graph
  if (filter_info.pGraph)
  {
    result = filter_info.pGraph->RemoveFilter (this);
    if (unlikely (FAILED (result)))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IFilterGraph::RemoveFilter(%s): \"%s\", continuing\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  } // end IF
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T (LPTSTR name_in,
                                                                                                                             LPUNKNOWN owner_in,
                                                                                                                             REFGUID CLSID_in,
                                                                                                                             HRESULT* result_out)
 : inherited (name_in,
              owner_in,
              CLSID_in,
              result_out)
 , filterConfiguration_ (NULL)
//, hasCOMReference_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T"));

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  OUTPUT_PIN_T* pin_p = NULL;
  HRESULT result = E_FAIL;
  ACE_NEW_NORETURN (pin_p,
                    OUTPUT_PIN_T (result_out,
                                  this,
                                  STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME));
  if (unlikely (!pin_p || (result_out && FAILED (*result_out))))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    if (result_out)
	  *result_out = E_OUTOFMEMORY;
    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: added output pin \"%s\"\n"),
  //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
  //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));
} // (Constructor)

template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
bool
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.pinConfiguration);

  IPin* ipin_p = Stream_MediaFramework_DirectShow_Tools::pin (this,
                                                              PINDIR_OUTPUT);
  if (unlikely (!ipin_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF
  IPIN_INITIALIZE_T* iinitialize_p = dynamic_cast<IPIN_INITIALIZE_T*> (ipin_p);
  if (unlikely (!iinitialize_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*>(%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  ipin_p->Release (); ipin_p = NULL;

  // *TODO*: remove type inference
  if (unlikely (!iinitialize_p->initialize (*configuration_in.pinConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  filterConfiguration_ = &const_cast<ConfigurationType&> (configuration_in);

  return true;

error:
  if (ipin_p)
    ipin_p->Release ();

  return false;
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename DataMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
bool
Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T<TimePolicyType,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        ConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType>::initialize (const MediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Asynch_Source_Filter_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  IPin* ipin_p =
    Stream_MediaFramework_DirectShow_Tools::pin (this, PINDIR_OUTPUT);
  if (unlikely (!ipin_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_MEDIA_INITIALIZE_T* iinitialize_p =
    dynamic_cast<IPIN_MEDIA_INITIALIZE_T*> (ipin_p);
  if (unlikely (!iinitialize_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  ipin_p->Release (); ipin_p = NULL;

  return iinitialize_p->initialize (*mediaType_in);

error:
  if (ipin_p)
    ipin_p->Release ();

  return false;
}

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T (HRESULT* result_out,
                                                                                                                                               FilterType* parentFilter_in,
                                                                                                                                               LPCWSTR pinName_in)
#if defined (UNICODE)
 : inherited (pinName_in,
#else
 : inherited (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (pinName_in)),
#endif // UNICODE
              parentFilter_in,                                                    // owning filter
              &lock_,                                                             // lock
              result_out,                                                         // OLE return code
              pinName_in,                                                         // pin name
              PINDIR_OUTPUT)                                                      // pin direction
 , allocatorProperties_ ()
 , configuration_ (NULL)
 , isInitialized_ (false)
 , mediaType_ (NULL)
 //, parentFilter_ (parent_in)
 , queue_ (NULL)
 , defaultFrameInterval_ (STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_FRAME_INTERVAL)
 , frameInterval_ (0)
 , numberOfMediaTypes_ (1)
 , flushing_ (false)
 , queriedForIAsyncReader_ (false)
 , requestQueue_ ()
 , lock_ ()
 , sampleTime_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T"));

  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  allocatorProperties_.cbAlign = 1;  // <-- use default
  allocatorProperties_.cbBuffer = -1; // <-- use default
  allocatorProperties_.cbPrefix = -1; // <-- use default
  allocatorProperties_.cBuffers = -1; // <-- use default
  //allocatorProperties_.cBuffers =
  //  MODULE_DEV_CAM_DIRECTSHOW_DEFAULT_DEVICE_BUFFERS;
} // (Constructor)

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::~Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::~Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T"));

  if (mediaType_)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_);
} // (Destructor)

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::NonDelegatingQueryInterface (REFIID riid_in,
                                                                                                          void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::NonDelegatingQueryInterface"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);

  if (InlineIsEqualGUID (riid_in, IID_IKsPropertySet))
    return GetInterface ((IKsPropertySet*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAMBufferNegotiation))
    return GetInterface ((IAMBufferNegotiation*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAMStreamConfig))
    return GetInterface ((IAMStreamConfig*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAsyncReader))
  {
    queriedForIAsyncReader_ = true;
    return GetInterface ((IAsyncReader*)this, interface_out);
  } // end ELSE IF

  return inherited::NonDelegatingQueryInterface (riid_in, interface_out);
}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::InitAllocator (IMemAllocator** allocator_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::InitAllocator"));

  // sanity check(s)
  CheckPointer (allocator_out, E_POINTER);

  // initialize return value(s)
  *allocator_out = NULL;

  HRESULT result = NOERROR;
  CMemAllocator* allocator_p = NULL;

  /* Create a default memory allocator */
  ACE_NEW_NORETURN (allocator_p,
                    CMemAllocator (NAME (STREAM_LIB_DIRECTSHOW_ALLOCATOR_NAME),
                                   NULL,
                                   &result));
  if (unlikely (!allocator_p || FAILED (result)))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    if (allocator_p)
      delete allocator_p;
    return (!allocator_p ? E_OUTOFMEMORY : result);
  } // end IF

  /* Get a reference counted IID_IMemAllocator interface */
  result = allocator_p->QueryInterface (IID_PPV_ARGS (allocator_out));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CMemAllocator::QueryInterface(IID_IMemAllocator): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    delete allocator_p;
    return result;
  } // end IF
  ACE_ASSERT (*allocator_out);

  return NOERROR;
}

// ---------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::Get (REFGUID guidPropSet_in,
                                                                                  DWORD dwPropID_in,
                                                                                  LPVOID pInstanceData_in,
                                                                                  DWORD cbInstanceData_in,
                                                                                  LPVOID pPropData_in,
                                                                                  DWORD cbPropData_in,
                                                                                  DWORD* pcbReturned_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::Get"));

  // sanity check(s)
  if (unlikely (!InlineIsEqualGUID (guidPropSet_in, AMPROPSETID_Pin)))
    return E_PROP_SET_UNSUPPORTED;
  if (unlikely (dwPropID_in != AMPROPERTY_PIN_CATEGORY))
    return E_PROP_ID_UNSUPPORTED;
  if (unlikely (pPropData_in == NULL && pcbReturned_in == NULL))
    return E_POINTER;

  if (pcbReturned_in)
    *pcbReturned_in = sizeof (struct _GUID);
  if (pPropData_in == NULL)  // Caller just wants to know the size.
    return S_OK;

  // sanity check(s)
  if (unlikely (cbPropData_in < sizeof (struct _GUID))) // The buffer is too small.
    return E_UNEXPECTED;

  *reinterpret_cast<struct _GUID*>(pPropData_in) = PIN_CATEGORY_CAPTURE;

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::QuerySupported (REFGUID guidPropSet_in,
                                                                                             DWORD dwPropID_in,
                                                                                             DWORD* pTypeSupport_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::QuerySupported"));

  // sanity check(s)
  if (unlikely (!InlineIsEqualGUID (guidPropSet_in, AMPROPSETID_Pin)))
    return E_PROP_SET_UNSUPPORTED;
  if (unlikely (dwPropID_in != AMPROPERTY_PIN_CATEGORY))
    return E_PROP_ID_UNSUPPORTED;

  if (pTypeSupport_in)
    // We support getting this property, but not setting it.
    *pTypeSupport_in = KSPROPERTY_SUPPORT_GET;

  return S_OK;
}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::SuggestAllocatorProperties (const struct _AllocatorProperties* pprop_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::SuggestAllocatorProperties"));

  // sanity check(s)
  CheckPointer (pprop_in, E_POINTER);

  allocatorProperties_ = *pprop_in;

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::GetAllocatorProperties (struct _AllocatorProperties* pprop_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::GetAllocatorProperties"));

  // sanity check(s)
  CheckPointer (pprop_inout, E_POINTER);

  *pprop_inout = allocatorProperties_;

  return S_OK;
}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::SetFormat (struct _AMMediaType* pmt_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::SetFormat"));

  // sanity check(s)
  ACE_ASSERT (mediaType_);
  if (unlikely (!pmt_in))
    pmt_in = mediaType_;

  // check compatibility
  IBaseFilter* filter_p =
    Stream_MediaFramework_DirectShow_Tools::toFilter (this);
  ACE_ASSERT (filter_p);
  CMediaType media_type, media_type_2;
  HRESULT result = media_type.Set (*pmt_in);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_type_2.Set (*mediaType_);
  ACE_ASSERT (SUCCEEDED (result));
  if (unlikely (!media_type.MatchesPartial (&media_type_2)))
  {
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: incompatible media types:\n%s\n%s\n, refusing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*pmt_in, true).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_, true).c_str ())));
#endif // _DEBUG
    filter_p->Release (); filter_p = NULL;
    return VFW_E_INVALIDMEDIATYPE;
  } // end IF

  if (pmt_in == mediaType_)
    goto continue_;
  if (mediaType_)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_);
  mediaType_ =
    Stream_MediaFramework_DirectShow_Tools::copy (*pmt_in);
  if (unlikely (!mediaType_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    filter_p->Release (); filter_p = NULL;
    return E_OUTOFMEMORY;
  } // end IF

continue_:
  // *TODO*: verify connectivity and state
  //         see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319788(v=vs.85).aspx
  // VFW_E_NOT_CONNECTED
  // VFW_E_NOT_STOPPED
  // VFW_E_WRONG_STATE

#if defined (_DEBUG)
  if (pmt_in == mediaType_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reset output format\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set output format: %s\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_, true).c_str ())));
#endif // _DEBUG
  filter_p->Release (); filter_p = NULL;

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::GetFormat (struct _AMMediaType** ppmt_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::GetFormat"));

  // sanity check(s)
  CheckPointer (ppmt_inout, E_POINTER);
  IPin* pin_p = NULL;
  //HRESULT result = inherited::ConnectedTo (&pin_p);
  //if (FAILED (result)) return result;
  //pin_p->Release ();
  ACE_ASSERT (!*ppmt_inout);
  ACE_ASSERT (mediaType_);

  *ppmt_inout =
    Stream_MediaFramework_DirectShow_Tools::copy (*mediaType_);
  if (unlikely (!*ppmt_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  ACE_ASSERT (*ppmt_inout);

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::GetNumberOfCapabilities (int* piCount_inout,
                                                                                                      int* piSize_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::GetNumberOfCapabilities"));

  // sanity check(s)
  CheckPointer (piCount_inout, E_POINTER);
  CheckPointer (piSize_inout, E_POINTER);

  *piCount_inout = 1;
  *piSize_inout = sizeof (struct _VIDEO_STREAM_CONFIG_CAPS);

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::GetStreamCaps (int iIndex_in,
                                                                                            struct _AMMediaType** ppmt_inout,
                                                                                            BYTE* pSCC_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::GetStreamCaps"));

  // sanity check(s)
  if (iIndex_in > 0)
    return S_FALSE; // E_INVALIDARG
  ACE_ASSERT (ppmt_inout);
  ACE_ASSERT (!*ppmt_inout);
  ACE_ASSERT (pSCC_in);
  ACE_ASSERT (mediaType_);

  *ppmt_inout =
    Stream_MediaFramework_DirectShow_Tools::copy (*mediaType_);
  if (unlikely (!*ppmt_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  ACE_ASSERT (*ppmt_inout);

  struct _VIDEO_STREAM_CONFIG_CAPS* capabilities_p =
    reinterpret_cast<struct _VIDEO_STREAM_CONFIG_CAPS*> (pSCC_in);
  ACE_OS::memset (capabilities_p,
                  0,
                  sizeof (struct _VIDEO_STREAM_CONFIG_CAPS));
  capabilities_p->guid = mediaType_->formattype;
  capabilities_p->VideoStandard = 0;
  struct tagVIDEOINFOHEADER* video_info_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_2 = NULL;
  if (InlineIsEqualGUID (mediaType_->formattype, FORMAT_VideoInfo))
  {
    video_info_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_->pbFormat);
    capabilities_p->InputSize.cx = video_info_p->bmiHeader.biWidth;
    capabilities_p->InputSize.cy = video_info_p->bmiHeader.biHeight;
    capabilities_p->MinCroppingSize.cx =
      (video_info_p->rcSource.right - video_info_p->rcSource.left);
    capabilities_p->MinCroppingSize.cy =
      (video_info_p->rcSource.bottom - video_info_p->rcSource.top);
    capabilities_p->MinOutputSize.cx = video_info_p->bmiHeader.biWidth;
    capabilities_p->MinOutputSize.cy = video_info_p->bmiHeader.biHeight;
    capabilities_p->MaxOutputSize.cx = video_info_p->bmiHeader.biWidth;
    capabilities_p->MaxOutputSize.cx = video_info_p->bmiHeader.biHeight;
    capabilities_p->MinFrameInterval = video_info_p->AvgTimePerFrame;
    capabilities_p->MaxFrameInterval = video_info_p->AvgTimePerFrame;
    capabilities_p->MinBitsPerSecond = video_info_p->dwBitRate;
    capabilities_p->MaxBitsPerSecond = video_info_p->dwBitRate;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_->formattype, FORMAT_VideoInfo2))
  {
    video_info_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_->pbFormat);
    capabilities_p->InputSize.cx = video_info_2->bmiHeader.biWidth;
    capabilities_p->InputSize.cy = video_info_2->bmiHeader.biHeight;
    capabilities_p->MinCroppingSize.cx =
      (video_info_2->rcSource.right - video_info_2->rcSource.left);
    capabilities_p->MinCroppingSize.cy =
      (video_info_2->rcSource.bottom - video_info_2->rcSource.top);
    capabilities_p->MinOutputSize.cx = video_info_2->bmiHeader.biWidth;
    capabilities_p->MinOutputSize.cy = video_info_2->bmiHeader.biHeight;
    capabilities_p->MaxOutputSize.cx = video_info_2->bmiHeader.biWidth;
    capabilities_p->MaxOutputSize.cx = video_info_2->bmiHeader.biHeight;
    capabilities_p->MinFrameInterval = video_info_2->AvgTimePerFrame;
    capabilities_p->MaxFrameInterval = video_info_2->AvgTimePerFrame;
    capabilities_p->MinBitsPerSecond = video_info_2->dwBitRate;
    capabilities_p->MaxBitsPerSecond = video_info_2->dwBitRate;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknnown media type format (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_->formattype).c_str ())));
    return E_OUTOFMEMORY;
  } // end ELSE

  return S_OK;
}

//
// CheckMediaType
//
// accept the preconfigured media type, if any
// Returns E_INVALIDARG if the mediatype is not acceptable
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::CheckMediaType (const CMediaType *mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::CheckMediaType"));

  // sanity check(s)
  CheckPointer (mediaType_in, E_POINTER);
  ACE_ASSERT (mediaType_);
  //ACE_ASSERT (parentFilter_);

  //CAutoLock cAutoLock (&parentFilter_->lock_);

  CMediaType media_type;
  HRESULT result = media_type.Set (*mediaType_);
  ACE_ASSERT (SUCCEEDED (result));
  if (unlikely (!media_type.MatchesPartial (mediaType_in)))
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s/%s: incompatible media types (\"%s\"\n\"%s\")\n"),
    //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (parentFilter_).c_str ()),
    //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*configuration_->format).c_str ()),
    //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_in).c_str ())));
    return S_FALSE;
  } // end IF

  return S_OK;
} // CheckMediaType

//
// GetMediaType
//
// Prefered types should be ordered by quality, zero as highest quality
// (iPosition > 4 is invalid)
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::GetMediaType (int position_in,
                                                                                           CMediaType* mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::GetMediaType"));

  if (position_in < 0)
    return E_INVALIDARG;
  if (position_in > 0)
    return VFW_S_NO_MORE_ITEMS;

  // sanity check(s)
  CheckPointer (mediaType_out, E_POINTER);
  ACE_ASSERT (mediaType_);

  return mediaType_out->Set (*mediaType_);
} // GetMediaType

//
// SetMediaType
//
// Called when a media type is agreed between filters
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::SetMediaType (const CMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::SetMediaType"));

  // sanity check(s)
  CheckPointer (mediaType_in, E_POINTER);
  //ACE_ASSERT (parentFilter_);

  //CAutoLock cAutoLock (&(parentFilter_->lock_));

  // pass the call up to my base class
  HRESULT result = inherited::SetMediaType (mediaType_in);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CSourceStream::SetMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return result;
  } // end IF
#if defined (_DEBUG)
  IBaseFilter* ibase_filter_p =
    Stream_MediaFramework_DirectShow_Tools::toFilter (this);
  ACE_ASSERT (ibase_filter_p);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set media type: %s"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_p).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_in, true).c_str ())));
  ibase_filter_p->Release (); ibase_filter_p = NULL;
#endif // _DEBUG

  return S_OK;
} // SetMediaType

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::CheckConnect (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::CheckConnect"));

  //queriedForIAsyncReader_ = false;

  return inherited::CheckConnect (pin_in);
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::CompleteConnect (IPin* pin_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::CompleteConnect"));

  //if (queriedForIAsyncReader_)
    return inherited::CompleteConnect (pin_in);

//#ifdef VFW_E_NO_TRANSPORT
//  return VFW_E_NO_TRANSPORT;
//#else
//  return E_FAIL;
//#endif
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::BreakConnect ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::BreakConnect"));

  queriedForIAsyncReader_ = false;

  return inherited::BreakConnect ();
}

// ------------------------------------

//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
//                                                       FilterType,
//                                                       MediaType>::Connect (IPin* inputPin_in,
//                                                                            const struct _AMMediaType* mediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::Connect"));
//
//  return inherited::Connect (inputPin_in, mediaType_in);
//}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::BeginFlush (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::BeginFlush"));

  // sanity check(s)
  ACE_ASSERT (!flushing_);

  flushing_ = true;

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::EndFlush (void)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::EndFlush"));

  // sanity check(s)
  ACE_ASSERT (!flushing_);

  flushing_ = false;

  return S_OK;
}

// return the length of the file, and the length currently
// available locally. We only support locally accessible files,
// so they are always the same
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::Length (LONGLONG* total_out,
                                                                                     LONGLONG* available_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::Length"));

  // sanity check(s)
  ACE_ASSERT (total_out);
  ACE_ASSERT (available_out);
  ACE_ASSERT (queue_);

  *total_out = std::numeric_limits<LONGLONG>::max ();
  *available_out = static_cast<LONGLONG> (queue_->message_length ());

  return S_OK;
}

// queue an aligned read request. call WaitForNext to get
// completion.
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::Request (IMediaSample* mediaSample_in,
                                                                                      DWORD_PTR userContext_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::Request"));

  // sanity check(s)
  CheckPointer (mediaSample_in, E_POINTER);
  if (flushing_)
    return VFW_E_WRONG_STATE;

  struct Stream_DirectShow_AsyncReadRequest request_s;
  request_s.mediaSample = mediaSample_in;
  request_s.userContext = userContext_in;
  requestQueue_.push (request_s);

  return S_OK;
}

// we need to return an addrefed allocator, even if it is the preferred
// one, since he doesn't know whether it is the preferred one or not.
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::RequestAllocator (IMemAllocator* allocator_in,
                                                                                               struct _AllocatorProperties* properties_in,
                                                                                               IMemAllocator** allocator_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::RequestAllocator"));

  // sanity check(s)
  CheckPointer (allocator_in, E_POINTER);
  CheckPointer (properties_in, E_POINTER);
  CheckPointer (allocator_out, E_POINTER);
  //ACE_ASSERT (parentFilter_);

  HRESULT result = E_FAIL;
  struct _AllocatorProperties allocator_properties;
  IBaseFilter* ibase_filter_p =
    Stream_MediaFramework_DirectShow_Tools::toFilter (this);
  ACE_ASSERT (ibase_filter_p);
  IPin* ipin_p = NULL;
  IBaseFilter* ibase_filter_2 = NULL;
  IMemAllocator* allocator_p = NULL;

  result = this->ConnectedTo (&ipin_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IPin::ConnectedTo(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (ipin_p);
  ibase_filter_2 = Stream_MediaFramework_DirectShow_Tools::toFilter (ipin_p);
  ACE_ASSERT (ibase_filter_2);

  if (allocator_in)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: input pin (was: %s/%s) provides an allocator\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_2).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ipin_p).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

    result = allocator_in->SetProperties (properties_in,
                                          &allocator_properties);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMemAllocator::SetProperties(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    allocator_in->AddRef ();
    *allocator_out = allocator_in;

    goto continue_;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: input pin (was: %s/%s) does not provide an allocator...\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_p).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_2).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ipin_p).c_str ()),
              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  // create own allocator
  result = InitAllocator (&allocator_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::InitAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (allocator_p);

  //...and check if it is suitable
  result = allocator_p->SetProperties (properties_in,
                                       &allocator_properties);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemAllocator::SetProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  *allocator_out = allocator_p;
  allocator_p = NULL;

continue_:
  ACE_DEBUG ((LM_DEBUG,
            ACE_TEXT ("%s/%s: negotiated buffer properties: %d x %d byte(s) (alignment: %d, prefix: %d byte(s)\n"),
            ACE_TEXT (STREAM_LIB_DIRECTSHOW_FILTER_NAME_ASYNCH_SOURCE),
            ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME),
            allocator_properties.cBuffers, allocator_properties.cbBuffer,
            allocator_properties.cbAlign, allocator_properties.cbPrefix));

  // clean up
  ibase_filter_p->Release ();
  ipin_p->Release ();
  ibase_filter_2->Release ();

  return S_OK;

error:
  if (ibase_filter_p)
    ibase_filter_p->Release ();
  if (ipin_p)
    ipin_p->Release ();
  if (ibase_filter_2)
    ibase_filter_2->Release ();
  if (allocator_p)
    allocator_p->Release ();

  return result;
}

//
// synchronous read that need not be aligned.
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::SyncRead (LONGLONG position_in,
                                                                                       LONG length_in,
                                                                                       BYTE* buffer_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::SyncRead"));

  ACE_UNUSED_ARG (position_in);
  ACE_UNUSED_ARG (length_in);
  ACE_UNUSED_ARG (buffer_out);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_NOTIMPL);

  ACE_NOTREACHED (return E_NOTIMPL;)
}

// sync-aligned request. just like a request/waitfornext pair.
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::SyncReadAligned (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::SyncReadAligned"));

  ACE_UNUSED_ARG (mediaSample_in);

  ACE_ASSERT (false);

  ACE_NOTSUP_RETURN (E_NOTIMPL);
  ACE_NOTREACHED (return E_NOTIMPL;)
}

//
// collect the next ready sample
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::WaitForNext (DWORD timeout_in,
                                                                                          IMediaSample** mediaSample_out,
                                                                                          DWORD_PTR* userContext_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::WaitForNext"));

  // sanity check(s)
  CheckPointer (mediaSample_out, E_POINTER);
  CheckPointer (userContext_out, E_POINTER);
  ACE_ASSERT (queue_);
  ACE_ASSERT (!requestQueue_.empty ());

  // initialize return value(s)
  *mediaSample_out = NULL;

  int result_2 = -1;
  ACE_Message_Block* message_block_p = NULL;
  ACE_Time_Value timeout;
  ACE_Time_Value* timeout_p = &timeout;
  struct Stream_DirectShow_AsyncReadRequest request_s;
  // currently flushing ?
  if (flushing_)
  {
    //result_2 = queue_->flush ();
    //if (result_2 == -1)
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to ACE_Message_Queue_Base::flush(): \"%m\", continuing\n")));
    timeout = COMMON_TIME_NOW;
    do
    {
      message_block_p = NULL;
      result_2 = queue_->dequeue_head (message_block_p, timeout_p);
      if (unlikely (result_2 == -1))
      {
        int error = ACE_OS::last_error ();
        if (error != ETIMEDOUT)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue_head(): \"%m\", continuing\n")));
        break;
      } // end IF
      ACE_ASSERT (message_block_p);
      message_block_p->release ();
    } while (true);

    do
    {
      request_s = requestQueue_.front ();
      request_s.mediaSample->Release ();
      requestQueue_.pop ();
    } while (!requestQueue_.empty ());

    return VFW_E_WRONG_STATE; // pin is flushing
  } // end IF

  if (timeout_in == INFINITE)
    timeout_p = NULL;
  else
  {
    timeout.msec (static_cast<long> (timeout_in));
    timeout += COMMON_TIME_NOW;
  } // end ELSE

  HRESULT result = E_FAIL;
  result_2 = queue_->dequeue_head (message_block_p, timeout_p);
  if (unlikely (result_2 == -1))
  {
    int error = ACE_OS::last_error ();
    if (error == ETIMEDOUT) result = VFW_E_TIMEOUT;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue_head(): \"%m\", aborting\n")));
    return result;
  } // end IF
  ACE_ASSERT (message_block_p);
  if (message_block_p->msg_type () == ACE_Message_Block::MB_STOP)
  {
    message_block_p->release ();
    return S_FALSE; // --> stop 'streaming thread'
  } // end IF

  // *TODO*: normal filters reuse the media samples passed in Request()
  IMediaSample* media_sample_p = dynamic_cast<IMediaSample*> (message_block_p);
  if (unlikely (!media_sample_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<IMediaSample*>(%@), aborting\n"),
                message_block_p));
    message_block_p->release ();
    return result;
  } // end IF
  *mediaSample_out = media_sample_p;

  request_s = requestQueue_.front ();
  *userContext_out = request_s.userContext;
  requestQueue_.pop ();

  // clean up
  request_s.mediaSample->Release ();

  return S_OK;
}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
bool
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  // *TODO*: remove type inferences
  //mediaType_ = configuration_->mediaType;
  queue_ = configuration_->queue;

  isInitialized_ = true;

  return true;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
bool
Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T<ConfigurationType,
                                                                 FilterType,
                                                                 MediaType>::initialize (const MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_AsynchOutputPin_T::initialize"));

  // sanity check(s)
  //ACE_ASSERT (parentFilter_);

  if (mediaType_)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_);
  mediaType_ =
    Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in);
  if (unlikely (!mediaType_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n")));
    return false;
  } // end IF

  IBaseFilter* ibase_filter_p =
    Stream_MediaFramework_DirectShow_Tools::toFilter (this);
  ACE_ASSERT (ibase_filter_p);

  ACE_DEBUG ((LM_DEBUG,
            ACE_TEXT ("%s/%s: set default output format: %s\n"),
            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ibase_filter_p).c_str ()),
            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_, true).c_str ())));

  // clean up
  ibase_filter_p->Release ();

  return true;
}
