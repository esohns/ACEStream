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

#include "stream_dev_mediafoundation_tools.h"

#include "oleauto.h"

#include "dmoreg.h"
#include "dshow.h"
#include "dvdmedia.h"
#include "Dmodshow.h"
#undef GetObject
#include "evr.h"
#include "ks.h"
#include "ksmedia.h"
#include "qedit.h"

#include "mfapi.h"
#include "mferror.h"

#include "wmcodecdsp.h"

#include <sstream>

#include "ace/Log_Msg.h"
#include "ace/OS.h"

#include "common_os_tools.h"

#include "common_time_common.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_lib_defines.h"
#include "stream_lib_mediafoundation_tools.h"
#include "stream_lib_tools.h"

struct Stream_Device_Identifier
Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice (REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice"));

  // initialize return value(s)
  struct Stream_Device_Identifier result;

  Stream_Device_List_t devices_a =
    Stream_Device_MediaFoundation_Tools::getCaptureDevices (deviceCategory_in);
  if (likely (!devices_a.empty ()))
    return devices_a.front ();

  return result;
}

Stream_Device_List_t
Stream_Device_MediaFoundation_Tools::getCaptureDevices (REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::getCaptureDevices"));

  // initialize return value(s)
  Stream_Device_List_t result;

  // sanity check(s)
  if (!InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID) &&
      !InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return result;
  } // end ELSE

  HRESULT result_2 = E_FAIL;
  IMFActivate** devices_pp = NULL;
  UINT32 count = 0;
  IMFAttributes* attributes_p = NULL;
  UINT32 length = 0;
  WCHAR buffer_a[BUFSIZ];
  struct Stream_Device_Identifier device_identifier_s;
  device_identifier_s.identifierDiscriminator = Stream_Device_Identifier::STRING;

  result_2 = MFCreateAttributes (&attributes_p, 1);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error_2;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  result_2 =
    attributes_p->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           deviceCategory_in);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error_2;
  } // end IF

  result_2 = MFEnumDeviceSources (attributes_p,
                                  &devices_pp,
                                  &count);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    goto error_2;
  } // end IF
#else
  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP_RETURN (false);
  ACE_NOTREACHED (return false;)
#endif // _WIN32_WINNT_WIN7
  attributes_p->Release (); attributes_p = NULL;
  ACE_ASSERT (devices_pp);

  for (UINT32 index = 0; index < count; index++)
  {
    ACE_OS::memset (buffer_a, 0, sizeof (WCHAR[BUFSIZ]));
    length = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
    result_2 =
      devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                    buffer_a, sizeof (WCHAR[BUFSIZ]),
                                    &length);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error_2;
    } // end IF
    device_identifier_s.description =
      ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a));

    ACE_OS::memset (buffer_a, 0, sizeof (WCHAR[BUFSIZ]));
    length = 0;
    result_2 =
      devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
      //devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID,
                                    buffer_a, sizeof (WCHAR[BUFSIZ]),
                                    &length);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error_2;
    } // end IF
    ACE_ASSERT (length < BUFSIZ);
    ACE_OS::strcpy (device_identifier_s.identifier._string,
                    ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a)));
    //device_identifier_s.identifier._guid =
    //  Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a)));
#else
    ACE_ASSERT (false); // *TODO*
    ACE_NOTSUP_RETURN (false);
    ACE_NOTREACHED (return false;)
#endif // _WIN32_WINNT_WIN7
    result.push_back (device_identifier_s);
  } // end FOR

error_2:
  if (attributes_p)
    attributes_p->Release ();
  if (devices_pp)
  {
    for (UINT32 i = 0; i < count; i++)
      devices_pp[i]->Release ();
    CoTaskMemFree (devices_pp);
  } // end IF

  return result;
}

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
bool
Stream_Device_MediaFoundation_Tools::loadDeviceTopology (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                         REFGUID deviceCategory_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
                                                         IMFMediaSourceEx*& mediaSource_inout,
#else
                                                         IMFMediaSource*& mediaSource_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                         IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                         IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                         IMFTopology*& topology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::loadDeviceTopology"));

  // initialize return value(s)
  if (topology_out)
  {
    Stream_MediaFramework_MediaFoundation_Tools::clear (topology_out,
                                                        true);
    topology_out->Release (); topology_out = NULL;
  } // end IF

  struct _GUID major_type_s = GUID_NULL;
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
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
  if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))
  {
    major_type_s = MFMediaType_Video;
    result = topology_out->SetUINT32 (MF_TOPOLOGY_ENABLE_XVP_FOR_PLAYBACK,
                                      TRUE);
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  else if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID))
  {
    major_type_s = MFMediaType_Audio;
  } // end ELSE IF
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category: \"%s\", aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceCategory_in).c_str ())));
    goto error;
  } // end ELSE
  result = topology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                    MFTOPOLOGY_DXVA_FULL);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_STATIC_PLAYBACK_OPTIMIZATIONS,
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                    TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_out->SetUINT32 (MF_TOPOLOGY_HARDWARE_MODE,
                                    MFTOPOLOGY_HWMODE_USE_HARDWARE);
  ACE_ASSERT (SUCCEEDED (result));
  // MF_TOPOLOGY_ENABLE_DXGI_TRANSCODE_BRANCH_LOADER: {BFE3335C-490E-47D1-AB70-4F2416F467BA}
  result = topology_out->SetUINT32 (Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{BFE3335C-490E-47D1-AB70-4F2416F467BA}")),
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  // *TODO*: {2BE0B4FB-B6C6-4768-9A79-B28A96DAE116}
  result = topology_out->SetUINT32 (Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{2BE0B4FB-B6C6-4768-9A79-B28A96DAE116}")),
                                    0);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0600) // _WIN32_WINNT_VISTA
  result = topology_out->SetUINT32 (MF_TOPOLOGY_NO_MARKIN_MARKOUT,
                                    TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)
  result = topology_out->SetUINT64 (MF_TOPOLOGY_START_TIME_ON_PRESENTATION_SWITCH,
                                    0);
  ACE_ASSERT (SUCCEEDED (result));
  // MF_TOPOLOGY_AUDIO_FORMAT_CHANGE_ON_START = {5AB2CFB2-EA8D-4961-99F2-DF0555F488F3}
  result = topology_out->SetUINT32 (Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{5AB2CFB2-EA8D-4961-99F2-DF0555F488F3}")),
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  // MF_TOPOLOGY_MFPLAYEX = {E48E1D3B-859A-40EC-928E-A5889EF0B458}
  result = topology_out->SetUINT32 (Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{E48E1D3B-859A-40EC-928E-A5889EF0B458}")),
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  // MF_TOPOLOGY_ENABLE_TRANSCODE_BRANCH_LOADER = {F297151B-1410-4936-A111-6D103A461F34}
  result = topology_out->SetUINT32 (Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{F297151B-1410-4936-A111-6D103A461F34}")),
                                    FALSE);
  ACE_ASSERT (SUCCEEDED (result));
  //// MF_TOPOLOGY_D3D_MANAGER: {66289BFB-1DF1-4951-A97A-D7BD1D03AC76}
  //result = topology_out->SetUnknown (Common_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{66289BFB-1DF1-4951-A97A-D7BD1D03AC76}")),
  //                                   NULL);
  //ACE_ASSERT (SUCCEEDED (result));

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
  // *TODO*: {4DB04908-0D94-47B3-933E-86BDAA16FA77}
  result =
    topology_node_p->SetUINT32 (Common_OS_Tools::StringToGUID (ACE_TEXT_ALWAYS_CHAR ("{4DB04908-0D94-47B3-933E-86BDAA16FA77}")),
                                0);
  ACE_ASSERT (SUCCEEDED (result));
  //MF_TOPONODE_WORKQUEUE_ID
  //MF_TOPONODE_WORKQUEUE_MMCSS_CLASS
  result = topology_node_p->SetUINT32 (MF_TOPONODE_LOCKED,
                                       TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT64 (MF_TOPONODE_MEDIASTART,
                                       0);
  ACE_ASSERT (SUCCEEDED (result));
  if (!mediaSource_inout)
  {
    if (!Stream_Device_MediaFoundation_Tools::getMediaSource (deviceIdentifier_in,
                                                              deviceCategory_in,
                                                              mediaSource_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getMediaSource(), aborting\n")));
      goto error;
    } // end IF
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

  result = topology_node_p->SetUINT32 (MF_TOPONODE_MARKIN_HERE, TRUE);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_p->SetUINT32 (MF_TOPONODE_MARKOUT_HERE, TRUE);
  ACE_ASSERT (SUCCEEDED (result));

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
              ACE_TEXT ("%q: added source node: \"%s\"...\n"),
              node_id,
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (mediaSource_inout).c_str ())));
  node_id = 0;

  if (!sampleGrabberSinkCallback_in)
  {
    // *IMPORTANT NOTE*: apparently the evr can only handle rgb32, explicitly: not rgb24
    Stream_Device_MediaFoundation_Tools::getCaptureFormat (mediaSource_inout,
                                                           media_type_p);
    ACE_ASSERT (media_type_p);
    struct _GUID format_s =
      Stream_MediaFramework_MediaFoundation_Tools::toFormat (media_type_p);
    if (InlineIsEqualGUID (format_s, MFVideoFormat_RGB24))
    {
      // add color converter dmo
      MFT_REGISTER_TYPE_INFO mft_register_type_info;
      mft_register_type_info.guidMajorType = MFMediaType_Video;
      mft_register_type_info.guidSubtype = MFVideoFormat_RGB24;
      UINT32 flags = 0;
      IMFActivate** decoders_p = NULL;
      UINT32 item_count = 0;
      std::string module_string;
      IMFTransform* transform_p = NULL;
      bool error_b = false;
      DWORD i = 0;
      struct _GUID sub_type = GUID_NULL;
      IMFMediaType* media_type_2 = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_PROCESSOR,
#else
      if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_VIDEO_DECODER,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                              flags,
                                                              &mft_register_type_info,    // input type
                                                              NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
                                                              decoders_p,                 // array of decoders
#else
                                                              NULL,                       // attributes
                                                              decoders_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
                                                              item_count))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                    ACE_TEXT (Common_OS_Tools::GUIDToString (MFT_CATEGORY_VIDEO_DECODER).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (decoders_p);
      if (!item_count)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
        error_b = true;
        goto clean_2;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
      module_string =
        Stream_MediaFramework_MediaFoundation_Tools::toString (decoders_p[0]);

      result = decoders_p[0]->ActivateObject (IID_PPV_ARGS (&transform_p));
      ACE_ASSERT (SUCCEEDED (result));

clean_2:
      for (UINT32 i = 0; i < item_count; i++)
        decoders_p[i]->Release ();
      CoTaskMemFree (decoders_p);
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
clean_2:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0601)
      if (error_b)
      {
        media_type_p->Release (); media_type_p = NULL;
        goto error;
      } // end IF
      ACE_ASSERT (transform_p);

      result = transform_p->SetInputType (0,
                                          media_type_p,
                                          0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::SetInputType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_p->Release (); media_type_p = NULL;
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      result = MFCreateTopologyNode (MF_TOPOLOGY_TRANSFORM_NODE,
                                     &topology_node_2);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_TRANSFORM_NODE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_p->Release (); media_type_p = NULL;
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
      ACE_ASSERT (topology_node_2);
    #endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      result = topology_node_2->SetObject (transform_p);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_node_2->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                           MF_CONNECT_DIRECT);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_node_2->SetUINT32 (MF_TOPONODE_D3DAWARE,
                                           TRUE);
      ACE_ASSERT (SUCCEEDED (result));
      result = topology_out->AddNode (topology_node_2);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopology::AddNode(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      result = topology_node_2->GetTopoNodeID (&node_id);
      ACE_ASSERT (SUCCEEDED (result));
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("added transform node (id: %q)...\n"),
      //            node_id));
      result = topology_node_p->ConnectOutput (0,
                                               topology_node_2,
                                               0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
      topology_node_p->Release ();
      topology_node_p = topology_node_2;
      topology_node_2 = NULL;

      while (!InlineIsEqualGUID (sub_type, MFVideoFormat_RGB32))
      {
        if (media_type_2)
        {
          media_type_2->Release (); media_type_2 = NULL;
        } // end IF
        result = transform_p->GetOutputAvailableType (0,
                                                      i,
                                                      &media_type_2);

        ACE_ASSERT (SUCCEEDED (result));
        result = media_type_2->GetGUID (MF_MT_SUBTYPE,
                                        &sub_type);
        ACE_ASSERT (SUCCEEDED (result));
        ++i;
      } // end WHILE
      //result = media_type_p->DeleteAllItems ();
      //ACE_ASSERT (SUCCEEDED (result));
      Stream_MediaFramework_MediaFoundation_Tools::copy (media_type_p,
                                                         media_type_2,
                                                         MF_MT_FRAME_RATE);
      Stream_MediaFramework_MediaFoundation_Tools::copy (media_type_p,
                                                         media_type_2,
                                                         MF_MT_FRAME_SIZE);
      Stream_MediaFramework_MediaFoundation_Tools::copy (media_type_p,
                                                         media_type_2,
                                                         MF_MT_INTERLACE_MODE);
      Stream_MediaFramework_MediaFoundation_Tools::copy (media_type_p,
                                                         media_type_2,
                                                         MF_MT_PIXEL_ASPECT_RATIO);
      Stream_MediaFramework_MediaFoundation_Tools::copy (media_type_p,
                                                         media_type_2,
                                                         MF_MT_ALL_SAMPLES_INDEPENDENT);
      result = transform_p->SetOutputType (0,
                                           media_type_2,
                                           0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFTransform::SetOutputType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_2->Release (); media_type_2 = NULL;
        transform_p->Release (); transform_p = NULL;
        goto error;
      } // end IF
      media_type_2->Release (); media_type_2 = NULL;
#if defined (_DEBUG)
      result = transform_p->GetOutputCurrentType (0,
                                                  &media_type_2);
      ACE_ASSERT (SUCCEEDED (result));
      result = media_type_2->GetGUID (MF_MT_SUBTYPE,
                                      &sub_type);
      ACE_ASSERT (SUCCEEDED (result));
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: output format: \"%s\"...\n"),
                  ACE_TEXT (module_string.c_str ()),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (sub_type, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
#endif // _DEBUG

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
      result =
        transform_p->QueryInterface (IID_PPV_ARGS (&video_processor_control_p));
      if (FAILED (result))
        goto continue_near;
      ACE_ASSERT (video_processor_control_p);
      //result = video_processor_control_p->SetRotation (ROTATION_NORMAL);
      //ACE_ASSERT (SUCCEEDED (result));
      //result = video_processor_control_p->SetMirror (MIRROR_VERTICAL);
      //ACE_ASSERT (SUCCEEDED (result));
      //result = video_processor_control_p->EnableHardwareEffects (TRUE);
      //ACE_ASSERT (SUCCEEDED (result));
      result =
        video_processor_control_p->SetRotationOverride (MFVideoRotationFormat_180);
      ACE_ASSERT (SUCCEEDED (result));
      video_processor_control_p->Release (); video_processor_control_p = NULL;
continue_near:
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
      transform_p->Release (); transform_p = NULL;

      // debug info
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%q: added processor for \"%s\": \"%s\"...\n"),
                  node_id,
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (mft_register_type_info.guidSubtype, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ()),
                  ACE_TEXT (module_string.c_str ())));
    } // end IF

    if (!Stream_MediaFramework_MediaFoundation_Tools::addRenderer (major_type_s,
                                                                   NULL,
                                                                   GUID_NULL, // *TODO*: this is not the null-renderer !
                                                                   topology_out,
                                                                   node_id,
                                                                   true))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addRenderer(), aborting\n")));
      goto error;
    } // end IF
    goto continue_;
  } // end IF

  if (!Stream_MediaFramework_MediaFoundation_Tools::addGrabber (sampleGrabberSinkCallback_in,
                                                                topology_out,
                                                                node_id))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::addGrabber(), aborting\n")));
    goto error;
  } // end IF

continue_:
  media_type_p->Release (); media_type_p = NULL;

  return true;

error:
  if (media_type_p)
    media_type_p->Release ();
  if (topology_node_p)
    topology_node_p->Release ();
  if (topology_node_2)
    topology_node_2->Release ();
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (topology_out)
  {
    Stream_MediaFramework_MediaFoundation_Tools::clear (topology_out,
                                                        true);
    topology_out->Release (); topology_out = NULL;
  } // end IF

  return false;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0600)

bool
#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0602) // _WIN32_WINNT_WIN8
Stream_Device_MediaFoundation_Tools::getCaptureFormat (IMFMediaSourceEx* mediaSource_in,
#else
Stream_Device_MediaFoundation_Tools::getCaptureFormat (IMFMediaSource* mediaSource_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM (0x0602)
                                                       IMFMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (mediaSource_in);
  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  BOOL is_selected = FALSE;

  HRESULT result =
    mediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
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
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;
  result = media_type_handler_p->GetCurrentMediaType (&mediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetCurrentMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release (); media_type_handler_p = NULL;

  return true;

error:
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();
  if (mediaType_out)
  {
    mediaType_out->Release (); mediaType_out = NULL;
  } // end IF

  return false;
}
//bool
//Stream_Device_MediaFoundation_Tools::getCaptureFormat (IMFSourceReader* sourceReader_in,
//                                                              IMFMediaType*& mediaType_out)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::getCaptureFormat"));
//
//  // sanity check(s)
//  ACE_ASSERT (sourceReader_in);
//  if (mediaType_out)
//  {
//    mediaType_out->Release (); mediaType_out = NULL;
//  } // end IF
//
//  HRESULT result =
//    sourceReader_in->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                          &mediaType_out);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFSourceReader::GetCurrentMediaType(): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//  ACE_ASSERT (mediaType_out);
//
//  return true;
//}
//

//bool
//Stream_Device_MediaFoundation_Tools::setCaptureFormat (IMFSourceReaderEx* IMFSourceReaderEx_in,
//                                                              const IMFMediaType* mediaType_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::setCaptureFormat"));
//
//  // sanit ycheck(s)
//  ACE_ASSERT (IMFSourceReaderEx_in);
//  ACE_ASSERT (mediaType_in);
//
//  HRESULT result = E_FAIL;
//  struct _GUID GUID_s = GUID_NULL;
//  UINT32 width, height;
//  UINT32 numerator, denominator;
//  result =
//    const_cast<IMFMediaType*> (mediaType_in)->GetGUID (MF_MT_SUBTYPE,
//                                                       &GUID_s);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//  result = MFGetAttributeSize (const_cast<IMFMediaType*> (mediaType_in),
//                               MF_MT_FRAME_SIZE,
//                               &width, &height);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//  result = MFGetAttributeRatio (const_cast<IMFMediaType*> (mediaType_in),
//                                MF_MT_FRAME_RATE,
//                                &numerator, &denominator);
//  if (FAILED (result))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
//                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//    return false;
//  } // end IF
//
//  DWORD count = 0;
//  IMFMediaType* media_type_p = NULL;
//  struct _GUID GUID_2 = GUID_NULL;
//  UINT32 width_2, height_2;
//  UINT32 numerator_2, denominator_2;
//  DWORD flags = 0;
//  while (result == S_OK)
//  {
//    media_type_p = NULL;
//    result =
//      IMFSourceReaderEx_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                                count,
//                                                &media_type_p);
//    if (result != S_OK) break;
//
//    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      media_type_p->Release (); media_type_p = NULL;
//      return false;
//    } // end IF
//    if (!InlineIsEqualGUID (GUID_s, GUID_2))
//      goto continue_;
//
//    result = MFGetAttributeSize (media_type_p,
//                                 MF_MT_FRAME_SIZE,
//                                 &width_2, &height_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFGetAttributeSize(MF_MT_FRAME_SIZE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      media_type_p->Release (); media_type_p = NULL;
//      return false;
//    } // end IF
//    if (width != width_2)
//      goto continue_;
//
//    result = MFGetAttributeRatio (media_type_p,
//                                  MF_MT_FRAME_RATE,
//                                  &numerator_2, &denominator_2);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      media_type_p->Release (); media_type_p = NULL;
//      return false;
//    } // end IF
//    if ((numerator   != numerator_2)  ||
//        (denominator != denominator_2))
//      goto continue_;
//
//    result =
//      IMFSourceReaderEx_in->SetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
//                                                media_type_p,
//                                                &flags);
//    if (FAILED (result))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to IMFSourceReader::SetNativeMediaType(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT (Stream_Device_MediaFoundation_Tools::mediaTypeToString (media_type_p).c_str ()),
//                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
//      media_type_p->Release (); media_type_p = NULL;
//      return false;
//    } // end IF
//    media_type_p->Release (); media_type_p = NULL;
//
//    return true;
//
//continue_:
//    media_type_p->Release (); media_type_p = NULL;
//
//    ++count;
//  } // end WHILE
//
//  // *NOTE*: this means that the device does not support the requested media
//  //         type 'natively'
//  //         --> try to auto-load a (MFT/DMO) decoder
//  //             see: https://msdn.microsoft.com/en-us/library/windows/desktop/dd389281(v=vs.85).aspx#setting_output_formats
//  if (!Stream_Device_MediaFoundation_Tools::setOutputFormat (IMFSourceReaderEx_in,
//                                                    mediaType_in))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::setOutputFormat(), aborting\n")));
//    goto error;
//  } // end IF
//
//  return true;
//
//error:
//  ACE_DEBUG ((LM_ERROR,
//              ACE_TEXT ("the source reader does not support the requested media type (was: \"%s\"), aborting\n"),
//              ACE_TEXT (Stream_Device_MediaFoundation_Tools::mediaTypeToString (mediaType_in).c_str ())));
//
//  // debug info
//  Stream_Device_MediaFoundation_Tools::dump (IMFSourceReaderEx_in);
//
//  return false;
//}
bool
Stream_Device_MediaFoundation_Tools::setCaptureFormat (IMFTopology* IMFTopology_in,
                                                       const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::setCaptureFormat"));

  // sanit ycheck(s)
  ACE_ASSERT (IMFTopology_in);
  ACE_ASSERT (mediaType_in);

  HRESULT result = E_FAIL;
  IMFTopologyNode* topology_node_p = NULL;
  IMFCollection* collection_p = NULL;
  result = IMFTopology_in->GetSourceNodeCollection (&collection_p);
  ACE_ASSERT (SUCCEEDED (result));
  DWORD number_of_source_nodes = 0;
  result = collection_p->GetElementCount (&number_of_source_nodes);
  ACE_ASSERT (SUCCEEDED (result));
  if (number_of_source_nodes <= 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("topology contains no source nodes, aborting\n")));
    collection_p->Release (); collection_p = NULL;
    return false;
  } // end IF
  IUnknown* unknown_p = NULL;
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
    return false;
  } // end IF
  unknown_p->Release (); unknown_p = NULL;

#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
  result = topology_node_p->GetUnknown (MF_TOPONODE_SOURCE,
                                        IID_PPV_ARGS (&media_source_p));
  ACE_ASSERT (SUCCEEDED (result));
  topology_node_p->Release ();
  if (!Stream_Device_MediaFoundation_Tools::setCaptureFormat (media_source_p,
                                                              mediaType_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::setCaptureFormat(), aborting\n")));
    media_source_p->Release (); media_source_p = NULL;
    return false;
  } // end IF
  media_source_p->Release (); media_source_p = NULL;

  return true;
}

bool
#if defined (_WIN32_WINNT) && (_WIN32_WINNT > 0x0602) // _WIN32_WINNT_WIN8
Stream_Device_MediaFoundation_Tools::setCaptureFormat (IMFMediaSourceEx* mediaSource_in,
#else
Stream_Device_MediaFoundation_Tools::setCaptureFormat (IMFMediaSource* mediaSource_in,
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
                                                       const IMFMediaType* mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::setCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (mediaSource_in);
  ACE_ASSERT (mediaType_in);

  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  BOOL is_selected = FALSE;
  IMFMediaType* media_type_p = NULL;

  HRESULT result =
    mediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
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
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;
  ACE_ASSERT (media_type_handler_p);
  if (FAILED (media_type_handler_p->IsMediaTypeSupported (const_cast<IMFMediaType*> (mediaType_in),
                                                          &media_type_p)))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::IsMediaTypeSupported(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (mediaType_in, false).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
#if defined (_DEBUG)
    DWORD count_i = 0;
    result = media_type_handler_p->GetMediaTypeCount (&count_i);
    ACE_ASSERT (SUCCEEDED (result));
    IMFMediaType* media_type_2 = NULL;
    for (DWORD i = 0; i < count_i; i++)
    {
      result = media_type_handler_p->GetMediaTypeByIndex (i,
                                                          &media_type_2);
      ACE_ASSERT (SUCCEEDED (result) && media_type_2);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("supported media type #%d:\n%s\n"),
                  i,
                  ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (media_type_2, true).c_str ())));
      media_type_2->Release (); media_type_2 = NULL;
    } // end FOR
#endif // DEBUG

    if (media_type_p)
    {
      media_type_p->Release (); media_type_p = NULL;
    } // end IF
    goto error;
  } // end IF
  result =
    media_type_handler_p->SetCurrentMediaType (const_cast<IMFMediaType*> (mediaType_in));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaTypeHandler::SetCurrentMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  media_type_handler_p->Release (); media_type_handler_p = NULL;

  return true;

error:
  if (presentation_descriptor_p)
    presentation_descriptor_p->Release ();
  if (stream_descriptor_p)
    stream_descriptor_p->Release ();
  if (media_type_handler_p)
    media_type_handler_p->Release ();

  return false;
}

bool
Stream_Device_MediaFoundation_Tools::getMediaSource (const struct Stream_Device_Identifier& deviceIdentifier_in,
                                                     REFGUID deviceCategory_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                     IMFMediaSourceEx*& mediaSource_out)
#else
                                                     IMFMediaSource*& mediaSource_out)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::getMediaSource"));

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

  switch (deviceIdentifier_in.identifierDiscriminator)
  {
    case Stream_Device_Identifier::ID:
    case Stream_Device_Identifier::GUID:
      device_identifier_string =
        Stream_MediaFramework_MediaFoundation_Tools::identifierToString (deviceIdentifier_in,
                                                                         deviceCategory_in);
      break;
    case Stream_Device_Identifier::STRING:
      device_identifier_string = deviceIdentifier_in.identifier._string;
      break;
    default:
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown device identifier discriminator (was: %d), aborting\n"),
                  deviceIdentifier_in.identifierDiscriminator));
      goto error;
  } // end SWITCH
  if (device_identifier_string.empty ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::identifierToString(\"%s\"), aborting\n"),
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceIdentifier_in).c_str ())));
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM (0x0601) // _WIN32_WINNT_WIN7
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
                ACE_TEXT (Common_OS_Tools::GUIDToString (deviceCategory_in).c_str ())));
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
#if defined (_DEBUG)
      // sanity check(s)
      length = 0;
      result_2 =
        devices_pp[index]->GetStringLength (GUID_s,
                                            &length);
      ACE_ASSERT (SUCCEEDED (result_2) && (length < sizeof (WCHAR[BUFSIZ])));
#endif // _DEBUG
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
                    ACE_TEXT (Common_OS_Tools::GUIDToString (GUID_s).c_str ()),
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

struct _GUID
Stream_Device_MediaFoundation_Tools::symbolicLinkToGUID (const std::string& symbolicLink_in)
{
  struct _GUID result_s = GUID_NULL;

  std::string::size_type offset_i = symbolicLink_in.find ('#', 0);
  ACE_ASSERT (offset_i != std::string::npos);
  offset_i = symbolicLink_in.find ('#', offset_i + 1);
  ACE_ASSERT (offset_i != std::string::npos);
  offset_i = symbolicLink_in.find ('#', offset_i + 1);
  ACE_ASSERT (offset_i != std::string::npos);
  std::string::size_type offset_2 = symbolicLink_in.find_last_of ('\\');
  ACE_ASSERT (offset_2 != std::string::npos);
  std::string GUID_string = symbolicLink_in.substr (offset_i + 1, offset_2 - (offset_i + 1));

  result_s = Common_OS_Tools::StringToGUID (GUID_string);

  return result_s;
}
