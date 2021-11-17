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

#include <sstream>

#include <oleauto.h>

#include <dmoreg.h>
#include <dshow.h>
//#include <dsound.h>
#include <dvdmedia.h>
#include <Dmodshow.h>
#include <evr.h>
//#include <fourcc.h>
#include <ks.h>
#include <ksmedia.h>
 //#include <ksuuids.h>
#include <qedit.h>

#include <mfapi.h>
#include <mferror.h>
//#include <mftransform.h>

#include <wmcodecdsp.h>

#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common_time_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "stream_lib_defines.h"
#include "stream_lib_mediafoundation_tools.h"

void
Stream_Device_MediaFoundation_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::initialize"));

}

std::string
Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice (REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::getDefaultCaptureDevice"));

  // initialize return value(s)
  std::string result;

  Stream_Device_List_t devices_a =
    Stream_Device_MediaFoundation_Tools::getCaptureDevices (deviceCategory_in);
  if (likely (!devices_a.empty ()))
  {
    const struct Stream_Device_Identifier& device_identifier_s =
      devices_a.front ();
    ACE_ASSERT (device_identifier_s.identifierDiscriminator == Stream_Device_Identifier::STRING);
    result = device_identifier_s.identifier._string;
  } // end IF

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
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
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
                                    buffer_a, sizeof (WCHAR[BUFSIZ]),
                                    &length);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      goto error_2;
    } // end IF
    ACE_OS::strcpy (device_identifier_s.identifier._string,
                    ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a)));
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

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
bool
Stream_Device_MediaFoundation_Tools::loadDeviceTopology (const std::string& deviceIdentifier_in,
                                                         REFGUID deviceCategory_in,
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
                                                         IMFMediaSourceEx*& mediaSource_inout,
#else
                                                         IMFMediaSource*& mediaSource_inout,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                         IMFSampleGrabberSinkCallback2* sampleGrabberSinkCallback_in,
#else
                                                         IMFSampleGrabberSinkCallback* sampleGrabberSinkCallback_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                         IMFTopology*& topology_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_MediaFoundation_Tools::loadDeviceTopology"));

  // initialize return value(s)
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

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
  if (InlineIsEqualGUID (deviceCategory_in, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID))
  {
    result = topology_out->SetUINT32 (MF_TOPOLOGY_DXVA_MODE,
                                      MFTOPOLOGY_DXVA_FULL);
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  result = topology_out->SetUINT32 (MF_TOPOLOGY_ENUMERATE_SOURCE_TYPES,
                                    TRUE);
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
    if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource (deviceIdentifier_in,
                                                                      deviceCategory_in,
                                                                      mediaSource_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(\"%s\"), aborting\n"),
                  ACE_TEXT (deviceIdentifier_in.c_str ())));
      goto error;
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
              ACE_TEXT ("%q: added source node: \"%s\"...\n"),
              node_id,
              ACE_TEXT (Stream_MediaFramework_MediaFoundation_Tools::toString (mediaSource_inout).c_str ())));

  // *NOTE*: add 'dummy' sink node so the topology can be loaded ?
  if (!sampleGrabberSinkCallback_in)
    goto continue_;

  if (!Stream_Device_MediaFoundation_Tools::getCaptureFormat (mediaSource_inout,
                                                              media_type_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getCaptureFormat(), aborting\n")));
    goto error;
  } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result =
    MFCreateSampleGrabberSinkActivate (media_type_p,
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
                ACE_TEXT ("failed to MFCreateSampleGrabberSinkActivate(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    media_type_p->Release (); media_type_p = NULL;
    goto error;
  } // end IF
  media_type_p->Release (); media_type_p = NULL;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
  result = activate_p->SetUINT32 (MF_SAMPLEGRABBERSINK_IGNORE_CLOCK,
                                  TRUE);
  ACE_ASSERT (SUCCEEDED (result));
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)

  result = activate_p->ActivateObject (IID_PPV_ARGS (&media_sink_p));
  ACE_ASSERT (SUCCEEDED (result));
  activate_p->Release (); activate_p = NULL;
  result = media_sink_p->GetStreamSinkByIndex (0,
                                               &stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  media_sink_p->Release (); media_sink_p = NULL;

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
  result = MFCreateTopologyNode (MF_TOPOLOGY_OUTPUT_NODE,
                                 &topology_node_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (topology_node_2);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
  result = topology_node_2->SetObject (stream_sink_p);
  ACE_ASSERT (SUCCEEDED (result));
  stream_sink_p->Release (); stream_sink_p = NULL;
  result = topology_node_2->SetUINT32 (MF_TOPONODE_CONNECT_METHOD,
                                       MF_CONNECT_DIRECT);
  ACE_ASSERT (SUCCEEDED (result));
  result = topology_node_2->SetUINT32 (MF_TOPONODE_STREAMID, 0);
  ACE_ASSERT (SUCCEEDED (result));
  //result = topology_node_2->SetUINT32 (MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);
  //ACE_ASSERT (SUCCEEDED (result));
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
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%q: added 'dummy' sink node...\n"),
              node_id));
  result = topology_node_p->ConnectOutput (0,
                                           topology_node_2,
                                           0);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFTopologyNode::ConnectOutput(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    goto error;
  } // end IF
  topology_node_2->Release (); topology_node_2 = NULL;
continue_:
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
  if (topology_out)
  {
    topology_out->Release (); topology_out = NULL;
  } // end IF

  return false;
}
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

bool
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
Stream_Device_MediaFoundation_Tools::getCaptureFormat (IMFMediaSourceEx* mediaSource_in,
#else
Stream_Device_MediaFoundation_Tools::getCaptureFormat (IMFMediaSource* mediaSource_in,
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
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
