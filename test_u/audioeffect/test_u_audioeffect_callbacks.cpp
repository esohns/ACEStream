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

#include <limits>
#include <map>
#include <set>
#include <sstream>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <Dmo.h>
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <OleAuto.h>
// *NOTE*: uuids.h doesn't have double include protection
#if defined (UUIDS_H)
#else
#define UUIDS_H
#include <uuids.h>
#endif // UUIDS_H
#else
#include "alsa/asoundlib.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "gdk/gdk.h"
#include "gtk/gtk.h"
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#include "gtkgl/gtkglarea.h"
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#include "gtkgl/gtkglarea.h"
#else
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/Dirent_Selector.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "ace/Guard_T.h"
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"

#include "common_file_tools.h"
#include "common_process_tools.h"

#if defined (GTKGL_SUPPORT)
//#include "common_gl_defines.h"
//#include "common_gl_tools.h"
#endif /* GTKGL_SUPPORT */

#include "common_image_tools.h"

#include "common_timer_manager_common.h"

#include "common_ui_gtk_common.h"
#include "common_ui_gtk_defines.h"
#include "common_ui_gtk_tools.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_dev_defines.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#include "stream_dev_mediafoundation_tools.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "stream_dev_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_lib_directshow_tools.h"
#include "stream_lib_mediafoundation_tools.h"
#include "stream_lib_tools.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "test_u_audioeffect_common.h"
#include "test_u_audioeffect_common_modules.h"
#include "test_u_audioeffect_defines.h"
#include "test_u_audioeffect_gl_callbacks.h"
#include "test_u_audioeffect_stream.h"

// global variables
bool un_toggling_stream = false;

bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_capture_devices (GtkListStore* listStore_in,
                      enum Stream_MediaFramework_Type mediaFramework_in)
#else
load_capture_devices (GtkListStore* listStore_in)
#endif // ACE_WIN32 || ACE_WIN64
{
  STREAM_TRACE (ACE_TEXT ("::load_capture_devices"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
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
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        //result = VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        goto error;
      } // end IF
      ACE_ASSERT (enum_moniker_p);
      enumerator_p->Release (); enumerator_p = NULL;

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
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error;
        } // end IF
        moniker_p->Release (); moniker_p = NULL;
        ACE_ASSERT (properties_p);

        VariantInit (&variant);
        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING,
                              &variant,
                              0);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", aborting\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_NAME_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));
          goto error;
        } // end IF
        friendly_name_string =
           ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (variant.bstrVal));
        VariantClear (&variant);
        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING,
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
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_DESCRIPTION_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));

        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING,
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
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_PATH_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));

        result_2 =
          properties_p->Read (STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING,
                              &variant,
                              0);
        if (SUCCEEDED (result_2))
        {
          wave_in_id = variant.lVal;
          VariantClear (&variant);
        } // end IF
        else
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IPropertyBag::Read(%s): \"%s\", continuing\n"),
                      ACE_TEXT_WCHAR_TO_TCHAR (STREAM_LIB_DIRECTSHOW_PROPERTIES_ID_STRING),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2, true).c_str ())));

        properties_p->Release (); properties_p = NULL;

        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("found device \"%s\": \"%s\" @ %s (id: %d)\n"),
                    ACE_TEXT (friendly_name_string.c_str ()),
                    ACE_TEXT (description_string.c_str ()),
                    ACE_TEXT (device_path.c_str ()),
                    wave_in_id));

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, friendly_name_string.c_str (),
                            -1);
      } // end WHILE
      enum_moniker_p->Release (); enum_moniker_p = NULL;

      result = true;

      goto continue_;

error:
      if (enumerator_p)
        enumerator_p->Release ();
      if (enum_moniker_p)
        enum_moniker_p->Release ();
      if (moniker_p)
        moniker_p->Release ();
      if (properties_p)
        properties_p->Release ();
      VariantClear (&variant);

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      IMFAttributes* attributes_p = NULL;
      IMFActivate** devices_pp = NULL;
      UINT32 item_count = 0;
      WCHAR buffer_a[BUFSIZ];
      UINT32 length = 0;

      result_2 = MFCreateAttributes (&attributes_p, 1);
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
                               MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFAttributes::SetGUID(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF

      result_2 = MFEnumDeviceSources (attributes_p,
                                      &devices_pp,
                                      &item_count);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFEnumDeviceSources(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      attributes_p->Release (); attributes_p = NULL;
      ACE_ASSERT (devices_pp);

      for (UINT32 index = 0; index < item_count; index++)
      {
        ACE_OS::memset (buffer_a, 0, sizeof (WCHAR[BUFSIZ]));
        length = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
        result_2 =
          devices_pp[index]->GetString (MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                        buffer_a,
                                        sizeof (WCHAR[BUFSIZ]),
                                        &length);
#else
        ACE_ASSERT (false); // *TODO*
        ACE_NOTSUP_RETURN (false);
        ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFActivate::GetString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error_2;
        } // end IF

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (buffer_a)),
                            -1);
      } // end FOR

      for (UINT32 i = 0; i < item_count; i++)
        devices_pp[i]->Release ();
      CoTaskMemFree (devices_pp); devices_pp = NULL;

      result = true;

      goto continue_;

error_2:
      if (attributes_p)
        attributes_p->Release ();
      if (devices_pp)
      {
        for (UINT32 i = 0; i < item_count; i++)
          devices_pp[i]->Release ();
        CoTaskMemFree (devices_pp); devices_pp = NULL;
      } // end IF

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      return false;
    }
  } // end SWITCH
#else
  void** hints_p = NULL;
  int result_2 =
      snd_device_name_hint (-1,
                            ACE_TEXT_ALWAYS_CHAR (STREAM_LIB_ALSA_PCM_INTERFACE_NAME),
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
    string_p = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("IOID"));
    if (!string_p) // NULL --> both
    {
      goto continue_;
    } // end IF
    if (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("Input")))
    {
      free (string_p); string_p = NULL;
      continue;
    } // end IF

continue_:
    string_p = snd_device_name_get_hint (*i, ACE_TEXT_ALWAYS_CHAR ("NAME"));
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
//        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("sysdefault:CARD=MID")) == 0)       ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround21:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround40:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround41:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround50:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround51:CARD=MID,DEV=0")) == 0) ||
        (ACE_OS::strcmp (string_p, ACE_TEXT_ALWAYS_CHAR ("surround71:CARD=MID,DEV=0")) == 0))
    {
      free (string_p); string_p = NULL;
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

    free (string_p); string_p = NULL;
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
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
continue_:
#endif // ACE_WIN32 || ACE_WIN64

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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);
    if (!InlineIsEqualGUID (media_type_p->formattype, FORMAT_WaveFormatEx))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF

    // *NOTE*: FORMAT_VideoInfo2 types do not work with the Video Renderer
    //         directly --> insert the Overlay Mixer
    GUIDs.insert (media_type_p->subtype);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
  } // end FOR

  GtkTreeIter iterator;
  for (std::set<GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                        1, ACE_TEXT (Common_Tools::GUIDToString (*iterator_2).c_str ()),
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stream_descriptor_p->Release (); stream_descriptor_p = NULL;
    return false;
  } // end IF
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

  DWORD count = 0;
  IMFMediaType* media_type_p = NULL;
  struct _GUID GUID_s = GUID_NULL;

  result = media_type_handler_p->GetMediaTypeCount (&count);
  ACE_ASSERT (SUCCEEDED (result));
  for (DWORD i = 0; i < count; ++i)
  {
    media_type_p = NULL;
    result =
      //IMFSourceReader_in->GetNativeMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM,
      //                                        i,
      //                                        &media_type_p);
      media_type_handler_p->GetMediaTypeByIndex (i,
                                                 &media_type_p);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_p->Release ();
      return false;
    } // end IF

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_p->Release ();
      return false;
    } // end IF

    GUIDs.insert (GUID_s);
    media_type_p->Release (); media_type_p = NULL;
  } // end FOR
  media_type_handler_p->Release (); media_type_handler_p = NULL;

  GtkTreeIter iterator;
  for (std::set<struct _GUID, less_guid>::const_iterator iterator_2 = GUIDs.begin ();
       iterator_2 != GUIDs.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (*iterator_2, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()),
                        1, ACE_TEXT (Common_Tools::GUIDToString (*iterator_2).c_str ()),
                        -1);
  } // end FOR

  return true;
}

bool
load_sample_rates (IAMStreamConfig* IAMStreamConfig_in,
                   REFGUID mediaSubType_in,
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);

    if ((!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in)) ||
        (!InlineIsEqualGUID (media_type_p->formattype, FORMAT_WaveFormatEx)))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF

    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    sample_rates.insert (waveformatex_p->nSamplesPerSec);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
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
                        0, ACE_TEXT (converter.str ().c_str ()),
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stream_descriptor_p->Release (); stream_descriptor_p = NULL;
    return false;
  } // end IF
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

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
    if (result != S_OK)
      break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF

    if (InlineIsEqualGUID (GUID_s, mediaSubType_in))
    {
      result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                        &samples_per_second);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        media_type_p->Release (); media_type_p = NULL;
        return false;
      } // end IF
      sample_rates.insert (samples_per_second);
    } // end IF
    media_type_p->Release (); media_type_p = NULL;

    ++count;
  } // end WHILE
  media_type_handler_p->Release (); media_type_handler_p = NULL;
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);

    if (!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in) ||
        !InlineIsEqualGUID (media_type_p->formattype, FORMAT_WaveFormatEx))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    if (waveformatex_p->nSamplesPerSec != sampleRate_in)
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    sample_resolutions.insert (waveformatex_p->wBitsPerSample);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stream_descriptor_p->Release (); stream_descriptor_p = NULL;
    return false;
  } // end IF
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

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
    if (result != S_OK)
      break;

    result = media_type_p->GetGUID (MF_MT_SUBTYPE, &GUID_s);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetGUID(MF_MT_SUBTYPE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF

    if (InlineIsEqualGUID (GUID_s, mediaSubType_in) &&
        (sample_rate == sampleRate_in))
    {
      result = media_type_p->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                        &bits_per_sample);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFGetAttributeRatio(MF_MT_FRAME_RATE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        media_type_p->Release (); media_type_p = NULL;
        return false;
      } // end IF
      sample_resolutions.insert (bits_per_sample);
    } // end IF
    media_type_p->Release (); media_type_p = NULL;

    ++count;
  } // end WHILE
  media_type_handler_p->Release (); media_type_handler_p = NULL;
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      return false;
    } // end IF
    ACE_ASSERT (media_type_p);

    if (!InlineIsEqualGUID (media_type_p->subtype, mediaSubType_in) ||
        !InlineIsEqualGUID (media_type_p->formattype, FORMAT_WaveFormatEx))
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    waveformatex_p =
      reinterpret_cast<struct tWAVEFORMATEX*> (media_type_p->pbFormat);
    if (waveformatex_p->nSamplesPerSec != sampleRate_in)
    {
      Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
      continue;
    } // end IF
    channels.insert (waveformatex_p->nChannels);

    Stream_MediaFramework_DirectShow_Tools::delete_ (media_type_p);
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
    return false;
  } // end IF
  ACE_ASSERT (is_selected);
  presentation_descriptor_p->Release (); presentation_descriptor_p = NULL;
  IMFMediaTypeHandler* media_type_handler_p = NULL;
  result = stream_descriptor_p->GetMediaTypeHandler (&media_type_handler_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to IMFStreamDescriptor::GetMediaTypeHandler(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
    stream_descriptor_p->Release (); stream_descriptor_p = NULL;
    return false;
  } // end IF
  stream_descriptor_p->Release (); stream_descriptor_p = NULL;

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
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                      &sample_rate);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF
    result = media_type_p->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                      &bits_per_sample);
    if (FAILED (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE): \"%s\", aborting\n"),
                  ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
      media_type_handler_p->Release (); media_type_handler_p = NULL;
      media_type_p->Release (); media_type_p = NULL;
      return false;
    } // end IF

    if (InlineIsEqualGUID (GUID_s, mediaSubType_in) &&
        (sample_rate     == sampleRate_in)          &&
        (bits_per_sample == bitsPerSample_in))
    {
      result = media_type_p->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                        &number_of_channels);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        media_type_handler_p->Release (); media_type_handler_p = NULL;
        media_type_p->Release (); media_type_p = NULL;
        return false;
      } // end IF
      channels.push_back (number_of_channels);
    } // end IF
    media_type_p->Release (); media_type_p = NULL;

    ++count;
  } // end WHILE
  media_type_handler_p->Release (); media_type_handler_p = NULL;
  if (result != MF_E_NO_MORE_TYPES)
  {
    ACE_DEBUG ((LM_ERROR,
                //ACE_TEXT ("failed to IMFSourceReader::GetNativeMediaType(%d): \"%s\", aborting\n"),
                ACE_TEXT ("failed to IMFMediaTypeHandler::GetMediaTypeByIndex(%d): \"%s\", aborting\n"),
                count,
                ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
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
              const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in,
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
                                         mediaType_in.access);
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
  snd_pcm_format_mask_free (format_mask_p); format_mask_p = NULL;

  for (std::set<snd_pcm_format_t>::const_iterator iterator_2 = formats_supported.begin ();
       iterator_2 != formats_supported.end ();
       ++iterator_2)
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, snd_pcm_format_name (*iterator_2),
//                        0, snd_pcm_format_description (*iterator_2),
                        2, *iterator_2,
                        -1);
  } // end FOR

  snd_pcm_hw_params_free (format_p); format_p = NULL;

  return true;

error:
  if (format_mask_p)
  {
    snd_pcm_format_mask_free (format_mask_p); format_mask_p = NULL;
  } // end IF
  if (format_p)
  {
    snd_pcm_hw_params_free (format_p); format_p = NULL;
  } // end IF

  return false;
}

bool
load_sample_rates (struct _snd_pcm* handle_in,
                   const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in,
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
                                         mediaType_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         mediaType_in.format);
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
    std::vector<unsigned int> rates_norm_a = { 8000, 11025, 22050, 44100, 48000, 96000 };
//    for (unsigned int i = rate_min;
//         i <= rate_max;
//         ++i)
    for (std::vector<unsigned int>::const_iterator iterator = rates_norm_a.begin ();
         iterator != rates_norm_a.end ();
         ++iterator)
    {
      result = snd_pcm_hw_params_test_rate (handle_in,
                                            format_p,
//                                            i,
                                            *iterator,
                                            0);
      if (result == 0)
//        sample_rates_supported.insert (i);
        sample_rates_supported.insert (*iterator);
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

  snd_pcm_hw_params_free (format_p); format_p = NULL;

  return true;

error:
  if (format_p)
  {
    snd_pcm_hw_params_free (format_p); format_p = NULL;
  } // end IF

  return false;
}

bool
load_sample_resolutions (struct _snd_pcm* handle_in,
                         const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in,
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
                                         mediaType_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         mediaType_in.format);
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
                                       &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).rate,
                                       &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_rate_near(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF

  resolutions_supported.insert (snd_pcm_format_width (mediaType_in.format));
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

  snd_pcm_hw_params_free (format_p); format_p = NULL;

  return true;

error:
  if (format_p)
  {
    snd_pcm_hw_params_free (format_p); format_p = NULL;
  } // end IF

  return false;
}

bool
load_channels (struct _snd_pcm* handle_in,
               const struct Stream_MediaFramework_ALSA_MediaType& mediaType_in,
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
                                         mediaType_in.access);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_set_access(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_set_format (handle_in,
                                         format_p,
                                         mediaType_in.format);
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
                                       &const_cast<struct Stream_MediaFramework_ALSA_MediaType&> (mediaType_in).rate,
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
    std::vector<unsigned int> channels_norm_a = { 1, 2 };
//    for (unsigned int i = channels_min;
//         i <= channels_max;
//         ++i)
    for (std::vector<unsigned int>::const_iterator iterator = channels_norm_a.begin ();
         iterator != channels_norm_a.end ();
         ++iterator)
    {
      result = snd_pcm_hw_params_test_channels (handle_in,
                                                format_p,
//                                                i);
                                                *iterator);
      if (result == 0)
//        channels_supported.insert (i);
        channels_supported.insert (*iterator);
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

  snd_pcm_hw_params_free (format_p); format_p = NULL;

  return true;

error:
  if (format_p)
  {
    snd_pcm_hw_params_free (format_p); format_p = NULL;
  } // end IF

  return false;
}
#endif
bool
#if defined (ACE_WIN32) || defined (ACE_WIN64)
load_audio_effects (GtkListStore* listStore_in,
                    enum Stream_MediaFramework_Type mediaFramework_in)
#else
load_audio_effects (GtkListStore* listStore_in)
#endif
{
  STREAM_TRACE (ACE_TEXT ("::load_audio_effects"));

  bool result = false;

  // initialize result
  gtk_list_store_clear (listStore_in);

  GtkTreeIter iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT result_2 = E_FAIL;
  switch (mediaFramework_in)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      IEnumDMO* enum_DMO_p = NULL;
      int result_3 = -1;
      CLSID class_id = GUID_NULL;
      WCHAR* string_p = NULL;
      std::string friendly_name_string;

      result_2 = DMOEnum (DMOCATEGORY_AUDIO_EFFECT,
                          DMO_ENUMF_INCLUDE_KEYED,
                          0, NULL,
                          0, NULL,
                          &enum_DMO_p);
      if (FAILED (result_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to DMOEnum(DMOCATEGORY_AUDIO_EFFECT): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
        goto error_2;
      } // end IF
      ACE_ASSERT (enum_DMO_p);

      while (S_OK == enum_DMO_p->Next (1, &class_id, &string_p, NULL))
      {
        ACE_ASSERT (string_p);

        friendly_name_string =
           ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (string_p));
        CoTaskMemFree (string_p); string_p = NULL;

        gtk_list_store_append (listStore_in, &iterator);
        gtk_list_store_set (listStore_in, &iterator,
                            0, friendly_name_string.c_str (),
                            1, Common_Tools::GUIDToString (class_id).c_str (),
                            -1);
      } // end WHILE
      enum_DMO_p->Release (); enum_DMO_p = NULL;

      result = true;

      goto continue_;

error_2:
      if (enum_DMO_p)
        enum_DMO_p->Release ();
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      UINT32 item_count = 0;
      IMFTransform* transform_p = NULL;
      MFT_REGISTER_TYPE_INFO mft_register_type_info =
        { MFMediaType_Audio, MFAudioFormat_PCM };
      UINT32 flags = 0;
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      flags = (MFT_ENUM_FLAG_SYNCMFT        |
               MFT_ENUM_FLAG_ASYNCMFT       |
               MFT_ENUM_FLAG_HARDWARE       |
               //MFT_ENUM_FLAG_FIELDOFUSE     |
               MFT_ENUM_FLAG_LOCALMFT       |
               //MFT_ENUM_FLAG_TRANSCODE_ONLY |
               MFT_ENUM_FLAG_SORTANDFILTER);
      IMFActivate** activate_p = NULL;
#else
      CLSID* activate_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      IMFAttributes* attributes_p = NULL;

      if (!Stream_MediaFramework_MediaFoundation_Tools::load (MFT_CATEGORY_AUDIO_EFFECT,
                                                              flags,
                                                              &mft_register_type_info,    // input type
                                                              NULL,                       // output type
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
                                                              activate_p,                 // array of decoders
#else
                                                              NULL,                       // attributes
                                                              activate_p,                 // array of decoders
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
                                                              item_count))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::load(%s,%s), aborting\n"),
                    ACE_TEXT (Common_Tools::GUIDToString (MFT_CATEGORY_AUDIO_EFFECT).c_str ()),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (MFAudioFormat_PCM, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (activate_p);
      if (!item_count)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("cannot find decoder for: \"%s\", aborting\n"),
                    ACE_TEXT (Stream_MediaFramework_Tools::mediaSubTypeToString (MFAudioFormat_PCM, STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION).c_str ())));
        goto error;
      } // end IF

      for (UINT32 i = 0; i < item_count; i++)
      {
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
        result_2 = activate_p[i]->ActivateObject (IID_PPV_ARGS (&transform_p));
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFActivate::ActivateObject(): \"%s\", aborting\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          goto error;
        } // end IF
#else
        ACE_ASSERT (false);
        ACE_NOTSUP_RETURN (false);
        ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
        ACE_ASSERT (transform_p);

        result_2 = transform_p->GetAttributes (&attributes_p);
        if (FAILED (result_2))
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to IMFTransform::GetAttributes(): \"%s\", continuing\n"),
                      ACE_TEXT (Common_Error_Tools::errorToString (result_2).c_str ())));
          transform_p->Release (); transform_p = NULL;
          continue;
        } // end IF
        ACE_ASSERT (attributes_p);

        attributes_p->Release (); attributes_p = NULL;
        transform_p->Release (); transform_p = NULL;
      } // end FOR

      result = true;

error:
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      for (UINT32 i = 0; i < item_count; i++)
        activate_p[i]->Release ();
      CoTaskMemFree (activate_p); activate_p = NULL;
#else
      ACE_ASSERT (false);
      ACE_NOTSUP_RETURN (false);
      ACE_NOTREACHED (return false;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  mediaFramework_in));
      return false;
    }
  } // end SWITCH
#else
  // *NOTE*: (oddly enough), there is currently no way to programmatically
  //         retrieve the list of 'supported' (i.e. internal) SoX effects.
  //         --> parse output of 'sox -h'
  // *TODO*: apparently, SoX also 'sox_effect_find()'s LADSPA effects in the
  //         directory specified by the LADSPA_HOME environment variable
  std::string command_line_string =
      ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_AUDIOEFFECT_SOX_HELP_SHELL_COMMAND);
//  int result_2 = -1;
  std::string command_output_string;
  std::string::size_type start_position, end_position;
  char* saveptr_p = NULL;
  char* effect_string_p = NULL;
  char buffer_a[BUFSIZ];
  ACE_OS::memset (buffer_a, 0, sizeof (char[BUFSIZ]));
  int exit_status = 0;
  if (!Common_Process_Tools::command (command_line_string,
                                      exit_status,
                                      command_output_string))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::command(\"%s\"), aborting\n"),
                ACE_TEXT (command_line_string.c_str ())));
    goto continue_;
  } // end IF
  start_position =
      command_output_string.find (ACE_TEXT_ALWAYS_CHAR ("EFFECTS: "));
  if (start_position == std::string::npos)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse shell command output (was: \"%s\"), aborting\n"),
                ACE_TEXT (command_output_string.c_str ())));
    goto continue_;
  } // end IF
  end_position =
      command_output_string.find_first_of (ACE_TEXT_ALWAYS_CHAR ("\n"),
                                           start_position);
  if (end_position == std::string::npos)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to parse shell command output (was: \"%s\"), aborting\n"),
                ACE_TEXT (command_output_string.c_str ())));
    goto continue_;
  } // end IF
  command_output_string.copy (buffer_a,
                              end_position - (start_position + 9),
                              start_position + 9);
  effect_string_p =
      ACE_OS::strtok_r (buffer_a,
                        ACE_TEXT_ALWAYS_CHAR (" "),
                        &saveptr_p);
  if (!effect_string_p)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("failed to parse shell command output (was: \"%s\"), aborting\n"),
                ACE_TEXT (command_output_string.c_str ())));
    result = true;
    goto continue_;
  } // end IF
  do
  {
    gtk_list_store_append (listStore_in, &iterator);
    gtk_list_store_set (listStore_in, &iterator,
                        0, ACE_TEXT (effect_string_p),
                        -1);

    effect_string_p = ACE_OS::strtok_r (NULL,
                                        ACE_TEXT_ALWAYS_CHAR (" "),
                                        &saveptr_p);
    if (!effect_string_p)
      break; // done
  } while (true);

  result = true;
#endif
continue_:

  return result;
}

unsigned int
get_buffer_size (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::get_buffer_size"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return 0;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

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
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  g_value_init (&value, G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif // ACE_WIN32 || ACE_WIN64
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
  // *IMPORTANT NOTE*: with DirectShow, lower buffer sizes result in lower
  //                   latency
  return (sample_rate * (bits_per_sample / 8) * channels) / 8; // <-- arbitrary factor
#else
  ACE_UNUSED_ARG (bits_per_sample);
  return (sample_rate * snd_pcm_format_size (format_e, 1) * channels);
#endif // ACE_WIN32 || ACE_WIN64
}
void
update_buffer_size (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::update_buffer_size"));

  struct Test_U_AudioEffect_UI_CBDataBase* data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

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
#endif // ACE_WIN32 || ACE_WIN64

  struct Test_U_GTK_ThreadData* data_base_p =
      static_cast<struct Test_U_GTK_ThreadData*> (arg_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator;
  std::ostringstream converter;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_ThreadData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_ThreadData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<Test_U_AudioEffect_DirectShow_ThreadData*> (arg_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->CBData);
      ACE_ASSERT (directshow_ui_cb_data_p->CBData->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->CBData->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<Test_U_AudioEffect_MediaFoundation_ThreadData*> (arg_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->CBData);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->CBData->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->CBData->stream);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_ThreadData* data_p =
    static_cast<struct Test_U_AudioEffect_ThreadData*> (arg_in);
  ACE_ASSERT (data_p->CBData);
  ACE_ASSERT (data_p->CBData->configuration);
  ACE_ASSERT (data_p->CBData->stream);
#endif // ACE_WIN32 || ACE_WIN64

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
#endif // ACE_WIN32 || ACE_WIN64

//  GtkProgressBar* progress_bar_p = NULL;
  GtkStatusbar* statusbar_p = NULL;
  //ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, data_p->CBData->lock);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      iterator =
        state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      // sanity check(s)
      ACE_ASSERT (iterator != state_r.builders.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      iterator =
        state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      // sanity check(s)
      ACE_ASSERT (iterator != state_r.builders.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());
#endif // ACE_WIN32 || ACE_WIN64

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
  Stream_IStream_t* istream_p = NULL;
  Stream_IStreamControlBase* istream_control_p = NULL;
  const Stream_Module_t* module_p = NULL;
  Test_U_Common_ISet_t* resize_notification_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Test_U_AudioEffect_DirectShow_Stream::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_U_AudioEffect_DirectShow_Stream::IINITIALIZE_T*> (directshow_ui_cb_data_p->CBData->stream);
      ACE_ASSERT (iinitialize_p);
      result_2 =
        iinitialize_p->initialize (directshow_ui_cb_data_p->CBData->configuration->streamConfiguration);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->CBData->stream);
      istream_control_p =
        dynamic_cast<Stream_IStreamControlBase*> (directshow_ui_cb_data_p->CBData->stream);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Test_U_AudioEffect_MediaFoundation_Stream::IINITIALIZE_T* iinitialize_p =
        dynamic_cast<Test_U_AudioEffect_MediaFoundation_Stream::IINITIALIZE_T*> (mediafoundation_ui_cb_data_p->CBData->stream);
      ACE_ASSERT (iinitialize_p);
      result_2 =
        iinitialize_p->initialize (mediafoundation_ui_cb_data_p->CBData->configuration->streamConfiguration);
      istream_p =
        dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->CBData->stream);
      istream_control_p =
        dynamic_cast<Stream_IStreamControlBase*> (mediafoundation_ui_cb_data_p->CBData->stream);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  Test_U_AudioEffect_ALSA_Stream::IINITIALIZE_T* iinitialize_p =
    dynamic_cast<Test_U_AudioEffect_ALSA_Stream::IINITIALIZE_T*> (data_p->CBData->stream);
  ACE_ASSERT (iinitialize_p);
  result_2 =
    iinitialize_p->initialize (data_p->CBData->configuration->streamConfiguration);
  istream_p =
    dynamic_cast<Stream_IStream_t*> (data_p->CBData->stream);
  istream_control_p =
    dynamic_cast<Stream_IStreamControlBase*> (data_p->CBData->stream);
  Common_IGetR_2_T<Test_U_AudioEffect_SessionData_t>* iget_p = NULL;
#endif // ACE_WIN32 || ACE_WIN64
  if (!result_2)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream: \"%m\", aborting\n")));
    goto error;
  } // end IF
  ACE_ASSERT (istream_p);
  ACE_ASSERT (istream_control_p);

  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStreamControlBase::find(\"%s\"), aborting\n"),
                ACE_TEXT (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
    goto error;
  } // end IF
  resize_notification_p =
    dynamic_cast<Test_U_Common_ISet_t*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!resize_notification_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Test_U_Common_ISet_t*>(%@), aborting\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    goto error;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->CBData->resizeNotification = resize_notification_p;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p->CBData->resizeNotification = resize_notification_p;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  data_p->CBData->resizeNotification = resize_notification_p;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_SessionData* session_data_p = NULL;
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      Common_IGetR_2_T<Test_U_AudioEffect_DirectShow_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_2_T<Test_U_AudioEffect_DirectShow_SessionData_t>*> (directshow_ui_cb_data_p->CBData->stream);
      ACE_ASSERT (iget_p);
      directshow_session_data_container_p = &iget_p->getR_2 ();
      directshow_session_data_p =
        &const_cast<Test_U_AudioEffect_DirectShow_SessionData&> (directshow_session_data_container_p->getR ());
      session_data_p = directshow_session_data_p;
      directshow_ui_cb_data_p->sessionId = session_data_p->sessionId;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      Common_IGetR_T<Test_U_AudioEffect_MediaFoundation_SessionData_t>* iget_p =
        dynamic_cast<Common_IGetR_T<Test_U_AudioEffect_MediaFoundation_SessionData_t>*> (mediafoundation_ui_cb_data_p->CBData->stream);
      ACE_ASSERT (iget_p);
      mediafoundation_session_data_container_p = &iget_p->getR ();
      mediafoundation_session_data_p =
        &const_cast<Test_U_AudioEffect_MediaFoundation_SessionData&> (mediafoundation_session_data_container_p->getR ());
      session_data_p = mediafoundation_session_data_p;
      mediafoundation_ui_cb_data_p->sessionId = session_data_p->sessionId;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  iget_p =
    dynamic_cast<Common_IGetR_2_T<Test_U_AudioEffect_SessionData_t>*> (data_p->CBData->stream);
  ACE_ASSERT (iget_p);
  session_data_container_p = &iget_p->getR_2 ();
  session_data_p =
      &const_cast<Test_U_AudioEffect_SessionData&> (session_data_container_p->getR ());
  data_p->sessionId = session_data_p->sessionId;
#endif // ACE_WIN32 || ACE_WIN64
  converter.clear ();
  converter.str (ACE_TEXT_ALWAYS_CHAR (""));
  converter << session_data_p->sessionId;

  // generate context id
  gdk_threads_enter ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                                 gtk_statusbar_get_context_id (statusbar_p,
                                                                               converter.str ().c_str ())));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                                 gtk_statusbar_get_context_id (statusbar_p,
                                                                               converter.str ().c_str ())));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  data_base_p->mediaFramework));
      goto error;
    }
  } // end SWITCH
#else
  state_r.contextIds.insert (std::make_pair (COMMON_UI_GTK_STATUSCONTEXT_INFORMATION,
                                             gtk_statusbar_get_context_id (statusbar_p,
                                                                           converter.str ().c_str ())));
#endif // ACE_WIN32 || ACE_WIN64

  gdk_threads_leave ();

  istream_control_p->start ();
  //if (!data_p->CBData->stream->isRunning ())
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to Test_U_AudioEffect_Stream::start(): \"%m\", aborting\n")));
  //  goto done;
  //} // end IF
  istream_control_p->wait (true,
                           false,
                           false);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = 0;
#else
  result = NULL;
#endif // ACE_WIN32 || ACE_WIN64

error:
  //guint event_source_id = g_idle_add (idle_session_end_cb,
  //                                    data_base_p->CBData);
  //if (event_source_id == 0)
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_idle_add(idle_session_end_cb): \"%m\", continuing\n")));
  //else
  //  data_base_p->CBData->eventSourceIds.insert (event_source_id);

  { // synch access
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, std::numeric_limits<ACE_THR_FUNC_RETURN>::max ());
    switch (data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        directshow_ui_cb_data_p->CBData->progressData.completedActions.insert (directshow_ui_cb_data_p->eventSourceId);
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        mediafoundation_ui_cb_data_p->CBData->progressData.completedActions.insert (mediafoundation_ui_cb_data_p->eventSourceId);
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), continuing\n"),
                    data_base_p->mediaFramework));
        break;
      }
    } // end SWITCH
#else
    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, arg_in);
    data_p->CBData->progressData.completedActions.insert (data_p->eventSourceId);
#endif // ACE_WIN32 || ACE_WIN64
  } // end lock scope

  // clean up
  delete data_base_p; data_base_p = NULL;

  return result;
}

//////////////////////////////////////////

gboolean
idle_initialize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_initialize_UI_cb"));

  ACE_ASSERT (userData_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      iterator =
        state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      ACE_ASSERT (iterator != state_r.builders.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);

      iterator =
        state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
      ACE_ASSERT (iterator != state_r.builders.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());
#endif // ACE_WIN32 || ACE_WIN64

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  HRESULT hresult = CoInitializeEx (NULL, COINIT_MULTITHREADED);
  if (FAILED (hresult))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CoInitializeEx(COINIT_MULTITHREADED): \"%s\", continuing\n"),
                ACE_TEXT (Common_Error_Tools::errorToString (hresult).c_str ())));
    //return G_SOURCE_REMOVE;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // step1: initialize dialog window(s)
  GtkDialog* dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_MAIN_NAME)));
  ACE_ASSERT (dialog_p);
  //  GtkWidget* image_icon_p = gtk_image_new_from_file (path.c_str ());
  //  ACE_ASSERT (image_icon_p);
  //  gtk_window_set_icon (GTK_WINDOW (dialog_p),
  //                       gtk_image_get_pixbuf (GTK_IMAGE (image_icon_p)));
  //GdkWindow* dialog_window_p = gtk_widget_get_window (dialog_p);

  GtkAboutDialog* about_dialog_p =
    GTK_ABOUT_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            FALSE);

  GtkSpinButton* spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());

  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());

  gdouble buffer_size = 0.0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      buffer_size =
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      buffer_size =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  buffer_size =
    data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
#endif // ACE_WIN32 || ACE_WIN64
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  gtk_spin_button_set_range (spin_button_p,
                             0.0,
                             std::numeric_limits<unsigned int>::max ());
  gtk_spin_button_set_value (spin_button_p, buffer_size);

  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        1, GTK_SORT_DESCENDING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_capture_devices (list_store_p,
                             ui_cb_data_base_p->mediaFramework))
#else
  if (!load_capture_devices (list_store_p))
#endif // ACE_WIN32 || ACE_WIN64
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

  std::string filename;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      filename =
        (*directshow_modulehandler_configuration_iterator).second.second.targetFileName;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      filename =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.targetFileName;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  filename =
    (*modulehandler_configuration_iterator).second.second.targetFileName;
#endif // ACE_WIN32 || ACE_WIN64

  //GtkFileChooserDialog* file_chooser_dialog_p =
  //  GTK_FILE_CHOOSER_DIALOG (gtk_builder_get_object ((*iterator).second.second,
  //                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERDIALOG_SAVE_NAME)));
  //ACE_ASSERT (file_chooser_dialog_p);
  GtkFileFilter* file_filter_p =
    GTK_FILE_FILTER (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILEFILTER_WAV_NAME)));
  ACE_ASSERT (file_filter_p);
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("application/x-troff-msaudio"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/wav"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/msaudio"));
  gtk_file_filter_add_mime_type (file_filter_p,
                                 ACE_TEXT ("audio/x-msaudio"));
  gtk_file_filter_add_pattern (file_filter_p,
                               ACE_TEXT ("*.wav"));
  gtk_file_filter_set_name (file_filter_p,
                            ACE_TEXT ("WAV files"));

  GtkFileChooserButton* file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  //GError* error_p = NULL;
  //GFile* file_p = NULL;
  gchar* filename_p = NULL;
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
    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        filename.c_str ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (filename.c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF

    //if (!gtk_file_chooser_select_file (GTK_FILE_CHOOSER (file_chooser_dialog_p),
    //                                   file_p,
    //                                   &error_p))
    //{
    //  ACE_DEBUG ((LM_ERROR,
    //              ACE_TEXT ("failed to gtk_file_chooser_select_file(\"%s\"): \"%s\", aborting\n"),
    //              ACE_TEXT (data_p->configuration->moduleHandlerConfiguration.targetFileName.c_str ()),
    //              ACE_TEXT (error_p->message)));
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

    if (!gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (file_chooser_button_p),
                                        Common_File_Tools::getTempDirectory ().c_str ()))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_file_chooser_set_filename(\"%s\"): \"%s\", aborting\n"),
                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ())));
      return G_SOURCE_REMOVE;
    } // end IF
    //g_object_unref (file_p);
  } // end ELSE

  std::string default_folder_uri = ACE_TEXT_ALWAYS_CHAR ("file://");
  default_folder_uri += filename;
  filename_p = Common_UI_GTK_Tools::localeToUTF8 (default_folder_uri);
  ACE_ASSERT (filename_p);
  if (!gtk_file_chooser_set_current_folder_uri (GTK_FILE_CHOOSER (file_chooser_button_p),
                                                filename_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_uri(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (filename_p)));
    g_free (filename_p);
    return G_SOURCE_REMOVE;
  } // end IF
  g_free (filename_p); filename_p = NULL;

  GtkScale* scale_p =
    GTK_SCALE (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SCALE_SINUS_FREQUENCY_NAME)));
  ACE_ASSERT (scale_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      gtk_range_set_value (GTK_RANGE (scale_p),
        (*directshow_modulehandler_configuration_iterator).second.second.sinusFrequency);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      gtk_range_set_value (GTK_RANGE (scale_p),
                           (*mediafoundation_modulehandler_configuration_iterator).second.second.sinusFrequency);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  gtk_range_set_value (GTK_RANGE (scale_p),
                       (*modulehandler_configuration_iterator).second.second.sinusFrequency);
#endif // ACE_WIN32 || ACE_WIN64
  gtk_scale_add_mark (scale_p,
                      gtk_range_get_value (GTK_RANGE (scale_p)),
                      GTK_POS_TOP,
                      NULL);

  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_EFFECT_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (list_store_p),
                                        0, GTK_SORT_ASCENDING);
  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_EFFECT_NAME)));
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
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (!load_audio_effects (list_store_p,
                           ui_cb_data_base_p->mediaFramework))
#else
  if (!load_audio_effects (list_store_p))
#endif // ACE_WIN32 || ACE_WIN64
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_audio_effects(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  bool is_mute = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      is_mute =
        (*directshow_modulehandler_configuration_iterator).second.second.mute;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      is_mute =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.mute;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  is_mute = (*modulehandler_configuration_iterator).second.second.mute;
#endif // ACE_WIN32 || ACE_WIN64
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                is_mute);

  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode_2d =
    STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
  enum Stream_Visualization_SpectrumAnalyzer_3DMode mode_3d =
    STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      mode_2d =
        (*directshow_modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode;
      mode_3d =
        (*directshow_modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mode_2d =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode;
      mode_3d =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  mode_2d =
    (*modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode;
  mode_3d =
    (*modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode;
#endif // ACE_WIN32 || ACE_WIN64
  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_3D_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_toggle_button_set_active (toggle_button_p,
                                (mode_3d < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX));

  //GtkProgressBar* progress_bar_p =
  //  GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  //ACE_ASSERT (progress_bar_p);
  //gint width, height;
  //gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  //gtk_progress_bar_set_pulse_step (progress_bar_p,
  //                                 1.0 / static_cast<double> (width));
  //gtk_progress_bar_set_text (progress_bar_p,
  //                           ACE_TEXT_ALWAYS_CHAR (""));

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);
  gint tooltip_timeout =
      COMMON_UI_GTK_TIMEOUT_DEFAULT_WIDGET_TOOLTIP_DELAY; // ms
#if GTK_CHECK_VERSION(3,0,0)
//#if GTK_CHECK_VERSION(3,10,0)
//#else
//  g_object_set (GTK_WIDGET (drawing_area_p),
//                ACE_TEXT_ALWAYS_CHAR ("gtk-tooltip-timeout"), &tooltip_timeout,
//                NULL);
//#endif // GTK_CHECK_VERSION (3,10,0)
#else
#if GTK_CHECK_VERSION(2,12,0)
  g_object_set (GTK_WIDGET (drawing_area_p),
                ACE_TEXT_ALWAYS_CHAR ("gtk-tooltip-timeout"), &tooltip_timeout,
                NULL);
#endif // GTK_CHECK_VERSION (2,12,0)
#endif // GTK_CHECK_VERSION (3,0,0)

//  GtkBox* box_p = NULL;
#if defined (GTKGL_SUPPORT) && defined (GTKGL_USE)
  Common_UI_GTK_GLContextsIterator_t opengl_contexts_iterator;
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
  GtkGLArea* gl_area_p =
    GTK_GL_AREA (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_NAME)));
  ACE_ASSERT (gl_area_p);
  gtk_widget_realize (GTK_WIDGET (gl_area_p));
  GdkGLContext* context_p = gtk_gl_area_get_context (gl_area_p);
  ACE_ASSERT (context_p);
  ACE_ASSERT (ui_cb_data_base_p->UIState);
  ui_cb_data_base_p->UIState->OpenGLContexts.insert (std::make_pair (gl_area_p, context_p));
  opengl_contexts_iterator =
      ui_cb_data_base_p->UIState->OpenGLContexts.find (gl_area_p);
  ACE_ASSERT (opengl_contexts_iterator != ui_cb_data_base_p->UIState->OpenGLContexts.end ());

  gint major_version, minor_version;
  gtk_gl_area_get_required_version (gl_area_p,
                                    &major_version,
                                    &minor_version);
#else
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GGLA_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int attribute_list[] = {
    GGLA_RGBA,
    GGLA_RED_SIZE,   1,
    GGLA_GREEN_SIZE, 1,
    GGLA_BLUE_SIZE,  1,
    GGLA_DOUBLEBUFFER,
    GGLA_NONE
  };

  GglaArea* gl_area_p = GGLA_AREA (ggla_area_new (attribute_list));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to ggla_area_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  state_r.OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                 gl_area_p->glcontext));
  opengl_contexts_iterator = state_r.OpenGLContexts.find (gl_area_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (G_SOURCE_REMOVE);
  ACE_NOTREACHED (return G_SOURCE_REMOVE;)
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#elif GTK_CHECK_VERSION(2,0,0)
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GDK_GL_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int gl_attributes_a[] = {
    GDK_GL_USE_GL,
// GDK_GL_BUFFER_SIZE
// GDK_GL_LEVEL
    GDK_GL_RGBA,
    GDK_GL_DOUBLEBUFFER,
//    GDK_GL_STEREO
//    GDK_GL_AUX_BUFFERS
    GDK_GL_RED_SIZE,   1,
    GDK_GL_GREEN_SIZE, 1,
    GDK_GL_BLUE_SIZE,  1,
    GDK_GL_ALPHA_SIZE, 1,
//    GDK_GL_DEPTH_SIZE
//    GDK_GL_STENCIL_SIZE
//    GDK_GL_ACCUM_RED_SIZE
//    GDK_GL_ACCUM_GREEN_SIZE
//    GDK_GL_ACCUM_BLUE_SIZE
//    GDK_GL_ACCUM_ALPHA_SIZE
//
//    GDK_GL_X_VISUAL_TYPE_EXT
//    GDK_GL_TRANSPARENT_TYPE_EXT
//    GDK_GL_TRANSPARENT_INDEX_VALUE_EXT
//    GDK_GL_TRANSPARENT_RED_VALUE_EXT
//    GDK_GL_TRANSPARENT_GREEN_VALUE_EXT
//    GDK_GL_TRANSPARENT_BLUE_VALUE_EXT
//    GDK_GL_TRANSPARENT_ALPHA_VALUE_EXT
    GDK_GL_NONE
  };

  GtkGLArea* gl_area_p = GTK_GL_AREA (gtk_gl_area_new (gl_attributes_a));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_gl_area_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  state_r.OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                 gl_area_p->glcontext));
  opengl_contexts_iterator = state_r.OpenGLContexts.find (gl_area_p);
#else
  GdkGLConfigMode features = static_cast<GdkGLConfigMode> (GDK_GL_MODE_DOUBLE  |
                                                           GDK_GL_MODE_ALPHA   |
                                                           GDK_GL_MODE_DEPTH   |
                                                           GDK_GL_MODE_STENCIL |
                                                           GDK_GL_MODE_ACCUM);
  GdkGLConfigMode configuration_mode =
    static_cast<GdkGLConfigMode> (GDK_GL_MODE_RGBA | features);
  GdkGLConfig* gl_config_p = gdk_gl_config_new_by_mode (configuration_mode);
  if (!gl_config_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_gl_config_new_by_mode(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  if (!gtk_widget_set_gl_capability (GTK_WIDGET (drawing_area_2), // widget
                                     gl_config_p,                 // configuration
                                     NULL,                        // share list
                                     true,                        // direct
                                     GDK_GL_RGBA_TYPE))           // render type
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_widget_set_gl_capability(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  state_r.OpenGLContexts.insert (std::make_pair (gtk_widget_get_window (GTK_WIDGET (drawing_area_2)),
                                                 gl_config_p));
  opengl_contexts_iterator = ui_cb_data_base_p->UIState.OpenGLContexts.find (gl_area_p);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION(3,0,0) */
  ACE_ASSERT (opengl_contexts_iterator != state_r.OpenGLContexts.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      //(*directshow_modulehandler_configuration_iterator).second.second.OpenGLWindow =
      //  (*opengl_contexts_iterator).first;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //(*mediafoundation_modulehandler_configuration_iterator).second.second.OpenGLWindow =
      //  (*opengl_contexts_iterator).first;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  //(*modulehandler_configuration_iterator).second.second.OpenGLWindow =
  //  (*opengl_contexts_iterator).first;
#endif // ACE_WIN32 || ACE_WIN64

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK);
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */

#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  // *NOTE*: (try to) enable legacy mode on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  gtk_gl_area_set_required_version ((*opengl_contexts_iterator).first, 2, 1);
#endif // ACE_WIN32 || ACE_WIN64
  gtk_gl_area_set_use_es ((*opengl_contexts_iterator).first, FALSE);
  // *WARNING*: the 'renderbuffer' (in place of 'texture') image attachment
  //            concept appears to be broken; setting this to 'false' gives
  //            "fb setup not supported" (see: gtkglarea.c:734)
  // *TODO*: more specifically, glCheckFramebufferStatusEXT() returns
  //         GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT; find out what is
  //         going on
  // *TODO*: the depth buffer feature is broken on Win32
  gtk_gl_area_set_has_alpha ((*opengl_contexts_iterator).first, TRUE);
  gtk_gl_area_set_has_depth_buffer ((*opengl_contexts_iterator).first, TRUE);
  gtk_gl_area_set_has_stencil_buffer ((*opengl_contexts_iterator).first, FALSE);
  gtk_gl_area_set_auto_render ((*opengl_contexts_iterator).first, TRUE);
  gtk_widget_set_can_focus (GTK_WIDGET ((*opengl_contexts_iterator).first), FALSE);
  gtk_widget_set_hexpand (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  //gtk_widget_set_visible (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */

//  box_p =
//    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
//                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_DISPLAY_NAME)));
//  ACE_ASSERT (box_p);
//  gtk_box_pack_start (box_p,
//                      GTK_WIDGET ((*opengl_contexts_iterator).first),
//                      TRUE, // expand
//                      TRUE, // fill
//                      0);   // padding
#if GTK_CHECK_VERSION(3,8,0)
//  gtk_builder_expose_object ((*iterator).second.second,
//                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_3D_NAME),
//                             G_OBJECT ((*opengl_contexts_iterator).first));
#endif /* GTK_CHECK_VERSION (3,8,0) */
#endif /* GTKGL_SUPPORT && GTKGL_USE */

  // step5: (auto-)connect signals/slots
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step5a: connect default signals
  gulong result_2 =
    g_signal_connect (G_OBJECT (dialog_p),
                      ACE_TEXT_ALWAYS_CHAR ("destroy"),
                      G_CALLBACK (gtk_widget_destroyed),
                      NULL);
  ACE_ASSERT (result_2);

  // step5b: connect custom signals
  //result_2 =
  //  g_signal_connect (G_OBJECT (file_chooser_dialog_p),
  //                    ACE_TEXT_ALWAYS_CHAR ("file-activated"),
  //                    G_CALLBACK (filechooserdialog_cb),
  //                    NULL);
  //ACE_ASSERT (result_2);

  //--------------------------------------

#if defined (GTKGL_SUPPORT) && defined (GTKGL_USE)
//  result_2 =
//    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
//                      ACE_TEXT_ALWAYS_CHAR ("realize"),
//                      G_CALLBACK (glarea_realize_cb),
//                      userData_in);
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
//  result_2 =
//    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
//                      ACE_TEXT_ALWAYS_CHAR ("create-context"),
//                      G_CALLBACK (glarea_create_context_cb),
//                      userData_in);
//  ACE_ASSERT (result_2);
//  result_2 =
//    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
//                      ACE_TEXT_ALWAYS_CHAR ("render"),
//                      G_CALLBACK (glarea_render_cb),
//                      userData_in);
//  ACE_ASSERT (result_2);
//  result_2 =
//    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
//                      ACE_TEXT_ALWAYS_CHAR ("resize"),
//                      G_CALLBACK (glarea_resize_cb),
//                      userData_in);
#else
#if defined (GTKGLAREA_SUPPORT)
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (glarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      G_CALLBACK (glarea_expose_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);
#else
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                      G_CALLBACK (glarea_size_allocate_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("draw"),
                      G_CALLBACK (glarea_draw_cb),
                      userData_in);
  ACE_ASSERT (result_2);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (glarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      G_CALLBACK (glarea_expose_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);
#else
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                      G_CALLBACK (glarea_configure_event_cb),
                      userData_in);
  ACE_ASSERT (result_2);
  result_2 =
    g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                      ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                      G_CALLBACK (glarea_expose_event_cb),
                      userData_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT && GTKGL_USE
  ACE_ASSERT (result_2);

  //--------------------------------------

  //object_p =
  //  gtk_builder_get_object ((*iterator).second.second,
  //                          ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CLEAR_NAME));
  //ACE_ASSERT (object_p);
  //result_2 =
  //  g_signal_connect (object_p,
  //                    ACE_TEXT_ALWAYS_CHAR ("clicked"),
  //                    G_CALLBACK (button_clear_clicked_cb),
  //                    userData_in);
  //ACE_ASSERT (result_2);

  //   // step8: use correct screen
  //   if (parentWidget_in)
  //     gtk_window_set_screen (GTK_WINDOW (dialog_p),
  //                            gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step9: draw main dialog
  gtk_widget_show_all (GTK_WIDGET (dialog_p));

  // debug info
#if defined (_DEBUG)
#if defined (GTKGL_SUPPORT)
  ACE_ASSERT ((*opengl_contexts_iterator).first);
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
  GdkGLContext* gl_context_p = NULL;
  GError* error_p = gtk_gl_area_get_error ((*opengl_contexts_iterator).first);
  if (error_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to realize OpenGL widget: \"%s\", continuing\n"),
                ACE_TEXT (error_p->message)));
    g_error_free (error_p); error_p = NULL;
    goto continue_;
  } // end ELSE

  gl_context_p = gtk_gl_area_get_context ((*opengl_contexts_iterator).first);
  ACE_ASSERT (gl_context_p);
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo (gl_context_p);
#else
#if defined (GTKGLAREA_SUPPORT)
  ACE_ASSERT ((*opengl_contexts_iterator).first->glcontext);
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo ((*opengl_contexts_iterator).first->glcontext);
#else
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo (gtk_widget_get_window (GTK_WIDGET ((*opengl_contexts_iterator).first)));
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  ACE_ASSERT ((*opengl_contexts_iterator).first->glcontext);
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo ((*opengl_contexts_iterator).first->glcontext);
#else
  Common_UI_GTK_Tools::dumpGtkOpenGLInfo ();
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT
#endif // _DEBUG

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,16,0)
continue_:
#endif // GTK_CHECK_VERSION(3,16,0)
#endif // GTKGL_SUPPORT

  // step10: retrieve device handle, OpenGL context, ...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  data_p->handle =
    (*modulehandler_configuration_iterator).second.second.captureDeviceHandle;
#endif // ACE_WIN32 || ACE_WIN64

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
    //ACE_ASSERT (!directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.OpenGLContext);
    //directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.OpenGLContext =
    //  gtk_gl_area_get_context (gl_area_p);
    //ACE_ASSERT (directshow_ui_cb_data_p->configuration->moduleHandlerConfiguration.OpenGLContext);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
    //ACE_ASSERT (!mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.OpenGLContext);
    //mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.OpenGLContext =
    //  gtk_gl_area_get_context (gl_area_p);
    //ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.OpenGLContext);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
//  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second.OpenGLWindow);
  (*modulehandler_configuration_iterator).second.second.window =
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
      gtk_widget_get_window (GTK_WIDGET ((*opengl_contexts_iterator).first));
#else
      gtk_widget_get_window (GTK_WIDGET ((*opengl_contexts_iterator).first));
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second.GdkWindow3D);
  (*modulehandler_configuration_iterator).second.second.GdkWindow3D =
    gtk_widget_get_window (GTK_WIDGET (&(*opengl_contexts_iterator).first->darea));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second.GdkWindow3D);
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
    (*opengl_contexts_iterator).first->glcontext;
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second.GdkWindow3D);
  (*modulehandler_configuration_iterator).second.second.GdkWindow3D =
    gtk_widget_get_window (GTK_WIDGET (&(*opengl_contexts_iterator).first->darea));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second.GdkWindow3D);
#else
    gl_context_p;
  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second.GdkWindow3D);
  (*modulehandler_configuration_iterator).second.second.GdkWindow3D =
    gtk_widget_get_gl_drawable (GTK_WIDGET (drawing_area_2));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second.GdkWindow3D);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second.window);
#endif // ACE_WIN32 || ACE_WIN64
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

  // step11: activate some widgets
  gint n_rows = 0;
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  n_rows =
    gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);
  if (n_rows)
  {
    combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                              TRUE);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF
  else
  {
    GtkToggleButton* toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
    ACE_ASSERT (toggle_button_p);
    gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p),
                              FALSE);
  } // end IF

  bool is_active = !filename.empty ();
  if (is_active)
  {
    toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME)));
    ACE_ASSERT (toggle_button_p);
    gtk_toggle_button_set_active (toggle_button_p,
                                  true);
    GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_SAVE_NAME)));
    ACE_ASSERT (box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                              true);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      is_active =
        (*directshow_modulehandler_configuration_iterator).second.second.sinus;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      is_active =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.sinus;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  is_active = (*modulehandler_configuration_iterator).second.second.sinus;
#endif // ACE_WIN32 || ACE_WIN64
  if (is_active)
  {
    toggle_button_p =
        GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                   ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_SINUS_NAME)));
    ACE_ASSERT (toggle_button_p);
    gtk_toggle_button_set_active (toggle_button_p,
                                  true);
    GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_SINUS_NAME)));
    ACE_ASSERT (box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                              true);
  } // end IF

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID effect_id = GUID_NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      effect_id =
        (*directshow_modulehandler_configuration_iterator).second.second.effect;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      effect_id =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.effect;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
  is_active = !InlineIsEqualGUID (effect_id, GUID_NULL);
#else
  is_active =
    (!(*modulehandler_configuration_iterator).second.second.effect.empty () || is_mute);
#endif // ACE_WIN32 || ACE_WIN64
  if (is_active)
  {
    list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_EFFECT_NAME)));
    ACE_ASSERT (list_store_p);
    n_rows =
      gtk_tree_model_iter_n_children (GTK_TREE_MODEL (list_store_p), NULL);

    combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_EFFECT_NAME)));
    ACE_ASSERT (combo_box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p),
                              (n_rows > 0));

    // *TODO*: there must be a better way to do this...
    GtkTreeIter tree_iterator;
    if (!gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store_p),
                                        &tree_iterator))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_tree_model_get_iter_first(), aborting\n")));
      return G_SOURCE_REMOVE;
    } // end IF
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
    g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
    std::string effect_string_2;
    do
    {
      gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                                &tree_iterator,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                1, &value);
#else
                                0, &value);
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
      effect_string_2 = g_value_get_string (&value);
      g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _GUID effect_id_2 = Common_Tools::StringToGUID (effect_string_2);
      if (InlineIsEqualGUID (effect_id_2, GUID_NULL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                    ACE_TEXT (effect_string_2.c_str ())));
        return G_SOURCE_REMOVE;
      } // end IF
      if (InlineIsEqualGUID (effect_id, effect_id_2))
#else
      if ((*modulehandler_configuration_iterator).second.second.effect == effect_string_2)
#endif // ACE_WIN32 || ACE_WIN64
        break;
    } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store_p),
                                       &tree_iterator));
    gtk_combo_box_set_active_iter (combo_box_p,
                                   &tree_iterator);

    toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_EFFECT_NAME)));
      ACE_ASSERT (toggle_button_p);
    gtk_toggle_button_set_active (toggle_button_p,
                                  true);

    GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_EFFECT_NAME)));
    ACE_ASSERT (box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                              true);
  } // end IF

  GtkRadioButton* radio_button_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      is_active =
        ((mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX) ||
         ((*directshow_modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      is_active =
        ((mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX) ||
         ((*mediafoundation_modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
  is_active =
      (((*modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode <
        STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX) ||
       ((*modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode <
        STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX));
#endif // ACE_WIN32 || ACE_WIN64
  if (is_active)
  {
    toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_VISUALIZATION_NAME)));
    ACE_ASSERT (toggle_button_p);
    gtk_toggle_button_set_active (toggle_button_p,
                                  true);

    if (mode_2d < STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_MAX)
    {
      radio_button_p =
          GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                    (mode_2d == STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE) ? ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME)
                                                                                                                        : ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_SPECTRUM_NAME)));
      ACE_ASSERT (radio_button_p);
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (radio_button_p),
                                    true);
    } // end IF
    if (mode_3d < STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_MAX)
    {
      toggle_button_p =
          GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_3D_NAME)));
      ACE_ASSERT (toggle_button_p);
      gtk_toggle_button_set_active (toggle_button_p,
                                    true);
    } // end IF

    GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_VISUALIZATION_NAME)));
    ACE_ASSERT (box_p);
    gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                              true);
  } // end IF

  GdkWindow* window_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (!(*directshow_modulehandler_configuration_iterator).second.second.window);
      (*directshow_modulehandler_configuration_iterator).second.second.window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second.window);
      window_p =
        (*directshow_modulehandler_configuration_iterator).second.second.window;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (!(*mediafoundation_modulehandler_configuration_iterator).second.second.window);
      (*mediafoundation_modulehandler_configuration_iterator).second.second.window =
        gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second.window);
      window_p =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.window;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return G_SOURCE_REMOVE;
    }
  } // end SWITCH
#else
//  ACE_ASSERT (!(*modulehandler_configuration_iterator).second.second.window);
  (*modulehandler_configuration_iterator).second.second.window =
    gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second.window);
  window_p = (*modulehandler_configuration_iterator).second.second.window;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (window_p);

//  // *NOTE*: the surface / pixel buffer haven't been created yet, as the window
//  //         wasn't 'viewable' during the first 'configure' event
//  //         --> create it now
//  GtkAllocation allocation;
//  gtk_widget_get_allocation (GTK_WIDGET (drawing_area_p),
//                             &allocation);
//  GdkEvent event_s;
//  event_s.type = GDK_CONFIGURE;
//  event_s.configure.type = GDK_CONFIGURE;
//  event_s.configure.window = window_p;
//  event_s.configure.send_event = TRUE;
//  event_s.configure.x = allocation.x;
//  event_s.configure.y = allocation.y;
//  event_s.configure.width = allocation.width;
//  event_s.configure.height = allocation.height;
//  GQuark detail = 0;
////#if GTK_CHECK_VERSION(2,30,0)
////  GValue value = G_VALUE_INIT;
////#else
////  GValue value;
////  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
////  g_value_init (&value, G_TYPE_BOOLEAN);
////#endif // GTK_CHECK_VERSION (2,30,0)
//  g_signal_emit (G_OBJECT (drawing_area_p),
//                 g_signal_lookup (ACE_TEXT_ALWAYS_CHAR ("configure-event"),
//                                  GTK_TYPE_DRAWING_AREA),
//                 detail,
//                 &event_s, userData_in,
//                 //&value);
//                 NULL);
//  //g_signal_emit_by_name (G_OBJECT (drawing_area_p),
//  //                       ACE_TEXT_ALWAYS_CHAR ("configure-event"),
//  //                       &event_s, userData_in,
//  //                       &value,
//  //                       NULL);
//  //g_value_unset (&value);

//  GdkRectangle area_s;
//#if GTK_CHECK_VERSION(3,10,0)
//  // *NOTE*: cairo surfaces are initialized in drawingarea_configure_event_cb
//  //         (see below)
//  cairo_surface_t* surface_p = NULL;
//#else
//  GdkPixbuf* pixel_buffer_p = NULL;
//#endif // GTK_CHECK_VERSION(3,10,0)
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  switch (ui_cb_data_base_p->mediaFramework)
//  {
//    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
//    {
//#if GTK_CHECK_VERSION(3,10,0)
//      surface_p =
//        (*directshow_modulehandler_configuration_iterator).second.second.cairoSurface2D;
//#else
//      pixel_buffer_p =
//        (*directshow_modulehandler_configuration_iterator).second.second.pixelBuffer2D;
//#endif // GTK_CHECK_VERSION(3,10,0)
//      area_s = ui_cb_data_base_p->area2D;
//      break;
//    }
//    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
//    {
//#if GTK_CHECK_VERSION(3,10,0)
//      surface_p =
//        (*mediafoundation_modulehandler_configuration_iterator).second.second.cairoSurface2D;
//#else
//      pixel_buffer_p =
//        (*mediafoundation_modulehandler_configuration_iterator).second.second.pixelBuffer2D;
//#endif // GTK_CHECK_VERSION(3,10,0)
//      area_s = ui_cb_data_base_p->area2D;
//      break;
//    }
//    default:
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
//                  ui_cb_data_base_p->mediaFramework));
//      return G_SOURCE_REMOVE;
//    }
//  } // end SWITCH
//#else
//#if GTK_CHECK_VERSION(3,10,0)
//  surface_p =
//    (*modulehandler_configuration_iterator).second.second.cairoSurface2D;
//#else
//  pixel_buffer_p =
//    (*modulehandler_configuration_iterator).second.second.pixelBuffer2D;
//#endif // GTK_CHECK_VERSION(3,10,0)
//  area_s = ui_cb_data_base_p->area2D;
//#endif // ACE_WIN32 || ACE_WIN64
//  //cairo_t* context_p = gdk_cairo_create (GDK_DRAWABLE (window_p));
//  cairo_t* context_p = gdk_cairo_create (window_p);
//  if (!context_p)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
//    return G_SOURCE_REMOVE;
//  } // end IF
//#if GTK_CHECK_VERSION(3,10,0)
//  ACE_ASSERT (surface_p);
//  //gdk_cairo_set_source_surface (context_p,
//  //                              surface_p,
//  gdk_cairo_set_source_window (context_p,
//                               window_p,
//                               area_s.x, area_s.y);
//#else
//  ACE_ASSERT (pixel_buffer_p);
//  gdk_cairo_set_source_pixbuf (context_p,
//                               pixel_buffer_p,
//                               0.0, 0.0);
//  cairo_reset_clip (context_p);
//#endif // GTK_CHECK_VERSION(3,10,0)
//  cairo_set_source_rgb (context_p, 0.0, 0.0, 0.0);
//  cairo_rectangle (context_p,
//                   0.0, 0.0,
//                   area_s.width, area_s.height);
//  cairo_fill (context_p);
//  cairo_paint (context_p);
//  cairo_destroy (context_p);
//  gdk_window_invalidate_rect (window_p,
//                              NULL,
//                              false);

  // step12: initialize updates
  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    // schedule asynchronous updates of the info view
    guint event_source_id =
        g_timeout_add (COMMON_UI_REFRESH_DEFAULT_WIDGET,
                       idle_update_info_display_cb,
                       userData_in);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE

    event_source_id =
      g_timeout_add (COMMON_UI_GTK_REFRESH_DEFAULT_OPENGL,
                     idle_update_display_cb,
                     userData_in);
    if (event_source_id > 0)
      state_r.eventSourceIds.insert (event_source_id);
    else
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end ELSE
  } // end lock scope

  return G_SOURCE_REMOVE;
}

gboolean
idle_finalize_UI_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_finalize_UI_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
  //    ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      stream_p = directshow_ui_cb_data_p->stream;

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
  //    ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      stream_p = mediafoundation_ui_cb_data_p->stream;

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  // sanity check(s)
//  ACE_ASSERT (data_base_p->configuration);

  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  ACE_ASSERT (data_p->stream);
  stream_p = data_p->stream;

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);

  if (stream_p->isRunning ())
    stream_p->stop (true,  // wait for completion ?
                    true); // locked access ?

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  // clean up
  int result = -1;
  if (data_p->handle)
  {
    result = snd_pcm_close (data_p->handle);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    data_p->handle = NULL;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64

  // leave GTK
  gtk_main_quit ();

  // one-shot action
  return G_SOURCE_REMOVE;
}

gboolean
idle_session_end_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_session_end_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // *IMPORTANT NOTE*: there are two major reasons for being here that are not
  //                   mutually exclusive, so there could be a race:
  //                   - user pressed stop
  //                   - there was an asynchronous error on the stream
  GtkToggleButton* toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_button_set_label (GTK_BUTTON (toggle_button_p),
                        GTK_STOCK_MEDIA_RECORD);
  if (gtk_toggle_button_get_active (toggle_button_p))
  {
    un_toggling_stream = true;
    gtk_toggle_button_set_active (toggle_button_p,
                                  TRUE);
  } // end IF
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p),
                            TRUE);

  GtkButton* button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), FALSE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            FALSE);

  GtkFrame* frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p), true);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_EFFECT_NAME)));
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
  //GtkProgressBar* progressbar_p =
  //  GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  //ACE_ASSERT (progressbar_p);
  //// *NOTE*: this disables "activity mode" (in Gtk2)
  //gtk_progress_bar_set_fraction (progressbar_p, 0.0);
  //gtk_progress_bar_set_text (progressbar_p, ACE_TEXT_ALWAYS_CHAR (""));
  //gtk_widget_set_sensitive (GTK_WIDGET (progressbar_p), false);

//  data_base_p->resizeNotification = NULL;

  return G_SOURCE_REMOVE;
}

//gboolean
//idle_update_log_display_cb (gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::idle_update_log_display_cb"));
//
//  struct Test_U_AudioEffect_UI_CBData* data_p =
//    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != data_p->builders.end ());
//
//  GtkTextView* view_p =
//      GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
//                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
//  ACE_ASSERT (view_p);
//  GtkTextBuffer* buffer_p = gtk_text_view_get_buffer (view_p);
//  ACE_ASSERT (buffer_p);
//
//  GtkTextIter text_iterator;
//  gtk_text_buffer_get_end_iter (buffer_p,
//                                &text_iterator);
//
//  gchar* converted_text = NULL;
//  { // synch access
//    ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->logStackLock, G_SOURCE_REMOVE);
//
//    // sanity check
//    if (data_p->logStack.empty ()) return G_SOURCE_CONTINUE;
//
//    // step1: convert text
//    for (Common_MessageStackConstIterator_t iterator_2 = data_p->logStack.begin ();
//         iterator_2 != data_p->logStack.end ();
//         iterator_2++)
//    {
//      converted_text = Common_UI_GTK_Tools::Locale2UTF8 (*iterator_2);
//      if (!converted_text)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to convert message text (was: \"%s\"), aborting\n"),
//                    ACE_TEXT ((*iterator_2).c_str ())));
//        return G_SOURCE_REMOVE;
//      } // end IF
//
//      // step2: display text
//      gtk_text_buffer_insert (buffer_p,
//                              &text_iterator,
//                              converted_text,
//                              -1);
//
//      // clean up
//      g_free (converted_text);
//    } // end FOR
//
//    data_p->logStack.clear ();
//  } // end lock scope
//
//  // step3: scroll the view accordingly
////  // move the iterator to the beginning of line, so it doesn't scroll
////  // in horizontal direction
////  gtk_text_iter_set_line_offset (&text_iterator, 0);
//
////  // ...and place the mark at iter. The mark will stay there after insertion
////  // because it has "right" gravity
////  GtkTextMark* text_mark_p =
////      gtk_text_buffer_get_mark (buffer_p,
////                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SCROLLMARK_NAME));
//////  gtk_text_buffer_move_mark (buffer_p,
//////                             text_mark_p,
//////                             &text_iterator);
//
////  // scroll the mark onscreen
////  gtk_text_view_scroll_mark_onscreen (view_p,
////                                      text_mark_p);
//  GtkAdjustment* adjustment_p =
//      GTK_ADJUSTMENT (gtk_builder_get_object ((*iterator).second.second,
//                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_ADJUSTMENT_NAME)));
//  ACE_ASSERT (adjustment_p);
//  gtk_adjustment_set_value (adjustment_p,
//                            gtk_adjustment_get_upper (adjustment_p));
//
//  return G_SOURCE_CONTINUE;
//}

gboolean
idle_update_info_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_info_display_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* data_base_p =
      static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  GtkSpinButton* spin_button_p = NULL;
  bool is_session_message = false;
  enum Common_UI_EventType* event_p = NULL;
  int result = -1;
  enum Common_UI_EventType event_e = COMMON_UI_EVENT_INVALID;

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, state_r.lock, G_SOURCE_REMOVE);
    for (Common_UI_Events_t::ITERATOR iterator_2 (state_r.eventStack);
         iterator_2.next (event_p);
         iterator_2.advance ())
    { ACE_ASSERT (event_p);
      switch (*event_p)
      {
        case COMMON_UI_EVENT_STARTED:
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
        case COMMON_UI_EVENT_DATA:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATA_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_base_p->progressData.statistic.bytes));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DATAMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          break;
        }
        case COMMON_UI_EVENT_FINISHED:
        case COMMON_UI_EVENT_STOPPED:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);
          is_session_message = true;
          break;
        }
        case COMMON_UI_EVENT_STATISTIC:
        {
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_CAPTUREDFRAMES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_base_p->progressData.statistic.capturedFrames));
          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_DROPPEDFRAMES_NAME)));
          ACE_ASSERT (spin_button_p);
          gtk_spin_button_set_value (spin_button_p,
                                     static_cast<gdouble> (data_base_p->progressData.statistic.droppedFrames));

          spin_button_p =
            GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_SESSIONMESSAGES_NAME)));
          ACE_ASSERT (spin_button_p);

          ACE_DEBUG ((LM_DEBUG,
                      ACE_TEXT ("statistic: a/s/v (a/v): %.2f/%.2f; (a/c/v): %.2f/%u/%.2f; (a/v): %.2f/%.2f\n"),
                      data_base_p->progressData.statistic.amplitudeAverage,
                      data_base_p->progressData.statistic.amplitudeVariance,
                      data_base_p->progressData.statistic.streakAverage,
                      data_base_p->progressData.statistic.streakCount,
                      data_base_p->progressData.statistic.streakVariance,
                      data_base_p->progressData.statistic.volumeAverage,
                      data_base_p->progressData.statistic.volumeVariance));

          is_session_message = true;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown event type (was: %d), continuing\n"),
                      *event_p));
          break;
        }
      } // end SWITCH
      ACE_UNUSED_ARG (is_session_message);
      gtk_spin_button_spin (spin_button_p,
                            GTK_SPIN_STEP_FORWARD,
                            1.0);
      event_p = NULL;
    } // end FOR

    // clean up
    while (!state_r.eventStack.is_empty ())
    {
      result = state_r.eventStack.pop (event_e);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Unbounded_Stack::pop(): \"%m\", continuing\n")));
    } // end WHILE
  } // end lock scope

  return G_SOURCE_CONTINUE;
}

gboolean
idle_update_progress_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_progress_cb"));

  struct Test_U_AudioEffect_ProgressData* data_p =
      static_cast<struct Test_U_AudioEffect_ProgressData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->state);

  // synch access
  ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->state->lock, G_SOURCE_REMOVE);

  int result = -1;
  Common_UI_GTK_BuildersConstIterator_t iterator =
    data_p->state->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != data_p->state->builders.end ());

  GtkProgressBar* progress_bar_p =
    GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  ACE_ASSERT (progress_bar_p);

  ACE_THR_FUNC_RETURN exit_status;
  ACE_Thread_Manager* thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);
  Common_UI_GTK_PendingActionsIterator_t iterator_2;
  for (Common_UI_GTK_CompletedActionsIterator_t iterator_3 = data_p->completedActions.begin ();
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
#endif // ACE_WIN32 || ACE_WIN64
    } // end ELSE

    data_p->state->eventSourceIds.erase (*iterator_3);
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

    result = data_p->state->condition.broadcast ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Condition::broadcast(): \"%m\", continuing\n")));
  } // end IF

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
idle_update_display_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::idle_update_display_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
      NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // step1: trigger refresh of the 2D drawing area
  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  GdkWindow* window_p = NULL;
  window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
  if (!window_p)
    goto continue_; // <-- not realized yet
  gdk_window_invalidate_rect (window_p,
                              NULL,
                              false);

continue_:
  // step2: trigger refresh of the 3D OpenGL area
  Common_UI_GTK_GLContextsIterator_t iterator_2;
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
  GtkGLArea* gl_area_p =
    GTK_GL_AREA (gtk_builder_get_object ((*iterator).second.second,
                                         ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_NAME)));
  ACE_ASSERT (gl_area_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
//      gl_area_p =
//        (*directshow_modulehandler_configuration_iterator).second.second.window;
      iterator_2 =
          directshow_ui_cb_data_p->UIState->OpenGLContexts.find (gl_area_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
//      gl_area_p =
//        (*mediafoundation_modulehandler_configuration_iterator).second.second.window;
      iterator_2 =
          mediafoundation_ui_cb_data_p->UIState->OpenGLContexts.find (gl_area_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
//  gl_area_p =
//    (*modulehandler_configuration_iterator).second.second.window;
  iterator_2 =
      ui_cb_data_p->UIState->OpenGLContexts.find (gl_area_p);
  ACE_ASSERT (iterator_2 != ui_cb_data_p->UIState->OpenGLContexts.end ());
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (gl_area_p);
  gtk_gl_area_queue_render (gl_area_p);

  window_p = gtk_widget_get_window (GTK_WIDGET (gl_area_p));
#else
#endif // GTK_CHECK_VERSION(3,16,0)
#else /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGLAREA_SUPPORT)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      window_p = (*directshow_modulehandler_configuration_iterator).second.second.window;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      window_p = (*mediafoundation_modulehandler_configuration_iterator).second.second.window;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  window_p = (*modulehandler_configuration_iterator).second.second.window;
#endif // ACE_WIN32 || ACE_WIN64
#else
  drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_3D_NAME)));
  ACE_ASSERT (drawing_area_p);
  window_p = gtk_widget_get_window (GTK_WIDGET (drawing_area_p));
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */
  if (!window_p)
    goto continue_2; // <-- not realized yet
  gdk_window_invalidate_rect (window_p,
                              NULL,
                              false);
continue_2:
#endif /* GTKGL_SUPPORT */
  return G_SOURCE_CONTINUE;
}

//////////////////////////////////////////

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
//void
//textview_size_allocate_cb (GtkWidget* widget_in,
//                           GdkRectangle* rectangle_in,
//                           gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::textview_size_allocate_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//  ACE_UNUSED_ARG (rectangle_in);
//  Test_U_AudioEffect_UI_CBData* data_p =
//      static_cast<Test_U_AudioEffect_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  // sanity check(s)
//  ACE_ASSERT(iterator != data_p->builders.end ());
//
//  GtkScrolledWindow* scrolled_window_p =
//    GTK_SCROLLED_WINDOW (gtk_builder_get_object ((*iterator).second.second,
//                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SCROLLEDWINDOW_NAME)));
//  ACE_ASSERT (scrolled_window_p);
//  GtkAdjustment* adjustment_p =
//    gtk_scrolled_window_get_vadjustment (scrolled_window_p);
//  ACE_ASSERT (adjustment_p);
//  gtk_adjustment_set_value (adjustment_p,
//                            gtk_adjustment_get_upper (adjustment_p) -
//                            gtk_adjustment_get_page_size (adjustment_p));
//} // textview_size_allocate_cb

void
togglebutton_record_toggled_cb (GtkToggleButton* toggleButton_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_record_toggled_cb"));

  // handle untoggle --> PLAY
  if (un_toggling_stream)
  {
    un_toggling_stream = false;
    return; // done
  } // end IF

  // --> user pressed play/pause/stop

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (gtk_manager_p->getR ());
  Common_UI_GTK_BuildersConstIterator_t iterator;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = GUID_NULL;
#endif
#if GTK_CHECK_VERSION(2,30,0)
    GValue value = G_VALUE_INIT;
#else
    GValue value;
    ACE_OS::memset (&value, 0, sizeof (struct _GValue));
    g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      stream_p = directshow_ui_cb_data_p->stream;

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      stream_p = mediafoundation_ui_cb_data_p->stream;

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);

  stream_p = ui_cb_data_p->stream;

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (stream_p);

  iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  // toggle ?
  if (!gtk_toggle_button_get_active (toggleButton_in))
  {
    // --> user pressed pause/stop

    // step0: modify widgets
    gtk_widget_set_sensitive (GTK_WIDGET (toggleButton_in),
                              FALSE);

    // step1: stop stream
    stream_p->stop (false,
                    true);

    return;
  } // end IF

  // --> user pressed record

  struct Test_U_GTK_ThreadData* thread_data_p = NULL;
  ACE_TCHAR thread_name[BUFSIZ];
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

  if (ui_cb_data_base_p->isFirst)
    ui_cb_data_base_p->isFirst = false;

  // step0: update configuration
  spin_button_p =
    GTK_SPIN_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_SPINBUTTON_BUFFERSIZE_NAME)));
  ACE_ASSERT (spin_button_p);
  value_i = gtk_spin_button_get_value_as_int (spin_button_p);
  if (value_i)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize =
          value_i;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize =
          value_i;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    ui_cb_data_base_p->mediaFramework));
        return;
      }
    } // end SWITCH
#else
    ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize =
      static_cast<unsigned int> (value_i);
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        value_i =
          directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        value_i =
          mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    ui_cb_data_base_p->mediaFramework));
        return;
      }
    } // end SWITCH
#else
    value_i =
      static_cast<gint> (ui_cb_data_p->configuration->streamConfiguration.configuration_->allocatorConfiguration->defaultBufferSize);
#endif // ACE_WIN32 || ACE_WIN64
    gtk_spin_button_set_value (spin_button_p,
                               static_cast<gdouble> (value_i));
  } // end ELSE

  GtkComboBox* combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_SOURCE_NAME)));
  ACE_ASSERT (combo_box_p);
  GtkTreeIter iterator_2;
  GtkListStore* list_store_p = NULL;
  GFile* file_p = NULL;
  GtkFrame* frame_p = NULL;
  GtkButton* button_p = NULL;
  GtkToggleButton* toggle_button_p = NULL;
  GtkFileChooserButton* file_chooser_button_p = NULL;
  bool save_to_file = false;

  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture device selected, returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            0, &value);
#else
                            1, &value);
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ACE_OS::strcpy ((*directshow_modulehandler_configuration_iterator).second.second.deviceIdentifier.identifier._string,
                      ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value)));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      ACE_OS::strcpy ((*mediafoundation_modulehandler_configuration_iterator).second.second.deviceIdentifier.identifier._string,
                      ACE_TEXT_ALWAYS_CHAR (g_value_get_string (&value)));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_configuration_iterator).second.second.deviceIdentifier.identifier =
    g_value_get_string (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  if (!Stream_Device_Tools::setFormat (ui_cb_data_p->handle,
                                       ui_cb_data_p->configuration->ALSAConfiguration.format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::setFormat(): \"%m\", returning\n")));
    return;
  } // end IF
  (*modulehandler_configuration_iterator).second.second.captureDeviceHandle =
      ui_cb_data_p->handle;
#endif // ACE_WIN32 || ACE_WIN64

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FORMAT_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture format selected, returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);
  //g_value_reset (&value);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
#else
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
#endif // ACE_WIN32 || ACE_WIN64
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  GUID_s = Common_Tools::StringToGUID (g_value_get_string (&value));
  HRESULT result = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.subtype =
        GUID_s;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetGUID (MF_MT_SUBTYPE,
                                                                                                          GUID_s);
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
//  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format =
//    static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_FREQUENCY_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture frequency selected, returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);
  //value = G_VALUE_INIT;
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.cbFormat == sizeof (struct tWAVEFORMATEX));
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      //waveformatex_p->nSamplesPerSec = g_value_get_uint (&value);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                                            g_value_get_uint (&value));
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.rate =
    g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_RESOLUTION_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture resolution selected, returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_RESOLUTION_NAME)));
  ACE_ASSERT (list_store_p);
  //value = G_VALUE_INIT;
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.cbFormat == sizeof (struct tWAVEFORMATEX));
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      waveformatex_p->wBitsPerSample = static_cast<WORD> (g_value_get_uint (&value));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                            g_value_get_uint (&value));
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // *NOTE*: ALSA encodes the resolution in the format identifier, so it has
  //         already been set at this stage
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s --> %d\n"),
              ACE_TEXT (snd_pcm_format_description (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format)),
              snd_pcm_format_width (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format)));
//  ui_cb_data_p->configuration->streamConfiguration.format. =
//    g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);

  combo_box_p =
    GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_CHANNELS_NAME)));
  ACE_ASSERT (combo_box_p);
  if (!gtk_combo_box_get_active_iter (combo_box_p,
                                      &iterator_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no capture channels selected, returning\n")));
    return;
  } // end IF
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_CHANNELS_NAME)));
  ACE_ASSERT (list_store_p);
  //value = G_VALUE_INIT;
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_UINT);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.cbFormat == sizeof (struct tWAVEFORMATEX));
      ACE_ASSERT (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      waveformatex_p->nChannels = static_cast<WORD> (g_value_get_uint (&value));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                            g_value_get_uint (&value));
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels =
    g_value_get_uint (&value);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);

  toggle_button_p =
    GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                               ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_CHECKBUTTON_SAVE_NAME)));
  ACE_ASSERT (toggle_button_p);
  save_to_file = gtk_toggle_button_get_active (toggle_button_p);
  file_chooser_button_p =
    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
  ACE_ASSERT (file_chooser_button_p);
  file_p =
    gtk_file_chooser_get_file (GTK_FILE_CHOOSER (file_chooser_button_p));
  if (file_p)
  {
    char* filename_p = g_file_get_path (file_p);
    if (!filename_p)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_file_get_path(): \"%m\", returning\n")));
      g_object_unref (file_p); file_p = NULL;
      return;
    } // end IF
    g_object_unref (file_p); file_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        if (save_to_file)
          (*directshow_modulehandler_configuration_iterator).second.second.targetFileName =
            Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
        else
          (*directshow_modulehandler_configuration_iterator).second.second.targetFileName.clear ();
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        if (save_to_file)
          (*mediafoundation_modulehandler_configuration_iterator).second.second.targetFileName =
            Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
        else
          (*mediafoundation_modulehandler_configuration_iterator).second.second.targetFileName.clear ();
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    ui_cb_data_base_p->mediaFramework));
        return;
      }
    } // end SWITCH
#else
    if (save_to_file)
      (*modulehandler_configuration_iterator).second.second.targetFileName =
        Common_UI_GTK_Tools::UTF8ToLocale (filename_p, -1);
    else
      (*modulehandler_configuration_iterator).second.second.targetFileName.clear ();
#endif // ACE_WIN32 || ACE_WIN64
    g_free (filename_p);
  } // end IF

  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_MUTE_NAME)));
  ACE_ASSERT (toggle_button_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second.mute =
        gtk_toggle_button_get_active (toggle_button_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second.mute =
        gtk_toggle_button_get_active (toggle_button_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_configuration_iterator).second.second.mute =
    gtk_toggle_button_get_active (toggle_button_p);
#endif // ACE_WIN32 || ACE_WIN64

  // sanity check(s)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: reusing a media session doesn't work reliably at the moment
  //         --> recreate a new session on every run
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if ((*mediafoundation_modulehandler_configuration_iterator).second.second.session)
      {
        //HRESULT result = E_FAIL;
        // *TODO*: this crashes in CTopoNode::UnlinkInput ()...
        //result =
        //  data_p->configuration->moduleHandlerConfiguration.session->Shutdown ();
        //if (FAILED (result))
        //  ACE_DEBUG ((LM_ERROR,
        //              ACE_TEXT ("failed to IMFMediaSession::Shutdown(): \"%s\", continuing\n"),
        //              ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        (*mediafoundation_modulehandler_configuration_iterator).second.second.session->Release ();
        (*mediafoundation_modulehandler_configuration_iterator).second.second.session =
          NULL;
      } // end IF

      // set missing format properties
      UINT32 number_of_channels, bits_per_sample, sample_rate;
      HRESULT result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->GetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                                       &sample_rate);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->GetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                       &bits_per_sample);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                       &number_of_channels);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->SetUINT32 (MF_MT_AUDIO_BLOCK_ALIGNMENT,
                                                                                                       (bits_per_sample * number_of_channels) / 8);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->SetUINT32 (MF_MT_AUDIO_AVG_BYTES_PER_SECOND,
                                                                                                       sample_rate * (bits_per_sample * number_of_channels / 8));
      ACE_ASSERT (SUCCEEDED (result));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT ((*modulehandler_configuration_iterator).second.second.captureDeviceHandle);

//  if (!Stream_Device_Tools::setCaptureFormat (data_p->configuration->moduleHandlerConfiguration.deviceHandle,
//                                                     *data_p->configuration->moduleHandlerConfiguration.format))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to Stream_Device_Tools::setCaptureFormat(), aborting\n")));
//    goto error;
//  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  topology_p->Release ();
//#endif
  //struct _AMMediaType* media_type_p = NULL;
  //Stream_Device_Tools::getCaptureFormat (data_p->configuration->moduleHandlerConfiguration.builder,
  //                                              media_type_p);
  //media_type.Set (*media_type_p);
  //ACE_ASSERT (media_type == *data_p->configuration->moduleHandlerConfiguration.format);

  // step2: modify widgets
  gtk_button_set_label (GTK_BUTTON (toggleButton_in),
                        GTK_STOCK_MEDIA_STOP);

  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_CUT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p), TRUE);
  button_p =
    GTK_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BUTTON_REPORT_NAME)));
  ACE_ASSERT (button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (button_p),
                            TRUE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_CONFIGURATION_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
                            FALSE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_SAVE_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
                            FALSE);

  frame_p =
    GTK_FRAME (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FRAME_EFFECT_NAME)));
  ACE_ASSERT (frame_p);
  gtk_widget_set_sensitive (GTK_WIDGET (frame_p),
                            FALSE);

  // step1: set up progress reporting
  ui_cb_data_base_p->progressData.statistic = Test_U_AudioEffect_Statistic ();
  //GtkProgressBar* progress_bar_p =
  //  GTK_PROGRESS_BAR (gtk_builder_get_object ((*iterator).second.second,
  //                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_PROGRESSBAR_NAME)));
  //ACE_ASSERT (progress_bar_p);
  //gint width, height;
  //gtk_widget_get_size_request (GTK_WIDGET (progress_bar_p), &width, &height);
  //gtk_progress_bar_set_pulse_step (progress_bar_p,
  //                                 1.0 / static_cast<double> (width));
  //gtk_progress_bar_set_fraction (progress_bar_p, 0.0);

  // step3: start processing thread
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      struct Test_U_AudioEffect_DirectShow_ThreadData* thread_data_2 = NULL;
      ACE_NEW_NORETURN (thread_data_2,
                        struct Test_U_AudioEffect_DirectShow_ThreadData ());
      if (thread_data_2)
      {
        thread_data_2->CBData = directshow_ui_cb_data_p;
        thread_data_p = thread_data_2;
      } // end IF
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      struct Test_U_AudioEffect_MediaFoundation_ThreadData* thread_data_2 =
        NULL;
      ACE_NEW_NORETURN (thread_data_2,
                        struct Test_U_AudioEffect_MediaFoundation_ThreadData ());
      if (thread_data_2)
      {
        thread_data_2->CBData = mediafoundation_ui_cb_data_p;
        thread_data_p = thread_data_2;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  ACE_NEW_NORETURN (thread_data_p,
                    struct Test_U_AudioEffect_ThreadData);
  if (thread_data_p)
    static_cast<struct Test_U_AudioEffect_ThreadData*> (thread_data_p)->CBData =
      ui_cb_data_p;
#endif // ACE_WIN32 || ACE_WIN64
  if (!thread_data_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to allocate memory: \"%m\", returning\n")));
    return;
  } // end IF
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
                  ACE_TEXT (TEST_U_STREAM_THREAD_NAME));
  thread_name_2 = thread_name;
  thread_manager_p = ACE_Thread_Manager::instance ();
  ACE_ASSERT (thread_manager_p);

  // *NOTE*: lock access to the progress report structures to avoid a race
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, state_r.lock);
    int result =
      thread_manager_p->spawn (::stream_processing_function,     // function
                               thread_data_p,                    // argument
                               (THR_NEW_LWP      |
                                THR_JOINABLE     |
                                THR_INHERIT_SCHED),              // flags
                               &thread_id,                       // thread id
                               &thread_handle,                   // thread handle
                               ACE_DEFAULT_THREAD_PRIORITY,      // priority
                               COMMON_EVENT_REACTOR_THREAD_GROUP_ID + 1, // *TODO*: group id
                               NULL,                             // stack
                               0,                                // stack size
                               &thread_name_2);                  // name
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Thread_Manager::spawn(): \"%m\", returning\n")));
      delete thread_data_p; thread_data_p = NULL;
      return;
    } // end IF

    // step3: start progress reporting
    //ACE_ASSERT (!data_p->progressEventSourceId);
    ui_cb_data_base_p->progressData.eventSourceId =
    //  //g_idle_add_full (G_PRIORITY_DEFAULT_IDLE, // _LOW doesn't work (on Win32)
    //  //                 idle_update_progress_cb,
    //  //                 &data_p->progressData,
    //  //                 NULL);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE,            // _LOW doesn't work (on Win32)
                          COMMON_UI_REFRESH_DEFAULT_PROGRESS, // ms (?)
                          idle_update_progress_cb,
                          &ui_cb_data_base_p->progressData,
                          NULL);
    if (!ui_cb_data_base_p->progressData.eventSourceId)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to g_timeout_add_full(idle_update_progress_cb): \"%m\", returning\n")));

      // clean up
      ACE_THR_FUNC_RETURN exit_status;
      result = thread_manager_p->join (thread_id, &exit_status);
      if (result == -1)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Thread_Manager::join(): \"%m\", continuing\n")));
      return;
    } // end IF
    thread_data_p->eventSourceId =
      ui_cb_data_base_p->progressData.eventSourceId;
    ui_cb_data_base_p->progressData.pendingActions[ui_cb_data_base_p->progressData.eventSourceId] =
      ACE_Thread_ID (thread_id, thread_handle);
    //    ACE_DEBUG ((LM_DEBUG,
    //                ACE_TEXT ("idle_update_progress_cb: %d\n"),
    //                event_source_id));
    state_r.eventSourceIds.insert (ui_cb_data_base_p->progressData.eventSourceId);
  } // end lock scope
} // togglebutton_record_toggled_cb

void
button_cut_clicked_cb (GtkButton* button_in,
                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_cut_clicked_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);

      stream_p = directshow_ui_cb_data_p->stream;
      Test_U_AudioEffect_DirectShow_IStreamControl_t* istream_control_p =
        dynamic_cast<Test_U_AudioEffect_DirectShow_IStreamControl_t*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (istream_control_p);
      istream_control_p->control (STREAM_CONTROL_STEP,
                                  false); // recurse upstream ?

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);

      stream_p = mediafoundation_ui_cb_data_p->stream;
      Test_U_AudioEffect_MediaFoundation_IStreamControl_t* istream_control_p =
        dynamic_cast<Test_U_AudioEffect_MediaFoundation_IStreamControl_t*> (mediafoundation_ui_cb_data_p->stream);
      ACE_ASSERT (istream_control_p);
      istream_control_p->control (STREAM_CONTROL_STEP,
                                  false); // recurse upstream ?

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->stream);

  stream_p = data_p->stream;
  Test_U_AudioEffect_IStreamControl_t* istream_control_p =
    dynamic_cast<Test_U_AudioEffect_IStreamControl_t*> (data_p->stream);
  ACE_ASSERT (istream_control_p);
  istream_control_p->control (STREAM_CONTROL_STEP,
                              false); // recurse upstream ?
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (stream_p);
} // action_cut_activate_cb

void
button_report_clicked_cb (GtkButton* button_in,
                          gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_report_activate_cb"));

  ACE_UNUSED_ARG (button_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);
      stream_p = directshow_ui_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);
      stream_p = mediafoundation_ui_cb_data_p->stream;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->stream);

  stream_p = data_p->stream;
#endif
  ACE_ASSERT (stream_p);
} // action_report_activate_cb

void
togglebutton_save_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_save_toggled_cb"));

  ACE_UNUSED_ARG (toggleButton_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);

  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_SAVE_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            is_active);

//  GtkFileChooserButton* file_chooser_button_p =
//    GTK_FILE_CHOOSER_BUTTON (gtk_builder_get_object ((*iterator).second.second,
//                                                     ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_FILECHOOSERBUTTON_SAVE_NAME)));
//  ACE_ASSERT (file_chooser_button_p);
//  GError* error_p = NULL;
//  GFile* file_p = NULL;
//  if (!is_active)
//  {
//    file_p =
//      g_file_new_for_path (Common_File_Tools::getTempDirectory ().c_str ());
//    ACE_ASSERT (file_p);
//    if (!gtk_file_chooser_set_current_folder_file (GTK_FILE_CHOOSER (file_chooser_button_p),
//                                                   file_p,
//                                                   &error_p))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to gtk_file_chooser_set_current_folder_file(\"%s\"): \"%s\", aborting\n"),
//                  ACE_TEXT (Common_File_Tools::getTempDirectory ().c_str ()),
//                  ACE_TEXT (error_p->message)));

//      // clean up
//      g_error_free (error_p);
//      g_object_unref (file_p);

//      return;
//    } // end IF
//    g_object_unref (file_p);
//  } // end ELSE
} // togglebutton_save_toggled_cb

void
togglebutton_sinus_toggled_cb (GtkToggleButton* toggleButton_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_sinus_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*directshow_modulehandler_configuration_iterator).second.second.sinus =
        is_active;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*mediafoundation_modulehandler_configuration_iterator).second.second.sinus =
        is_active;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second.sinus =
    is_active;
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_SINUS_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            is_active);
} // togglebutton_sinus_toggled_cb

void
scale_sinus_frequency_value_changed_cb (GtkRange* range_in,
                                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::scale_sinus_frequency_value_changed_cb"));

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*directshow_modulehandler_configuration_iterator).second.second.sinusFrequency =
        gtk_range_get_value (range_in);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*mediafoundation_modulehandler_configuration_iterator).second.second.sinusFrequency =
        gtk_range_get_value (range_in);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second.sinusFrequency =
    gtk_range_get_value (range_in);
#endif
} // scale_sinus_frequency_value_changed_cb

void
togglebutton_effect_toggled_cb (GtkToggleButton* toggleButton_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_effect_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);

  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_EFFECT_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            is_active);

  if (is_active)
  {
    GtkComboBox* combo_box_p =
      GTK_COMBO_BOX (gtk_builder_get_object ((*iterator).second.second,
                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_COMBOBOX_EFFECT_NAME)));
    ACE_ASSERT (combo_box_p);

    GtkTreeIter iterator_2;
    if (gtk_combo_box_get_active_iter (combo_box_p,
                                       &iterator_2))
    {
      GtkListStore* list_store_p =
        GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_EFFECT_NAME)));
      ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
      GValue value = G_VALUE_INIT;
#else
      GValue value;
      ACE_OS::memset (&value, 0, sizeof (struct _GValue));
      g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
      gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                                &iterator_2,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                1, &value);
#else
                                0, &value);
#endif // ACE_WIN32 || ACE_WIN64
      ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
      std::string effect_string = g_value_get_string (&value);
      g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      struct _GUID GUID_s = Common_Tools::StringToGUID (effect_string);
      if (InlineIsEqualGUID (GUID_s, GUID_NULL))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                    ACE_TEXT (effect_string.c_str ())));
        return;
      } // end IF

      switch (ui_cb_data_base_p->mediaFramework)
      {
        case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
        {
          (*directshow_modulehandler_configuration_iterator).second.second.effect =
            GUID_s;
          break;
        }
        case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
        {
          (*mediafoundation_modulehandler_configuration_iterator).second.second.effect =
            GUID_s;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                      ui_cb_data_base_p->mediaFramework));
          return;
        }
      } // end SWITCH
#else
      (*modulehandler_configuration_iterator).second.second.effect =
        effect_string;
#endif // ACE_WIN32 || ACE_WIN64
    } // end IF
  } // end IF
  else
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
        (*directshow_modulehandler_configuration_iterator).second.second.effect =
          GUID_NULL;
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
        (*mediafoundation_modulehandler_configuration_iterator).second.second.effect =
          GUID_NULL;
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    ui_cb_data_base_p->mediaFramework));
        return;
      }
    } // end SWITCH
#else
    (*modulehandler_configuration_iterator).second.second.effect.clear ();
#endif // ACE_WIN32 || ACE_WIN64
  } // end IF
} // togglebutton_effect_toggled_cb

void
togglebutton_mute_toggled_cb (GtkToggleButton* toggleButton_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_mute_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*directshow_modulehandler_configuration_iterator).second.second.mute =
        is_active;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      (*mediafoundation_modulehandler_configuration_iterator).second.second.mute =
        is_active;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (data_p);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());

  (*modulehandler_configuration_iterator).second.second.mute =
    is_active;
#endif
} // togglebutton_mute_toggled_cb

void
togglebutton_visualization_toggled_cb (GtkToggleButton* toggleButton_in,
                                       gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_visualization_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkBox* box_p =
      GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                       ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_BOX_VISUALIZATION_NAME)));
  ACE_ASSERT (box_p);
  gtk_widget_set_sensitive (GTK_WIDGET (box_p),
                            gtk_toggle_button_get_active (toggleButton_in));
} // togglebutton_visualization_toggled_cb

void
radiobutton_2d_toggled_cb (GtkToggleButton* toggleButton_in,
                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::radiobutton_2d_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);
  if (!is_active)
    return;

  GtkRadioButton* radio_button_p =
      GTK_RADIO_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_RADIOBUTTON_OSCILLOSCOPE_NAME)));
  ACE_ASSERT (radio_button_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode =
          (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                                : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode =
          (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                                : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode =
      (radio_button_p == GTK_RADIO_BUTTON (toggleButton_in) ? STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE
                                                            : STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM);
#endif
} // radiobutton_2d_toggled_cb

void
togglebutton_3d_toggled_cb (GtkToggleButton* toggleButton_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::togglebutton_3d_toggled_cb"));

  // sanity check(s)
  ACE_ASSERT (toggleButton_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  bool is_active = gtk_toggle_button_get_active (toggleButton_in);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode =
        (is_active ? STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT
                   : STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode =
        (is_active ? STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT
                   : STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_configuration_iterator).second.second.spectrumAnalyzer3DMode =
      (is_active ? STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_DEFAULT
                 : STREAM_VISUALIZATION_SPECTRUMANALYZER_3DMODE_INVALID);
#endif
} // togglebutton_3d_toggled_cb

// -----------------------------------------------------------------------------

//gint
//button_clear_clicked_cb (GtkWidget* widget_in,
//                         gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::button_clear_clicked_cb"));
//
//  ACE_UNUSED_ARG (widget_in);
//  Test_U_AudioEffect_UI_CBData* data_p =
//    static_cast<Test_U_AudioEffect_UI_CBData*> (userData_in);
//
//  // sanity check(s)
//  ACE_ASSERT (data_p);
//
//  Common_UI_GTK_BuildersConstIterator_t iterator =
//    data_p->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
//  ACE_ASSERT (iterator != data_p->builders.end ());
//
////  GtkTextView* view_p =
////    GTK_TEXT_VIEW (gtk_builder_get_object ((*iterator).second.second,
////                                           ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TEXTVIEW_NAME)));
////  ACE_ASSERT (view_p);
////  GtkTextBuffer* buffer_p =
//////    gtk_text_buffer_new (NULL); // text tag table --> create new
////    gtk_text_view_get_buffer (view_p);
////  ACE_ASSERT (buffer_p);
////  gtk_text_buffer_set_text (buffer_p,
////                            ACE_TEXT_ALWAYS_CHAR (""), 0);
//
//  return FALSE;
//}

void
button_about_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_about_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());

  // retrieve about dialog handle
  GtkDialog* about_dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DIALOG_ABOUT_NAME)));
  ACE_ASSERT (about_dialog_p);

  // run dialog
  gint result = gtk_dialog_run (about_dialog_p);
  switch (result)
  {
    case GTK_RESPONSE_ACCEPT:
      break;
    default:
      break;
  } // end SWITCH
  gtk_widget_hide (GTK_WIDGET (about_dialog_p));
} // button_about_clicked_cb

void
button_settings_clicked_cb (GtkButton* button_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_settings_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());
} // button_about_clicked_cb

void
button_reset_clicked_cb (GtkButton* button_in,
                         gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::action_reset_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != state_r.builders.end ());
} // button_reset_clicked_cb

void
button_quit_clicked_cb (GtkButton* button_in,
                        gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::button_quit_clicked_cb"));

  ACE_UNUSED_ARG (button_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
  enum Stream_StateMachine_ControlState status_e = STREAM_STATE_INVALID;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      stream_p = directshow_ui_cb_data_p->stream;
      Test_U_AudioEffect_DirectShow_IStreamControl_t* istream_control_p =
        dynamic_cast<Test_U_AudioEffect_DirectShow_IStreamControl_t*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (istream_control_p);
      status_e = istream_control_p->status ();

      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      stream_p = mediafoundation_ui_cb_data_p->stream;
      Test_U_AudioEffect_MediaFoundation_IStreamControl_t* istream_control_p =
        dynamic_cast<Test_U_AudioEffect_MediaFoundation_IStreamControl_t*> (directshow_ui_cb_data_p->stream);
      ACE_ASSERT (istream_control_p);
      status_e = istream_control_p->status ();

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  stream_p = ui_cb_data_p->stream;
  Test_U_AudioEffect_IStreamControl_t* istream_control_p =
    dynamic_cast<Test_U_AudioEffect_IStreamControl_t*> (ui_cb_data_p->stream);
  ACE_ASSERT (istream_control_p);
  status_e = istream_control_p->status ();
#endif
  ACE_ASSERT (stream_p);

  if ((status_e == STREAM_STATE_RUNNING) ||
      (status_e == STREAM_STATE_PAUSED))
    stream_p->stop (false, true, true);

  // wait for processing thread(s)
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, ui_cb_data_base_p->UIState->lock);
    while (!ui_cb_data_base_p->progressData.pendingActions.empty ())
      ui_cb_data_base_p->UIState->condition.wait (NULL);
  } // end lock scope

  // step1: remove event sources
  { ACE_GUARD (ACE_Thread_Mutex, aGuard, ui_cb_data_base_p->UIState->lock);
    for (Common_UI_GTK_EventSourceIdsIterator_t iterator = ui_cb_data_base_p->UIState->eventSourceIds.begin ();
         iterator != ui_cb_data_base_p->UIState->eventSourceIds.end ();
         iterator++)
      if (!g_source_remove (*iterator))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to g_source_remove(%u), continuing\n"),
                    *iterator));
    ui_cb_data_base_p->UIState->eventSourceIds.clear ();
  } // end lock scope

  // step2: initiate shutdown sequence
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop (false, // wait ?
                                                      true); // high priority ?

  int result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::raise(%S): \"%m\", continuing\n"),
                SIGINT));
} // button_quit_clicked_cb

void
combobox_effect_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_effect_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
      static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_EFFECT_NAME)));
  ACE_ASSERT (list_store_p);
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string effect_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  g_value_init (&value_2, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  struct _GUID effect_GUID =
    Common_Tools::StringToGUID (g_value_get_string (&value_2));
  if (InlineIsEqualGUID (effect_GUID, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (g_value_get_string (&value_2))));
    return;
  } // end IF
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      (*directshow_modulehandler_configuration_iterator).second.second.effect =
        effect_GUID;
      if (InlineIsEqualGUID (effect_GUID, GUID_DSCFX_CLASS_AEC))
      {

      } // end IF
      //////////////////////////////////////
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_CHORUS))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_COMPRESSOR))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_DISTORTION))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_ECHO))
      {
        struct _DSFXEcho effect_options;
        effect_options.fFeedback = 50.0F;
        effect_options.fLeftDelay = 10.0F;
        effect_options.fRightDelay = 10.0F;
        effect_options.fWetDryMix = 50.0F;
        effect_options.lPanDelay = 0;
        (*directshow_modulehandler_configuration_iterator).second.second.effectOptions.echoOptions =
          effect_options;
      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_PARAMEQ))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_FLANGER))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_GARGLE))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_STANDARD_I3DL2REVERB))
      {

      } // end ELSE IF
      else if (InlineIsEqualGUID (effect_GUID, GUID_DSFX_WAVES_REVERB))
      {

      } // end ELSE IF
      //////////////////////////////////////
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown effect (was: \"%s\"; GUID: %s), continuing\n"),
                    ACE_TEXT (effect_string.c_str ()),
                    ACE_TEXT (Common_Tools::GUIDToString (effect_GUID).c_str ())));
      } // end ELSE
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      (*mediafoundation_modulehandler_configuration_iterator).second.second.effect =
        effect_GUID;
      if (InlineIsEqualGUID (effect_GUID, GUID_NULL))
      {

      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  (*modulehandler_configuration_iterator).second.second.effectOptions.clear ();
  if (effect_string == ACE_TEXT_ALWAYS_CHAR ("chorus"))
  {
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.5"));  // gain in
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.9"));  // gain out
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("50"));   // delay (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.4"));  // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.25")); // speed (Hz)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("2"));    // depth (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-t"));   // modulation
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("60"));   // delay (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.32")); // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.4"));  // speed (Hz)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("2.3"));  // depth (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-t"));   // modulation
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("40"));   // delay (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.3"));  // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.3"));  // speed (Hz)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("1.3"));  // depth (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("-s"));   // modulation
  } // end IF
  else if (effect_string == ACE_TEXT_ALWAYS_CHAR ("echo"))
  {
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.8"));  // gain in
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.9"));  // gain out
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("100"));  // delay (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.3"));  // decay (% gain in)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("200"));  // delay (ms)
    (*modulehandler_configuration_iterator).second.second.effectOptions.push_back (ACE_TEXT_ALWAYS_CHAR ("0.25")); // decay (% gain in)
  } // end ELSE IF
  else
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("invalid/unknown effect (was: \"%s\"), using default options, continuing\n"),
                ACE_TEXT (effect_string.c_str ())));
#endif // ACE_WIN32 || ACE_WIN64
} // combobox_effect_changed_cb

void
combobox_source_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_source_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStreamControlBase* stream_p = NULL;
  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p = NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p = NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (directshow_ui_cb_data_p->stream);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      stream_p = directshow_ui_cb_data_p->stream;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->stream);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      stream_p = mediafoundation_ui_cb_data_p->stream;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->stream);

  stream_p = ui_cb_data_p->stream;

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif
  ACE_ASSERT (stream_p);

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkTreeIter iterator_2;
  if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX (widget_in),
                                      &iterator_2))
    return; // <-- nothing selected
  GtkListStore* list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_SOURCE_NAME)));
  ACE_ASSERT (list_store_p);
  std::string device_string;
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
  g_value_init (&value, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if GTK_CHECK_VERSION(2,30,0)
  GValue value_2 = G_VALUE_INIT;
#else
  GValue value_2;
  ACE_OS::memset (&value_2, 0, sizeof (struct _GValue));
  g_value_init (&value_2, G_TYPE_STRING);
#endif // GTK_CHECK_VERSION (2,30,0)
#endif
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            0, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  device_string = g_value_get_string (&value);
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value_2);
  ACE_ASSERT (G_VALUE_TYPE (&value_2) == G_TYPE_STRING);
  device_string = g_value_get_string (&value_2);
  g_value_unset (&value_2);
#endif

  gint n_rows = 0;
  GtkToggleButton* toggle_button_p = NULL;

  list_store_p =
      GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FORMAT_NAME)));
  ACE_ASSERT (list_store_p);

//  bool result = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      //if (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader)
      //{
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader->Release ();
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.sourceReader =
      //    NULL;
      //} // end IF
      //if (mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.mediaSource)
      //{
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.mediaSource->Release ();
      //  mediafoundation_ui_cb_data_p->configuration->moduleHandlerConfiguration.mediaSource =
      //    NULL;
      //} // end IF
      if ((*mediafoundation_modulehandler_configuration_iterator).second.second.session)
      {
        (*mediafoundation_modulehandler_configuration_iterator).second.second.session->Release (); (*mediafoundation_modulehandler_configuration_iterator).second.second.session = NULL;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::getMediaSource (device_string,
                                                                       MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                       media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::getMediaSource(\"%s\"), returning\n"),
                    ACE_TEXT (device_string.c_str ())));
        return;
      } // end IF
#else
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (return;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      ACE_ASSERT (media_source_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif

  Stream_IStream_t* istream_p = dynamic_cast<Stream_IStream_t*> (stream_p);
  ACE_ASSERT (istream_p);
  std::string module_name;
  Stream_Module_t* module_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      module_name =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_WAVEIN_DEFAULT_NAME_STRING);
      module_p =
        const_cast<Stream_Module_t*> (istream_p->find (module_name));
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      module_name =
        ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_MEDIAFOUNDATION_DEFAULT_NAME_STRING);
      module_p =
        const_cast<Stream_Module_t*> (istream_p->find (module_name));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  module_name =
    ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING);
  module_p =
    const_cast<Stream_Module_t*> (istream_p->find (module_name));
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
  //Test_U_Dev_Mic_Source_DirectShow* directshow_source_impl_p = NULL;
  Test_U_Dev_Mic_Source_WaveIn* directshow_source_impl_p = NULL;
  Test_U_Dev_Mic_Source_MediaFoundation* mediafoundation_source_impl_p = NULL;
  IMFTopology* topology_p = NULL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (directshow_ui_cb_data_p->streamConfiguration)
      {
        directshow_ui_cb_data_p->streamConfiguration->Release (); directshow_ui_cb_data_p->streamConfiguration = NULL;
      } // end IF
      if ((*directshow_modulehandler_configuration_iterator).second.second.builder)
      {
        (*directshow_modulehandler_configuration_iterator).second.second.builder->Release (); (*directshow_modulehandler_configuration_iterator).second.second.builder = NULL;
      } // end IF

      IAMBufferNegotiation* buffer_negotiation_p = NULL;
      if (!Stream_Device_DirectShow_Tools::loadDeviceGraph (device_string,
                                                            CLSID_AudioInputDeviceCategory,
                                                            (*directshow_modulehandler_configuration_iterator).second.second.builder,
                                                            buffer_negotiation_p,
                                                            directshow_ui_cb_data_p->streamConfiguration,
                                                            directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->filterGraphConfiguration))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_DirectShow_Tools::loadDeviceGraph(\"%s\"), returning\n"),
                    ACE_TEXT (device_string.c_str ())));
        return;
      } // end IF
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second.builder);
      ACE_ASSERT (buffer_negotiation_p);
      ACE_ASSERT (directshow_ui_cb_data_p->streamConfiguration);

      buffer_negotiation_p->Release (); buffer_negotiation_p = NULL;

      result_2 =
        load_formats (directshow_ui_cb_data_p->streamConfiguration,
                      list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_source_impl_p =
        dynamic_cast<Test_U_Dev_Mic_Source_MediaFoundation*> (module_p->writer ());
      ACE_ASSERT (mediafoundation_source_impl_p);

      struct _MFRatio pixel_aspect_ratio = { 1, 1 };
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0601) // _WIN32_WINNT_WIN7
      if (!Stream_Device_MediaFoundation_Tools::loadDeviceTopology (device_string,
                                                                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID,
                                                                    media_source_p,
                                                                    mediafoundation_source_impl_p,
                                                                    topology_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_Device_MediaFoundation_Tools::loadDeviceTopology(), aborting\n")));
        goto error;
      } // end IF
#else
      ACE_ASSERT (false);
      ACE_NOTSUP;
      ACE_NOTREACHED (return;)
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0601)
      ACE_ASSERT (topology_p);

      // sanity check(s)
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT (!(*mediafoundation_modulehandler_configuration_iterator).second.second.session);
      if (!Stream_MediaFramework_MediaFoundation_Tools::setTopology (topology_p,
                                                                     (*mediafoundation_modulehandler_configuration_iterator).second.second.session,
                                                                     true))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::setTopology(), aborting\n")));
        goto error;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      topology_p->Release (); topology_p = NULL;

      if ((*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat)
      {
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->Release (); (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat = NULL;
      } // end IF
      HRESULT result =
        MFCreateMediaType (&(*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to MFCreateMediaType(): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat);
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->SetGUID (MF_MT_MAJOR_TYPE,
                                                                                                     MFMediaType_Audio);
      ACE_ASSERT (SUCCEEDED (result));
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->SetUINT32 (MF_MT_ALL_SAMPLES_INDEPENDENT,
                                                                                                       TRUE);
      ACE_ASSERT (SUCCEEDED (result));

      //if (!load_formats (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result_2 = load_formats (media_source_p,
                               list_store_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  int result = -1;
  struct _snd_pcm_hw_params* format_p = NULL;

  if (ui_cb_data_p->handle)
  {
    result = snd_pcm_close (ui_cb_data_p->handle);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed ALSA device\n")));
    ACE_ASSERT (ui_cb_data_p->handle == (*modulehandler_configuration_iterator).second.second.captureDeviceHandle);
    ui_cb_data_p->handle = NULL;
    (*modulehandler_configuration_iterator).second.second.captureDeviceHandle =
        NULL;
  } // end IF
  ACE_ASSERT (!ui_cb_data_p->handle);
//  int mode = STREAM_DEV_MIC_ALSA_DEFAULT_MODE;
  int mode = 0;
  //    snd_spcm_init();
  result = snd_pcm_open (&ui_cb_data_p->handle,
                         device_string.c_str (),
                         SND_PCM_STREAM_CAPTURE, mode);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_open(\"%s\") for capture: \"%s\", aborting\n"),
                ACE_TEXT (device_string.c_str ()),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  (*modulehandler_configuration_iterator).second.second.captureDeviceHandle =
    ui_cb_data_p->handle;
  ACE_ASSERT (ui_cb_data_p->handle);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("opened ALSA device (capture) \"%s\"\n"),
              ACE_TEXT (device_string.c_str ())));

//    snd_pcm_hw_params_alloca (&format_p);
  snd_pcm_hw_params_malloc (&format_p);
  if (!format_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to snd_pcm_hw_params_malloc(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  result = snd_pcm_hw_params_any (ui_cb_data_p->handle,
                                  format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_any(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  result = snd_pcm_hw_params (ui_cb_data_p->handle,
                              format_p);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params(): \"%s\", aborting\n"),
                ACE_TEXT (snd_strerror (result))));
    goto error;
  } // end IF
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: default format:\n%s"),
              ACE_TEXT (snd_pcm_name (ui_cb_data_p->handle)),
              ACE_TEXT (Stream_Device_Tools::formatToString (format_p).c_str ())));
  snd_pcm_hw_params_free (format_p); format_p = NULL;

  if (!Stream_Device_Tools::getFormat (ui_cb_data_p->handle,
                                       ui_cb_data_p->configuration->streamConfiguration.configuration_->format))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Device_Tools::getFormat(): \"%m\", aborting\n")));
    goto error;
  } // end IF
  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.access =
    STREAM_LIB_MIC_ALSA_DEFAULT_ACCESS;

  result_2 =
      load_formats (ui_cb_data_p->handle,
                    ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                    list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
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
    gtk_widget_set_sensitive (GTK_WIDGET (combo_box_p), TRUE);
    gtk_combo_box_set_active (combo_box_p, 0);
  } // end IF

  toggle_button_p =
      GTK_TOGGLE_BUTTON (gtk_builder_get_object ((*iterator).second.second,
                                                 ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_TOGGLEBUTTON_RECORD_NAME)));
  ACE_ASSERT (toggle_button_p);
  gtk_widget_set_sensitive (GTK_WIDGET (toggle_button_p), TRUE);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      media_source_p->Release (); media_source_p = NULL;
      topology_p->Release (); topology_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64

  return;

error:
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (media_source_p)
        media_source_p->Release ();
      if (topology_p)
        topology_p->Release ();
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  if (ui_cb_data_p->handle)
  {
    result = snd_pcm_close (ui_cb_data_p->handle);
    if (result < 0)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to snd_pcm_close(): \"%s\", continuing\n"),
                  ACE_TEXT (snd_strerror (result))));
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("closed ALSA device...\n")));
    ui_cb_data_p->handle = NULL;
  } // end IF

  if (format_p)
  {
    snd_pcm_hw_params_free (format_p); format_p = NULL;
  } // end IF
#endif // ACE_WIN32 || ACE_WIN64
} // combobox_source_changed_cb

void
combobox_format_changed_cb (GtkWidget* widget_in,
                            gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_format_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

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

#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)  
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  g_value_init (&value, G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
//  g_value_init (&value, G_TYPE_INT);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif // ACE_WIN32 || ACE_WIN64
  list_store_p =
    GTK_LIST_STORE (gtk_builder_get_object ((*iterator).second.second,
                                            ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_LISTSTORE_FREQUENCY_NAME)));
  ACE_ASSERT (list_store_p);

  bool result_2 = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#if defined (_WIN32_WINNT) && (_WIN32_WINNT >= 0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // _WIN32_WINNT) && (_WIN32_WINNT >= 0x0602)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.subtype =
        GUID_s;

      result_2 =
        load_sample_rates (directshow_ui_cb_data_p->streamConfiguration,
                           GUID_s,
                           list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second.session);

      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetGUID (MF_MT_SUBTYPE,
                                                                                                          GUID_s);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetGUID(MF_MT_SUBTYPE): \"%s\", returning\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second.session,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
        return;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)
      ACE_ASSERT (media_source_p);

      //if (!load_sample_rates (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result_2 = load_sample_rates (media_source_p,
                                    GUID_s,
                                    list_store_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->handle);

  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format =
      format_e;
  // *TODO*: format setting doesn't work yet
  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("format setting is currently broken, continuing\n")));
  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format =
      STREAM_LIB_MIC_ALSA_DEFAULT_FORMAT;

  result_2 =
      load_sample_rates (ui_cb_data_p->handle,
                         ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                         list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_rates(\"%s\"), aborting\n"),
                Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()));
    goto error_2;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_rates(), returning\n")));
    return;
#endif // ACE_WIN32 || ACE_WIN64
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
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      media_source_p->Release (); media_source_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH

  return;

error_2:
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (media_source_p)
      {
        media_source_p->Release (); media_source_p = NULL;
      }
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64
} // combobox_format_changed_cb

void
combobox_frequency_changed_cb (GtkWidget* widget_in,
                               gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_frequency_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

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
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  g_value_init (&value, G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
  ACE_UNUSED_ARG (format_e);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif // ACE_WIN32 || ACE_WIN64
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));
      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      //audio_info_header_p->nSamplesPerSec = sample_rate;

      result_2 = load_sample_resolutions (directshow_ui_cb_data_p->streamConfiguration,
                                          GUID_s,
                                          sample_rate,
                                          list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second.session);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_SAMPLES_PER_SECOND,
                                                                                                            sample_rate);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND,%u): \"%s\", returning\n"),
                    sample_rate,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second.session,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
        return;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      //if (!load_sample_resolutions (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result_2 = load_sample_resolutions (media_source_p,
                                          GUID_s,
                                          sample_rate,
                                          list_store_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->configuration);
  ACE_ASSERT (ui_cb_data_p->handle);

  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.rate =
      sample_rate;

  result_2 =
      load_sample_resolutions (ui_cb_data_p->handle,
                               ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                               list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_resolutions(\"%s\"), aborting\n"),
                Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()));
    goto error_2;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_sample_resolutions(%d), returning\n"),
                ui_cb_data_p->handle));
    return;
#endif // ACE_WIN32 || ACE_WIN64
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
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      media_source_p->Release (); media_source_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64

  return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
error_2:
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (media_source_p)
      {
        media_source_p->Release (); media_source_p = NULL;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64
} // combobox_frequency_changed_cb

void
combobox_resolution_changed_cb (GtkWidget* widget_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_resolution_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

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
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  g_value_init (&value, G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
  ACE_UNUSED_ARG (format_e);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif // ACE_WIN32 || ACE_WIN64
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
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
  IMFMediaSourceEx* media_source_p = NULL;
#else
  IMFMediaSource* media_source_p = NULL;
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));

      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      audio_info_header_p->wBitsPerSample = bits_per_sample;

      result_2 = load_channels (directshow_ui_cb_data_p->streamConfiguration,
                                GUID_s,
                                sample_rate,
                                bits_per_sample,
                                list_store_p);
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second.session);
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_BITS_PER_SAMPLE,
                                                                                                            bits_per_sample);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,%d): \"%s\", returning\n"),
                    bits_per_sample,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0600) // _WIN32_WINNT_VISTA
      if (!Stream_MediaFramework_MediaFoundation_Tools::getMediaSource ((*mediafoundation_modulehandler_configuration_iterator).second.second.session,
                                                                        media_source_p))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to Stream_MediaFramework_MediaFoundation_Tools::getMediaSource(), returning\n")));
        return;
      } // end IF
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0600)

      //if (!load_rates (data_p->configuration->moduleHandlerConfiguration.sourceReader,
      result_2 = load_channels (media_source_p,
                                GUID_s,
                                sample_rate,
                                bits_per_sample,
                                list_store_p);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->handle);
  ACE_ASSERT (ui_cb_data_p->configuration);

//  ui_cb_data_p->streamConfiguration.configuration->format->format =
//      bits_per_sample;

  result_2 =
      load_channels (ui_cb_data_p->handle,
                     ui_cb_data_p->configuration->streamConfiguration.configuration_->format,
                     list_store_p);
#endif // ACE_WIN32 || ACE_WIN64
  if (!result_2)
  {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_channels(\"%s\"), aborting\n"),
                Stream_MediaFramework_Tools::mediaSubTypeToString (GUID_s, STREAM_MEDIAFRAMEWORK_DIRECTSHOW).c_str ()));
    goto error_2;
#else
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ::load_channels(%d), returning\n"),
                ui_cb_data_p->handle));
    return;
#endif // ACE_WIN32 || ACE_WIN64
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
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      media_source_p->Release (); media_source_p = NULL;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64

  return;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
error_2:
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      break;
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (media_source_p)
      {
        media_source_p->Release (); media_source_p = NULL;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#endif // ACE_WIN32 || ACE_WIN64
} // combobox_resolution_changed_cb

void
combobox_channels_changed_cb (GtkWidget* widget_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::combobox_channels_changed_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

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
#if GTK_CHECK_VERSION(2,30,0)
  GValue value = G_VALUE_INIT;
#else
  GValue value;
  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
#endif // GTK_CHECK_VERSION (2,30,0)
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  g_value_init (&value, G_TYPE_STRING);
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            1, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_STRING);
  std::string format_string = g_value_get_string (&value);
#else
  gtk_tree_model_get_value (GTK_TREE_MODEL (list_store_p),
                            &iterator_2,
                            2, &value);
  ACE_ASSERT (G_VALUE_TYPE (&value) == G_TYPE_INT);
  enum _snd_pcm_format format_e =
      static_cast<enum _snd_pcm_format> (g_value_get_int (&value));
  ACE_UNUSED_ARG (format_e);
#endif // ACE_WIN32 || ACE_WIN64
  g_value_unset (&value);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct _GUID GUID_s = Common_Tools::StringToGUID (format_string);
  if (InlineIsEqualGUID (GUID_s, GUID_NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::StringToGUID(\"%s\"), returning\n"),
                ACE_TEXT (format_string.c_str ())));
    return;
  } // end IF
#else
//  snd_pcm_format_t format_i = snd_pcm_format_value (format_string.c_str ());
#endif // ACE_WIN32 || ACE_WIN64
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
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    { ACE_ASSERT (directshow_ui_cb_data_p->configuration);
      ACE_ASSERT (InlineIsEqualGUID (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.formattype, FORMAT_WaveFormatEx));

      struct tWAVEFORMATEX* audio_info_header_p =
        reinterpret_cast<struct tWAVEFORMATEX*> (directshow_ui_cb_data_p->configuration->streamConfiguration.configuration_->format.pbFormat);
      audio_info_header_p->nChannels = number_of_channels;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    { ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format);
      //ACE_ASSERT ((*mediafoundation_modulehandler_configuration_iterator).second.second.session);

      HRESULT result =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.configuration_->format->SetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                            number_of_channels);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,%d): \"%s\", returning\n"),
                    number_of_channels,
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p->handle);
  ACE_ASSERT (ui_cb_data_p->configuration);

  ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels =
      number_of_channels;
#endif // ACE_WIN32 || ACE_WIN64

  update_buffer_size (userData_in);
} // combobox_channels_changed_cb

gboolean
drawingarea_query_tooltip_cb (GtkWidget*  widget_in,
                              gint        x_in, gint y_in,
                              gboolean    keyboardMode_in,
                              GtkTooltip* tooltip_in,
                              gpointer    userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_query_tooltip_cb"));

  ACE_UNUSED_ARG (keyboardMode_in);

  // sanity check(s)
  ACE_ASSERT (userData_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

  Stream_IStream_t* istream_p = NULL;
  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode =
      STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_INVALID;
  unsigned int sample_size = 0; // bytes
  bool is_signed_format = false;
  unsigned int channels = 0;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  HRESULT result = E_FAIL;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());

      istream_p = dynamic_cast<Stream_IStream_t*> (directshow_ui_cb_data_p->stream);
      mode =
        (*directshow_modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode;
      ACE_ASSERT ((*directshow_modulehandler_configuration_iterator).second.second.outputFormat.cbFormat == sizeof (struct tWAVEFORMATEX));
      struct tWAVEFORMATEX* waveformatex_p =
        reinterpret_cast<struct tWAVEFORMATEX*> ((*directshow_modulehandler_configuration_iterator).second.second.outputFormat.pbFormat);
      ACE_ASSERT (waveformatex_p);
      sample_size = waveformatex_p->wBitsPerSample / 8;
      // *NOTE*: Microsoft(TM) uses signed little endian
      is_signed_format = true;
      channels = waveformatex_p->nChannels;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());

      istream_p = dynamic_cast<Stream_IStream_t*> (mediafoundation_ui_cb_data_p->stream);
      mode =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode;
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->GetUINT32 (MF_MT_SAMPLE_SIZE,
                                                                                                       &sample_size);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_SAMPLE_SIZE): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return FALSE;
      } // end IF
      // *NOTE*: Microsoft(TM) uses signed little endian
      is_signed_format = true;
      result =
        (*mediafoundation_modulehandler_configuration_iterator).second.second.outputFormat->GetUINT32 (MF_MT_AUDIO_NUM_CHANNELS,
                                                                                                       &channels);
      if (FAILED (result))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to IMFMediaType::GetUINT32(MF_MT_AUDIO_NUM_CHANNELS): \"%s\", aborting\n"),
                    ACE_TEXT (Common_Error_Tools::errorToString (result).c_str ())));
        return FALSE;
      } // end IF
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);
  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());

  istream_p = dynamic_cast<Stream_IStream_t*> (ui_cb_data_p->stream);
  mode =
    (*modulehandler_configuration_iterator).second.second.spectrumAnalyzer2DMode;
  is_signed_format =
      snd_pcm_format_signed (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format);
  sample_size =
      snd_pcm_format_physical_width (ui_cb_data_p->configuration->streamConfiguration.configuration_->format.format) / 8;
  channels =
    ui_cb_data_p->configuration->streamConfiguration.configuration_->format.channels;
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (istream_p);

  const Stream_Module_t* module_p = NULL;
  module_p =
    istream_p->find (ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_IStream::find(%s), returning\n"),
                ACE_TEXT (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING)));
    return FALSE;
  } // end IF
  Common_Math_FFT* math_fft_p =
    dynamic_cast<Common_Math_FFT*> (const_cast<Stream_Module_t*> (module_p)->writer ());
  if (!math_fft_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to dynamic_cast<Common_Math_FFT*>(%@), returning\n"),
                const_cast<Stream_Module_t*> (module_p)->writer ()));
    return FALSE;
  } // end IF

  GtkAllocation allocation;
  gtk_widget_get_allocation (widget_in,
                             &allocation);
  std::ostringstream converter;
  switch (mode)
  {
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE:
    {
      // *NOTE*: works for integer value types only
      // *WARNING*: correct only for two's complement value representations
      unsigned int maximum_value =
          (is_signed_format ? ((1 << ((sample_size * 8) - 1)) - 1)
                            : ((1 << (sample_size * 8)) - 1));
      double half_height = allocation.height / 2.0;
      // *TODO*: the value type depends on the format, so this isn't accurate
      if (is_signed_format)
        converter <<
          static_cast<int> ((half_height - y_in) * (maximum_value / half_height));
      else
        converter <<
          static_cast<unsigned int> ((half_height - y_in) * (maximum_value / half_height));
      break;
    }
    case STREAM_VISUALIZATION_SPECTRUMANALYZER_2DMODE_SPECTRUM:
    {
      unsigned int allocation_per_channel = (allocation.width / channels);
      unsigned int slot =
        static_cast<unsigned int> ((x_in % allocation_per_channel) * (math_fft_p->Slots () / static_cast<double> (allocation_per_channel)));
      converter << math_fft_p->Frequency (slot)
                << ACE_TEXT_ALWAYS_CHAR (" Hz");
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown mode (was: %d), returning\n"),
                  mode));
      return FALSE;
    }
  } // end SWITCH

  gtk_tooltip_set_text (tooltip_in,
                        converter.str ().c_str ());

  return TRUE;
}

void
drawingarea_realize_cb (GtkWidget* widget_in,
                        gpointer   userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_realize_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (userData_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  //GtkAllocation allocation;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

//  // *NOTE*: the surface / pixel buffer haven't been created yet
//  //         --> emit resize signal
//  GtkAllocation allocation;
//  gtk_widget_get_allocation (widget_in,
//                             &allocation);
//  GQuark detail = 0;
////#if GTK_CHECK_VERSION(2,30,0)
////  GValue value = G_VALUE_INIT;
////#else
////  GValue value;
////  ACE_OS::memset (&value, 0, sizeof (struct _GValue));
////  g_value_init (&value, G_TYPE_BOOLEAN);
////#endif // GTK_CHECK_VERSION (2,30,0)
//  g_signal_emit (G_OBJECT (widget_in),
//                 g_signal_lookup (ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
//                                  GTK_TYPE_WIDGET),
//                 detail,
//                 &allocation, userData_in,
//                 //&value);
//                 NULL);
//  //g_signal_emit_by_name (G_OBJECT (drawing_area_p),
//  //                       ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
//  //                       &allocation, userData_in,
//  //                       //&value,
//  //                       NULL);
//  //g_value_unset (&value);
} // drawingarea_realize_cb

void
drawingarea_size_allocate_cb (GtkWidget* widget_in,
                              GdkRectangle* allocation_in,
                              gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_size_allocate_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (allocation_in);
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* ui_cb_data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_p);
  ACE_ASSERT (ui_cb_data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T streamconfiguration_iterator =
      ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (streamconfiguration_iterator != ui_cb_data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  ACE_SYNCH_MUTEX* lock_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      lock_p = &directshow_ui_cb_data_p->surfaceLock;
      (*directshow_modulehandler_configuration_iterator).second.second.area2D =
        *allocation_in;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      lock_p = &mediafoundation_ui_cb_data_p->surfaceLock;
      (*mediafoundation_modulehandler_configuration_iterator).second.second.area2D =
        *allocation_in;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  lock_p = &ui_cb_data_p->surfaceLock;
  (*streamconfiguration_iterator).second.second.area2D = *allocation_in;
#endif // ACE_WIN32 || ACE_WIN64

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  if (!window_p)
    return; // <-- not realized yet
#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_t* surface_p =
      gdk_window_create_similar_image_surface (window_p,
                                               CAIRO_FORMAT_RGB24,
                                               allocation_in->width, allocation_in->height,
                                               1);
  if (!surface_p)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_window_create_similar_image_surface(), returning\n")));
    return;
  } // end IF
#else
  GdkPixbuf* pixel_buffer_p =
#if GTK_CHECK_VERSION(3,0,0)
    gdk_pixbuf_get_from_window (window_p,
                                0, 0, allocation_in->width, allocation_in->height);
#else
    gdk_pixbuf_get_from_drawable (NULL,                    // destination pixbuf --> create new
                                  GDK_DRAWABLE (window_p), // source window
                                  NULL,                    // colormap
                                  0, 0,                    // source coordinates (of drawable)
                                  0, 0,                    // destination coordinates
                                  allocation_in->width, allocation_in->height);
#endif // GTK_CHECK_VERSION(3,0,0)
  if (!pixel_buffer_p)
  { // *NOTE*: most probable reason: window hasn't been mapped yet
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(), aborting\n")));
    return;
  } // end IF
#endif // GTK_CHECK_VERSION(3,10,0)
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (surface_p);
#else
  ACE_ASSERT (pixel_buffer_p);
#endif // GTK_CHECK_VERSION(3,10,0)

  ACE_ASSERT (lock_p);
  { ACE_GUARD (ACE_SYNCH_MUTEX, aGuard, *lock_p);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
#if GTK_CHECK_VERSION(3,10,0)
        if (directshow_ui_cb_data_p->cairoSurface2D)
          cairo_surface_destroy (directshow_ui_cb_data_p->cairoSurface2D);
        directshow_ui_cb_data_p->cairoSurface2D = surface_p;
        (*directshow_modulehandler_configuration_iterator).second.second.cairoSurface2D =
          surface_p;
        ACE_ASSERT (directshow_ui_cb_data_p->cairoSurface2D);
#else
        // *NOTE*: in Gtk2, the surface is first created in the "configure-event"
        //         signal handler (see below)
        if (directshow_ui_cb_data_p->pixelBuffer2D)
          g_object_unref (directshow_ui_cb_data_p->pixelBuffer2D);
        directshow_ui_cb_data_p->pixelBuffer2D = pixel_buffer_p;
        (*directshow_modulehandler_configuration_iterator).second.second.pixelBuffer2D =
          pixel_buffer_p;
        ACE_ASSERT (directshow_ui_cb_data_p->pixelBuffer2D);
#endif // GTK_CHECK_VERSION(3,10,0)
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
#if GTK_CHECK_VERSION(3,10,0)
        if (mediafoundation_ui_cb_data_p->cairoSurface2D)
          cairo_surface_destroy (mediafoundation_ui_cb_data_p->cairoSurface2D);
        mediafoundation_ui_cb_data_p->cairoSurface2D = surface_p;
        (*mediafoundation_modulehandler_configuration_iterator).second.second.cairoSurface2D =
            surface_p;
        ACE_ASSERT (mediafoundation_ui_cb_data_p->cairoSurface2D);
#else
        // *NOTE*: in Gtk2, the surface is first created in the "configure-event"
        //         signal handler (see below)
        if (mediafoundation_ui_cb_data_p->pixelBuffer2D)
          g_object_unref (mediafoundation_ui_cb_data_p->pixelBuffer2D);
        mediafoundation_ui_cb_data_p->pixelBuffer2D = pixel_buffer_p;
        (*mediafoundation_modulehandler_configuration_iterator).second.second.pixelBuffer2D =
            pixel_buffer_p;
        ACE_ASSERT (mediafoundation_ui_cb_data_p->pixelBuffer2D);
#endif // GTK_CHECK_VERSION(3,10,0)
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                    ui_cb_data_base_p->mediaFramework));
        return;
      }
    } // end SWITCH
#else
#if GTK_CHECK_VERSION(3,10,0)
    if (ui_cb_data_p->cairoSurface2D)
      cairo_surface_destroy (ui_cb_data_p->cairoSurface2D);
    ui_cb_data_p->cairoSurface2D = surface_p;
    (*streamconfiguration_iterator).second.second.cairoSurface2D =
      surface_p;
    ACE_ASSERT (ui_cb_data_p->cairoSurface2D);
#elif GTK_CHECK_VERSION(2,0,0)
    if (ui_cb_data_p->pixelBuffer2D)
      g_object_unref (ui_cb_data_p->pixelBuffer2D);
    ui_cb_data_p->pixelBuffer2D = pixel_buffer_p;
    (*streamconfiguration_iterator).second.second.pixelBuffer2D =
      pixel_buffer_p;
    ACE_ASSERT (ui_cb_data_p->pixelBuffer2D);
#else
    ACE_ASSERT (false); // *TODO*
    ACE_NOTSUP_RETURN;
    ACE_NOTREACHED (return;)
#endif // GTK_CHECK_VERSION()
#endif // ACE_WIN32 || ACE_WIN64
  } // end lock scope

  Test_U_Common_ISet_t* notification_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      notification_p = directshow_ui_cb_data_p->resizeNotification;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      notification_p = mediafoundation_ui_cb_data_p->resizeNotification;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  notification_p = ui_cb_data_p->resizeNotification;
#endif // ACE_WIN32 || ACE_WIN64
  if (notification_p)
  {
    try {
#if GTK_CHECK_VERSION(3,10,0)
      notification_p->setP (surface_p);
#else
      notification_p->setP (pixel_buffer_p);
#endif // GTK_CHECK_VERSION(3,10,0)
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
    }
  } // end IF
} // drawingarea_size_allocate_cb

#if GTK_CHECK_VERSION(3,0,0)
gboolean
drawingarea_configure_event_cb (GtkWidget* widget_in,
                                GdkEvent* event_in,
                                gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_configure_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (event_in);
  ACE_ASSERT (userData_in);

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR_2 ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
      data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

  GtkDrawingArea* drawing_area_p =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_DRAWINGAREA_NAME)));
  ACE_ASSERT (drawing_area_p);

  GdkRectangle* area_p = NULL;
  ACE_SYNCH_MUTEX* lock_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      lock_p = &directshow_ui_cb_data_p->surfaceLock;
      if (widget_in == GTK_WIDGET (drawing_area_p))
        area_p = &directshow_ui_cb_data_p->area2D;
#if defined (GTKGL_SUPPORT)
      else
        area_p = &directshow_ui_cb_data_p->area3D;
#endif // GTKGL_SUPPORT
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      lock_p = &mediafoundation_ui_cb_data_p->surfaceLock;
      if (widget_in == GTK_WIDGET (drawing_area_p))
        area_p = &mediafoundation_ui_cb_data_p->area2D;
#if defined (GTKGL_SUPPORT)
      else
        area_p = &mediafoundation_ui_cb_data_p->area3D;
#endif // GTKGL_SUPPORT
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  lock_p = &data_p->surfaceLock;
  if (widget_in == GTK_WIDGET (drawing_area_p))
    area_p = &data_p->area2D;
#if defined (GTKGL_SUPPORT)
  else
    area_p = &data_p->area3D;
#endif // GTKGL_SUPPORT
#endif // ACE_WIN32 || ACE_WIN64
  ACE_ASSERT (lock_p);
  ACE_ASSERT (area_p);
  ACE_ASSERT (event_in->type == GDK_CONFIGURE);
  area_p->x = event_in->configure.x;
  area_p->y = event_in->configure.y;
  area_p->height = event_in->configure.height;
  area_p->width = event_in->configure.width;

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      if (widget_in == GTK_WIDGET (drawing_area_p))
        (*directshow_modulehandler_configuration_iterator).second.second.area2D =
          *area_p;
#if defined (GTKGL_SUPPORT)
      else
        (*directshow_modulehandler_configuration_iterator).second.second.area3D =
          *area_p;
#endif /* GTKGL_SUPPORT */
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      if (widget_in == GTK_WIDGET (drawing_area_p))
        (*mediafoundation_modulehandler_configuration_iterator).second.second.area2D =
          *area_p;
#if defined (GTKGL_SUPPORT)
      else
        (*mediafoundation_modulehandler_configuration_iterator).second.second.area3D =
          *area_p;
#endif /* GTKGL_SUPPORT */
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  if (widget_in == GTK_WIDGET (drawing_area_p))
    (*modulehandler_configuration_iterator).second.second.area2D = *area_p;
#if defined (GTKGL_SUPPORT)
  else
    (*modulehandler_configuration_iterator).second.second.area3D = *area_p;
#endif /* GTKGL_SUPPORT */
#endif // ACE_WIN32 || ACE_WIN64

  if (widget_in != GTK_WIDGET (drawing_area_p))
    return TRUE;

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  ACE_ASSERT (window_p);
#if GTK_CHECK_VERSION(3,10,0)
  cairo_surface_t* surface_p =
      gdk_window_create_similar_image_surface (window_p,
                                               CAIRO_FORMAT_RGB24,
                                               event_in->configure.width, event_in->configure.height,
                                               1);
  if (!surface_p)
  { // *NOTE*: most probable reason: window is not mapped
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_window_create_similar_image_surface(), aborting\n")));
    return FALSE;
  } // end IF
#else
  GdkPixbuf* pixel_buffer_p =
#if GTK_CHECK_VERSION(3,0,0)
    gdk_pixbuf_get_from_window (window_p,
                                0, 0, event_in->configure.width, event_in->configure.height);
#else
    gdk_pixbuf_get_from_drawable (NULL,                    // destination pixbuf --> create new
                                  GDK_DRAWABLE (window_p), // source window
                                  NULL,                    // colormap
                                  0, 0,                    // source coordinates (of drawable)
                                  0, 0,                    // destination coordinates
                                  event_in->configure.width, event_in->configure.height);
#endif // GTK_CHECK_VERSION(3,0,0)
  if (!pixel_buffer_p)
  { // *NOTE*: most probable reason: window hasn't been mapped yet
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_pixbuf_get_from_drawable(), aborting\n")));
    return TRUE;
  } // end IF
#endif // GTK_CHECK_VERSION(3,10,0)
#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (surface_p);
#else
  ACE_ASSERT (pixel_buffer_p);
#endif // GTK_CHECK_VERSION(3,10,0)

  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, *lock_p, FALSE);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    switch (ui_cb_data_base_p->mediaFramework)
    {
      case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
      {
#if GTK_CHECK_VERSION(3,10,0)
        if (directshow_ui_cb_data_p->cairoSurface2D)
          cairo_surface_destroy (directshow_ui_cb_data_p->cairoSurface2D);
        directshow_ui_cb_data_p->cairoSurface2D = surface_p;
        (*directshow_modulehandler_configuration_iterator).second.second.cairoSurface2D =
          surface_p;
        ACE_ASSERT (directshow_ui_cb_data_p->cairoSurface2D);
#else
        // *NOTE*: in Gtk2, the surface is first created in the "configure-event"
        //         signal handler (see below)
        if (directshow_ui_cb_data_p->pixelBuffer2D)
          g_object_unref (directshow_ui_cb_data_p->pixelBuffer2D);
        directshow_ui_cb_data_p->pixelBuffer2D = pixel_buffer_p;
        (*directshow_modulehandler_configuration_iterator).second.second.pixelBuffer2D =
          pixel_buffer_p;
        ACE_ASSERT (directshow_ui_cb_data_p->pixelBuffer2D);
#endif // GTK_CHECK_VERSION(3,10,0)
        break;
      }
      case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
      {
#if GTK_CHECK_VERSION(3,10,0)
        if (mediafoundation_ui_cb_data_p->cairoSurface2D)
          cairo_surface_destroy (mediafoundation_ui_cb_data_p->cairoSurface2D);
        mediafoundation_ui_cb_data_p->cairoSurface2D = surface_p;
        (*mediafoundation_modulehandler_configuration_iterator).second.second.cairoSurface2D =
          surface_p;
        ACE_ASSERT (mediafoundation_ui_cb_data_p->cairoSurface2D);
#else
        // *NOTE*: in Gtk2, the surface is first created in the "configure-event"
        //         signal handler (see below)
        if (mediafoundation_ui_cb_data_p->pixelBuffer2D)
          g_object_unref (mediafoundation_ui_cb_data_p->pixelBuffer2D);
        mediafoundation_ui_cb_data_p->pixelBuffer2D = pixel_buffer_p;
        (*mediafoundation_modulehandler_configuration_iterator).second.second.pixelBuffer2D =
          pixel_buffer_p;
        ACE_ASSERT (mediafoundation_ui_cb_data_p->pixelBuffer2D);
#endif // GTK_CHECK_VERSION(3,10,0)
        break;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                    ui_cb_data_base_p->mediaFramework));
        return false;
      }
    } // end SWITCH
#else
#if GTK_CHECK_VERSION(3,10,0)
    if (data_p->cairoSurface2D)
      cairo_surface_destroy (data_p->cairoSurface2D);
    data_p->cairoSurface2D = surface_p;
    (*modulehandler_configuration_iterator).second.second.cairoSurface2D =
      surface_p;
    ACE_ASSERT (data_p->cairoSurface2D);
#else
    if (data_p->pixelBuffer2D)
      g_object_unref (data_p->pixelBuffer2D);
    data_p->pixelBuffer2D = pixel_buffer_p;
    (*modulehandler_configuration_iterator).second.second.pixelBuffer2D =
      pixel_buffer_p;
    ACE_ASSERT (data_p->pixelBuffer2D);
#endif // GTK_CHECK_VERSION(3,10,0)
#endif // ACE_WIN32 || ACE_WIN64
  } // end lock scope

  Test_U_Common_ISet_t* notification_p = NULL;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      notification_p = directshow_ui_cb_data_p->resizeNotification;
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      notification_p = mediafoundation_ui_cb_data_p->resizeNotification;
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), aborting\n"),
                  ui_cb_data_base_p->mediaFramework));
      return false;
    }
  } // end SWITCH
#else
  notification_p = data_p->resizeNotification;
#endif // ACE_WIN32 || ACE_WIN64
  if (notification_p)
  {
    try {
#if GTK_CHECK_VERSION(3,10,0)
      notification_p->setP (surface_p);
#else
      notification_p->setP (pixel_buffer_p);
#endif // GTK_CHECK_VERSION(3,10,0)
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
    }
  } // end IF

  return TRUE;
} // drawingarea_configure_event_cb

gboolean
drawingarea_draw_cb (GtkWidget* widget_in,
                     cairo_t* context_in,
                     gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_2d_draw_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  ACE_ASSERT (widget_in);
  ACE_ASSERT (context_in);
  ACE_ASSERT (userData_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

//  bool destroy_context = false;
  // sanity check(s)
#if GTK_CHECK_VERSION(3,10,0)
  if (!ui_cb_data_base_p->cairoSurface2D)
#else
  if (!ui_cb_data_base_p->pixelBuffer2D)
#endif // GTK_CHECK_VERSION(3,10,0)
    return FALSE; // --> widget has not been realized yet

#if GTK_CHECK_VERSION(3,10,0)
  ACE_ASSERT (ui_cb_data_base_p->cairoSurface2D);
  cairo_set_source_surface (context_in,
                            ui_cb_data_base_p->cairoSurface2D,
                            0.0, 0.0);
                            //data_p->area2D->x, data_p->area2D->y);
#else
  ACE_ASSERT (ui_cb_data_base_p->pixelBuffer2D);

  // *TODO*: this currently segfaults on Linux, find out why
//  gdk_cairo_set_source_pixbuf (context_in,
//                               data_p->pixelBuffer2D,
//                               data_p->area2D.x, data_p->area2D.y);
  cairo_t* context_p =
    gdk_cairo_create (gtk_widget_get_window (widget_in));
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
    return FALSE;
  } // end IF
  gdk_cairo_set_source_pixbuf (context_p,
                               ui_cb_data_base_p->pixelBuffer2D,
                               ui_cb_data_base_p->area2D.x, ui_cb_data_base_p->area2D.y);
#endif // GTK_CHECK_VERSION(3,10,0)

#if GTK_CHECK_VERSION(3,0,0)
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->cairoSurfaceLock, FALSE);
//#else
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_p->pixelBufferLock, FALSE);
//#endif
    cairo_paint (context_in);
#else
    cairo_paint (context_p);
//  } // end lock scope
#endif // GTK_CHECK_VERSION(3,0,0)

#if GTK_CHECK_VERSION(3,0,0)
#else
  cairo_destroy (context_p);
#endif // GTK_CHECK_VERSION(3,0,0)

  return TRUE;
}
#else
gboolean
drawingarea_expose_event_cb (GtkWidget* widget_in,
                             GdkEvent* event_in,
                             gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::drawingarea_expose_event_cb"));

  // sanity check(s)
  ACE_ASSERT (widget_in);
//  ACE_ASSERT (event_in);
  ACE_UNUSED_ARG (event_in);
  ACE_ASSERT (userData_in);

  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (ui_cb_data_base_p);

//  bool destroy_context = false;
#if GTK_CHECK_VERSION(3,10,0)
  // sanity check(s)
  if (!ui_cb_data_base_p->cairoSurface2D)
    return FALSE; // --> widget has not been realized yet

  cairo_set_source_surface (context_in,
                            ui_cb_data_base_p->cairoSurface2D,
                            0.0, 0.0);
                            //data_p->area2D->x, data_p->area2D->y);
#else
  // sanity check(s)
  if (!ui_cb_data_base_p->pixelBuffer2D)
    return FALSE; // --> widget has not been realized yet

//  // *TODO*: this currently segfaults on Linux, find out why
//  gdk_cairo_set_source_pixbuf (context_in,
//                               data_p->pixelBuffer2D,
//                               0.0, 0.0);
  cairo_t* context_p =
    gdk_cairo_create (GDK_DRAWABLE (gtk_widget_get_window (widget_in)));
  if (!context_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
    return FALSE;
  } // end IF
  gdk_cairo_set_source_pixbuf (context_p,
                               ui_cb_data_base_p->pixelBuffer2D,
                               0.0, 0.0);
#endif // GTK_CHECK_VERSION(3,10,0)

#if GTK_CHECK_VERSION(3,10,0)
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_base_p->cairoSurfaceLock, FALSE);
//#else
//  { ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, aGuard, data_base_p->pixelBufferLock, FALSE);
//#endif
    cairo_paint (context_in);
#else
//    cairo_paint (context_in);
    cairo_paint (context_p);
//  } // end lock scope
#endif // GTK_CHECK_VERSION(3,10,0)

#if GTK_CHECK_VERSION(3,10,0)
#else
  cairo_destroy (context_p);
#endif // GTK_CHECK_VERSION(3,10,0)

  return TRUE;
}
#endif // GTK_CHECK_VERSION(3,0,0)

void
filechooserbutton_destination_file_set_cb (GtkFileChooserButton* button_in,
                                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::filechooserbutton_destination_file_set_cb"));

  // sanity check(s)
  struct Test_U_AudioEffect_UI_CBDataBase* ui_cb_data_base_p =
    static_cast<struct Test_U_AudioEffect_UI_CBDataBase*> (userData_in);
  ACE_ASSERT (ui_cb_data_base_p);

  Common_UI_GTK_Manager_t* gtk_manager_p =
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ();
  ACE_ASSERT (gtk_manager_p);
  const Common_UI_GTK_State_t& state_r = gtk_manager_p->getR ();
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct Test_U_AudioEffect_DirectShow_UI_CBData* directshow_ui_cb_data_p =
    NULL;
  struct Test_U_AudioEffect_MediaFoundation_UI_CBData* mediafoundation_ui_cb_data_p =
    NULL;
  Test_U_AudioEffect_MediaFoundation_StreamConfiguration_t::ITERATOR_T mediafoundation_modulehandler_configuration_iterator;
  Test_U_AudioEffect_DirectShow_StreamConfiguration_t::ITERATOR_T directshow_modulehandler_configuration_iterator;
  switch (ui_cb_data_base_p->mediaFramework)
  {
    case STREAM_MEDIAFRAMEWORK_DIRECTSHOW:
    {
      directshow_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_DirectShow_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (directshow_ui_cb_data_p);
      ACE_ASSERT (directshow_ui_cb_data_p->configuration);

      directshow_modulehandler_configuration_iterator =
        directshow_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (directshow_modulehandler_configuration_iterator != directshow_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    case STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION:
    {
      mediafoundation_ui_cb_data_p =
        static_cast<struct Test_U_AudioEffect_MediaFoundation_UI_CBData*> (userData_in);
      // sanity check(s)
      ACE_ASSERT (mediafoundation_ui_cb_data_p);
      ACE_ASSERT (mediafoundation_ui_cb_data_p->configuration);

      mediafoundation_modulehandler_configuration_iterator =
        mediafoundation_ui_cb_data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
      ACE_ASSERT (mediafoundation_modulehandler_configuration_iterator != mediafoundation_ui_cb_data_p->configuration->streamConfiguration.end ());
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media framework (was: %d), returning\n"),
                  ui_cb_data_base_p->mediaFramework));
      return;
    }
  } // end SWITCH
#else
  struct Test_U_AudioEffect_UI_CBData* data_p =
    static_cast<struct Test_U_AudioEffect_UI_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->configuration);

  Test_U_AudioEffect_ALSA_StreamConfiguration_t::ITERATOR_T modulehandler_configuration_iterator =
    data_p->configuration->streamConfiguration.find (ACE_TEXT_ALWAYS_CHAR (""));
  ACE_ASSERT (modulehandler_configuration_iterator != data_p->configuration->streamConfiguration.end ());
#endif // ACE_WIN32 || ACE_WIN64

  Common_UI_GTK_BuildersConstIterator_t iterator =
    state_r.builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != state_r.builders.end ());

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
  //GtkToggleAction* toggle_button_p =
  //  GTK_TOGGLE_ACTION (gtk_builder_get_object ((*iterator).second.second,
  //                                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_togglebutton_RECORD_NAME)));
  //ACE_ASSERT (toggle_button_p);
  //gtk_action_set_sensitive (GTK_ACTION (toggle_button_p),
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
