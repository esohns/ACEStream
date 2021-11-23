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

#include <sstream>

#include "evr.h"
#include "mfapi.h"
#include "mferror.h"
#define INITGUID
#include "mmdeviceapi.h"
#include "wmcodecdsp.h"

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_time_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_lib_defines.h"
#include "stream_lib_directshow_tools.h"
#include "stream_lib_guids.h"
#include "stream_lib_macros.h"
#include "stream_lib_tools.h"

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

bool
Stream_MediaFramework_MediaFoundation_Tools::expand (TOPOLOGY_PATH_T& path_inout,
                                                     TOPOLOGY_PATHS_T& paths_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::expand"));

  TOPOLOGY_PATH_ITERATOR_T iterator = path_inout.begin ();
  for (;
       iterator != path_inout.end ();
       ++iterator);
  --iterator;

  HRESULT result = E_FAIL;
  DWORD number_of_outputs = 0;
  result = (*iterator)->GetOutputCount (&number_of_outputs);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_outputs == 0)
    return false; // done (no changes)

  IMFTopologyNode* topology_node_p = NULL;
  DWORD input_index = 0;
  TOPOLOGY_PATH_T topology_path;
  for (DWORD i = 0;
       i < number_of_outputs;
       ++i)
  {
    topology_node_p = NULL;
    result = (*iterator)->GetOutput (i,
                                     &topology_node_p,
                                     &input_index);
    ACE_ASSERT (SUCCEEDED (result));

    topology_path = path_inout;
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = topology_path.begin ();
         iterator_2 != topology_path.end ();
         ++iterator_2)
      (*iterator_2)->AddRef ();
    topology_path.push_back (topology_node_p);
    paths_inout.push_back (topology_path);
  } // end FOR

  return true;
}

void
Stream_MediaFramework_MediaFoundation_Tools::dump (IMFTopology* topology_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::dump"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  HRESULT result = S_OK;
  WORD count = 0;
  result = topology_in->GetNodeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  TOPOID id = 0;
  result = topology_in->GetTopologyID (&id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("topology (id: %q, %u nodes)\n"),
              id,
              count));
  IMFTopologyNode* topology_node_p = NULL;
  MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  //for (WORD i = 0;
  //     i < count;
  //     ++i)
  //{
  //  topology_node_p = NULL;
  //  result = topology_in->GetNode (i,
  //                                    &topology_node_p);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result = topology_node_p->GetNodeType (&node_type);
  //  ACE_ASSERT (SUCCEEDED (result));
  //  result = topology_node_p->GetTopoNodeID (&id);
  //  ACE_ASSERT (SUCCEEDED (result));

  //  ACE_DEBUG ((LM_INFO,
  //              ACE_TEXT ("#%u: %s (id: %q)\n"),
  //              i + 1,
  //              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (node_type).c_str ()),
  //              id));

  //  topology_node_p->Release ();
  //} // end FOR
  IMFCollection* collection_p = NULL;
  result = topology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("topology contains no source nodes, done\n")));

    // clean up
    collection_p->Release ();

    return;
  } // end IF
  TOPOLOGY_PATHS_T topology_paths;
  TOPOLOGY_PATH_T topology_path;
  IUnknown* unknown_p = NULL;
  for (DWORD i = 0;
       i < number_of_source_nodes;
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
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", returning\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));

      // clean up
      unknown_p->Release ();
      collection_p->Release ();

      return;
    } // end IF
    unknown_p->Release ();
    ACE_ASSERT (topology_node_p);
    topology_path.clear ();
    topology_path.push_back (topology_node_p);
    topology_paths.push_back (topology_path);
  } // end FOR
  collection_p->Release ();

  bool changed = false;
  do
  {
    changed = false;
    for (TOPOLOGY_PATHS_ITERATOR_T iterator = topology_paths.begin ();
         iterator != topology_paths.end ();
         ++iterator)
    {
      if (Stream_MediaFramework_MediaFoundation_Tools::expand (*iterator,
                                                               topology_paths))
      {
        for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
             iterator_2 != (*iterator).end ();
             ++iterator_2)
          (*iterator_2)->Release ();
        topology_paths.erase (iterator);
        changed = true;
        break;
      } // end IF
    } // end FOR
    if (!changed)
      break;
  } while (true);

  std::string topology_string_base, topology_string;
  std::ostringstream converter;
  int index = 0;
  for (TOPOLOGY_PATHS_ITERATOR_T iterator = topology_paths.begin ();
       iterator != topology_paths.end ();
       ++iterator, ++index)
  {
    topology_string_base = ACE_TEXT_ALWAYS_CHAR ("#");
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter.clear ();
    converter << index + 1;
    topology_string_base += converter.str ();
    topology_string_base += ACE_TEXT_ALWAYS_CHAR (": ");

    TOPOLOGY_PATH_ITERATOR_T iterator_3;
    for (TOPOLOGY_PATH_ITERATOR_T iterator_2 = (*iterator).begin ();
         iterator_2 != (*iterator).end ();
         ++iterator_2)
    {
      result = (*iterator_2)->GetNodeType (&node_type);
      ACE_ASSERT (SUCCEEDED (result));
      result = (*iterator_2)->GetTopoNodeID (&id);
      ACE_ASSERT (SUCCEEDED (result));

      topology_string =
        Stream_MediaFramework_MediaFoundation_Tools::toString (node_type);
      topology_string += ACE_TEXT_ALWAYS_CHAR (" (");
      converter.str (ACE_TEXT_ALWAYS_CHAR (""));
      converter.clear ();
      converter << id;
      topology_string += converter.str ();
      topology_string += ACE_TEXT_ALWAYS_CHAR (")");

      iterator_3 = iterator_2;
      if (++iterator_3 != (*iterator).end ())
        topology_string += ACE_TEXT_ALWAYS_CHAR (" --> ");

      topology_string_base += topology_string;

      // clean up
      (*iterator_2)->Release ();
    } // end FOR
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("%s\n"),
                ACE_TEXT (topology_string_base.c_str ())));
  } // end FOR
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
      if (FAILED (result))
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
Stream_MediaFramework_MediaFoundation_Tools::addGrabber (const IMFMediaType* mediaType_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                         IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                         IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                         IMFTopology* topology_in,
                                                         TOPOID& grabberNodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::addGrabber"));

  // sanity check(s)
  ACE_ASSERT (mediaType_in);
  ACE_ASSERT (sampleGrabberSinkCallback_in);
  ACE_ASSERT (topology_in);

  // initialize return value(s)
  grabberNodeId_out = 0;

  // step1: create sample grabber sink
  IMFActivate* activate_p = NULL;
  HRESULT result = S_OK;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    MFCreateSampleGrabberSinkActivate (const_cast<IMFMediaType*> (mediaType_in),
                                       sampleGrabberSinkCallback_in,
                                       &activate_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (activate_p);

  IMFMediaSink* media_sink_p = NULL;
  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  if (FAILED (result))
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
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release (); media_sink_p = NULL;
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
  result = topology_node_p->GetTopoNodeID (&grabberNodeId_out);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added grabber node (id: %q)...\n"),
              grabberNodeId_out));
  topology_node_p->Release (); topology_node_p = NULL;

  if (!Stream_MediaFramework_MediaFoundation_Tools::append (topology_in,
                                                            grabberNodeId_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::append(%q), aborting\n"),
                grabberNodeId_out));
    goto error;
  } // end IF

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
  if (grabberNodeId_out)
  {
    result = topology_in->GetNodeByID (grabberNodeId_out,
                                       &topology_node_p);
    ACE_ASSERT (SUCCEEDED (result));
    result = topology_in->RemoveNode (topology_node_p);
    if (FAILED (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTopology::RemoveNode(%q): \"%s\", continuing\n"),
                  grabberNodeId_out,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    grabberNodeId_out = 0;
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

bool
Stream_MediaFramework_MediaFoundation_Tools::addRenderer (HWND windowHandle_in,
                                                          IMFTopology* topology_in,
                                                          TOPOID& rendererNodeId_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::addRenderer"));

  // sanity check(s)
  ACE_ASSERT (topology_in);

  // initialize return value(s)
  rendererNodeId_out = 0;

  IMFTopologyNode* topology_node_p = NULL;

  // step1: create (EVR) renderer
  IMFActivate* activate_p = NULL;
  HRESULT result = S_OK;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateVideoRendererActivate (windowHandle_in,
                                          &activate_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateVideoRendererActivate() \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    return false;
  } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
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
  IMFMediaSink* media_sink_p = NULL;
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

  //result = MFCreateVideoRenderer (IID_PPV_ARGS (&media_sink_p));
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoRenderer(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
  //} // end IF

  IMFStreamSink* stream_sink_p = NULL;
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
  result = presentation_clock_p->Start (0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationClock::Start(0): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_clock_p->Release (); presentation_clock_p = NULL;
    goto error;
  } // end IF
  presentation_clock_p->Release (); presentation_clock_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

  //IMFTransform* transform_p = NULL;
  //result =
  //  MFCreateVideoMixer (NULL,                         // owner
  //                      IID_IDirect3DDevice9,         // device
  //                      IID_PPV_ARGS (&transform_p)); // return value: interface handle
  //if (FAILED (result))
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFCreateVideoPresenter(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
  //  goto error;
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
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release (); media_sink_p = NULL;
  result = topology_node_p->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_p->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_p->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
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
                                                            rendererNodeId_out))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::append(%q), aborting\n"),
                rendererNodeId_out));
    goto error;
  } // end IF

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
    result = topology_node_p->SetObject (NULL);
    ACE_ASSERT (SUCCEEDED (result));
    topology_node_p->Release (); topology_node_p = NULL;
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
  DWORD topology_flags = (MFSESSION_SETTOPOLOGY_IMMEDIATE   |
                          MFSESSION_SETTOPOLOGY_NORESOLUTION);
  if (isPartial_in)
    topology_flags &= ~MFSESSION_SETTOPOLOGY_NORESOLUTION;
  IMFMediaEvent* media_event_p = NULL;
  bool received_topology_event = false;
  MediaEventType event_type = MEUnknown;
  ACE_Time_Value timeout (STREAM_LIB_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT, 0);
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
    result = MFCreateMediaSession (attributes_p,
                                   &mediaSession_inout);
    if (FAILED (result)) // MF_E_SHUTDOWN: 0xC00D3E85L
    {
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
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
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
      {
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

  result = topology_p->GetUINT32 (MF_TOPOLOGY_RESOLUTION_STATUS,
                                  &value_i);
  if (SUCCEEDED (result))
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology resolution status: 0x%x\n"),
                value_i));

  received_topology_event = false;
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = mediaSession_inout->GetEvent (0,
                                           &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
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
                                                     TOPOID nodeId_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::append"));

  // sanity check(s)
  ACE_ASSERT (topology_in);
  ACE_ASSERT (nodeId_in);

  // step0: retrieve node handle
  bool add_tee_node = true;
  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  result = topology_in->GetNodeByID (nodeId_in,
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

  // step1: find a suitable upstream node
  IMFTopologyNode* topology_node_2 = NULL; // source/output node
  IMFTopologyNode* topology_node_3 = NULL; // upstream node
  IMFMediaType* media_type_p = NULL;
  IMFCollection* collection_p = NULL;
  result = topology_in->GetOutputNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_nodes = 0;
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  IUnknown* unknown_p = NULL;
  TOPOID node_id = 0;
  DWORD number_of_inputs = 0;
  DWORD output_index = 0;
  enum MF_TOPOLOGY_TYPE node_type = MF_TOPOLOGY_MAX;
  IMFTopologyNode* topology_node_4 = NULL;
  DWORD input_index = 0;

  if (number_of_nodes <= 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology contains no output nodes, continuing\n")));

use_source_node:
    add_tee_node = false;
    collection_p->Release ();
    collection_p = NULL;
    result = topology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result));
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
    ACE_ASSERT (SUCCEEDED (result));
    collection_p->Release (); collection_p = NULL;
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      unknown_p->Release (); unknown_p = NULL;
      goto error;
    } // end IF
    unknown_p->Release (); unknown_p = NULL;
    ACE_ASSERT (topology_node_2);

    do
    {
      result = topology_node_2->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0)
        break;

      topology_node_3 = NULL;
      result = topology_node_2->GetOutput (0,
                                           &topology_node_3,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_2->Release ();
      topology_node_2 = topology_node_3;
    } while (true);

    goto continue_;
  } // end IF

  for (DWORD i = 0;
       i < number_of_nodes;
       ++i)
  {
    result = collection_p->GetElement (i, &unknown_p);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_ASSERT (unknown_p);
    result = unknown_p->QueryInterface (IID_PPV_ARGS (&topology_node_2));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTopologyNode): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      unknown_p->Release (); unknown_p = NULL;
      goto error;
    } // end IF
    unknown_p->Release (); unknown_p = NULL;
    ACE_ASSERT (topology_node_2);

    result = topology_node_2->GetTopoNodeID (&node_id);
    ACE_ASSERT (SUCCEEDED (result));
    if (node_id == nodeId_in)
    {
      topology_node_2->Release (); topology_node_2 = NULL;
      continue;
    } // end IF

    break;
  } // end FOR
  if (!topology_node_2)
    goto use_source_node;
  collection_p->Release (); collection_p = NULL;

  result = topology_node_2->GetInputCount (&number_of_inputs);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_ASSERT (number_of_inputs > 0);
  result = topology_node_2->GetInput (0,
                                      &topology_node_3,
                                      &output_index);
  ACE_ASSERT (SUCCEEDED (result));

continue_:
  ACE_ASSERT (topology_node_p);
  ACE_ASSERT (topology_node_2);
  ACE_ASSERT (topology_node_3);

  result = topology_node_3->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_SOURCESTREAM_NODE:
    {
      // source node --> unknown contains a stream dscriptor handle
      IMFStreamDescriptor* stream_descriptor_p = NULL;
      result =
        topology_node_3->GetUnknown (MF_TOPONODE_STREAM_DESCRIPTOR,
                                     IID_PPV_ARGS (&stream_descriptor_p));
      ACE_ASSERT (SUCCEEDED (result));
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result));
      stream_descriptor_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&media_type_p);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release ();

      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      unknown_p = NULL;
      result = topology_node_3->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      IMFTransform* transform_p = NULL;
      result = unknown_p->QueryInterface (IID_PPV_ARGS (&transform_p));
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IUnknown::QueryInterface(IID_IMFTransform): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        unknown_p->Release (); unknown_p = NULL;
        topology_node_2->Release (); topology_node_2 = NULL;
        topology_node_3->Release (); topology_node_3 = NULL;
        goto error;
      } // end IF
      unknown_p->Release (); unknown_p = NULL;
      result = transform_p->GetOutputCurrentType (0,
                                                  &media_type_p);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::GetOutputCurrentType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        topology_node_2->Release (); topology_node_2 = NULL;
        topology_node_3->Release (); topology_node_3 = NULL;
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
                  node_type));
      topology_node_2->Release (); topology_node_2 = NULL;
      topology_node_3->Release (); topology_node_3 = NULL;
      goto error;
    }
  } // end SWITCH

  // step2: add a tee node ?
  if (!add_tee_node)
  {
    topology_node_4 = topology_node_3;
    topology_node_3 = NULL;
    goto continue_2;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_TEE_NODE,
                                 &topology_node_4);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TEE_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    topology_node_2->Release (); topology_node_2 = NULL;
    topology_node_3->Release (); topology_node_3 = NULL;
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
    topology_node_2->Release (); topology_node_2 = NULL;
    topology_node_3->Release (); topology_node_3 = NULL;
    topology_node_4->Release (); topology_node_4 = NULL;
    goto error;
  } // end IF
  node_id = 0;
  result = topology_node_4->GetTopoNodeID (&node_id);
  ACE_ASSERT (SUCCEEDED (result));
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("added tee node (id: %q)...\n"),
              node_id));

  // step3: connect the upstream node to the tee
  result = topology_node_3->ConnectOutput (0,
                                           topology_node_4,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    topology_node_2->Release (); topology_node_2 = NULL;
    topology_node_3->Release (); topology_node_3 = NULL;
    topology_node_4->Release (); topology_node_4 = NULL;
    goto error;
  } // end IF
  topology_node_3->Release ();
  result = topology_node_4->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (0,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_4->SetOutputPrefType (1,
                                               media_type_p);
  ACE_ASSERT (SUCCEEDED (result));

  // step4: connect the (two) outputs
  result = topology_node_4->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    topology_node_2->Release (); topology_node_2 = NULL;
    topology_node_4->Release (); topology_node_4 = NULL;
    goto error;
  } // end IF
  result = topology_node_2->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_2->Release (); topology_node_2 = NULL;

continue_2:
  result = topology_node_4->ConnectOutput ((add_tee_node ? 1 : 0),
                                           topology_node_p,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    topology_node_4->Release (); topology_node_4 = NULL;
    goto error;
  } // end IF
  result = topology_node_p->SetInputPrefType (0,
                                              media_type_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_type_p->Release ();
  topology_node_p->Release (); topology_node_p = NULL;
  topology_node_4->Release (); topology_node_4 = NULL;

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();

  return false;
}

bool
Stream_MediaFramework_MediaFoundation_Tools::clear (IMFMediaSession* mediaSession_in,
                                                    bool waitForCompletion_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::clear"));

  ACE_Time_Value timeout (STREAM_LIB_MEDIAFOUNDATION_TOPOLOGY_GET_TIMEOUT, 0);
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
      if (result != MF_E_MULTIPLE_SUBSCRIBERS) // 0xc00d36da
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return false;
      } // end IF
      else
      {
        received_topology_event = true;
        continue;
      } // end ELSE
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologiesCleared)
      received_topology_event = true;
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
  do
  { // *TODO*: this shouldn't block
    media_event_p = NULL;
    result = mediaSession_in->GetEvent (0,
                                        &media_event_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaSession::GetEvent(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_event_p);
    result = media_event_p->GetType (&event_type);
    ACE_ASSERT (SUCCEEDED (result));
    if (event_type == MESessionTopologySet)
      received_topology_event = true;
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
    result = topology_node_2->GetNodeType (&node_type_e);
    ACE_ASSERT (SUCCEEDED (result));
    switch (node_type_e)
    {
      case MF_TOPOLOGY_OUTPUT_NODE:
      {
        result = topology_node_2->GetObject (&unknown_p);
        ACE_ASSERT (SUCCEEDED (result) && unknown_p);
        IMFStreamSink* stream_sink_p = NULL;
        result = unknown_p->QueryInterface (IID_PPV_ARGS (&stream_sink_p));
        ACE_ASSERT (SUCCEEDED (result) && stream_sink_p);
        unknown_p->Release (); unknown_p = NULL;
        IMFMediaSink* media_sink_p = NULL;
        result = stream_sink_p->GetMediaSink (&media_sink_p);
        ACE_ASSERT (SUCCEEDED (result) && media_sink_p);
        stream_sink_p->Release (); stream_sink_p = NULL;
        result = media_sink_p->Shutdown ();
        ACE_ASSERT (SUCCEEDED (result));
        media_sink_p->Release (); media_sink_p = NULL;
        break;
      }
      case MF_TOPOLOGY_SOURCESTREAM_NODE:
      {
        result = topology_node_2->GetObject (&unknown_p);
        ACE_ASSERT (SUCCEEDED (result) && unknown_p);
        IMFMediaSource* media_source_p = NULL;
        result = topology_node_2->GetUnknown (MF_TOPONODE_SOURCE,
                                              IID_PPV_ARGS (&media_source_p));
        ACE_ASSERT (SUCCEEDED (result) && media_source_p);
        result = media_source_p->Shutdown ();
        ACE_ASSERT (SUCCEEDED (result));
        media_source_p->Release (); media_source_p = NULL;
        break;
      }
      default:
        break;
    } // end SWITCH
    result = topology_node_2->SetObject (NULL);
    ACE_ASSERT (SUCCEEDED (result));
    topology_node_2->Release (); topology_node_2 = NULL;
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("removed node (id was: %q)...\n"),
                node_id_2));
  } // end FOR

  return true;
}

//bool
//Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat (IMFSourceReader* sourceReader_in,
//                                             IMFMediaType*& mediaType_out)
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
                                                              IMFMediaType*& IMFMediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::getOutputFormat"));

  // sanity check(s)
  ACE_ASSERT (IMFTransform_in);
  if (IMFMediaType_out)
  {
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;
  } // end IF

  HRESULT result = S_OK;
  DWORD number_of_input_streams = 0;
  DWORD number_of_output_streams = 0;
  DWORD* input_stream_ids_p = NULL;
  DWORD* output_stream_ids_p = NULL;
  DWORD index = 0;
  struct _GUID media_subtype = GUID_NULL;
  bool prefer_rgb = true;
  bool prefer_chroma = false;

  result = IMFTransform_in->GetStreamCount (&number_of_input_streams,
                                            &number_of_output_streams);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTransform::GetStreamCount(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
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
                  ACE_TEXT ("failed to IMFTransform::GetStreamIDs(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
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
  input_stream_ids_p = NULL;

iterate:
  do
  {
    result = IMFTransform_in->GetOutputAvailableType (output_stream_ids_p[0],
                                                      index,
                                                      &IMFMediaType_out);
    if (FAILED (result))
    {
      if (result == MF_E_NO_MORE_TYPES)
        break;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFTransform::GetOutputAvailableType(%d): \"%s\", aborting\n"),
                  index,
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (IMFMediaType_out);

    result = IMFMediaType_out->GetGUID (MF_MT_SUBTYPE,
                                        &media_subtype);
    ACE_ASSERT (SUCCEEDED (result));
    ACE_DEBUG ((LM_DEBUG,
                //ACE_TEXT ("%s: output type %d: \"%s\"\n"),
                //ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::transformToString (IMFTransform_in).c_str ()),
                ACE_TEXT ("#%d: \"%s\"\n"),
                index,
                ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (media_subtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));

    if ((prefer_rgb &&
         Stream_MediaFramework_Tools::isRGB (media_subtype,
                                             STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION))             ||
        (prefer_chroma &&
         Stream_MediaFramework_Tools::isChromaLuminance (media_subtype,
                                                         STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION)) ||
        (!prefer_rgb && !prefer_chroma))
      break;

    // clean up
    IMFMediaType_out->Release (); IMFMediaType_out = NULL;

    ++index;
  } while (true);
  if (!IMFMediaType_out &&
      (prefer_rgb || prefer_chroma))
  {
    if (prefer_rgb)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("transform has no rgb output type(s), trying chroma-luminance\n")));
      prefer_rgb = false;
      prefer_chroma = true;
    } // end IF
    else if (prefer_chroma)
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("transform has no chroma-luminance output type(s), returning first format\n")));
      prefer_chroma = false;
    } // end ELSE IF
    index = 0;
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
  ACE_ASSERT (SUCCEEDED (result));
  result = collection_p->GetElementCount (&number_of_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_nodes <= 0)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("topology contains no output nodes, continuing\n")));
    collection_p->Release (); collection_p = NULL;
    result = topology_in->GetSourceNodeCollection (&collection_p);
    ACE_ASSERT (SUCCEEDED (result));
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
      goto error;
    } // end IF
    unknown_p->Release (); unknown_p = NULL;

    do
    {
      result = topology_node_p->GetOutputCount (&number_of_nodes);
      ACE_ASSERT (SUCCEEDED (result));
      if (number_of_nodes <= 0)
        break;

      topology_node_2 = NULL;
      result = topology_node_p->GetOutput (0,
                                           &topology_node_2,
                                           &input_index);
      ACE_ASSERT (SUCCEEDED (result));
      topology_node_p->Release ();
      topology_node_p = topology_node_2;
    } while (true);

    goto continue_;
  } // end IF

  result = collection_p->GetElement (0, &unknown_p);
  ACE_ASSERT (SUCCEEDED (result));
  collection_p->Release ();
  ACE_ASSERT (unknown_p);
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

continue_:
  if (!topology_node_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("output node not found [id was: %q], aborting\n"),
                nodeId_in));
    goto error;
  } // end IF

  result = topology_node_p->GetNodeType (&node_type);
  ACE_ASSERT (SUCCEEDED (result));
  switch (node_type)
  {
    case MF_TOPOLOGY_OUTPUT_NODE:
    {
      unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT (unknown_p);

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
      ACE_ASSERT (SUCCEEDED (result));
      IMFMediaTypeHandler* media_type_handler_p = NULL;
      result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
      ACE_ASSERT (SUCCEEDED (result));
      stream_descriptor_p->Release ();
      result = media_type_handler_p->GetCurrentMediaType (&IMFMediaType_out);
      ACE_ASSERT (SUCCEEDED (result));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      break;
    }
    case MF_TOPOLOGY_TRANSFORM_NODE:
    {
      unknown_p = NULL;
      result = topology_node_p->GetObject (&unknown_p);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_ASSERT (unknown_p);

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

//std::string
//Stream_MediaFramework_MediaFoundation_Tools::transformToString (IMFTransform* IMFTransform_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::transformToString"));
//
//  std::string result;
//
//  IMFAttributes* attributes_p = NULL;
//  HRESULT result_2 = IMFTransform_in->GetAttributes (&attributes_p);
//  ACE_ASSERT (SUCCEEDED (result_2));
//  WCHAR buffer[BUFSIZ];
//  result_2 = attributes_p->GetString (MFT_FRIENDLY_NAME_Attribute,
//                                      buffer, sizeof (buffer),
//                                      NULL);
//  if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
//  {
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("failed to IMFAttributes::GetString(MFT_FRIENDLY_NAME_Attribute): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
//    goto error;
//  } // end IF
//  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer));
//
//error:
//  if (attributes_p)
//    attributes_p->Release ();
//
//  return result;
//}

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
Stream_MediaFramework_MediaFoundation_Tools::toString (const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_MediaFramework_MediaFoundation_Tools::toString"));

  std::string result;

  //struct _AMMediaType media_type_s;
  //ACE_OS::memset (&media_type_s, 0, sizeof (struct _AMMediaType));
  //HRESULT result_2 =
  //  MFInitAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (mediaType_in),
  //                                    GUID_NULL, // auto-deduce format type
  //                                    &media_type_s);
  //if (FAILED (result_2)) // MF_E_ATTRIBUTENOTFOUND: 0xC00D36E6L
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to MFInitAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
  //              ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
  //  return std::string ();
  //} // end IF

  //result =
  //  Stream_MediaFramework_DirectShow_Tools::toString (media_type_s);

  //// clean up
  //Stream_MediaFramework_DirectShow_Tools::freeMediaType (media_type_s);
  struct _GUID guid_s = GUID_NULL;
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
  BOOL uses_temporal_compression = FALSE;
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
    { ACE_ASSERT (value_v.vt == VT_UINT);
      result += ACE_TEXT_ALWAYS_CHAR ("\nindependent samples: ");
      result +=
        (value_v.uintVal ? ACE_TEXT_ALWAYS_CHAR ("true")
                         : ACE_TEXT_ALWAYS_CHAR ("false"));
    } // end IF
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
    { ACE_ASSERT (value_v.vt == VT_UINT);
      result += ACE_TEXT_ALWAYS_CHAR ("\nfixed size samples: ");
      result +=
        (value_v.uintVal ? ACE_TEXT_ALWAYS_CHAR ("true")
                         : ACE_TEXT_ALWAYS_CHAR ("false"));
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_MAJOR_TYPE))
    { ACE_ASSERT (value_v.vt == VT_CLSID);
    } // end ELSE IF
    else if (InlineIsEqualGUID (guid_s, MF_MT_SAMPLE_SIZE))
    { ACE_ASSERT (value_v.vt == VT_UINT);
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
