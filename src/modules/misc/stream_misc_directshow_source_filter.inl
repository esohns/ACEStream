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

#include "initguid.h" // *NOTE*: this exports DEFINE_GUIDs

#include "ace/Log_Msg.h"
#include "ace/Message_Block.h"
#include "ace/OS_Memory.h"

#include "streams.h"

#include "common_tools.h"

#include "stream_macros.h"

#include "stream_misc_defines.h"

// *TODO*: this should be defined in the filter DLL ONLY
// {F9F62434-535B-4934-A695-BE8D10A4C699}
DEFINE_GUID (CLSID_ACEStream_Source_Filter,
0xf9f62434, 0x535b, 0x4934, 0xa6, 0x95, 0xbe, 0x8d, 0x10, 0xa4, 0xc6, 0x99);

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ModuleType>
CUnknown* WINAPI
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ModuleType>::CreateInstance (LPUNKNOWN lpunk_in,
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
                                lpunk_in,
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
//template <typename TimePolicyType,
//          typename SessionMessageType,
//          typename ProtocolMessageType,
//          typename ModuleType>
//void WINAPI
//Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
//                                       SessionMessageType,
//                                       ProtocolMessageType,
//                                       ModuleType>::InitializeInstance (BOOL loading_in,
//                                                                        const CLSID* CLSID_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::InitializeInstance"));
//
//  ACE_UNUSED_ARG (loading_in);
//  ACE_UNUSED_ARG (CLSID_in);
//} // InitializeInstance

template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ModuleType>
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ModuleType>::Stream_Misc_DirectShow_Source_Filter_T ()
 : inherited (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE, // name
              NULL,                                    // owner
              CLSID_ACEStream_Source_Filter)           // CLSID
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Stream_Misc_DirectShow_Source_Filter_T"));

  // sanity check(s)
  ACE_ASSERT (!inherited::m_paStreams);

  CAutoLock cAutoLock (&(inherited::m_cStateLock));

  ACE_NEW_NORETURN (inherited::m_paStreams,
                    CSourceStream*[1]);
  if (!inherited::m_paStreams)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF

  HRESULT result = S_OK;
  ACE_NEW_NORETURN (inherited::m_paStreams[0],
                    FILTER_T (&result,
                              this,
                              MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME));
  if (!inherited::m_paStreams[0])
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
  ACE_UNUSED_ARG (result);
} // (Constructor)
template <typename TimePolicyType,
          typename SessionMessageType,
          typename ProtocolMessageType,
          typename ModuleType>
Stream_Misc_DirectShow_Source_Filter_T<TimePolicyType,
                                       SessionMessageType,
                                       ProtocolMessageType,
                                       ModuleType>::Stream_Misc_DirectShow_Source_Filter_T (LPTSTR name_in,
                                                                                            LPUNKNOWN owner_in,
                                                                                            const GUID& GUID_in,
                                                                                            HRESULT* result_out)
 : inherited (name_in,
              owner_in,
              GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_T::Stream_Misc_DirectShow_Source_Filter_T"));

  // sanity check(s)
  ACE_ASSERT (!inherited::m_paStreams);

  CAutoLock cAutoLock (&(inherited::m_cStateLock));

  ACE_NEW_NORETURN (inherited::m_paStreams,
                    CSourceStream*[1]);
  if (!inherited::m_paStreams)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    *result_out = E_OUTOFMEMORY;
    return;
  } // end IF

  ACE_NEW_NORETURN (inherited::m_paStreams[0],
                    FILTER_T (result_out,
                              this,
                              MODULE_MISC_DS_WIN32_FILTER_PIN_OUTPUT_NAME));
  if (!inherited::m_paStreams[0])
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    *result_out = E_OUTOFMEMORY;
    return;
  } // end IF
} // (Constructor)

////////////////////////////////////////////////////////////////////////////////

template <typename FilterType,
          typename ModuleType>
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::Stream_Misc_DirectShow_Source_Filter_OutputPin_T (HRESULT* result_out,
                                                                                                                FilterType* parent_in,
                                                                                                                LPCWSTR pinName_in)
 : inherited (NAME (MODULE_MISC_DS_WIN32_FILTER_NAME_SOURCE),
              result_out,
              parent_in,
              pinName_in)
 , defaultFrameInterval_ (MODULE_MISC_DS_WIN32_FILTER_SOURCE_FRAME_INTERVAL)
 , frameInterval_ (0)
 , lock_ ()
 , sampleTime_ ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::Stream_Misc_DirectShow_Source_Filter_OutputPin_T"));

} // (Constructor)

template <typename FilterType,
          typename ModuleType>
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::~Stream_Misc_DirectShow_Source_Filter_OutputPin_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::~Stream_Misc_DirectShow_Source_Filter_OutputPin_T"));

} // (Destructor)

//
// CheckMediaType
//
// We will accept 8, 16, 24 or 32 bit video formats, in any
// image size that gives room to bounce.
// Returns E_INVALIDARG if the mediatype is not acceptable
//
template <typename FilterType,
          typename ModuleType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::CheckMediaType (const CMediaType *mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::CheckMediaType"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  if ((*(mediaType_in->Type ()) != MEDIATYPE_Video) ||
      !mediaType_in->IsFixedSize ())
    return E_FAIL;
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  const GUID* sub_type_p = mediaType_in->Subtype ();
  ACE_ASSERT (sub_type_p);
  // *TODO*: device-dependent --> make this configurable
  if ((*sub_type_p != MEDIASUBTYPE_RGB24) &&
      (*sub_type_p != MEDIASUBTYPE_RGB32) &&
      (*sub_type_p != MEDIASUBTYPE_MJPG)  &&
      (*sub_type_p != MEDIASUBTYPE_YUY2))
    return E_FAIL;

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
    return E_FAIL;

  return S_OK;
} // CheckMediaType

//
// GetMediaType
//
// Prefered types should be ordered by quality, zero as highest quality
// (iPosition > 4 is invalid)
//
template <typename FilterType,
          typename ModuleType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::GetMediaType (int position_in,
                                                                            CMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::GetMediaType"));

  // sanity check(s)
  if (position_in < 0)
    return E_INVALIDARG;
  if (position_in > 4)
    return VFW_S_NO_MORE_ITEMS;
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  struct tagVIDEOINFO* video_info_p =
    (struct tagVIDEOINFO*)mediaType_in->AllocFormatBuffer (sizeof (struct tagVIDEOINFO));
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

  video_info_p->bmiHeader.biSize = sizeof (struct tagBITMAPINFOHEADER);
  video_info_p->bmiHeader.biWidth = 320;
  video_info_p->bmiHeader.biHeight = 240;
  video_info_p->bmiHeader.biPlanes = 1;
  video_info_p->bmiHeader.biSizeImage =
    GetBitmapSize (&video_info_p->bmiHeader);
  //video_info_p->bmiHeader.biClrImportant = 0;

  BOOL result = SetRectEmpty (&(video_info_p->rcSource));
  ACE_ASSERT (result);
  result = SetRectEmpty (&(video_info_p->rcTarget));
  ACE_ASSERT (result);

  mediaType_in->SetType (&MEDIATYPE_Video);
  mediaType_in->SetFormatType (&FORMAT_VideoInfo);
  mediaType_in->SetTemporalCompression (FALSE);

  // Work out the GUID for the subtype from the header info.
  const GUID SubTypeGUID = GetBitmapSubtype (&video_info_p->bmiHeader);
  mediaType_in->SetSubtype (&SubTypeGUID);
  mediaType_in->SetSampleSize (video_info_p->bmiHeader.biSizeImage);

  return S_OK;
} // GetMediaType

//
// SetMediaType
//
// Called when a media type is agreed between filters
//
template <typename FilterType,
          typename ModuleType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::SetMediaType (const CMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::DecideBufferSize"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  // Pass the call up to my base class
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

//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
template <typename FilterType,
          typename ModuleType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::DecideBufferSize (IMemAllocator* allocator_in,
                                                                                struct _AllocatorProperties* properties_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::DecideBufferSize"));

  // sanity check(s)
  ACE_ASSERT (allocator_in);
  ACE_ASSERT (properties_in);
  ACE_ASSERT (inherited::m_pFilter);

  CAutoLock cAutoLock (inherited::m_pFilter->pStateLock ());

  struct tagVIDEOINFO* video_info_p =
    (struct tagVIDEOINFO*)inherited::m_mt.Format ();
  ACE_ASSERT (video_info_p);
  //properties_in->cbAlign = 0;
  properties_in->cbBuffer = video_info_p->bmiHeader.biSizeImage;
  ACE_ASSERT (properties_in->cbBuffer);
  //properties_in->cbPrefix = 0;
  properties_in->cBuffers = MODULE_MISC_DS_WIN32_FILTER_SOURCE_BUFFERS;

  // Ask the allocator to reserve us some sample memory, NOTE the function
  // can succeed (that is return NOERROR) but still not have allocated the
  // memory that we requested, so we must check we got whatever we wanted
  struct _AllocatorProperties properties;
  ACE_OS::memset (&properties, 0, sizeof (struct _AllocatorProperties));
  HRESULT result = allocator_in->SetProperties (properties_in,
                                                &properties);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMemAllocator::SetProperties(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF
    // Is this allocator unsuitable
  if (properties.cbBuffer < properties_in->cbBuffer)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("IMemAllocator::SetProperties() returned %d (expected: %d), aborting\n"),
                properties.cbBuffer, properties_in->cbBuffer));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (properties.cBuffers == 1);

  return S_OK;
} // DecideBufferSize

//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
template <typename FilterType,
          typename ModuleType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::OnThreadCreate ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::OnThreadCreate"));

  CAutoLock cAutoLockShared (&lock_);

  // we need to also reset the repeat time in case the system
  // clock is turned off after m_iRepeatTime gets very big
  frameInterval_ = defaultFrameInterval_;

  return NOERROR;
} // OnThreadCreate

//
// FillBuffer
//
template <typename FilterType,
          typename ModuleType>
HRESULT
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::FillBuffer (IMediaSample* mediaSample_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Misc_DirectShow_Source_Filter_OutputPin_T::FillBuffer"));

  // sanity check(s)
  ACE_ASSERT (inherited::m_pFilter);
  ACE_ASSERT (mediaSample_in);
  CheckPointer (mediaSample_in, E_POINTER);

  HRESULT result = E_FAIL;
  BYTE* data_p = NULL;
  result = mediaSample_in->GetPointer (&data_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::GetPointer(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (data_p);
  long data_length_l = 0;
  data_length_l = mediaSample_in->GetSize ();
  ACE_ASSERT (data_length_l);

  ACE_Message_Block* message_block_p = NULL;
  ModuleType* filter_p = dynamic_cast<ModuleType*> (inherited::m_pFilter);
  if (!filter_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<ModuleType*>(%@), aborting\n"),
                inherited::m_pFilter));
    return E_FAIL;
  } // end IF
  int result_2 = filter_p->getq (message_block_p, NULL);
  if (result_2 == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Task::getq(): \"%m\", aborting\n")));
    return E_FAIL;
  } // end IF
  ACE_ASSERT (message_block_p);

  size_t data_length_2 = message_block_p->length ();
  ACE_ASSERT (static_cast<size_t> (data_length_l) >= data_length_2);
  // *TODO*: this shouldn't happen
  ACE_OS::memcpy (data_p,
                  message_block_p->rd_ptr (), data_length_2);
  result = mediaSample_in->SetActualDataLength (data_length_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::SetActualDataLength(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  // The current time is the sample's start
  CRefTime ref_time = sampleTime_;
  // Increment to find the finish time
  sampleTime_ += (LONG)frameInterval_;
  result = mediaSample_in->SetTime ((REFERENCE_TIME*)&ref_time,
                                    (REFERENCE_TIME*)&sampleTime_);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::SetTime(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  result = mediaSample_in->SetSyncPoint (TRUE);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMediaSample::SetSyncPoint(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return result;
  } // end IF

  return NOERROR;
} // FillBuffer

//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
template <typename FilterType,
          typename ModuleType>
STDMETHODIMP
Stream_Misc_DirectShow_Source_Filter_OutputPin_T<FilterType,
                                                 ModuleType>::Notify (IBaseFilter* filter_in,
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
