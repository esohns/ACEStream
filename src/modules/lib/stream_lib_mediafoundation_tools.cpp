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
#include "stdafx.h"

#include "stream_lib_mediafoundation_tools.h"

#include <iomanip>
#include <sstream>

#include "AudioSessionTypes.h"
#include "d3d9.h"
#include "dxva2api.h"
#undef GetObject
#include "evr.h"
#include "mfapi.h"
#include "mferror.h"
#define INITGUID
#include "mmdeviceapi.h"
#include "wmcodecdsp.h"
#include "uuids.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_time_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_directsound_tools.h"
#include "stream_lib_guids.h"
#include "stream_lib_macros.h"
#include "stream_lib_tools.h"

#include "stream_lib_mediafoundation_null.h"

// initialize statics
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap;
Stream_MediaFramework_GUIDToStringMap_t Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaSubTypeToStringMap;

void
Stream_MediaFramework_MediaFoundation_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::initialize"));

  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Default, ACE_TEXT_ALWAYS_CHAR ("MF default")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Audio, ACE_TEXT_ALWAYS_CHAR ("MF audio")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Video, ACE_TEXT_ALWAYS_CHAR ("MF video")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Protected, ACE_TEXT_ALWAYS_CHAR ("MF protected")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_SAMI, ACE_TEXT_ALWAYS_CHAR ("MF SAMI")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Script, ACE_TEXT_ALWAYS_CHAR ("MF script")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Image, ACE_TEXT_ALWAYS_CHAR ("MF image")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_HTML, ACE_TEXT_ALWAYS_CHAR ("MF HTML")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Binary, ACE_TEXT_ALWAYS_CHAR ("MF binary")));
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_FileTransfer, ACE_TEXT_ALWAYS_CHAR ("MF file transfer")));
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.insert (std::make_pair (MFMediaType_Stream, ACE_TEXT_ALWAYS_CHAR ("MF stream")));
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
}

std::string
Stream_MediaFramework_MediaFoundation_Tools::identifierToString (REFGUID deviceIdentifier_in,
                                                                 REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::identifierToString"));

  std::string return_value;

  HRESULT result = E_FAIL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID))
  {
    IMMDeviceEnumerator* enumerator_p = NULL;
    result = CoCreateInstance (__uuidof (MMDeviceEnumerator), NULL,
                               CLSCTX_ALL, __uuidof (IMMDeviceEnumerator),
                               (void**)&enumerator_p);
    if (FAILED (result) || !enumerator_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(MMDeviceEnumerator): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
      return ACE_TEXT_ALWAYS_CHAR ("");
    } // end IF
    DWORD state_mask_i = DEVICE_STATEMASK_ALL;
    IMMDeviceCollection* device_collection_p = NULL;
    result = enumerator_p->EnumAudioEndpoints (eAll,
                                               state_mask_i,
                                               &device_collection_p);
    if (FAILED (result) || !device_collection_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMMDeviceEnumerator::EnumAudioEndpoints(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false, false).c_str ())));
      enumerator_p->Release ();
      return ACE_TEXT_ALWAYS_CHAR ("");
    } // end IF
    enumerator_p->Release (); enumerator_p = NULL;
    UINT number_of_devices_i = 0;
    result = device_collection_p->GetCount (&number_of_devices_i);
    ACE_ASSERT (SUCCEEDED (result));
    IMMDevice* device_p = NULL;
    IPropertyStore* properties_p = NULL;
    struct tagPROPVARIANT property_s;
    struct _GUID GUID_s = GUID_NULL;
    LPWSTR string_p = NULL;
    for (UINT i = 0;
         i < number_of_devices_i;
         ++i)
    {
      result = device_collection_p->Item (i, &device_p);
      ACE_ASSERT (SUCCEEDED (result) && device_p);
      result = device_p->OpenPropertyStore (STGM_READ,
                                            &properties_p);
      ACE_ASSERT (SUCCEEDED (result) && properties_p);
      PropVariantInit (&property_s);
      result = properties_p->GetValue (PKEY_AudioEndpoint_GUID,
                                       &property_s);
      ACE_ASSERT (SUCCEEDED (result) && (property_s.vt == VT_LPWSTR));
      properties_p->Release (); properties_p = NULL;
      GUID_s =
        Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (property_s.pwszVal)));
      if (!InlineIsEqualGUID (GUID_s, deviceIdentifier_in))
      {
        PropVariantClear (&property_s);
        device_p->Release (); device_p = NULL;
        continue;
      } // end IF
      PropVariantClear (&property_s);
      result = device_p->GetId (&string_p);
      ACE_ASSERT (SUCCEEDED (result) && string_p);
      return_value = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (string_p));
      CoTaskMemFree (string_p);
      device_p->Release (); device_p = NULL;
      break;
    } // end FOR
    device_collection_p->Release (); device_collection_p = NULL;
  } // end IF
  else if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))
  {
    ACE_ASSERT (false); // *TODO*
    ACE_NOTSUP_RETURN (ACE_TEXT_ALWAYS_CHAR (""));
    ACE_NOTREACHED (return ACE_TEXT_ALWAYS_CHAR ("");)
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s, aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end ELSE
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  return return_value;
}

struct _GUID
Stream_MediaFramework_MediaFoundation_Tools::toFormat (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toFormat"));

  struct _GUID result = GUID_NULL;

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  HRESULT result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
                                                       &result);
  if (FAILED (result_2))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));

  return result;
}

Common_Image_Resolution_t
Stream_MediaFramework_MediaFoundation_Tools::toResolution (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toResolution"));

  Common_Image_Resolution_t result;

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  UINT32 width = 0, height = 0;
  HRESULT result_2 = MFGetAttributeSize (const_cast<IMFMediaType*> (mediaType_in),
                                         MF_MT_FRAME_SIZE,
                                         &width, &height);
  if (FAILED (result_2))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
  else
  {
    result.cx = static_cast<LONG> (width);
    result.cy = static_cast<LONG> (height);
  } // end ELSE

  return result;
}

unsigned int
Stream_MediaFramework_MediaFoundation_Tools::toFramerate (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toFramerate"));

  unsigned int result = 0;

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  UINT32 numerator = 0, denominator = 1;
  HRESULT result_2 = MFGetAttributeRatio (const_cast<IMFMediaType*> (mediaType_in),
                                          MF_MT_FRAME_RATE,
                                          &numerator, &denominator);
  if (FAILED (result_2))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false, false).c_str ())));
  else
  { ACE_ASSERT (denominator == 1);
    result = numerator;
  } // end ELSE

  return result;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::canRender (const IMFMediaType* mediaType_in,
                                                        IMFMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::canRender"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  struct _GUID GUID_s = GUID_NULL;
  HRESULT result =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_MAJOR_TYPE,
                                                       &GUID_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_MAJOR_TYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false; // *TODO*: false negative !
  } // end IF

  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  if (InlineIsEqualGUID (GUID_s, MFMediaType_Audio))
  {
    result = MFCreateAudioRenderer (NULL,
                                    &media_sink_p);
    if (FAILED (result) || !media_sink_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateAudioRenderer() \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
  } // end IF
  else
  {
    ACE_ASSERT (false); // *TODO*
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown major media type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
    goto error;
  } // end ELSE

  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  if (FAILED (result) || !stream_sink_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::GetStreamSinkByIndex(0) \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result) || !media_type_handler_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamSink::GetMediaTypeHandler() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result =
    media_type_handler_p->IsMediaTypeSupported (const_cast<IMFMediaType*> (mediaType_in),
                                                &mediaType_out);
  if (FAILED (result) && !mediaType_out)
  {
    HRESULT result_2 =
      media_type_handler_p->GetMediaTypeByIndex (0,
                                                 &mediaType_out);
    if (FAILED (result_2) || !mediaType_out)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(0) \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error;
    } // end IF
  } // end IF

  media_type_handler_p->Release (); media_type_handler_p = NULL;
  stream_sink_p->Release (); stream_sink_p = NULL;
  media_sink_p->Release (); media_sink_p = NULL;

  return SUCCEEDED (result);

error:
  if (media_sink_p)
    media_sink_p->Release ();
  if (stream_sink_p)
    stream_sink_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();
  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  return false; // *TODO*: false negative !
}

bool
Stream_MediaFramework_MediaFoundation_Tools::has (const IMFMediaType* mediaType_in,
                                                  REFGUID GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::has"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);
  HRESULT result =
    const_cast<IMFMediaType*> (mediaType_in)->GetItem (GUID_in,
                                                       &property_s);
  PropVariantClear (&property_s);
  return SUCCEEDED (result);
}

bool
Stream_MediaFramework_MediaFoundation_Tools::isPartial (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::isPartial"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _GUID GUID_s = GUID_NULL;
  UINT32 value_i = 0;
  HRESULT result = E_FAIL;

  result = 
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_MAJOR_TYPE,
                                                       &GUID_s);
  if (FAILED (result))
  {
    if (result == MF_E_ATTRIBUTENOTFOUND)
      return true;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_MAJOR_TYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false; // *TODO*: false negative !
  } // end IF

  if (InlineIsEqualGUID (GUID_s, MFMediaType_Audio))
  {
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
                                                         &GUID_s);
    if (FAILED (result))
    {
      if (result == MF_E_ATTRIBUTENOTFOUND)
        return true;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false; // *TODO*: false negative !
    } // end IF
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                           &value_i);
    if (FAILED (result))
    {
      if (result == MF_E_ATTRIBUTENOTFOUND)
        return true;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false; // *TODO*: false negative !
    } // end IF
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                           &value_i);
    if (FAILED (result))
    {
      if (result == MF_E_ATTRIBUTENOTFOUND)
        return true;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false; // *TODO*: false negative !
    } // end IF
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                           &value_i);
    if (FAILED (result))
    {
      if (result == MF_E_ATTRIBUTENOTFOUND)
        return true;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false; // *TODO*: false negative !
    } // end IF
  } // end IF
  else
  {
    ACE_ASSERT (false); // *TODO*
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown major media type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
    // *TODO*: false negative !
  } // end ELSE

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::isPartOf (const IMFMediaType* mediaType_in,
                                                       const IMFMediaType* mediaType2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::isPartOf"));
  
  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (mediaType2_in);

  UINT32 items_i = 0, items_2 = 0;
  HRESULT result =
    const_cast<IMFMediaType*> (mediaType_in)->GetCount (&items_i);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    const_cast<IMFMediaType*> (mediaType2_in)->GetCount (&items_2);
  ACE_ASSERT (SUCCEEDED (result));
  if (items_2 > items_i)
    return false;
  // --> items_2 <= items_i

  //struct _GUID GUID_s = GUID_NULL, GUID_2 = GUID_NULL;
  //struct tagPROPVARIANT property_s, property_2;
  //PropVariantInit (&property_s); PropVariantInit (&property_2);
  //for (UINT32 i = 0;
  //     i < items_2;
  //     ++i)
  //{
  //  PropVariantClear (&property_s); PropVariantClear (&property_2);
  //  result =
  //    const_cast<IMFMediaType*> (mediaType2_in)->GetItemByIndex (i,
  //                                                               &GUID_s,
  //                                                               &property_s);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result =
  //    const_cast<IMFMediaType*> (mediaType_in)->GetItem (GUID_2,
  //                                                       &property_2);
  //  if (FAILED (result))
  //  { ACE_ASSERT (result == MF_E_ATTRIBUTENOTFOUND);
  //    PropVariantClear (&property_s);
  //    return false;
  //  } // end IF
  //} // end FOR

  BOOL matched_b = FALSE;
  result =
    const_cast<IMFMediaType*> (mediaType_in)->Compare (const_cast<IMFMediaType*> (mediaType2_in),
                                                       MF_ATTRIBUTES_MATCH_SMALLER,
                                                       &matched_b);
  ACE_ASSERT (SUCCEEDED (result));

  return !!matched_b;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::match (const IMFMediaType* mediaType_in,
                                                    const IMFMediaType* mediaType2_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::match"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (mediaType2_in);
  bool has_1 =
    Stream_MediaFramework_MediaFoundation_Tools::has (mediaType_in,
                                                      MF_MT_MAJOR_TYPE);
  bool has_2 =
    Stream_MediaFramework_MediaFoundation_Tools::has (mediaType2_in,
                                                      MF_MT_MAJOR_TYPE);
  if (!has_1 && !has_2)
    return true;
  if (!has_1 || !has_2)
    return false;

  DWORD flags_i = 0;
  HRESULT result =
    const_cast<IMFMediaType*> (mediaType_in)->IsEqual (const_cast<IMFMediaType*> (mediaType2_in),
                                                       &flags_i);
  ACE_ASSERT (SUCCEEDED (result));
  if (result == S_OK)
    return true;
  ACE_ASSERT (result == S_FALSE);
  return ((flags_i & MF_MEDIATYPE_EQUAL_MAJOR_TYPES)  &&
          (flags_i & MF_MEDIATYPE_EQUAL_FORMAT_TYPES) &&
          (flags_i & MF_MEDIATYPE_EQUAL_FORMAT_DATA));
}

bool
Stream_MediaFramework_MediaFoundation_Tools::merge (const IMFMediaType* mediaType_in,
                                                    IMFMediaType* mediaType2_in,
                                                    bool reconfigure_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::merge"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (mediaType2_in);

  HRESULT result = E_FAIL;
  struct _GUID GUID_s = GUID_NULL, GUID_2 = GUID_NULL;
  UINT32 value_i = 0, value_2 = 0;

  result =
    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_MAJOR_TYPE,
                                                       &GUID_s);
  ACE_ASSERT (SUCCEEDED (result));
  if (InlineIsEqualGUID (GUID_s, MFMediaType_Audio))
  {
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
                                                         &GUID_s);
    if (FAILED (result) &&
        (result != MF_E_ATTRIBUTENOTFOUND))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    if (result == MF_E_ATTRIBUTENOTFOUND)
      goto continue_;
    result = mediaType2_in->GetGUID (MF_MT_SUBTYPE,
                                     &GUID_2);
    if (result == MF_E_ATTRIBUTENOTFOUND)
    {
      result = mediaType2_in->SetGUID (MF_MT_SUBTYPE,
                                       GUID_s);
      ACE_ASSERT (SUCCEEDED (result));
    } // end IF

continue_:
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                           &value_i);
    if (FAILED (result) &&
        (result != MF_E_ATTRIBUTENOTFOUND))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    if (result == MF_E_ATTRIBUTENOTFOUND)
      goto continue_2;
    result = mediaType2_in->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                       &value_2);
    if (result == MF_E_ATTRIBUTENOTFOUND)
    {
      result = mediaType2_in->SetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                         value_i);
      ACE_ASSERT (SUCCEEDED (result));
    } // end IF

continue_2:
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                           &value_i);
    if (FAILED (result) &&
        (result != MF_E_ATTRIBUTENOTFOUND))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    if (result == MF_E_ATTRIBUTENOTFOUND)
      goto continue_3;
    result = mediaType2_in->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                       &value_2);
    if (result == MF_E_ATTRIBUTENOTFOUND)
    {
      result = mediaType2_in->SetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                         value_i);
      ACE_ASSERT (SUCCEEDED (result));
    } // end IF

continue_3:
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                           &value_i);
    if (FAILED (result) &&
        (result != MF_E_ATTRIBUTENOTFOUND))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    if (result == MF_E_ATTRIBUTENOTFOUND)
      goto continue_4;
    result = mediaType2_in->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                       &value_2);
    if (result == MF_E_ATTRIBUTENOTFOUND)
    {
      result = mediaType2_in->SetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                         value_i);
      ACE_ASSERT (SUCCEEDED (result));
    } // end IF

    // optional
continue_4:
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT,
                                                           &value_i);
    if (FAILED (result) &&
        (result != MF_E_ATTRIBUTENOTFOUND))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    if (result == MF_E_ATTRIBUTENOTFOUND)
      goto continue_5;
    result = mediaType2_in->GetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT,
                                       &value_2);
    if (result == MF_E_ATTRIBUTENOTFOUND)
    {
      result = mediaType2_in->SetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT,
                                         value_i);
      ACE_ASSERT (SUCCEEDED (result));
    } // end IF

continue_5:
    result =
      const_cast<IMFMediaType*> (mediaType_in)->GetUINT32 (MF_MT_FIXED_SIZE_SAMPLES,
                                                           &value_i);
    if (FAILED (result) &&
        (result != MF_E_ATTRIBUTENOTFOUND))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    if (result == MF_E_ATTRIBUTENOTFOUND)
      goto continue_6;
    result = mediaType2_in->GetUINT32 (MF_MT_FIXED_SIZE_SAMPLES,
                                       &value_2);
    if (result == MF_E_ATTRIBUTENOTFOUND)
    {
      result = mediaType2_in->SetUINT32 (MF_MT_FIXED_SIZE_SAMPLES,
                                         value_i);
      ACE_ASSERT (SUCCEEDED (result));
    } // end IF

continue_6:
    if (likely (reconfigure_in))
      return Stream_MediaFramework_MediaFoundation_Tools::reconfigure (mediaType2_in);

      return true;
  } // end IF
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown major media type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::reconfigure (IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::reconfigure"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);

  struct _GUID GUID_s = GUID_NULL;
  UINT32 value_i = 0, value_2 = 0;
  HRESULT result = E_FAIL;

  result = mediaType_in->GetGUID (MF_MT_MAJOR_TYPE,
                                  &GUID_s);
  if (FAILED (result))
  {
    if (result == MF_E_ATTRIBUTENOTFOUND)
      return true;
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_MAJOR_TYPE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false; // *TODO*: false negative !
  } // end IF

  if (InlineIsEqualGUID (GUID_s, MFMediaType_Audio))
  {
    result = mediaType_in->GetGUID (MF_MT_SUBTYPE,
                                    &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF

    // *NOTE*: iff MF_MT_SUBTYPE == MFAudioFormat_Float:
    //         --> set MF_MT_AUDIO_BITS_PER_SAMPLE to 32
    if (InlineIsEqualGUID (GUID_s, MFAudioFormat_Float))
    {
      result = mediaType_in->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                        &value_i);
      ACE_ASSERT (SUCCEEDED (result) && value_i);
      if (value_i != 32)
      {
        result = mediaType_in->SetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                          32);
        ACE_ASSERT (SUCCEEDED (result));
        ACE_DEBUG ((LM_WARNING,
                    ACE_TEXT ("reset MF_MT_AUDIO_BITS_PER_SAMPLE to 32 (was: %u), continuing\n"),
                    value_i));
      } // end IF
    } // end IF

    // reset MF_MT_AUDIO_BLOCK_ALIGNMENT
    result = mediaType_in->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                      &value_i);
    ACE_ASSERT (SUCCEEDED (result) && value_i);
    ACE_ASSERT ((value_i % 8) == 0);
    result = mediaType_in->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                      &value_2);
    ACE_ASSERT (SUCCEEDED (result) && value_2);
    result = mediaType_in->SetUINT32 (MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                      value_2 * (value_i / 8));
    ACE_ASSERT (SUCCEEDED (result));

    // reset MF_MT_AUDIO_AVG_BYTES_PER_SECOND
    result = mediaType_in->GetUINT32 (MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                      &value_i);
    ACE_ASSERT (SUCCEEDED (result) && value_i);
    result = mediaType_in->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &value_2);
    ACE_ASSERT (SUCCEEDED (result) && value_2);
    result = mediaType_in->SetUINT32 (MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                      value_i * value_2);
    ACE_ASSERT (SUCCEEDED (result));

    // re(set) MF_MT_AUDIO_CHANNEL_MASK
    UINT32 channel_mask_i = 0;
    result = mediaType_in->GetUINT32 (MF_MT_AUDIO_CHANNEL_MASK,
                                      &channel_mask_i);
    if (FAILED (result))
    {
      if (result != MF_E_ATTRIBUTENOTFOUND)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_MAJOR_TYPE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return false;
      } // end IF
      channel_mask_i = (SPEAKER_FRONT_LEFT |
                        SPEAKER_FRONT_RIGHT);
      result = mediaType_in->SetUINT32 (MF_MT_AUDIO_CHANNEL_MASK,
                                        channel_mask_i);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("set MF_MT_AUDIO_CHANNEL_MASK (now: 0x%x), continuing\n"),
                  channel_mask_i));
    } // end IF

    return true;
  } // end IF
  else
  {
    ACE_ASSERT (false); // *TODO*
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown major media type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
  } // end ELSE

  return false;
}

IMFMediaType*
Stream_MediaFramework_MediaFoundation_Tools::to (const struct tWAVEFORMATEX& format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::to"));

  IMFMediaType* result_p = NULL;

  HRESULT result_2 = MFCreateMediaType (&result_p);
  ACE_ASSERT (SUCCEEDED (result_2) && result_p);
  result_2 = MFInitMediaTypeFromWaveFormatEx (result_p,
                                              &format_in,
                                              sizeof (struct tWAVEFORMATEX) + format_in.cbSize);
  ACE_ASSERT (SUCCEEDED (result_2));

  // validate subtype
  struct _GUID GUID_s = GUID_NULL;
  result_2 = result_p->GetGUID (MF_MT_SUBTYPE,
                                &GUID_s);
  ACE_ASSERT (SUCCEEDED (result_2));
  if (unlikely (!InlineIsEqualGUID (GUID_s, MFAudioFormat_PCM) &&
                !InlineIsEqualGUID (GUID_s, MFAudioFormat_Float)))
  {
    GUID_s = Stream_MediaFramework_DirectShow_Tools::toSubType (format_in);
    result_2 = result_p->SetGUID (MF_MT_SUBTYPE,
                                  GUID_s);
    ACE_ASSERT (SUCCEEDED (result_2));
    Stream_MediaFramework_MediaFoundation_Tools::reconfigure (result_p);
  } // end IF

  return result_p;
}

//void
//Stream_MediaFramework_MediaFoundation_Tools::dump (IMFSourceReader* IMFSourceReader_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::dump"));
//
//  // sanity check(s)
//  ACE_ASSERT (IMFSourceReader_in);
//
//  HRESULT result = S_OK;
//  DWORD count = 0;
//  IMFMediaType* media_type_p = NULL;
//  while (true)
//  {
//    media_type_p = NULL;
//    result =
//      IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                              count,
//                                              &media_type_p);
//    if (FAILED (result)) break;
//
//    ACE_DEBUG ((LM_INFO,
//                ACE_TEXT ("#%d: %s\n"),
//                count + 1,
//                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p).c_str ())));
//
//    // clean up
//    media_type_p->Release ();
//
//    ++count;
//  } // end WHILE
//  if (result != MF_E_NO_MORE_TYPES)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", returning\n"),
//                count,
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return;
//  } // end IF
//}

void
Stream_MediaFramework_MediaFoundation_Tools::clean (TOPOLOGY_PATH_T& path_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::clean"));

  for (TOPOLOGY_PATH_ITERATOR_T iterator = path_in.begin ();
       iterator != path_in.end ();
       ++iterator)
    (*iterator)->Release ();
  path_in.clear ();
}

void
Stream_MediaFramework_MediaFoundation_Tools::clean (TOPOLOGY_PATHS_T& paths_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::clean"));

  for (TOPOLOGY_PATHS_ITERATOR_T iterator = paths_in.begin ();
       iterator != paths_in.end ();
       ++iterator)
    Stream_MediaFramework_MediaFoundation_Tools::clean (*iterator);
  paths_in.clear ();
}

bool
Stream_MediaFramework_MediaFoundation_Tools::parse (const IMFTopology* topology_in,
                                                    TOPOLOGY_PATHS_T& branches_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::parse"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  Stream_MediaFramework_MediaFoundation_Tools::clean (branches_out);

  IMFCollection* collection_p = NULL;
  HRESULT result =
    const_cast<IMFTopology*> (topology_in)->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_nodes_i = 0;
  result = collection_p->GetElementCount (&number_of_nodes_i);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_nodes_i <= 0)
  {
    collection_p->Release (); collection_p = NULL;
    return true;
  } // end IF
  IUnknown* unknown_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  TOPOLOGY_PATHS_T branches_a;
  for (DWORD i = 0;
       i < number_of_nodes_i;
       ++i)
  {
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result) && unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    ACE_ASSERT (SUCCEEDED (result) && topology_node_p);
    unknown_p->Release (); unknown_p = NULL;
    TOPOLOGY_PATH_T path_s;
    path_s.push_back (topology_node_p);
    branches_a.push_back (path_s);
  } // end FOR
  collection_p->Release (); collection_p = NULL;

  for (TOPOLOGY_PATHS_ITERATOR_T iterator = branches_a.begin ();
       iterator != branches_a.end ();
       ++iterator)
  {
    TOPOLOGY_PATHS_T branches_2;
    Stream_MediaFramework_MediaFoundation_Tools::expand (*iterator,
                                                         (*iterator).begin (),
                                                         branches_2);
    branches_out.splice (branches_out.end (), branches_2);
  } // end FOR
  Stream_MediaFramework_MediaFoundation_Tools::clean (branches_a);

  return true;
}

void
Stream_MediaFramework_MediaFoundation_Tools::expand (const TOPOLOGY_PATH_T& path_in,
                                                     TOPOLOGY_PATH_ITERATOR_T& iterator_in,
                                                     TOPOLOGY_PATHS_T& paths_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::expand"));

  HRESULT result = E_FAIL;
  DWORD number_of_nodes_i = 0;
  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index_i = 0;
  TOPOLOGY_PATH_T topology_path;

  for (;
       iterator_in != path_in.end ();
       ++iterator_in)
  {
    result = (*iterator_in)->GetOutputCount (&number_of_nodes_i);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_nodes_i <= 0)
    { ACE_ASSERT (++iterator_in == path_in.end ());
      topology_path = path_in;
      for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
           iterator_2 != topology_path.end ();
           ++iterator_2)
        (*iterator_2)->AddRef ();
      paths_inout.push_back (topology_path);
      return;
    } // end IF
    else if (number_of_nodes_i == 1)
    {
      // last node ? expand : continue
      TOPOLOGY_PATH_ITERATOR_T iterator_2 = iterator_in;
      std::advance (iterator_2, 1);
      if (iterator_2 != path_in.end ())
        continue;
      topology_node_p = NULL;
      result = (*iterator_in)->GetOutput (0,
                                          &topology_node_p,
                                          &input_index_i);
      if (FAILED (result))
      {
        if (result == MF_E_NOT_FOUND) // output exists but is not connected
        { ACE_ASSERT (++iterator_in == path_in.end ());
          topology_path = path_in;
          for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
               iterator_2 != topology_path.end ();
               ++iterator_2)
            (*iterator_2)->AddRef ();
          paths_inout.push_back (topology_path);
          return;
        } // end IF
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopologyNode::GetOutput(%u): \"%s\", aborting\n"),
                    0,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      topology_path = path_in;
      iterator_2 = topology_path.begin ();
      for (;
           iterator_2 != topology_path.end ();
           ++iterator_2)
        (*iterator_2)->AddRef ();
      topology_path.push_back (topology_node_p);
      iterator_2 = --topology_path.end ();
      Stream_MediaFramework_MediaFoundation_Tools::expand (topology_path,
                                                           iterator_2,
                                                           paths_inout);
      Stream_MediaFramework_MediaFoundation_Tools::clean (topology_path);
      return;
    } // end ELSE IF

    for (DWORD i = 0;
         i < number_of_nodes_i;
         ++i)
    {
      topology_node_p = NULL;
      result = (*iterator_in)->GetOutput (i,
                                          &topology_node_p,
                                          &input_index_i);
      if (FAILED (result))
      {
        if (result == MF_E_NOT_FOUND) // output exists but is not connected
        {
          topology_path = path_in;
          for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
               iterator_2 != topology_path.end ();
               ++iterator_2)
            (*iterator_2)->AddRef ();
          paths_inout.push_back (topology_path);
          continue;
        } // end IF
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopologyNode::GetOutput(%u): \"%s\", aborting\n"),
                    i,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

      topology_path = path_in;
      TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
      for (;
           iterator_2 != topology_path.end ();
           ++iterator_2)
        (*iterator_2)->AddRef ();
      topology_path.push_back (topology_node_p);
      iterator_2 = --topology_path.end ();
      Stream_MediaFramework_MediaFoundation_Tools::expand (topology_path,
                                                           iterator_2,
                                                           paths_inout);
      Stream_MediaFramework_MediaFoundation_Tools::clean (topology_path);
    } // end FOR
  } // end FOR
}

bool
Stream_MediaFramework_MediaFoundation_Tools::hasTee (TOPOLOGY_PATH_T& path_in,
                                                     IMFTopologyNode*& node_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::hasTee"));

  // sanity check(s)
  ACE_ASSERT (!node_out);

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;

  for (TOPOLOGY_PATH_ITERATOR_T iterator = path_in.begin ();
       iterator != path_in.end ();
       ++iterator)
  {
    result = (*iterator)->GetNodeType (&node_type_e);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_type_e == MF_TOPOLOGY_TEE_NODE)
    {
      node_out = *iterator;
      node_out->AddRef ();
      return true;
    } // end IF
  } // end FOR

  return false;
}

void
Stream_MediaFramework_MediaFoundation_Tools::dump (IMFTopology* topology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  WORD count_i = 0;
  HRESULT result = topology_in->GetNodeCount (&count_i);
  ACE_ASSERT (SUCCEEDED (result));
  TOPOID node_id = 0;
  result = topology_in->GetTopologyID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("topology (id: %q, %u nodes)\n"),
              node_id, count_i));

  TOPOLOGY_PATHS_T paths_s;
  if (!Stream_MediaFramework_MediaFoundation_Tools::parse (topology_in,
                                                           paths_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::parse(): \"%s\", returning\n")));
    return;
  } // end IF

  std::string topology_string_base, topology_string;
  std::ostringstream converter;
  int index_i = 0;
  IMFTopologyNode* topology_node_p = NULL;
  MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  IMFMediaType* media_type_p = NULL;
  TOPOLOGY_PATH_ITERATOR_T iterator_3;
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = paths_s.begin ();
       iterator != paths_s.end ();
       ++iterator, ++index_i)
  {
    topology_string_base = ACE_TEXT_ALWAYS_CHAR ("#");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << index_i + 1;
    topology_string_base += converter.str ();
    topology_string_base += ACE_TEXT_ALWAYS_CHAR (": ");

    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
         iterator_2 != (*iterator).end ();
         ++iterator_2)
    {
      result = (*iterator_2)->GetNodeType (&node_type_e);
      ACE_ASSERT (SUCCEEDED (result));
      result = (*iterator_2)->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));

      topology_string =
        Stream_MediaFramework_MediaFoundation_Tools::toString (node_type_e);
      topology_string += ACE_TEXT_ALWAYS_CHAR (" (");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << node_id;
      topology_string += converter.str ();

      iterator_3 = iterator_2;
      if (++iterator_3 != (*iterator).end ())
      {
        topology_string += ACE_TEXT_ALWAYS_CHAR (") --> ");
        Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_in,
                                                                      node_id,
                                                                      media_type_p);
        ACE_ASSERT (media_type_p);
        topology_string +=
          Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p,
                                                                 true);
        media_type_p->Release (); media_type_p = NULL;
        topology_string += ACE_TEXT_ALWAYS_CHAR (" --> ");
      } // end IF
      else
        topology_string += ACE_TEXT_ALWAYS_CHAR (")");
      topology_string_base += topology_string;
    } // end FOR
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s\n"),
                ACE_TEXT (topology_string_base.c_str ())));
  } // end FOR

  Stream_MediaFramework_MediaFoundation_Tools::clean (paths_s);
}

void
Stream_MediaFramework_MediaFoundation_Tools::dump (IMFAttributes* attributes_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::dump"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

void
Stream_MediaFramework_MediaFoundation_Tools::dump (IMFTransform* IMFTransform_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  result =
    IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                     &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetStreamCount(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
  DWORD* input_stream_ids_p = NULL;
  ACE_NEW_NORETURN (input_stream_ids_p,
                    DWORD [number_of_input_streams]);
  DWORD* output_stream_ids_p = NULL;
  ACE_NEW_NORETURN (output_stream_ids_p,
                    DWORD [number_of_output_streams]);
  if (!input_stream_ids_p || !output_stream_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, returning\n")));

    // clean up
    if (input_stream_ids_p)
      delete [] input_stream_ids_p;
    if (output_stream_ids_p)
      delete[] output_stream_ids_p;

    return;
  } // end IF
  result =
    IMFTransform_in->GetStreamIDs (number_of_input_streams,
                                   input_stream_ids_p,
                                   number_of_output_streams,
                                   output_stream_ids_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetStreamIDs(): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

      // clean up
      delete [] input_stream_ids_p;
      delete [] output_stream_ids_p;

      return;
    } // end IF

    int i = 0;
    for (;
         i < static_cast<int> (number_of_input_streams);
         ++i)
      input_stream_ids_p[i] = i;
    for (i = 0;
         i < static_cast<int> (number_of_output_streams);
         ++i)
      output_stream_ids_p[i] = i;
  } // end IF
  delete [] input_stream_ids_p;
  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  for (int i = 0;
       i < static_cast<int> (number_of_output_streams);
       ++i)
  {
    count = 0;

    while (true)
    {
      media_type_p = NULL;
      result =
        IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[i],
                                                 count,
                                                 &media_type_p);
      if (FAILED (result)) // MF_E_NO_MORE_TYPES: 0xC00D36B9
        break; // MF_E_TRANSFORM_TYPE_NOT_SET: 0xC00D6D60L
      ACE_ASSERT (media_type_p);

      ACE_DEBUG ((LM_INFO,
                  ACE_TEXT ("#%d: %s\n"),
                  //ACE_TEXT ("%s: #%d: %s\n"),
                  //ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::transformToString (IMFTransform_in).c_str ()),
                  count,
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p).c_str ())));

      // clean up
      media_type_p->Release ();

      ++count;
    } // end WHILE
  } // end FOR
  delete [] output_stream_ids_p;
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetOutputAvailableType(%d): \"%s\", returning\n"),
                count,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return;
  } // end IF
}

void
Stream_MediaFramework_MediaFoundation_Tools::dump (IMFStreamSink* streamSink_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (streamSink_in);

  IMFMediaTypeHandler* media_type_handler_p = NULL;
  HRESULT result = streamSink_in->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
  DWORD count_i = 0;
  result = media_type_handler_p->GetMediaTypeCount (&count_i);
  ACE_ASSERT (SUCCEEDED (result));
  IMFMediaType* media_type_p = NULL;
  for (DWORD i = 0;
       i < count_i;
       ++i)
  { ACE_ASSERT (!media_type_p);
    result = media_type_handler_p->GetMediaTypeByIndex (i,
                                                        &media_type_p);
    ACE_ASSERT (SUCCEEDED (result) && media_type_p);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("#%u: %s\n"),
                i,
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p).c_str ())));
    media_type_p->Release (); media_type_p = NULL;
  } // end FOR
  media_type_handler_p->Release (); media_type_handler_p = NULL;
}

//bool
//Stream_MediaFramework_MediaFoundation_Tools::getSourceReader (IMFMediaSource*& mediaSource_inout,
//                                             WCHAR*& symbolicLink_out,
//                                             UINT32& symbolicLinkSize_out,
//                                             const IDirect3DDeviceManager9* IDirect3DDeviceManager9_in,
//                                             const IMFSourceReaderCallback* callback_in,
//                                             bool isChromaLuminance_in, 
//                                             IMFSourceReaderEx*& sourceReader_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getSourceReader"));
//
//  bool result = false;
//
//  if (symbolicLinkSize_out)
//  {
//    // sanity check(s)
//    ACE_ASSERT (symbolicLink_out);
//
//    CoTaskMemFree (symbolicLink_out);
//    symbolicLink_out = NULL;
//    symbolicLinkSize_out = 0;
//  } // end IF
//  if (sourceReader_out)
//  {
//    sourceReader_out->Release ();
//    sourceReader_out = NULL;
//  } // end IF
//
//  if (!mediaSource_inout)
//    if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (std::string (),
//                                                     mediaSource_inout,
//                                                     symbolicLink_out,
//                                                     symbolicLinkSize_out))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), aborting\n")));
//      return false;
//    } // end IF
//  ACE_ASSERT (mediaSource_inout);
//
//  IMFAttributes* attributes_p = NULL;
//  HRESULT result_2 = MFCreateAttributes (&attributes_p, 10);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    return false;
//  } // end IF
//
//  IUnknown* iunknown_p = NULL;
//  //result_2 = attributes_p->SetUINT32 (MF_READWRITE_DISABLE_CONVERTERS,
//  //                                    FALSE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_READWRITE_DISABLE_CONVERTERS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  if (IDirect3DDeviceManager9_in)
//  {
//    result_2 = attributes_p->SetUINT32 (MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS,
//                                        TRUE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  if (callback_in)
//  {
//    iunknown_p = const_cast<IMFSourceReaderCallback*> (callback_in);
//    result_2 =
//      attributes_p->SetUnknown (MF_SOURCE_READER_ASYNC_CALLBACK,
//                                iunknown_p);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUnknown(): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  if (IDirect3DDeviceManager9_in)
//  {
//    iunknown_p =
//      const_cast<IDirect3DDeviceManager9*> (IDirect3DDeviceManager9_in);
//    result_2 =
//      attributes_p->SetUnknown (MF_SOURCE_READER_D3D_MANAGER,
//                                iunknown_p);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUnknown(MF_SOURCE_READER_D3D_MANAGER): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//      goto error;
//    } // end IF
//
//    result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISABLE_DXVA,
//                                        FALSE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISABLE_DXVA): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  //result_2 = attributes_p->SetUnknown (MF_SOURCE_READER_MEDIASOURCE_CONFIG,
//  //                                     NULL); // IPropertyBag handle
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_MEDIASOURCE_CONFIG): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS,
//  //                                    TRUE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  // *NOTE*: allow conversion from YUV to RGB-32 ?
//  // *NOTE*: "...Avoid this setting if you are using Direct3D to display the
//  //         video frames, because the GPU generally provides better video
//  //         processing capabilities.
//  //         If this attribute is TRUE, the following attributes must be FALSE :
//  //         • MF_SOURCE_READER_D3D_MANAGER
//  //         • MF_READWRITE_DISABLE_CONVERTERS ..."
//  if (!IDirect3DDeviceManager9_in)
//  {
//    if (isChromaLuminance_in)
//    {
//      result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING,
//                                          TRUE);
//      if (FAILED (result_2))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING): \"%s\", aborting\n"),
//                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//        goto error;
//      } // end IF
//    } // end IF
//  } // end IF
//  else
//  {
//    // *NOTE*: "... If this attribute is TRUE, the
//    //         MF_READWRITE_DISABLE_CONVERTERS attribute must be FALSE. ..."
//    result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING,
//                                        TRUE);
//    if (FAILED (result_2))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//      goto error;
//    } // end IF
//  } // end IF
//
//  result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISABLE_CAMERA_PLUGINS,
//                                      TRUE);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISABLE_CAMERA_PLUGINS): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN,
//  //                                    TRUE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//  //result_2 = attributes_p->SetUINT32 (MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS,
//  //                                    FALSE);
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUINT32(MF_SOURCE_READER_ENABLE_TRANSCODE_ONLY_TRANSFORMS): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  //result_2 = attributes_p->SetUnknown (MFT_FIELDOFUSE_UNLOCK_Attribute,
//  //                                     NULL); // IMFFieldOfUseMFTUnlock handle
//  //if (FAILED (result_2))
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IMFAttributes::SetUnknown(MFT_FIELDOFUSE_UNLOCK_Attribute): \"%s\", aborting\n"),
//  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//  //  goto error;
//  //} // end IF
//
//  IMFSourceReader* source_reader_p = NULL;
//  result_2 = MFCreateSourceReaderFromMediaSource (mediaSource_inout,
//                                                  attributes_p,
//                                                  &source_reader_p);
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateSourceReaderFromMediaSource(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  result_2 = source_reader_p->QueryInterface (IID_PPV_ARGS (&sourceReader_out));
//  if (FAILED (result_2))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::QueryInterface(IID_IMFSourceReaderEx): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//
//    // clean up
//    source_reader_p->Release ();
//
//    goto error;
//  } // end IF
//  source_reader_p->Release ();
//
//  result = true;
//
//error:
//  if (attributes_p)
//    attributes_p->Release ();
//
//  return result;
//}

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
bool
Stream_MediaFramework_MediaFoundation_Tools::getTopology (IMFMediaSession* mediaSession_in,
                                                          IMFTopology*& topology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getTopology"));

    // initialize return value(s)
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (mediaSession_in);

  enum MFSESSION_GETFULLTOPOLOGY_FLAGS flags =
    MFSESSION_GETFULLTOPOLOGY_CURRENT;
  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve the topology handle may fail (MF_E_INVALIDREQUEST)
  //         --> (try to) wait for the next MESessionTopologySet event
  // *NOTE*: this procedure doesn't always work as expected (GetFullTopology()
  //         still fails with MF_E_INVALIDREQUEST)
  HRESULT result = E_FAIL;
  do
  {
    result = mediaSession_in->GetFullTopology (flags,
                                               0,
                                               &topology_out);
  } while (result == MF_E_INVALIDREQUEST);
  if (FAILED (result)) // MF_E_INVALIDREQUEST: 0xC00D36B2L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::GetFullTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_out);

  return true;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (IMFMediaSession* mediaSession_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                             IMFMediaSourceEx*& mediaSource_out)
#else
                                                             IMFMediaSource*& mediaSource_out)
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getMediaSource"));

  // initialize return value(s)
  if (mediaSource_out)
  {
    mediaSource_out->Release (); mediaSource_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (mediaSession_in);

  IMFTopology* topology_p = NULL;
  if (Stream_MediaFramework_MediaFoundation_Tools::getTopology (mediaSession_in,
                                                                topology_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getTopology(), aborting\n")));
    return false;
  } // end IF
  ACE_ASSERT (topology_p);

  if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (topology_p,
                                                                    mediaSource_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), aborting\n")));
    topology_p->Release (); topology_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (mediaSource_out);
  topology_p->Release (); topology_p = NULL;

  return true;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

bool
Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (const IMFTopology* topology_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                             IMFMediaSourceEx*& mediaSource_out)
#else
                                                             IMFMediaSource*& mediaSource_out)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getMediaSource"));

  // initialize return value(s)
  if (mediaSource_out)
  {
    mediaSource_out->Release (); mediaSource_out = NULL;
  } // end IF

  // sanity check(s)
  ACE_ASSERT (topology_in);

  IMFCollection* collection_p = NULL;
  HRESULT result =
    const_cast<IMFTopology*> (topology_in)->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("media session topology has no source nodes, aborting\n")));
    collection_p->Release ();
    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release ();
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);
  unknown_p->Release (); unknown_p = NULL;
  result = topology_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&mediaSource_out));
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release (); topology_node_p = NULL;

  return true;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (REFGUID deviceIdentifier_in,
                                                             REFGUID deviceCategory_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                             IMFMediaSourceEx*& mediaSource_out)
#else
                                                             IMFMediaSource*& mediaSource_out)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getMediaSource"));

  bool result = false;

  if (mediaSource_out)
  {
    mediaSource_out->Release (); mediaSource_out = NULL;
  } // end IF

  IMFAttributes* attributes_p = NULL;
  UINT32 count = 0;
  IMFActivate** devices_pp = NULL;
  unsigned int index = 0;
  struct _GUID GUID_s = GUID_NULL;
  std::string device_identifier_string;

  device_identifier_string =
    Stream_MediaFramework_MediaFoundation_Tools::identifierToString (deviceIdentifier_in,
                                                                     deviceCategory_in);
  if (device_identifier_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::identifierToString(\"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID))
  {
    //GUID_s = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK;
    GUID_s = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID;
  } // end IF
  else if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))
    GUID_s = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s, aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  HRESULT result_2 = MFCreateAttributes (&attributes_p, 1);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return false;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result_2 =
    attributes_p->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           deviceCategory_in);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result_2 = MFEnumDeviceSources (attributes_p,
                                  &devices_pp,
                                  &count);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  ACE_ASSERT (devices_pp);
  if (count == 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("no capture devices found, aborting\n")));
    goto error;
  } // end IF

  if (!InlineIsEqualGUID (deviceIdentifier_in, GUID_NULL))
  {
    WCHAR buffer_a[BUFSIZ];
    UINT32 length;
    bool found = false;
    for (UINT32 i = 0; i < count; i++)
    {
      ACE_OS::memset (buffer_a, 0, sizeof (WCHAR[BUFSIZ]));
      length = 0;
      result_2 =
        devices_pp[index]->GetString (GUID_s,
                                      buffer_a,
                                      sizeof (WCHAR[BUFSIZ]),
                                      &length);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFActivate::GetString(%s): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error;
      } // end IF
      if (!ACE_OS::strcmp (buffer_a,
                           ACE_TEXT_ALWAYS_WCHAR (device_identifier_string.c_str ())))
      {
        found = true;
        index = i;
        break;
      } // end IF
    } // end FOR
    if (!found)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("media source (device identifier was: \"%s\") not found, aborting\n"),
                  ACE_TEXT (device_identifier_string.c_str ())));
      goto error;
    } // end IF
  } // end IF
  result_2 =
    devices_pp[index]->ActivateObject (IID_PPV_ARGS (&mediaSource_out));
  if (FAILED (result_2)) // MF_E_SHUTDOWN: 0xC00D3E85
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF

  result = true;

error:
  if (attributes_p)
    attributes_p->Release ();

  for (UINT32 i = 0; i < count; i++)
    devices_pp[i]->Release ();
  CoTaskMemFree (devices_pp);

  if (!result && mediaSource_out)
  {
    mediaSource_out->Release (); mediaSource_out = NULL;
  } // end IF

  return result;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId (const IMFTopology* topology_in,
                                                                     TOPOID& nodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getSampleGrabberNodeId"));

  // initialize return value(s)
  nodeId_out = 0;

  // sanity check(s)
  ACE_ASSERT (topology_in);

  IMFCollection* collection_p = NULL;
  HRESULT result =
    const_cast<IMFTopology*> (topology_in)->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_output_nodes = 0;
  result = collection_p->GetElementCount (&number_of_output_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_output_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("media session topology has no sink nodes, aborting\n")));
    collection_p->Release (); collection_p = NULL;
    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release (); unknown_p = NULL;
    return false;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;
  result = topology_node_p->GetTopoNodeID (&nodeId_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::GetTopoNodeID(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    topology_node_p->Release (); topology_node_p = NULL;
    return false;
  } // end IF
  topology_node_p->Release (); topology_node_p = NULL;

  return true;
}

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
bool
Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology (const std::string& URL_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                                 IMFMediaSourceEx*& mediaSource_inout,
#else
                                                                 IMFMediaSource*& mediaSource_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                                                 IMFTopology*& topology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology"));

  // initialize return value(s)
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

  bool release_source = false;
  IMFTopologyNode* topology_node_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL;
  TOPOID node_id = 0;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  IMFMediaType* media_type_p = NULL;
  IMFMediaSink* media_sink_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  IMFActivate* activate_p = NULL;

  HRESULT result = MFCreateTopology (&topology_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = topology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                    MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                    MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = topology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                    TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  if (!mediaSource_inout)
  {
//#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
//    DWORD flags = MF_RESOLUTION_MEDIASOURCE;
//    enum MF_OBJECT_TYPE object_type = MF_OBJECT_INVALID;
//    IUnknown* unknown_p = NULL;
//    IMFSourceResolver* source_resolver_p = NULL;
//    result = MFCreateSourceResolver (&source_resolver_p);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFCreateSourceResolver(): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      goto error;
//    } // end IF
//    result =
//      source_resolver_p->CreateObjectFromURL (ACE_TEXT_ALWAYS_WCHAR (URL_in.c_str ()),
//                                              flags,
//                                              NULL, // properties
//                                              &object_type,
//                                              &unknown_p);
//    if (FAILED (result)) // 0xc00d36c3: MF_E_UNSUPPORTED_SCHEME
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFSourceResolver::CreateObjectFromURL(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT (URL_in.c_str ()),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      PROPVARIANT prop_s;
//      PropVariantInit (&prop_s);
//      MFGetSupportedSchemes (&prop_s);
//      PropVariantClear (&prop_s);
//      source_resolver_p->Release (); source_resolver_p = NULL;
//      goto error;
//    } // end IF
//    ACE_ASSERT (unknown_p);
//    ACE_ASSERT (object_type = MF_OBJECT_MEDIASOURCE);
//    source_resolver_p->Release (); source_resolver_p = NULL;
//    result = unknown_p->QueryInterface (IID_PPV_ARGS (&mediaSource_inout));
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFMediaSource): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      unknown_p->Release (); unknown_p = NULL;
//      goto error;
//    } // end IF
//    unknown_p->Release (); unknown_p = NULL;
//    release_source = true;
//#else
    //result =
    //  CoCreateInstance (CLSID_ACEStream_MediaFramework_MF_MediaSource, NULL,
    //                    CLSCTX_INPROC_SERVER,
    //                    IID_PPV_ARGS (&mediaSource_inout));
    //if (FAILED (result))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
    //              ACE_TEXT (Common_Tools::GUIDToString (CLSID_ACEStream_MediaFramework_MF_MediaSource).c_str ()),
    //              ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    //  goto error;
    //} // end IF
    //ACE_ASSERT (mediaSource_inout);
//#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  } // end IF
  ACE_ASSERT (mediaSource_inout);
  result = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                        mediaSource_inout);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    mediaSource_inout->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  result = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                        stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

  result = topology_out->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added source node (id: %q)...\n"),
              node_id));

  topology_node_p->Release (); topology_node_p = NULL;

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (topology_node_2)
    topology_node_2->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (release_source)
  {
    mediaSource_inout->Release (); mediaSource_inout = NULL;
  } // end IF
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

  return false;
}

bool
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology (IMFMediaSourceEx* mediaSource_in,
#else
Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology (IMFMediaSource* mediaSource_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
                                                                 IMFTopology*& topology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::loadSourceTopology"));

  // initialize return value(s)
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

  IMFTopologyNode* topology_node_p = NULL;
  TOPOID node_id = 0;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = false;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  IMFMediaType* media_type_p = NULL;
  struct _GUID sub_type = GUID_NULL;

  HRESULT result = MFCreateTopology (&topology_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = topology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                    MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                    MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = topology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                    TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_SOURCESTREAM_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (mediaSource_in);
  result = topology_node_p->SetUnknown (MF_TOPONODE_SOURCE,
                                        mediaSource_in);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    mediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result =
    topology_node_p->SetUnknown (MF_TOPONODE_PRESENTATION_DESCRIPTOR,
                                 presentation_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  result = topology_node_p->SetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                        stream_descriptor_p);
  ACE_ASSERT (SUCCEEDED (result));

  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;
  result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_handler_p->Release (); media_type_handler_p = NULL;
  result = media_type_p->GetGUID (MF_MT_SUBTYPE,
                                  &sub_type);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_p->Release (); media_type_p = NULL;

  result = topology_out->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added source: \"%s\" -->...\n"),
              node_id,
              ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

  topology_node_p->Release (); topology_node_p = NULL;

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

  return false;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

bool
Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration (IMFTopology* topology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  // step1: find a(n output) node that supports the Direct3D manager (typically:
  //        EVR)
  IDirect3DDeviceManager9* device_manager_p = NULL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  HRESULT result = topology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_output_nodes = 0;
  result = collection_p->GetElementCount (&number_of_output_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  TOPOID node_id = 0;
  for (DWORD i = 0;
       i < number_of_output_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    topology_node_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      unknown_p->Release (); unknown_p = NULL;
      collection_p->Release (); collection_p = NULL;
      return false;
    } // end IF
    ACE_ASSERT (topology_node_p);
    unknown_p->Release (); unknown_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
    result = MFGetService (topology_node_p,
                           MR_VIDEO_ACCELERATION_SERVICE,
                           IID_PPV_ARGS (&device_manager_p));
    if (SUCCEEDED (result))
    {
      result = topology_node_p->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("node (id was: %q) supports MR_VIDEO_ACCELERATION_SERVICE...\n"),
                  node_id));
      break;
    } // end IF
#else
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (false);
    ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
    topology_node_p->Release (); topology_node_p = NULL;
  } // end FOR
  collection_p->Release (); collection_p = NULL;
  //topology_node_p->Release (); topology_node_p = NULL;
  if (!device_manager_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no topology node supports MR_VIDEO_ACCELERATION_SERVICE, aborting\n")));
    return false;
  } // end IF

  // step2: iterate over all transfrom nodes and activate DirectX acceleration
  WORD count = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  IMFTransform* transform_p = NULL;
  IMFAttributes* attributes_p = NULL;
  UINT32 is_Direct3D_aware = 0;
  ULONG_PTR pointer_p = reinterpret_cast<ULONG_PTR> (device_manager_p);
  result = topology_in->GetNodeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  for (WORD i = 0;
       i < count;
       ++i)
  {
    topology_node_p = NULL;
    result = topology_in->GetNode (i, &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->GetNodeType (&node_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_type != MF_TOPOLOGY_TRANSFORM_NODE)
    {
      topology_node_p->Release (); topology_node_p = NULL;
      continue;
    } // end IF

    unknown_p = NULL;
    result = topology_node_p->GetObject (&unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    transform_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      unknown_p->Release (); unknown_p = NULL;
      topology_node_p->Release (); topology_node_p = NULL;
      goto error;
    } // end IF
    unknown_p->Release (); unknown_p = NULL;

    result = transform_p->GetAttributes (&attributes_p);
    ACE_ASSERT (SUCCEEDED (result));
    is_Direct3D_aware = MFGetAttributeUINT32 (attributes_p,
                                              MF_SA_D3D_AWARE,
                                              FALSE);
    if (!is_Direct3D_aware)
    {
      // clean up
      attributes_p->Release (); attributes_p = NULL;
      transform_p->Release (); transform_p = NULL;
      topology_node_p->Release (); topology_node_p = NULL;
      continue;
    } // end IF
    attributes_p->Release (); attributes_p = NULL;

    result = transform_p->ProcessMessage (MFT_MESSAGE_SET_D3D_MANAGER,
                                          pointer_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::ProcessMessage(MFT_MESSAGE_SET_D3D_MANAGER): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      transform_p->Release (); transform_p = NULL;
      topology_node_p->Release (); topology_node_p = NULL;
      goto error;
    } // end IF
    transform_p->Release (); transform_p = NULL;

    result = topology_node_p->SetUINT32 (MF_TOPONODE_D3DAWARE, TRUE);
    ACE_ASSERT (SUCCEEDED (result));

    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("node (id was: %q) enabled MR_VIDEO_ACCELERATION_SERVICE...\n"),
                node_id));

    topology_node_p->Release (); topology_node_p = NULL;
  } // end FOR
  device_manager_p->Release (); device_manager_p = NULL;

  return true;

error:
  if (device_manager_p)
    device_manager_p->Release ();

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::addResampler (const IMFMediaType* mediaType_in,
                                                           IMFTopology* topology_in,
                                                           TOPOID& nodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::addResampler"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (!nodeId_out);

  IMFTransform* transform_p = NULL;
  HRESULT result = CoCreateInstance (CLSID_CResamplerMediaObject, NULL,
                                     CLSCTX_INPROC_SERVER,
                                     IID_PPV_ARGS (&transform_p));
  if (FAILED (result) || !transform_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_CResamplerMediaObject): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  IMFAttributes* attributes_p = NULL;
  result = transform_p->GetAttributes (&attributes_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetAttributes(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
  } // end IF
  if (attributes_p)
  {
    result = attributes_p->SetUINT32 (MF_TRANSFORM_ASYNC_UNLOCK, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    attributes_p->Release (); attributes_p = NULL;
  } // end IF

  IWMResamplerProps* resampler_properties_p = NULL;
  result = transform_p->QueryInterface (IID_PPV_ARGS (&resampler_properties_p));
  ACE_ASSERT (SUCCEEDED (result) && resampler_properties_p);
  result = resampler_properties_p->SetHalfFilterLength (60);
  ACE_ASSERT (SUCCEEDED (result));
  resampler_properties_p->Release (); resampler_properties_p = NULL;

  IMFTopologyNode* topology_node_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                 &topology_node_p);
  if (FAILED (result) || !topology_node_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetObject (transform_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_DECODER,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_DRAIN,
                                       MF_TOPONODE_DRAIN_ALWAYS);
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: {E139E0EE-F14D-4A53-BC1E-B805723E0105}
  result = topology_node_p->SetUINT32 (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{E139E0EE-F14D-4A53-BC1E-B805723E0105}")),
                                       0);
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: {4DB04908-0D94-47B3-933E-86BDAA16FA77}
  result = topology_node_p->SetUINT32 (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{4DB04908-0D94-47B3-933E-86BDAA16FA77}")),
                                       0);
  ACE_ASSERT (SUCCEEDED (result));
  // MF_TOPOLOGY_D3D_MANAGER: {66289BFB-1DF1-4951-A97A-D7BD1D03AC76}
  result = topology_node_p->SetUnknown (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{66289BFB-1DF1-4951-A97A-D7BD1D03AC76}")),
                                        NULL);
  ACE_ASSERT (SUCCEEDED (result));
  // MF_TOPONODE_SAMPLE_PROCESSING_TRACKER: {D81F457D-B4B6-4816-B27F-2C0EDF6AB303}
  result = topology_node_p->SetUnknown (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{D81F457D-B4B6-4816-B27F-2C0EDF6AB303}")),
                                        NULL);
  ACE_ASSERT (SUCCEEDED (result));

  result = topology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&nodeId_out);
  ACE_ASSERT (SUCCEEDED (result));

  if (!Stream_MediaFramework_MediaFoundation_Tools::append (topology_in,
                                                            nodeId_out,
                                                            true)) // set input format
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::append(%q), aborting\n"),
                nodeId_out));
    goto error;
  } // end IF

  if (mediaType_in)
  {
    result = transform_p->SetOutputType (0,
                                         const_cast<IMFMediaType*> (mediaType_in),
                                         0);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (transform_p).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      Stream_MediaFramework_MediaFoundation_Tools::dump (transform_p);
      goto error;
    } // end IF

    result =
      topology_node_p->SetOutputPrefType (0,
                                          const_cast<IMFMediaType*> (mediaType_in));
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added resampler: \"%s\"...\n"),
              nodeId_out,
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (transform_p).c_str ())));

  transform_p->Release (); transform_p = NULL;
  topology_node_p->Release (); topology_node_p = NULL;

  return true;

error:
  if (transform_p)
    transform_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (nodeId_out)
  {
    topology_in->GetNodeByID (nodeId_out,
                              &topology_node_p);
    topology_in->RemoveNode (topology_node_p);
    Stream_MediaFramework_MediaFoundation_Tools::shutdown (topology_node_p);
    topology_node_p->Release ();
    nodeId_out = 0;
  } // end IF

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::addGrabber (
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                         IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                         IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                         IMFTopology* topology_in,
                                                         TOPOID& nodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::addGrabber"));

  // sanity check(s)
  ACE_ASSERT (sampleGrabberSinkCallback_in);
  ACE_ASSERT (topology_in);
  ACE_ASSERT (!nodeId_out);

  // step1: create sample grabber sink
  IMFMediaType* media_type_p = NULL;
  HRESULT result = MFCreateMediaType (&media_type_p);
  ACE_ASSERT (SUCCEEDED (result) && media_type_p);
  result = media_type_p->SetGUID (MF_MT_MAJOR_TYPE,
                                  MFMediaType_Audio);
  ACE_ASSERT (SUCCEEDED (result));
  IMFActivate* activate_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,//const_cast<IMFMediaType*> (mediaType_in),
                                       sampleGrabberSinkCallback_in,
                                       &activate_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (FAILED (result) || !activate_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    media_type_p->Release ();
    return false;
  } // end IF
  media_type_p->Release (); media_type_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result) || !media_sink_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    activate_p->Release (); activate_p = NULL;
    return false;
  } // end IF
  activate_p->Release (); activate_p = NULL;

  // step2: add node to topology
  IMFTopologyNode* topology_node_p = NULL;
  IMFStreamSink* stream_sink_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
  media_sink_p->Release (); media_sink_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result) || !topology_node_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&nodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release (); topology_node_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::append (topology_in,
                                                            nodeId_out,
                                                            true)) // set input format
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::append(%q), aborting\n"),
                nodeId_out));
    goto error;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added sample grabber node...\n"),
              nodeId_out));

  return true;

error:
  if (stream_sink_p)
    stream_sink_p->Release ();
  if (media_sink_p)
    media_sink_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (nodeId_out)
  {
    topology_in->GetNodeByID (nodeId_out,
                              &topology_node_p);
    topology_in->RemoveNode (topology_node_p);
    Stream_MediaFramework_MediaFoundation_Tools::shutdown (topology_node_p);
    topology_node_p->Release ();
    nodeId_out = 0;
  } // end IF

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::addRenderer (REFGUID majorMediaType_in,
                                                          HWND windowHandle_in,
                                                          REFGUID deviceIdentifier_in,
                                                          IMFTopology* topology_in,
                                                          TOPOID& rendererNodeId_out,
                                                          bool setInputFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::addRenderer"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  // initialize return value(s)
  rendererNodeId_out = 0;

  IMFTopologyNode* topology_node_p = NULL;

  // step1: create renderer
  IMFActivate* activate_p = NULL;
  HRESULT result = S_OK;
  DWORD characteristics_i = 0;
  IMFMediaSink* media_sink_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  if (InlineIsEqualGUID (majorMediaType_in, MFMediaType_Audio))
  {
    if (InlineIsEqualGUID (deviceIdentifier_in, GUID_NULL))
    {
      IMFMediaType* media_type_p = NULL;
      if (!Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (topology_in,
                                                                         0,
                                                                         media_type_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat(), aborting\n")));
        return false;
      } // end IF
      ACE_ASSERT (media_type_p);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      IMFSampleGrabberSinkCallback2* sample_grabber_p = NULL;
#else
      IMFSampleGrabberSinkCallback* sample_grabber_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      ACE_NEW_NORETURN (sample_grabber_p,
                        Stream_MediaFramework_MediaFoundation_Null ());
      if (!sample_grabber_p)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("failed to allocate memory, aborting\n")));
        media_type_p->Release (); media_type_p = NULL;
        return false;
      } // end IF
      result = MFCreateSampleGrabberSinkActivate (media_type_p,
                                                  sample_grabber_p,
                                                  &activate_p);
      ACE_ASSERT (SUCCEEDED (result) && activate_p);
      media_type_p->Release (); media_type_p = NULL;
      goto continue_2;
    } // end IF

    IMFAttributes* attributes_p = NULL;
    result = MFCreateAttributes (&attributes_p, 4);
    ACE_ASSERT (SUCCEEDED (result) && attributes_p);
    if (!InlineIsEqualGUID (deviceIdentifier_in, GUID_NULL))
    {
      std::string device_string =
        Stream_MediaFramework_MediaFoundation_Tools::identifierToString (deviceIdentifier_in,
                                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
      result = attributes_p->SetString (MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ID,
                                        ACE_TEXT_ALWAYS_WCHAR (device_string.c_str ()));
      ACE_ASSERT (SUCCEEDED (result));
    } // end ELSE
    //result = attributes_p->SetUINT32 (MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ROLE,
    //                                  eMultimedia);
    //ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetUINT32 (MF_AUDIO_RENDERER_ATTRIBUTE_FLAGS,
                                      MF_AUDIO_RENDERER_ATTRIBUTE_FLAGS_NOPERSIST);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetGUID (MF_AUDIO_RENDERER_ATTRIBUTE_SESSION_ID,
                                    CLSID_ACEStream_MediaFramework_WASAPI_AudioSession);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetUINT32 (MF_AUDIO_RENDERER_ATTRIBUTE_STREAM_CATEGORY,
                                      AudioCategory_Media);
    ACE_ASSERT (SUCCEEDED (result));
    result = MFCreateAudioRenderer (attributes_p,
                                    &media_sink_p);
    ACE_ASSERT (SUCCEEDED (result) && media_sink_p);
    attributes_p->Release (); attributes_p = NULL;
    goto continue_;
    //result = MFCreateAudioRendererActivate (&activate_p);
  } // end IF
  else if (InlineIsEqualGUID (majorMediaType_in, MFMediaType_Video))
    result = MFCreateVideoRendererActivate (windowHandle_in,
                                            &activate_p);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown major media type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (majorMediaType_in).c_str ())));
    return false;
  } // end ELSE
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateXXXRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
continue_2:
  ACE_ASSERT (activate_p);

  //// *NOTE*: select a (custom) video presenter
  //result = activate_p->SetGUID (MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID,
  //                              );
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFActivate::SetGUID(MF_ACTIVATE_CUSTOM_VIDEO_PRESENTER_CLSID) \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  activate_p->Release (); activate_p = NULL;
  //  return false;
  //} // end IF
  if (InlineIsEqualGUID (majorMediaType_in, MFMediaType_Audio))
  {
    std::string device_string;

    if (InlineIsEqualGUID (deviceIdentifier_in, GUID_NULL))
      goto continue_3;

    device_string =
      Stream_MediaFramework_MediaFoundation_Tools::identifierToString (deviceIdentifier_in,
                                                                       MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    result = activate_p->SetString (MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ID,
                                    ACE_TEXT_ALWAYS_WCHAR (device_string.c_str ()));
    ACE_ASSERT (SUCCEEDED (result));
    //result = activate_p->SetUINT32 (MF_AUDIO_RENDERER_ATTRIBUTE_ENDPOINT_ROLE,
    //                                eMultimedia);
    //ACE_ASSERT (SUCCEEDED (result));
    result = activate_p->SetUINT32 (MF_AUDIO_RENDERER_ATTRIBUTE_FLAGS,
                                    MF_AUDIO_RENDERER_ATTRIBUTE_FLAGS_NOPERSIST);
    ACE_ASSERT (SUCCEEDED (result));
    //result = activate_p->SetGUID (MF_AUDIO_RENDERER_ATTRIBUTE_SESSION_ID,
    //                              GUID_NULL);
    //ACE_ASSERT (SUCCEEDED (result));
    result = activate_p->SetUINT32 (MF_AUDIO_RENDERER_ATTRIBUTE_STREAM_CATEGORY,
                                    AudioCategory_Media);
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF

continue_3:
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFActivate::ActivateObject(IID_IMFMediaSink) \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    activate_p->Release (); activate_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (media_sink_p);
  activate_p->Release (); activate_p = NULL;

continue_:
  IMFStreamSink* stream_sink_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  IMFPresentationTimeSource* presentation_time_source_p = NULL;

  IMFPresentationClock* presentation_clock_p = NULL;
  result = MFCreatePresentationClock (&presentation_clock_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreatePresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = MFCreateSystemTimeSource (&presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSystemTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_clock_p->Release (); presentation_clock_p = NULL;
    goto error;
  } // end IF
  result = presentation_clock_p->SetTimeSource (presentation_time_source_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::SetTimeSource(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_time_source_p->Release (); presentation_time_source_p = NULL;
    presentation_clock_p->Release (); presentation_clock_p = NULL;
    goto error;
  } // end IF
  presentation_time_source_p->Release (); presentation_time_source_p = NULL;
  result = media_sink_p->SetPresentationClock (presentation_clock_p);
  if (FAILED (result)) // MF_E_NOT_INITIALIZED: 0xC00D36B6L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSink::SetPresentationClock(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_clock_p->Release (); presentation_clock_p = NULL;
    goto error;
  } // end IF
  //result = presentation_clock_p->Start (0);
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IMFPresentationClock::Start(0): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  presentation_clock_p->Release (); presentation_clock_p = NULL;
  //  goto error;
  //} // end IF
  presentation_clock_p->Release (); presentation_clock_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = media_sink_p->GetCharacteristics (&characteristics_i);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("IMFMediaSink characteristics: 0x%x\n"),
              characteristics_i));
  //if (characteristics_i & MEDIASINK_CLOCK_REQUIRED)
  //{
    //result = media_sink_p->SetPresentationClock (NULL);
    //ACE_ASSERT (SUCCEEDED (result));
  //} // end IF

  // step2: add node to topology
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_p);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
  media_sink_p->Release (); media_sink_p = NULL;

  //IMFMediaEvent* media_event_p = NULL;
  //DWORD event_type = 0;
  //HRESULT status = E_FAIL;
  //do
  //{
  //  ACE_ASSERT (!media_event_p);
  //  result = stream_sink_p->GetEvent (MF_EVENT_FLAG_NO_WAIT, &media_event_p);
  //  if (FAILED (result))
  //  {
  //    ACE_ASSERT (result == MF_E_NO_EVENTS_AVAILABLE);
  //    break;
  //  } // end IF
  //  result = media_event_p->GetType (&event_type);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  switch (event_type)
  //  {
  //    case MEExtendedType:
  //    {
  //      struct _GUID GUID_s = GUID_NULL;
  //      result = media_event_p->GetExtendedType (&GUID_s);
  //      ACE_ASSERT (SUCCEEDED (result));
  //      // MF_MEEXT_SAR_AUDIO_ENDPOINT_CHANGED: {02E7187D-0087-437E-A27F-CF5ADCCD3112}
  //      break;
  //    }
  //    default:
  //      break;
  //  } // end SWITCH
  //  result = media_event_p->GetStatus (&status);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  ACE_DEBUG ((LM_DEBUG,
  //              ACE_TEXT ("popped stream sink event (type: \"%s\")...\n"),
  //              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (event_type).c_str ())));
  //  media_event_p->Release (); media_event_p = NULL;
  //} while (true);

  //Stream_MediaFramework_MediaFoundation_Tools::dump (stream_sink_p);
  //result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //result = media_type_handler_p->SetCurrentMediaType (media_type_p);
  //ACE_ASSERT (SUCCEEDED (result));
  //media_type_handler_p->Release (); media_type_handler_p = NULL;
  // *IMPORTANT NOTE*: topology loaders do not support sink activates (MF_E_TOPO_SINK_ACTIVATES_UNSUPPORTED)
  //                   --> bind stream sink manually
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_DISABLE_PREROLL, TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: {E139E0EE-F14D-4A53-BC1E-B805723E0105}
  result = topology_node_p->SetUINT32 (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{E139E0EE-F14D-4A53-BC1E-B805723E0105}")),
                                       0);
  ACE_ASSERT (SUCCEEDED (result));

  result = topology_in->AddNode (topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_p->GetTopoNodeID (&rendererNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added renderer node (id: %q)...\n"),
              rendererNodeId_out));
  topology_node_p->Release (); topology_node_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::append (topology_in,
                                                            rendererNodeId_out,
                                                            setInputFormat_in)) // set input format ?
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::append(%q), aborting\n"),
                rendererNodeId_out));
    goto error;
  } // end IF

  if (InlineIsEqualGUID (majorMediaType_in, MFMediaType_Video))
    if (!Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration (topology_in))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::enableDirectXAcceleration(), continuing\n")));

  return true;

error:
  if (stream_sink_p)
  {
    stream_sink_p->Release (); stream_sink_p = NULL;
  } // end IF
  if (media_sink_p)
  {
    media_sink_p->Release (); media_sink_p = NULL;
  } // end IF
  if (topology_node_p)
  {
    topology_node_p->Release (); topology_node_p = NULL;
  } // end IF
  if (rendererNodeId_out)
  {
    topology_node_p = NULL;
    result = topology_in->GetNodeByID (rendererNodeId_out,
                                       &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_in->RemoveNode (topology_node_p);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::RemoveNode(%q): \"%s\", continuing\n"),
                  rendererNodeId_out,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    rendererNodeId_out = 0;
    IUnknown* unknown_p = NULL;
    result = topology_node_p->GetObject (&unknown_p);
    ACE_ASSERT (SUCCEEDED (result) && unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
    ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
    unknown_p->Release (); unknown_p = NULL;
    result = stream_sink_p->GetMediaSink (&media_sink_p);
    ACE_ASSERT (SUCCEEDED (result) && media_sink_p);
    stream_sink_p->Release (); stream_sink_p = NULL;
    result = media_sink_p->Shutdown ();
    ACE_ASSERT (SUCCEEDED (result));
    media_sink_p->Release (); media_sink_p = NULL;
    result = topology_node_p->SetObject (NULL);
    ACE_ASSERT (SUCCEEDED (result));
    topology_node_p->Release (); topology_node_p = NULL;
  } // end IF

  return false;
}

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
bool
Stream_MediaFramework_MediaFoundation_Tools::setTopology (IMFTopology* topology_in,
                                                          IMFMediaSession*& mediaSession_inout,
                                                          bool isPartial_in,
                                                          bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::setTopology"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  HRESULT result = E_FAIL;
  bool release_media_session = false;
  IMFTopoLoader* topology_loader_p = NULL;
  IMFTopology* topology_p = NULL;
  // *WARNING*: do NOT set the MFSESSION_SETTOPOLOGY_CLEAR_CURRENT flag here
  //            --> if the flag is passed in, the new topology will NOT be set
  DWORD topology_flags = MFSESSION_SETTOPOLOGY_IMMEDIATE   |
                         MFSESSION_SETTOPOLOGY_NORESOLUTION;
  if (isPartial_in)
    topology_flags &= ~MFSESSION_SETTOPOLOGY_NORESOLUTION;
  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_event = false;
  MediaEventType event_type = MEUnknown;
  ACE_Time_Value timeout (STREAM_LIB_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT_S, 0);
  UINT32 value_i = 0;

  // initialize return value(s)
  if (!mediaSession_inout)
  {
    IMFAttributes* attributes_p = NULL;
    result = MFCreateAttributes (&attributes_p, 4);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    result = attributes_p->SetUINT32 (MF_SESSION_GLOBAL_TIME, FALSE);
    ACE_ASSERT (SUCCEEDED (result));
    result = attributes_p->SetGUID (MF_SESSION_QUALITY_MANAGER, GUID_NULL);
    ACE_ASSERT (SUCCEEDED (result));
    //result = attributes_p->SetGUID (MF_SESSION_TOPOLOADER, );
    //ACE_ASSERT (SUCCEEDED (result));
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
    result = attributes_p->SetUINT32 (MF_LOW_LATENCY, TRUE);
    ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
    // IID_IMFTelemetrySession: {627D2CA6-E1CD-4898-999D-101308F1D431}
    //result = attributes_p->SetUnknown (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{627D2CA6-E1CD-4898-999D-101308F1D431}")),
    //                                   NULL);
    //ACE_ASSERT (SUCCEEDED (result));
    // MF_TELEMETRY_SESSION_OBJECT_ATTRIBUTE: {2ACF1917-3743-41DF-A564-E727A80EA33D}
    result = attributes_p->SetGUID (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{2ACF1917-3743-41DF-A564-E727A80EA33D}")),
                                    GUID_NULL);
    ACE_ASSERT (SUCCEEDED (result));
    // *TODO*: {B1BEA77F-99BA-4AF5-AF20-76D209550E73}
    result = attributes_p->SetUINT32 (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{B1BEA77F-99BA-4AF5-AF20-76D209550E73}")),
                                      0);
    ACE_ASSERT (SUCCEEDED (result));
    //// *TODO*: {8B1034CF-AE42-4A7C-A0A6-BFD7EE052CCB}
    //result = attributes_p->SetBlob (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{8B1034CF-AE42-4A7C-A0A6-BFD7EE052CCB}")),
    //                                NULL,
    //                                0);
    //ACE_ASSERT (SUCCEEDED (result));
    // MF_LOW_LATENCY: {9C27891A-ED7A-40E1-88E8-B22727A024EE}
    result = attributes_p->SetUINT32 (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{9C27891A-ED7A-40E1-88E8-B22727A024EE}")),
                                      TRUE);
    ACE_ASSERT (SUCCEEDED (result));
    // MF_SESSION_PREROLL_FROM_RATE0: {09A7FF3E-F62A-465E-B1F9-E63D190B1A10}
    result = attributes_p->SetUINT32 (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{09A7FF3E-F62A-465E-B1F9-E63D190B1A10}")),
                                      TRUE);
    ACE_ASSERT (SUCCEEDED (result));

    result = MFCreateMediaSession (attributes_p,
                                   &mediaSession_inout);
    if (FAILED (result)) // MF_E_SHUTDOWN: 0xC00D3E85L
    {                    // DISCARDED:     0x8007009d
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaSession(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      attributes_p->Release (); attributes_p = NULL;
      goto error;
    } // end IF
    attributes_p->Release (); attributes_p = NULL;
    release_media_session = true;
  } // end IF
  else
  {
    if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_inout,
                                                             waitForCompletion_in))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (mediaSession_inout);

  if (!isPartial_in)
  {
    topology_p = topology_in;
    topology_p->AddRef ();
    goto continue_;
  } // end IF
  result = MFCreateTopoLoader (&topology_loader_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopoLoader(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_loader_p->Load (topology_in,
                                    &topology_p,
                                    NULL);
  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE               : 0xC00D36B4
  {                    // MF_E_NO_MORE_TYPES                  : 0xC00D36B9
                       // MF_E_TOPO_SINK_ACTIVATES_UNSUPPORTED: 0xC00D521B
                       // MF_E_TOPO_CODEC_NOT_FOUND           : 0xC00D5212
                       // MF_E_TOPO_UNSUPPORTED               : 0xC00D5214
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopoLoader::Load(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    Stream_MediaFramework_MediaFoundation_Tools::dump (topology_in);
    goto error;
  } // end IF
  topology_loader_p->Release (); topology_loader_p = NULL;
continue_:
  ACE_ASSERT (topology_p);

  Stream_MediaFramework_MediaFoundation_Tools::dump (topology_p);

  result = mediaSession_inout->SetTopology (topology_flags,
                                            topology_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous; subsequent calls
  //         to retrieve a topology handle will fail (MF_E_INVALIDREQUEST)
  //         --> wait until it finishes ?
  if (!waitForCompletion_in)
    goto continue_2;

  timeout += COMMON_TIME_NOW;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = mediaSession_inout->GetEvent (0,
                                           &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((((result == MF_E_MULTIPLE_SUBSCRIBERS) ? LM_WARNING : LM_ERROR),
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", %s\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ()),
                  ((result == MF_E_MULTIPLE_SUBSCRIBERS) ? ACE_TEXT ("continuing") : ACE_TEXT ("aborting"))));
      if (result != MF_E_MULTIPLE_SUBSCRIBERS) // 0xc00d36da
        goto error;
      received_topology_event = true;
      continue;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
    {
      HRESULT status = E_FAIL;
      result = media_event_p->GetStatus (&status);
      ACE_ASSERT (SUCCEEDED (result));
      if (FAILED (status))
      { // MF_E_STREAMSINK_REMOVED: 0xC00D4A38
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::SetTopology(): \"%s\"), aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (status).c_str ())));
        goto error;
      } // end IF
      received_topology_event = true;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("popped media session event (type: \"%s\")...\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (event_type).c_str ())));
    media_event_p->Release ();
  } while (!received_topology_event &&
           (COMMON_TIME_NOW < timeout));
  if (!received_topology_event)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): timed out, continuing\n")));

  received_topology_event = false;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = mediaSession_inout->GetEvent (0,
                                           &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((((result == MF_E_MULTIPLE_SUBSCRIBERS) ? LM_WARNING : LM_ERROR),
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", %s\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ()),
                  ((result == MF_E_MULTIPLE_SUBSCRIBERS) ? ACE_TEXT ("continuing") : ACE_TEXT ("aborting"))));
      if (result != MF_E_MULTIPLE_SUBSCRIBERS) // 0xc00d36da
        goto error;
      received_topology_event = true;
      continue;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologyStatus)
    {
      UINT32 attribute_value = 0;
      MF_TOPOSTATUS topology_status = MF_TOPOSTATUS_INVALID;
      result = media_event_p->GetUINT32 (MF_EVENT_TOPOLOGY_STATUS,
                                         &attribute_value);
      ACE_ASSERT (SUCCEEDED (result));
      topology_status = static_cast<MF_TOPOSTATUS> (attribute_value);
      if (topology_status == MF_TOPOSTATUS_READY)
        received_topology_event = true;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("popped media session event (type: \"%s\")...\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (event_type).c_str ())));
    media_event_p->Release ();
  } while (!received_topology_event &&
           (COMMON_TIME_NOW < timeout));
  if (!received_topology_event)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): timed out, continuing\n")));

  result = topology_p->GetUINT32 (MF_TOPOLOGY_RESOLUTION_STATUS,
                                  &value_i);
  if (SUCCEEDED (result))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology resolution status: 0x%x\n"),
                value_i));

continue_2:
  topology_p->Release (); topology_p = NULL;
  return true;

error:
  if (topology_loader_p)
    topology_loader_p->Release ();
  if (topology_p)
    topology_p->Release ();
  if (release_media_session)
  { 
    mediaSession_inout->Release (); mediaSession_inout = NULL;
  } // end IF

  return false;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

bool
Stream_MediaFramework_MediaFoundation_Tools::append (IMFTopology* topology_in,
                                                     TOPOID nodeId_in,
                                                     bool setInputFormat_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::append"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (nodeId_in);

  // step0: retrieve node handle
  bool add_tee_node_b = false, is_tee_node_b = false;
  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL; // source/output node
  IMFTopologyNode* topology_node_3 = NULL; // upstream node
  IMFTopologyNode* topology_node_4 = NULL; // upstream (tee) node (if any)
  IMFMediaType* media_type_p = NULL;
  IUnknown* unknown_p = NULL;
  TOPOID node_id = 0, node_id_2 = 0;
  DWORD output_index_i = 0;

  result = topology_in->GetNodeByID (nodeId_in,
                                     &topology_node_p);
  if (unlikely (FAILED (result)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                nodeId_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);

  // step1: parse topology
  TOPOLOGY_PATHS_T paths_s;
  if (unlikely (!Stream_MediaFramework_MediaFoundation_Tools::parse (topology_in,
                                                                     paths_s)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::parse(), aborting\n")));
    topology_node_p->Release ();
    return false;
  } // end IF
  if (unlikely (paths_s.empty ()))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));
    topology_node_p->Release ();
    return false;
  } // end IF

  // test rule 1: does branch i have a sink ? --> continue : append
  TOPOLOGY_PATH_ITERATOR_T iterator_2;
  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = paths_s.begin ();
       iterator != paths_s.end ();
       ++iterator)
  {
    iterator_2 = (*iterator).begin ();
    std::advance (iterator_2, (*iterator).size () - 1);
    result = (*iterator_2)->GetNodeType (&node_type_e);
    ACE_ASSERT (SUCCEEDED (result));
    switch (node_type_e)
    {
      case MF_TOPOLOGY_OUTPUT_NODE:
        break; // continue
      case MF_TOPOLOGY_SOURCESTREAM_NODE:
      case MF_TOPOLOGY_TRANSFORM_NODE:
      case MF_TOPOLOGY_TEE_NODE:
      {
        topology_node_2 = *(*iterator).begin ();
        topology_node_2->AddRef ();
        topology_node_3 = *iterator_2;
        topology_node_3->AddRef ();
        goto continue_; // append
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                    node_type_e));
        goto error;
      }
    } // end SWITCH
  } // end FOR

  // test rule 2: does branch i have a tee ? --> append : continue
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = paths_s.begin ();
       iterator != paths_s.end ();
       ++iterator)
    if (Stream_MediaFramework_MediaFoundation_Tools::hasTee (*iterator,
                                                             topology_node_3))
    {
      topology_node_2 = *(*iterator).begin ();
      topology_node_2->AddRef ();
      is_tee_node_b = true;
      goto continue_; // append
    } // end IF

  // --> use rule 3: tee upstream node of first sink
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = paths_s.begin ();
       iterator != paths_s.end ();
       ++iterator)
  {
    iterator_2 = (*iterator).begin ();
    std::advance (iterator_2, (*iterator).size () - 1);
    result = (*iterator_2)->GetNodeType (&node_type_e);
    ACE_ASSERT (SUCCEEDED (result));
    switch (node_type_e)
    {
      case MF_TOPOLOGY_OUTPUT_NODE:
      {
        // *NOTE*: in this case, topology_node_2 points to the current sink
        topology_node_2 = *iterator_2;
        topology_node_2->AddRef ();
        topology_node_3 = *(--iterator_2);
        topology_node_3->AddRef ();
        add_tee_node_b = true;
        is_tee_node_b = true;
        goto continue_; // append
      }
      case MF_TOPOLOGY_SOURCESTREAM_NODE:
      case MF_TOPOLOGY_TRANSFORM_NODE:
      case MF_TOPOLOGY_TEE_NODE:
        break; // continue
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                    node_type_e));
        goto error;
      }
    } // end SWITCH
  } // end FOR
  ACE_ASSERT (false);
  ACE_NOTREACHED (return false;)

continue_:
  ACE_ASSERT (topology_node_p);
  ACE_ASSERT (topology_node_2);
  ACE_ASSERT (topology_node_3);

  // step2: retrieve media type of upstream node
  result = topology_node_3->GetNodeType (&node_type_e);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type_e)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_3->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result) && stream_descriptor_p);
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
      stream_descriptor_p->Release (); stream_descriptor_p = NULL;
      result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
      ACE_ASSERT (SUCCEEDED (result) && media_type_p);
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      result = topology_node_3->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        goto error;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
continue_2:
      result = transform_p->GetOutputCurrentType (0,
                                                  &media_type_p);
      if (FAILED (result))
      {
        if (result == MF_E_TRANSFORM_TYPE_NOT_SET) // 0xc00d6d60
        {
          result = topology_node_3->GetTopoNodeID (&node_id_2);
          ACE_ASSERT (SUCCEEDED (result) && node_id_2);
          ACE_DEBUG ((LM_WARNING,
                      ACE_TEXT ("transform (node id: %q) output #0 type not set; trying input type, continuing\n"),
                      node_id_2));
          IMFMediaType* media_type_2 = NULL;
          if (!Stream_MediaFramework_MediaFoundation_Tools::getInputFormat (topology_in,
                                                                            node_id_2,
                                                                            media_type_2))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getInputFormat(), aborting\n")));
            transform_p->Release (); transform_p = NULL;
            goto error;
          } // end IF
          ACE_ASSERT (media_type_2);
          result = transform_p->SetOutputType (0,
                                               media_type_2,
                                               0);
          ACE_ASSERT (SUCCEEDED (result));
          media_type_2->Release (); media_type_2 = NULL;
          goto continue_2;
        } // end IF
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
      transform_p->Release (); transform_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_3->GetOutputPrefType (0,
                                                   &media_type_p);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type_e));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (media_type_p);

  // step2: add a tee node ?
  if (!add_tee_node_b)
  {
    topology_node_4 = topology_node_3;
    topology_node_3 = NULL;
    goto continue_3;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_4);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_in->AddNode (topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  result = topology_node_4->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added tee node...\n"),
              node_id));

  // step2a: connect the upstream node to the tee
  result = topology_node_3->GetTopoNodeID (&node_id_2);
  ACE_ASSERT (SUCCEEDED (result) && node_id_2);
  result = topology_node_3->ConnectOutput (0,
                                           topology_node_4,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_node_3->Release (); topology_node_3 = NULL;
  result = topology_node_4->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (0,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (1,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q >>> %q...\n"),
              node_id_2, node_id));

  // step2b: connect the downstream (sink) node to the tee
  result = topology_node_2->GetTopoNodeID (&node_id_2);
  ACE_ASSERT (SUCCEEDED (result) && node_id_2);
  result = topology_node_4->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  //result = topology_node_2->SetInputPrefType (0,
  //                                            media_type_p);
  //ACE_ASSERT (SUCCEEDED (result));
  topology_node_2->Release (); topology_node_2 = NULL;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q >>> %q...\n"),
              node_id, node_id_2));

continue_3:
  if (is_tee_node_b)
  {
    result = topology_node_4->GetOutputCount (&output_index_i);
    ACE_ASSERT (SUCCEEDED (result));
    --output_index_i;
  } // end IF
  result = topology_node_4->ConnectOutput (output_index_i,
                                           topology_node_p,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(%u): \"%s\", aborting\n"),
                output_index_i,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF

  if (setInputFormat_in &&
      !Stream_MediaFramework_MediaFoundation_Tools::setInputFormat (topology_in,
                                                                    nodeId_in,
                                                                    media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setInputFormat(%q,%s), aborting\n"),
                nodeId_in,
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_p).c_str ())));
    goto error;
  } // end IF

  result = topology_node_4->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q >>> %q...\n"),
              node_id, nodeId_in));

  media_type_p->Release (); media_type_p = NULL;
  topology_node_p->Release (); topology_node_p = NULL;
  if (topology_node_2)
  {
    topology_node_2->Release (); topology_node_2 = NULL;
  } // end IF
  topology_node_4->Release (); topology_node_4 = NULL;
  Stream_MediaFramework_MediaFoundation_Tools::clean (paths_s);

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (topology_node_2)
    topology_node_2->Release ();
  if (topology_node_3)
    topology_node_3->Release ();
  if (topology_node_4)
    topology_node_4->Release ();
  Stream_MediaFramework_MediaFoundation_Tools::clean (paths_s);

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::clear (IMFMediaSession* mediaSession_in,
                                                    bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::clear"));

  ACE_Time_Value timeout (STREAM_LIB_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT_S, 0);
  ACE_Time_Value deadline;
  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_event = false;
  MediaEventType event_type = MEUnknown;

  // *NOTE*: this method is asynchronous
  //         --> wait for MESessionTopologiesCleared
  HRESULT result = mediaSession_in->ClearTopologies ();
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::ClearTopologies(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  if (!waitForCompletion_in)
    goto continue_;

  deadline = COMMON_TIME_NOW + timeout;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = mediaSession_in->GetEvent (0,
                                        &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((((result == MF_E_MULTIPLE_SUBSCRIBERS) ? LM_WARNING : LM_ERROR),
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", %s\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ()),
                  ((result == MF_E_MULTIPLE_SUBSCRIBERS) ? ACE_TEXT ("continuing") : ACE_TEXT ("aborting"))));
      if (result != MF_E_MULTIPLE_SUBSCRIBERS) // 0xc00d36da
        return false;
      received_topology_event = true;
      continue;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologiesCleared)
      received_topology_event = true;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("popped media session event (type: \"%s\")...\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (event_type).c_str ())));
    media_event_p->Release ();
  } while (!received_topology_event &&
           (COMMON_TIME_NOW < deadline));
  if (!received_topology_event)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to IMFMediaSession::ClearTopologies(): timed out, continuing\n")));

continue_:
  DWORD topology_flags = MFSESSION_SETTOPOLOGY_CLEAR_CURRENT;
  result = mediaSession_in->SetTopology (topology_flags,
                                         NULL);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(MFSESSION_SETTOPOLOGY_CLEAR_CURRENT): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF

  // *NOTE*: IMFMediaSession::SetTopology() is asynchronous
  //         --> wait for the next MESessionTopologySet
  if (!waitForCompletion_in)
    return true;

  //deadline = COMMON_TIME_NOW + timeout;
  received_topology_event = false;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = mediaSession_in->GetEvent (0,
                                        &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((((result == MF_E_MULTIPLE_SUBSCRIBERS) ? LM_WARNING : LM_ERROR),
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", %s\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ()),
                  ((result == MF_E_MULTIPLE_SUBSCRIBERS) ? ACE_TEXT ("continuing") : ACE_TEXT ("aborting"))));
      if (result != MF_E_MULTIPLE_SUBSCRIBERS) // 0xc00d36da
        return false;
      received_topology_event = true;
      continue;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
      received_topology_event = true;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("popped media session event (type: \"%s\")...\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (event_type).c_str ())));
    media_event_p->Release ();
  } while (!received_topology_event &&
           (COMMON_TIME_NOW < deadline));
  if (!received_topology_event)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to IMFMediaSession::SetTopology(): timed out, continuing\n")));

  return true;
}

void
Stream_MediaFramework_MediaFoundation_Tools::shutdown (IMFMediaSession* mediaSession_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::shutdown"));

  // sanity check(s)
  ACE_ASSERT (mediaSession_in);

  // step1: stop session
  HRESULT result = mediaSession_in->Stop ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::Stop(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

  // step2: tear down topology/ies
  IMFTopology* topology_p = NULL;
  if (!Stream_MediaFramework_MediaFoundation_Tools::getTopology (mediaSession_in,
                                                                 topology_p))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getTopology(), continuing\n")));
  if (!Stream_MediaFramework_MediaFoundation_Tools::clear (mediaSession_in,
                                                           true)) // wait for completion
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), continuing\n")));
  if (topology_p &&
      !Stream_MediaFramework_MediaFoundation_Tools::clear (topology_p,
                                                           true)) // shutdown
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::clear(), continuing\n")));
  if (topology_p)
  {
    topology_p->Release (); topology_p = NULL;
  } // end IF

  // step3: tear down session
  result = mediaSession_in->Close ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::Close(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  result = mediaSession_in->Shutdown ();
  if (FAILED (result))
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
}

bool
Stream_MediaFramework_MediaFoundation_Tools::clear (IMFTopology* topology_in,
                                                    bool shutdown_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::clear"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  HRESULT result = E_FAIL;
  WORD number_of_nodes_i = 0;
retry:
  result = topology_in->GetNodeCount (&number_of_nodes_i);
  ACE_ASSERT (SUCCEEDED (result));
  IMFTopologyNode* topology_node_p = NULL;
  TOPOID node_id = 0;
  for (WORD i = 0;
       i < number_of_nodes_i;
       ++i)
  { ACE_ASSERT (!topology_node_p);
    result = topology_in->GetNode (i, &topology_node_p);
    if (FAILED (result) && (result == MF_E_INVALIDINDEX))
      goto retry; // *NOTE*: apparently, removing a node reindexes the remaining ones
    ACE_ASSERT (topology_node_p);
    result = topology_in->RemoveNode (topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_node_p->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    if (shutdown_in)
      Stream_MediaFramework_MediaFoundation_Tools::shutdown (topology_node_p);
    topology_node_p->Release (); topology_node_p = NULL;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed node (id was: %q)...\n"),
                node_id));
  } // end FOR

  return true;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::clearTransforms (IMFTopology* topology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::clearTransforms"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  HRESULT result = E_FAIL;
  IMFCollection* collection_p = NULL;
  result =
    topology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    collection_p->Release (); collection_p = NULL;
    result = topology_in->Clear ();
    ACE_ASSERT (SUCCEEDED (result));
    return true;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  IUnknown* unknown_p = NULL;
  DWORD number_of_outputs = 0;
  DWORD input_index = 0;
  IMFTopologyNode* topology_node_2 = NULL;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  TOPOID node_id = 0;
  DWORD number_of_outputs_2 = 0;
  IMFTopologyNode* topology_node_3 = NULL;
  for (DWORD i = 0;
       i < number_of_source_nodes;
       ++i)
  {
    unknown_p = NULL;
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      collection_p->Release (); collection_p = NULL;
      unknown_p->Release (); unknown_p = NULL;
      return false;
    } // end IF
    unknown_p->Release (); unknown_p = NULL;

    number_of_outputs = 0;
    result = topology_node_p->GetOutputCount (&number_of_outputs);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_outputs <= 0)
    {
      topology_node_p->Release (); topology_node_p = NULL;
      continue;
    } // end IF
    for (DWORD j = 0;
         j < number_of_outputs;
         ++j)
    {
      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (j,
                                           &topology_node_2,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_node_2->GetNodeType (&node_type);
      ACE_ASSERT (SUCCEEDED (result));
      if (node_type != MF_TOPOLOGY_TRANSFORM_NODE)
      {
        topology_node_p->Release (); topology_node_p = NULL;
        topology_node_2->Release (); topology_node_2 = NULL;
        continue;
      } // end IF

      number_of_outputs_2 = 0;
      result = topology_node_2->GetOutputCount (&number_of_outputs_2);
      ACE_ASSERT (SUCCEEDED (result));
      for (DWORD k = 0;
           k < number_of_outputs_2;
           ++k)
      {
        topology_node_3 = NULL;
        result = topology_node_2->GetOutput (k,
                                             &topology_node_3,
                                             &input_index);
        ACE_ASSERT (SUCCEEDED (result));
        result = topology_node_3->GetTopoNodeID (&node_id);
        ACE_ASSERT (SUCCEEDED (result));
        result = topology_in->RemoveNode (topology_node_3);
        if (FAILED (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFTopology::RemoveNode(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
          topology_node_3->Release (); topology_node_3 = NULL;
          topology_node_2->Release (); topology_node_2 = NULL;
          topology_node_p->Release (); topology_node_p = NULL;
          collection_p->Release (); collection_p = NULL;
          return false;
        } // end IF
        result = topology_node_3->SetObject (NULL);
        ACE_ASSERT (SUCCEEDED (result));
        topology_node_3->Release (); topology_node_3 = NULL;
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("removed node (id was: %q)...\n"),
                    node_id));
      } // end FOR
      result = topology_node_2->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_in->RemoveNode (topology_node_2);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopology::RemoveNode(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        topology_node_2->Release (); topology_node_2 = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        collection_p->Release (); collection_p = NULL;
        return false;
      } // end IF
      result = topology_node_2->SetObject (NULL);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_2->Release (); topology_node_2 = NULL;
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("removed transform node (id was: %q)...\n"),
                  node_id));
    } // end FOR
    topology_node_p->Release (); topology_node_p = NULL;
  } // end FOR
  collection_p->Release (); collection_p = NULL;

  return true;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::disconnect (IMFTopologyNode* IMFTopologyNode_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::disconnect"));

  // sanity check(s)
  ACE_ASSERT (IMFTopologyNode_in);

  DWORD number_of_outputs = 0;
  HRESULT result = IMFTopologyNode_in->GetOutputCount (&number_of_outputs);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::GetOutputCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index = 0;
  for (DWORD i = 0;
       i < number_of_outputs;
       ++i)
  {
    result = IMFTopologyNode_in->GetOutput (i,
                                            &topology_node_p,
                                            &input_index);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::GetOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF

    result = IMFTopologyNode_in->DisconnectOutput (i);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopologyNode::DisconnectOutput(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    } // end IF

    if (Stream_MediaFramework_MediaFoundation_Tools::disconnect (topology_node_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::disconnect(), aborting\n")));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    } // end IF
    topology_node_p->Release (); topology_node_p = NULL;
  } // end FOR

  return true;
}

void
Stream_MediaFramework_MediaFoundation_Tools::shutdown (IMFTopologyNode* topologyNode_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::shutdown"));

  // sanity check(s)
  ACE_ASSERT (topologyNode_in);

  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  HRESULT result = topologyNode_in->GetNodeType (&node_type_e);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type_e)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      IUnknown* unknown_p = NULL;
      result = topologyNode_in->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFStreamSink* stream_sink_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
      ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
      unknown_p->Release (); unknown_p = NULL;
      IMFMediaSink* media_sink_p = NULL;
      result = stream_sink_p->GetMediaSink (&media_sink_p);
      if (FAILED (result) || !media_sink_p)
      { // MF_E_STREAMSINK_REMOVED: 0xc00d4a38
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFStreamSink::GetMediaSink(): \"%s\"; cannot shutdown, continuing\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        stream_sink_p->Release (); stream_sink_p = NULL;
        break;
      } // end IF
      result = media_sink_p->Shutdown ();
      ACE_ASSERT (SUCCEEDED (result));
      media_sink_p->Release (); media_sink_p = NULL;
      stream_sink_p->Release (); stream_sink_p = NULL;
      break;
    }
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      IMFMediaSource* media_source_p = NULL;
      result = topologyNode_in->GetUnknown (MF_TOPONODE_SOURCE,
                                            IID_PPV_ARGS (&media_source_p));
      ACE_ASSERT (SUCCEEDED (result) && media_source_p);
      result = media_source_p->Shutdown ();
      ACE_ASSERT (SUCCEEDED (result));
      RELEASE_COM_OBJECT(media_source_p); media_source_p = NULL;
      break;
    }
    default:
      break;
  } // end SWITCH
  result = topologyNode_in->SetObject (NULL);
  ACE_ASSERT (SUCCEEDED (result));
}

bool
Stream_MediaFramework_MediaFoundation_Tools::reset (IMFTopology* topology_in,
                                                    REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::reset"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  struct _GUID GUID_s = GUID_NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID))
    //GUID_s = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK;
    GUID_s = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_ENDPOINT_ID;
  else if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))
    GUID_s = MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK;
  else if (InlineIsEqualGUID (deviceCategory_in, GUID_NULL))
    ;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s, aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (GUID_s).c_str ())));
    return false;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  // remove all but the first source node
  // *TODO*: identify the correct source node by category
  IMFCollection* collection_p = NULL;
  HRESULT result = topology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology has no source nodes, aborting\n")));
    collection_p->Release ();
    return false;
  } // end IF
  if (number_of_source_nodes > 1)
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("topology has several source nodes, continuing\n")));
  IMFTopologyNode* topology_node_p = NULL, *topology_node_2 = NULL;
  IUnknown* unknown_p = NULL;
  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release (); collection_p = NULL;
  ACE_ASSERT (unknown_p);
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release ();
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);
  unknown_p->Release (); unknown_p = NULL;
  TOPOID node_id = 0, node_id_2 = 0;
  result = topology_node_p->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release (); topology_node_p = NULL;

  // remove/shutdown all other nodes
  WORD number_of_nodes_i = 0;
retry:
  result = topology_in->GetNodeCount (&number_of_nodes_i);
  ACE_ASSERT (SUCCEEDED (result));
  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  for (WORD i = 0;
       i < number_of_nodes_i;
       ++i)
  { ACE_ASSERT (!topology_node_2);
    result = topology_in->GetNode (i, &topology_node_2);
    if (FAILED (result) && (result == MF_E_INVALIDINDEX))
      goto retry; // *TODO*: why does this happen ?
    ACE_ASSERT (topology_node_2);
    result = topology_node_2->GetTopoNodeID (&node_id_2);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_id == node_id_2)
    {
      topology_node_2->Release (); topology_node_2 = NULL;
      continue;
    } // end IF
    result = topology_in->RemoveNode (topology_node_2);
    ACE_ASSERT (SUCCEEDED (result));
    Stream_MediaFramework_MediaFoundation_Tools::shutdown (topology_node_2);
    topology_node_2->Release (); topology_node_2 = NULL;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed node (id was: %q)...\n"),
                node_id_2));
  } // end FOR

  return true;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::getInputFormat (IMFTopology* topology_in,
                                                             TOPOID nodeId_in,
                                                             IMFMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getInputFormat"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (nodeId_in);
  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  // step0: retrieve node handle
  IMFTopologyNode* topology_node_p = NULL;
  HRESULT result = topology_in->GetNodeByID (nodeId_in,
                                             &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                nodeId_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);

  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  result = topology_node_p->GetNodeType (&node_type_e);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type_e)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("source nodes do not have inputs (id was: %q), aborting\n"),
                  nodeId_in));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      IUnknown* unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        return false;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
      result = transform_p->GetInputCurrentType (0,
                                                 &mediaType_out);
      if (FAILED (result))
      {
        if (result == MF_E_TRANSFORM_TYPE_NOT_SET) // 0xC00D6D60
        { // get first available media type instead
          result = transform_p->GetInputAvailableType (0,
                                                       0,
                                                       &mediaType_out);
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMFTransform::GetInputAvailableType(): \"%s\", aborting\n"),
                      ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (transform_p).c_str ()),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
          transform_p->Release (); transform_p = NULL;
          topology_node_p->Release (); topology_node_p = NULL;
          return false;
        } // end ELSE
      } // end IF
      ACE_ASSERT (SUCCEEDED (result) && mediaType_out);
      transform_p->Release (); transform_p = NULL;

      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_p->GetInputPrefType (0,
                                                  &mediaType_out);
      ACE_ASSERT (SUCCEEDED (result) && mediaType_out);

      break;
    }
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      IUnknown* unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFStreamSink* stream_sink_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
      ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
      unknown_p->Release (); unknown_p = NULL;
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
      stream_sink_p->Release (); stream_sink_p = NULL;
      result = media_type_handler_p->GetCurrentMediaType (&mediaType_out);
      if (FAILED (result))
      {
        if (result == MF_E_NOT_INITIALIZED) // 0xC00D36B6
        { // get first available media type instead
          result = media_type_handler_p->GetMediaTypeByIndex (0,
                                                              &mediaType_out);
        } // end IF
        else
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
          media_type_handler_p->Release (); media_type_handler_p = NULL;
          topology_node_p->Release (); topology_node_p = NULL;
          return false;
        } // end ELSE
      } // end IF
      ACE_ASSERT (SUCCEEDED (result) && mediaType_out);
      media_type_handler_p->Release (); media_type_handler_p = NULL;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type_e));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    }
  } // end SWITCH
  ACE_ASSERT (mediaType_out);

  topology_node_p->Release (); topology_node_p = NULL;

  return true;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::setInputFormat (IMFTopology* topology_in,
                                                             TOPOID nodeId_in,
                                                             IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::setInputFormat"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (nodeId_in);
  ACE_ASSERT (mediaType_in);

  // step0: retrieve node handle and set preferred input type
  IMFTopologyNode* topology_node_p = NULL;
  HRESULT result = topology_in->GetNodeByID (nodeId_in,
                                             &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                nodeId_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);
  result = topology_node_p->SetInputPrefType (0,
                                              mediaType_in);
  ACE_ASSERT (SUCCEEDED (result));

  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  result = topology_node_p->GetNodeType (&node_type_e);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type_e)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("source nodes do not have inputs (id was: %q), aborting\n"),
                  nodeId_in));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      IUnknown* unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        return false;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
      result = transform_p->SetInputType (0,
                                          mediaType_in,
                                          0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (transform_p).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        return false;
      } // end IF
      transform_p->Release (); transform_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_p->SetInputPrefType (0,
                                                  mediaType_in);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      IUnknown* unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFStreamSink* stream_sink_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
      ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
      unknown_p->Release (); unknown_p = NULL;
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
      stream_sink_p->Release (); stream_sink_p = NULL;
      IMFMediaType* closest_match_p = NULL;
      result = media_type_handler_p->IsMediaTypeSupported (mediaType_in,
                                                           &closest_match_p);
      if (FAILED (result))
      {
        std::string closest_match_string =
          (closest_match_p ? ACE_TEXT_ALWAYS_CHAR (" (closest match: ") + Stream_MediaFramework_MediaFoundation_Tools::toString (closest_match_p) + ACE_TEXT_ALWAYS_CHAR (")")
                           : ACE_TEXT_ALWAYS_CHAR (""));
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("media type (was: %s) not supported%s, aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (mediaType_in).c_str ()),
                    ACE_TEXT (closest_match_string.c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        return false;
      } // end IF
      result = media_type_handler_p->SetCurrentMediaType (mediaType_in);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type_e));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    }
  } // end SWITCH
  topology_node_p->Release (); topology_node_p = NULL;

  return true;
}

//bool
//Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (IMFSourceReader* sourceReader_in,
//                                                              IMFMediaType*& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat"));
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_in);
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  HRESULT result = MFCreateMediaType (&mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//
//  result =
//    sourceReader_in->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                          &mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    goto error;
//  } // end IF
//  ACE_ASSERT (mediaType_out);
//
//  return true;
//
//error:
//  if (mediaType_out)
//  {
//    mediaType_out->Release ();
//    mediaType_out = NULL;
//  } // end IF
//
//  return false;
//}

bool
Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (IMFTransform* IMFTransform_in,
                                                              IMFMediaType*& IMFMediaType_out,
                                                              bool& isCurrentFormat_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;
  } // end IF
  isCurrentFormat_out = true;

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  DWORD* input_stream_ids_p = NULL;
  DWORD* output_stream_ids_p = NULL;
  DWORD index_i = 0;
  struct _GUID media_majortype = GUID_NULL, media_subtype = GUID_NULL;
  bool prefer_pcm = true;
  bool prefer_float = false;
  bool prefer_rgb = true;
  bool prefer_chroma = false;

  result = IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                            &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IMFTransform::GetStreamCount(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (IMFTransform_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_NEW_NORETURN (input_stream_ids_p,
                    DWORD[number_of_input_streams]);
  ACE_NEW_NORETURN (output_stream_ids_p,
                    DWORD[number_of_output_streams]);
  if (!input_stream_ids_p || !output_stream_ids_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory, aborting\n")));
    goto error;
  } // end IF
  result = IMFTransform_in->GetStreamIDs (number_of_input_streams,
                                          input_stream_ids_p,
                                          number_of_output_streams,
                                          output_stream_ids_p);
  if (FAILED (result))
  {
    if (result != E_NOTIMPL)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFTransform::GetStreamIDs(): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (IMFTransform_in).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF

    for (int i = 0;
         i < static_cast<int> (number_of_output_streams);
         ++i)
      output_stream_ids_p[i] = i;
  } // end IF
  delete [] input_stream_ids_p; input_stream_ids_p = NULL;

  result = IMFTransform_in->GetOutputCurrentType (output_stream_ids_p[0],
                                                  &IMFMediaType_out);
  if (SUCCEEDED (result) && IMFMediaType_out)
    goto continue_;
  // MF_E_TRANSFORM_TYPE_NOT_SET: 0xc00d6d60
  isCurrentFormat_out = false;

iterate:
  do
  {
    result = IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[0],
                                                      index_i,
                                                      &IMFMediaType_out);
    if (FAILED (result))
    {
      if (result == MF_E_NO_MORE_TYPES)
        break;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to IMFTransform::GetOutputAvailableType(%d): \"%s\", aborting\n"),
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (IMFTransform_in).c_str ()),
                  index_i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
continue_:
    ACE_ASSERT (IMFMediaType_out);

    result = IMFMediaType_out->GetGUID (MF_MT_MAJOR_TYPE,
                                        &media_majortype);
    ACE_ASSERT (SUCCEEDED (result));
    result = IMFMediaType_out->GetGUID (MF_MT_SUBTYPE,
                                        &media_subtype);
    ACE_ASSERT (SUCCEEDED (result));
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("%s: #%d: \"%s\"\n"),
    //            ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (IMFTransform_in).c_str ()),
    //            index,
    //            ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_subtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

    if (InlineIsEqualGUID (media_majortype, MFMediaType_Audio))
    {
      if ((prefer_pcm &&
           InlineIsEqualGUID (media_subtype, MEDIASUBTYPE_PCM))        ||
          (prefer_float &&
           InlineIsEqualGUID (media_subtype, MEDIASUBTYPE_IEEE_FLOAT)) ||
          (!prefer_pcm && !prefer_float))
        break;
    } // end IF
    else if (InlineIsEqualGUID (media_majortype, MFMediaType_Video))
    {
      if ((prefer_rgb &&
           Stream_MediaFramework_Tools::isRGB (media_subtype,
                                               STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))             ||
          (prefer_chroma &&
           Stream_MediaFramework_Tools::isChromaLuminance (media_subtype,
                                                           STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)) ||
          (!prefer_rgb && !prefer_chroma))
        break;
    } // end ELSE IF
    else
      break;

    // clean up
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;

    if (likely (!isCurrentFormat_out))
      ++index_i;
    else
      isCurrentFormat_out = false;
  } while (true);
  if (!IMFMediaType_out &&
      (InlineIsEqualGUID (media_majortype, MFMediaType_Audio) &&
       (prefer_pcm || prefer_float)) ||
      (InlineIsEqualGUID (media_majortype, MFMediaType_Video) &&
       (prefer_rgb || prefer_chroma)))
  {
    if (InlineIsEqualGUID (media_majortype, MFMediaType_Audio))
    {
      if (prefer_pcm)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("transform has no PCM output type(s), trying IEEE float\n")));
        prefer_pcm = false;
        prefer_float = true;
      } // end IF
      else if (prefer_float)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("transform has no IEEE float output type(s), returning first format\n")));
        prefer_float = false;
      } // end ELSE IF
    } // end IF
    else
    { ACE_ASSERT (InlineIsEqualGUID (media_majortype, MFMediaType_Video));
      if (prefer_rgb)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("transform has no RGB output type(s), trying chroma-luminance\n")));
        prefer_rgb = false;
        prefer_chroma = true;
      } // end IF
      else if (prefer_chroma)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("transform has no chroma-luminance output type(s), returning first format\n")));
        prefer_chroma = false;
      } // end ELSE IF
    } // end ELSE
    index_i = 0;
    goto iterate;
  } // end IF
  ACE_ASSERT (IMFMediaType_out);
  delete [] output_stream_ids_p; output_stream_ids_p = NULL;

  return true;

error:
  if (input_stream_ids_p)
    delete [] input_stream_ids_p;
  if (output_stream_ids_p)
    delete [] output_stream_ids_p;
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;
  } // end IF

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (IMFTopology* topology_in,
                                                              TOPOID nodeId_in,
                                                              IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  DWORD number_of_nodes = 0;
  IUnknown* unknown_p = NULL;
  IMFTopologyNode* topology_node_2 = NULL;
  DWORD input_index = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;

  if (nodeId_in)
  {
    result = topology_in->GetNodeByID (nodeId_in,
                                       &topology_node_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                  nodeId_in,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    goto continue_;
  } // end IF

  result = topology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result) && collection_p);
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_nodes <= 0)
  {
    //ACE_DEBUG ((LM_DEBUG,
    //            ACE_TEXT ("topology contains no output nodes, continuing\n")));
    collection_p->Release (); collection_p = NULL;
    result = topology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result) && collection_p);
    result = collection_p->GetElementCount (&number_of_nodes);
    ACE_ASSERT (SUCCEEDED (result));
    if (number_of_nodes <= 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("topology contains no source nodes, aborting\n")));
      collection_p->Release (); collection_p = NULL;
      goto error;
    } // end IF
    result = collection_p->GetElement (0, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result) && unknown_p);
    collection_p->Release (); collection_p = NULL;
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      unknown_p->Release (); unknown_p = NULL;
      goto error;
    } // end IF
    unknown_p->Release (); unknown_p = NULL;

    do
    { ACE_ASSERT (topology_node_p);
      result = topology_node_p->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0)
        break;

      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (0,
                                           &topology_node_2,
                                           &input_index);
      if (FAILED (result))
      {
        if (result == MF_E_NOT_FOUND) // output exists but is not connected
          break;
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopologyNode::GetOutput(%u): \"%s\", aborting\n"),
                    0,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (topology_node_2);
      topology_node_p->Release ();
      topology_node_p = topology_node_2;
    } while (true);

    goto continue_;
  } // end IF

  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result) && unknown_p);
  collection_p->Release (); collection_p = NULL;
  result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    unknown_p->Release (); unknown_p = NULL;
    goto error;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;
  ACE_ASSERT (topology_node_p);

continue_:
  if (!topology_node_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("(output-) node not found [id was: %q], aborting\n"),
                nodeId_in));
    goto error;
  } // end IF

  result = topology_node_p->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFStreamSink* stream_sink_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFStreamSink): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        goto error;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
      ACE_ASSERT (stream_sink_p);
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_sink_p->GetMediaTypeHandler (&media_type_handler_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFStreamSink::GetMediaTypeHandler(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        stream_sink_p->Release (); stream_sink_p = NULL;
        goto error;
      } // end IF
      stream_sink_p->Release (); stream_sink_p = NULL;
      ACE_ASSERT (media_type_handler_p);
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        goto error;
      } // end IF
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      break;
    }
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_p->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result) && stream_descriptor_p);
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
      stream_descriptor_p->Release (); stream_descriptor_p = NULL;
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        goto error;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
      ACE_ASSERT (transform_p);
      result = transform_p->GetOutputCurrentType (0,
                                                  &IMFMediaType_out);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
      transform_p->Release (); transform_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
    {
      result = topology_node_p->GetOutputPrefType (0,
                                                   &IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type));
      goto error;
    }
  } // end SWITCH
  ACE_ASSERT (IMFMediaType_out);
  topology_node_p->Release (); topology_node_p = NULL;

  return true;

error:
  if (topology_node_p)
    topology_node_p->Release ();
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;
  } // end IF

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat (IMFTopology* topology_in,
                                                              TOPOID nodeId_in,
                                                              const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (nodeId_in);
  ACE_ASSERT (mediaType_in);

  // step0: retrieve node handle and set preferred output type
  IMFTopologyNode* topology_node_p = NULL;
  HRESULT result = topology_in->GetNodeByID (nodeId_in,
                                             &topology_node_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopology::GetNodeByID(%q): \"%s\", aborting\n"),
                nodeId_in,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (topology_node_p);
  DWORD num_outputs_i = 0;
  result = topology_node_p->GetOutputCount (&num_outputs_i);
  ACE_ASSERT (SUCCEEDED (result));
  for (DWORD i = 0;
       i < num_outputs_i;
       ++i)
  {
    result =
      topology_node_p->SetOutputPrefType (i,
                                          const_cast<IMFMediaType*> (mediaType_in));
    ACE_ASSERT (SUCCEEDED (result));
  } // end FOR

  enum MF_TOPOLOGY_TYPE node_type_e = MF_TOPOLOGY_MAX;
  result = topology_node_p->GetNodeType (&node_type_e);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  switch (node_type_e)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_p->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result) && stream_descriptor_p);
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result) && media_type_handler_p);
      stream_descriptor_p->Release (); stream_descriptor_p = NULL;
      result =
        media_type_handler_p->SetCurrentMediaType (const_cast<IMFMediaType*> (mediaType_in));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        return false;
      } // end IF
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result) && unknown_p);
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        topology_node_p->Release (); topology_node_p = NULL;
        return false;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
      DWORD num_inputs_i = 0;
      result = transform_p->GetStreamCount (&num_inputs_i,
                                            &num_outputs_i);
      ACE_ASSERT (SUCCEEDED (result) && num_inputs_i && num_outputs_i);
      for (DWORD i = 0;
           i < num_outputs_i;
           ++i)
      {
        result =
          transform_p->SetOutputType (i,
                                      const_cast<IMFMediaType*> (mediaType_in),
                                      0);
        if (FAILED (result))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to IMFTransform::SetOutputType(%u): \"%s\", aborting\n"),
                      ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (transform_p).c_str ()),
                      i,
                      ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
          transform_p->Release (); transform_p = NULL;
          topology_node_p->Release (); topology_node_p = NULL;
          return false;
        } // end IF
      } // end FOR
      transform_p->Release (); transform_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TEE_NODE:
      break;
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("sink nodes do not have outputs (id was: %q), aborting\n"),
                  nodeId_in));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %d), aborting\n"),
                  node_type_e));
      topology_node_p->Release (); topology_node_p = NULL;
      return false;
    }
  } // end SWITCH
  topology_node_p->Release (); topology_node_p = NULL;

  return true;
}

//bool
//Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat (IMFSourceReader* IMFSourceReader_in,
//                                             const IMFMediaType* mediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::setOutputFormat"));
//
//  // sanit ycheck(s)
//  ACE_ASSERT (IMFSourceReader_in);
//  ACE_ASSERT (mediaType_in);
//
//  HRESULT result =
//    IMFSourceReader_in->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                             NULL,
//                                             const_cast<IMFMediaType*> (mediaType_in));
//  if (FAILED (result)) // MF_E_INVALIDMEDIATYPE: 0xC00D36B4L
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::SetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//
//  return true;
//}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString (MediaEventType eventType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  switch (eventType_in)
  {
    case MEUnknown:
      return ACE_TEXT_ALWAYS_CHAR ("MEUnknown");
    case MEError:
      return ACE_TEXT_ALWAYS_CHAR ("MEError");
    case MEExtendedType:
      return ACE_TEXT_ALWAYS_CHAR ("MEExtendedType");
    case MENonFatalError:
    //case MEGenericV1Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MENonFatalError");
    case MESessionUnknown:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionUnknown");
    case MESessionTopologySet:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionTopologySet");
    case MESessionTopologiesCleared:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionTopologiesCleared");
    case MESessionStarted:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionStarted");
    case MESessionPaused:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionPaused");
    case MESessionStopped:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionStopped");
    case MESessionClosed:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionClosed");
    case MESessionEnded:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionEnded");
    case MESessionRateChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionRateChanged");
    case MESessionScrubSampleComplete:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionScrubSampleComplete");
    case MESessionCapabilitiesChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionCapabilitiesChanged");
    case MESessionTopologyStatus:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionTopologyStatus");
    case MESessionNotifyPresentationTime:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionNotifyPresentationTime");
    case MENewPresentation:
      return ACE_TEXT_ALWAYS_CHAR ("MENewPresentation");
    case MELicenseAcquisitionStart:
      return ACE_TEXT_ALWAYS_CHAR ("MELicenseAcquisitionStart");
    case MELicenseAcquisitionCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MELicenseAcquisitionCompleted");
    case MEIndividualizationStart:
      return ACE_TEXT_ALWAYS_CHAR ("MEIndividualizationStart");
    case MEIndividualizationCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEIndividualizationCompleted");
    case MEEnablerProgress:
      return ACE_TEXT_ALWAYS_CHAR ("MEEnablerProgress");
    case MEEnablerCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEEnablerCompleted");
    case MEPolicyError:
      return ACE_TEXT_ALWAYS_CHAR ("MEPolicyError");
    case MEPolicyReport:
      return ACE_TEXT_ALWAYS_CHAR ("MEPolicyReport");
    case MEBufferingStarted:
      return ACE_TEXT_ALWAYS_CHAR ("MEBufferingStarted");
    case MEBufferingStopped:
      return ACE_TEXT_ALWAYS_CHAR ("MEBufferingStopped");
    case MEConnectStart:
      return ACE_TEXT_ALWAYS_CHAR ("MEConnectStart");
    case MEConnectEnd:
      return ACE_TEXT_ALWAYS_CHAR ("MEConnectEnd");
    case MEReconnectStart:
      return ACE_TEXT_ALWAYS_CHAR ("MEReconnectStart");
    case MEReconnectEnd:
      return ACE_TEXT_ALWAYS_CHAR ("MEReconnectEnd");
    case MERendererEvent:
      return ACE_TEXT_ALWAYS_CHAR ("MERendererEvent");
    case MESessionStreamSinkFormatChanged:
    //case MESessionV1Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MESessionStreamSinkFormatChanged");
    case MESourceUnknown:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceUnknown");
    case MESourceStarted:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceStarted");
    case MEStreamStarted:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamStarted");
    case MESourceSeeked:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceSeeked");
    case MEStreamSeeked:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSeeked");
    case MENewStream:
      return ACE_TEXT_ALWAYS_CHAR ("MENewStream");
    case MEUpdatedStream:
      return ACE_TEXT_ALWAYS_CHAR ("MEUpdatedStream");
    case MESourceStopped:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceStopped");
    case MEStreamStopped:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamStopped");
    case MESourcePaused:
      return ACE_TEXT_ALWAYS_CHAR ("MESourcePaused");
    case MEStreamPaused:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamPaused");
    case MEEndOfPresentation:
      return ACE_TEXT_ALWAYS_CHAR ("MEEndOfPresentation");
    case MEEndOfStream:
      return ACE_TEXT_ALWAYS_CHAR ("MEEndOfStream");
    case MEMediaSample:
      return ACE_TEXT_ALWAYS_CHAR ("MEMediaSample");
    case MEStreamTick:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamTick");
    case MEStreamThinMode:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamThinMode");
    case MEStreamFormatChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamFormatChanged");
    case MESourceRateChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceRateChanged");
    case MEEndOfPresentationSegment:
      return ACE_TEXT_ALWAYS_CHAR ("MEEndOfPresentationSegment");
    case MESourceCharacteristicsChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceCharacteristicsChanged");
    case MESourceRateChangeRequested:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceRateChangeRequested");
    case MESourceMetadataChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MESourceMetadataChanged");
    case MESequencerSourceTopologyUpdated:
    //case MESourceV1Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MESequencerSourceTopologyUpdated");
    case MESinkUnknown:
      return ACE_TEXT_ALWAYS_CHAR ("MESinkUnknown");
    case MEStreamSinkStarted:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkStarted");
    case MEStreamSinkStopped:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkStopped");
    case MEStreamSinkPaused:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkPaused");
    case MEStreamSinkRateChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkRateChanged");
    case MEStreamSinkRequestSample:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkRequestSample");
    case MEStreamSinkMarker:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkMarker");
    case MEStreamSinkPrerolled:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkPrerolled");
    case MEStreamSinkScrubSampleComplete:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkScrubSampleComplete");
    case MEStreamSinkFormatChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkFormatChanged");
    case MEStreamSinkDeviceChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkDeviceChanged");
    case MEQualityNotify:
      return ACE_TEXT_ALWAYS_CHAR ("MEQualityNotify");
    case MESinkInvalidated:
      return ACE_TEXT_ALWAYS_CHAR ("MESinkInvalidated");
    case MEAudioSessionNameChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionNameChanged");
    case MEAudioSessionVolumeChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionVolumeChanged");
    case MEAudioSessionDeviceRemoved:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionDeviceRemoved");
    case MEAudioSessionServerShutdown:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionServerShutdown");
    case MEAudioSessionGroupingParamChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionGroupingParamChanged");
    case MEAudioSessionIconChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionIconChanged");
    case MEAudioSessionFormatChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionFormatChanged");
    case MEAudioSessionDisconnected:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionDisconnected");
    case MEAudioSessionExclusiveModeOverride:
    //case MESinkV1Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MEAudioSessionExclusiveModeOverride");
    case MECaptureAudioSessionVolumeChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MECaptureAudioSessionVolumeChanged");
    case MECaptureAudioSessionDeviceRemoved:
      return ACE_TEXT_ALWAYS_CHAR ("MECaptureAudioSessionDeviceRemoved");
    case MECaptureAudioSessionFormatChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MECaptureAudioSessionFormatChanged");
    case MECaptureAudioSessionDisconnected:
      return ACE_TEXT_ALWAYS_CHAR ("MECaptureAudioSessionDisconnected");
    case MECaptureAudioSessionExclusiveModeOverride:
      return ACE_TEXT_ALWAYS_CHAR ("MECaptureAudioSessionExclusiveModeOverride");
    case MECaptureAudioSessionServerShutdown:
    //case MESinkV2Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MECaptureAudioSessionServerShutdown");
    case METrustUnknown:
      return ACE_TEXT_ALWAYS_CHAR ("METrustUnknown");
    case MEPolicyChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEPolicyChanged");
    case MEContentProtectionMessage:
      return ACE_TEXT_ALWAYS_CHAR ("MEContentProtectionMessage");
    case MEPolicySet:
    //case METrustV1Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MEPolicySet");
    case MEWMDRMLicenseBackupCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMLicenseBackupCompleted");
    case MEWMDRMLicenseBackupProgress:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMLicenseBackupProgress");
    case MEWMDRMLicenseRestoreCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMLicenseRestoreCompleted");
    case MEWMDRMLicenseRestoreProgress:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMLicenseRestoreProgress");
    case MEWMDRMLicenseAcquisitionCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMLicenseAcquisitionCompleted");
    case MEWMDRMIndividualizationCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMIndividualizationCompleted");
    case MEWMDRMIndividualizationProgress:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMIndividualizationProgress");
    case MEWMDRMProximityCompleted:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMProximityCompleted");
    case MEWMDRMLicenseStoreCleaned:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMLicenseStoreCleaned");
    case MEWMDRMRevocationDownloadCompleted:
    //case MEWMDRMV1Anchor:
      return ACE_TEXT_ALWAYS_CHAR ("MEWMDRMRevocationDownloadCompleted");
    case METransformUnknown:
      return ACE_TEXT_ALWAYS_CHAR ("METransformUnknown");
    case METransformNeedInput:
      return ACE_TEXT_ALWAYS_CHAR ("METransformNeedInput");
    case METransformHaveOutput:
      return ACE_TEXT_ALWAYS_CHAR ("METransformHaveOutput");
    case METransformDrainComplete:
      return ACE_TEXT_ALWAYS_CHAR ("METransformDrainComplete");
    case METransformMarker:
      return ACE_TEXT_ALWAYS_CHAR ("METransformMarker");
    case METransformInputStreamStateChanged:
      return ACE_TEXT_ALWAYS_CHAR ("METransformInputStreamStateChanged");
    case MEByteStreamCharacteristicsChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEByteStreamCharacteristicsChanged");
    case MEVideoCaptureDeviceRemoved:
      return ACE_TEXT_ALWAYS_CHAR ("MEVideoCaptureDeviceRemoved");
    case MEVideoCaptureDevicePreempted:
      return ACE_TEXT_ALWAYS_CHAR ("MEVideoCaptureDevicePreempted");
    case MEStreamSinkFormatInvalidated:
      return ACE_TEXT_ALWAYS_CHAR ("MEStreamSinkFormatInvalidated");
    case MEEncodingParameters:
      return ACE_TEXT_ALWAYS_CHAR ("MEEncodingParameters");
    case MEContentProtectionMetadata:
      return ACE_TEXT_ALWAYS_CHAR ("MEContentProtectionMetadata");
    case MEDeviceThermalStateChanged:
      return ACE_TEXT_ALWAYS_CHAR ("MEDeviceThermalStateChanged");
    case MEReservedMax:
      return ACE_TEXT_ALWAYS_CHAR ("MEReservedMax");
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media event type (was: %u), aborting\n"),
                  eventType_in));
      break;
    }
  } // end SWITCH
  return ACE_TEXT_ALWAYS_CHAR ("");
}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString (enum MF_TOPOLOGY_TYPE nodeType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  std::string result;

  switch (nodeType_in)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("output"); break;
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("source"); break;
    case MF_TOPOLOGY_TRANSFORM_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("transform"); break;
    case MF_TOPOLOGY_TEE_NODE:
      result = ACE_TEXT_ALWAYS_CHAR ("tee"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown node type (was: %u), aborting\n"),
                  nodeType_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString (MF_TOPOSTATUS topologyStatus_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  std::string result;

  switch (topologyStatus_in)
  {
    case MF_TOPOSTATUS_INVALID:
      result = ACE_TEXT_ALWAYS_CHAR ("invalid"); break;
    case MF_TOPOSTATUS_READY:
      result = ACE_TEXT_ALWAYS_CHAR ("ready"); break;
    case MF_TOPOSTATUS_STARTED_SOURCE:
      result = ACE_TEXT_ALWAYS_CHAR ("started"); break;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    case MF_TOPOSTATUS_DYNAMIC_CHANGED:
      result = ACE_TEXT_ALWAYS_CHAR ("changed"); break;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    case MF_TOPOSTATUS_SINK_SWITCHED:
      result = ACE_TEXT_ALWAYS_CHAR ("switched"); break;
    case MF_TOPOSTATUS_ENDED:
      result = ACE_TEXT_ALWAYS_CHAR ("ended"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown topology status (was: %u), aborting\n"),
                  topologyStatus_in));
      break;
    }
  } // end SWITCH

  return result;
}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString (IMFActivate* activate_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  std::string result;

  HRESULT result_2 = E_FAIL;
  WCHAR buffer_a[BUFSIZ];
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result_2 = activate_in->GetString (MFT_FRIENDLY_NAME_Attribute,
                                     buffer_a, sizeof (WCHAR[BUFSIZ]),
                                     NULL);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (result);
  ACE_NOTREACHED (return result;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to IMFAttributes::GetString(MFT_FRIENDLY_NAME_Attribute): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));

  return result;
}
std::string
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
Stream_MediaFramework_MediaFoundation_Tools::toString (IMFMediaSourceEx* mediaSource_in)
#else
Stream_MediaFramework_MediaFoundation_Tools::toString (IMFMediaSource* mediaSource_in)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  std::string result;

  HRESULT result_2 = E_FAIL;
  IMFAttributes* attributes_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  WCHAR buffer_a[BUFSIZ];
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  result_2 = mediaSource_in->GetSourceAttributes (&attributes_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to IMFMediaSourceEx::GetSourceAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (attributes_p);
  result_2 = attributes_p->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                      buffer_a, sizeof (WCHAR[BUFSIZ]),
                                      NULL);
  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to IMFAttributes::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME ): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error;
  } // end IF
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));
#else
  // *TODO*
  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("cannot convert IMFMediaSource to string on this target platform, continuing\n")));

  return ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_MEDIAFOUNDATION_MEDIASOURCE_FRIENDLY_NAME));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
error:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  if (attributes_p)
    attributes_p->Release ();

  return result;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::load (REFGUID category_in,
                                                   UINT32 flags_in,
                                                   const MFT_REGISTER_TYPE_INFO* inputMediaType_in,
                                                   const MFT_REGISTER_TYPE_INFO* outputMediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                   IMFActivate**& handles_out,
#else
                                                   IMFAttributes* attributes_in,
                                                   CLSID*& handles_out,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                   UINT32& numberOfHandles_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::load"));

  // sanity check(s)
  ACE_ASSERT (!InlineIsEqualGUID (category_in, GUID_NULL));
  ACE_ASSERT (!handles_out);

  // initialize return value(s)
  numberOfHandles_out = 0;

  HRESULT result = E_FAIL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = MFTEnumEx (category_in,           // category
                      flags_in,              // flags
                      inputMediaType_in,     // input media type
                      outputMediaType_in,    // output media type
                      &handles_out,          // array of decoders
                      &numberOfHandles_out); // size of array
#else
  result =
    MFTEnum (category_in,                                              // category
             0,                                                        // reserved
             const_cast<MFT_REGISTER_TYPE_INFO*> (inputMediaType_in),  // input media type
             const_cast<MFT_REGISTER_TYPE_INFO*> (outputMediaType_in), // output media type
             attributes_in,                                            // attributes
             &handles_out,                                             // array of decoders
             &numberOfHandles_out);                                    // size of array
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
  if (FAILED (result))
  {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFTEnumEx(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFTEnum(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
    return false;
  } // end IF
  if (!numberOfHandles_out)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("cannot find filter (category was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (category_in).c_str ())));
    return false;
  } // end IF

  return true;
}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString (IMFTransform* IMFTransform_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);

  std::string result;

  IMFAttributes* attributes_p = NULL;
  HRESULT result_2 = IMFTransform_in->GetAttributes (&attributes_p);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((((result_2 != E_NOTIMPL) ? LM_ERROR : LM_DEBUG),
                ACE_TEXT ("failed to IMFTransform::GetAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF
  ACE_ASSERT (attributes_p);
  WCHAR buffer_a[BUFSIZ];
  result_2 = attributes_p->GetString (MFT_FRIENDLY_NAME_Attribute,
                                      buffer_a, sizeof (WCHAR[BUFSIZ]),
                                      NULL);
  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  {
    ACE_DEBUG ((((result_2 != MF_E_ATTRIBUTENOTFOUND) ? LM_ERROR : LM_DEBUG),
                ACE_TEXT ("failed to IMFAttributes::GetString(MFT_FRIENDLY_NAME_Attribute): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    attributes_p->Release (); attributes_p = NULL;
    return ACE_TEXT_ALWAYS_CHAR ("");
  } // end IF
  attributes_p->Release (); attributes_p = NULL;
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));

  return result;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::copy (const IMFAttributes* source_in,
                                                   IMFAttributes* destination_in,
                                                   REFGUID key_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::copy"));

  // sanity check(s)
  ACE_ASSERT (source_in);
  ACE_ASSERT (destination_in);

  struct tagPROPVARIANT property_s;
  PropVariantInit (&property_s);

  HRESULT result =
    const_cast<IMFAttributes*> (source_in)->GetItem (key_in, &property_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::GetItem(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF
  result = destination_in->SetItem (key_in, property_s);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetItem(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto clean;
  } // end IF

clean:
  PropVariantClear (&property_s);

  return (result == S_OK);
}

IMFMediaType*
Stream_MediaFramework_MediaFoundation_Tools::copy (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::copy"));

  // initialize return value(s)
  IMFMediaType* result_p = NULL;

  HRESULT result = MFCreateMediaType (&result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return NULL;
  } // end IF

  result =
    const_cast<IMFMediaType*> (mediaType_in)->CopyAllItems (result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaType::CopyAllItems(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    result_p->Release (); result_p = NULL;
    return NULL;
  } // end IF

  return result_p;
}

//std::string
//Stream_MediaFramework_MediaFoundation_Tools::mediaSubTypeToString (REFGUID GUID_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::mediaSubTypeToString"));
//
//  //std::string result;
//
//  //GUID2STRING_MAP_ITERATOR_T iterator =
//  //  Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaSubType2StringMap.find (GUID_in);
//  //if (iterator == Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaSubType2StringMap.end ())
//  //{
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("invalid/unknown media subtype (was: \"%s\"), aborting\n"),
//  //              ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (GUID_in).c_str ())));
//  //  return result;
//  //} // end IF
//  //result = (*iterator).second;
//
//  //return result;
//
//  FOURCCMap fourcc_map (&GUID_in);
//
//  return Stream_Module_Decoder_Tools::FOURCCToString (fourcc_map.GetFOURCC ());
//}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString (const IMFMediaType* mediaType_in,
                                                       bool condensed_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  if (unlikely (condensed_in))
    return Stream_MediaFramework_MediaFoundation_Tools::toString_2 (mediaType_in);

  std::string result;

  struct _GUID guid_s = GUID_NULL;
  bool is_video_b = false;
  result = ACE_TEXT_ALWAYS_CHAR ("majortype: \"");
  HRESULT result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->GetMajorType (&guid_s);
  ACE_ASSERT (SUCCEEDED (result_2));
  Stream_MediaFramework_GUIDToStringMapConstIterator_t iterator =
    Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.find (guid_s);
  result = ACE_TEXT_ALWAYS_CHAR ("majortype: ");
  if (iterator == Stream_MediaFramework_MediaFoundation_Tools::Stream_MediaMajorTypeToStringMap.end ())
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("invalid/unknown media majortype (was: \"%s\"), continuing\n"),
                ACE_TEXT (Common_Tools::GUIDToString (guid_s).c_str ())));
    result += Common_Tools::GUIDToString (guid_s);
  } // end IF
  else
    result += (*iterator).second;
  is_video_b = InlineIsEqualGUID (guid_s, MFMediaType_Video);
  BOOL uses_temporal_compression = FALSE;
  // *WARNING*: if MF_MT_ALL_SAMPLES_INDEPENDENT is not set, this returns 'true',
  //            which may well be a false positive
  result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->IsCompressedFormat (&uses_temporal_compression);
  ACE_ASSERT (SUCCEEDED (result_2));
  result += ACE_TEXT_ALWAYS_CHAR ("\ntemporal compression: ");
  result +=
    (uses_temporal_compression ? ACE_TEXT_ALWAYS_CHAR ("yes")
                               : ACE_TEXT_ALWAYS_CHAR ("no"));
  UINT32 item_count = 0;
  result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->GetCount (&item_count);
  ACE_ASSERT (SUCCEEDED (result_2));
  struct tagPROPVARIANT value_v;
  PropVariantInit (&value_v);
  std::ostringstream converter;
  for (UINT32 i = 0;
       i < item_count;
       ++i)
  {
    guid_s = GUID_NULL;
    result_2 = PropVariantClear (&value_v);
    ACE_ASSERT (SUCCEEDED (result_2));
    result_2 =
      const_cast<IMFMediaType*> (mediaType_in)->GetItemByIndex (i,
                                                                &guid_s,
                                                                &value_v);
    ACE_ASSERT (SUCCEEDED (result_2));
    if (InlineIsEqualGUID (guid_s, MF_MT_ALL_SAMPLES_INDEPENDENT))
    { ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nindependent samples: ");
      result +=
        (value_v.uintVal ? ACE_TEXT_ALWAYS_CHAR ("true")
                         : ACE_TEXT_ALWAYS_CHAR ("false"));
    } // end IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_MAJOR_TYPE))
    {} // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AM_FORMAT_TYPE))
    { ACE_ASSERT (value_v.vt == VT_CLSID);
      result += ACE_TEXT_ALWAYS_CHAR ("\nformat type: ");
      result +=
        Stream_MediaFramework_Tools::mediaFormatTypeToString (*value_v.puuid);
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_COMPRESSED))
    { ACE_ASSERT (value_v.vt == VT_UINT);
      result += ACE_TEXT_ALWAYS_CHAR ("\nmedia data is compressed: ");
      result +=
        (value_v.uintVal ? ACE_TEXT_ALWAYS_CHAR ("true")
                         : ACE_TEXT_ALWAYS_CHAR ("false"));
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_FIXED_SIZE_SAMPLES))
    { ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nfixed size samples: ");
      result +=
        (value_v.uintVal ? ACE_TEXT_ALWAYS_CHAR ("true")
                         : ACE_TEXT_ALWAYS_CHAR ("false"));
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_MAJOR_TYPE))
    { ACE_ASSERT (value_v.vt == VT_CLSID);
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_SAMPLE_SIZE))
    { ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nsample size: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR (" byte(s)");
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_SUBTYPE))
    { ACE_ASSERT (value_v.vt == VT_CLSID);
      result += ACE_TEXT_ALWAYS_CHAR ("\nsubtype: ");
      result +=
        Stream_MediaFramework_Tools::mediaSubTypeToString (*value_v.puuid,
                                                           STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION);
    } // end IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_USER_DATA))
    { ACE_ASSERT (value_v.vt == VT_BLOB);
      result += ACE_TEXT_ALWAYS_CHAR ("\nuser data: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.blob.cbSize;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR (" byte(s)");
    } // end IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_WRAPPED_TYPE))
    { ACE_ASSERT (value_v.vt == VT_BLOB);
      result += ACE_TEXT_ALWAYS_CHAR ("\nwrapped media type: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.blob.cbSize;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR (" byte(s)");
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_SAMPLES_PER_SECOND))
    { ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nsamples/sec: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_NUM_CHANNELS))
    {
      ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\n#channels: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_CHANNEL_MASK))
    {
      ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\naudio channel mask: 0x");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << std::setw (8) << std::setfill ('0') << std::hex << value_v.uintVal << std::dec;
      result += converter.str ();
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_AVG_BYTES_PER_SECOND))
    {
      ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\naverage bytes/sec: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_BLOCK_ALIGNMENT))
    {
      ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nblock alignment: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
      result += ACE_TEXT_ALWAYS_CHAR (" byte(s)");
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_PREFER_WAVEFORMATEX))
    {
      ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nprefer waveformatex: ");
      result +=
        (value_v.uintVal ? ACE_TEXT_ALWAYS_CHAR ("true")
                         : ACE_TEXT_ALWAYS_CHAR ("false"));
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AUDIO_BITS_PER_SAMPLE))
    {
      ACE_ASSERT (value_v.vt == VT_UI4);
      result += ACE_TEXT_ALWAYS_CHAR ("\nbits/sample: ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_FRAME_SIZE))
    { ACE_ASSERT (value_v.vt == VT_UINT);
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_AVG_BITRATE))
    { ACE_ASSERT (value_v.vt == VT_UINT);
      result += ACE_TEXT_ALWAYS_CHAR ("\naverage bitrate (1/s): ");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << value_v.uintVal;
      result += converter.str ();
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_FRAME_RATE))
    { ACE_ASSERT (value_v.vt == VT_UINT);
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_PIXEL_ASPECT_RATIO))
    { ACE_ASSERT (value_v.vt == VT_UINT);
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_INTERLACE_MODE))
    { ACE_ASSERT (value_v.vt == VT_UINT);
      result += ACE_TEXT_ALWAYS_CHAR ("\ninterlace mode: ");
      switch (static_cast<enum _MFVideoInterlaceMode> (value_v.uintVal))
      {
        case MFVideoInterlace_Unknown:
          result += ACE_TEXT_ALWAYS_CHAR ("Unknown"); break;
        case MFVideoInterlace_Progressive:
          result += ACE_TEXT_ALWAYS_CHAR ("Progressive"); break;
        case MFVideoInterlace_FieldInterleavedUpperFirst:
          result += ACE_TEXT_ALWAYS_CHAR ("FieldInterleavedUpperFirst"); break;
        case MFVideoInterlace_FieldInterleavedLowerFirst:
          result += ACE_TEXT_ALWAYS_CHAR ("FieldInterleavedLowerFirst"); break;
        case MFVideoInterlace_FieldSingleUpper:
          result += ACE_TEXT_ALWAYS_CHAR ("FieldSingleUpper"); break;
        case MFVideoInterlace_FieldSingleLower:
          result += ACE_TEXT_ALWAYS_CHAR ("FieldSingleLower"); break;
        case MFVideoInterlace_MixedInterlaceOrProgressive:
          result += ACE_TEXT_ALWAYS_CHAR ("MixedInterlaceOrProgressive"); break;
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown interlace mode (was: %d), continuing\n"),
                      value_v.uintVal));
          break;
        }
      } // end SWITCH
    } // end ELSE IF
    else
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media type attribute (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (guid_s).c_str ())));
  } // end FOR
  if (!is_video_b)
    return result;
  UINT32 width, height;
  result_2 = MFGetAttributeSize (const_cast<IMFMediaType*> (mediaType_in),
                                 MF_MT_FRAME_SIZE,
                                 &width, &height);
  ACE_ASSERT (SUCCEEDED (result_2));
  result += ACE_TEXT_ALWAYS_CHAR ("\nframe size [wxh]: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << width;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("x");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << height;
  result += converter.str ();
  UINT32 numerator, denominator;
  result_2 = MFGetAttributeRatio (const_cast<IMFMediaType*> (mediaType_in),
                                  MF_MT_FRAME_RATE,
                                  &numerator, &denominator);
  ACE_ASSERT (SUCCEEDED (result_2));
  result += ACE_TEXT_ALWAYS_CHAR ("\nframe rate (1/s): ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << numerator;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR ("/");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << denominator;
  result += converter.str ();
  result_2 = MFGetAttributeRatio (const_cast<IMFMediaType*> (mediaType_in),
                                  MF_MT_PIXEL_ASPECT_RATIO,
                                  &numerator, &denominator);
  ACE_ASSERT (SUCCEEDED (result_2));
  result += ACE_TEXT_ALWAYS_CHAR ("\npixel aspect ratio: ");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << numerator;
  result += converter.str ();
  result += ACE_TEXT_ALWAYS_CHAR (":");
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter.clear ();
  converter << denominator;
  result += converter.str ();

  return result;
}

std::string
Stream_MediaFramework_MediaFoundation_Tools::toString_2 (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString_2"));

  std::string result;

  struct _GUID guid_s = GUID_NULL;
  HRESULT result_2 =
    const_cast<IMFMediaType*> (mediaType_in)->GetMajorType (&guid_s);
  ACE_ASSERT (SUCCEEDED (result_2));
  if (InlineIsEqualGUID (guid_s, MFMediaType_Video))
  {
    ACE_ASSERT (false); // *TODO*
    return result;
  } // end IF

  struct tWAVEFORMATEX* waveformatex_p = NULL;
  UINT32 cbSize = 0;
  result_2 =
    MFCreateWaveFormatExFromMFMediaType (const_cast<IMFMediaType*> (mediaType_in),
                                         &waveformatex_p,
                                         &cbSize,
                                         MFWaveFormatExConvertFlag_Normal);
  ACE_ASSERT (SUCCEEDED (result_2) && waveformatex_p);
  result = Stream_MediaFramework_DirectSound_Tools::toString (*waveformatex_p,
                                                              true);
  CoTaskMemFree (waveformatex_p); waveformatex_p = NULL;

  return result;
}
