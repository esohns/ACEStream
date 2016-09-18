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

#include "test_u_audioeffect_callbacks.h"

#include <math.h>

#include <limits>
#include <map>
#include <set>
#include <sstream>

#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "streams.h"
#include "mfapi.h"
#include "mferror.h"
#include "mfidl.h"
#include "mfreadwrite.h"
#else
#include "ace/Dirent_Selector.h"

#include "alsa/asoundlib.h"
#endif

#include "gtk/gtk.h"

#include "common_file_tools.h"
#include "common_timer_manager_common.h"
#include "common_tools.h"

#include "common_ui_common.h"
#include "common_ui_defines.h"
#include "common_ui_tools.h"

#include "stream_macros.h"

#include "stream_dev_defines.h"
#include "stream_dev_tools.h"

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_common_modules.h"
#include "test_u_audioeffect_defines.h"
#include "test_u_audioeffect_stream.h"

// global variables
bool un_toggling_stream = false;

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_capture_devices (GtkListStore* listStore_in,
                      bool useMediaFoundation_in)
#else
load_capture_devices (GtkListStore* listStore_in)
#endif
{
  STREAM_TRACE (ACE_TEXT ("::load_capture_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  if (useMediaFoundation_in)
  {
    IMFAttributes* attributes_p = NULL;
    IMFActivate** devices_pp = NULL;
    UINT32 count = 0;
    WCHAR buffer[BUFSIZ];
    UINT32 length = 0;

    result_2 = MFCreateAttributes (&attributes_p, 1);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateAttributes(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      return false;
    } // end IF

    result_2 =
      attributes_p->SetGUID (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                             MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF

    result_2 = MFEnumDeviceSources (attributes_p,
                                    &devices_pp,
                                    &count);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    attributes_p->Release ();
    attributes_p = NULL;
    ACE_ASSERT (devices_pp);

    for (UINT32 index = 0; index < count; index++)
    {
      ACE_OS::memset (buffer, 0, sizeof (buffer));
      length = 0;
      result_2 =
        devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                      buffer,
                                      sizeof (buffer),
                                      &length);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error;
      } // end IF

      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer)),
                          -1);
    } // end FOR

    for (UINT32 i = 0; i < count; i++)
      devices_pp[i]->Release ();
    CoTaskMemFree (devices_pp);

    result = true;

    goto continue_;

error:
    if (attributes_p)
      attributes_p->Release ();
    if (devices_pp)
    {
      for (UINT32 i = 0; i < count; i++)
        devices_pp[i]->Release ();
      CoTaskMemFree (devices_pp);
    } // end IF
  } // end IF
  else
  {
    ICreateDevEnum* enumerator_p = NULL;
    IEnumMoniker* enum_moniker_p = NULL;
    IMoniker* moniker_p = NULL;
    IPropertyBag* properties_p = NULL;
    VARIANT variant;
    std::string friendly_name_string, description_string, device_path;
    DWORD wave_in_id;

    result_2 =
      CoCreateInstance (CLSID_SystemDeviceEnum, NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS (&enumerator_p));
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to CoCreateInstance(CLSID_SystemDeviceEnum): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (enumerator_p);

    result_2 =
      enumerator_p->CreateClassEnumerator (CLSID_AudioInputDeviceCategory,
                                           &enum_moniker_p,
                                           0);
    if (FAILED (result_2))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ICreateDevEnum::CreateClassEnumerator(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
      //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
      goto error_2;
    } // end IF
    ACE_ASSERT (enum_moniker_p);
    enumerator_p->Release ();
    enumerator_p = NULL;

    while (S_OK == enum_moniker_p->Next (1, &moniker_p, NULL))
    {
      ACE_ASSERT (moniker_p);

      properties_p = NULL;
      result_2 = moniker_p->BindToStorage (NULL, NULL,
                                           IID_PPV_ARGS (&properties_p));
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMoniker::BindToStorage(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error_2;
      } // end IF
      moniker_p->Release ();
      moniker_p = NULL;
      ACE_ASSERT (properties_p);

      VariantInit (&variant);
      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING,
                            &variant,
                            0);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_NAME_STRING),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
        goto error_2;
      } // end IF
      friendly_name_string =
         ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant.bstrVal));
      VariantClear (&variant);
      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING,
                            &variant,
                            0);
      if (SUCCEEDED (result_2))
      {
        description_string =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant.bstrVal));
        VariantClear (&variant);
      } // end IF
      else // 0x80070002 : ERROR_FILE_NOT_FOUND
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING,
                            &variant,
                            0);
      if (SUCCEEDED (result_2))
      {
        device_path =
          ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant.bstrVal));
        VariantClear (&variant);
      } // end IF
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_PATH_STRING),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

      result_2 =
        properties_p->Read (MODULE_DEV_DIRECTSHOW_PROPERTIES_ID_STRING,
                            &variant,
                            0);
      if (SUCCEEDED (result_2))
      {
        wave_in_id = variant.intVal;
        VariantClear (&variant);
      } // end IF
      else
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                    ACE_TEXT_WCHAR_TO_TCHAR (MODULE_DEV_DIRECTSHOW_PROPERTIES_ID_STRING),
                    ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));

      properties_p->Release ();
      properties_p = NULL;

      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("found device \"%s\": \"%s\" @ %s (id: %d)...\n"),
                  ACE_TEXT (friendly_name_string.c_str ()),
                  ACE_TEXT (description_string.c_str ()),
                  ACE_TEXT (device_path.c_str ()),
                  wave_in_id));

      gtk_list_store_append (listStore_in, &iterator);
      gtk_list_store_set (listStore_in, &iterator,
                          0, friendly_name_string.c_str (),
                          -1);
    } // end WHILE
    enum_moniker_p->Release ();
    enum_moniker_p = NULL;

    result = true;

    goto continue_;

error_2:
    if (enumerator_p)
      enumerator_p->Release ();
    if (enum_moniker_p)
      enum_moniker_p->Release ();
    if (moniker_p)
      moniker_p->Release ();
    if (properties_p)
      properties_p->Release ();
    VariantClear (&variant);
  } // end ELSE
#else
  void** hints_p = NULL;
  int result_2 =
      snd_device_name_hint (-1,
                            ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_ALSA_DEFAULT_INTERFACE_NAME),
                            &hints_p);
  if (result_2 < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_device_name_hint(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result_2))));
    return false;
  } // end IF

  std::string device_name_string;
  char* string_p = NULL;
  for (void** i = hints_p;
       *i;
       ++i)
  {
    string_p = snd_device_name_get_hint (*i, "IOID");
    if (!string_p)
    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
      goto continue_;
    } // end IF
    if (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("Input")))
    {
      // clean up
      free (string_p);
      string_p = NULL;

      continue;
    } // end IF

continue_:
    string_p = snd_device_name_get_hint (*i, "NAME");
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
      goto clean;
    } // end IF
    if ((ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("default")) == 0)                   ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("dmix:CARD=MID,DEV=0")) == 0)       ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("dsnoop:CARD=MID,DEV=0")) == 0)     ||
//        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("hw:CARD=MID,DEV=0")) == 0)         ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("front:CARD=MID,DEV=0")) == 0)      ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("null")) == 0)                      ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("plughw:CARD=MID,DEV=0")) == 0)     ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("pulse")) == 0)                     ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("sysdefault:CARD=MID")) == 0)       ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround21:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround40:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround41:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround50:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround51:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround71:CARD=MID,DEV=0")) == 0))
    {
      // clean up
      free (string_p);
      string_p = NULL;

      continue;
    } // end IF
    device_name_string = string_p;
    free (string_p);
    string_p = NULL;
    string_p = snd_device_name_get_hint (*i, "DESC");
    if (!string_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_get_hint(): \"%m\", aborting\n")));
      goto clean;
    } // end IF

    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT (string_p),
                        1, ACE_TEXT (device_name_string.c_str ()),
                        -1);

    // clean up
    free (string_p);
    string_p = NULL;
  } // end IF
  result = true;

clean:
  if (hints_p)
  {
    result_2 = snd_device_name_free_hint (hints_p);
    if (result_2 < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_device_name_free_hint(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result_2))));
  } // end IF
#endif
#if defined (ACE_WIN32) || defined (ACE_WIN64)
continue_:
#endif

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct less_guid
{
  bool operator () (const struct _GUID& lhs_in,
                    const struct _GUID& rhs_in) const
  {
    //ACE_ASSERT (lhs_in.Data2 == rhs_in.Data2);
    //ACE_ASSERT (lhs_in.Data3 == rhs_in.Data3);
    //ACE_ASSERT (*(long long*)lhs_in.Data4 == *(long long*)rhs_in.Data4);
    return (lhs_in.Data1 < rhs_in.Data1);
  }
};

bool
load_formats (IAMStreamConfig* IAMStreamConfig_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  int count = 0, size = 0;
  std::set<struct _GUID, less_guid> GUIDs;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _AUDIO_STREAM_CONFIG_CAPS capabilities;
  struct tagWAVEFORMATEX* waveformatex_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (media_type_p->formattype != FORMAT_WaveFormatEx)
    {
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF

    // *NOTE*: FORMAT_VideoInfo2 types do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    GUIDs.insert (media_type_p->subtype);

    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end FOR

  std::string media_subtype_string;
  std::string GUID_stdstring;
  GtkTreeIter iterator;
  OLECHAR GUID_string[CHARS_IN_GUID];
  ACE_OS::memset (GUID_string, 0, sizeof (GUID_string));
  for (std::set<GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    int result_2 = StringFromGUID2 (*iterator_2,
                                    GUID_string, CHARS_IN_GUID);
    ACE_ASSERT (result_2 == CHARS_IN_GUID);
    GUID_stdstring =
      ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (GUID_string));
    gtk_list_store_append (listStore_in, &iterator);
    media_subtype_string =
      Stream_Module_Device_Tools::mediaSubTypeToString (*iterator_2);
    gtk_list_store_set (listStore_in, &iterator,
                        0, media_subtype_string.c_str (),
                        1, GUID_stdstring.c_str (),
                        -1);
  } // end FOR

  return true;
}
bool
//load_formats (IMFSourceReader* IMFSourceReader_in,
load_formats (IMFMediaSource* IMFMediaSource_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // sanity check(s)
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (IMFMediaSource_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = S_OK;
  //int count = 0, size = 0;
  std::set<struct _GUID, less_guid> GUIDs;
  std::string media_subtype_string;
  std::string GUID_stdstring;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_descriptor_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = { 0 };
  while (result == S_OK)
  {
    media_type_p = NULL;
    result =
      //IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                        count,
      //                                        &media_type_p);
      media_type_handler_p->GetMediaTypeByIndex (count,
                                                 &media_type_p);
    if (result != S_OK) break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF

    GUIDs.insert (GUID_s);
    media_type_p->Release ();

    ++count;
  } // end WHILE
  media_type_handler_p->Release ();
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  GtkTreeIter iterator;
  OLECHAR GUID_string[CHARS_IN_GUID];
  ACE_OS::memset (GUID_string, 0, sizeof (GUID_string));
  for (std::set<struct _GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    int result_2 = StringFromGUID2 (*iterator_2,
                                    GUID_string, CHARS_IN_GUID);
    ACE_ASSERT (result_2 == CHARS_IN_GUID);
    GUID_stdstring =
      ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (GUID_string));
    gtk_list_store_append (listStore_in, &iterator);
    media_subtype_string =
      Stream_Module_Device_Tools::mediaSubTypeToString (*iterator_2);
    gtk_list_store_set (listStore_in, &iterator,
                        0, media_subtype_string.c_str (),
                        1, GUID_stdstring.c_str (),
                        -1);
  } // end FOR

  return true;
}

bool
load_sample_rates (IAMStreamConfig* IAMStreamConfig_in,
                   const struct _GUID& mediaSubType_in,
                   GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  int count = 0, size = 0;
  std::set<DWORD> sample_rates;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _AUDIO_STREAM_CONFIG_CAPS capabilities;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);

    if ((media_type_p->subtype != mediaSubType_in) ||
        (media_type_p->formattype != FORMAT_WaveFormatEx))
    {
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF

    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    sample_rates.insert (waveformatex_p->nSamplesPerSec);

    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<DWORD>::const_iterator iterator_2 = sample_rates.begin ();
       iterator_2 != sample_rates.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  return true;
}
bool
//load_sample_rates (IMFSourceReader* IMFSourceReader_in,
load_sample_rates (IMFMediaSource* IMFMediaSource_in,
                   const struct _GUID& mediaSubType_in,
                   GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaSource_in);
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = S_OK;
  std::set<unsigned int> sample_rates;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_descriptor_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = { 0 };
  UINT32 samples_per_second;
  while (result == S_OK)
  {
    media_type_p = NULL;
    result =
      //IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                        count,
      //                                        &media_type_p);
      media_type_handler_p->GetMediaTypeByIndex (count,
                                                 &media_type_p);
    if (result != S_OK) break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF

    if (GUID_s == mediaSubType_in)
    {
      result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                        &samples_per_second);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        media_type_handler_p->Release ();
        media_type_p->Release ();

        return false;
      } // end IF
      sample_rates.insert (samples_per_second);
    } // end IF
    media_type_p->Release ();

    ++count;
  } // end WHILE
  media_type_handler_p->Release ();
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<unsigned int>::const_iterator iterator_2 = sample_rates.begin ();
       iterator_2 != sample_rates.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  return true;
}

bool
load_sample_resolutions (IAMStreamConfig* IAMStreamConfig_in,
                         const struct _GUID& mediaSubType_in,
                         unsigned int sampleRate_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  int count = 0, size = 0;
  std::set<WORD> sample_resolutions;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _AUDIO_STREAM_CONFIG_CAPS capabilities;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);

    if ((media_type_p->subtype != mediaSubType_in) ||
        (media_type_p->formattype != FORMAT_WaveFormatEx))
    {
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    if (waveformatex_p->nSamplesPerSec != sampleRate_in)
    {
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    sample_resolutions.insert (waveformatex_p->wBitsPerSample);

    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<WORD>::const_iterator iterator_2 = sample_resolutions.begin ();
       iterator_2 != sample_resolutions.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  return true;
}
bool
//load_sample_resolutions (IMFSourceReader* IMFSourceReader_in,
load_sample_resolutions (IMFMediaSource* IMFMediaSource_in,
                         const struct _GUID& mediaSubType_in,
                         unsigned int sampleRate_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_resolutions"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaSource_in);
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  std::set<unsigned int> sample_resolutions;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_descriptor_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = { 0 };
  UINT32 sample_rate, bits_per_sample;
  while (result == S_OK)
  {
    media_type_p = NULL;
    result =
      //IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                        count,
      //                                        &media_type_p);
      media_type_handler_p->GetMediaTypeByIndex (count,
                                                 &media_type_p);
    if (result != S_OK) break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF

    if ((GUID_s == mediaSubType_in) &&
        (sample_rate == sampleRate_in))
    {
      result = media_type_p->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                        &bits_per_sample);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        media_type_handler_p->Release ();
        media_type_p->Release ();

        return false;
      } // end IF
      sample_resolutions.insert (bits_per_sample);
    } // end IF
    media_type_p->Release ();

    ++count;
  } // end WHILE
  media_type_handler_p->Release ();
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<unsigned int>::const_iterator iterator_2 = sample_resolutions.begin ();
       iterator_2 != sample_resolutions.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  return true;
}

bool
load_channels (IAMStreamConfig* IAMStreamConfig_in,
               const struct _GUID& mediaSubType_in,
               unsigned int sampleRate_in,
               unsigned int bitePerSample_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // sanity check(s)
  ACE_ASSERT (IAMStreamConfig_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  int count = 0, size = 0;
  std::set<WORD> channels;
  result = IAMStreamConfig_in->GetNumberOfCapabilities (&count, &size);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IAMStreamConfig::GetNumberOfCapabilities(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  struct _AMMediaType* media_type_p = NULL;
  struct _AUDIO_STREAM_CONFIG_CAPS capabilities;
  struct tWAVEFORMATEX* waveformatex_p = NULL;
  for (int i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result = IAMStreamConfig_in->GetStreamCaps (i,
                                                &media_type_p,
                                                (BYTE*)&capabilities);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IAMStreamConfig::GetStreamCaps(%d): \"%s\", aborting\n"),
                  i,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);

    if ((media_type_p->subtype != mediaSubType_in) ||
        (media_type_p->formattype != FORMAT_WaveFormatEx))
    {
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    if (waveformatex_p->nSamplesPerSec != sampleRate_in)
    {
      Stream_Module_Device_Tools::deleteMediaType (media_type_p);
      continue;
    } // end IF
    channels.insert (waveformatex_p->nChannels);

    Stream_Module_Device_Tools::deleteMediaType (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::set<WORD>::const_iterator iterator_2 = channels.begin ();
       iterator_2 != channels.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  return true;
}
bool
//load_channels (IMFSourceReader* IMFSourceReader_in,
load_channels (IMFMediaSource* IMFMediaSource_in,
               const struct _GUID& mediaSubType_in,
               unsigned int sampleRate_in,
               unsigned int bitsPerSample_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_channels"));

  // sanity check(s)
  ACE_ASSERT (IMFMediaSource_in);
  //ACE_ASSERT (IMFSourceReader_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  HRESULT result = E_FAIL;
  std::vector<unsigned int> channels;
  IMFPresentationDescriptor* presentation_descriptor_p = NULL;
  result =
    IMFMediaSource_in->CreatePresentationDescriptor (&presentation_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFMediaSource::CreatePresentationDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF
  IMFStreamDescriptor* stream_descriptor_p = NULL;
  BOOL is_selected = FALSE;
  result =
    presentation_descriptor_p->GetStreamDescriptorByIndex (0,
                                                           &is_selected,
                                                           &stream_descriptor_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFPresentationDescriptor::GetStreamDescriptor(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    presentation_descriptor_p->Release ();

    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release ();
  presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));

    // clean up
    stream_descriptor_p->Release ();

    return false;
  } // end IF
  stream_descriptor_p->Release ();
  stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = { 0 };
  UINT32 sample_rate, bits_per_sample, number_of_channels;
  while (result == S_OK)
  {
    media_type_p = NULL;
    result =
      //IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                        count,
      //                                        &media_type_p);
      media_type_handler_p->GetMediaTypeByIndex (count,
                                                 &media_type_p);
    if (result != S_OK) break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                      &bits_per_sample);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));

      // clean up
      media_type_handler_p->Release ();
      media_type_p->Release ();

      return false;
    } // end IF

    if ((GUID_s          == mediaSubType_in) &&
        (sample_rate     == sampleRate_in)   &&
        (bits_per_sample == bitsPerSample_in))
    {
      result = media_type_p->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                        &number_of_channels);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Tools::error2String (result).c_str ())));

        // clean up
        media_type_handler_p->Release ();
        media_type_p->Release ();

        return false;
      } // end IF
      channels.push_back (number_of_channels);
    } // end IF
    media_type_p->Release ();

    ++count;
  } // end WHILE
  media_type_handler_p->Release ();
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return false;
  } // end IF

  GtkTreeIter iterator;
  std::ostringstream converter;
  for (std::vector<unsigned int>::const_iterator iterator_2 = channels.begin ();
       iterator_2 != channels.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  return true;
}
#else
bool
load_formats (struct _snd_pcm* handle_in,
              const Stream_Module_Device_ALSAConfiguration& format_in,
              GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_formats"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct _snd_pcm_hw_params* format_p = NULL;
  snd_pcm_format_mask_t* format_mask_p = NULL;
  std::set<snd_pcm_format_t> formats_supported;
  GtkTreeIter iterator;

  //    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  int result = snd_pcm_hw_params_any (handle_in,
                                      format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (handle_in,
                                         format_p,
                                         format_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_format_mask_malloc (&format_mask_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_format_mask_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  snd_pcm_hw_params_get_format_mask (format_p, format_mask_p);
  for (int i = 0;
       i < SND_PCM_FORMAT_LAST;
       ++i)
  {
    result =
//        snd_pcm_hw_params_test_format (handle_in,
//                                       format_in,
//                                       static_cast<enum _snd_pcm_format> (i));
        snd_pcm_format_mask_test (format_mask_p,
                                  static_cast<enum _snd_pcm_format> (i));
    if (result == 0)
      formats_supported.insert (static_cast<enum _snd_pcm_format> (i));
  } // end FOR
  snd_pcm_format_mask_free (format_mask_p);

  for (std::set<snd_pcm_format_t>::const_iterator iterator_2 = formats_supported.begin ();
       iterator_2 != formats_supported.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
//                        0, snd_pcm_format_name (*iterator_2),
                        0, snd_pcm_format_description (*iterator_2),
                        2, *iterator_2,
                        -1);
  } // end FOR

  snd_pcm_hw_params_free (format_p);

  return true;

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);

  return false;
}

bool
load_sample_rates (struct _snd_pcm* handle_in,
                   const Stream_Module_Device_ALSAConfiguration& format_in,
                   GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_rates"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct _snd_pcm_hw_params* format_p = NULL;
  unsigned int rate_min, rate_max;
  int subunit_direction = 0;
  std::set<unsigned int> sample_rates_supported;
  GtkTreeIter iterator;
  std::ostringstream converter;

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  int result = snd_pcm_hw_params_any (handle_in,
                                      format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (handle_in,
                                         format_p,
                                         format_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         format_in.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
//    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_rate_min (format_p,
                                           &rate_min,
                                           &subunit_direction);
  ACE_ASSERT (result == 0);
  result = snd_pcm_hw_params_get_rate_max (format_p,
                                           &rate_max,
                                           &subunit_direction);
  ACE_ASSERT (result == 0);
  if (rate_min < rate_max)
  {
    for (unsigned int i = rate_min;
         i <= rate_max;
         ++i)
    {
      result = snd_pcm_hw_params_test_rate (handle_in,
                                            format_p,
                                            i,
                                            0);
      if (result == 0)
        sample_rates_supported.insert (i);
    } // end FOR
  } // end IF
  else
    sample_rates_supported.insert (rate_min);

  for (std::set<unsigned int>::const_iterator iterator_2 = sample_rates_supported.begin ();
       iterator_2 != sample_rates_supported.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  snd_pcm_hw_params_free (format_p);

  return true;

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);

  return false;
}

bool
load_sample_resolutions (struct _snd_pcm* handle_in,
                         const Stream_Module_Device_ALSAConfiguration& format_in,
                         GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_sample_resolutions"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  int subunit_direction = 0;
  struct _snd_pcm_hw_params* format_p = NULL;
  std::set<int> resolutions_supported;
  GtkTreeIter iterator;
  std::ostringstream converter;

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  int result = snd_pcm_hw_params_any (handle_in,
                                      format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (handle_in,
                                         format_p,
                                         format_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         format_in.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
//    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_rate_near (handle_in,
                                       format_p,
                                       &const_cast<Stream_Module_Device_ALSAConfiguration&> (format_in).rate,
                                       &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_rate_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  resolutions_supported.insert (snd_pcm_format_width (format_in.format));
//  for (unsigned int* i = resolutions;
//       *i;
//       ++i)
//  {
//    result = snd_pcm_hw_params_test_format (handle_in,
//                                            format_in,
//                                            *i,
//                                            -1);
//    if (result == 0)
//      resolutions_supported.insert (*i);
//  } // end FOR

  for (std::set<int>::const_iterator iterator_2 = resolutions_supported.begin ();
       iterator_2 != resolutions_supported.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  snd_pcm_hw_params_free (format_p);

  return true;

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);

  return false;
}

bool
load_channels (struct _snd_pcm* handle_in,
               const Stream_Module_Device_ALSAConfiguration& format_in,
               GtkListStore* listStore_in)
{
  STREAM_TRACE (ACE_TEXT ("::load_channels"));

  // sanity check(s)
  ACE_ASSERT (handle_in);
  ACE_ASSERT (listStore_in);

  // initialize result
  gtk_list_store_clear (listStore_in);

  struct _snd_pcm_hw_params* format_p = NULL;
  int subunit_direction = 0;
  unsigned int channels_min, channels_max;
  std::set<unsigned int> channels_supported;
  GtkTreeIter iterator;
  std::ostringstream converter;

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    return false;
  } // end IF
  int result = snd_pcm_hw_params_any (handle_in,
                                      format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_set_access (handle_in,
                                         format_p,
                                         format_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         format_in.format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_format(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
//    goto error;
  } // end IF
  result =
      snd_pcm_hw_params_set_rate_near (handle_in,
                                       format_p,
                                       &const_cast<Stream_Module_Device_ALSAConfiguration&> (format_in).rate,
                                       &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_rate_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  result = snd_pcm_hw_params_get_channels_min (format_p,
                                               &channels_min);
  ACE_ASSERT (result == 0);
  result = snd_pcm_hw_params_get_channels_max (format_p,
                                               &channels_max);
  ACE_ASSERT (result == 0);
  if (channels_min < channels_max)
  {
    for (unsigned int i = channels_min;
         i <= channels_max;
         ++i)
    {
      result = snd_pcm_hw_params_test_channels (handle_in,
                                                format_p,
                                                i);
      if (result == 0)
        channels_supported.insert (i);
    } // end FOR
  } // end IF
  else
    channels_supported.insert (channels_min);

  for (std::set<unsigned int>::const_iterator iterator_2 = channels_supported.begin ();
       iterator_2 != channels_supported.end ();
       ++iterator_2)
  {
    converter.clear ();
    converter.str (ACE_TEXT_ALWAYS_CHAR (""));
    converter << *iterator_2;
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, converter.str ().c_str (),
                        1, *iterator_2,
                        -1);
  } // end FOR

  snd_pcm_hw_params_free (format_p);

  return true;

error:
  if (format_p)
    snd_pcm_hw_params_free (format_p);

  return false;
}
#endif

unsigned int
get_buffer_size (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::get_buffer_size"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
    return 0; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = { 0, };
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
#endif
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result = CLSIDFromString (format_string.c_str (), &GUID_i);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (format_string.c_str ()), &GUID_s);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return 0;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
    return 0; // <-- nothing selected
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int sample_rate = g_value_get_uint (&value);
  g_value_unset (&value);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
    return 0; // <-- nothing selected
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int bits_per_sample = g_value_get_uint (&value);
  g_value_unset (&value);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
    return 0; // <-- nothing selected
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int channels = g_value_get_uint (&value);
  g_value_unset (&value);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  return (sample_rate * (bits_per_sample / 8) * channels);
#else
  ACE_UNUSED_ARG (bits_per_sample);
  return (sample_rate * snd_pcm_format_size (format_e, 1) * channels);
#endif
}
void
update_buffer_size (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::update_buffer_size"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);

  gtk_spin_button_set_value (spin_button_p,
                             static_cast<gdouble> (get_buffer_size (userData_in)));
}

//////////////////////////////////////////

ACE_THR_FUNC_RETURN
stream_processing_function (void* arg_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_processing_function"));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("processing thread (id was: %t) starting...\n")));

  ACE_THR_FUNC_RETURN result;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = std::numeric_limits<unsigned long>::max ();
#else
  result = arg_in;
#endif

  Test_U_AudioEffect_ThreadData* data_p =
      static_cast<Test_U_AudioEffect_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_ThreadData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_ThreadData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_ThreadData*> (arg_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->CBData);
    ACE_ASSERT (mediafoundation_data_p->CBData->configuration);
    ACE_ASSERT (mediafoundation_data_p->CBData->stream);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_ThreadData*> (arg_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->CBData);
    ACE_ASSERT (directshow_data_p->CBData->configuration);
    ACE_ASSERT (directshow_data_p->CBData->stream);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->CBData->configuration);
  ACE_ASSERT (data_p->CBData->stream);
#endif

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  const Test_U_AudioEffect_DirectShow_SessionData_t* directshow_session_data_container_p =
    NULL;
  Test_U_AudioEffect_DirectShow_SessionData* directshow_session_data_p =
    NULL;
  const Test_U_AudioEffect_MediaFoundation_SessionData_t* mediafoundation_session_data_container_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_SessionData* mediafoundation_session_data_p =
    NULL;
#else
  const Test_U_AudioEffect_SessionData_t* session_data_container_p = NULL;
  Test_U_AudioEffect_SessionData* session_data_p = NULL;
#endif

//  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  std::ostringstream converter;
  //ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->CBData->lock);

  Common_UI_GTKBuildersIterator_t iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
    iterator =
      mediafoundation_data_p->CBData->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  else
    iterator =
      directshow_data_p->CBData->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
#else
  iterator =
    data_p->CBData->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->CBData->builders.end ());
#endif

  // retrieve progress bar handle
  gdk_threads_enter ();
//    progress_bar_p =
//      GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
//                                                ACE_TEXT_ALWAYS_CHAR (TEST_USTREAM_UI_GTK_PROGRESSBAR_NAME)));
//    ACE_ASSERT (progress_bar_p);

  // generate context ID
  statusbar_p =
    GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_STATUSBAR_NAME)));
  ACE_ASSERT (statusbar_p);

  gdk_threads_leave ();

  bool result_2 = false;
  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    result_2 =
      mediafoundation_data_p->CBData->stream->initialize (mediafoundation_data_p->CBData->configuration->streamConfiguration);
    stream_p = mediafoundation_data_p->CBData->stream;
  } // end IF
  else
  {
    result_2 =
      directshow_data_p->CBData->stream->initialize (directshow_data_p->CBData->configuration->streamConfiguration);
    stream_p = directshow_data_p->CBData->stream;
  } // end ELSE
#else
  result_2 =
    data_p->CBData->stream->initialize (data_p->CBData->configuration->streamConfiguration);
  stream_p = data_p->CBData->stream;
#endif
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream: \"%m\", aborting\n")));
    goto error;
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_SessionData* session_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_session_data_container_p =
      mediafoundation_data_p->CBData->stream->get ();
    mediafoundation_session_data_p =
      &const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (mediafoundation_session_data_container_p->get ());
    session_data_p = mediafoundation_session_data_p;
  } // end IF
  else
  {
    directshow_session_data_container_p =
      directshow_data_p->CBData->stream->get ();
    directshow_session_data_p =
      &const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (directshow_session_data_container_p->get ());
    session_data_p = directshow_session_data_p;
  } // end ELSE
#else
  session_data_container_p = data_p->CBData->stream->get ();
  ACE_ASSERT (session_data_container_p);
  session_data_p =
      &const_cast<Test_U_AudioEffect_SessionData&> (session_data_container_p->get ());
#endif
  data_p->sessionID = session_data_p->sessionID;
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionID;

  // generate context ID
  gdk_threads_enter ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p->CBData->configuration->moduleHandlerConfiguration.contextID =
      gtk_statusbar_get_context_id (statusbar_p,
                                    converter.str ().c_str ());
   } // end IF
   else
   {
     directshow_data_p->CBData->configuration->moduleHandlerConfiguration.contextID =
       gtk_statusbar_get_context_id (statusbar_p,
                                     converter.str ().c_str ());
   } // end ELSE
#else
  data_p->CBData->configuration->moduleHandlerConfiguration.contextID =
    gtk_statusbar_get_context_id (statusbar_p,
                                  converter.str ().c_str ());
#endif

  gdk_threads_leave ();

  stream_p->start ();
  //if (!data_p->CBData->stream->isRunning ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Test_U_AudioEffect_Stream::start(): \"%m\", aborting\n")));
  //  goto done;
  //} // end IF
  stream_p->wait (true, false, false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif

error:
  //guint event_source_id = g_idle_add (idle_session_end_cb,
  //                                    data_p->CBData);
  //if (event_source_id == 0)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
  //else
  //  data_p->CBData->eventSourceIds.insert (event_source_id);

  { // synch access
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (data_p->useMediaFoundation)
    {
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, mediafoundation_data_p->CBData->lock, -1);
      mediafoundation_data_p->CBData->progressData.completedActions.insert (mediafoundation_data_p->eventSourceID);
    } // end IF
    else
    {
      ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, directshow_data_p->CBData->lock, -1);
      directshow_data_p->CBData->progressData.completedActions.insert (directshow_data_p->eventSourceID);
    } // end ELSE
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->CBData->lock, std::numeric_limits<void*>::max ());
    data_p->CBData->progressData.completedActions.insert (data_p->eventSourceID);
#endif
  } // end lock scope

  // clean up
  delete data_p;

  return result;
}

//////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
  } // end ELSE
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL, COINIT_MULTITHREADED);
  if (FAILED (hresult))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (hresult).c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF
#endif

  // step1: initialize dialog window(s)
  GtkWidget* dialog_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);
  //gtk_window4096_set_title (,
  //                      caption.c_str ());

//  GtkWidget* about_dialog_p =
//    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
//                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
//  ACE_ASSERT (about_dialog_p);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_capture_devices (list_store_p,
                             data_p->useMediaFoundation))
#else
  if (!load_capture_devices (list_store_p))
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_capture_devices(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  GtkCellRenderer* cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_ASCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME)));
  ACE_ASSERT (combo_box_p);
  //gtk_combo_box_set_model (combo_box_p,
  //                         GTK_TREE_MODEL (list_store_p));
  cell_renderer_p = gtk_cell_renderer_text_new ();
  if (!cell_renderer_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_cell_renderer_text_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                              true);
  // *NOTE*: cell_renderer_p does not need to be g_object_unref()ed because it
  //         is GInitiallyUnowned and the floating reference has been
  //         passed to combo_box_p by the gtk_cell_layout_pack_start() call
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box_p), cell_renderer_p,
                                  //"cell-background", 0,
                                  //"text", 1,
                                  "text", 0,
                                  NULL);

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);
  GtkFileFilter* file_filter_p =
    GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILEFILTER_AVI_NAME)));
  ACE_ASSERT (file_filter_p);
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("application/x-troff-msvideo"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("video/avi"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("video/msvideo"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("video/x-msvideo"));
  gtk_file_filter_add_pattern (file_filter_p,
                               ACE_TEXT ("*.avi"));
  gtk_file_filter_set_name (file_filter_p,
                            ACE_TEXT ("AVI files"));
  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  gchar* filename_p = NULL;

  gdouble buffer_size = 0.0;
  std::string filename;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    buffer_size =
      mediafoundation_data_p->configuration->streamConfiguration.bufferSize;
    filename =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.targetFileName;
  } // end IF
  else
  {
    buffer_size =
      directshow_data_p->configuration->streamConfiguration.bufferSize;
    filename =
      directshow_data_p->configuration->moduleHandlerConfiguration.targetFileName;
  } // end ELSE
#else
  buffer_size = data_p->configuration->streamConfiguration.bufferSize;
  filename =
    data_p->configuration->moduleHandlerConfiguration.targetFileName;
#endif
  if (!filename.empty ())
  {
    // *NOTE*: gtk does not complain if the file doesn't exist, but the button
    //         will display "(None)" --> create empty file
    if (!Common_File_Tools::isReadable (filename))
      if (!Common_File_Tools::create (filename))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_File_Tools::create(\"%s\"): \"%m\", aborting\n"),
                    ACE_TEXT (filename.c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF
    //file_p =
    //  g_file_new_for_path (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ());
    //ACE_ASSERT (file_p);
    //ACE_ASSERT (g_file_query_exists (file_p, NULL));

    //std::string file_uri =
    //  ACE_TEXT_ALWAYS_CHAR ("file://") +
    //  data_p->configuration->moduleHandlerConfiguration.targetFileName;
    //if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
    //                                              file_uri.c_str ()))
    filename_p =
      Common_UI_Tools::Locale2UTF8 (filename);
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (filename.c_str ())));

      // clean up
      g_free (filename_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_free (filename_p);

    //if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
    //                                   file_p,
    //                                   &error_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", aborting\n"),
    //              ACE_TEXT (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ()),
    //              ACE_TEXT (error_p->message)));

    //  // clean up
    //  g_error_free (error_p);
    //  g_object_unref (file_p);

    //  return G_SOURCE_REMOVE;
    //} // end IF
    //g_object_unref (file_p);
  } // end IF
  else
  {
    //file_p =
    //  g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
    //ACE_ASSERT (file_p);

    filename_p =
      Common_UI_Tools::Locale2UTF8 (Common_File_Tools::getTempDirectory ());
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ())));

      // clean up
      g_free (filename_p);

      return G_SOURCE_REMOVE;
    } // end IF
    g_free (filename_p);
    //g_object_unref (file_p);
  } // end ELSE

  GtkCheckButton* check_button_p =
    GTK_CHECK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME)));
  ACE_ASSERT (check_button_p);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button_p),
                                !filename.empty ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<double>::max ());
  gtk_spin_button_set_value (spin_button_p, buffer_size);

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  gint width, height;
  gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  gtk_progress_bar_set_pulse_step (progress_bar_p,
                                   1.0 / static_cast<double> (width));
  gtk_progress_bar_set_text (progress_bar_p,
                             ACE_TEXT_ALWAYS_CHAR (""));

  // step4: initialize text view, setup auto-scrolling
  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p =
////    gtk_text_buffer_new (NULL); // text tag table --> create new
//      gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
////  gtk_text_view_set_buffer (view_p, buffer_p);

  PangoFontDescription* font_description_p =
    pango_font_description_from_string (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION));
  if (!font_description_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to pango_font_description_from_string(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_STREAM_UI_GTK_PANGO_LOG_FONT_DESCRIPTION)));
    return G_SOURCE_REMOVE;
  } // end IF
  // apply font
  GtkRcStyle* rc_style_p = gtk_rc_style_new ();
  if (!rc_style_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_rc_style_new(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  rc_style_p->font_desc = font_description_p;
  GdkColor base_colour, text_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_BASE),
                   &base_colour);
  rc_style_p->base[GTK_STATE_NORMAL] = base_colour;
  gdk_color_parse (ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PANGO_LOG_COLOR_TEXT),
                   &text_colour);
  rc_style_p->text[GTK_STATE_NORMAL] = text_colour;
  rc_style_p->color_flags[GTK_STATE_NORMAL] =
    static_cast<GtkRcFlags> (GTK_RC_BASE |
                             GTK_RC_TEXT);
  gtk_widget_modify_style (GTK_WIDGET (view_p),
                           rc_style_p);
  //gtk_rc_style_unref (rc_style_p);
  g_object_unref (rc_style_p);

  //  GtkTextIter iterator;
  //  gtk_text_buffer_get_end_iter (buffer_p,
  //                                &iterator);
  //  gtk_text_buffer_create_mark (buffer_p,
  //                               ACE_TEXT_ALWAYS_CHAR (NET_UI_SCROLLMARK_NAME),
  //                               &iterator,
  //                               TRUE);
  //  g_object_unref (buffer_p);

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  // step5: initialize updates
  {
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);

    // schedule asynchronous updates of the log view
    guint event_source_id = g_timeout_add_seconds (1,
                                                   idle_update_log_display_cb,
                                                   userData_in);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_seconds(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
    // schedule asynchronous updates of the info view
    event_source_id = g_timeout_add (TEST_U_STREAM_UI_GTKEVENT_RESOLUTION,
                                     idle_update_info_display_cb,
                                     userData_in);
    if (event_source_id > 0)
      data_p->eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  // step6: disable some functions ?
  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, FALSE);

  // step7: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_autoconnect does not work reliably
  //glade_xml_signal_autoconnect(userData_out.xml);

  // step6a: connect default signals
  gulong result_2 =
      g_signal_connect (dialog_p,
                        ACE_TEXT_ALWAYS_CHAR ("destroy"),
                        G_CALLBACK (gtk_widget_destroyed),
                        NULL);
  ACE_ASSERT (result_2);

  result_2 =
    g_signal_connect (file_chooser_button_p,
                      ACE_TEXT_ALWAYS_CHAR ("file-set"),
                      G_CALLBACK (filechooserbutton_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  //result_2 =
  //  g_signal_connect (file_chooser_dialog_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("file-activated"),
  //                    G_CALLBACK (filechooserdialog_cb),
  //                    NULL);
  //ACE_ASSERT (result_2);

  // step6b: connect custom signals
  GObject* object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME));
  ACE_ASSERT (object_p);
  result_2 = g_signal_connect (object_p,
                               ACE_TEXT_ALWAYS_CHAR ("toggled"),
                               G_CALLBACK (toggleaction_record_toggled_cb),
                               userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_cut_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("activate"),
                      G_CALLBACK (action_report_activate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("changed"),
                        G_CALLBACK (combobox_source_changed_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_format_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_frequency_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_resolution_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("changed"),
                      G_CALLBACK (combobox_channels_changed_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_SAVE_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("toggled"),
                      G_CALLBACK (toggleaction_save_toggled_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (textview_size_allocate_cb),
                      userData_in);
  ACE_ASSERT (result_2);

  //--------------------------------------

  result_2 =
      g_signal_connect (G_OBJECT (drawing_area_p),
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
                        ACE_TEXT_ALWAYS_CHAR ("draw"),
#else
                        ACE_TEXT_ALWAYS_CHAR ("expose-event"),
#endif
                        G_CALLBACK (drawingarea_draw_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  result_2 =
//    g_signal_connect (G_OBJECT (drawing_area_p),
//                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
//                      G_CALLBACK (drawingarea_configure_event_cb),
//                      userData_in);
      g_signal_connect (G_OBJECT (drawing_area_p),
                        ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                        G_CALLBACK (drawingarea_size_allocate_cb),
                        userData_in);
  ACE_ASSERT (result_2);

  //--------------------------------------

  object_p =
    gtk_builder_get_object ((*iterator).second.second,
                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME));
  ACE_ASSERT (object_p);
  result_2 =
    g_signal_connect (object_p,
                      ACE_TEXT_ALWAYS_CHAR ("clicked"),
                      G_CALLBACK (button_clear_clicked_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_ABOUT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_about_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  object_p =
      gtk_builder_get_object ((*iterator).second.second,
                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_QUIT_NAME));
  ACE_ASSERT (object_p);
  result_2 =
      g_signal_connect (object_p,
                        ACE_TEXT_ALWAYS_CHAR ("clicked"),
                        G_CALLBACK (button_quit_clicked_cb),
                        userData_in);
  ACE_ASSERT (result_2);
  ACE_UNUSED_ARG (result_2);

  // set defaults
  //file_chooser_button_p =
  //  GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += filename;
  gboolean result =
    gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                             default_folder_uri.c_str ());
  if (!result)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (default_folder_uri.c_str ())));
    return G_SOURCE_REMOVE;
  } // end IF

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (dialog_p);

  // step10: retrieve canvas coordinates, window handle and pixel buffer
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  data_p->device =
      data_p->configuration->moduleHandlerConfiguration.captureDeviceHandle;
#endif

  GtkAllocation allocation;
  ACE_OS::memset (&allocation, 0, sizeof (allocation));
  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
                             &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  //data_p->configuration->moduleHandlerConfiguration.area.bottom =
  //  allocation.y + allocation.height;
  //data_p->configuration->moduleHandlerConfiguration.area.left = allocation.x;
  //data_p->configuration->moduleHandlerConfiguration.area.right =
  //  allocation.x + allocation.width;
  //data_p->configuration->moduleHandlerConfiguration.area.top = allocation.y;
  if (data_p->useMediaFoundation)
  {
    ACE_ASSERT (!mediafoundation_data_p->configuration->moduleHandlerConfiguration.gdkWindow);
    mediafoundation_data_p->configuration->moduleHandlerConfiguration.gdkWindow =
      gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.gdkWindow);
    mediafoundation_data_p->configuration->moduleHandlerConfiguration.area =
      allocation;
  } // end IF
  else
  {
    //ACE_ASSERT (gdk_win32_window_is_win32 (window_p));
    //data_p->configuration->moduleHandlerConfiguration.window =
    //  //gdk_win32_window_get_impl_hwnd (window_p);
    //  static_cast<HWND> (GDK_WINDOW_HWND (GDK_DRAWABLE (window_p)));
    ACE_ASSERT (!directshow_data_p->configuration->moduleHandlerConfiguration.gdkWindow);
    directshow_data_p->configuration->moduleHandlerConfiguration.gdkWindow =
      gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.gdkWindow);
    directshow_data_p->configuration->moduleHandlerConfiguration.area =
      allocation;
  } // end ELSE
#else
  ACE_ASSERT (!data_p->configuration->moduleHandlerConfiguration.gdkWindow);
  data_p->configuration->moduleHandlerConfiguration.gdkWindow =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.gdkWindow);
  data_p->configuration->moduleHandlerConfiguration.area = allocation;
#endif

  //GdkPixbuf* pixel_buffer_p = NULL;
  cairo_surface_t* surface_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    surface_p =
      gdk_window_create_similar_image_surface (directshow_data_p->configuration->moduleHandlerConfiguration.gdkWindow,
                                               CAIRO_FORMAT_RGB24,
                                               allocation.width, allocation.height,
                                               1);
    mediafoundation_data_p->cairoSurface = surface_p;
    mediafoundation_data_p->configuration->moduleHandlerConfiguration.cairoSurface =
      surface_p;
  } // end IF
  else
  {
    surface_p =
      gdk_window_create_similar_image_surface (directshow_data_p->configuration->moduleHandlerConfiguration.gdkWindow,
                                               CAIRO_FORMAT_RGB24,
                                               allocation.width, allocation.height,
                                               1);
    directshow_data_p->cairoSurface = surface_p;
    directshow_data_p->configuration->moduleHandlerConfiguration.cairoSurface =
      surface_p;
  } // end ELSE
#else
  surface_p =
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
      gdk_window_create_similar_image_surface (data_p->configuration->moduleHandlerConfiguration.gdkWindow,
                                               CAIRO_FORMAT_RGB24,
                                               allocation.width, allocation.height,
                                               1);
#else
      gdk_window_create_similar_surface (data_p->configuration->moduleHandlerConfiguration.gdkWindow,
                                         CAIRO_CONTENT_COLOR_ALPHA,
                                         allocation.width, allocation.height);
      cairo_surface_type_t type = cairo_surface_get_type (surface_p);
      ACE_ASSERT (type == CAIRO_SURFACE_TYPE_IMAGE);
#endif
  data_p->cairoSurface = surface_p;
  data_p->configuration->moduleHandlerConfiguration.cairoSurface =
    surface_p;
#endif
  if (!surface_p)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  // step11: select default capture source (if any)
  //         --> populate the options comboboxes
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
  else
  {
    GtkToggleAction* toggle_action_p =
        GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
    ACE_ASSERT (toggle_action_p);
    gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), false);
  } // end IF

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // clean up
  int result = -1;
  if (data_p->device)
  {
    result = snd_pcm_close (data_p->device);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    data_p->device = NULL;
  } // end IF
#endif

  // leave GTK
  gtk_main_quit ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  CoUninitialize ();
#endif

  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  // *IMPORTANT NOTE*: there are two major reasons for being here that are not
  //                   mutually exclusive, so there could be a race:
  //                   - user pressed stop
  //                   - there was an asynchronous error on the stream
  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_stock_id (GTK_ACTION (toggle_action_p), GTK_STOCK_MEDIA_RECORD);
  if (gtk_toggle_action_get_active (toggle_action_p))
  {
    un_toggling_stream = true;
    gtk_action_activate (GTK_ACTION (toggle_action_p));
  } // end IF

  GtkAction* action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, false);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, false);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  //// stop progress reporting
  //ACE_ASSERT (data_p->progressEventSourceID);
  //{
  //  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard_2, data_p->lock, G_SOURCE_REMOVE);

  //  if (!g_source_remove (data_p->progressEventSourceID))
  //    ACE_DEBUG ((LM_ERROR,
  //                ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                data_p->progressEventSourceID));
  //  data_p->eventSourceIds.erase (data_p->progressEventSourceID);
  //  data_p->progressEventSourceID = 0;

  //  ACE_OS::memset (&(data_p->progressData.statistic),
  //                  0,
  //                  sizeof (data_p->progressData.statistic));
  //} // end lock scope
  GtkProgressBar* progressbar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progressbar_p);
  // *NOTE*: this disables "activity mode" (in Gtk2)
  gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  gtk_progress_bar_set_text (progressbar_p, ACE_TEXT_ALWAYS_CHAR (""));
  gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);

  return G_SOURCE_REMOVE;
}

gboolean
idle_update_log_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);

  GtkTextIter text_iterator;
  gtk_text_buffer_get_end_iter (buffer_p,
                                &text_iterator);

  gchar* converted_text = NULL;
  { // synch access
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->logStackLock, G_SOURCE_REMOVE);

    // sanity check
    if (data_p->logStack.empty ()) return G_SOURCE_CONTINUE;

    // step1: convert text
    for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
         iterator_2 != data_p->logStack.end ();
         iterator_2++)
    {
      converted_text = Common_UI_Tools::Locale2UTF8 (*iterator_2);
      if (!converted_text)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
                    ACE_TEXT ((*iterator_2).c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF

      // step2: display text
      gtk_text_buffer_insert (buffer_p,
                              &text_iterator,
                              converted_text,
                              -1);

      // clean up
      g_free (converted_text);
    } // end FOR

    data_p->logStack.clear ();
  } // end lock scope

  // step3: scroll the view accordingly
//  // move the iterator to the beginning of line, so it doesn't scroll
//  // in horizontal direction
//  gtk_text_iter_set_line_offset (&text_iterator, 0);

//  // ...and place the mark at iter. The mark will stay there after insertion
//  // because it has "right" gravity
//  GtkTextMark* text_mark_p =
//      gtk_text_buffer_get_mark (buffer_p,
//                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME));
////  gtk_text_buffer_move_mark (buffer_p,
////                             text_mark_p,
////                             &text_iterator);

//  // scroll the mark onscreen
//  gtk_text_view_scroll_mark_onscreen (view_p,
//                                      text_mark_p);
  GtkAdjustment* adjustment_p =
      GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME)));
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p));

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
      static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  { // synch access
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->lock, G_SOURCE_REMOVE);

    if (data_p->eventStack.empty ()) return G_SOURCE_CONTINUE;

    for (Stream_GTK_EventsIterator_t iterator_2 = data_p->eventStack.begin ();
         iterator_2 != data_p->eventStack.end ();
         iterator_2++)
    {
      switch (*iterator_2)
      {
        case STREAM_GTKEVENT_START:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p, 0.0);
          is_session_message = true;
          break;
        }
        case STREAM_GTKEVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case STREAM_GTKEVENT_END:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case STREAM_GTKEVENT_STATISTIC:
        {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.dataMessages));
#endif

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_p->progressData.statistic.droppedMessages));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          is_session_message = true;
          break;
        }
        case STREAM_GKTEVENT_INVALID:
        case STREAM_GTKEVENT_MAX:
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *iterator_2));
          break;
        }
      } // end SWITCH
      ACE_UNUSED_ARG (is_session_message);
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_FORWARD,
                            1.0);
    } // end FOR

    data_p->eventStack.clear ();
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  Test_U_AudioEffect_GTK_ProgressData* data_p =
      static_cast<Test_U_AudioEffect_GTK_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->GTKState);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->GTKState->lock, G_SOURCE_REMOVE);

  int result = -1;
  Common_UI_GTKBuildersIterator_t iterator =
    data_p->GTKState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->GTKState->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Test_U_AudioEffect_PendingActionsIterator_t iterator_2;
  for (Test_U_AudioEffect_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
       iterator_3 != data_p->completedActions.end ();
       ++iterator_3)
  {
    iterator_2 = data_p->pendingActions.find (*iterator_3);
    ACE_ASSERT (iterator_2 != data_p->pendingActions.end ());
    result = thread_manager_p->join ((*iterator_2).first, &exit_status);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::join(%u): \"%m\", continuing\n"),
                  (*iterator_2).first));
    else
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: %u)...\n"),
                  (*iterator_2).first,
                  exit_status));
#else
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("thread %u has joined (status was: %@)...\n"),
                  (*iterator_2).first,
                  exit_status));
#endif
    } // end ELSE

    data_p->GTKState->eventSourceIds.erase (*iterator_3);
    data_p->pendingActions.erase (iterator_2);
  } // end FOR
  data_p->completedActions.clear ();

  bool done = false;
  if (data_p->pendingActions.empty ())
  {
    //if (data_p->cursorType != GDK_LAST_CURSOR)
    //{
    //  GdkCursor* cursor_p = gdk_cursor_new (data_p->cursorType);
    //  if (!cursor_p)
    //  {
    //    ACE_DEBUG ((LM_ERROR,
    //                ACE_TEXT ("failed to gdk_cursor_new(%d): \"%m\", continuing\n"),
    //                data_p->cursorType));
    //    return G_SOURCE_REMOVE;
    //  } // end IF
    //  GtkWindow* window_p =
    //    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
    //                                        ACE_TEXT_ALWAYS_CHAR (IRC_CLIENT_GUI_GTK_WINDOW_MAIN)));
    //  ACE_ASSERT (window_p);
    //  GdkWindow* window_2 = gtk_widget_get_window (GTK_WIDGET (window_p));
    //  ACE_ASSERT (window_2);
    //  gdk_window_set_cursor (window_2, cursor_p);
    //  data_p->cursorType = GDK_LAST_CURSOR;
    //} // end IF

    done = true;
  } // end IF

  // synch access
  std::ostringstream converter;
  converter << data_p->statistic.messagesPerSecond;
  converter << ACE_TEXT_ALWAYS_CHAR (" fps");
  gtk_progress_bar_set_text (progress_bar_p,
                             (done ? ACE_TEXT_ALWAYS_CHAR ("")
                                   : converter.str ().c_str ()));
  gtk_progress_bar_pulse (progress_bar_p);

  // reschedule ?
  return (done ? G_SOURCE_REMOVE : G_SOURCE_CONTINUE);
}

gboolean
idle_audio_video_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_audio_video_display_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  gdk_window_invalidate_rect (gtk_widget_get_window (GTK_WIDGET (drawing_area_p)),
                              NULL,
                              false);

  return G_SOURCE_REMOVE;
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
void
textview_size_allocate_cb (GtkWidget* widget_in,
                           GdkRectangle* rectangle_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (rectangle_in);
  Test_U_AudioEffect_GTK_CBData* data_p =
      static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT(iterator != data_p->builders.end ());

  GtkScrolledWindow* scrolled_window_p =
    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SCROLLEDWINDOW_NAME)));
  ACE_ASSERT (scrolled_window_p);
  GtkAdjustment* adjustment_p =
    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
  ACE_ASSERT (adjustment_p);
  gtk_adjustment_set_value (adjustment_p,
                            gtk_adjustment_get_upper (adjustment_p) -
                            gtk_adjustment_get_page_size (adjustment_p));
} // textview_size_allocate_cb

void
toggleaction_record_toggled_cb (GtkToggleAction* toggleAction_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_record_toggled_cb"));

  // handle untoggle --> PLAY
  if (un_toggling_stream)
  {
    un_toggling_stream = false;
    return; // done
  } // end IF

  // --> user pressed play/pause/stop

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
    ACE_ASSERT (mediafoundation_data_p->stream);
    stream_p = mediafoundation_data_p->stream;
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
    ACE_ASSERT (directshow_data_p->stream);
    stream_p = directshow_data_p->stream;
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->stream);
  stream_p = data_p->stream;
#endif
  ACE_ASSERT (stream_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  // toggle ?
  GtkAction* action_p = NULL;
  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  if (!gtk_toggle_action_get_active (toggleAction_in))
  {
    // --> user pressed pause/stop

    //// step0: modify widgets
    //gtk_action_set_stock_id (GTK_ACTION (toggleAction_in),
    //                         GTK_STOCK_MEDIA_RECORD);

    //action_p =
    //  GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
    //                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME)));
    //ACE_ASSERT (action_p);
    //gtk_action_set_sensitive (action_p, false);
    //action_p =
    //  GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
    //                                      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME)));
    //ACE_ASSERT (action_p);
    //gtk_action_set_sensitive (action_p, false);

    //gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

    // step1: stop stream
    stream_p->stop (false, true);

    return;
  } // end IF

  // --> user pressed record

  Test_U_AudioEffect_ThreadData* thread_data_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_thread_t thread_id = std::numeric_limits<unsigned long>::max ();
#else
  ACE_thread_t thread_id = -1;
#endif
  ACE_hthread_t thread_handle = ACE_INVALID_HANDLE;
  const char* thread_name_2 = NULL;
  ACE_Thread_Manager* thread_manager_p = NULL;

  GtkSpinButton* spin_button_p = NULL;
  gint value_i = 0;

  if (data_p->isFirst)
    data_p->isFirst = false;

  // step0: modify widgets
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_STOP);

  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_CUT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);
  action_p =
    GTK_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ACTION_REPORT_NAME)));
  ACE_ASSERT (action_p);
  gtk_action_set_sensitive (action_p, true);

  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), false);

  // step1: set up progress reporting
  data_p->progressData.statistic = Test_U_RuntimeStatistic_t ();
  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);
  //gint width, height;
  //gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  //gtk_progress_bar_set_pulse_step (progress_bar_p,
  //                                 1.0 / static_cast<double> (width));
  gtk_progress_bar_set_fraction (progress_bar_p, 0.0);

  // step2: update configuration
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_2;
  if (gtk_combo_box_get_active_iter (combo_box_p,
                                     &iterator_2))
  {
    GtkListStore* list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
    ACE_ASSERT (list_store_p);
    GValue value = {0,};
    gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                              &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              0, &value);
#else
                              1, &value);
#endif
    ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (data_p->useMediaFoundation)
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.device =
        g_value_get_string (&value);
    else
      directshow_data_p->configuration->moduleHandlerConfiguration.device =
        g_value_get_string (&value);
#else
    data_p->configuration->moduleHandlerConfiguration.device =
      g_value_get_string (&value);
#endif
    g_value_unset (&value);
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (!Stream_Module_Device_Tools::setFormat (data_p->device,
                                              data_p->configuration->ALSAConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::setFormat(): \"%m\", returning\n")));
    return;
  } // end IF
  data_p->configuration->moduleHandlerConfiguration.captureDeviceHandle =
      data_p->device;
#endif

  GtkToggleAction* toggle_action_p =
    GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_SAVE_NAME)));
  ACE_ASSERT (toggle_action_p);
  bool save_to_file = gtk_toggle_action_get_active (toggle_action_p);
  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GFile* file_p =
    gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (file_p)
  {
    char* filename_p = g_file_get_path (file_p);
    if (!filename_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));

      // clean up
      g_object_unref (file_p);

      goto error;
    } // end IF
    g_object_unref (file_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (data_p->useMediaFoundation)
    {
      if (save_to_file)
        mediafoundation_data_p->configuration->moduleHandlerConfiguration.targetFileName =
          Common_UI_Tools::UTF82Locale (filename_p, -1);
      else
        mediafoundation_data_p->configuration->moduleHandlerConfiguration.targetFileName.clear ();
    } // end IF
    else
    {
      if (save_to_file)
        directshow_data_p->configuration->moduleHandlerConfiguration.targetFileName =
          Common_UI_Tools::UTF82Locale (filename_p, -1);
      else
        directshow_data_p->configuration->moduleHandlerConfiguration.targetFileName.clear ();
    } // end ELSE
#else
    if (save_to_file)
      data_p->configuration->moduleHandlerConfiguration.targetFileName =
        Common_UI_Tools::UTF82Locale (filename_p, -1);
    else
      data_p->configuration->moduleHandlerConfiguration.targetFileName.clear ();
#endif
    g_free (filename_p);
  } // end IF

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  value_i = gtk_spin_button_get_value_as_int (spin_button_p);
  if (value_i)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (data_p->useMediaFoundation)
    {
      mediafoundation_data_p->configuration->streamConfiguration.bufferSize =
        value_i;
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.bufferSize =
        value_i;
    } // end IF
    else
    {
      directshow_data_p->configuration->streamConfiguration.bufferSize =
        value_i;
      directshow_data_p->configuration->moduleHandlerConfiguration.bufferSize =
        value_i;
    } // end ELSE
#else
    data_p->configuration->streamConfiguration.bufferSize = value_i;
    data_p->configuration->moduleHandlerConfiguration.bufferSize = value_i;
#endif
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    if (data_p->useMediaFoundation)
      value_i =
        mediafoundation_data_p->configuration->streamConfiguration.bufferSize;
    else
      value_i =
        directshow_data_p->configuration->streamConfiguration.bufferSize;
#else
    value_i =
      data_p->configuration->streamConfiguration.bufferSize;
#endif
    gtk_spin_button_set_value (spin_button_p,
                               static_cast<gdouble> (value_i));
  } // end ELSE

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: reusing a media session doesn't work reliably at the moment
  //         --> recreate a new session on every run
  if (data_p->useMediaFoundation)
  {
    if (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session)
    {
      //HRESULT result = E_FAIL;
      // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
      //result =
      //  data_p->configuration->moduleHandlerConfiguration.session->Shutdown ();
      //if (FAILED (result))
      //  ACE_DEBUG ((LM_ERROR,
      //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
      //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.session->Release ();
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.session =
        NULL;
    } // end IF

    // set missing format properties
    UINT32 number_of_channels, bits_per_sample, sample_rate;
    HRESULT result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                           &sample_rate);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                           &bits_per_sample);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                           &number_of_channels);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetUINT32 (MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                                                                           (bits_per_sample * number_of_channels) / 8);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetUINT32 (MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                                                                           sample_rate * (bits_per_sample * number_of_channels / 8));
    ACE_ASSERT (SUCCEEDED (result));
  } // end IF
  else
  {

  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.captureDeviceHandle);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);

//  if (!Stream_Module_Device_Tools::setCaptureFormat (data_p->configuration->moduleHandlerConfiguration.deviceHandle,
//                                                     *data_p->configuration->moduleHandlerConfiguration.format))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Module_Device_Tools::setCaptureFormat(), aborting\n")));
//    goto error;
//  } // end IF
#endif
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  topology_p->Release ();
//#endif
  //struct _AMMediaType* media_type_p = NULL;
  //Stream_Module_Device_Tools::getCaptureFormat (data_p->configuration->moduleHandlerConfiguration.builder,
  //                                              media_type_p);
  //media_type.Set (*media_type_p);
  //ACE_ASSERT (media_type == *data_p->configuration->moduleHandlerConfiguration.format);

  // step3: start processing thread
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    Test_U_AudioEffect_MediaFoundation_ThreadData* thread_data_2 = NULL;
    ACE_NEW_NORETURN (thread_data_2,
                      Test_U_AudioEffect_MediaFoundation_ThreadData ());
    if (thread_data_2)
    {
      thread_data_2->CBData = mediafoundation_data_p;
      thread_data_p = thread_data_2;
    } // end IF
  } // end IF
  else
  {
    Test_U_AudioEffect_DirectShow_ThreadData* thread_data_2 = NULL;
    ACE_NEW_NORETURN (thread_data_2,
                      Test_U_AudioEffect_DirectShow_ThreadData ());
    if (thread_data_2)
    {
      thread_data_2->CBData = directshow_data_p;
      thread_data_p = thread_data_2;
    } // end IF
  } // end ELSE
#else
  ACE_NEW_NORETURN (thread_data_p,
                    Test_U_AudioEffect_ThreadData ());
  if (thread_data_p)
    thread_data_p->CBData = data_p;
#endif
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    goto error;
  } // end IF
  ACE_TCHAR thread_name[BUFSIZ];
  ACE_OS::memset (thread_name, 0, sizeof (thread_name));
//  char* thread_name_p = NULL;
//  ACE_NEW_NORETURN (thread_name_p,
//                    ACE_TCHAR[BUFSIZ]);
//  if (!thread_name_p)
//  {
//    ACE_DEBUG ((LM_CRITICAL,
//                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));

//    // clean up
//    delete thread_data_p;

//    return;
//  } // end IF
//  ACE_OS::memset (thread_name_p, 0, sizeof (thread_name_p));
//  ACE_OS::strcpy (thread_name_p,
//                  ACE_TEXT (TEST_U_Test_U_AudioEffect_THREAD_NAME));
//  const char* thread_name_2 = thread_name_p;
  ACE_OS::strcpy (thread_name,
                  ACE_TEXT (TEST_U_STREAM_AUDIOEFFECT_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  {
    ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->lock);
    int result =
      thread_manager_p->spawn (::stream_processing_function,     // function
                               thread_data_p,                    // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),              // flags
                               &thread_id,                       // thread id
                               &thread_handle,                   // thread handle
                               ACE_DEFAULT_THREAD_PRIORITY,      // priority
                               COMMON_EVENT_THREAD_GROUP_ID + 2, // *TODO*: group id
                               NULL,                             // stack
                               0,                                // stack size
                               &thread_name_2);                  // name
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));

      // clean up
//    delete thread_name_p;
      delete thread_data_p;

      goto error;
    } // end IF

    // step3: start progress reporting
    //ACE_ASSERT (!data_p->progressEventSourceID);
    data_p->progressEventSourceID =
      //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
      //                 idle_update_progress_cb,
      //                 &data_p->progressData,
      //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,                          // _LOW doesn't work (on Win32)
                          TEST_U_STREAM_UI_GTK_PROGRESSBAR_UPDATE_INTERVAL, // ms (?)
                          idle_update_progress_cb,
                          &data_p->progressData,
                          NULL);
    if (!data_p->progressEventSourceID)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", returning\n")));

      // clean up
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(): \"%m\", continuing\n")));

      goto error;
    } // end IF
    thread_data_p->eventSourceID = data_p->progressEventSourceID;
    data_p->progressData.pendingActions[data_p->progressEventSourceID] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    data_p->eventSourceIds.insert (data_p->progressEventSourceID);
  } // end lock scope

  return;

error:
  gtk_action_set_stock_id (GTK_ACTION (toggleAction_in), GTK_STOCK_MEDIA_RECORD);
  gtk_action_set_sensitive (action_p, false);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);
} // toggleaction_record_toggled_cb
void
toggleaction_save_toggled_cb (GtkToggleAction* toggleAction_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::toggleaction_save_toggled_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  GError* error_p = NULL;
  GFile* file_p = NULL;
  if (!gtk_toggle_action_get_active (toggleAction_in))
  {
    file_p =
      g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
    ACE_ASSERT (file_p);
    if (!gtk_file_chooser_set_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                   file_p,
                                                   &error_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_file(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ()),
                  ACE_TEXT (error_p->message)));

      // clean up
      g_error_free (error_p);
      g_object_unref (file_p);

      return;
    } // end IF
    g_object_unref (file_p);
  } // end ELSE
} // toggleaction_save_toggled_cb

void
action_cut_activate_cb (GtkAction* action_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_cut_activate_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Test_U_AudioEffect_IStreamControl_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->stream);
    stream_p = mediafoundation_data_p->stream;
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->stream);
    stream_p = directshow_data_p->stream;
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->stream);
  stream_p = data_p->stream;
#endif
  ACE_ASSERT (stream_p);

  stream_p->control (STREAM_CONTROL_STEP,
                     false);
} // action_cut_activate_cb
void
action_report_activate_cb (GtkAction* action_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_report_activate_cb"));

  ACE_UNUSED_ARG (action_in);

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Test_U_AudioEffect_IStreamControl_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->stream);
    stream_p = mediafoundation_data_p->stream;
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->stream);
    stream_p = directshow_data_p->stream;
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->stream);
  stream_p = data_p->stream;
#endif
  ACE_ASSERT (stream_p);
} // action_report_activate_cb

// -----------------------------------------------------------------------------

gint
button_clear_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTextView* view_p =
    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
  ACE_ASSERT (view_p);
  GtkTextBuffer* buffer_p =
//    gtk_text_buffer_new (NULL); // text tag table --> create new
    gtk_text_view_get_buffer (view_p);
  ACE_ASSERT (buffer_p);
  gtk_text_buffer_set_text (buffer_p,
                            ACE_TEXT_ALWAYS_CHAR (""), 0);

  return FALSE;
}

gint
button_about_clicked_cb (GtkWidget* widget_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to glade_xml_get_widget(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
    return TRUE; // propagate
  } // end IF

  // run dialog
  gint result = gtk_dialog_run (about_dialog);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (about_dialog));

  return FALSE;
} // button_about_clicked_cb

gint
button_quit_clicked_cb (GtkWidget* widget_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (widget_in);
  Test_U_AudioEffect_GTK_CBData* data_p = static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);

  Test_U_AudioEffect_IStreamControl_t* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->stream);
    stream_p = mediafoundation_data_p->stream;
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->stream);
    stream_p = directshow_data_p->stream;
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->stream);
  stream_p = data_p->stream;
#endif
  ACE_ASSERT (stream_p);

  //// step1: remove event sources
  //{
  //  ACE_Guard<ACE_Thread_Mutex> aGuard (data_p->lock);

  //  for (Common_UI_GTKEventSourceIdsIterator_t iterator = data_p->eventSourceIds.begin ();
  //       iterator != data_p->eventSourceIds.end ();
  //       iterator++)
  //    if (!g_source_remove (*iterator))
  //      ACE_DEBUG ((LM_ERROR,
  //                  ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
  //                  *iterator));
  //  data_p->eventSourceIds.clear ();
  //} // end lock scope

  const Stream_StateMachine_ControlState& status_r =
    stream_p->status ();
  if ((status_r == STREAM_STATE_RUNNING) ||
      (status_r == STREAM_STATE_PAUSED))
    stream_p->stop (false, true);

  // step2: initiate shutdown sequence
  int result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));

  return FALSE;
} // button_quit_clicked_cb

void
combobox_source_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_source_changed_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->stream);
    stream_p = mediafoundation_data_p->stream;
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->stream);
    stream_p = directshow_data_p->stream;
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->stream);
  stream_p = data_p->stream;
#endif
  ACE_ASSERT (stream_p);

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  GValue value_2 = {0,};
#endif
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string device_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  std::string device_name = g_value_get_string (&value_2);
  g_value_unset (&value_2);
#endif

  gint n_rows = 0;
  GtkToggleAction* toggle_action_p = NULL;

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);

//  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  WCHAR* symbolic_link_p = NULL;
  UINT32 symbolic_link_size = 0;
  IMFMediaSource* media_source_p = NULL;
  if (data_p->useMediaFoundation)
  {
    //if (mediafoundation_data_p->configuration->moduleHandlerConfiguration.sourceReader)
    //{
    //  mediafoundation_data_p->configuration->moduleHandlerConfiguration.sourceReader->Release ();
    //  mediafoundation_data_p->configuration->moduleHandlerConfiguration.sourceReader =
    //    NULL;
    //} // end IF
    //if (mediafoundation_data_p->configuration->moduleHandlerConfiguration.mediaSource)
    //{
    //  mediafoundation_data_p->configuration->moduleHandlerConfiguration.mediaSource->Release ();
    //  mediafoundation_data_p->configuration->moduleHandlerConfiguration.mediaSource =
    //    NULL;
    //} // end IF
    if (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session)
    {
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.session->Release ();
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.session = NULL;
    } // end IF

    if (!Stream_Module_Device_Tools::getMediaSource (device_string,
                                                     MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                     media_source_p,
                                                     symbolic_link_p,
                                                     symbolic_link_size))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(\"%s\"), returning\n"),
                  ACE_TEXT (device_string.c_str ())));
      return;
    } // end IF
    ACE_ASSERT (media_source_p);
    ACE_ASSERT (symbolic_link_p);
    ACE_ASSERT (symbolic_link_size);
    CoTaskMemFree (symbolic_link_p);
  } // end IF
#endif

  std::string module_name =
    ACE_TEXT_ALWAYS_CHAR (MODULE_DEV_MIC_SOURCE_MODULE_NAME);
  Stream_Module_t* module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
    module_p =
      const_cast<Stream_Module_t*> (mediafoundation_data_p->stream->find (module_name));
  else
    module_p =
      const_cast<Stream_Module_t*> (directshow_data_p->stream->find (module_name));
#else
  module_p =
    const_cast<Stream_Module_t*> (data_p->stream->find (module_name));
#endif
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Base_T::find(\"%s\"), returning\n"),
                ACE_TEXT (module_name.c_str ())));
    return;
  } // end IF

  bool result_2 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_Dev_Mic_Source_MediaFoundation* mediafoundation_source_impl_p = NULL;
  Test_U_Dev_Mic_Source_DirectShow* directshow_source_impl_p = NULL;
  IMFTopology* topology_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_source_impl_p =
      dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation*> (module_p->writer ());
    ACE_ASSERT (mediafoundation_source_impl_p);

    struct _MFRatio pixel_aspect_ratio = { 1, 1 };
    if (!Stream_Module_Device_Tools::loadDeviceTopology (device_string,
                                                         MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                         media_source_p,
                                                         mediafoundation_source_impl_p,
                                                         topology_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceTopology(), aborting\n")));
      goto error;
    } // end IF
    ACE_ASSERT (topology_p);

    // sanity check(s)
    ACE_ASSERT (!mediafoundation_data_p->configuration->moduleHandlerConfiguration.session);
    if (!Stream_Module_Device_Tools::setTopology (topology_p,
                                                  mediafoundation_data_p->configuration->moduleHandlerConfiguration.session,
                                                  true))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::setTopology(), aborting\n")));
      goto error;
    } // end IF
    topology_p->Release ();
    topology_p = NULL;

    if (mediafoundation_data_p->configuration->moduleHandlerConfiguration.format)
    {
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->Release ();
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format =
        NULL;
    } // end IF
    HRESULT result =
      MFCreateMediaType (&mediafoundation_data_p->configuration->moduleHandlerConfiguration.format);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      goto error;
    } // end IF
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.format);
    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetGUID (MF_MT_MAJOR_TYPE,
                                                                                         MFMediaType_Audio);
    ACE_ASSERT (SUCCEEDED (result));
    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT,
                                                                                           TRUE);
    ACE_ASSERT (SUCCEEDED (result));

    //if (!load_formats (data_p->configuration->moduleHandlerConfiguration.sourceReader,
    result_2 = load_formats (media_source_p,
                             list_store_p);
  } // end IF
  else
  {
    if (directshow_data_p->streamConfiguration)
    {
      directshow_data_p->streamConfiguration->Release ();
      directshow_data_p->streamConfiguration = NULL;
    } // end IF
    if (directshow_data_p->configuration->moduleHandlerConfiguration.builder)
    {
      directshow_data_p->configuration->moduleHandlerConfiguration.builder->Release ();
      directshow_data_p->configuration->moduleHandlerConfiguration.builder =
        NULL;
    } // end IF

    IAMBufferNegotiation* buffer_negotiation_p = NULL;
    if (!Stream_Module_Device_Tools::loadDeviceGraph (device_string,
                                                      CLSID_AudioInputDeviceCategory,
                                                      directshow_data_p->configuration->moduleHandlerConfiguration.builder,
                                                      buffer_negotiation_p,
                                                      directshow_data_p->streamConfiguration))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                  ACE_TEXT (device_string.c_str ())));
      return;
    } // end IF
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.builder);
    ACE_ASSERT (buffer_negotiation_p);
    ACE_ASSERT (directshow_data_p->streamConfiguration);

    buffer_negotiation_p->Release ();

    result_2 =
      load_formats (directshow_data_p->streamConfiguration,
                    list_store_p);
  } // end ELSE
#else
  int result = -1;
  struct _snd_pcm_hw_params* format_p = NULL;

  if (data_p->device)
  {
    result = snd_pcm_close (data_p->device);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed ALSA device...\n")));
    ACE_ASSERT (data_p->device == data_p->configuration->moduleHandlerConfiguration.captureDeviceHandle);
    data_p->device = NULL;
    data_p->configuration->moduleHandlerConfiguration.captureDeviceHandle = NULL;
  } // end IF
  ACE_ASSERT (!data_p->device);
//  int mode = MODULE_DEV_MIC_ALSA_DEFAULT_MODE;
  int mode = 0;
  //    snd_spcm_init();
  result = snd_pcm_open (&data_p->device,
                         device_name.c_str (),
                         SND_PCM_STREAM_CAPTURE, mode);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_open(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (device_name.c_str ()),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  data_p->configuration->moduleHandlerConfiguration.captureDeviceHandle =
      data_p->device;
  ACE_ASSERT (data_p->device);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened ALSA device (capture) \"%s\"...\n"),
              ACE_TEXT (device_name.c_str ())));

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_any (data_p->device,
                                  format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params (data_p->device,
                              format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
#if defined (_DEBUG)
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: default format:\n%s"),
              ACE_TEXT (snd_pcm_name (data_p->device)),
              ACE_TEXT (Stream_Module_Device_Tools::formatToString (format_p).c_str ())));
#endif
  snd_pcm_hw_params_free (format_p);

  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);
  if (!Stream_Module_Device_Tools::getFormat (data_p->device,
                                              *data_p->configuration->moduleHandlerConfiguration.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Device_Tools::getFormat(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  data_p->configuration->moduleHandlerConfiguration.format->access =
      MODULE_DEV_MIC_ALSA_DEFAULT_ACCESS;

  result_2 =
      load_formats (data_p->device,
                    *data_p->configuration->moduleHandlerConfiguration.format,
                    list_store_p);
#endif
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_formats(), returning\n")));
    goto error;
  } // end IF
  n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

  toggle_action_p =
      GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  ACE_ASSERT (toggle_action_p);
  gtk_action_set_sensitive (GTK_ACTION (toggle_action_p), true);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
    media_source_p->Release ();
#endif

  return;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    if (media_source_p)
      media_source_p->Release ();
    if (topology_p)
      topology_p->Release ();
  } // end IF
#else
  if (data_p->device)
  {
    result = snd_pcm_close (data_p->device);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed ALSA device...\n")));
    data_p->device = NULL;
  } // end IF

  if (format_p)
    snd_pcm_hw_params_free (format_p);
#endif

  return;
} // combobox_source_changed_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
#endif
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result = CLSIDFromString (format_string.c_str (),
                            &GUID_i);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (format_string.c_str ()),
                     &GUID_s);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IMFMediaSource* media_source_p = NULL;
  if (data_p->useMediaFoundation)
  {
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session);

    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetGUID (MF_MT_SUBTYPE,
                                                                                         GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::SetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF

    if (!Stream_Module_Device_Tools::getMediaSource (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session,
                                                     media_source_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), returning\n")));
      return;
    } // end IF

    //if (!load_sample_rates (data_p->configuration->moduleHandlerConfiguration.sourceReader,
    result_2 = load_sample_rates (media_source_p,
                                  GUID_s,
                                  list_store_p);
  } // end IF
  else
  {
    // sanity check(s)
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (directshow_data_p->streamConfiguration);

    directshow_data_p->configuration->moduleHandlerConfiguration.format->subtype =
      GUID_s;

    result_2 =
      load_sample_rates (directshow_data_p->streamConfiguration,
                         GUID_s,
                         list_store_p);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);

  data_p->configuration->moduleHandlerConfiguration.format->format = format_e;
  // *TODO*: format setting doesn't work yet
  data_p->configuration->moduleHandlerConfiguration.format->format =
      MODULE_DEV_MIC_ALSA_DEFAULT_FORMAT;

  result_2 =
      load_sample_rates (data_p->device,
                         *data_p->configuration->moduleHandlerConfiguration.format,
                         list_store_p);
#endif
  if (!result_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_rates(\"%s\"), aborting\n"),
                Stream_Module_Device_Tools::mediaSubTypeToString (GUID_s).c_str ()));
    goto error_2;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_rates(), returning\n")));
    return;
#endif
  } // end IF
  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    media_source_p->Release ();
    media_source_p = NULL;
  } // end IF

  return;

error_2:
  if (data_p->useMediaFoundation)
  {
    if (media_source_p)
      media_source_p->Release ();
  } // end IF
#endif
} // combobox_format_changed_cb
void
combobox_frequency_changed_cb (GtkWidget* widget_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_frequency_changed_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTreeIter iterator_2;
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
  ACE_UNUSED_ARG (format_e);
#endif
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (format_string.c_str (),
                     &GUID_s);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (format_string.c_str ()),
                     &GUID_s);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int sample_rate = g_value_get_uint (&value);
  g_value_unset (&value);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IMFMediaSource* media_source_p = NULL;
  if (data_p->useMediaFoundation)
  {
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session);

    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                           sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,%u): \"%s\", returning\n"),
                  sample_rate,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF

    if (!Stream_Module_Device_Tools::getMediaSource (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session,
                                                     media_source_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), returning\n")));
      return;
    } // end IF
  
    //if (!load_sample_resolutions (data_p->configuration->moduleHandlerConfiguration.sourceReader,
    result_2 = load_sample_resolutions (media_source_p,
                                        GUID_s,
                                        sample_rate,
                                        list_store_p);
  } // end IF
  else
  {
    // sanity check(s)
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.format->formattype == FORMAT_WaveFormatEx);
    ACE_ASSERT (directshow_data_p->streamConfiguration);

    struct tWAVEFORMATEX* audio_info_header_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (directshow_data_p->configuration->moduleHandlerConfiguration.format->pbFormat);
    audio_info_header_p->nSamplesPerSec = sample_rate;

    result_2 = load_sample_resolutions (directshow_data_p->streamConfiguration,
                                        GUID_s,
                                        sample_rate,
                                        list_store_p);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);

  data_p->configuration->moduleHandlerConfiguration.format->rate = sample_rate;

  result_2 =
      load_sample_resolutions (data_p->device,
                               *data_p->configuration->moduleHandlerConfiguration.format,
                               list_store_p);
#endif
  if (!result_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_resolutions(\"%s\"), aborting\n"),
                Stream_Module_Device_Tools::mediaSubTypeToString (GUID_s).c_str ()));
    goto error_2;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_resolutions(%d), returning\n"),
                data_p->device));
    return;
#endif
  } // end IF

  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    media_source_p->Release ();
    media_source_p = NULL;
  } // end IF
#endif

  return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
error_2:
  if (data_p->useMediaFoundation)
  {
    if (media_source_p)
      media_source_p->Release ();
  } // end IF
#endif
} // combobox_frequency_changed_cb
void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTreeIter iterator_2;
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
  ACE_UNUSED_ARG (format_e);
#endif
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (format_string.c_str (),
                     &GUID_s);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (format_string.c_str ()),
                     &GUID_s);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int sample_rate = g_value_get_uint (&value);
  ACE_UNUSED_ARG (sample_rate);
  g_value_unset (&value);

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int bits_per_sample = g_value_get_uint (&value);
  ACE_UNUSED_ARG (bits_per_sample);
  g_value_unset (&value);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  IMFMediaSource* media_source_p = NULL;
  if (data_p->useMediaFoundation)
  {
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session);

    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                           bits_per_sample);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,%d): \"%s\", returning\n"),
                  bits_per_sample,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF

    if (!Stream_Module_Device_Tools::getMediaSource (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session,
                                                     media_source_p))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to Stream_Module_Device_Tools::getMediaSource(), returning\n")));
      return;
    } // end IF

    //if (!load_rates (data_p->configuration->moduleHandlerConfiguration.sourceReader,
    result_2 = load_channels (media_source_p,
                              GUID_s,
                              sample_rate,
                              bits_per_sample,
                              list_store_p);
  } // end IF
  else
  {
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (directshow_data_p->configuration->moduleHandlerConfiguration.format->formattype == FORMAT_WaveFormatEx);
    ACE_ASSERT (directshow_data_p->streamConfiguration);

    struct tWAVEFORMATEX* audio_info_header_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (directshow_data_p->configuration->moduleHandlerConfiguration.format->pbFormat);
    audio_info_header_p->wBitsPerSample = bits_per_sample;

    result_2 = load_channels (directshow_data_p->streamConfiguration,
                              GUID_s,
                              sample_rate,
                              bits_per_sample,
                              list_store_p);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);

//  data_p->configuration->moduleHandlerConfiguration.format->format =
//      bits_per_sample;

  result_2 =
      load_channels (data_p->device,
                     *data_p->configuration->moduleHandlerConfiguration.format,
                     list_store_p);
#endif
  if (!result_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_channels(\"%s\"), aborting\n"),
                Stream_Module_Device_Tools::mediaSubTypeToString (GUID_s).c_str ()));
    goto error_2;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_channels(%d), returning\n"),
                data_p->device));
    return;
#endif
  } // end IF

  gint n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), true);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    media_source_p->Release ();
    media_source_p = NULL;
  } // end IF
#endif

  return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
error_2:
  if (data_p->useMediaFoundation)
    if (media_source_p)
      media_source_p->Release ();
#endif
} // combobox_resolution_changed_cb
void
combobox_channels_changed_cb (GtkWidget* widget_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_channels_changed_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  GtkTreeIter iterator_2;
  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  GValue value = {0,};
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
  ACE_UNUSED_ARG (format_e);
#endif
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
  HRESULT result = E_FAIL;
#if defined (OLE2ANSI)
  result =
    CLSIDFromString (format_string.c_str (),
                     &GUID_s);
#else
  result =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (format_string.c_str ()),
                     &GUID_s);
#endif
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(): \"%s\", returning\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int frequency = g_value_get_uint (&value);
  ACE_UNUSED_ARG (frequency);
  g_value_unset (&value);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int bits_per_sample = g_value_get_uint (&value);
  ACE_UNUSED_ARG (bits_per_sample);
  g_value_unset (&value);

  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
  {
    //ACE_DEBUG ((LM_ERROR,
    //            ACE_TEXT ("failed to gtk_combo_box_get_active_iter(), returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
  unsigned int number_of_channels = g_value_get_uint (&value);
  ACE_UNUSED_ARG (number_of_channels);
  g_value_unset (&value);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
  {
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.format);
    ACE_ASSERT (mediafoundation_data_p->configuration->moduleHandlerConfiguration.session);

    result =
      mediafoundation_data_p->configuration->moduleHandlerConfiguration.format->SetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                           number_of_channels);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,%d): \"%s\", returning\n"),
                  number_of_channels,
                  ACE_TEXT (Common_Tools::error2String (result).c_str ())));
      return;
    } // end IF
  } // end IF
  else
  {
    // sanity check(s)
    ACE_ASSERT (directshow_data_p->streamConfiguration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->device);
  ACE_ASSERT (data_p->configuration);
  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.format);

  data_p->configuration->moduleHandlerConfiguration.format->channels =
      number_of_channels;
#endif

  update_buffer_size (userData_in);
} // combobox_channels_changed_cb

gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_draw_cb"));

  //ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (context_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  //ACE_ASSERT (context_in);
  ACE_ASSERT (userData_in);

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  if (!data_p->cairoSurface)
    return FALSE; // --> widget has not been realized yet

  //GdkWindow* window_p = gtk_widget_get_window (widget_in);
  //ACE_ASSERT (window_p);
  //cairo_t* context_p = gdk_cairo_create (GDK_DRAWABLE (window_p));
  //if (!context_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
  //  return FALSE;
  //} // end IF
  //gdk_cairo_set_source_pixbuf (context_p,
  //gdk_cairo_set_source_pixbuf (context_in,
  //                             data_p->pixelBuffer,
  //                             0.0, 0.0);
  cairo_set_source_surface (context_in,
                            data_p->cairoSurface,
                            0.0, 0.0);

  {
    //ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->cairoSurfaceLock, FALSE);

    cairo_paint (context_in);
  } // end lock scope

  // clean up
  //cairo_destroy (context_p);

  return TRUE;
}

//void
//drawingarea_configure_event_cb (GtkWindow* window_in,
//                                GdkEvent* event_in,
//                                gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

//  Test_U_AudioEffect_GTK_CBData* data_p =
//    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  ACE_ASSERT (data_p->configuration);

//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  if (!data_p->configuration->moduleHandlerConfiguration.window          ||
//      !data_p->configuration->moduleHandlerConfiguration.windowController) // <-- window not realized yet ?
//    return;
//#else
//  if (!data_p->configuration->moduleHandlerConfiguration.window) // <-- window not realized yet ?
//    return;
//#endif

//  Common_UI_GTKBuildersIterator_t iterator =
//    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT (iterator != data_p->builders.end ());

//  GtkDrawingArea* drawing_area_p =
//    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
//  ACE_ASSERT (drawing_area_p);
//  GtkAllocation allocation;
//  ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
//  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                             &allocation);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  // sanity check(s)
//  ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration.windowController);

//  data_p->configuration->moduleHandlerConfiguration.area.bottom =
//    allocation.height;
//  data_p->configuration->moduleHandlerConfiguration.area.left =
//    allocation.x;
//  data_p->configuration->moduleHandlerConfiguration.area.right =
//    allocation.width;
//  data_p->configuration->moduleHandlerConfiguration.area.top =
//    allocation.y;

//  //HRESULT result =
//  //  data_p->configuration->moduleHandlerConfiguration.windowController->SetWindowPosition (data_p->configuration->moduleHandlerConfiguration.area.left,
//  //                                                                                                               data_p->configuration->moduleHandlerConfiguration.area.top,
//  //                                                                                                               data_p->configuration->moduleHandlerConfiguration.area.right,
//  //                                                                                                               data_p->configuration->moduleHandlerConfiguration.area.bottom);
//  //if (FAILED (result))
//  //  ACE_DEBUG ((LM_ERROR,
//  //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
//  //              data_p->configuration->moduleHandlerConfiguration.area.left, data_p->configuration->moduleHandlerConfiguration.area.top,
//  //              data_p->configuration->moduleHandlerConfiguration.area.right, data_p->configuration->moduleHandlerConfiguration.area.bottom,
//  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
//#else
//  data_p->configuration->moduleHandlerConfiguration.area =
//    allocation;
//#endif
//} // drawingarea_configure_event_cb
void
drawingarea_size_allocate_cb (GtkWidget* widget_in,
                              GdkRectangle* allocation_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (allocation_in);
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  //if (!data_p->configuration->moduleHandlerConfiguration.gdkWindow) // <-- window not realized yet ?
  //  return;

  //Common_UI_GTKBuildersIterator_t iterator =
  //  data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  //// sanity check(s)
  //ACE_ASSERT (iterator != data_p->builders.end ());

  //GtkAllocation allocation;
  //ACE_OS::memset (&allocation, 0, sizeof (GtkAllocation));
  //gtk_widget_get_allocation (widget_in,
  //                           &allocation);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (data_p->useMediaFoundation)
    mediafoundation_data_p->configuration->moduleHandlerConfiguration.area =
      *allocation_in;
  else
    directshow_data_p->configuration->moduleHandlerConfiguration.area =
      *allocation_in;

  //// sanity check(s)
  //ACE_ASSERT (data_p->configuration->moduleHandlerConfiguration->windowController);

  //data_p->configuration->moduleHandlerConfiguration.area.bottom =
  //  allocation_in->height;
  //data_p->configuration->moduleHandlerConfiguration.area.left =
  //  allocation_in->x;
  //data_p->configuration->moduleHandlerConfiguration.area.right =
  //  allocation_in->width;
  //data_p->configuration->moduleHandlerConfiguration.area.top =
  //  allocation_in->y;

  //HRESULT result =
  //  data_p->configuration.moduleHandlerConfiguration->windowController->SetWindowPosition (data_p->configuration->moduleHandlerConfiguration.area.left,
  //                                                                                         data_p->configuration->moduleHandlerConfiguration.area.top,
  //                                                                                         data_p->configuration->moduleHandlerConfiguration.area.right,
  //                                                                                         data_p->configuration->moduleHandlerConfiguration.area.bottom);
  //if (FAILED (result))
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to IVideoWindow::SetWindowPosition(%d,%d,%d,%d): \"%s\", continuing\n"),
  //              data_p->configuration->moduleHandlerConfiguration.area.left, data_p->configuration->moduleHandlerConfiguration.area.top,
  //              data_p->configuration->moduleHandlerConfiguration.area.right, data_p->configuration->moduleHandlerConfiguration.area.bottom,
  //              ACE_TEXT (Common_Tools::error2String (result).c_str ())));
#else
  data_p->configuration->moduleHandlerConfiguration.area = *allocation_in;
#endif

} // drawingarea_size_allocate_cb

void
filechooserbutton_cb (GtkFileChooserButton* button_in,
                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_cb"));

  Test_U_AudioEffect_GTK_CBData* data_p =
    static_cast<Test_U_AudioEffect_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_DirectShow_GTK_CBData* directshow_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_GTK_CBData* mediafoundation_data_p = NULL;
  if (data_p->useMediaFoundation)
  {
    mediafoundation_data_p =
      static_cast<Test_U_AudioEffect_MediaFoundation_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (mediafoundation_data_p);
    ACE_ASSERT (mediafoundation_data_p->configuration);
  } // end IF
  else
  {
    directshow_data_p =
      static_cast<Test_U_AudioEffect_DirectShow_GTK_CBData*> (userData_in);
    // sanity check(s)
    ACE_ASSERT (directshow_data_p);
    ACE_ASSERT (directshow_data_p->configuration);
  } // end ELSE
#else
  // sanity check(s)
  ACE_ASSERT (data_p->configuration);
#endif

  Common_UI_GTKBuildersIterator_t iterator =
    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_GTK_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != data_p->builders.end ());

  //// step1: display chooser dialog
  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_FILECHOOSER_OPEN_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);

  //// run dialog
  //GFile* file_p = NULL;
  //gint result = gtk_dialog_run (GTK_DIALOG (file_chooser_dialog_p));
  //switch (result)
  //{
  //  case GTK_RESPONSE_OK:
  //    file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_dialog_p));
  //    if (!file_p) return FALSE; // ? *TODO*
  //    break;
  //  case GTK_RESPONSE_DELETE_EVENT: // ESC
  //  case GTK_RESPONSE_CANCEL:
  //  default:
  //    //gtk_widget_hide (GTK_WIDGET (file_chooser_dialog_p));
  //    return FALSE;
  //} // end SWITCH
  //ACE_ASSERT (file_p);
  //gtk_widget_hide (GTK_WIDGET (file_chooser_dialog_p));
  //GtkEntry* entry_p =
  //  GTK_ENTRY (gtk_builder_get_object ((*iterator).second.second,
  //  ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ENTRY_SOURCE_NAME)));
  //ACE_ASSERT (entry_p);

  //GFile* file_p = gtk_file_chooser_get_file (GTK_FILE_CHOOSER (button_in));
  //ACE_ASSERT (file_p);
  //char* string_p = g_file_get_path (file_p);
  //if (!string_p)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_file_get_path(%@): \"%m\", returning\n"),
  //              file_p));

  //  // clean up
  //  g_object_unref (file_p);

  //  return;
  //} // end IF
  //g_object_unref (file_p);
  //gtk_entry_set_text (entry_p, string_p);

  // record button
  //GtkToggleAction* toggle_action_p =
  //  GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEACTION_RECORD_NAME)));
  //ACE_ASSERT (toggle_action_p);
  //gtk_action_set_sensitive (GTK_ACTION (toggle_action_p),
  //                          !data_p->configuration->moduleHandlerConfiguration.targetFileName.empty ());
} // filechooserbutton_cb

void
filechooserdialog_cb (GtkFileChooser* chooser_in,
                      gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserdialog_cb"));

  ACE_UNUSED_ARG (userData_in);

  gtk_dialog_response (GTK_DIALOG (GTK_FILE_CHOOSER_DIALOG (chooser_in)),
                       GTK_RESPONSE_ACCEPT);
} // filechooserdialog_cb

#ifdef __cplusplus
}
#endif /* __cplusplus */