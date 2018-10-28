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

#include "ace/Synch.h"
#include "stream_dev_directshow_tools.h"

#include <sstream>

#include <amvideo.h>
#include <d3d9types.h>
#include <dmoreg.h>
//#include <dshow.h>
#include <dvdmedia.h>
#include <Dmodshow.h>
#include <evr.h>
//#include <Ks.h>
//#include <ksmedia.h>
#include <KsProxy.h>
#include <mediaobj.h>
#include <qedit.h>
#include <mfapi.h>
#include <mferror.h>
#include <OleAuto.h>
#include <strmif.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#include <wmcodecdsp.h>

#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch.h"

#include "common.h"
#include "common_time_common.h"
#include "common_tools.h"

#include "common_error_tools.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_lib_directshow_tools.h"
#include "stream_lib_tools.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

bool
Stream_Device_DirectShow_Tools::initialize (bool coInitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::initialize"));

  HRESULT result = E_FAIL;
  if (likely (coInitialize_in))
  {
    result = CoInitializeEx (NULL,
                             (COINIT_MULTITHREADED    |
                              COINIT_DISABLE_OLE1DDE  |
                              COINIT_SPEED_OVER_MEMORY));
    if (FAILED (result))
    {
      // *NOTE*: most probable reason: already initialized (happens in the
      //         debugger) --> continue
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("failed to CoInitializeEx(): \"%s\", continuing\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    } // end IF
  } // end IF

  return true;
}

void
Stream_Device_DirectShow_Tools::finalize (bool coUninitialize_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::finalize"));

  if (likely (coUninitialize_in))
    CoUninitialize ();
}

std::string
Stream_Device_DirectShow_Tools::devicePathToString (const std::string& devicePath_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::devicePathToString"));

  // sanity check(s)
  ACE_ASSERT (!devicePath_in.empty ());

  std::string result;

  ICreateDevEnum* enumerator_p = NULL;
  IEnumMoniker* enum_moniker_p = NULL;
  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  struct tagVARIANT variant_s;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID class_id_s = GUID_NULL;
  Common_Identifiers_t class_ids_a;
  bool done = false;

  HRESULT result_2 =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (enumerator_p);

  VariantInit (&variant_s);

  class_ids_a.push_back (CLSID_AudioInputDeviceCategory);
  class_ids_a.push_back (CLSID_VideoInputDeviceCategory);

  for (Common_IdentifiersIterator_t iterator = class_ids_a.begin ();
       iterator != class_ids_a.end ();
       ++iterator)
  { ACE_ASSERT (!enum_moniker_p);
    result_2 =
      enumerator_p->CreateClassEnumerator (*iterator,
                                           &enum_moniker_p,
                                           0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (*iterator).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (enum_moniker_p);

    ACE_ASSERT (!moniker_p);
    while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
    { ACE_ASSERT (moniker_p);
      result_2 = moniker_p->BindToStorage (NULL, NULL,
                                           IID_PPV_ARGS (&properties_p));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (properties_p);
      moniker_p->Release (); moniker_p = NULL;
      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING,
                            &variant_s,
                            0);
      if (FAILED (result_2)) // ERROR_FILE_NOT_FOUND: 0x80070002
      { // most probable reason: audio device
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to IPropertyBag::Read(\"%s\"): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        result_2 = VariantClear (&variant_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        properties_p->Release (); properties_p = NULL;
        continue;
      } // end IF
      ACE_Wide_To_Ascii converter (variant_s.bstrVal);
      result_2 = VariantClear (&variant_s);
      ACE_ASSERT (SUCCEEDED (result_2));
      if (ACE_OS::strcmp (devicePath_in.c_str (),
                          converter.char_rep ()))
      {
        properties_p->Release (); properties_p = NULL;
        continue;
      } // end IF
      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                            &variant_s,
                            0);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        goto error;
      } // end IF
      properties_p->Release (); properties_p = NULL;
      ACE_Wide_To_Ascii converter_2 (variant_s.bstrVal);
      result_2 = VariantClear (&variant_s);
      ACE_ASSERT (SUCCEEDED (result_2));
      result = converter_2.char_rep ();
      done = true;
      break;
    } // end WHILE
    enum_moniker_p->Release (); enum_moniker_p = NULL;
    if (done)
      break;
  } // end FOR

error:
  if (properties_p)
    properties_p->Release ();
  if (moniker_p)
    moniker_p->Release ();
  if (enum_moniker_p)
    enum_moniker_p->Release ();
  if (enumerator_p)
    enumerator_p->Release ();

  return result;
}
std::string
Stream_Device_DirectShow_Tools::devicePath (const std::string& friendlyName_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::devicePath"));

  // sanity check(s)
  ACE_ASSERT (!friendlyName_in.empty ());

  std::string result;

  ICreateDevEnum* enumerator_p = NULL;
  IEnumMoniker* enum_moniker_p = NULL;
  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  struct tagVARIANT variant_s;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID class_id_s = GUID_NULL;
  Common_Identifiers_t class_ids_a;
  bool done = false;

  HRESULT result_2 =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (enumerator_p);

  VariantInit (&variant_s);

  class_ids_a.push_back (CLSID_AudioInputDeviceCategory);
  class_ids_a.push_back (CLSID_VideoInputDeviceCategory);

  for (Common_IdentifiersIterator_t iterator = class_ids_a.begin ();
       iterator != class_ids_a.end ();
       ++iterator)
  { ACE_ASSERT (!enum_moniker_p);
    result_2 =
      enumerator_p->CreateClassEnumerator (*iterator,
                                           &enum_moniker_p,
                                           0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(%s): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::GUIDToString (*iterator).c_str ()),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (enum_moniker_p);

    ACE_ASSERT (!moniker_p);
    while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
    { ACE_ASSERT (moniker_p);
      result_2 = moniker_p->BindToStorage (NULL, NULL,
                                           IID_PPV_ARGS (&properties_p));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (properties_p);
      moniker_p->Release (); moniker_p = NULL;
      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                            &variant_s,
                            0);
      if (FAILED (result_2)) // ERROR_FILE_NOT_FOUND: 0x80070002
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to IPropertyBag::Read(\"%s\"): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        result_2 = VariantClear (&variant_s);
        ACE_ASSERT (SUCCEEDED (result_2));
        properties_p->Release (); properties_p = NULL;
        continue;
      } // end IF
      ACE_Wide_To_Ascii converter (variant_s.bstrVal);
      result_2 = VariantClear (&variant_s);
      ACE_ASSERT (SUCCEEDED (result_2));
      if (ACE_OS::strcmp (friendlyName_in.c_str (),
                          converter.char_rep ()))
      {
        properties_p->Release (); properties_p = NULL;
        continue;
      } // end IF
      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING,
                            &variant_s,
                            0);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(\"%s\"): \"%s\", aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
        goto error;
      } // end IF
      properties_p->Release (); properties_p = NULL;
      ACE_Wide_To_Ascii converter_2 (variant_s.bstrVal);
      result_2 = VariantClear (&variant_s);
      ACE_ASSERT (SUCCEEDED (result_2));
      result = converter_2.char_rep ();
      done = true;
      break;
    } // end WHILE
    enum_moniker_p->Release (); enum_moniker_p = NULL;
    if (done)
      break;
  } // end FOR

error:
  if (properties_p)
    properties_p->Release ();
  if (moniker_p)
    moniker_p->Release ();
  if (enum_moniker_p)
    enum_moniker_p->Release ();
  if (enumerator_p)
    enumerator_p->Release ();

  return result;
}

std::string
Stream_Device_DirectShow_Tools::getDefaultCaptureDevice (REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getDefaultCaptureDevice"));

  // initialize return value(s)
  std::string result;

  Stream_Device_List_t devices_a =
    Stream_Device_DirectShow_Tools::getCaptureDevices (deviceCategory_in);
  if (likely (!devices_a.empty ()))
    result = devices_a.front ();

  return result;
}

Stream_Device_List_t
Stream_Device_DirectShow_Tools::getCaptureDevices (REFGUID deviceCategory_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getCaptureDevices"));

  // initialize return value(s)
  Stream_Device_List_t result;

  // sanity check(s)
  if (!InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory) &&
      !InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory) &&
      !InlineIsEqualGUID (deviceCategory_in, AM_KSCATEGORY_CAPTURE)) // WDM (i.e. kernel-) Streaming Capture Devices
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return result;
  } // end ELSE

  ICreateDevEnum* enumerator_p = NULL;
  IEnumMoniker* enum_moniker_p = NULL;
  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  struct tagVARIANT variant_s;
  HRESULT result_2 = E_FAIL;

  result_2 =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, false).c_str ())));
    return result;
  } // end IF
  ACE_ASSERT (enumerator_p);

  result_2 =
    enumerator_p->CreateClassEnumerator (deviceCategory_in,
                                         &enum_moniker_p,
                                         0);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    enumerator_p->Release (); enumerator_p = NULL;
    //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    return result;
  } // end IF
  ACE_ASSERT (enum_moniker_p);
  enumerator_p->Release (); enumerator_p = NULL;

  VariantInit (&variant_s);
  while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
  { ACE_ASSERT (moniker_p);
    ACE_ASSERT (!properties_p);
    result_2 = moniker_p->BindToStorage (NULL, NULL,
                                         IID_PPV_ARGS (&properties_p));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      moniker_p->Release (); moniker_p = NULL;
      enum_moniker_p->Release (); enum_moniker_p = NULL;
      return result;
    } // end IF
    ACE_ASSERT (properties_p);

    result_2 =
      properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING,
                          &variant_s,
                          0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      properties_p->Release (); properties_p = NULL;
      moniker_p->Release (); moniker_p = NULL;
      enum_moniker_p->Release (); enum_moniker_p = NULL;
      return result;
    } // end IF
    ACE_Wide_To_Ascii converter (variant_s.bstrVal);
    result_2 = VariantClear (&variant_s);
    ACE_ASSERT (SUCCEEDED (result_2));
    result.push_back (converter.char_rep ());
#if defined (_DEBUG)
    std::string friendly_name_string;
    result_2 =
      properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                          &variant_s,
                          0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
      properties_p->Release (); properties_p = NULL;
      moniker_p->Release (); moniker_p = NULL;
      enum_moniker_p->Release (); enum_moniker_p = NULL;
      return result;
    } // end IF
    ACE_Wide_To_Ascii converter_2 (variant_s.bstrVal);
    result_2 = VariantClear (&variant_s);
    ACE_ASSERT (SUCCEEDED (result_2));
    friendly_name_string = converter_2.char_rep ();
    result_2 =
      properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING,
                          &variant_s,
                          0);
    if (FAILED (result_2))
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                  ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING),
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
    ACE_Wide_To_Ascii converter_3 (variant_s.bstrVal);
    result_2 = VariantClear (&variant_s);
    ACE_ASSERT (SUCCEEDED (result_2));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("found %scapture device \"%s\" [%s]: %s\n"),
                (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory) ? ACE_TEXT ("audio ")
                                                                                       : (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory) ? ACE_TEXT ("video ")
                                                                                                                                                                : (InlineIsEqualGUID (deviceCategory_in, AM_KSCATEGORY_CAPTURE) ? ACE_TEXT ("WDM ")
                                                                                                                                                                                                                                : ACE_TEXT ("")))),
                ACE_TEXT (converter_3.char_rep ()),
                ACE_TEXT (friendly_name_string.c_str ()),
                ACE_TEXT (converter.char_rep ())));
#endif // _DEBUG
    properties_p->Release (); properties_p = NULL;
  } // end WHILE
  moniker_p->Release (); moniker_p = NULL;
  enum_moniker_p->Release (); enum_moniker_p = NULL;

  return result;
}

bool
Stream_Device_DirectShow_Tools::isMediaTypeBottomUp (const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::isMediaTypeBottomUp"));

  // initialize return value(s)
  bool result = false;

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo))
  {
    video_info_header_p = (struct tagVIDEOINFOHEADER*)mediaType_in.pbFormat;
    result = video_info_header_p->bmiHeader.biHeight > 0;
  } // end IF
  else if (InlineIsEqualGUID (mediaType_in.formattype, FORMAT_VideoInfo2))
  {
    video_info_header2_p =
      (struct tagVIDEOINFOHEADER2*)mediaType_in.pbFormat;
    result = video_info_header2_p->bmiHeader.biHeight > 0;
  } // end ELSE IF
  else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), aborting\n"),
                ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (mediaType_in.formattype).c_str ())));

  return result;
}

Common_Identifiers_t
Stream_Device_DirectShow_Tools::getCaptureSubFormats (IAMStreamConfig* IAMStreamConfig_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getCaptureSubFormats"));

  // initialize return value(s)
  Common_Identifiers_t result;

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);

  HRESULT result_2 = E_FAIL;
  int count = 0, size = 0;
  result_2 = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities_s;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result_2 = IAMStreamConfig_in->GetStreamCaps (i,
                                                  &media_type_p,
                                                  (BYTE*)&capabilities_s);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      return result;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo) &&
        !InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF

    // *NOTE*: FORMAT_VideoInfo2 types do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    result.push_back (media_type_p->subtype);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end FOR
  result.sort (common_less_guid ());
  result.unique (common_equal_guid ());

  return result;
}
Common_UI_Resolutions_t
Stream_Device_DirectShow_Tools::getCaptureResolutions (IAMStreamConfig* IAMStreamConfig_in,
                                                       REFGUID mediaSubType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getCaptureSubFormats"));

  // initialize return value(s)
  Common_UI_Resolutions_t result;

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);

  HRESULT result_2 = E_FAIL;
  int count = 0, size = 0;
  result_2 = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities_s;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  Common_UI_Resolution_t resolution_s;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result_2 = IAMStreamConfig_in->GetStreamCaps (i,
                                                  &media_type_p,
                                                  (BYTE*)&capabilities_s);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      return result;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL) &&
        !InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      resolution_s.cx = video_info_header_p->bmiHeader.biWidth;
      resolution_s.cy = video_info_header_p->bmiHeader.biHeight;
      result.push_back (resolution_s);
    } // end IF
    else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      // *NOTE*: these media subtypes do not work with the Video Renderer
      //         directly --> insert the Overlay Mixer
      video_info_header2_p =
        (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      resolution_s.cx = video_info_header2_p->bmiHeader.biWidth;
      resolution_s.cy = video_info_header2_p->bmiHeader.biHeight;
      result.push_back (resolution_s);
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (media_type_p->formattype).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end ELSE
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end FOR
  result.sort (common_ui_resolution_less ());
  result.unique (common_ui_resolution_equal ());

  return result;
}
Common_UI_Framerates_t
Stream_Device_DirectShow_Tools::getCaptureFramerates (IAMStreamConfig*IAMStreamConfig_in,
                                                      REFGUID mediaSubType_in,
                                                      Common_UI_Resolution_t resolution_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getCaptureSubFormats"));

  // initialize return value(s)
  Common_UI_Framerates_t result;

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (!InlineIsEqualGUID (mediaSubType_in, GUID_NULL));

  HRESULT result_2 = E_FAIL;
  int count = 0, size = 0;
  result_2 = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
    return result;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS capabilities_s;
  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header2_p = NULL;
  unsigned int frame_duration = 0;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result_2 = IAMStreamConfig_in->GetStreamCaps (i,
                                                  &media_type_p,
                                                  (BYTE*)&capabilities_s);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
      return result;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo))
    {
      video_info_header_p = (struct tagVIDEOINFOHEADER*)media_type_p->pbFormat;
      if ((video_info_header_p->bmiHeader.biWidth  != resolution_in.cx) ||
          (video_info_header_p->bmiHeader.biHeight != resolution_in.cy))
      {
        Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
        continue;
      } // end IF
      frame_duration =
        static_cast<unsigned int> (video_info_header_p->AvgTimePerFrame);
    } // end IF
    else if (InlineIsEqualGUID (media_type_p->formattype, FORMAT_VideoInfo2))
    {
      video_info_header2_p =
        (struct tagVIDEOINFOHEADER2*)media_type_p->pbFormat;
      if ((video_info_header2_p->bmiHeader.biWidth  != resolution_in.cx) ||
          (video_info_header2_p->bmiHeader.biHeight != resolution_in.cy))
      {
        Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
        continue;
      } // end IF
      frame_duration =
        static_cast<unsigned int> (video_info_header2_p->AvgTimePerFrame);
    } // end ELSE IF
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media format type (was: \"%s\"), continuing\n"),
                  ACE_TEXT (Stream_MediaFramework_Tools::mediaFormatTypeToString (media_type_p->formattype).c_str ())));
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end ELSE
    result.push_back (NANOSECONDS / frame_duration);
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end FOR
  std::sort (result.begin (), result.end ());
  result.erase (std::unique (result.begin (), result.end ()), result.end ());

  return result;
}

bool
Stream_Device_DirectShow_Tools::getCaptureFormat (IGraphBuilder* builder_in,
                                                         REFGUID deviceCategory_in,
                                                         struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  // initialize return value(s)
  if (mediaType_out)
  {
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_out);
    mediaType_out = NULL;
  } // end IF

  // sanity check(s)
  std::wstring filter_name;
  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    filter_name = MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return false;
  } // end ELSE

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filter_name.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IPin* pin_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;

  pin_p = Stream_MediaFramework_DirectShow_Tools::capturePin (filter_p);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));
    filter_p->Release ();
    return false;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_p->Release ();
    return false;
  } // end IF
  ACE_ASSERT (stream_config_p);
  pin_p->Release (); pin_p = NULL;
  result = stream_config_p->GetFormat (&mediaType_out);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetFormat(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    stream_config_p->Release ();
    return false;
  } // end IF
  ACE_ASSERT (mediaType_out);
  stream_config_p->Release (); stream_config_p = NULL;

  return true;
}

bool
Stream_Device_DirectShow_Tools::getVideoCaptureFormat (IGraphBuilder* builder_in,
                                                              REFGUID mediaSubType_in,
                                                              LONG width_in,
                                                              LONG height_in,
                                                              unsigned int frameRate_in,
                                                              struct _AMMediaType*& mediaType_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::getVideoCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  // initialize return value(s)
  if (mediaType_out)
    Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_out);

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO,
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);

  IPin* pin_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  int count, size;

  pin_p = Stream_MediaFramework_DirectShow_Tools::capturePin (filter_p);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO)));
    return false;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_p->Release (); pin_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (stream_config_p);
  pin_p->Release (); pin_p = NULL;
  result = stream_config_p->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    stream_config_p->Release (); stream_config_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (size == sizeof (struct _VIDEO_STREAM_CONFIG_CAPS));

  struct _VIDEO_STREAM_CONFIG_CAPS video_stream_config_caps_s;
  ACE_OS::memset (&video_stream_config_caps_s, 0, sizeof (struct _VIDEO_STREAM_CONFIG_CAPS));
  Common_UI_Resolution_t resolution_s;
  unsigned int framerate_i = 0;
  for (int i = 0;
        i < count;
        ++i)
  {
    result =
      stream_config_p->GetStreamCaps (i,
                                      &mediaType_out,
                                      reinterpret_cast<BYTE*> (&video_stream_config_caps_s));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      stream_config_p->Release ();
      return false;
    } // end IF
    ACE_ASSERT (mediaType_out);
    if (!InlineIsEqualGUID (mediaSubType_in, mediaType_out->subtype))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_out);
      continue;
    } // end IF
    resolution_s =
      Stream_MediaFramework_DirectShow_Tools::toResolution (*mediaType_out);
    if ((width_in  && (resolution_s.cx != width_in)) ||
        (height_in && (resolution_s.cy != height_in)))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_out);
      continue;
    } // end IF
    framerate_i =
      Stream_MediaFramework_DirectShow_Tools::toFramerate (*mediaType_out);
    if (frameRate_in && (framerate_i != frameRate_in))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (mediaType_out);
      continue;
    } // end IF
    break; // --> found a match
  } // end FOR
  stream_config_p->Release (); stream_config_p = NULL;

  return !!mediaType_out;
}

void
Stream_Device_DirectShow_Tools::listCaptureFormats (IBaseFilter* filter_in,
                                                           REFGUID formatType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::listCaptureFormats"));

  // sanity check(s)
  ACE_ASSERT (filter_in);
  ACE_ASSERT (!InlineIsEqualGUID (formatType_in, GUID_NULL));

  HRESULT result = E_FAIL;
  IPin* pin_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;
  int count, size;

  pin_p = Stream_MediaFramework_DirectShow_Tools::capturePin (filter_in);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, returning\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_in).c_str ())));
    return;
  } // end IF
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_p->Release ();
    return;
  } // end IF
  ACE_ASSERT (stream_config_p);
  pin_p->Release (); pin_p = NULL;
  result = stream_config_p->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", returning\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    stream_config_p->Release ();
    return;
  } // end IF

  struct _AMMediaType* media_type_p = NULL;
  BYTE audio_SCC[sizeof (struct _AUDIO_STREAM_CONFIG_CAPS)];
  BYTE video_SCC[sizeof (struct _VIDEO_STREAM_CONFIG_CAPS)];
  BYTE* SCC_p = NULL;
  if (size == sizeof (struct _AUDIO_STREAM_CONFIG_CAPS))
    SCC_p = audio_SCC;
  else if (size == sizeof (struct _VIDEO_STREAM_CONFIG_CAPS))
    SCC_p = video_SCC;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (IAMStreamConfig::GetNumberOfCapabilities() returned size: %d), returning\n"),
                size));
    stream_config_p->Release ();
    return;
  } // end ELSE
  struct _AUDIO_STREAM_CONFIG_CAPS* audio_stream_config_caps_p = NULL;
  struct _VIDEO_STREAM_CONFIG_CAPS* video_stream_config_caps_p = NULL;
  for (int i = 0;
        i < count;
        ++i)
  {
    result = stream_config_p->GetStreamCaps (i,
                                             &media_type_p,
                                             SCC_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", returning\n"),
                  i,
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      stream_config_p->Release ();
      return;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (formatType_in, GUID_NULL) &&
        !InlineIsEqualGUID (formatType_in, media_type_p->formattype))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    ACE_ASSERT (media_type_p->pbFormat);

    if (size == sizeof (struct _AUDIO_STREAM_CONFIG_CAPS))
    {
      audio_stream_config_caps_p =
        reinterpret_cast<struct _AUDIO_STREAM_CONFIG_CAPS*> (SCC_p);
      ACE_ASSERT (InlineIsEqualGUID (media_type_p->formattype, FORMAT_WaveFormatEx));
      Stream_MediaFramework_DirectShow_Tools::dump (*media_type_p);
    } // end IF
    else if (size == sizeof (struct _VIDEO_STREAM_CONFIG_CAPS))
    {
      video_stream_config_caps_p =
        reinterpret_cast<struct _VIDEO_STREAM_CONFIG_CAPS*> (SCC_p);
      Stream_MediaFramework_DirectShow_Tools::dump (*media_type_p);
    } // end ELSE
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown device category (IAMStreamConfig::GetNumberOfCapabilities() returned size: %d), returning\n"),
                  size));
      stream_config_p->Release ();
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      return;
    } // end ELSE
    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end FOR
  stream_config_p->Release (); stream_config_p = NULL;
}

bool
Stream_Device_DirectShow_Tools::setCaptureFormat (IGraphBuilder* builder_in,
                                                         REFGUID deviceCategory_in,
                                                         const struct _AMMediaType& mediaType_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::setCaptureFormat"));

  // sanity check(s)
  ACE_ASSERT (builder_in);

  std::wstring filter_name;
  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    filter_name = MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    return false;
  } // end ELSE

  IBaseFilter* filter_p = NULL;
  HRESULT result =
    builder_in->FindFilterByName (filter_name.c_str (),
                                  &filter_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::FindFilterByName(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    return false;
  } // end IF
  ACE_ASSERT (filter_p);
//#if defined (_DEBUG)
//  Stream_Device_DirectShow_Tools::listCaptureFormats (filter_p,
//                                                             mediaType_in.formattype);
//#endif

  IPin* pin_p = NULL;
  IAMStreamConfig* stream_config_p = NULL;

  pin_p = Stream_MediaFramework_DirectShow_Tools::capturePin (filter_p);
  if (!pin_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ())));
    filter_p->Release ();
    return false;
  } // end IF
  filter_p->Release (); filter_p = NULL;
  result = pin_p->QueryInterface (IID_PPV_ARGS (&stream_config_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_p->Release ();
    return false;
  } // end IF
  ACE_ASSERT (stream_config_p);
  pin_p->Release (); pin_p = NULL;
  result =
    stream_config_p->SetFormat (&const_cast<struct _AMMediaType&> (mediaType_in));
  if (FAILED (result)) // VFW_E_INVALIDMEDIATYPE: 0x80040200L
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::SetFormat(): \"%s\" (0x%x) (media type was: %s), aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ()), result,
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (mediaType_in, false).c_str ())));
    stream_config_p->Release ();
    return false;
  } // end IF
  stream_config_p->Release (); stream_config_p = NULL;

#if defined (_DEBUG)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: set capture format: %s\n"),
              ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
              ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::toString (mediaType_in, true).c_str ())));
#endif // _DEBUG

  return true;
}

bool
Stream_Device_DirectShow_Tools::loadDeviceGraph (const std::string& devicePath_in,
                                                 REFGUID deviceCategory_in,
                                                 IGraphBuilder*& IGraphBuilder_inout,
                                                 IAMBufferNegotiation*& IAMBufferNegotiation_out,
                                                 IAMStreamConfig*& IAMStreamConfig_out,
                                                 Stream_MediaFramework_DirectShow_Graph_t& graphLayout_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Device_DirectShow_Tools::loadDeviceGraph"));

  // initialize return value(s)
  graphLayout_out.clear ();

  // sanity check(s)
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release (); IAMBufferNegotiation_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  } // end IF

  std::wstring filter_name;
  ICreateDevEnum* enumerator_p = NULL;
  IEnumMoniker* enum_moniker_p = NULL;
  IMoniker* moniker_p = NULL;
  IPropertyBag* properties_p = NULL;
  struct tagVARIANT variant_s;
  IEnumPins* enumerator_2 = NULL;
  IPin* pin_p, *pin_2 = NULL;
  enum _PinDirection pin_direction;
  IKsPropertySet* property_set_p = NULL;
  struct _GUID GUID_s = GUID_NULL;
  DWORD returned_size = 0;
  std::string device_path_string;
  LONG device_id = -1;
  HRESULT result = E_FAIL;
  IBaseFilter* filter_p = NULL;

  if (!IGraphBuilder_inout)
  {
    result = CoCreateInstance (CLSID_FilterGraph, NULL,
                               CLSCTX_INPROC_SERVER,
                               IID_PPV_ARGS (&IGraphBuilder_inout));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_FilterGraph): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
      //builder_2->Release ();
      return false;
    } // end IF
    ACE_ASSERT (IGraphBuilder_inout);
  } // end IF
  else
  {
    if (!Stream_MediaFramework_DirectShow_Tools::clear (IGraphBuilder_inout))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_MediaFramework_DirectShow_Tools::clear(), aborting\n")));
      goto error;
    } // end IF
  } // end ELSE
  ACE_ASSERT (IGraphBuilder_inout);

  result =
    CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                      CLSCTX_INPROC_SERVER,
                      IID_PPV_ARGS (&enumerator_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, false).c_str ())));
    goto error;
  } // end IF
  ACE_ASSERT (enumerator_p);
  result =
    enumerator_p->CreateClassEnumerator (deviceCategory_in,
                                         &enum_moniker_p,
                                         0); // flags
  // *NOTE*: "...If the category does not exist or is empty, the return value is
  //         S_FALSE, and the ppEnumMoniker parameter receives the value NULL.
  //         Therefore, test for the return value S_OK instead of using the
  //         SUCCEEDED macro
  //if (FAILED (result))
  if (result != S_OK)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(%s): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    enumerator_p->Release (); enumerator_p = NULL;
    //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
    goto error;
  } // end IF
  ACE_ASSERT (enum_moniker_p);
  enumerator_p->Release (); enumerator_p = NULL;
  VariantInit (&variant_s);
  while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
  { ACE_ASSERT (moniker_p);
    ACE_ASSERT (!properties_p);
    result = moniker_p->BindToStorage (NULL, NULL,
                                       IID_PPV_ARGS (&properties_p));
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      moniker_p->Release (); moniker_p = NULL;
      enum_moniker_p->Release (); enum_moniker_p = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (properties_p);
    if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    {
      result =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING,
                            &variant_s,
                            0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
        properties_p->Release (); properties_p = NULL;
        moniker_p->Release (); moniker_p = NULL;
        enum_moniker_p->Release (); enum_moniker_p = NULL;
        goto error;
      } // end IF
      ACE_Wide_To_Ascii converter (variant_s.bstrVal);
      device_path_string = converter.char_rep ();
    } // end IF
    else if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    {
      result =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                            &variant_s,
                            0);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
                    ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
        properties_p->Release (); properties_p = NULL;
        moniker_p->Release (); moniker_p = NULL;
        enum_moniker_p->Release (); enum_moniker_p = NULL;
        goto error;
      } // end IF
      ACE_Wide_To_Ascii converter (variant_s.bstrVal);
      device_path_string = converter.char_rep ();
    } // end IF
    result = VariantClear (&variant_s);
    ACE_ASSERT (SUCCEEDED (result));
    properties_p->Release (); properties_p = NULL;
    if (devicePath_in.empty () ||
        !ACE_OS::strcmp (devicePath_in.c_str (),
                         device_path_string.c_str ()))
      break;
    moniker_p->Release (); moniker_p = NULL;
  } // end WHILE
  enum_moniker_p->Release (); enum_moniker_p = NULL;
  if (!moniker_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("device (path was: \"%s\") not found, aborting\n"),
                ACE_TEXT (devicePath_in.c_str ())));
    goto error;
  } // end IF

  result = moniker_p->BindToObject (NULL, NULL,
                                    IID_PPV_ARGS (&filter_p));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMoniker::BindToObject(IID_IBaseFilter): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    moniker_p->Release (); moniker_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (filter_p);
  moniker_p->Release (); moniker_p = NULL;

  if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
    filter_name = MODULE_DEV_MIC_DIRECTSHOW_FILTER_NAME_CAPTURE_AUDIO;
  else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
    filter_name = MODULE_DEV_CAM_DIRECTSHOW_FILTER_NAME_CAPTURE_VIDEO;
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid/unknown device category (was: %s), aborting\n"),
                ACE_TEXT (Common_Tools::GUIDToString (deviceCategory_in).c_str ())));
    filter_p->Release (); filter_p = NULL;
    goto error;
  } // end ELSE
  result = IGraphBuilder_inout->AddFilter (filter_p,
                                           filter_name.c_str ());
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IGraphBuilder::AddFilter(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    filter_p->Release (); filter_p = NULL;
    goto error;
  } // end IF
  graphLayout_out.push_back (filter_name);

  result = filter_p->EnumPins (&enumerator_2);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to IBaseFilter::EnumPins(): \"%s\", aborting\n"),
                ACE_TEXT (Stream_MediaFramework_DirectShow_Tools::name (filter_p).c_str ()),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    filter_p->Release (); filter_p = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (enumerator_2);
  filter_p->Release (); filter_p = NULL;

  while (S_OK == enumerator_2->Next (1, &pin_p, NULL))
  { ACE_ASSERT (pin_p);
    result = pin_p->QueryDirection (&pin_direction);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryDirection(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      enumerator_2->Release (); enumerator_2 = NULL;
      goto error;
    } // end IF
    switch (pin_direction)
    {
      case PINDIR_INPUT:
      {
        if (InlineIsEqualGUID (deviceCategory_in, CLSID_AudioInputDeviceCategory))
        {
          IAMAudioInputMixer* audio_input_mixer_p = NULL;
          struct _PinInfo pin_info_s;
          result = pin_p->QueryInterface (IID_PPV_ARGS (&audio_input_mixer_p));
          if (FAILED (result))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMAudioInputMixer): \"%s\", aborting\n"),
                        ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
            pin_p->Release (); pin_p = NULL;
            enumerator_2->Release (); enumerator_2 = NULL;
            goto error;
          } // end IF
          ACE_ASSERT (audio_input_mixer_p);
          result = audio_input_mixer_p->put_Enable (TRUE);
          if (FAILED (result))
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("failed to IAMAudioInputMixer::put_Enable(): \"%s\", aborting\n"),
                        ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
            audio_input_mixer_p->Release (); audio_input_mixer_p = NULL;
            pin_p->Release (); pin_p = NULL;
            enumerator_2->Release (); enumerator_2 = NULL;
            goto error;
          } // end IF
          audio_input_mixer_p->Release (); audio_input_mixer_p = NULL;
          ACE_OS::memset (&pin_info_s, 0, sizeof (struct _PinInfo));
          result = pin_p->QueryPinInfo (&pin_info_s);
          ACE_ASSERT (SUCCEEDED (result));
          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("%s: enabled input pin \"%s\"\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ()),
                      ACE_TEXT_WCHAR_TO_TCHAR (pin_info_s.achName)));
        } // end IF
        //else if (InlineIsEqualGUID (deviceCategory_in, CLSID_VideoInputDeviceCategory))
        pin_p->Release (); pin_p = NULL;
        continue;
      }
      case PINDIR_OUTPUT:
        break;
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown pin direction (was: %d), aborting\n"),
                    pin_direction));
        pin_p->Release (); pin_p = NULL;
        enumerator_2->Release (); enumerator_2 = NULL;
        goto error;
      }
    } // end SWITCH
    ACE_ASSERT (!property_set_p);
    result = pin_p->QueryInterface (IID_IKsPropertySet,
                                    (void**)&property_set_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IPin::QueryInterface(IID_IKsPropertySet): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      pin_p->Release (); pin_p = NULL;
      enumerator_2->Release (); enumerator_2 = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (property_set_p);
    returned_size = 0;
    result =
      property_set_p->Get (AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY,
                           NULL, 0,
                           &GUID_s, sizeof (struct _GUID), &returned_size);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IKsPropertySet::Get(AMPROPERTY_PIN_CATEGORY): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
      property_set_p->Release (); property_set_p = NULL;
      pin_p->Release (); pin_p = NULL;
      enumerator_2->Release (); enumerator_2 = NULL;
      goto error;
    } // end IF
    ACE_ASSERT (returned_size == sizeof (struct _GUID));
    property_set_p->Release (); property_set_p = NULL;
    if (InlineIsEqualGUID (GUID_s, PIN_CATEGORY_CAPTURE))
    {
      pin_2 = pin_p;
      pin_p = NULL;
      break;
    } // end IF
    pin_p->Release (); pin_p = NULL;
  } // end WHILE
  enumerator_2->Release (); enumerator_2 = NULL;
  if (!pin_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: no capture pin found, aborting\n"),
                ACE_TEXT_WCHAR_TO_TCHAR (filter_name.c_str ())));
    goto error;
  } // end IF

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMBufferNegotiation_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMBufferNegotiation): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_2->Release (); pin_2 = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (IAMBufferNegotiation_out);

  result = pin_2->QueryInterface (IID_PPV_ARGS (&IAMStreamConfig_out));
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IPin::QueryInterface(IID_IAMStreamConfig): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result, true).c_str ())));
    pin_2->Release (); pin_2 = NULL;
    goto error;
  } // end IF
  ACE_ASSERT (IAMStreamConfig_out);
  pin_2->Release (); pin_2 = NULL;

  return true;

error:
  if (IGraphBuilder_inout)
  {
    IGraphBuilder_inout->Release (); IGraphBuilder_inout = NULL;
  } // end IF
  if (IAMBufferNegotiation_out)
  {
    IAMBufferNegotiation_out->Release (); IAMBufferNegotiation_out = NULL;
  } // end IF
  if (IAMStreamConfig_out)
  {
    IAMStreamConfig_out->Release (); IAMStreamConfig_out = NULL;
  } // end IF
  graphLayout_out.clear ();

  return false;
}
