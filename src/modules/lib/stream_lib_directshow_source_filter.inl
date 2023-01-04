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

#include <utility>

#include "amvideo.h"
#include "vfwmsgs.h"

#include "refclock.h"

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_lib_common.h"
#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_guids.h"

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
CUnknown* WINAPI
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::CreateInstance (LPUNKNOWN IUnknown_in,
                                                                                        HRESULT* result_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::CreateInstance"));

  // sanity check(s)
  ACE_ASSERT (result_out);

  // initialize return value(s)
  *result_out = S_OK;

  CUnknown* unknown_p = NULL;
  ACE_NEW_NORETURN (unknown_p,
                    OWN_TYPE_T (NAME (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE),
                                IUnknown_in,
                                CLSID_ACEStream_MediaFramework_Source_Filter,
                                result_out));
  if (!unknown_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", aborting\n")));
    *result_out = E_OUTOFMEMORY;
  } // end IF

  return unknown_p;
} // CreateInstance

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
void WINAPI
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::DeleteInstance (void* pointer_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::DeleteInstance"));

  // sanity check(s)
  ACE_ASSERT (pointer_in);

  //CUnknown* unknown_p = reinterpret_cast<CUnknown*> (pointer_in);
  //OWN_TYPE_T* instance_p = dynamic_cast<OWN_TYPE_T*> (unknown_p);
  //ACE_ASSERT (instance_p);
  OWN_TYPE_T* instance_p = static_cast<OWN_TYPE_T*> (pointer_in);

  delete instance_p;
} // DeleteInstance

//template <typename MessageType,
//          typename ConfigurationType,
//          typename PinConfigurationType>
//void
//Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
//                                                 ConfigurationType,
//                                                 PinConfigurationType>::operator delete (void* pointer_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::operator delete"));
//
//  //// *NOTE*: when used as a regular C++ library (template) class, applications
//  ////         instantiate filter objects through the default ctor. In this case,
//  ////         class instances behave as standard COM objects, and must therefore
//  ////         implement the IUnknown reference-counting mechanism to avoid memory
//  ////         leakage.
//  //if (hasCOMReference_)
//  //{
//  //  ULONG reference_count = Release ();
//  //  return; // dtor has been invoked --> done
//  //} // end IF
//
//  // *NOTE*: when applications instantiate filter (COM) objects from DLLs, that
//  //         filter instance may be allocated in a separate heap (this depends
//  //         on the C runtime version (and, apparently, also type, i.e. static/
//  //         dynamic) that was compiled into(/with ? ...) the DLL) and needs to
//  //         be deallocated 'from' the same heap; i.e. the global 'delete'
//  //         operator may (see above) fail in this particular scenario (
//  //         _CrtIsValidHeapPointer() assertion), which is a known and long-
//  //         standing issue. *TODO*: does this affect _DEBUG builds only ?
//  //         --> overload the delete operator and forward the instance handle to
//  //             a static function 'inside' (see 'translation/compilation units'
//  //             and/or scope/namespace issues on how to address the 'global
//  //             delete' operator) the DLL
//  //         This implementation also handles the scenario where filter
//  //         instances are allocated from 'plugin' DLLs that can be loaded/
//  //         unloaded at runtime
//  OWN_TYPE_T::DeleteInstance (pointer_in);
//}

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename MediaType,
//          typename ModuleType>
//void
//Stream_MediaFramework_DirectShow_Source_Filter_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       MediaType,
//                                       ModuleType>::operator delete (void* pointer_in,
//                                                                     size_t bytes_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::operator delete"));
//
//  ACE_UNUSED_ARG (bytes_in);
//
//  // *NOTE*: see above
//  OWN_TYPE_T::DeleteInstance (pointer_in);
//}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::Stream_MediaFramework_DirectShow_Source_Filter_T ()
 : inherited (ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_DIRECTSHOW_FILTER_NAME_SOURCE), // name
              NULL,                                                            // owner
              CLSID_ACEStream_MediaFramework_Source_Filter,                    // CLSID
              NULL)                                                            // result
 , configuration_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::Stream_MediaFramework_DirectShow_Source_Filter_T"));

  // *IMPORTANT NOTE*: the reference count is 0 and is incremented by each
  //                   sucessful QueryInterface()

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  OUTPUT_PIN_T* pin_p = NULL;
  HRESULT result = E_FAIL;
  ACE_NEW_NORETURN (pin_p,
                    OUTPUT_PIN_T (&result,
                                  this,
                                  STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME_L));
  if (!pin_p || FAILED (result))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: added output pin \"%s\"\n"),
  //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
  //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ())));

  // *NOTE*: 'this' 'owns' the output pin
  // *IMPORTANT NOTE*: increments this' reference count as well; should be 1
  //                   after this call
  pin_p->AddRef ();
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::Stream_MediaFramework_DirectShow_Source_Filter_T (LPTSTR name_in,
                                                                                                                          LPUNKNOWN owner_in,
                                                                                                                          REFGUID CLSID_in,
                                                                                                                          HRESULT* result_out)
 : inherited (name_in,
              owner_in,
              CLSID_in,
              result_out)
 , configuration_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::Stream_MediaFramework_DirectShow_Source_Filter_T"));

  // sanity check(s)
  ACE_ASSERT (result_out);
  ACE_ASSERT (SUCCEEDED (*result_out));

  // *IMPORTANT NOTE*: the reference count is 0 and is incremented by each
  //                   sucessful QueryInterface()

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  OUTPUT_PIN_T* pin_p = NULL;
  HRESULT result = E_FAIL;
  ACE_NEW_NORETURN (pin_p,
                    OUTPUT_PIN_T (&result,
                                  this,
                                  STREAM_LIB_DIRECTSHOW_FILTER_PIN_OUTPUT_NAME_L));
  if (!pin_p || FAILED (result))
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    if (result_out)
      *result_out = (!pin_p ? E_OUTOFMEMORY : result);
    return;
  } // end IF
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("%s: added output pin \"%s\"\n"),
  //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
  //            ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ())));

  // *NOTE*: 'this' 'owns' the output pin
  // *IMPORTANT NOTE*: increments this' reference count as well; should be 1
  //                   after this call
  pin_p->AddRef ();

  if (result_out)
    *result_out = S_OK;
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::~Stream_MediaFramework_DirectShow_Source_Filter_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::~Stream_MediaFramework_DirectShow_Source_Filter_T"));

  struct _FilterInfo filter_info;
  ACE_OS::memset (&filter_info, 0, sizeof (struct _FilterInfo));
  HRESULT result = inherited::QueryFilterInfo (&filter_info);
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::QueryFilterInfo(): \"%s\", continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));

  // step1: disconnect from graph
  if (!Stream_MediaFramework_DirectShow_Tools::disconnect (this))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Tools::disconnect(), continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));

  // step2: remove from graph ?
  if (filter_info.pGraph)
  {
    result = filter_info.pGraph->RemoveFilter (this);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IFilterGraph::RemoveFilter(%s): \"%s\", continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT_WCHAR_TO_TCHAR (filter_info.achName),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    filter_info.pGraph->Release ();
  } // end IF

  // step3: remove output pin
  CBasePin* pin_p = inherited::GetPin (0);
  if (likely (pin_p))
    pin_p->Release (); // <-- should 'delete' the pin
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::NonDelegatingQueryInterface (REFIID riid_in,
                                                                                                     void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::NonDelegatingQueryInterface"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);

  if (InlineIsEqualGUID (riid_in, IID_IAMFilterMiscFlags))
    return GetInterface ((IAMFilterMiscFlags*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IMemAllocator))
    return GetInterface ((IMemAllocator*)this, interface_out);

  return inherited::NonDelegatingQueryInterface (riid_in, interface_out);
}

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename PinConfigurationType,
//          typename MediaType>
//CBasePin*
//Stream_MediaFramework_DirectShow_Source_Filter_T<TimePolicyType,
//                                                 SessionMessageType,
//                                                 ProtocolMessageType,
//                                                 ConfigurationType,
//                                                 PinConfigurationType,
//                                                 MediaType>::GetPin (int pin_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::GetPin"));
//
//  ACE_UNUSED_ARG (pin_in);
//
//  CBasePin* result_p = NULL;
//
//  IPin* ipin_p = Stream_MediaFramework_DirectShow_Tools::pin (this,
//                                                              PINDIR_OUTPUT);
//  if (!ipin_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s has no output pin, aborting\n"),
//                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
//    return NULL;
//  } // end IF
//  result_p = dynamic_cast<CBasePin*> (ipin_p);
//  ACE_ASSERT (result_p);
//
//  return result_p;
//}

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename PinConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_DirectShow_Source_Filter_T<TimePolicyType,
//                                                 SessionMessageType,
//                                                 ProtocolMessageType,
//                                                 ConfigurationType,
//                                                 PinConfigurationType,
//                                                 MediaType>::AddPin (CBasePin* pin_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::AddPin"));
//
//  CAutoLock cAutoLock (&lock_);
//
//  /*  Allocate space for this pin and the old ones */
//  CBasePin** pins_p = NULL;
//  ACE_NEW_NORETURN (pins_p,
//                    CBasePin*[1]);
//  if (!pins_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("%s: failed to allocate memory: \"%m\", returning\n"),
//                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
//    return E_OUTOFMEMORY;
//  } // end IF
//  if (pins_)
//  {
//    CopyMemory ((PVOID)pins_p, (PVOID)pins_,
//                1 * sizeof (pins_[0]));
//    pins_p[1] = pin_in;
//    delete [] pins_; pins_ = NULL;
//  } // end IF
//  pins_ = pins_p;
//  pins_[1] = pin_in;
//  numberOfPins_++;
//
//  return S_OK;
//}
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ConfigurationType,
//          typename PinConfigurationType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_DirectShow_Source_Filter_T<TimePolicyType,
//                                                 SessionMessageType,
//                                                 ProtocolMessageType,
//                                                 ConfigurationType,
//                                                 PinConfigurationType,
//                                                 MediaType>::RemovePin (CBasePin* pin_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::RemovePin"));
//
//  for (int i = 0; i < numberOfPins_; i++)
//  {
//    if (pins_[i] == pin_in)
//    {
//      if (numberOfPins_ == 1)
//      {
//        delete [] pins_; pins_ = NULL;
//      }
//      else
//      {
//        /*  no need to reallocate */
//        while (++i < numberOfPins_)
//          pins_[i - 1] = pins_[i];
//      } // end ELSE
//      numberOfPins_--;
//      return S_OK;
//    } // end IF
//  } // end FOR
//
//  return S_FALSE;
//}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::SetProperties (struct _AllocatorProperties* requestedProperties_in,
                                                                                       struct _AllocatorProperties* properties_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::SetProperties"));

  // sanity check(s)
  CheckPointer (requestedProperties_in, E_POINTER);
  CheckPointer (properties_out, E_POINTER);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorProperties);

  // step1: find 'least common denominator'
  properties_out->cBuffers =
    std::min (requestedProperties_in->cBuffers, configuration_->allocatorProperties->cBuffers);
  properties_out->cbBuffer =
    std::min (requestedProperties_in->cbBuffer, configuration_->allocatorProperties->cbBuffer);
  // *TODO*: cannot align buffers at this time
  ACE_ASSERT (requestedProperties_in->cbAlign <= 1);
  properties_out->cbPrefix =
    std::min (requestedProperties_in->cbPrefix, configuration_->allocatorProperties->cbPrefix);

  // step2: validate against set properties
  // *TODO*

  return NOERROR;
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::GetBuffer (IMediaSample** mediaSample_out,
                                                                                   REFERENCE_TIME* startTime_out,
                                                                                   REFERENCE_TIME* endTime_out,
                                                                                   DWORD flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::GetBuffer"));

  ACE_UNUSED_ARG (flags_in);

  // sanity check(s)
  CheckPointer (mediaSample_out, E_POINTER);
  ACE_ASSERT (!*mediaSample_out);
  //CheckPointer (startTime_out, E_POINTER);
  //CheckPointer (endTime_out, E_POINTER);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorProperties);
  ACE_ASSERT (configuration_->allocatorProperties->cBuffers && configuration_->allocatorProperties->cbBuffer);

  // initialize return value(s)
  *mediaSample_out = NULL;
  //*startTime_out = 0;
  //*endTime_out = 0;

  // step1: allocate message
  unsigned int message_size = (configuration_->allocatorProperties->cbPrefix +
                               configuration_->allocatorProperties->cbBuffer);
  MessageType* message_p = NULL;
  if (likely (configuration_->allocator))
  {
allocate:
    try {
      message_p =
        static_cast<MessageType*> (configuration_->allocator->malloc (message_size));
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  message_size));
      return E_OUTOFMEMORY;
    }

    // keep retrying ?
    if (!message_p && !configuration_->allocator->block ())
      goto allocate;
  } // end IF
  else
    ACE_NEW_NORETURN (message_p,
                      MessageType (0,              // session id
                                   message_size)); // size
  if (unlikely (!message_p))
  {
    if (configuration_->allocator &&
        configuration_->allocator->block ())
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate data message: \"%m\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    } // end IF
    else
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("%s: failed to allocate data message: \"%m\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    return E_OUTOFMEMORY;
  } // end IF
  *mediaSample_out = dynamic_cast<IMediaSample*> (message_p);
  if (unlikely (!*mediaSample_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to dynamic_cast<IMediaSample*>(%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                message_p));
    message_p->release ();
    return E_FAIL;
  } // end IF

  // step2: set times
  // *TODO*: calculate times from the average sample rate and the reference
  //         start clock

  return NOERROR;
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::ReleaseBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::ReleaseBuffer"));

  // sanity check(s)
  CheckPointer (mediaSample_in, E_POINTER);

  ACE_Message_Block* message_p =
    dynamic_cast<ACE_Message_Block*> (mediaSample_in);
  if (likely (message_p))
    message_p->release ();
  else
    mediaSample_in->Release ();

  return NOERROR;
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
bool
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (configuration_in.pinConfiguration);

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  IPin* ipin_p = Stream_MediaFramework_DirectShow_Tools::pin (this,
                                                              PINDIR_OUTPUT,
                                                              0);
  if (unlikely (!ipin_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_INITIALIZE_T* iinitialize_p = dynamic_cast<IPIN_INITIALIZE_T*> (ipin_p);
  if (unlikely (!iinitialize_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  if (unlikely (!iinitialize_p->initialize (*configuration_in.pinConfiguration)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_MEDIA_INITIALIZE_T* iinitialize_2 =
    dynamic_cast<IPIN_MEDIA_INITIALIZE_T*> (ipin_p);
  if (unlikely (!iinitialize_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*> (0x%@), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (ipin_p).c_str ()),
                ipin_p));
    goto error;
  } // end IF
  // *TODO*: remove type inference
  ACE_ASSERT (configuration_in.pinConfiguration->format);
  if (unlikely (!iinitialize_2->initialize (*configuration_in.pinConfiguration->format)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF
  ipin_p->Release (); ipin_p = NULL;

  return true;

error:
  if (ipin_p)
    ipin_p->Release ();

  return false;
}

template <typename MessageType,
          typename ConfigurationType,
          typename PinConfigurationType>
bool
Stream_MediaFramework_DirectShow_Source_Filter_T<MessageType,
                                                 ConfigurationType,
                                                 PinConfigurationType>::initialize (const struct _AMMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_T::initialize"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  // *NOTE*: the pin will inherited::AddPin() itself to 'this'
  IPin* ipin_p = Stream_MediaFramework_DirectShow_Tools::pin (this,
                                                              PINDIR_OUTPUT,
                                                              0);
  if (!ipin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s has no output pin, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    goto error;
  } // end IF

  IPIN_MEDIA_INITIALIZE_T* iinitialize_p =
    dynamic_cast<IPIN_MEDIA_INITIALIZE_T*> (ipin_p);
  if (!iinitialize_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to dynamic_cast<Common_IInitialize_T*>(%@), aborting\n"),
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

template <typename ConfigurationType>
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T (HRESULT* result_out,
                                                                                                                                           CSource* parentFilter_in,
                                                                                                                                           LPCWSTR pinName_in)
#if defined (UNICODE)
 : inherited (pinName_in,
#else
 : inherited (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (pinName_in)),
#endif // UNICODE
              result_out,      // OLE return code
              parentFilter_in, // owning filter
              pinName_in)      // pin name
 , configuration_ (NULL)
 , isInitialized_ (false)
 , mediaType_ (NULL)
 /////////////////////////////////////////
 , frameInterval_ (0)
 , numberOfMediaTypes_ (1)
 , directShowHasEnded_ (false)
 , sampleNumber_ (0)
 , sampleSize_ (0)
 , sampleTime_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T"));

  // *IMPORTANT NOTE*: the reference count is 0 and is incremented by each
  //                   sucessful QueryInterface()

  // sanity check(s)
  ACE_ASSERT (inherited::m_pFilter);
  ACE_ASSERT (result_out);
  ACE_ASSERT (SUCCEEDED (*result_out));
} // (Constructor)

template <typename ConfigurationType>
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::~Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::~Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T"));

  //HRESULT result = E_FAIL;
  //if (allocator_)
  //{
  //  result = allocator_->Decommit ();
  //  if (FAILED (result))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to IMemAllocator::Decommit(): \"%s\", continuing\n"),
  //                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  allocator_->Release ();
  //} // end IF

  if (mediaType_)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_);
} // (Destructor)

template <typename ConfigurationType>
bool
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::initialize"));

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  directShowHasEnded_ = false;

  isInitialized_ = true;

  return true;
}

template <typename ConfigurationType>
bool
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::initialize (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::initialize"));

  if (mediaType_)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_, false);
  mediaType_ =
    Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in);
  if (!mediaType_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (inherited::m_pFilter);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set default output format: %s\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_, true).c_str ())));

  return true;
}

// ------------------------------------

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::NonDelegatingQueryInterface (REFIID riid_in,
                                                                                                            void** interface_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::NonDelegatingQueryInterface"));

  // sanity check(s)
  CheckPointer (interface_out, E_POINTER);

  if (InlineIsEqualGUID (riid_in, IID_IKsPropertySet))
    return GetInterface ((IKsPropertySet*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAMBufferNegotiation))
    return GetInterface ((IAMBufferNegotiation*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAMStreamConfig))
    return GetInterface ((IAMStreamConfig*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAMPushSource))
    return GetInterface ((IAMPushSource*)this, interface_out);
  else if (InlineIsEqualGUID (riid_in, IID_IAMLatency))
    return GetInterface ((IAMLatency*)this, interface_out);

  return inherited::NonDelegatingQueryInterface (riid_in, interface_out);
}

// ------------------------------------

//
// CheckMediaType
//
// accept the preconfigured media type, if any
// Returns E_INVALIDARG if the mediatype is not acceptable
//
template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::CheckMediaType (const CMediaType *mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::CheckMediaType"));

  // sanity check(s)
  CheckPointer (mediaType_in, E_POINTER);
  ACE_ASSERT (mediaType_);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  CMediaType media_type;
  HRESULT result = media_type.Set (*mediaType_);
  ACE_ASSERT (SUCCEEDED (result));
  if (!media_type.MatchesPartial (mediaType_in))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: incompatible media types (\"%s\"\n\"%s\")\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_in).c_str ())));
    return S_FALSE;
  } // end IF

  return S_OK;
} // CheckMediaType

//
// GetMediaType
//
// Prefered types should be ordered by quality, zero as highest quality
// (iPosition > numberOfMediaTypes_ - 1 is invalid)
//
template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetMediaType (int position_in,
                                                                                             CMediaType* mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetMediaType"));

  HRESULT result = E_FAIL;

  if (position_in < 0)
    return E_INVALIDARG;
  if (static_cast<unsigned int> (position_in) > (numberOfMediaTypes_ - 1))
    return VFW_S_NO_MORE_ITEMS;
  // *TODO*: implement a default set of supported media types

  // sanity check(s)
  CheckPointer (mediaType_out, E_POINTER);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (mediaType_);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  result = mediaType_out->Set (*mediaType_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to CMediaType::Set(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return result;
  } // end IF

  // correct frame orientation ?
  if (!InlineIsEqualGUID (mediaType_out->majortype, MEDIATYPE_Video))
    goto continue_;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  if (InlineIsEqualGUID (mediaType_out->formattype, FORMAT_VideoInfo))
  { ACE_ASSERT (mediaType_out->cbFormat == sizeof (struct tagVIDEOINFOHEADER));
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (mediaType_out->pbFormat);
    ACE_ASSERT (video_info_header_p);
    if (configuration_->isTopToBottom)
    {
      if (video_info_header_p->bmiHeader.biHeight > 0)
        video_info_header_p->bmiHeader.biHeight =
          -video_info_header_p->bmiHeader.biHeight;
    }
    else if (video_info_header_p->bmiHeader.biHeight < 0)
      video_info_header_p->bmiHeader.biHeight =
        -video_info_header_p->bmiHeader.biHeight;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_out->formattype, FORMAT_VideoInfo2))
  { ACE_ASSERT (mediaType_out->cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
    video_info_header2_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (mediaType_out->pbFormat);
    ACE_ASSERT (video_info_header2_p);
    if (configuration_->isTopToBottom)
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
                ACE_TEXT ("%s/%s: invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Tools::GUIDToString (mediaType_out->formattype).c_str ())));
    return E_FAIL;
  } // end ELSE

continue_:
  return S_OK;
} // GetMediaType

//
// SetMediaType
//
// Called when a media type is agreed between filters
//
template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::SetMediaType (const CMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::SetMediaType"));

  // sanity check(s)
  CheckPointer (mediaType_in, E_POINTER);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  HRESULT result = inherited::SetMediaType (mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to CSourceStream::SetMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return result;
  } // end IF

  // compute frame interval/size to correctly set the sample time(s)
  REFERENCE_TIME avg_time_per_frame; // 100ns units
  if (InlineIsEqualGUID (inherited::m_mt.majortype, MEDIATYPE_Video))
  {
    struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
    struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
    if (InlineIsEqualGUID (inherited::m_mt.formattype, FORMAT_VideoInfo))
    { ACE_ASSERT (inherited::m_mt.cbFormat == sizeof (struct tagVIDEOINFOHEADER));
      video_info_header_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (inherited::m_mt.pbFormat);
      avg_time_per_frame = video_info_header_p->AvgTimePerFrame;
      sampleSize_ = video_info_header_p->bmiHeader.biSizeImage;
    } // end IF
    else if (InlineIsEqualGUID (inherited::m_mt.formattype, FORMAT_VideoInfo2))
    { ACE_ASSERT (inherited::m_mt.cbFormat == sizeof (struct tagVIDEOINFOHEADER2));
      video_info_header2_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER2*> (inherited::m_mt.pbFormat);
      avg_time_per_frame = video_info_header2_p->AvgTimePerFrame;
      sampleSize_ = video_info_header2_p->bmiHeader.biSizeImage;
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: invalid/unknown media type format type (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Common_Tools::GUIDToString (inherited::m_mt.formattype).c_str ())));
      return E_FAIL;
    } // end ELSE
    frameInterval_ = avg_time_per_frame;
  } // end IF
  else if (InlineIsEqualGUID (inherited::m_mt.majortype, MEDIATYPE_Audio))
  {
    if (inherited::m_mt.cbFormat)
    { ACE_ASSERT (inherited::m_mt.cbFormat == sizeof (struct tWAVEFORMATEX));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (inherited::m_mt.pbFormat);
      frameInterval_ =
        (REFERENCE_TIME)(MILLISECONDS_TO_100NS_UNITS(1000) / (float)waveformatex_p->nSamplesPerSec); // Ts = 1/fs
      sampleSize_ =
        (waveformatex_p->wBitsPerSample / 8) * waveformatex_p->nChannels;
    } // end ELSE
    else
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s/%s: passed media type has no format identifier, cannot set frame interval, continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: invalid/unknown media type major type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Tools::GUIDToString (inherited::m_mt.majortype).c_str ())));
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
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_in, true).c_str ())));

  return S_OK;
} // SetMediaType

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::QueryAccept (const struct _AMMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::QueryAccept"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _AMMediaType media_type_s;
  ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  HRESULT result = inherited::ConnectionMediaType (&media_type_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to ConnectionMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return E_FAIL;
  } // end IF
  if (Stream_MediaFramework_DirectShow_Tools::match (media_type_s,
                                                     *mediaType_in))
    return S_OK;

  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("%s/%s: downstream requests a format change (was: %s), rejecting\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_in, true).c_str ())));

  // *NOTE*: "...The upstream filter can reject the format change by returning
  //         S_FALSE from QueryAccept. In that case, the Video Renderer
  //         continues to use GDI with the original format. ..."
  return S_FALSE;
}

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::DecideAllocator (IMemInputPin* inputPin_in,
                                                                                                IMemAllocator** allocator_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::DecideAllocator"));

  // sanity check(s)
  CheckPointer (inputPin_in, E_POINTER);
  CheckPointer (allocator_out, E_POINTER);
  ACE_ASSERT (!*allocator_out);
  ACE_ASSERT (!inherited::m_pAllocator);
  ACE_ASSERT (inherited::m_pFilter);
  ACE_ASSERT (configuration_);

  HRESULT result = E_FAIL;
  IPin* pin_p = NULL;
  IBaseFilter* filter_p = NULL;
  struct _GUID class_id = GUID_NULL;

  // debug info
  result = inputPin_in->QueryInterface (IID_PPV_ARGS (&pin_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMemInputPin::QueryInterface(IID_IPin) (handle was: 0x%@): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                inputPin_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (pin_p);
  filter_p = Stream_MediaFramework_DirectShow_Tools::toFilter (pin_p);
  ACE_ASSERT (filter_p);

  struct _AllocatorProperties allocator_requirements;
  ACE_OS::memset (&allocator_requirements,
                  0,
                  sizeof (struct _AllocatorProperties));
  result = inputPin_in->GetAllocatorRequirements (&allocator_requirements);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL) // E_NOTIMPL: 0x80004001
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to IMemInputPin::GetAllocatorRequirements() (was: %s/%s): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: failed to IMemInputPin::GetAllocatorRequirements() (was: %s/%s): \"%s\", continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  } // end IF
  else
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: allocator requirements (buffers/size/alignment/prefix) of %s/%s: %d/%d/%d/%d\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                allocator_requirements.cBuffers,
                allocator_requirements.cbBuffer,
                allocator_requirements.cbAlign,
                allocator_requirements.cbPrefix));
  } // end ELSE

  // *NOTE*: see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319039(v=vs.85).aspx

  //// *TODO*: the Color Space Converter filters' allocator appears to be broken,
  ////         IMemAllocator::SetProperties() does not work and the filter hangs
  ////         in CBaseOutputPin::Deliver()
  ////         --> use our own
  //result = filter_p->GetClassID (&class_id);
  //ACE_ASSERT (SUCCEEDED (result));
  //if (InlineIsEqualGUID (class_id, CLSID_Colour))
  //{
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("%s/%s: using own allocator...\n"),
  //              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
  //              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
  //  goto continue_;
  //} // end IF

  // use input pins' allocator ?
  result = inputPin_in->GetAllocator (allocator_out);
  if (SUCCEEDED (result))
  { ACE_ASSERT (*allocator_out);
    //if (inherited::m_pAllocator)
    //  inherited::m_pAllocator->Release ();
    //inherited::m_pAllocator = *allocator_out;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: configuring allocator from \"%s\"/%s\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ())));
    goto decide;
  } // end IF

//continue_:
  // the input pin does not supply an allocator
  // --> use our own
  ACE_ASSERT (!*allocator_out);

  // *NOTE*: how this really makes sense for asynchronous filters only
  if (configuration_->hasMediaSampleBuffers)
    *allocator_out = dynamic_cast<IMemAllocator*> (inherited::m_pFilter);
  else
  {
    result = inherited::InitAllocator (allocator_out);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to CBaseOutputPin::InitAllocator(): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (*allocator_out);

decide:
  result = DecideBufferSize (*allocator_out,
                             &allocator_requirements);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to CBaseOutputPin::DecideBufferSize(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF

//notify:
  //ACE_ASSERT (inherited::m_pAllocator == *allocator_out);
  ACE_ASSERT (*allocator_out);

  result = inputPin_in->NotifyAllocator (*allocator_out,
                                         FALSE); // read-only buffers ?
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMemInputPin::NotifyAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (pin_p).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              allocator_requirements.cBuffers,
              allocator_requirements.cbBuffer,
              allocator_requirements.cbAlign,
              allocator_requirements.cbPrefix));

  pin_p->Release (); pin_p = NULL;
  filter_p->Release (); filter_p = NULL;

  return S_OK;

error:
  //if (inherited::m_pAllocator)
  //{
  //  inherited::m_pAllocator->Release (); inherited::m_pAllocator = NULL;
  //} // end IF
  if (pin_p)
    pin_p->Release ();
  if (filter_p)
    filter_p->Release ();

  return result;
}

//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image/sample was agreed.
// Then ask for buffers of the correct size to contain them.
//
template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::DecideBufferSize (IMemAllocator* allocator_in,
                                                                                                 struct _AllocatorProperties* properties_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::DecideBufferSize"));

  // sanity check(s)
  CheckPointer (allocator_in, E_POINTER);
  CheckPointer (properties_inout, E_POINTER);
  ACE_ASSERT (inherited::m_pFilter);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorProperties);

  struct _AMMediaType media_type;
  ACE_OS::memset (&media_type, 0, sizeof (struct _AMMediaType));
  HRESULT result = inherited::ConnectionMediaType (&media_type);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to ConnectionMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return result;
  } // end IF

  long number_of_buffers_i = 0;
  if (InlineIsEqualGUID (media_type.formattype, FORMAT_VideoInfo) &&
      (media_type.cbFormat == sizeof (struct tagVIDEOINFOHEADER)))
  { struct tagVIDEOINFOHEADER* video_info_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type.pbFormat);
    ACE_ASSERT (video_info_p);

    number_of_buffers_i = STREAM_LIB_DIRECTSHOW_VIDEO_DEFAULT_SOURCE_BUFFERS;
    properties_inout->cbBuffer =
      std::max (static_cast<long> (video_info_p->bmiHeader.biSizeImage),
                configuration_->allocatorProperties->cbBuffer);
  } // end IF
  else if (InlineIsEqualGUID (media_type.formattype, FORMAT_VideoInfo2) &&
           (media_type.cbFormat == sizeof (struct tagVIDEOINFOHEADER2)))
  { struct tagVIDEOINFOHEADER2* video_info_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type.pbFormat);
    ACE_ASSERT (video_info_p);

    number_of_buffers_i = STREAM_LIB_DIRECTSHOW_VIDEO_DEFAULT_SOURCE_BUFFERS;
    properties_inout->cbBuffer =
      std::max (static_cast<long> (video_info_p->bmiHeader.biSizeImage),
                configuration_->allocatorProperties->cbBuffer);
  } // end ELSE IF
  else if (InlineIsEqualGUID (media_type.formattype, FORMAT_WaveFormatEx) &&
           (media_type.cbFormat == sizeof (struct tWAVEFORMATEX)))
  { struct tWAVEFORMATEX* audio_info_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type.pbFormat);
    ACE_ASSERT (audio_info_p);

    number_of_buffers_i = STREAM_LIB_DIRECTSHOW_AUDIO_DEFAULT_SOURCE_BUFFERS;
    properties_inout->cbBuffer =
      std::max (static_cast<long> ((audio_info_p->nAvgBytesPerSec * STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_MAX_LATENCY_MS) / (float)MILLISECONDS),
                configuration_->allocatorProperties->cbBuffer);
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: invalid/unknown media type format (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (media_type).c_str ())));
    FreeMediaType (media_type);
    return VFW_E_INVALIDMEDIATYPE;
  } // end ELSE
  properties_inout->cBuffers =
    std::max (number_of_buffers_i,
              configuration_->allocatorProperties->cBuffers);
  ACE_ASSERT (properties_inout->cBuffers);
  ACE_ASSERT (properties_inout->cbBuffer);

  properties_inout->cbAlign = configuration_->allocatorProperties->cbAlign;
  // *NOTE*: IMemAllocator::SetProperties() returns VFW_E_BADALIGN (0x8004020e)
  //         if this is -1/0
  if (properties_inout->cbAlign <= 0)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s/%s: adjusting _AllocatorProperties::cbAlign (was %d), continuing\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                properties_inout->cbAlign));
    properties_inout->cbAlign = 1;
  } // end IF
  properties_inout->cbPrefix = configuration_->allocatorProperties->cbPrefix;

  FreeMediaType (media_type);

  // configure the allocator to reserve sample memory; remember to validate
  // the return value to confirm eligibility of buffer configuration
  // *NOTE*: this function does not actually allocate any memory (see
  //         IMemAllocator::Commit ())
  // *NOTE*: this may invoke QueryAccept() (see above)
  struct _AllocatorProperties allocator_properties_s;
  ACE_OS::memset (&allocator_properties_s, 0, sizeof (struct _AllocatorProperties));
  result = allocator_in->SetProperties (properties_inout,
                                        &allocator_properties_s);
  if (FAILED (result)) // VFW_E_TYPE_NOT_ACCEPTED: 0x8004022a
                       // E_INVALIDARG           : 0x80070057
                       
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMemAllocator::SetProperties(): \"%s\" (0x%x), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()),
                result));
    return result;
  } // end IF

  // --> is this allocator suitable ?
  // *TODO*: this needs more work
  ACE_ASSERT (allocator_properties_s.cBuffers >= 1);
  if (allocator_properties_s.cbBuffer < properties_inout->cbBuffer)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: IMemAllocator::SetProperties() returned buffer size %d (expected: %d), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                allocator_properties_s.cbBuffer, properties_inout->cbBuffer));
    return E_FAIL;
  } // end IF

  //// (try to) allocate required memory
  //result = allocator_in->Commit ();
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMemAllocator::Commit(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
  //  return result;
  //} // end IF

  return S_OK;
} // DecideBufferSize

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::FillBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::FillBuffer"));

  // sanity check(s)
  CheckPointer (mediaSample_in, E_POINTER);
  ACE_ASSERT (inherited::m_pFilter);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->queue);

  static bool is_first = true;

  // done ?
  HRESULT result = E_FAIL;
  enum Command command_e = CMD_INIT;
  if (likely (!directShowHasEnded_))
  {
    if (likely (inherited::CheckRequest (&command_e)))
    {
      if (unlikely (command_e == CMD_STOP))
        directShowHasEnded_ = true; // --> wait for stream
      Reply ((command_e == CMD_STOP) ? S_FALSE : S_OK);
    } // end IF
  } // end IF

  BYTE* data_p = NULL;
  result = mediaSample_in->GetPointer (&data_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return S_FALSE; // --> stop
  } // end IF
  ACE_ASSERT (data_p);
  long available_buffer_size_i = mediaSample_in->GetSize ();
  long total_buffer_size_i = available_buffer_size_i;
  int result_2 = -1;
  size_t bytes_to_write_i = 0;
  size_t offset_i = 0;
  ACE_Message_Block* message_block_p = NULL;

  if (configuration_->buffer)
    goto continue_;

deqeue_next_buffer:
  ACE_ASSERT (!configuration_->buffer);
  result_2 = configuration_->queue->dequeue_head (configuration_->buffer, NULL);
  if (unlikely (result_2 == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to ACE_Message_Queue_Base::dequeue_head(): \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    return S_FALSE; // --> stop
  } // end IF
continue_:
  ACE_ASSERT (configuration_->buffer);
  if (unlikely (configuration_->buffer->msg_type () == ACE_Message_Block::MB_STOP))
  {
    configuration_->buffer->release (); configuration_->buffer = NULL;

    // stream has ended --> wait for DirectShow ?
    if (!directShowHasEnded_)
    {
      while (command_e != CMD_STOP)
      {
        command_e = (enum Command)inherited::GetRequest ();
        inherited::Reply ((command_e == CMD_STOP) ? S_FALSE : S_OK);
      } // end WHILE
      directShowHasEnded_ = true;
    } // end IF

    return S_FALSE; // --> stop
  } // end IF

  bytes_to_write_i = std::min (configuration_->buffer->length (),
                               static_cast<size_t> (available_buffer_size_i));
  // *NOTE*: ideally, the message buffers should derive from IMediaSample to
  //         avoid this copy; this is how asynchronous 'push' source filters can
  //         be more efficient in 'capture' pipelines
  // *TODO*: for synchronous 'pull' filters, consider blocking in the
  //         GetBuffer() method (see above) and supplying the samples as soon as
  //         they arrive. Note that this should work as long as the downstream
  //         filter accepts 'external' allocators (apparently, some default
  //         filters, e.g. the Microsoft Color Space Converter, do not support
  //         this (IMemAllocator::SetProperties() fails with E_INVALIDARG, and
  //         the allocator handle passed into IMemInputPin::NotifyAllocator() is
  //         ignored; this requires investigation). Until further notice,
  //         memcpy() seems to be unavoidable to forward the data)
  ACE_OS::memcpy (data_p + offset_i, configuration_->buffer->rd_ptr (), bytes_to_write_i);
  offset_i += bytes_to_write_i;
  available_buffer_size_i -= static_cast<long> (bytes_to_write_i);
  configuration_->buffer->rd_ptr (bytes_to_write_i);
  if (!configuration_->buffer->length ())
  {
    if (configuration_->buffer->cont ())
    {
      message_block_p = configuration_->buffer->cont ();
      configuration_->buffer->cont (NULL);
      configuration_->buffer->release (); configuration_->buffer = NULL;
      configuration_->buffer = message_block_p;
      goto continue_;
    } // end IF
    configuration_->buffer->release (); configuration_->buffer = NULL;
  } // end IF
  if (available_buffer_size_i)
    goto deqeue_next_buffer;
  
  result = mediaSample_in->SetActualDataLength (total_buffer_size_i);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMediaSample::SetActualDataLength(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return S_FALSE; // --> stop
  } // end IF

  REFERENCE_TIME start_time = sampleTime_;
  ACE_ASSERT (sampleSize_);
  ACE_ASSERT ((total_buffer_size_i % sampleSize_) == 0);
  long samples_written_i = (total_buffer_size_i / sampleSize_);
  sampleTime_ += (frameInterval_ * samples_written_i);
  // *NOTE*: this sets the samples' "stream" time (== "presentation" time)
  result =
    mediaSample_in->SetTime ((configuration_->setSampleTimes ? &start_time : NULL),
                             (configuration_->setSampleTimes ? &sampleTime_ : NULL));
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMediaSample::SetTime(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return S_FALSE; // --> stop
  } // end IF

  if (!configuration_->setSampleTimes)
    goto continue_2;
  // *NOTE*: this sets the samples' "media" time (== frame/sample number)
  start_time = sampleNumber_;
  sampleNumber_ += samples_written_i;
  result = mediaSample_in->SetMediaTime (&start_time,
                                         &sampleNumber_);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMediaSample::SetMediaTime(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return S_FALSE; // --> stop
  } // end IF

continue_2:
  // *NOTE*: "...The filter that first generates the data in the sample should
  //         set this flag to TRUE or FALSE, as appropriate. For uncompressed
  //         video and PCM audio, set every sample to TRUE. For compressed
  //         video, set key frames to TRUE and delta frames to FALSE..."
  // *TODO*: support delta frames
  result = mediaSample_in->SetSyncPoint (TRUE);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMediaSample::SetSyncPoint(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return S_FALSE; // --> stop
  } // end IF

  result = mediaSample_in->SetPreroll (FALSE);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to IMediaSample::SetPreroll(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return S_FALSE; // --> stop
  } // end IF

  if (is_first)
  {
    is_first = false;
    result = mediaSample_in->SetDiscontinuity (TRUE);
    if (unlikely (FAILED (result)))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: failed to IMediaSample::SetDiscontinuity(): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      return S_FALSE; // --> stop
    } // end IF
  } // end IF

  return S_OK; // --> continue
} // FillBuffer

//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::OnThreadCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::OnThreadCreate"));

  // sanity check(s)
  ACE_ASSERT (inherited::m_pFilter);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: %t: spawned DirectShow processing thread\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));

  // *WARNING*: inherited::m_pLock is held by CBaseFilter::Pause() (see: amfilter.cpp:557)
  //{ CAutoLock cAutoLockShared (inherited::m_pLock);
    //reset the repeat time in case the system
    //clock is turned off after m_iRepeatTime gets very big
  //} // end lock scope

  return NOERROR;
} // OnThreadCreate

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::OnThreadDestroy ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::OnThreadDestroy"));

  // sanity check(s)
  ACE_ASSERT (inherited::m_pFilter);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: %t: stopped DirectShow processing thread\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));

  return NOERROR;
} // OnThreadDestroy

template <typename ConfigurationType>
HRESULT
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::OnThreadStartPlay ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::OnThreadStartPlay"));

  // sanity check(s)
  ACE_ASSERT (inherited::m_pFilter);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: %t: started DirectShow processing thread\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));

  return NOERROR;
} // OnThreadStartPlay

//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//DWORD
//Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
//                                                           FilterType,
//                                                           MediaType>::ThreadProc ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::ThreadProc"));
//
//  enum CommandType command_e;
//  do
//  {
//    command_e = (enum CommandType)GetRequest ();
//    if (unlikely (command_e != CMD_INIT))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: invalid state change (expected %d, was: %d), aborting\n"),
//                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                  CMD_INIT, command_e));
//      Reply ((DWORD)E_UNEXPECTED);
//    } // end IF
//  } while (command_e != CMD_INIT);
//#if defined (_DEBUG)
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: initialized DirectShow processing thread (id: %t)\n"),
//              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ())));
//#endif // _DEBUG
//
//  HRESULT result = OnThreadCreate ();
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to OnThreadCreate(): \"%s\", aborting\n"),
//                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    HRESULT result_2 = OnThreadDestroy ();
//    ACE_UNUSED_ARG (result_2);
//    Reply (result); // send failed return code from OnThreadCreate
//    return -1;
//  } // end IF
//
//  // Initialisation suceeded
//  Reply (NOERROR);
//
//  do
//  {
//    command_e = (enum CommandType)GetRequest ();
//    switch (command_e)
//    {
//      case CMD_EXIT:
//        Reply (NOERROR);
//        break;
//      case CMD_RUN:
//        ACE_DEBUG ((LM_WARNING,
//                    ACE_TEXT ("%s: CMD_RUN received before CMD_PAUSE, continuing\n"),
//                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ())));
//        // !!! fall through ???
//      case CMD_PAUSE:
//        Reply (NOERROR);
//        result = DoBufferProcessingLoop ();
//        break;
//      case CMD_STOP:
//        Reply (NOERROR);
//        break;
//      default:
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: invalid/unknown command (was: %d), continuing\n"),
//                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                    command_e));
//        Reply ((DWORD)E_NOTIMPL);
//        break;
//    } // end SWITCH
//  } while (command_e != CMD_EXIT);
//
//  result = OnThreadDestroy ();
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to OnThreadDestroy(): \"%s\", aborting\n"),
//                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return -1;
//  } // end IF
//
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("%s: finalized DirectShow processing thread (id: %t)\n"),
//              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//  return 0;
//}
//
//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
//                                                           FilterType,
//                                                           MediaType>::DoBufferProcessingLoop ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::DoBufferProcessingLoop"));
//
//  HRESULT result = OnThreadStartPlay ();
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to OnThreadStartPlay(): \"%s\", aborting\n"),
//                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return result;
//  } // end IF
//
//  enum CommandType command_e;
//  IMediaSample* media_sample_p = NULL;
//  do
//  {
//    while (!CheckRequest ((DWORD*)&command_e))
//    { ACE_ASSERT (!media_sample_p);
//      result = GetDeliveryBuffer (&media_sample_p,
//                                  NULL,
//                                  NULL,
//                                  0);
//      if (FAILED (result))
//      {
//        Sleep (1);
//        continue; // go round again. Perhaps the error will go away
//                  // or the allocator is decommited & we will be asked to
//                  // exit soon
//      } // end IF
//      ACE_ASSERT (media_sample_p);
//
//      result = FillBuffer (media_sample_p);
//      if (result == S_OK)
//      {
//        result = Deliver (media_sample_p);
//        // downstream filter returns S_FALSE if it wants us to
//        // stop or an error if it's reporting an error.
//        if (FAILED (result))
//        {
//          ACE_DEBUG ((LM_ERROR,
//                      ACE_TEXT ("%s: failed to Deliver(): \"%s\", aborting\n"),
//                      ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//          media_sample_p->Release(); media_sample_p = NULL;
//          return result;
//        } // end IF
//      } // end IF
//      else if (result == S_FALSE)
//      {
//        // derived class wants us to stop pushing data
//        media_sample_p->Release(); media_sample_p = NULL;
//        HRESULT result_2 = DeliverEndOfStream ();
//        ACE_UNUSED_ARG (result_2);
//        return S_OK;
//      } // end ELSE IF
//      else
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("%s: failed to FillBuffer(): \"%s\", aborting\n"),
//                    ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//        media_sample_p->Release(); media_sample_p = NULL;
//        HRESULT result_2 = DeliverEndOfStream ();
//        ACE_UNUSED_ARG (result_2);
//        inherited::m_pFilter->NotifyEvent (EC_ERRORABORT, result, 0);
//        return result;
//      } // end ELSE
//      media_sample_p->Release(); media_sample_p = NULL;
//    } // end WHILE
//
//    // reply to all received commands
//    if ((command_e == CMD_RUN) ||
//        (command_e == CMD_PAUSE))
//      Reply (NOERROR);
//    else if (command_e != CMD_STOP)
//    {
//      ACE_DEBUG ((LM_WARNING,
//                  ACE_TEXT ("%s: invalid/unknown command (was: %d): \"%s\", continuing\n"),
//                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
//                  command_e));
//      Reply ((DWORD)E_UNEXPECTED);
//    } // end ELSE IF
//  } while (command_e != CMD_STOP);
//
//  return S_FALSE;
//}
//
//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
//                                                           FilterType,
//                                                           MediaType>::Active (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::Active"));
//
//  // sanity check(s)
//  if (parentFilter_->IsActive ())
//    return S_FALSE;
//
//  ACE_ASSERT (parentFilter_);
//  CAutoLock lock (&(parentFilter_->lock_));
//
//  // do nothing if not connected - its ok not to connect to
//  // all pins of a source filter
//  if (!IsConnected ())
//    return NOERROR;
//
//  // commit the allocator
//  HRESULT result = inherited::Active ();
//  if (FAILED (result))
//    return result;
//
//  ACE_ASSERT (!ThreadExists ());
//
//  if (!Create ())
//    return E_FAIL;
//
//  // Tell thread to initialize. If OnThreadCreate Fails, so does this.
//  result = Init ();
//  if (FAILED (result))
//    return result;
//
//  return Pause ();
//}
//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//HRESULT
//Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
//                                                           FilterType,
//                                                           MediaType>::Inactive (void)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::Inactive"));
//
//  // sanity check(s)
//  ACE_ASSERT (parentFilter_);
//  CAutoLock lock (&(parentFilter_->lock_));
//
//  // do nothing if not connected - its ok not to connect to
//  // all pins of a source filter
//  if (!IsConnected ())
//    return NOERROR;
//
//  // *NOTE*: decommit the allocator first to avoid a potential deadlock
//  HRESULT result = CBaseOutputPin::Inactive ();
//  if (FAILED (result))
//    return result;
//
//  if (ThreadExists ())
//  {
//    result = Stop ();
//    if (FAILED (result))
//      return result;
//    result = Exit ();
//    if (FAILED (result))
//      return result;
//  } // end IF
//
//  Close (); // wait for thread
//
//  return NOERROR;
//}

//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::Notify (IBaseFilter* filter_in,
                                                                                       Quality quality_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::Notify"));

  // Adjust the repeat rate.
  if (quality_in.Proportion <= 0)
      //frameInterval_ = 1000;        // We don't go slower than 1 per second
    /*frameInterval_ = defaultFrameInterval_*/;
  else
  {
    frameInterval_ *=
      static_cast<REFERENCE_TIME> (1000.0F / (float)quality_in.Proportion);
    //if (frameInterval_ > 1000)
    //  frameInterval_ = 1000;    // We don't go slower than 1 per second
    //else if (frameInterval_ < 10)
    //  frameInterval_ = 10;      // We don't go faster than 100/sec
  } // end ELSE

  // skip forwards
  // *TODO*: verify that tagQuality.Late has 'unit' (i.e. *100ns) format
  if (quality_in.Late > 0)
    sampleTime_ = sampleTime_ + quality_in.Late;

  return NOERROR;
} // Notify

// ---------------------------------------

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::Get (REFGUID guidPropSet_in,
                                                                                    DWORD dwPropID_in,
                                                                                    LPVOID pInstanceData_in,
                                                                                    DWORD cbInstanceData_in,
                                                                                    LPVOID pPropData_in,
                                                                                    DWORD cbPropData_in,
                                                                                    DWORD* pcbReturned_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::Get"));

  // sanity check(s)
  if (!InlineIsEqualGUID (guidPropSet_in, AMPROPSETID_Pin))
    return E_PROP_SET_UNSUPPORTED;
  if (dwPropID_in != AMPROPERTY_PIN_CATEGORY)
    return E_PROP_ID_UNSUPPORTED;
  if (pPropData_in == NULL || pcbReturned_in == NULL)
    return E_POINTER;

  if (pcbReturned_in)
    *pcbReturned_in = sizeof (struct _GUID);
  if (pPropData_in == NULL)  // Caller just wants to know the size.
    return S_OK;

  // sanity check(s)
  if (cbPropData_in < sizeof (struct _GUID)) // The buffer is too small.
    return E_UNEXPECTED;

  *reinterpret_cast<struct _GUID*> (pPropData_in) = PIN_CATEGORY_CAPTURE;

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::QuerySupported (REFGUID guidPropSet_in,
                                                                                               DWORD dwPropID_in,
                                                                                               DWORD* pTypeSupport_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::QuerySupported"));

  // sanity check(s)
  if (!InlineIsEqualGUID (guidPropSet_in, AMPROPSETID_Pin))
    return E_PROP_SET_UNSUPPORTED;
  if (dwPropID_in != AMPROPERTY_PIN_CATEGORY)
    return E_PROP_ID_UNSUPPORTED;

  if (pTypeSupport_in)
    // We support getting this property, but not setting it.
    *pTypeSupport_in = KSPROPERTY_SUPPORT_GET;

  return S_OK;
}

//template <typename ConfigurationType,
//          typename FilterType,
//          typename MediaType>
//STDMETHODIMP
//Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
//                                                           FilterType,
//                                                           MediaType>::Get (REFGUID rguidPropSet_in,
//                                                                            ULONG ulId_in,
//                                                                            LPVOID pInstanceData_in,
//                                                                            ULONG ulInstanceLength_in,
//                                                                            LPVOID pPropertyData_in,
//                                                                            ULONG ulDataLength_in,
//                                                                            PULONG pulBytesReturned_in)
//{
//  return Get (rguidPropSet_in,
//              static_cast<DWORD> (ulId_in),
//              pInstanceData_in,
//              static_cast<DWORD> (ulInstanceLength_in),
//              pPropertyData_in,
//              static_cast<DWORD> (ulDataLength_in),
//              static_cast<DWORD*> (pulBytesReturned_in));
//}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::QuerySupport (REFGUID rguidPropSet_in,
                                                                                             ULONG ulId_in,
                                                                                             PULONG pulTypeSupport_in)
{
  return QuerySupported (rguidPropSet_in,
                         static_cast<DWORD> (ulId_in),
                         static_cast<DWORD*> (pulTypeSupport_in));
}

// ------------------------------------

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::SuggestAllocatorProperties (const struct _AllocatorProperties* properties_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::SuggestAllocatorProperties"));

  // sanity check(s)
  CheckPointer (properties_in, E_POINTER);
  ACE_ASSERT (configuration_);

  *configuration_->allocatorProperties = *properties_in;

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetAllocatorProperties (struct _AllocatorProperties* properties_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetAllocatorProperties"));

  // sanity check(s)
  CheckPointer (properties_out, E_POINTER);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (configuration_->allocatorProperties);

  *properties_out = *configuration_->allocatorProperties;

  return NOERROR;
}

// ------------------------------------

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::SetFormat (struct _AMMediaType* pmt_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::SetFormat"));

  // sanity check(s)
  CheckPointer (pmt_in, E_POINTER);
  ACE_ASSERT (mediaType_);

  // check compatibility
  CMediaType media_type, media_type_2;
  HRESULT result = media_type.Set (*pmt_in);
  ACE_ASSERT (SUCCEEDED (result));
  result = media_type_2.Set (*mediaType_);
  ACE_ASSERT (SUCCEEDED (result));
  if (!media_type.MatchesPartial (&media_type_2))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: incompatible media types (\"%s\"\n\"%s\")\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*pmt_in).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_).c_str ())));
    return VFW_E_INVALIDMEDIATYPE;
  } // end IF

  if (pmt_in == mediaType_)
    goto continue_;
  if (mediaType_)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_, false);
  mediaType_ =
    Stream_MediaFramework_DirectShow_Tools::copy (*pmt_in);
  if (!mediaType_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    return E_OUTOFMEMORY;
  } // end IF

continue_:
  // *TODO*: verify connectivity and state
  //         see also: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319788(v=vs.85).aspx
  // VFW_E_NOT_CONNECTED
  // VFW_E_NOT_STOPPED
  // VFW_E_WRONG_STATE

  if (pmt_in == mediaType_)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: reset preferred format\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s/%s: set preferred format: %s\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (*mediaType_, true).c_str ())));

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetFormat (struct _AMMediaType** ppmt_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetFormat"));

  // sanity check(s)
  CheckPointer (ppmt_inout, E_POINTER);
  IPin* pin_p = NULL;
  //HRESULT result = inherited::ConnectedTo (&pin_p);
  //if (FAILED (result)) return result;
  //pin_p->Release ();
  ACE_ASSERT (!*ppmt_inout);
  ACE_ASSERT (mediaType_);

  *ppmt_inout = Stream_MediaFramework_DirectShow_Tools::copy (*mediaType_);
  if (!*ppmt_inout)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    return E_OUTOFMEMORY;
  } // end IF
  ACE_ASSERT (*ppmt_inout);

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetNumberOfCapabilities (int* piCount_inout,
                                                                                                        int* piSize_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetNumberOfCapabilities"));

  // sanity check(s)
  CheckPointer (piCount_inout, E_POINTER);
  CheckPointer (piSize_inout, E_POINTER);

  *piCount_inout = 1;
  *piSize_inout =
    (Stream_MediaFramework_DirectShow_Tools::isVideoFormat (inherited::m_mt) ? sizeof (struct _VIDEO_STREAM_CONFIG_CAPS)
                                                                             : sizeof (struct _AUDIO_STREAM_CONFIG_CAPS));

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetStreamCaps (int iIndex_in,
                                                                                              struct _AMMediaType** ppmt_inout,
                                                                                              BYTE* pSCC_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetStreamCaps"));

  // sanity check(s)
  if (iIndex_in > 0)
    return S_FALSE; // E_INVALIDARG
  CheckPointer (ppmt_inout, E_POINTER);
  ACE_ASSERT (!*ppmt_inout);
  CheckPointer (pSCC_in, E_POINTER);

  *ppmt_inout =
    Stream_MediaFramework_DirectShow_Tools::copy (inherited::m_mt);
  if (!*ppmt_inout)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s/%s: failed to Stream_MediaFramework_DirectShow_Tools::copy(): \"%m\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ())));
    return E_OUTOFMEMORY;
  } // end IF
  ACE_ASSERT (*ppmt_inout);

  if (Stream_MediaFramework_DirectShow_Tools::isVideoFormat (inherited::m_mt))
  {
    struct _VIDEO_STREAM_CONFIG_CAPS* capabilities_p =
      reinterpret_cast<struct _VIDEO_STREAM_CONFIG_CAPS*> (pSCC_in);
    ACE_OS::memset (capabilities_p, 0, sizeof (struct _VIDEO_STREAM_CONFIG_CAPS));
    capabilities_p->guid = inherited::m_mt.formattype;
    capabilities_p->VideoStandard = 0;
    struct tagVIDEOINFOHEADER* video_info_p = NULL;
    struct tagVIDEOINFOHEADER2* video_info_2 = NULL;
    if (InlineIsEqualGUID (inherited::m_mt.formattype, FORMAT_VideoInfo))
    {
      video_info_p =
        reinterpret_cast<struct tagVIDEOINFOHEADER*> (inherited::m_mt.pbFormat);
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
    else if (InlineIsEqualGUID (inherited::m_mt.formattype, FORMAT_VideoInfo2))
    {
      video_info_2 =
        reinterpret_cast<struct tagVIDEOINFOHEADER2*> (inherited::m_mt.pbFormat);
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
                  ACE_TEXT ("%s/%s: invalid/unknnown media type format (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Common_Tools::GUIDToString (inherited::m_mt.formattype).c_str ())));
      return E_FAIL;
    } // end ELSE
  } // end IF
  else
  {
    struct _AUDIO_STREAM_CONFIG_CAPS* capabilities_p =
      reinterpret_cast<struct _AUDIO_STREAM_CONFIG_CAPS*> (pSCC_in);
    ACE_OS::memset (capabilities_p, 0, sizeof (struct _AUDIO_STREAM_CONFIG_CAPS));
    capabilities_p->guid = inherited::m_mt.formattype;
    if (InlineIsEqualGUID (inherited::m_mt.formattype, FORMAT_WaveFormatEx))
    {
      struct tWAVEFORMATEX* audio_info_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (inherited::m_mt.pbFormat);
      capabilities_p->MinimumChannels = audio_info_p->nChannels;
      capabilities_p->MaximumChannels = audio_info_p->nChannels;
      capabilities_p->ChannelsGranularity = 1;
      capabilities_p->MinimumBitsPerSample = audio_info_p->wBitsPerSample;
      capabilities_p->MaximumBitsPerSample = audio_info_p->wBitsPerSample;
      capabilities_p->BitsPerSampleGranularity = 8;
      capabilities_p->MinimumSampleFrequency = audio_info_p->nSamplesPerSec;
      capabilities_p->MaximumSampleFrequency = audio_info_p->nSamplesPerSec;
      capabilities_p->SampleFrequencyGranularity = 0; // *TODO*
    } // end IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s/%s: invalid/unknnown media type format (was: \"%s\"), aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
                  ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
                  ACE_TEXT (Common_Tools::GUIDToString (inherited::m_mt.formattype).c_str ())));
      return E_FAIL;
    } // end ELSE
  } // end ELSE

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetLatency (REFERENCE_TIME* prtLatency_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetLatency"));

  // sanity check(s)
  CheckPointer (prtLatency_out, E_POINTER);

  *prtLatency_out =
    MILLISECONDS_TO_100NS_UNITS(STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_MAX_LATENCY_MS);

  return NOERROR;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetPushSourceFlags (ULONG* pFlags_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetPushSourceFlags"));

  // sanity check(s)
  CheckPointer (pFlags_out, E_POINTER);
  ACE_ASSERT (configuration_);

  ULONG flags_i = AM_PUSHSOURCECAPS_PRIVATE_CLOCK;
  if (configuration_->setSampleTimes)
    flags_i |= AM_PUSHSOURCECAPS_NOT_LIVE;
  else
    flags_i |= AM_PUSHSOURCECAPS_INTERNAL_RM;
    
  *pFlags_out = flags_i;

  return NOERROR;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::SetPushSourceFlags (ULONG flags_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::SetPushSourceFlags"));

  ACE_UNUSED_ARG (flags_in);

  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (E_FAIL);
  ACE_NOTREACHED (return E_FAIL;)
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::SetStreamOffset (REFERENCE_TIME rtOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::SetStreamOffset"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set stream offset to %qms, continuing\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ConvertToMilliseconds (rtOffset_in)));

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetStreamOffset (REFERENCE_TIME* prtOffset_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetStreamOffset"));

  // sanity check(s)
  CheckPointer (prtOffset_out, E_POINTER);

  *prtOffset_out = sampleTime_;

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::GetMaxStreamOffset (REFERENCE_TIME* prtMaxOffset_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::GetStreamOffset"));

  // sanity check(s)
  CheckPointer (prtMaxOffset_out, E_POINTER);

  *prtMaxOffset_out = frameInterval_;

  return S_OK;
}

template <typename ConfigurationType>
STDMETHODIMP
Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T<ConfigurationType>::SetMaxStreamOffset (REFERENCE_TIME rtMaxOffset_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_DirectShow_Source_Filter_OutputPin_T::SetMaxStreamOffset"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s/%s: set max stream offset to %qms, continuing\n"),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (inherited::m_pFilter).c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (this).c_str ()),
              ConvertToMilliseconds (rtMaxOffset_in)));

  return S_OK;
}
