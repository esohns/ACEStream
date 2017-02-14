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

#include <vfwmsgs.h>

#include <ace/Log_Msg.h>
#include <ace/Message_Block.h>
#include <ace/OS_Memory.h>

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

//#include "stream_dev_defines.h"
#include "stream_dev_directshow_tools.h"

#include "stream_misc_common.h"
#include "stream_misc_defines.h"

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
CUnknown* WINAPI
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::CreateInstance (LPUNKNOWN IUnknown_in,
                                                                   HRESULT* result_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::CreateInstance"));

  // sanity check(s)
  ACE_ASSERT (result_out);

  // initialize return value(s)
  *result_out = S_OK;

  CUnknown* unknown_p = NULL;
  ACE_NEW_NORETURN (unknown_p,
                    OWN_TYPE_T (NAME (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE),
                                IUnknown_in,
                                CLSID_ACEStream_Source_Filter,
                                result_out));
  if (!unknown_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    *result_out = E_OUTOFMEMORY;
  } // end IF

  return unknown_p;
} // CreateInstance
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
void WINAPI
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::DeleteInstance (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::DeleteInstance"));

  // sanity check(s)
  ACE_ASSERT (pointer_in);

  //CUnknown* unknown_p = reinterpret_cast<CUnknown*> (pointer_in);
  //OWN_TYPE_T* instance_p = dynamic_cast<OWN_TYPE_T*> (unknown_p);
  //ACE_ASSERT (instance_p);
  OWN_TYPE_T* instance_p = static_cast<OWN_TYPE_T*> (pointer_in);

  ::delete instance_p;
} // DeleteInstance

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
void
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::operator delete (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::operator delete"));

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
//          typename ProtocolMessageType,
//          typename MediaType,
//          typename ModuleType>
//void
//Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       MediaType,
//                                       ModuleType>::operator delete (void* pointer_in,
//                                                                     size_t bytes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::operator delete"));
//
//  ACE_UNUSED_ARG (bytes_in);
//
//  // *NOTE*: see above
//  OWN_TYPE_T::DeleteInstance (pointer_in);
//}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::Stream_Misc_DirectShow_Source_Filter_T ()
 : inherited (NAME (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE), // name
              NULL,                                           // owner
              CLSID_ACEStream_Source_Filter,                  // CLSID
              NULL)                                           // result
 , filterConfiguration_ (NULL)
 , allocator_ (NULL)
 , allocatorProperties_ ()
//, hasCOMReference_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Stream_Misc_DirectShow_Source_Filter_T"));

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  OUTPUT_PIN_T* pin_p = NULL;
  HRESULT result = E_FAIL;
  ACE_NEW_NORETURN (pin_p,
                    OUTPUT_PIN_T (&result,
                                  this,
                                  MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME));
  if (!pin_p ||
      FAILED (result))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: added output pin \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));

  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  //allocatorProperties_.cbAlign = -1;  // <-- use default
  allocatorProperties_.cbAlign = 1;
  allocatorProperties_.cbBuffer = -1; // <-- use default
  allocatorProperties_.cbPrefix = -1; // <-- use default
  allocatorProperties_.cBuffers =
    MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS;
  //allocatorProperties_.cBuffers = -1; // <-- use default
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::Stream_Misc_DirectShow_Source_Filter_T (LPTSTR name_in,
                                                                                           LPUNKNOWN owner_in,
                                                                                           const struct _GUID& CLSID_in,
                                                                                           HRESULT* result_out)
 : inherited (name_in,
              owner_in,
              CLSID_in,
              result_out)
 , filterConfiguration_ (NULL)
 , allocator_ (NULL)
 , allocatorProperties_ ()
//, hasCOMReference_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Stream_Misc_DirectShow_Source_Filter_T"));

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  OUTPUT_PIN_T* pin_p = NULL;
  ACE_NEW_NORETURN (pin_p,
                    OUTPUT_PIN_T (result_out,
                                  this,
                                  MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME));
  if (!pin_p ||
      (result_out && FAILED (*result_out)))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));

    if (result_out) *result_out = E_OUTOFMEMORY;

    return;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: added output pin \"%s\"...\n"),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));

  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  //allocatorProperties_.cbAlign = -1;  // <-- use default
  allocatorProperties_.cbAlign = 1;
  allocatorProperties_.cbBuffer = -1; // <-- use default
  allocatorProperties_.cbPrefix = -1; // <-- use default
  allocatorProperties_.cBuffers =
    MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS;
  //allocatorProperties_.cBuffers = -1; // <-- use default
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::~Stream_Misc_DirectShow_Source_Filter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::~Stream_Misc_DirectShow_Source_Filter_T"));

  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
  HRESULT result = inherited::QueryFilterInfo (&filter_info);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IBaseFilter::QueryFilterInfo(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

  // step1: disconnect from graph
  if (!Stream_Module_Device_DirectShow_Tools::disconnect (this))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::disconnect(), continuing\n")));

  // step2: remove from graph
  if (filter_info.pGraph)
  {
    result = filter_info.pGraph->RemoveFilter (this);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IFilterGraph::RemoveFilter(%s): \"%s\", continuing\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  } // end IF
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::NonDelegatingQueryInterface (REFIID riid_in,
                                                                                void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::NonDelegatingQueryInterface"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);

  if (riid_in == IID_IMemAllocator)
    return GetInterface ((IMemAllocator*)this, interface_out);

  return inherited::NonDelegatingQueryInterface (riid_in, interface_out);
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::SetProperties (struct _AllocatorProperties* requestedProperties_in,
                                                                  struct _AllocatorProperties* properties_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::SetProperties"));

  // sanity check(s)
  CheckPointer (requestedProperties_in, E_POINTER);
  CheckPointer (properties_out, E_POINTER);

  // step1: find 'least common denominator'
  properties_out->cBuffers =
    std::min (requestedProperties_in->cBuffers, allocatorProperties_.cBuffers);
  properties_out->cbBuffer =
    std::min (requestedProperties_in->cbBuffer, allocatorProperties_.cbBuffer);
  // *TODO*: cannot align buffers at this time
  ACE_ASSERT (requestedProperties_in->cbAlign <= 1);
  properties_out->cbPrefix =
    std::min (requestedProperties_in->cbPrefix, allocatorProperties_.cbPrefix);

  // step2: validate against set properties
  // *TODO*

  return NOERROR;
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::GetBuffer (IMediaSample** mediaSample_out,
                                                              REFERENCE_TIME* startTime_out,
                                                              REFERENCE_TIME* endTime_out,
                                                              DWORD flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::GetBuffer"));

  ACE_UNUSED_ARG (flags_in);

  // sanity check(s)
  CheckPointer (mediaSample_out, E_POINTER);
  //CheckPointer (startTime_out, E_POINTER);
  //CheckPointer (endTime_out, E_POINTER);
  ACE_ASSERT (allocatorProperties_.cBuffers && allocatorProperties_.cbBuffer);

  // initialize return value(s)
  *mediaSample_out = NULL;
  //*startTime_out = 0;
  //*endTime_out = 0;

  // step1: allocate message
  unsigned int message_size = (allocatorProperties_.cbPrefix +
                               allocatorProperties_.cbBuffer);
  ProtocolMessageType* message_p = NULL;
  if (allocator_)
  {
allocate:
    try {
      message_p =
        static_cast<ProtocolMessageType*> (allocator_->malloc (message_size));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  message_size));
      return E_OUTOFMEMORY;
    }

    // keep retrying ?
    if (!message_p && !allocator_->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      ProtocolMessageType (message_size));
  if (!message_p)
  {
    if (allocator_)
    {
      if (allocator_->block ())
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate data message: \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  *mediaSample_out = dynamic_cast<IMediaSample*> (message_p);
  if (!*mediaSample_out)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<IMediaSample*>(ProtocolMessageType): check implementation !, aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ())));

    // clean up
    message_p->release ();

    return E_FAIL;
  } // end IF

  // step2: set times
  // *TODO*: calculate times from the average sample rate and the reference
  //         start clock

  return NOERROR;
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::ReleaseBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::ReleaseBuffer"));

  // sanity check(s)
  CheckPointer (mediaSample_in, E_POINTER);

  ACE_Message_Block* message_p =
    dynamic_cast<ACE_Message_Block*> (mediaSample_in);
  if (message_p)
    message_p->release ();
  else
    mediaSample_in->Release ();

  return NOERROR;
}

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
bool
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.pinConfiguration);
  ACE_ASSERT (configuration_in.pinConfiguration->format);

  IPin* ipin_p = Stream_Module_Device_DirectShow_Tools::pin (this,
                                                             PINDIR_OUTPUT);
  if (!ipin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_INITIALIZE_T* iinitialize_p = dynamic_cast<IPIN_INITIALIZE_T*> (ipin_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  if (!iinitialize_p->initialize (*configuration_in.pinConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_MEDIA_INITIALIZE_T* iinitialize_2 =
    dynamic_cast<IPIN_MEDIA_INITIALIZE_T*> (ipin_p);
  if (!iinitialize_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  if (!iinitialize_2->initialize (*configuration_in.pinConfiguration->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF
  ipin_p->Release ();
  ipin_p = NULL;

  filterConfiguration_ = &const_cast<ConfigurationType&> (configuration_in);
  allocator_ = configuration_in.allocator;
  allocatorProperties_ = configuration_in.allocatorProperties;

  return true;

error:
  if (ipin_p)
    ipin_p->Release ();

  return false;
}
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
bool
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ConfigurationType,
                                       PinConfigurationType,
                                       MediaType>::initialize (const MediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  IPin* ipin_p = Stream_Module_Device_DirectShow_Tools::pin (this,
                                                             PINDIR_OUTPUT);
  if (!ipin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_MEDIA_INITIALIZE_T* iinitialize_p =
    dynamic_cast<IPIN_MEDIA_INITIALIZE_T*> (ipin_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  ipin_p->Release ();
  ipin_p = NULL;

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
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::Stream_Misc_DirectShow_Source_Filter_OutputPin_T (HRESULT* result_out,
                                                                                                               CSource* parentFilter_in,
                                                                                                               LPCWSTR pinName_in)
 : inherited (NAME (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (pinName_in))),
              result_out,
              parentFilter_in,
              pinName_in)
 , configuration_ (NULL)
 , isInitialized_ (false)
 , mediaType_ (NULL)
 , parentFilter_ (dynamic_cast<FilterType*> (parentFilter_in))
 , queue_ (NULL)
 /////////////////////////////////////////
 , defaultFrameInterval_ (MODULE_MISC_DS_WIN32_FILTER_SOURCE_FRAME_INTERVAL)
 , frameInterval_ (0)
 , hasMediaSampleBuffers_ (false)
 , isTopToBottom_ (false)
 , numberOfMediaTypes_ (1)
 , lock_ ()
 , sampleTime_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::Stream_Misc_DirectShow_Source_Filter_OutputPin_T"));

  // sanity check(s)
  ACE_ASSERT (result_out);
  if (!parentFilter_)
  { ACE_ASSERT (parentFilter_in);
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<FilterType*>(0x%@), aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (parentFilter_in).c_str ()),
                ACE_TEXT_WCHAR_TO_TCHAR (pinName_in),
                parentFilter_in));

    *result_out = E_FAIL;

    return;
  } // end IF
} // (Constructor)

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::~Stream_Misc_DirectShow_Source_Filter_OutputPin_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::~Stream_Misc_DirectShow_Source_Filter_OutputPin_T"));

  //HRESULT result = E_FAIL;
  //if (allocator_)
  //{
  //  result = allocator_->Decommit ();
  //  if (FAILED (result))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMemAllocator::Decommit(): \"%s\", continuing\n"),
  //                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  allocator_->Release ();
  //} // end IF

  if (mediaType_)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_);
} // (Destructor)

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
bool
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  // *TODO*: remove type inferences
  hasMediaSampleBuffers_ = configuration_->hasMediaSampleBuffers;
  isTopToBottom_ = configuration_->isTopToBottom;
  queue_ = configuration_->queue;

  isInitialized_ = true;

  return true;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
bool
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::initialize (const MediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize"));

  if (mediaType_)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_);
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in,
                                                             mediaType_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(): \"%m\", aborting\n")));
    return false;
  } // end IF

  IBaseFilter* ibase_filter_p =
    Stream_Module_Device_DirectShow_Tools::pin2Filter (this);
  ACE_ASSERT (SUCCEEDED (ibase_filter_p));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set default output format: %s...\n"),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (ibase_filter_p).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*mediaType_, true).c_str ())));

  // clean up
  ibase_filter_p->Release ();

  return true;
}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::NonDelegatingQueryInterface (REFIID riid_in,
                                                                                          void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::NonDelegatingQueryInterface"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);

  if (riid_in == IID_IKsPropertySet)
    return GetInterface ((IKsPropertySet*)this, interface_out);
  else if (riid_in == IID_IAMBufferNegotiation)
    return GetInterface ((IAMBufferNegotiation*)this, interface_out);
  else if (riid_in == IID_IAMStreamConfig)
    return GetInterface ((IAMStreamConfig*)this, interface_out);

  return inherited::NonDelegatingQueryInterface (riid_in, interface_out);
}

// ------------------------------------

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
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::CheckMediaType (const CMediaType *mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::CheckMediaType"));

  // sanity check(s)
  CheckPointer (mediaType_in, E_POINTER);
  //if ((*(mediaType_in->Type ()) != MEDIATYPE_Video) ||
  //    !mediaType_in->IsFixedSize ())
  //  return E_FAIL;
  ACE_ASSERT (mediaType_);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  CMediaType media_type;
  HRESULT result = media_type.Set (*mediaType_);
  ACE_ASSERT (SUCCEEDED (result));
  if (!media_type.MatchesPartial (mediaType_in))
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s/%s: incompatible media types (\"%s\"\n\"%s\")\n"),
    //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (parentFilter_).c_str ()),
    //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ())));
    //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*configuration_->format).c_str ()),
    //            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*mediaType_in).c_str ())));
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
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::GetMediaType (int position_in,
                                                                           CMediaType* mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::GetMediaType"));

  HRESULT result = E_FAIL;

  if (position_in < 0)
    return E_INVALIDARG;
  // *TODO*: implement a default set of supported media types
  if (static_cast<unsigned int> (position_in) > (numberOfMediaTypes_ - 1))
    return VFW_S_NO_MORE_ITEMS;

  // sanity check(s)
  CheckPointer (mediaType_out, E_POINTER);
  ACE_ASSERT (mediaType_);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  result = mediaType_out->Set (*mediaType_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CMediaType::Set(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  // correct frame orientation ?
  if (mediaType_out->majortype == MEDIATYPE_Video)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
    struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
    if (mediaType_out->formattype == FORMAT_VideoInfo)
    { ACE_ASSERT (mediaType_out->cbFormat ==
                  sizeof (struct tagVIDEOINFOHEADER));
      video_info_header_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_out->pbFormat);
      ACE_ASSERT (video_info_header_p);
      if (isTopToBottom_)
      {
        if (video_info_header_p->bmiHeader.biHeight > 0)
          video_info_header_p->bmiHeader.biHeight =
            -video_info_header_p->bmiHeader.biHeight;
      }
      else if (video_info_header_p->bmiHeader.biHeight < 0)
        video_info_header_p->bmiHeader.biHeight =
          -video_info_header_p->bmiHeader.biHeight;
    } // end IF
    else if (mediaType_out->formattype == FORMAT_VideoInfo2)
    { ACE_ASSERT (mediaType_out->cbFormat ==
                  sizeof (struct tagVIDEOINFOHEADER2));
      video_info_header2_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_out->pbFormat);
      ACE_ASSERT (video_info_header2_p);
      if (isTopToBottom_)
      {
        if (video_info_header2_p->bmiHeader.biHeight > 0)
          video_info_header2_p->bmiHeader.biHeight =
            -video_info_header2_p->bmiHeader.biHeight;
      }
      else if (video_info_header2_p->bmiHeader.biHeight < 0)
        video_info_header2_p->bmiHeader.biHeight =
          -video_info_header2_p->bmiHeader.biHeight;
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_out->formattype).c_str ())));
      return E_FAIL;
    } // end ELSE
  } // end IF

  return S_OK;
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
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::SetMediaType (const CMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::SetMediaType"));

  // sanity check(s)
  CheckPointer (mediaType_in, E_POINTER);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  HRESULT result = inherited::SetMediaType (mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CSourceStream::SetMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  // compute frame interval to correctly set the sample time
  REFERENCE_TIME avg_time_per_frame; // 100ns units
  if (inherited::m_mt.majortype == MEDIATYPE_Video)
  {
    struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
    struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
    if (inherited::m_mt.formattype == FORMAT_VideoInfo)
    { ACE_ASSERT (inherited::m_mt.cbFormat ==
                  sizeof (struct tagVIDEOINFOHEADER));
      video_info_header_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (inherited::m_mt.pbFormat);
      avg_time_per_frame = video_info_header_p->AvgTimePerFrame;
    } // end IF
    else if (inherited::m_mt.formattype == FORMAT_VideoInfo2)
    { ACE_ASSERT (inherited::m_mt.cbFormat ==
                  sizeof (struct tagVIDEOINFOHEADER2));
      video_info_header2_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER2*> (inherited::m_mt.pbFormat);
      avg_time_per_frame = video_info_header2_p->AvgTimePerFrame;
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (inherited::m_mt.formattype).c_str ())));
      return E_FAIL;
    } // end ELSE
    frameInterval_ = avg_time_per_frame / 10000; // 100ns --> ms
  } // end IF
  else if (inherited::m_mt.majortype == MEDIATYPE_Audio)
  { ACE_ASSERT (inherited::m_mt.cbFormat == sizeof (struct tWAVEFORMATEX));
    struct tWAVEFORMATEX* waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (inherited::m_mt.pbFormat);
    frameInterval_ = waveformatex_p->nSamplesPerSec / 1000;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media type major type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (inherited::m_mt.majortype).c_str ())));
    return E_FAIL;
  } // end ELSE

  // *NOTE*: the 'intelligent' DirectShow graph assembly implementation (i.e.
  //         IGraphBuilder::Connect()) involves media type resolution. In this
  //         case, the media type argument 'format type' field passed in here
  //         can be GUID_NULL, which means 'not specified'.
  //         Given the publicly available documentation, for video types, this
  //         essentially means that FORMAT_VideoInfo and FORMAT_VideoInfo2 must
  //         both be supported. However, support for this feature is 'brittle';
  //         many 'built-in' filter(-pin)s have not been updated and support
  //         only FORMAT_VideoInfo, while more recent filters only support
  //         FORMAT_VideoInfo2, leading to compatibility issues
  // *NOTE*: how this 'feature creep' effectively also breaks support for
  //         vendor-specific filter implementations
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set media type: %s\n"),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*mediaType_in, true).c_str ())));

  return S_OK;
} // SetMediaType

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::DecideAllocator (IMemInputPin* inputPin_in,
                                                                              IMemAllocator** allocator_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::DecideAllocator"));

  // sanity check(s)
  CheckPointer (inputPin_in, E_POINTER);
  CheckPointer (allocator_out, E_POINTER);
  ACE_ASSERT (!*allocator_out);
  ACE_ASSERT (inherited::m_pFilter);
  //ACE_ASSERT (!allocator_);
  ACE_ASSERT (!inherited::m_pAllocator);
  ACE_ASSERT (parentFilter_);

  HRESULT result = E_FAIL;
  IPin* pin_p = NULL;
  IBaseFilter* filter_p = NULL;
  ULONG reference_count = 0;

  // debug info
  result = inputPin_in->QueryInterface (IID_PPV_ARGS (&pin_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemInputPin::QueryInterface(IID_IPin): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (pin_p);
  filter_p = Stream_Module_Device_DirectShow_Tools::pin2Filter (pin_p);
  ACE_ASSERT (filter_p);

  struct _AllocatorProperties allocator_requirements;
  ACE_OS::memset (&allocator_requirements,
                  0,
                  sizeof (struct _AllocatorProperties));
  result = inputPin_in->GetAllocatorRequirements (&allocator_requirements);
  if (FAILED (result)) // E_NOTIMPL: 0x80004001
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: failed to IMemInputPin::GetAllocatorRequirements(): \"%s\", continuing\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result, true).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: allocator requirements (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                allocator_requirements.cBuffers,
                allocator_requirements.cbBuffer,
                allocator_requirements.cbAlign,
                allocator_requirements.cbPrefix));

  // *NOTE*: see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319039(v=vs.85).aspx

  // use input pins' allocator ?
  result = inputPin_in->GetAllocator (allocator_out);
  if (SUCCEEDED (result))
  {
    // sanity check(s)
    ACE_ASSERT (*allocator_out);

    LONG reference_count = (*allocator_out)->AddRef ();
    inherited::m_pAllocator = *allocator_out;

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: using allocator from %s/%s...\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ())));

    result = (*allocator_out)->GetProperties (&allocator_requirements);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMemAllocator::GetProperties(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF

    struct _AllocatorProperties actual_properties;
    ACE_OS::memset (&actual_properties,
                    0,
                    sizeof (struct _AllocatorProperties));
    result =
      (*allocator_out)->SetProperties (&parentFilter_->allocatorProperties_,
                                       &actual_properties);
    if (FAILED (result)) // E_INVALIDARG: 0x80070057
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s/%s: failed to IMemAllocator::SetProperties(buffers/size/alignment/prefix: %d/%d/%d/%d): \"%s\", falling back\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                  parentFilter_->allocatorProperties_.cBuffers,
                  parentFilter_->allocatorProperties_.cbBuffer,
                  parentFilter_->allocatorProperties_.cbAlign,
                  parentFilter_->allocatorProperties_.cbPrefix,
                  ACE_TEXT (Common_Tools::error2String (result, true).c_str ())));

      // clean up
      (*allocator_out)->Release ();
      *allocator_out = NULL;

      goto continue_;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: negotiated allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                actual_properties.cBuffers,
                actual_properties.cbBuffer,
                actual_properties.cbAlign,
                actual_properties.cbPrefix));

    goto notify;
  } // end IF

continue_:
  // --> roll our own
  ACE_ASSERT (!*allocator_out);

  // *NOTE*: how this really makes sense for asynchronous filters only
  if (hasMediaSampleBuffers_)
    *allocator_out = parentFilter_;
  else
  {
    result = inherited::InitAllocator (allocator_out);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to CBaseOutputPin::InitAllocator(): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (*allocator_out);

  result = DecideBufferSize (*allocator_out,
                             &allocator_requirements);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to CBaseOutputPin::DecideBufferSize(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
            ACE_TEXT ("%s/%s: negotiated allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
            ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (this).c_str ()),
            allocator_requirements.cBuffers,
            allocator_requirements.cbBuffer,
            allocator_requirements.cbAlign,
            allocator_requirements.cbPrefix));

notify:
  ACE_ASSERT (*allocator_out);
  ACE_ASSERT (inherited::m_pAllocator == *allocator_out);

  result = inputPin_in->NotifyAllocator (*allocator_out,
                                         FALSE); // read-only buffers ?
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMemInputPin::NotifyAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    goto error;
  } // end IF

  if (pin_p) pin_p->Release ();
  if (filter_p) filter_p->Release ();

  return S_OK;

error:
  if (pin_p) pin_p->Release ();
  if (filter_p) filter_p->Release ();
  //if (allocator_)
  //{
  //  allocator_->Release ();
  //  allocator_ = NULL;
  //} // end IF
  if (inherited::m_pAllocator)
  {
    inherited::m_pAllocator->Release ();
    inherited::m_pAllocator = NULL;
  } // end IF

  return result;
}

//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::DecideBufferSize (IMemAllocator* allocator_in,
                                                                               struct _AllocatorProperties* properties_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::DecideBufferSize"));

  // sanity check(s)
  CheckPointer (allocator_in, E_POINTER);
  CheckPointer (properties_inout, E_POINTER);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  struct _AMMediaType media_type;
  ACE_OS::memset (&media_type, 0, sizeof (struct _AMMediaType));
  //HRESULT result = CopyMediaType (&media_type, &(inherited::m_mt));
  HRESULT result = inherited::ConnectionMediaType (&media_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ConnectionMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT ((media_type.formattype == FORMAT_VideoInfo) &&
              (media_type.cbFormat == sizeof (struct tagVIDEOINFOHEADER)));
  struct tagVIDEOINFOHEADER* video_info_p =
    reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type.pbFormat);
  ACE_ASSERT (video_info_p);

  // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0 (why ?)
  if (properties_inout->cbAlign <= 0) properties_inout->cbAlign = 1;
  //properties_inout->cbBuffer = //DIBSIZE (video_info_p->bmiHeader) * 2;
    //GetBitmapSize (&video_info_p->bmiHeader);
  properties_inout->cbBuffer = video_info_p->bmiHeader.biSizeImage;
  ACE_ASSERT (properties_inout->cbBuffer);
  //properties_inout->cbPrefix = 0;
  properties_inout->cBuffers = MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS;

  FreeMediaType (media_type);

  // configure the allocator to reserve sample memory; remember to validate
  // the return value to confirm eligibility of buffer configuration
  // *NOTE*: this function does not actually allocate any memory (see
  //         IMemAllocator::Commit ())
  struct _AllocatorProperties actual_properties;
  ACE_OS::memset (&actual_properties, 0, sizeof (struct _AllocatorProperties));
  result = allocator_in->SetProperties (properties_inout,
                                        &actual_properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemAllocator::SetProperties(): \"%s\" (0x%x), aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                result));
    return result;
  } // end IF

  // --> is this allocator suitable ?
  // *TODO*: this needs more work
  if (actual_properties.cbBuffer < properties_inout->cbBuffer)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("IMemAllocator::SetProperties() returned %d (expected: %d), aborting\n"),
                actual_properties.cbBuffer, properties_inout->cbBuffer));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (actual_properties.cBuffers >= 1);

  //// (try to) allocate required memory
  //result = allocator_in->Commit ();
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMemAllocator::Commit(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
  //  return result;
  //} // end IF

  return S_OK;
} // DecideBufferSize

//
// FillBuffer
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::FillBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::FillBuffer"));

  // sanity check(s)
  CheckPointer (mediaSample_in, E_POINTER);
  ACE_ASSERT (queue_);

  HRESULT result = E_FAIL;
  BYTE* data_p = NULL;
  result = mediaSample_in->GetPointer (&data_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return S_FALSE; // --> stop 'streaming thread'
  } // end IF
  ACE_ASSERT (data_p);
  long data_length_l = 0;
  data_length_l = mediaSample_in->GetSize ();
  ACE_ASSERT (data_length_l);

  ACE_Message_Block* message_block_p = NULL;
  int result_2 = queue_->dequeue_head (message_block_p, NULL);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue_head(): \"%m\", aborting\n")));
    return S_FALSE; // --> stop 'streaming thread'
  } // end IF
  ACE_ASSERT (message_block_p);
  if (message_block_p->msg_type () == ACE_Message_Block::MB_STOP)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%t: stopping DirectShow streaming thread...\n")));

    // clean up
    message_block_p->release ();

    return S_FALSE; // --> stop 'streaming thread'
  } // end IF

  size_t data_length_2 = message_block_p->length ();
  ACE_ASSERT (static_cast<size_t> (data_length_l) >= data_length_2);
  // *NOTE*: ideally, the message buffers should derive from IMediaSample to
  //         avoid this copy; this is why asynchronous 'push' source filters may
  //         be more efficient in 'capture' pipelines
  // *TODO*: consider blocking in the GetBuffer() method (see above) and supplying
  //         pre-filled samples for graph configurations that support foreign
  //         allocators; then this method could be a NOP
  ACE_OS::memcpy (data_p,
                  message_block_p->rd_ptr (),
                  data_length_2);
  message_block_p->release ();
  result = mediaSample_in->SetActualDataLength (data_length_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::SetActualDataLength(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return S_FALSE; // --> stop 'streaming thread'
  } // end IF

  // the current time is the samples' start
  CRefTime ref_time = sampleTime_;
  // increment to find the finish time
  sampleTime_ += (LONG)frameInterval_;
  result = mediaSample_in->SetTime ((REFERENCE_TIME*)&ref_time,
                                    (REFERENCE_TIME*)&sampleTime_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::SetTime(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return S_FALSE; // --> stop 'streaming thread'
  } // end IF

  result = mediaSample_in->SetSyncPoint (TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::SetSyncPoint(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return S_FALSE; // --> stop 'streaming thread'
  } // end IF

  return S_OK;
} // FillBuffer

//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::OnThreadCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::OnThreadCreate"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%t: spawned DirectShow streaming thread...\n")));

  CAutoLock cAutoLockShared (&lock_);

  // we need to also reset the repeat time in case the system
  // clock is turned off after m_iRepeatTime gets very big
  //frameInterval_ = defaultFrameInterval_;

  return NOERROR;
} // OnThreadCreate

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::OnThreadDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::OnThreadDestroy"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%t: stopped DirectShow streaming thread...\n")));

  return NOERROR;
} // OnThreadDestroy

//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::Notify (IBaseFilter* filter_in,
                                                                     Quality quality_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::Notify"));

  // Adjust the repeat rate.
  if (quality_in.Proportion <= 0)
      frameInterval_ = 1000;        // We don't go slower than 1 per second
  else
  {
    frameInterval_ = frameInterval_ * 1000 / quality_in.Proportion;
    if (frameInterval_ > 1000)
      frameInterval_ = 1000;    // We don't go slower than 1 per second
    else if (frameInterval_ < 10)
      frameInterval_ = 10;      // We don't go faster than 100/sec
  } // end ELSE

  // skip forwards
  if (quality_in.Late > 0)
    sampleTime_ = sampleTime_ + quality_in.Late;

  return NOERROR;
} // Notify

// ---------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::Get (REFGUID guidPropSet_in,
                                                                  DWORD dwPropID_in,
                                                                  LPVOID pInstanceData_in,
                                                                  DWORD cbInstanceData_in,
                                                                  LPVOID pPropData_in,
                                                                  DWORD cbPropData_in,
                                                                  DWORD* pcbReturned_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::Get"));

  // sanity check(s)
  if (guidPropSet_in != AMPROPSETID_Pin)
    return E_PROP_SET_UNSUPPORTED;
  if (dwPropID_in != AMPROPERTY_PIN_CATEGORY)
    return E_PROP_ID_UNSUPPORTED;
  if (pPropData_in == NULL && pcbReturned_in == NULL)
    return E_POINTER;

  if (pcbReturned_in)
    *pcbReturned_in = sizeof (struct _GUID);
  if (pPropData_in == NULL)  // Caller just wants to know the size.
    return S_OK;

  // sanity check(s)
  if (cbPropData_in < sizeof (struct _GUID)) // The buffer is too small.
    return E_UNEXPECTED;

  *reinterpret_cast<struct _GUID*>(pPropData_in) = PIN_CATEGORY_CAPTURE;

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::QuerySupported (REFGUID guidPropSet_in,
                                                                             DWORD dwPropID_in,
                                                                             DWORD* pTypeSupport_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::QuerySupported"));

  // sanity check(s)
  if (guidPropSet_in != AMPROPSETID_Pin)
    return E_PROP_SET_UNSUPPORTED;
  if (dwPropID_in != AMPROPERTY_PIN_CATEGORY)
    return E_PROP_ID_UNSUPPORTED;

  if (pTypeSupport_in)
    // We support getting this property, but not setting it.
    *pTypeSupport_in = KSPROPERTY_SUPPORT_GET;

  return S_OK;
}

// ------------------------------------

//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//STDMETHODIMP
//Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
//                                                 FilterType,
//                                                 MediaType>::SuggestAllocatorProperties (const struct _AllocatorProperties* pprop_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::SuggestAllocatorProperties"));
//
//  // sanity check(s)
//  CheckPointer (pprop_in, E_POINTER);
//
//  allocatorProperties_ = *pprop_in;
//
//  return S_OK;
//}

// ------------------------------------

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::SetFormat (struct _AMMediaType* pmt_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::SetFormat"));

  // sanity check(s)
  ACE_ASSERT (mediaType_);
  if (!pmt_in) pmt_in = mediaType_;

  // check compatibility
  CMediaType media_type, media_type_2;
  HRESULT result = media_type.Set (*pmt_in);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_type_2.Set (*mediaType_);
  ACE_ASSERT (SUCCEEDED (result));
  if (!media_type.MatchesPartial (&media_type_2))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("incompatible media types (\"%s\"\n\"%s\")\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*pmt_in).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*mediaType_).c_str ())));
    return VFW_E_INVALIDMEDIATYPE;
  } // end IF

  if (pmt_in == mediaType_) goto continue_;
  if (mediaType_)
    Stream_Module_Device_DirectShow_Tools::deleteMediaType (mediaType_);
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*pmt_in,
                                                             mediaType_))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(): \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF

continue_:
  // *TODO*: verify connectivity and state
  //         see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319788(v=vs.85).aspx
  // VFW_E_NOT_CONNECTED
  // VFW_E_NOT_STOPPED
  // VFW_E_WRONG_STATE

  IBaseFilter* filter_p = Stream_Module_Device_DirectShow_Tools::pin2Filter (this);
  ACE_ASSERT (filter_p);
  if (pmt_in == mediaType_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: reset output format...\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: set output format: %s...\n"),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_Module_Device_DirectShow_Tools::mediaTypeToString (*mediaType_, true).c_str ())));
  filter_p->Release ();

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::GetFormat (struct _AMMediaType** ppmt_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::GetFormat"));

  // sanity check(s)
  CheckPointer (ppmt_inout, E_POINTER);
  IPin* pin_p = NULL;
  //HRESULT result = inherited::ConnectedTo (&pin_p);
  //if (FAILED (result)) return result;
  //pin_p->Release ();
  ACE_ASSERT (!*ppmt_inout);
  ACE_ASSERT (mediaType_);

  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*mediaType_,
                                                             *ppmt_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(): \"%m\", aborting\n")));
    return E_OUTOFMEMORY;
  } // end IF
  ACE_ASSERT (*ppmt_inout);

  return S_OK;
}
template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::GetNumberOfCapabilities (int* piCount_inout,
                                                                                      int* piSize_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::GetNumberOfCapabilities"));

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
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::GetStreamCaps (int iIndex_in,
                                                                            struct _AMMediaType** ppmt_inout,
                                                                            BYTE* pSCC_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::GetStreamCaps"));

  // sanity check(s)
  if (iIndex_in > 0) return S_FALSE; // E_INVALIDARG
  ACE_ASSERT (ppmt_inout);
  ACE_ASSERT (!*ppmt_inout);
  ACE_ASSERT (pSCC_in);
  ACE_ASSERT (mediaType_);

  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*mediaType_,
                                                             *ppmt_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(): \"%m\", aborting\n")));
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
  if (mediaType_->formattype == FORMAT_VideoInfo)
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
  else if (mediaType_->formattype == FORMAT_VideoInfo2)
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
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (mediaType_->formattype).c_str ())));
    return E_OUTOFMEMORY;
  } // end ELSE

  return S_OK;
}
