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
#include "ace/OS_Memory.h"

#include "common_tools.h"

#include "stream_macros.h"

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

  CUnknown* unknown_p = reinterpret_cast<CUnknown*> (pointer_in);
  OWN_TYPE_T* instance_p = dynamic_cast<OWN_TYPE_T*> (unknown_p);
  ACE_ASSERT (instance_p);

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

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename MediaType,
//          typename ModuleType>
//Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       MediaType,
//                                       ModuleType>::Stream_Misc_DirectShow_Source_Filter_T ()
// : inherited (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE, // name
//              NULL,                                    // owner
//              CLSID_ACEStream_Source_Filter)           // CLSID
// , hasCOMReference_ (false)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Stream_Misc_DirectShow_Source_Filter_T"));
//
//  CUnknown* unknown_p = this;
//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("CUnknown COM handle offset: %d\n"),
//              (unsigned int)this - (unsigned int)unknown_p));
//
//  // sanity check(s)
//  ACE_ASSERT (!inherited::m_paStreams);
//
//  CAutoLock cAutoLock (&(inherited::m_cStateLock));
//
//  ACE_NEW_NORETURN (inherited::m_paStreams,
//                    CSourceStream*[1]);
//  if (!inherited::m_paStreams)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
//    return;
//  } // end IF
//
//  HRESULT result = S_OK;
//  ACE_NEW_NORETURN (inherited::m_paStreams[0],
//                    FILTER_T (&result,
//                              this,
//                              MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME));
//  if (!inherited::m_paStreams[0])
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
//    return;
//  } // end IF
//  ACE_UNUSED_ARG (result);
//
//  // support the COM object instance lifecycle
//  hasCOMReference_ = (inherited::AddRef () >= 1);
//} // (Constructor)
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
                                                                                           const struct _GUID& GUID_in,
                                                                                           HRESULT* result_out)
 : inherited (name_in,
              owner_in,
              GUID_in)
//, hasCOMReference_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Stream_Misc_DirectShow_Source_Filter_T"));

} // (Constructor)

//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename MediaType,
//          typename ModuleType>
//ULONG
//Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       MediaType,
//                                       ModuleType>::Stream_Misc_DirectShow_Source_Filter_T::Release ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Release"));
//
//  ULONG reference_count = inherited::Release ();
//
//  //if (reference_count == 0)
//  //  return; // dtor has been invoked --> done
//}

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
                                       MediaType>::Stream_Misc_DirectShow_Source_Filter_T::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::initialize"));

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
    return false;
  } // end IF
  // *TODO*: remove type inference
  if (!pin_p->initialize (*configuration_in.pinConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Misc_DirectShow_Source_Filter_OutputPin_T::initialize(), aborting\n")));
    return false;
  } // end IF

  configuration_ = &const_cast<ConfigurationType&> (configuration_in);

  return true;
}

////////////////////////////////////////////////////////////////////////////////

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::Stream_Misc_DirectShow_Source_Filter_OutputPin_T (HRESULT* result_out,
                                                                                                               FilterType* parent_in,
                                                                                                               LPCWSTR pinName_in)
 : inherited (NAME (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE),
              result_out,
              parent_in,
              pinName_in)
 , isInitialized_ (false)
 //, mediaType_ (NULL)
 , queue_ (NULL)
 , configuration_ (NULL)
 , defaultFrameInterval_ (MODULE_MISC_DS_WIN32_FILTER_SOURCE_FRAME_INTERVAL)
 , frameInterval_ (0)
 , lock_ ()
 , numberOfMediaTypes_ (1)
 , sampleTime_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::Stream_Misc_DirectShow_Source_Filter_OutputPin_T"));

} // (Constructor)

template <typename ConfigurationType,
          typename FilterType,
          typename MediaType>
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::~Stream_Misc_DirectShow_Source_Filter_OutputPin_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::~Stream_Misc_DirectShow_Source_Filter_OutputPin_T"));

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
  //mediaType_ = configuration_->mediaType;
  queue_ = configuration_->queue;

  isInitialized_ = true;

  return true;
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
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<ConfigurationType,
                                                 FilterType,
                                                 MediaType>::CheckMediaType (const CMediaType *mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::CheckMediaType"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  //if ((*(mediaType_in->Type ()) != MEDIATYPE_Video) ||
  //    !mediaType_in->IsFixedSize ())
  //  return E_FAIL;
  ACE_ASSERT (configuration_);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  if (isInitialized_)
  {
    // sanity check(s)
    ACE_ASSERT (configuration_->format);

    CMediaType media_type;
    HRESULT result = media_type.Set (*configuration_->format);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CMediaType::Set(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return E_FAIL;
    } // end IF
    if (!media_type.MatchesPartial (mediaType_in))
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("incompatible media types (\"%s\"\n\"%s\")\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (*configuration_->format).c_str ()),
                  ACE_TEXT (Stream_Module_Device_Tools::mediaTypeToString (*mediaType_in).c_str ())));
      return S_FALSE;
    } // end IF

    return S_OK;
  } // end IF

  const GUID* sub_type_p = mediaType_in->Subtype ();
  ACE_ASSERT (sub_type_p);
  // *TODO*: device-dependent --> make this configurable
  if ((*sub_type_p != MEDIASUBTYPE_RGB24) &&
      (*sub_type_p != MEDIASUBTYPE_RGB32) &&
      (*sub_type_p != MEDIASUBTYPE_MJPG)  &&
      (*sub_type_p != MEDIASUBTYPE_YUY2))
    return S_FALSE;

  struct tagVIDEOINFO* video_info_p =
    (struct tagVIDEOINFO*)mediaType_in->Format ();
  ACE_ASSERT (video_info_p);
  //if ((video_info_p->bmiHeader.biWidth < 20) ||
  //    (video_info_p->bmiHeader.biHeight < 20))
  //  return E_FAIL;

  // If the image width/height is changed, fail CheckMediaType() to force
  // the renderer to resize the image.
  // *TODO*: stream-dependent --> make this configurable
  if (video_info_p->bmiHeader.biWidth  != 320 ||
      video_info_p->bmiHeader.biHeight != 240)
    return S_FALSE;

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
  ACE_ASSERT (mediaType_out);
  ACE_ASSERT (configuration_);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  if (isInitialized_)
  {
    // sanity check(s)
    ACE_ASSERT (configuration_->format);

    result = mediaType_out->Set (*configuration_->format);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CMediaType::Set(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return result;
    } // end IF

    return S_OK;
  } // end IF

  struct tagVIDEOINFO* video_info_p =
    (struct tagVIDEOINFO*)mediaType_out->AllocFormatBuffer (sizeof (struct tagVIDEOINFO));
  if (!video_info_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CMediaType::AllocFormatBuffer(%u): \"%m\", aborting\n"),
                sizeof (struct tagVIDEOINFO)));
    return E_OUTOFMEMORY;
  } // end IF
  ZeroMemory (video_info_p, sizeof (struct tagVIDEOINFO));

  video_info_p->bmiHeader.biCompression = BI_RGB;
  switch (position_in)
  {
    //case 0:
    //{
    //  video_info_p->bmiHeader.biBitCount = 24;
    //  break;
    //}
    case 0:
    {
      video_info_p->bmiHeader.biBitCount = 24;
      break;
    }
    //case 1:
    //{
    //  video_info_p->bmiHeader.biBitCount = 32;
    //  break;
    //}
    case 1:
    {
      video_info_p->bmiHeader.biCompression = BI_JPEG;
      video_info_p->bmiHeader.biBitCount = 32;
      break;
    }
    // *TODO*: determine YUY2 settings
    case 2:
    {
      video_info_p->bmiHeader.biBitCount = 8;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown position (was: %d), aborting\n"),
                  position_in));
      return E_FAIL;
    }
  } // end SWITCH

  // *TODO*: make this configurable (and part of a protocol)
  video_info_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_p->bmiHeader.biWidth = 320;
  video_info_p->bmiHeader.biHeight = 240;
  video_info_p->bmiHeader.biPlanes = 1;
  video_info_p->bmiHeader.biBitCount = 24;
  video_info_p->bmiHeader.biCompression = BI_RGB;
  video_info_p->bmiHeader.biSizeImage =
    GetBitmapSize (&video_info_p->bmiHeader);
  //video_info_p->bmiHeader.biXPelsPerMeter;
  //video_info_p->bmiHeader.biYPelsPerMeter;
  //video_info_p->bmiHeader.biClrUsed;
  //video_info_p->bmiHeader.biClrImportant;

  BOOL result_2 = SetRectEmpty (&(video_info_p->rcSource));
  ACE_ASSERT (result_2);
  result_2 = SetRectEmpty (&(video_info_p->rcTarget));
  ACE_ASSERT (result_2);

  mediaType_out->SetType (&MEDIATYPE_Video);
  mediaType_out->SetFormatType (&FORMAT_VideoInfo);
  mediaType_out->SetTemporalCompression (FALSE);

  // work out the GUID for the subtype from the header info
  struct _GUID SubTypeGUID = GetBitmapSubtype (&video_info_p->bmiHeader);
  if (SubTypeGUID == GUID_NULL)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetBitmapSubtype(), falling back\n")));
    SubTypeGUID = MEDIASUBTYPE_Avi; // fallback
  } // end IF
  mediaType_out->SetSubtype (&SubTypeGUID);
  mediaType_out->SetSampleSize (video_info_p->bmiHeader.biSizeImage);

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
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::DecideBufferSize"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  // pass the call up to my base class
  HRESULT result = inherited::SetMediaType (mediaType_in);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CSourceStream::SetMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  //struct tagVIDEOINFO* video_info_p =
  //  (struct tagVIDEOINFO*)inherited::m_mt.Format ();
  //ACE_ASSERT (video_info_p);

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
  ACE_ASSERT (inputPin_in);
  ACE_ASSERT (allocator_out);
  ACE_ASSERT (!*allocator_out);

  // *NOTE*: see also https://msdn.microsoft.com/en-us/library/windows/desktop/dd319039(v=vs.85).aspx

  struct _AllocatorProperties properties;
  ACE_OS::memset (&properties, 0, sizeof (struct _AllocatorProperties));
  HRESULT result = E_FAIL;

  result = inputPin_in->GetAllocator (allocator_out);
  if (SUCCEEDED (result))
  {
    // sanity check(s)
    ACE_ASSERT (*allocator_out);

    result = (*allocator_out)->GetProperties (&properties);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMemAllocator::GetProperties(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // *NOTE*: if the input pin has an 'unconfigured' allocator, proceed as if
    //         it had none (IMemAllocator::SetProperties() fails with
    //         E_INVALIDARG (0x80070057) [in some cases (?) (e.g. the
    //         ColourSpaceConverter)], so consistent allocator reuse is not
    //         always possible)
    // *TODO*: find out what is happening here (see: continue_3)
    if (properties.cBuffers == 0)
    {
      IPin* pin_p = NULL;
      result = inputPin_in->QueryInterface (IID_IPin,
                                            (void**)&pin_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMemInputPin::QueryInterface(IID_IPin): \"%s\", continuing\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));
        goto error_;
      } // end IF
      ACE_ASSERT (pin_p);
      IBaseFilter* filter_p = Stream_Module_Device_Tools::pin2Filter (pin_p);
      if (!filter_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Module_Device_Tools::pin2Filter(), continuing\n")));
        goto error_;
      } // end IF

      goto continue_;

error_:
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("input pin allocator is not configured, continuing\n")));
      goto clean;
continue_:
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: input pin allocator is not configured, continuing\n"),
                  ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ())));
      goto clean;
clean:
      if (filter_p)
        filter_p->Release ();
      if (pin_p)
        pin_p->Release ();
      (*allocator_out)->Release ();
      *allocator_out = NULL;

      goto continue_2;
      //goto continue_3;
    } // end IF

    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
                properties.cBuffers,
                properties.cbBuffer,
                properties.cbAlign,
                properties.cbPrefix));

    return S_OK;
  } // end IF

continue_2:
  ACE_ASSERT (!*allocator_out);

  result = inherited::InitAllocator (allocator_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CBaseOutputPin::InitAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (*allocator_out);

//continue_3:
  result = DecideBufferSize (*allocator_out,
                             &properties);
  if (FAILED (result))
  {
    IBaseFilter* filter_p = Stream_Module_Device_Tools::pin2Filter (this);
    if (!filter_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::pin2Filter(), aborting\n")));
      goto clean_2;
    } // end IF

    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to CBaseOutputPin::DecideBufferSize(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_Module_Device_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

clean_2:
    if (filter_p)
      filter_p->Release ();
    (*allocator_out)->Release ();
    *allocator_out = NULL;

    return result;
  } // end IF

  result = inputPin_in->NotifyAllocator (*allocator_out,
                                         FALSE); // read-only buffers ?
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemInputPin::NotifyAllocator(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    (*allocator_out)->Release ();
    *allocator_out = NULL;

    return result;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("allocator properties (buffers/size/alignment/prefix): %d/%d/%d/%d\n"),
              properties.cBuffers,
              properties.cbBuffer,
              properties.cbAlign,
              properties.cbPrefix));

  return S_OK;
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
                                                                               struct _AllocatorProperties* properties_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::DecideBufferSize"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);
  ACE_ASSERT (properties_in);
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
  struct tagVIDEOINFO* video_info_p =
    (struct tagVIDEOINFO*)media_type.pbFormat;
  ACE_ASSERT (video_info_p);
  //video_info_p->bmiHeader.biBitCount = 32;

  // *NOTE*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
  //         if this is 0 (why ?)
  if (properties_in->cbAlign == 0)
    properties_in->cbAlign = 1;
  properties_in->cbBuffer = //DIBSIZE (video_info_p->bmiHeader) * 2;
    //GetBitmapSize (&video_info_p->bmiHeader);
  properties_in->cbBuffer = video_info_p->bmiHeader.biSizeImage;
  ACE_ASSERT (properties_in->cbBuffer);
  //properties_in->cbPrefix = 0;
  properties_in->cBuffers = MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS;

  FreeMediaType (media_type);

  // configure the allocator to reserve sample memory; remember to validate
  // the return value to confirm availability (?) of buffer space.
  // *NOTE*: this function does not actually allocate any memory (see
  //         IMemAllocator::Commit ())
  struct _AllocatorProperties properties;
  ACE_OS::memset (&properties, 0, sizeof (struct _AllocatorProperties));
  VFW_E_ALREADY_COMMITTED;
  result = allocator_in->SetProperties (properties_in,
                                        &properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemAllocator::SetProperties(): \"%s\" (0x%x), aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ()),
                result));
    return result;
  } // end IF
  // --> is this allocator suitable ?
  // *TODO*: this definetly needs more work
  if (properties.cbBuffer < properties_in->cbBuffer)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("IMemAllocator::SetProperties() returned %d (expected: %d), aborting\n"),
                properties.cbBuffer, properties_in->cbBuffer));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (properties.cBuffers >= 1);

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
  ACE_ASSERT (mediaSample_in);
  //CheckPointer (mediaSample_in, E_POINTER);
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
    // clean up
    message_block_p->release ();

    return S_FALSE; // --> stop 'streaming thread'
  } // end IF

  size_t data_length_2 = message_block_p->length ();
  ACE_ASSERT (static_cast<size_t> (data_length_l) >= data_length_2);
  // *TODO*: use the pull strategy instead (see: IAsyncReader)
  ACE_OS::memcpy (data_p,
                  message_block_p->rd_ptr (), data_length_2);
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
  frameInterval_ = defaultFrameInterval_;

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
